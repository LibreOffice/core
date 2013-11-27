/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_THREADPOOL_HXX
#define SC_THREADPOOL_HXX

#include <sal/config.h>
#include <salhelper/thread.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <rtl/ref.hxx>
#include <vector>

class ThreadTask
{
public:
    virtual      ~ThreadTask() {}
    virtual void doWork() = 0;
};

/// A very basic thread pool implementation
class ThreadPool
{
public:
                ThreadPool( sal_Int32 nWorkers );
    virtual    ~ThreadPool();
    void        pushTask( ThreadTask *pTask /* takes ownership */ );
    void        waitUntilEmpty();
    void        waitUntilWorkersDone();

private:
    class ThreadWorker;
    friend class ThreadWorker;

    ThreadTask *waitForWork( osl::Condition &rNewWork );
    ThreadTask *popWork();

    osl::Mutex maGuard;
    osl::Condition maTasksEmpty;
    bool mbTerminate;
    std::vector< rtl::Reference< ThreadWorker > > maWorkers;
    std::vector< ThreadTask * >   maTasks;
};

#endif // SC_THREADPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
