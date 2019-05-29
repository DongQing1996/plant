/**
 * Author: Qing Dong, Kartik Lakhotia
 * Email id: qingdong@usc.edu, klakhoti@usc.edu
 * Date: 04-May-2018
 *
 * This code implements the parallelization of the pruned highway labeling algorithm which is the second phase of the hhl algorithm (Akiba et al. 2014).
 */

#include "labeling.hpp"
#include "ordering.hpp"
#include "akiba.hpp"
#include "dijkstra.hpp"
#include "query.hpp"
#include "omp.h"


int main(int argc, char** argv)
{
    
    unsigned int NUM_THREAD = 1;
	unsigned int NUM_SYNCH = 1;
	unsigned int baseline=0;
	float first_sync = 0.0001;
	float common_label_budget = 100;
	float phase_switch = 1024.0;
    hl::Graph gDij;
    std::vector<unsigned int> order;
    int query_mode = 5;
    unsigned int i=0;
   // for(unsigned int argi=0;argi<argc;argi++)
   // {
   //     printf("%s ",argv[argi]);
   // }
   // std::cout<<std::endl;
    for (unsigned int i=0; i<argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0) // vertex ordering
            hl::Order::read(argv[i+1], order);
        if (strcmp(argv[i], "-p") == 0) // number of threads
            NUM_THREAD=std::stoi(argv[i+1]);
        if (strcmp(argv[i], "-ft") == 0) // First sync point
            first_sync=std::atof(argv[i+1]);
        if (strcmp(argv[i], "-cb") == 0) // common label budget
            common_label_budget=std::atof(argv[i+1]);
        if (strcmp(argv[i], "-ps") == 0) // phasw switch ratio (tree_size/label_size)
            phase_switch=std::atof(argv[i+1]);
        if (strcmp(argv[i], "-b") == 0) // baseline: paraPLL
            baseline=std::stoi(argv[i+1]);
        if (strcmp(argv[i], "-qm") == 0) // baseline: paraPLL
            query_mode=std::stoi(argv[i+1]);
        if (strcmp(argv[i], "-g") == 0) // graph files
        {
            if (!gDij.read(argv[i+1])) {
                std::cerr << "Unable to read graph from raw " << argv[i+1] << std::endl;
                std::exit(1);
            }
        }
		if (strcmp(argv[i], "-s")==0) // number of synchronizations
		{
			NUM_SYNCH = std::stoi(argv[i+1]);
		}
    }

    unsigned int N = order.size();
    //std::cerr<<"Order size: "<<N<<" |V|: "<<gDij.get_n()<<std::endl;
    hl::Labeling labeling(N);
    //local_labeling=new hl::Labeling(N);
	std::vector <unsigned int> rev_map (N);
    #pragma omp parallel for num_threads (NUM_THREAD) schedule (dynamic, NUM_THREAD)
    for (unsigned int i=0; i<N; i++)
        rev_map[order[i]] = i;
//    hl::run_paraPLL(&gDij, order, *labeling, 0.01, NUM_THREAD);
    hl::run(&gDij, order, rev_map, labeling, first_sync, common_label_budget, phase_switch, NUM_THREAD);
////////////////////////////////////////
/////////////// Labeling ///////////////
////u///////////////////////////////////
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int num_queries = 10000000;
    //int num_queries = 100000;
    //remove common labeling
    labeling.remove_common(NUM_THREAD, world_rank, world_size);
    std::cout<<"QFDL # labels in node, "<<world_rank<<", is, "<<labeling.get_total()<<std::endl;
    std::cout<<"DFDL # cap in node, "<<world_rank<<", is, "<<labeling.get_cap()<<std::endl;
	std::vector<hl::Vertex> queries(num_queries*2);
	hl::generate_query(queries, num_queries, N, NUM_THREAD);
	std::vector<hl::Distance> dist(num_queries);
	hl::query(dist, queries, labeling, query_mode, NUM_THREAD); 

//    for (unsigned int i=0; i<argc; i++) {
//        if (strcmp(argv[i], "-l") == 0)  {//if write to a file
//		
//			std::string dataset(argv[i+1]);
//			std::string id = std::to_string(world_rank);
//			std::string size = std::to_string(world_size);
//			std::string name = dataset+"_"+id+"_"+size; 
//			char name_char[name.length()+1]; 
//			strcpy(name_char, name.c_str());
//            labeling->write(name_char);
//		}
//        if (strcmp(argv[i], "-qr") == 0) //if write query resutlt to a file
//            hl::write_result(dist, argv[i+1]);
//    }
    return 0;
}
