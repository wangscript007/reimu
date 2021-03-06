//
// Created by 苏畅 on 2019/6/21.
//

#include "ThreadingPool.h"
#include "event/Task.h"

namespace reimu{
    ThreadingPool::ThreadingPool(int threads, int wait_ms) :_threads(threads), _waitMs(wait_ms) {

    }

    ThreadingPool::~ThreadingPool() {
        Exit();
    }

    void ThreadingPool::Start() {
        for (auto &th : _threads) {
            auto t = std::thread([this](){
               while(!this->_tasks.Exited()){
                   TaskPtr task;
                    if (! _tasks.PopWait(&task)){
                        return;
                    }
                    if (task != nullptr) {
                        task->_cb();
                        task->SetStatus(Task::TaskStatus::FINISH);
                    }
               }
            });
            th.swap(t);
        }
    }

    bool ThreadingPool::AddTask(reimu::TaskPtr t) {
        return _tasks.Push(std::move(t));
    }

    void ThreadingPool::Exit() {
        _tasks.Exit();
        for (auto &t : _threads) {
            t.join();
        }
    }
}