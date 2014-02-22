/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "threadpool.hxx"

#include <algorithm>


class ThreadPool::ThreadWorker : public salhelper::Thread
{
    ThreadPool    *mpPool;
    osl::Condition maNewWork;
public:
    ThreadWorker( ThreadPool *pPool ) :
        salhelper::Thread("sheet-import-thread-pool"),
        mpPool( pPool ) {}

    virtual void execute()
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
            maNewWork.reset();

            if( mpPool->mbTerminate )
                break;

            aGuard.clear(); 

            maNewWork.wait();

            aGuard.reset(); 

            pRet = mpPool->popWork();
        }

        return pRet;
    }

    //
    
    //
    
    
    
    
    
    
    //
    void signalNewWork()
    {
        maNewWork.set();
    }
};

ThreadPool::ThreadPool( sal_Int32 nWorkers ) :
    mbTerminate( false )
{
    for( sal_Int32 i = 0; i < nWorkers; i++ )
        maWorkers.push_back( new ThreadWorker( this ) );

    maTasksEmpty.reset();

    osl::MutexGuard aGuard( maGuard );
    for( size_t i = 0; i < maWorkers.size(); i++ )
        maWorkers[ i ]->launch();
}

ThreadPool::~ThreadPool()
{
    waitUntilWorkersDone();
}



void ThreadPool::waitUntilWorkersDone()
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
        { 
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
    
    for( size_t i = 0; i < maWorkers.size(); i++ )
        maWorkers[ i ]->signalNewWork();
    maTasksEmpty.reset();
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
        maTasksEmpty.set();
    return NULL;
}

void ThreadPool::waitUntilEmpty()
{
    osl::ResettableMutexGuard aGuard( maGuard );

    if( maWorkers.empty() )
    { 
        ThreadTask *pTask;
        while ( ( pTask = popWork() ) )
        {
            pTask->doWork();
            delete pTask;
        }
        mbTerminate = true;
    }
    else
    {
        aGuard.clear();
        maTasksEmpty.wait();
        aGuard.reset();
    }
    assert( maTasks.empty() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
