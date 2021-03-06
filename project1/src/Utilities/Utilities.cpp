#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
#include <math.h>
#include <map>

#include "Utilities.hpp"

using namespace std;
// Function to get the args for LSH.
int lsh_parse_args ( 
    int argc, char **argv, 
    string *i_file, string *qr_file, string *o_file,
    int *k, int *L, int *N, int *R ) 
    {
    if(argc < 7) {
        cerr << "Usage of : " << argv[0] << " -i <input_file> -q <query_file> -o <output_file>" << endl;
        exit(-1);
    }
    if(strcmp(argv[1], "-i") != 0) {    
        cerr << "Please give the input file." << endl;
        return -1;
    }
    *i_file = argv[2];
    if(strcmp(argv[3], "-q") != 0) {    
        cerr << "Please give the input file." << endl;
        return -1;
    }
    *qr_file = argv[4];
    for(int i = 5; i < argc; i++) {
        if(strcmp(argv[i], "-k") == 0) {
            *k = atoi(argv[i + 1]);
        } else if(strcmp(argv[i], "-L") == 0) {
            *L = atoi(argv[i+1]);
        } else if(strcmp(argv[i], "-N") == 0) {
            *N = atoi(argv[i+1]);
        } else if(strcmp(argv[i], "-R") == 0) {
            *R = atoi(argv[i+1]);
        } else if(strcmp(argv[i], "-o") == 0) {
            *o_file = argv[i+1];
        }
    }
    if(o_file->empty()) {
        cerr << "No output file given.";
        return -1;
    }
    return 1;
}

void cube_parse_args(int argc, char **argv, string *in, string *qr, string *o, 
                int *k, int *L_or_M, int *probes, int *N, int *R) 
{
    if(argc < 7) {
        cerr << "Usage of : " << argv[0] << " -i <input_file> -q <query_file> -o <output_file>" << endl;
        exit(-1);
    }

    if(strcmp(argv[1], "-i") != 0) {    
        cerr << "Please give the input file." << endl;
        exit(-1);
    }
    *in = argv[2];

    if(strcmp(argv[3], "-q") != 0) {    
        cerr << "Please give the input file." << endl;
        exit(-1);
    } 
    *qr = argv[4];

    for(int i = 5; i < argc; i++) {
        if(strcmp(argv[i], "-k") == 0) *k = atoi(argv[i + 1]);
        else if(strcmp(argv[i], "-M") == 0) *L_or_M = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-N") == 0) *N = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-R") == 0) *R = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-o") == 0) *o = argv[i+1];
        else if(strcmp(argv[i], "-probes") == 0) *probes = atoi(argv[i+1]); 
    }
    if(o->empty()) {
        cerr << "No output file given.";
        exit(-1);    
    }
}

int getVectorDim(string str) {
    stringstream ss(str);
    string temp;
    int countDim = 0;
    while(getline(ss, temp, ' ')) {
        if(isspace(temp.at(0)) == 0)
            countDim++;
    }

    return countDim - 1; 
}

void calc_dimensions(int *tableSize, int *dim, string *filename)
{
    bool flag = true;

    ifstream input_file(*filename);   
    
    while(!input_file) {
        // user input
        cout << "Give correct file.";
        
        string new_file;
        cin >> new_file;
        // try new file
        input_file.open(new_file);

        // update filename
        filename->clear();
        *filename = new_file;
    }

    string str;
    while(getline(input_file, str)) {
        // do it only one time 
        if(flag) {
            flag = false;
                *dim = getVectorDim(str);
        }
        // cound total vectors
        (*tableSize)++;    
    }
    
    // close the file 
    input_file.close();
}

void normal_distribution_fun(double *n, float x, float y) {
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count(); 
    default_random_engine e (seed); 
  
    /* declaring normal distribution object 'distN' and initializing its mean and standard deviation fields. */
    /* Mean and standard deviation are distribution parameters of Normal distribution. Here, we have used mean=5, and standard deviation=2. You can take mean and standard deviation as per your choice */
    normal_distribution<double> distN(x, y);
    *n  = distN(e);
}

int modular_pow(int base, int exponent, int modulus)
{
    int result = 1;
    base = base % modulus;
    while (exponent > 0)
    {
        if(exponent & 1)
            result = (result * base) & modulus;
        exponent = exponent >> 1;
        base = (base*base) % modulus;
    }
    return result;
}

long long inn_product(int *h, int *r, int dim) {
    int product = 0;
    for(int i = 0; i < dim; i++) 
        product += h[i] * r[i];

    return product;
}   

unsigned long positive_modulo( unsigned long x, unsigned y) {
    return  ( ( x % y ) + y ) % y;
}

int coinToss() {
    random_device rd;
	mt19937 gen(rd());
  
        /* declaring normal distribution object 'distN' and initializing its mean and standard deviation fields. */
        /* Mean and standard deviation are distribution parameters of Normal distribution. Here, we have used mean=5, and standard deviation=2. You can take mean and standard deviation as per your choice */
    uniform_int_distribution<int> distN(0, 1);

    return distN(gen);
}

int hammingDistance(int n1, int n2){
    int x = n1 ^ n2;
    int setBits = 0;

    while (x > 0) {
        setBits += x & 1;
        x >>= 1;
    }

    return setBits;
}

