/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/threadpool.hxx>

#include <rtl/instance.hxx>
#include <algorithm>
#include <memory>
#include <thread>

namespace comphelper {

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

    virtual void execute() SAL_OVERRIDE
    {
        ThreadTask *pTask;
        while ( ( pTask = waitForWork() ) )
        {
            pTask->doWork();
            delete pTask;
        }
    }

    ThreadTask *waitForWork()
    {
        ThreadTask *pRet = NULL;

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
    for( size_t i = 0; i < maWorkers.size(); i++ )
        maWorkers[ i ]->launch();
}

ThreadPool::~ThreadPool()
{
    waitAndCleanupWorkers();
}

struct ThreadPoolStatic : public rtl::StaticWithInit< std::shared_ptr< ThreadPool >,
                                                      ThreadPoolStatic >
{
    std::shared_ptr< ThreadPool > operator () () {
        sal_Int32 nThreads = std::max( std::thread::hardware_concurrency(), 1U );
        return std::shared_ptr< ThreadPool >( new ThreadPool( nThreads ) );
    };
};

ThreadPool& ThreadPool::getSharedOptimalPool()
{
    return *ThreadPoolStatic::get().get();
}

void ThreadPool::waitAndCleanupWorkers()
{
    waitUntilEmpty();

    osl::ResettableMutexGuard aGuard( maGuard );
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
    maTasks.insert( maTasks.begin(), pTask );

    // horrible beyond belief:
    for( size_t i = 0; i < maWorkers.size(); i++ )
        maWorkers[ i ]->signalNewWork();
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
        return NULL;
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

void ThreadPool::waitUntilEmpty()
{
    osl::ResettableMutexGuard aGuard( maGuard );

    if( maWorkers.empty() )
    { // no threads at all -> execute the work in-line
        ThreadTask *pTask;
        while ( ( pTask = popWork() ) )
        {
            pTask->doWork();
            delete pTask;
        }
    }
    else
    {
        aGuard.clear();
        maTasksComplete.wait();
        aGuard.reset();
    }
    assert( maTasks.empty() );
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
