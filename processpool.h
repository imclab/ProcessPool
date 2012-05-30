#pragma once
#ifndef INTERNAL_PROCESS_POOL_H
#define INTERNAL_PROCESS_POOL_H

#include <string>
#include <vector>
#include <queue>
#include <map>

// From Google Style Guide
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);               \
    void operator=(const TypeName&)
#endif

#ifdef _WIN32

#include <windows.h>

class OSProcess {
    public:
        OSProcess();
        ~OSProcess();
        
        std::string WaitForChildMessage();
        void SendMessageToChild(const std::string &msg);
    private:
    
        HANDLE read_pipe_;
        HANDLE write_pipe_;
        PROCESS_INFORMATION process_info_;
    
        DISALLOW_COPY_AND_ASSIGN(OSProcess);
};

#endif //_WIN32

class ProcessPool;

class ProcessHandle {
public:
    void Process(const std::string& task);
    inline bool idle(){return idle_;}
    void ProcessInBackground();
    
    ProcessHandle(ProcessPool* parent_process_pool);
   
private:
    bool idle_;
    OSProcess os_process_;
    ProcessPool* parent_process_pool_;
    
    void CreateProcess();
    bool ProcessMessageFromChild(const std::string& msg);
    DISALLOW_COPY_AND_ASSIGN(ProcessHandle);
};


class ProcessPool {
public:
    typedef int (*JobFunctionPtr)(int argc, const char* argv[]);
    typedef std::map<std::string, ProcessPool::JobFunctionPtr> JobMap;
    enum Error{SUCCESS = 0,
               NO_IDLE_PROCESS = -1,
               NO_TASK_IN_QUEUE = -2};
    static bool AmIAWorkerProcess( int argc, char* argv[] );
    static int WorkerProcessMain(const JobMap &job_map);
    
    void NotifyTaskComplete();
    void Schedule(const std::string& task);
    ProcessPool(int size);
    ~ProcessPool();
private:
    // Change number of process handlers
    void Resize(int _size);
    
    // Attempts to start processing the first task in the queue in the first
    // idle process. Can return SUCCESS, NO_IDLE_PROCESS or NO_TASK_IN_QUEUE. 
    ProcessPool::Error ProcessFirstTaskInQueue();
    
    // Returns index of the first idle process in pool, or -1 if there
    // are no idle processes
    int GetIdleProcessIndex();
    
    std::vector<ProcessHandle*> processes_;
    std::queue<std::string> tasks_;
    DISALLOW_COPY_AND_ASSIGN(ProcessPool);
};

#endif //INTERNAL_PROCESS_POOL_H