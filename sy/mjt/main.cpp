#include "Server.hpp"
#include "Client.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <mutex>

using namespace std;
std::mutex mtx;

void delay(double req_delay) {
  struct timeval start, check, end;
  double elapsed_seconds;
  gettimeofday(&start, NULL);
  do {
    gettimeofday(&check, NULL);
    elapsed_seconds = (check.tv_sec + (check.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));
  } while (elapsed_seconds < req_delay);
}

int addToBucket(vector<int> & vec, int delayCount, int idx) {
  mtx.lock();
  vec[idx] += delayCount;
  int ret = vec[idx];
  mtx.unlock();
  return ret;
}

void thread_func(int clientFD, vector<int> & vec) {
  char req[65535];
  int lenRecved = recv(clientFD, req, 65535, 0);
  string cnt = "";
  for(int i = 0; i < lenRecved; i++) {
    cnt.push_back(req[i]);
  }
  int commaIdx = cnt.find(",");
  int delayCount = stoi(cnt.substr(0, commaIdx));
  int bucketIdx = stoi(cnt.substr(commaIdx+1, cnt.size()-1-commaIdx));
  delay(delayCount);   // convert from int to double
  int res = addToBucket(vec, delayCount, bucketIdx);
  cout<<res<<endl;
  send(clientFD,&res,sizeof(res), 0);
}

int main(int argc, char * argv[]) {
  if(argc != 2) {
    cout << "Wrong argumet number!" << endl;
  }

  int len = atoi(argv[1]);
  vector<int> info(len, 0);

  Server server;
  server.build_server("0.0.0.0", "12345");
   while(true) {
    int tmpFd = server.accept();
    thread newThread(thread_func, tmpFd, ref(info));
    newThread.detach();
    }
  return 0;
}
