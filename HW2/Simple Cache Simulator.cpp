#include <iostream>
#include <string>
#include <cstring>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <bitset>
#include <cmath>
using namespace std;

///////////////////////////////////////////////////////CACHE CLASS////////////////////////////////////////////////////////////////
class Cache_class
{
    public:
        int cache_capacity;
        int associativity;
        int blocksize;
        char replacement_policy;
    
    void init(int cache_capacity, int associativity, int blocksize);
    int get_cache_block_number (int cache_capacity, int blocksize);
    int get_set_number (int cache_block_number, int associativity);

};

void Cache_class::init(int cache_capacity, int associativity, int blocksize){                 //Cache initialization
    cache_capacity=cache_capacity;
    associativity=associativity;
    replacement_policy=replacement_policy;
};

int Cache_class::get_cache_block_number (int cache_capacity, int blocksize){                  // Calculate Block number in the cache
    return cache_capacity/blocksize;
};
int Cache_class::get_set_number (int cache_block_number, int associativity){                  // Calculate Set number in the cache
    return cache_block_number/associativity;
};


unsigned long long int get_address( char* address, int length){                              // Get the address from instruction and turn the address to an int type
    long long int addr;
    int i;
    string t;
    for(i=2;i<length;i++){                                                                  // Concatenate char one by one. Turn address into string type first.
    t = t + *(address+i);
    }
    addr = stoull(t,nullptr,16);                                                            // using stoull turn address into int type.

    return addr;
};


////////////////////////////////////////////////////////////////////RANDOM REPLACEMENT POLICY//////////////////////////////////////////////////////////////

void Rreplacement(unsigned long long int *array,int address,int associativity,unsigned long long int tag,int tagbits){                                                        
    srand ((int)(time (NULL)));                                                             // Set random seed
    int random_index = rand()%associativity;                                                // Get random number within the range of associativity
    array[random_index]=1;                                                                  
    array[random_index]=(array[random_index]<<tagbits)+tag;                                 // Replace the victim

};

/////////////////////////////////////////////////////////////////////LRU REPLACEMENT POLICY/////////////////////////////////////////////////////////////////
int LRU(unsigned long long int *array,int address,int associativity,unsigned long long int tag,int tagbits,int * LRU_array){
    
    int j=0;
    while(LRU_array[j]!=0){                                                                 // Find the least used one to be the victim.
        j=j+1;
    };
    array[j]=1;                                                                             // Replace the victim.
    array[j]=(array[j]<<tagbits)+tag;
    return j;
    
};

void LRU_order(int *array,int associativity,int index){                                 // Function to change the order of LRU array                      
    int temp = array[index];                                                            // store the victim value
    array[index]=associativity - 1;                                                     // Set the recent used block with highest value
    for(int i=0;i<associativity;i++){                                                   
        if( i== index){                                                                 // Change the value in LRU array one by one
            continue;                                                                   // Follow the Rule below.
        }
        else if(array[i]<temp){                                                         // RULE: Value which is smaller than victim value keep the same.                                      
            continue;
            }
        else if(array[i]>temp){                                                         // RULE: Value which is larger than victim value minus one.
            array[i]=array[i]-1;
            }
    };
};


int load_cache(unsigned long long int address,int set_number, int block_size,int associativity,unsigned long long int tag,int tagbits,char replacement_policy, unsigned long long int **array,int miss){
        int set;
        int i;                                                                            //  Load cache & Check HIT/MISS
        unsigned long long int  cache_tag;
        unsigned long long int one =1;                                                
        set = (address/block_size) % set_number;                                          //  Calculate the set which probably contains the data
        bool replacement_required = false;
        for(i=0;i<=associativity;i++){
            if((array[set][i]>>tagbits)==1){                                              // Check Valid-bit
                cache_tag = array[set][i]-(one<<tagbits);                                 // If Valid-bit = 1 , Take the cache_tag from cache. If Valid-bit = 0, says no other data are available. MISS+1, then load/replacement.
                if(cache_tag == tag){                                                     // Check Tag match Cache_tag or not?
                    return miss;                                                          // HIT -> MISS keep the same
                }
                else {
                    if(i==associativity){                                                 // IF check all the data in the set (i=associativity), not a hit. MISS occurs. Then to perform replacement function.
                        replacement_required = true;
                        miss = miss +1;
                        break;
                    }
                }
            }
            else {  
                miss = miss + 1;                                                          // Valid-bit = 0, no need to check left set space. MISS occurs.
                break;
            };
        };
        for(i=0;i<=associativity;i++){                                                   // LOAD THE DATA, If there is empty in the set.
            if (replacement_required){
                Rreplacement(array[set],address,associativity,tag,tagbits);
            }                                                   
            else if (array[set][i]!=0){continue;}                                        // Check the valid-bit, to find which is empty.
            else{
                array[set][i]=array[set][i]+1;                                           
                array[set][i]=array[set][i]<<tagbits;                                    // Set the valid-bit->1 
                array[set][i]=array[set][i]+tag;                                         // Change tag   
                break;
            }
        };
        return miss;
};


int load_cache_LRU(unsigned long long int address,int set_number,int block_size,int associativity,unsigned long long int tag,int tagbits,char replacement_policy, unsigned long long int **array,int** LRU_array,int miss){
        int set;
        int i;
        unsigned long long int  cache_tag;
        unsigned long long int one =1;
        set = (address/block_size) % set_number;
        bool replacement_required = false;                                                    // Mostly similar to the random one
        for(i=0;i<=associativity;i++){
            if(i==associativity){
                        replacement_required = true;
                        miss = miss +1;
                        break;
            }
            else if((array[set][i]>>tagbits)==1){
                cache_tag = array[set][i]-(one<<tagbits);
                if(cache_tag == tag){ 
                    LRU_order(LRU_array[set],associativity,i);                                // IF HIT, Change the order of LRU array.
                    return miss;
                }
                else{continue;}
            }
            else {
                miss = miss + 1;
                break;
            };
        };
        for(i=0;i<=associativity;i++){
            if (replacement_required){
                int index=LRU(array[set],address,associativity,tag,tagbits,LRU_array[set]);   // Perform LRU replacement and keep the victim index.
                LRU_order(LRU_array[set],associativity,index);                                // Change order in LRU array
                break;
            }
            else if (array[set][i]!=0){continue;}
            else{
                array[set][i]=array[set][i]+1;
                array[set][i]=array[set][i]<<tagbits;
                array[set][i]=array[set][i]+tag;
                LRU_order(LRU_array[set],associativity,i);
                break;
            }
        };
        return miss;
};

int main(){
    /////////////////////////////////////////////VARIABLE DECLARATION & SETTING/////////////////////////////////////////////////////////
    int cache_capacity ;
    int associativity ;
    int blocksize ;
    char replacement_policy ;
    string filename;
    
    int mem_block_number;
    int cache_block_number;
    int set_number;
    double instructions = 0;
    double read = 0;
    double write = 0;
    double miss = 0;
    double rd_miss = 0;
    double wr_miss = 0;
    double miss_rate = 0;
    double rd_miss_rate = 0;
    double wr_miss_rate = 0;

    cout<<"Input cache setting"<<endl;
    cout<<"cache_capacity   |"<<"   associativity   |"<<"   blocksize   |"<<"   replacement_policy   "<<"eg: 128 16 64 r/l (enter)"<<endl;
    cin>>cache_capacity>>associativity>>blocksize>>replacement_policy;

    cache_capacity=cache_capacity*1024;
    cout<<"\n"<<"Input trace directory"<<endl;
    cin.ignore();
    getline(cin,filename);
    cout<<"\n"<<endl;

//////////////////////////////////////////////////////////////////Using Cache Class & Calculating some parameters///////////////////////////////////// 
    Cache_class cache;                                                                                                                              //
                                                                                                                                                    //
    cache.init(cache_capacity, associativity, blocksize);                                                                                           //
                                                                                                                                                    //
    cache_block_number = cache.get_cache_block_number (cache_capacity, blocksize);                                                                  //
                                                                                                                                                    //
    set_number = cache.get_set_number (cache_block_number, associativity);                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    char instructionline[20];
    int len;
    unsigned long long int address;
    int offsetbit = log2(blocksize);
    int indexbit = log2(set_number);
    int tag;
    int set;

//////////////////////////////MEMORY ALLOCATING///////////////////////////OPEN THE MEMORY SPACE FOR CACHE /////////////////////////////////
    unsigned long long int ** cache_array = new unsigned long long int *[set_number];                                                    //
                                                                                                                                         
    for(int i = 0; i<set_number; ++i){                                                                                                   //
        cache_array[i] = new unsigned long long int [associativity];                                                                     
    };                                                                                                                                   //
                                                                                                                                            
    for(int i = 0 ; i < set_number ;++i)                                                                                                 //
	{                                                                                                                                    
		for( int j = 0 ; j < associativity ;++j)                                                                                             //
		{
			cache_array[i][j] = 0;                                                                                                             //
		}
	};                                                                                                                                     //
///////////////////////////////////////////////////////////////////////////OPEN THE MEMORY SPACE FOR  LRU TABLE////////////////////////////
    int **LRU_array = new int*[set_number];                                                                                              //        

    for(int i = 0; i<set_number; ++i){                                                                                                   //
        LRU_array[i] = new  int [associativity];
    };                                                                                                                                   //

    for(int i = 0 ; i < set_number ;++i)                                                                                                 //
	{
		for( int j = 0 ; j < associativity ;++j)                                                                                             //
		{
			LRU_array[i][j] = j;                                                                                                               //
		}
	};                                                                                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ifstream traces(filename, ios::in | ios::binary);  

    if (!traces) {
        cout << "not found"<< endl;
        system("pause");
        return 0;
    }

    while(traces.getline(instructionline, 20)){                                                             // Read the trace line by line

    instructions = instructions +1;                                                                         // Instruction counter
    len = strlen(instructionline);
    address = get_address(instructionline,len);
    tag = address>>(offsetbit+indexbit);                                                                    // Get tags
    int tagbits = 64 - offsetbit - indexbit;                                                                // Get the location of tag bits

    int temp = miss;
    if (replacement_policy == 'r'){                                                                         // Choose replacement policy
        miss = load_cache(address, set_number, blocksize, associativity,tag,tagbits,replacement_policy, cache_array,miss);
    }
    else if (replacement_policy == 'l'){
        miss = load_cache_LRU(address, set_number, blocksize, associativity,tag,tagbits,replacement_policy, cache_array, LRU_array,miss);
    }

    if(instructionline[0]=='r'){                                                                            // Read counter
            read = read + 1;
            }
        else if (instructionline[0]=='w'){                                                                  // Write counter
            write = write + 1;
            }

    if(miss == temp+1){                                                         
        if(instructionline[0]=='r'){                                                                         // Read miss counter
            rd_miss = rd_miss + 1;
        }
        else if (instructionline[0]=='w'){                                                                   // Write miss counter
            wr_miss = wr_miss + 1;
        }
    }
    else continue;

    };

    miss_rate = miss / instructions;                                                                        // Result 
    rd_miss_rate = rd_miss / read;
    wr_miss_rate = wr_miss / write;
    cout<<"Trace name: "<<filename<<endl;
    cout<<"cache nk:"<<cache_capacity/1024<<"     assoc:"<<associativity<<"     blocksize:"<<blocksize<<"     repl:"<<replacement_policy<<"\n"<<endl;
    cout<<"miss number:   "<<miss;
    cout<<"    miss rate:   "<<setprecision(10)<<miss_rate<<endl;
    cout<<"read miss:   "<<rd_miss;
    cout<<"    rd_miss_rate:   "<<setprecision(10)<<rd_miss_rate<<endl;
    cout<<"write miss:   "<<wr_miss;
    cout<<"    wr_miss_rate:   "<<setprecision(10)<<wr_miss_rate<<endl;

    delete []cache_array;                                                                                   // Close the space for Cache and LRU table 
    delete []LRU_array;
    traces.close();                                                                                         // Close the file

    system("pause");
    return 0;
}
