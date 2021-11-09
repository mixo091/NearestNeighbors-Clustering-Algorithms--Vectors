#include <iostream>
#include <vector>
#include <unordered_map>
#include <math.h>
#include <random>
#include <chrono>

#include "../HashTable/HashTable.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Data/Data.hpp"
#include "../LSH/lsh.hpp"

using namespace std::chrono;

template <typename K>
class hypercube : public Lsh<K>
{
private:
    // number of neighbour buckets to check
    int probes; 
    // number of total vectors to check
    int M; 
    // p-> f(h(p)) : 0 ? 1
    std::unordered_map<int, int> f_map;
public:
    hypercube(int _probes, int _M, int _w, int _k, int _dim, int totalVectors, Data<K> *dataset) 
    :   probes(_probes), 
        M(_M), 
        Lsh<K>(1, pow(2, _k), _dim, _k, _w) // call constructor of Base class Lsh
                                        // where 1 is the number of Hash Tables and
                                        // pow(2, _k) is the size of the table
    {
        cout << "Constructing hypercube...\n"; 

        for(int i = 0; i < totalVectors; i++) {

            // this is our bucket
            int bucket_num = 0;      
            // bucket_num = create_hash(dataset[i]);
            for(int j = 0; j < this->numberOfHashFunctions; j++) {
                int h = this->hash_tables[0]->getHashFunction()->hfunction(dataset[i].getVector());

                int f = check_key(h);

                bucket_num += f * pow(2, this->numberOfHashFunctions - j - 1);
            }
            // insert in hash table
            this->hash_tables[0]->insertHyperCube(&dataset[i], bucket_num);
        } 
        // print_fmap();
    }

    ~hypercube() {
        // cout << "Destructing hypercube...\n";
        f_map.clear();
    }

    int check_key(int input) {
        /* Returns 0 ? 1 if input value exists, else new input inserted and 0 ? 1 created */
        unordered_map<int,int>::const_iterator found = f_map.find(input); 

        if(found == f_map.end()) {
            // insert new key and it's value
            int zero_or_one = coinToss();

            f_map.insert(std::make_pair(input, zero_or_one));

            return zero_or_one;

        } else 
            return found->second; 
    }

    int cube_hashing(Data<K> dataset) {
        /* Find bucket of hypercube for the vertex given */

        int result = 0;
        // temp values of h1....hk
        int h = 0;

        for(int i = 0; i < this->numberOfHashFunctions; i++) {
            // compute h1...hk
            h = this->hash_tables[0]->getHashFunction()->hfunction(dataset.getVector());

            // check if h value maps to 0 ? 1
            int zero_or_one = check_key(h);

            // to avoid overflow
            result += zero_or_one * pow(2, this->numberOfHashFunctions - i -1);
        }
        return result;
    }

    std::vector<int> get_neigbors_by_distance(int query_vertex, int diff, int maxDim) {
        /* Returns an array of distances diff, of query vertex with every
        *  possible vertex 
        */
        std::vector<int> possible_neighbors;
        for(int i = 0; i < pow(2, maxDim); i++) {
            if(hammingDistance(i, query_vertex) == diff) {
                possible_neighbors.push_back(i);
            }
        }

        random_device rd;
        mt19937 g(rd());
        shuffle(possible_neighbors.begin(), possible_neighbors.end(), g);

        // we check hashed bucked at first
        int count_probes = 1;

        // from every neigbor choose randomly
        std::vector<int> final_neighbors;
        for(int j = final_neighbors.size(); 
            j < probes && count_probes < possible_neighbors.size(); 
            j++, count_probes++)
         {
            final_neighbors.push_back(possible_neighbors[count_probes - 1]);
            cout << possible_neighbors[count_probes - 1] << " ";
        } cout << endl;

        return final_neighbors;
    }

    void ANN(Data<K> *qr_data, int qr_lines, Data<K> *in_data, int in_dataSize, int N, const string &out_file, float radius) {
        /* Aproximate NN. For every query create his neighbours with the help of hamming distance.
        * Hamming distance is calculated for every dimension from i=1,...,k. 
        * We check the bucket of hashed query at first, and then we move into the neighbours vector */

        ofstream output(out_file);
        if(!output.is_open()) {
            cout << "Error with output file\n";
            exit(-1);
        }
         
        // maps for the output
        map<double, int> my_map;
        map<double, int> result_map;

        std::vector<int> neighbors;
        std::vector<int> radius_nearest_neighbors;
    
        for(int i = 0; i < qr_lines ; i++) {

            int count_items_searched = 0;
            int count_probes_searched = 0;
            bool stop_searching = false;

            int bucket_num = cube_hashing(qr_data[i]);
           
            auto start = high_resolution_clock::now();

            cout << "for query-" << qr_data[i].getId() << " : " << bucket_num << endl;
            
            int maxDim = this->numberOfHashFunctions;
            for(int j = 1; j < maxDim && stop_searching == false; j++) {
                cout << "Checking hamming distance " << j << endl;
                neighbors = get_neigbors_by_distance(bucket_num, j, maxDim);

                this->hash_tables[0]->find_NN(&qr_data[i], my_map, N, neighbors, 
                                            bucket_num, M, probes, &count_items_searched, 
                                            &count_probes_searched);

                cout << "ANN ended. mymap size = " << my_map.size() << endl << endl;

                for (auto it = my_map.cbegin(); it != my_map.cend(); ++it) {
                    result_map.insert(pair<double, int>((*it).first, (*it).second));    
                }
                my_map.clear();

                // need to stop searching, used instead of flag
                if(count_items_searched >= M || count_probes_searched >= probes ) stop_searching = true;
            }
            auto stop = high_resolution_clock::now();
            auto durationCube = duration_cast<microseconds>(stop - start);   

            auto startBF = high_resolution_clock::now();
            // vector used for brute force
            vector<double> brute_force_v;

            //Brute force method for NN
            BruteForceNN(qr_data[i].getVector(), in_data, in_dataSize, &brute_force_v);

            // sort brute force vector
            sort(brute_force_v.begin(), brute_force_v.end());
  
            auto stopBF = high_resolution_clock::now();
            auto durationBF = duration_cast<microseconds>(stopBF - startBF);

            // call range search

            int n_ostos = 0;
            output << "\nQuery: " << qr_data[i].getId();
            for (auto it = result_map.cbegin(); it != result_map.cend(); ++it) {
                

                output<< "\nNearest neighbor-" << n_ostos + 1 << ": " << (*it).second \ 
                << "\ndistanceCube: " << (*it).first \ 
                << "\ndistanceTrue: " << brute_force_v.at(n_ostos) \
                << "\ntCube: " << durationCube.count() \
                << "\ntTrue:" << durationBF.count() \ 
                <<"\nR-near neighbors :" << endl;
                
                if(++n_ostos == N) {
                    break;
                }
            }
            result_map.clear();
            brute_force_v.clear();
            neighbors.clear();
            radius_nearest_neighbors.clear();
        }
        output.close();
    }


    // void range_search(Data<K> qr, double radius) {
        
    //     int bucket_num = cube_hashing(qr);
    //     std::vector<int> neighbors

    //     int maxDim = this->numberOfHashFunctions;
    //     for(int i = 1; i < maxDim; i++) {

    //         cout << "Checking hamming distance " << i << endl;
    //         neighbors = get_neigbors_by_distance(bucket_num, j, maxDim);

    //         for(int j = 0; j < (int) neighbors.size(); j++ )


    //         // if(euclidean_dist(qr.getVector(), neighbors.at(j)));
    // }



    void print_fmap() {
        for(auto &it : f_map){
            cout<< it.first << " : " << it.second << endl;
        }
    }

};



