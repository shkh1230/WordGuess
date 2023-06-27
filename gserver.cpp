#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>
#include <iterator>
#include <numeric>
#include <algorithm>

#include "LineInfo.h"

using namespace std;

const int READ_MAX_LEN = 100;
const int OK = 0;
const int CHILD_PID = 0;
const int FORK_ERROR = 0;

int main(int argc, char* argv[]) {
    
    try {
        
        if(argc != 2)
            throw domain_error(LineInfo("Argument missing ", __FILE__, __LINE__));
        
        vector<string> vectorStrings;
        ifstream FileStringStream(argv[1]);
        if(FileStringStream.fail()) {
            stringstream s;
            s << "Error opening file Random Numbers File " << argv[1] << endl;
            throw domain_error(LineInfo(s.str(), __FILE__, __LINE__));
        }
        
        istream_iterator<string> inputIt(FileStringStream);
        
        copy(inputIt, istream_iterator<string>(), back_inserter(vectorStrings));
        
        unsigned long NoOfElements = vectorStrings.size();
        static int clientcount = 0;
        
        string srd_cwr_req_np_str = "./srd_cwr_req_np";
        mkfifo(srd_cwr_req_np_str.c_str(), 0600);
        
        int srd_cwr_req_np_fd = open(srd_cwr_req_np_str.c_str(), O_RDONLY);
        if(srd_cwr_req_np_fd < OK)
            throw domain_error(LineInfo("open FAILURE", __FILE__, __LINE__));
        
        char swr_crd_np_ary[READ_MAX_LEN] = {0};
        if(read(srd_cwr_req_np_fd, swr_crd_np_ary, READ_MAX_LEN) < OK)
            throw domain_error(LineInfo("read FAILURE", __FILE__,__LINE__));
        
        string swr_crd_np_str(swr_crd_np_ary);
        
        close(srd_cwr_req_np_fd);
        unlink(srd_cwr_req_np_str.c_str());
        
        mkfifo(swr_crd_np_str.c_str(), 0600);
        
        int swr_crd_np_fd = open(swr_crd_np_str.c_str(), O_WRONLY);
        if(swr_crd_np_fd < OK)
            throw domain_error(LineInfo("open FAILURE", __FILE__, __LINE__));
        
        time_t t;
        srand((unsigned)time(&t));
        int randomIndexChoice = (rand() % NoOfElements);
        string randomword_str = vectorStrings[randomIndexChoice];
        string guessword_str(randomword_str.length(), '-');
        
        clientcount++;
        
        pid_t forkpid = fork();
        
        if(forkpid < 0) {
            stringstream s;
            s << "fork failed" << endl;
            throw domain_error(LineInfo(s.str(), __FILE__, __LINE__));
        }
        
        if(forkpid == CHILD_PID) {
            stringstream ss;
            ss << clientcount;
            string str = ss.str();
            string clientcount_str = ss.str();
            
            if(write(swr_crd_np_fd, clientcount_str.c_str(), clientcount_str.size() + 1) < OK)
                throw domain_error(LineInfo("write FAILURE", __FILE__, __LINE__));
            
            sleep(3);
            
            if(write(swr_crd_np_fd, randomword_str.c_str(), (randomword_str.size() + 1)) < OK)
                throw domain_error(LineInfo("write FAILURE", __FILE__, __LINE__));
            
            string srd_cwr_np_str = "./srd_cwr_np-";
            int i = getpid();
            string s;
            stringstream out;
            out << i;
            s = out.str();
            srd_cwr_np_str += s;
            
            if(write(swr_crd_np_fd, srd_cwr_np_str.c_str(), srd_cwr_np_str.size() + 1) < OK)
                throw domain_error(LineInfo("write FAILURE", __FILE__, __LINE__));
            
            mkfifo(srd_cwr_np_str.c_str(), 0600);
            
            int srd_cwr_np_fd = open(srd_cwr_np_str.c_str(), O_RDONLY);
            if(srd_cwr_np_fd < OK)
                throw domain_error(LineInfo("open FAILURE", __FILE__, __LINE__));
            
            do {
                
                if(write(swr_crd_np_fd, guessword_str.c_str(), (guessword_str.size() + 1)) < OK)
                    throw domain_error(LineInfo("write FAILURE", __FILE__, __LINE__));
                
                char guessletter_ary[READ_MAX_LEN] = {0};
                if(read(srd_cwr_np_fd, guessletter_ary, READ_MAX_LEN) < OK)
                    throw domain_error(LineInfo("read FAILURE", __FILE__, __LINE__));
                
                for(int i = 0; i < randomword_str.length(); i++)
                    if(randomword_str[i] == guessletter_ary[0])
                        guessword_str[i] = guessletter_ary[0];
                
            } while(true);
            
            
        }
        
        exit(EXIT_SUCCESS);
        
    }
    
    catch(exception& e) {
        cout << e.what() << endl << endl;
        
        
        cout << "Press the enter key once or twice to leave..." << endl << endl;
        
        
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
    
}

