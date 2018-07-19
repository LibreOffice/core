/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_THREADPOOL_HXX
#define INCLUDED_COMPHELPER_THREADPOOL_HXX

#include <sal/config.h>
#include <rtl/ref.hxx>
#include <comphelper/comphelperdllapi.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <memory>

namespace comphelper
{
class ThreadTaskTag;
class ThreadPool;

class COMPHELPER_DLLPUBLIC ThreadTask
{
friend class ThreadPool;
friend struct std::default_delete<ThreadTask>;
    std::shared_ptr<ThreadTaskTag>  mpTag;

    /// execute this task
    void      exec();
protected:
    /// override to get your task performed by the pool
    virtual void doWork() = 0;
    /// once pushed ThreadTasks are destroyed by the pool
    virtual   ~ThreadTask() {}
public:
    ThreadTask(const std::shared_ptr<ThreadTaskTag>& pTag);
};

/// A very basic thread-safe thread pool implementation
class COMPHELPER_DLLPUBLIC ThreadPool final
{
public:
    /// returns a pointer to a shared pool with optimal thread
    /// count for the CPU
    static      ThreadPool& getSharedOptimalPool();

    static std::shared_ptr<ThreadTaskTag> createThreadTaskTag();

    static bool isTaskTagDone(const std::shared_ptr<ThreadTaskTag>&);

    /// returns a configurable max-concurrency
    /// limit to avoid spawning an unnecessarily
    /// large number of threads on high-core boxes.
    /// MAX_CONCURRENCY env. var. controls the cap.
    static      sal_Int32 getPreferredConcurrency();

    ThreadPool( sal_Int32 nWorkers );
    ~ThreadPool();

    /// push a new task onto the work queue
    void        pushTask( std::unique_ptr<ThreadTask> pTask);

    /// wait until all queued tasks associated with the tag are completed
    void        waitUntilDone(const std::shared_ptr<ThreadTaskTag>&);

    /// return the number of live worker threads
    sal_Int32   getWorkerCount() const { return mnWorkers; }

    /// wait until all work is completed, then join all threads
    void        shutdown();

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    class ThreadWorker;
    friend class ThreadWorker;

    /** Pop a work task
        @param  bWait - if set wait until task present or termination
        @return a new task to perform, or NULL if list empty or terminated
    */
    std::unique_ptr<ThreadTask> popWorkLocked( std::unique_lock< std::mutex > & rGuard, bool bWait );
    void shutdownLocked(std::unique_lock<std::mutex>&);

    /// signalled when all in-progress tasks are complete
    std::mutex              maMutex;
    std::condition_variable maTasksChanged;
    bool                    mbTerminate;
    std::size_t const       mnWorkers;
    std::vector< std::unique_ptr<ThreadTask> >   maTasks;
    std::vector< rtl::Reference< ThreadWorker > > maWorkers;
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_THREADPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
