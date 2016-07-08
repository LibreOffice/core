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
#include <salhelper/thread.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <rtl/ref.hxx>
#include <comphelper/comphelperdllapi.h>
#include <vector>
#include <memory>

namespace comphelper
{
class ThreadTaskTag;
class ThreadPool;

class COMPHELPER_DLLPUBLIC ThreadTask
{
friend class ThreadPool;
    std::shared_ptr<ThreadTaskTag>  mpTag;
public:
    ThreadTask(const std::shared_ptr<ThreadTaskTag>& pTag);
    virtual      ~ThreadTask() {}
    virtual void doWork() = 0;
    const std::shared_ptr<ThreadTaskTag>& getTag() { return mpTag; }
};

/// A very basic thread pool implementation
class COMPHELPER_DLLPUBLIC ThreadPool
{
public:
    /// returns a pointer to a shared pool with optimal thread
    /// count for the CPU
    static      ThreadPool& getSharedOptimalPool();

    /// returns a configurable max-concurrency
    /// limit to avoid spawning an unnecessarily
    /// large number of threads on high-core boxes.
    /// MAX_CONCURRENCY envar controls the cap.
    static      sal_Int32 getPreferredConcurrency();

    ThreadPool( sal_Int32 nWorkers );
    virtual    ~ThreadPool();

    /// push a new task onto the work queue
    void        pushTask( ThreadTask *pTask /* takes ownership */ );

    /// wait until all queued tasks associated with the tag are completed
    void        waitUntilDone(const std::shared_ptr<ThreadTaskTag>&);

    /// return the number of live worker threads
    sal_Int32   getWorkerCount() const { return maWorkers.size(); }

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    class ThreadWorker;
    friend class ThreadWorker;

    /// wait until all work is completed, then join all threads
    void        waitAndCleanupWorkers();

    ThreadTask *popWork();
    void        startWork();
    void        stopWork();

    osl::Mutex     maGuard;
    sal_Int32      mnThreadsWorking;
    /// signalled when all in-progress tasks are complete
    osl::Condition maTasksComplete;
    bool           mbTerminate;
    std::vector< rtl::Reference< ThreadWorker > > maWorkers;
    std::vector< ThreadTask * >   maTasks;
};

// used to group thread-tasks for waiting in waitTillDone()
class COMPHELPER_DLLPUBLIC ThreadTaskTag
{
    oslInterlockedCount  mnTasksWorking;
    osl::Condition       maTasksComplete;

public:
    ThreadTaskTag();
    bool           isDone();
    void           waitUntilDone();
    void           threadTaskWorkerDone();
    void           newThreadTask();
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_THREADPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
