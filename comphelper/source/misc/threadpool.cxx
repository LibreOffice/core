/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/threadpool.hxx>

#include <com/sun/star/uno/Exception.hpp>
#include <rtl/instance.hxx>
#include <rtl/string.hxx>
#include <algorithm>
#include <memory>
#include <thread>

namespace comphelper {

/** prevent waiting for a task from inside a task */
#if defined DBG_UTIL && defined LINUX
static thread_local bool gbIsWorkerThread;
#endif

// used to group thread-tasks for waiting in waitTillDone()
class COMPHELPER_DLLPUBLIC ThreadTaskTag
{
    oslInterlockedCount  mnTasksWorking;
    osl::Condition       maTasksComplete;

public:
    ThreadTaskTag();
    bool           isDone();
    void           waitUntilDone();
    void           onTaskWorkerDone();
    void           onTaskPushed();
};


class ThreadPool::ThreadWorker : public salhelper::Thread
{
    ThreadPool    *mpPool;
    osl::Condition maNewWork;
    bool           mbWorking;
public:

    explicit ThreadWorker( ThreadPool *pPool ) :
        salhelper::Thread("thread-pool"),
        mpPool( pPool ),
        mbWorking( false )
    {
    }

    virtual void execute() override
    {
#if defined DBG_UTIL && defined LINUX
        gbIsWorkerThread = true;
#endif
        ThreadTask *pTask;
        while ( ( pTask = waitForWork() ) )
        {
            std::shared_ptr<ThreadTaskTag> pTag(pTask->getTag());
            try {
                pTask->doWork();
            }
            catch (const std::exception &e)
            {
                SAL_WARN("comphelper", "exception in thread worker while calling doWork(): " << e.what());
            }
            catch (const css::uno::Exception &e)
            {
                SAL_WARN("comphelper", "exception in thread worker while calling doWork(): " << e.Message);
            }
            try {
                delete pTask;
            }
            catch (const std::exception &e)
            {
                SAL_WARN("comphelper", "exception in thread worker while deleting task: " << e.what());
            }
            catch (const css::uno::Exception &e)
            {
                SAL_WARN("comphelper", "exception in thread worker while deleting task: " << e.Message);
            }
            pTag->onTaskWorkerDone();
        }
    }

    ThreadTask *waitForWork()
    {
        ThreadTask *pRet = nullptr;

        osl::ResettableMutexGuard aGuard( mpPool->maGuard );

        pRet = mpPool->popWork();

        while( !pRet )
        {
            if (mbWorking)
                mpPool->stopWork();
            mbWorking = false;
            maNewWork.reset();

            if( mpPool->mbTerminate )
                break;

            aGuard.clear(); // unlock

            maNewWork.wait();

            aGuard.reset(); // lock

            pRet = mpPool->popWork();
        }

        if (pRet)
        {
            if (!mbWorking)
                mpPool->startWork();
            mbWorking = true;
        }

        return pRet;
    }

    // Why a condition per worker thread - you may ask.
    //
    // Unfortunately the Windows synchronisation API that we wrap
    // is horribly inadequate cf.
    //    http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
    // The existing osl::Condition API should only ever be used
    // between one producer and one consumer thread to avoid the
    // lost wakeup problem.

    void signalNewWork()
    {
        maNewWork.set();
    }
};

ThreadPool::ThreadPool( sal_Int32 nWorkers ) :
    mnThreadsWorking( 0 ),
    mbTerminate( false )
{
    for( sal_Int32 i = 0; i < nWorkers; i++ )
        maWorkers.push_back( new ThreadWorker( this ) );

    maTasksComplete.set();

    osl::MutexGuard aGuard( maGuard );
    for(rtl::Reference<ThreadWorker> & rpWorker : maWorkers)
        rpWorker->launch();
}

ThreadPool::~ThreadPool()
{
    waitAndCleanupWorkers();
}

struct ThreadPoolStatic : public rtl::StaticWithInit< std::shared_ptr< ThreadPool >,
                                                      ThreadPoolStatic >
{
    std::shared_ptr< ThreadPool > operator () () {
        const sal_Int32 nThreads = ThreadPool::getPreferredConcurrency();
        return std::make_shared< ThreadPool >( nThreads );
    };
};

ThreadPool& ThreadPool::getSharedOptimalPool()
{
    return *ThreadPoolStatic::get().get();
}

sal_Int32 ThreadPool::getPreferredConcurrency()
{
    static sal_Int32 ThreadCount = 0;
    if (ThreadCount == 0)
    {
        const sal_Int32 nHardThreads = std::max(std::thread::hardware_concurrency(), 1U);
        sal_Int32 nThreads = nHardThreads;
        const char *pEnv = getenv("MAX_CONCURRENCY");
        if (pEnv != nullptr)
        {
            // Override with user/admin preferrence.
            nThreads = rtl_str_toInt32(pEnv, 10);
        }

        nThreads = std::min(nHardThreads, nThreads);
        ThreadCount = std::max<sal_Int32>(nThreads, 1);
    }

    return ThreadCount;
}

void ThreadPool::waitAndCleanupWorkers()
{
    osl::ResettableMutexGuard aGuard( maGuard );

    if( maWorkers.empty() )
    { // no threads at all -> execute the work in-line
        ThreadTask *pTask;
        while ( ( pTask = popWork() ) )
        {
            std::shared_ptr<ThreadTaskTag> pTag(pTask->getTag());
            pTask->doWork();
            delete pTask;
            pTag->onTaskWorkerDone();
        }
    }
    else
    {
        aGuard.clear();
        maTasksComplete.wait();
        aGuard.reset();
    }
    assert( maTasks.empty() );

    mbTerminate = true;

    while( !maWorkers.empty() )
    {
        rtl::Reference< ThreadWorker > xWorker = maWorkers.back();
        maWorkers.pop_back();
        assert(std::find(maWorkers.begin(), maWorkers.end(), xWorker)
                == maWorkers.end());
        xWorker->signalNewWork();
        aGuard.clear();
        { // unlocked
            xWorker->join();
            xWorker.clear();
        }
        aGuard.reset();
    }
}

void ThreadPool::pushTask( ThreadTask *pTask )
{
    osl::MutexGuard aGuard( maGuard );
    pTask->mpTag->onTaskPushed();
    maTasks.insert( maTasks.begin(), pTask );

    // horrible beyond belief:
    for(rtl::Reference<ThreadWorker> & rpWorker : maWorkers)
        rpWorker->signalNewWork();
    maTasksComplete.reset();
}

ThreadTask *ThreadPool::popWork()
{
    if( !maTasks.empty() )
    {
        ThreadTask *pTask = maTasks.back();
        maTasks.pop_back();
        return pTask;
    }
    else
        return nullptr;
}

void ThreadPool::startWork()
{
    mnThreadsWorking++;
}

void ThreadPool::stopWork()
{
    assert( mnThreadsWorking > 0 );
    if ( --mnThreadsWorking == 0 )
        maTasksComplete.set();
}

void ThreadPool::waitUntilDone(const std::shared_ptr<ThreadTaskTag>& rTag)
{
#if defined DBG_UTIL && defined LINUX
    assert(!gbIsWorkerThread && "cannot wait for tasks from inside a task");
#endif
    osl::ResettableMutexGuard aGuard( maGuard );

    if( maWorkers.empty() )
    { // no threads at all -> execute the work in-line
        ThreadTask *pTask;
        while ( ( pTask = popWork() ) )
        {
            std::shared_ptr<ThreadTaskTag> pTag(pTask->getTag());
            pTask->doWork();
            delete pTask;
            pTag->onTaskWorkerDone();
        }
    }
    aGuard.clear();
    rTag->waitUntilDone();
}

std::shared_ptr<ThreadTaskTag> ThreadPool::createThreadTaskTag()
{
    return std::make_shared<ThreadTaskTag>();
}

bool ThreadPool::isTaskTagDone(const std::shared_ptr<ThreadTaskTag>& pTag)
{
    return pTag->isDone();
}


ThreadTask::ThreadTask(const std::shared_ptr<ThreadTaskTag>& pTag)
    : mpTag(pTag)
{
}

ThreadTaskTag::ThreadTaskTag() : mnTasksWorking(0)
{
    maTasksComplete.set();
}

void ThreadTaskTag::onTaskPushed()
{
    oslInterlockedCount n = osl_atomic_increment(&mnTasksWorking);
    assert( n < 65536 ); // sanity checking
    (void)n; // avoid -Wunused-variable in release build
    maTasksComplete.reset();
}

void ThreadTaskTag::onTaskWorkerDone()
{
    sal_Int32 nCount = osl_atomic_decrement(&mnTasksWorking);
    assert(nCount >= 0);
    if (nCount == 0)
        maTasksComplete.set();
}

void ThreadTaskTag::waitUntilDone()
{
#if defined DBG_UTIL && defined LINUX
    assert(!gbIsWorkerThread && "cannot wait for tasks from inside a task");
#endif

#ifdef DBG_UTIL
    // 3 minute timeout in debug mode so our tests fail sooner rather than later
    osl::Condition::Result rv = maTasksComplete.wait(TimeValue { 3*60, 0 });
    assert(rv != osl::Condition::result_timeout);
#else
    // 10 minute timeout in production so the app eventually throws some kind of error
    if (maTasksComplete.wait(TimeValue { 10*60, 0 }) == osl::Condition::Result::result_timeout)
        throw std::runtime_error("timeout waiting for threadpool tasks");
#endif
}

bool ThreadTaskTag::isDone()
{
    return mnTasksWorking == 0;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
