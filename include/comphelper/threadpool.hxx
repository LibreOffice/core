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
#include <vector>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

class COMPHELPER_DLLPUBLIC ThreadTask
{
public:
    virtual      ~ThreadTask() {}
    virtual void doWork() = 0;
};

/// A very basic thread pool implementation
class COMPHELPER_DLLPUBLIC ThreadPool
{
public:
    /// returns a pointer to a shared pool with optimal thread
    /// count for the CPU
    static      ThreadPool& getSharedOptimalPool();

                ThreadPool( sal_Int32 nWorkers );
    virtual    ~ThreadPool();

    /// push a new task onto the work queue
    void        pushTask( ThreadTask *pTask /* takes ownership */ );

    /// wait until all queued tasks are completed
    void        waitUntilEmpty();

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

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_THREADPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
