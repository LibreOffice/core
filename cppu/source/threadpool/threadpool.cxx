/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cassert>
#include <chrono>
#include <algorithm>
#include <unordered_map>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <sal/log.hxx>

#include <uno/threadpool.h>

#include "threadpool.hxx"
#include "thread.hxx"

using namespace ::std;
using namespace ::osl;
using namespace ::rtl;

namespace cppu_threadpool
{
    WaitingThread::WaitingThread(
        rtl::Reference<ORequestThread> const & theThread): thread(theThread)
    {}

    struct theDisposedCallerAdmin :
        public rtl::StaticWithInit< DisposedCallerAdminHolder, theDisposedCallerAdmin >
    {
        DisposedCallerAdminHolder operator () () {
            return std::make_shared<DisposedCallerAdmin>();
        }
    };

    DisposedCallerAdminHolder const & DisposedCallerAdmin::getInstance()
    {
        return theDisposedCallerAdmin::get();
    }

    DisposedCallerAdmin::~DisposedCallerAdmin()
    {
        SAL_WARN_IF( !m_vector.empty(), "cppu.threadpool", "DisposedCallerList :  " << m_vector.size() << " left");
    }

    void DisposedCallerAdmin::dispose( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        m_vector.push_back( nDisposeId );
    }

    void DisposedCallerAdmin::destroy( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        m_vector.erase(std::remove(m_vector.begin(), m_vector.end(), nDisposeId), m_vector.end());
    }

    bool DisposedCallerAdmin::isDisposed( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        return (std::find(m_vector.begin(), m_vector.end(), nDisposeId) != m_vector.end());
    }


    ThreadPool::ThreadPool() :
        m_DisposedCallerAdmin( DisposedCallerAdmin::getInstance() )
    {
    }

    ThreadPool::~ThreadPool()
    {
        SAL_WARN_IF( m_mapQueue.size(), "cppu.threadpool", "ThreadIdHashMap:  " << m_mapQueue.size() << " left");
    }

    void ThreadPool::dispose( sal_Int64 nDisposeId )
    {
        m_DisposedCallerAdmin->dispose( nDisposeId );

        MutexGuard guard( m_mutex );
        for (auto const& item :  m_mapQueue)
        {
            if( item.second.first )
            {
                item.second.first->dispose( nDisposeId );
            }
            if( item.second.second )
            {
                item.second.second->dispose( nDisposeId );
            }
        }
    }

    void ThreadPool::destroy( sal_Int64 nDisposeId )
    {
        m_DisposedCallerAdmin->destroy( nDisposeId );
    }

    /******************
     * This methods lets the thread wait a certain amount of time. If within this timespan
     * a new request comes in, this thread is reused. This is done only to improve performance,
     * it is not required for threadpool functionality.
     ******************/
    void ThreadPool::waitInPool( rtl::Reference< ORequestThread > const & pThread )
    {
        WaitingThread waitingThread(pThread);
        {
            MutexGuard guard( m_mutexWaitingThreadList );
            m_dequeThreads.push_front( &waitingThread );
        }

        // let the thread wait 2 seconds
        waitingThread.condition.wait( std::chrono::seconds(2) );

        {
            MutexGuard guard ( m_mutexWaitingThreadList );
            if( waitingThread.thread.is() )
            {
                // thread wasn't reused, remove it from the list
                WaitingThreadDeque::iterator ii = find(
                    m_dequeThreads.begin(), m_dequeThreads.end(), &waitingThread );
                OSL_ASSERT( ii != m_dequeThreads.end() );
                m_dequeThreads.erase( ii );
            }
        }
    }

    void ThreadPool::joinWorkers()
    {
        {
            MutexGuard guard( m_mutexWaitingThreadList );
            for (auto const& thread : m_dequeThreads)
            {
                // wake the threads up
                thread->condition.set();
            }
        }
        m_aThreadAdmin.join();
    }

    bool ThreadPool::createThread( JobQueue *pQueue ,
                                   const ByteSequence &aThreadId,
                                   bool bAsynchron )
    {
        {
            // Can a thread be reused ?
            MutexGuard guard( m_mutexWaitingThreadList );
            if( ! m_dequeThreads.empty() )
            {
                // inform the thread and let it go
                struct WaitingThread *pWaitingThread = m_dequeThreads.back();
                pWaitingThread->thread->setTask( pQueue , aThreadId , bAsynchron );
                pWaitingThread->thread = nullptr;

                // remove from list
                m_dequeThreads.pop_back();

                // let the thread go
                pWaitingThread->condition.set();
                return true;
            }
        }

        rtl::Reference pThread(
            new ORequestThread( this, pQueue , aThreadId, bAsynchron) );
        return pThread->launch();
    }

    bool ThreadPool::revokeQueue( const ByteSequence &aThreadId, bool bAsynchron )
    {
        MutexGuard guard( m_mutex );

        ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );
        OSL_ASSERT( ii != m_mapQueue.end() );

        if( bAsynchron )
        {
            if( ! (*ii).second.second->isEmpty() )
            {
                // another thread has put something into the queue
                return false;
            }

            (*ii).second.second = nullptr;
            if( (*ii).second.first )
            {
                // all oneway request have been processed, now
                // synchronous requests may go on
                (*ii).second.first->resume();
            }
        }
        else
        {
            if( ! (*ii).second.first->isEmpty() )
            {
                // another thread has put something into the queue
                return false;
            }
            (*ii).second.first = nullptr;
        }

        if( nullptr == (*ii).second.first && nullptr == (*ii).second.second )
        {
            m_mapQueue.erase( ii );
        }

        return true;
    }


    bool ThreadPool::addJob(
        const ByteSequence &aThreadId ,
        bool bAsynchron,
        void *pThreadSpecificData,
        RequestFun * doRequest )
    {
        bool bCreateThread = false;
        JobQueue *pQueue = nullptr;
        {
            MutexGuard guard( m_mutex );

            ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );

            if( ii == m_mapQueue.end() )
            {
                m_mapQueue[ aThreadId ] = pair < JobQueue * , JobQueue * > ( nullptr , nullptr );
                ii = m_mapQueue.find( aThreadId );
                OSL_ASSERT( ii != m_mapQueue.end() );
            }

            if( bAsynchron )
            {
                if( ! (*ii).second.second )
                {
                    (*ii).second.second = new JobQueue();
                    bCreateThread = true;
                }
                pQueue = (*ii).second.second;
            }
            else
            {
                if( ! (*ii).second.first )
                {
                    (*ii).second.first = new JobQueue();
                    bCreateThread = true;
                }
                pQueue = (*ii).second.first;

                if( (*ii).second.second && ( (*ii).second.second->isBusy() ) )
                {
                    pQueue->suspend();
                }
            }
            pQueue->add( pThreadSpecificData , doRequest );
        }

        return !bCreateThread || createThread( pQueue , aThreadId , bAsynchron);
    }

    void ThreadPool::prepare( const ByteSequence &aThreadId )
    {
        MutexGuard guard( m_mutex );

        ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );

        if( ii == m_mapQueue.end() )
        {
            JobQueue *p = new JobQueue();
            m_mapQueue[ aThreadId ] = pair< JobQueue * , JobQueue * > ( p , nullptr );
        }
        else if( nullptr == (*ii).second.first )
        {
            (*ii).second.first = new JobQueue();
        }
    }

    void * ThreadPool::enter( const ByteSequence & aThreadId , sal_Int64 nDisposeId )
    {
        JobQueue *pQueue = nullptr;
        {
            MutexGuard guard( m_mutex );

            ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );

            OSL_ASSERT( ii != m_mapQueue.end() );
            pQueue = (*ii).second.first;
        }

        OSL_ASSERT( pQueue );
        void *pReturn = pQueue->enter( nDisposeId );

        if( pQueue->isCallstackEmpty() )
        {
            if( revokeQueue( aThreadId , false) )
            {
                // remove queue
                delete pQueue;
            }
        }
        return pReturn;
    }
}

// All uno_ThreadPool handles in g_pThreadpoolHashSet with overlapping life
// spans share one ThreadPool instance.  When g_pThreadpoolHashSet becomes empty
// (within the last uno_threadpool_destroy) all worker threads spawned by that
// ThreadPool instance are joined (which implies that uno_threadpool_destroy
// must never be called from a worker thread); afterwards, the next call to
// uno_threadpool_create (if any) will lead to a new ThreadPool instance.

using namespace cppu_threadpool;

struct uno_ThreadPool_Equal
{
    bool operator () ( const uno_ThreadPool &a , const uno_ThreadPool &b ) const
        {
            return a == b;
        }
};

struct uno_ThreadPool_Hash
{
    std::size_t operator () ( const uno_ThreadPool &a  )  const
        {
            return reinterpret_cast<std::size_t>( a );
        }
};


typedef std::unordered_map< uno_ThreadPool, ThreadPoolHolder, uno_ThreadPool_Hash, uno_ThreadPool_Equal > ThreadpoolHashSet;

static ThreadpoolHashSet *g_pThreadpoolHashSet;

struct _uno_ThreadPool
{
    sal_Int32 dummy;
};

namespace {

ThreadPoolHolder getThreadPool( uno_ThreadPool hPool )
{
    MutexGuard guard( Mutex::getGlobalMutex() );
    assert( g_pThreadpoolHashSet != nullptr );
    ThreadpoolHashSet::iterator i( g_pThreadpoolHashSet->find(hPool) );
    assert( i != g_pThreadpoolHashSet->end() );
    return i->second;
}

}

extern "C" uno_ThreadPool SAL_CALL
uno_threadpool_create() SAL_THROW_EXTERN_C()
{
    MutexGuard guard( Mutex::getGlobalMutex() );
    ThreadPoolHolder p;
    if( ! g_pThreadpoolHashSet )
    {
        g_pThreadpoolHashSet = new ThreadpoolHashSet;
        p = new ThreadPool;
    }
    else
    {
        assert( !g_pThreadpoolHashSet->empty() );
        p = g_pThreadpoolHashSet->begin()->second;
    }

    // Just ensure that the handle is unique in the process (via heap)
    uno_ThreadPool h = new struct _uno_ThreadPool;
    g_pThreadpoolHashSet->emplace( h, p );
    return h;
}

extern "C" void SAL_CALL
uno_threadpool_attach( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C()
{
    sal_Sequence *pThreadId = nullptr;
    uno_getIdOfCurrentThread( &pThreadId );
    getThreadPool( hPool )->prepare( pThreadId );
    rtl_byte_sequence_release( pThreadId );
    uno_releaseIdFromCurrentThread();
}

extern "C" void SAL_CALL
uno_threadpool_enter( uno_ThreadPool hPool , void **ppJob )
    SAL_THROW_EXTERN_C()
{
    sal_Sequence *pThreadId = nullptr;
    uno_getIdOfCurrentThread( &pThreadId );
    *ppJob =
        getThreadPool( hPool )->enter(
            pThreadId,
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(hPool)) );
    rtl_byte_sequence_release( pThreadId );
    uno_releaseIdFromCurrentThread();
}

extern "C" void SAL_CALL
uno_threadpool_detach(SAL_UNUSED_PARAMETER uno_ThreadPool) SAL_THROW_EXTERN_C()
{
    // we might do here some tidying up in case a thread called attach but never detach
}

extern "C" void SAL_CALL
uno_threadpool_putJob(
    uno_ThreadPool hPool,
    sal_Sequence *pThreadId,
    void *pJob,
    void ( SAL_CALL * doRequest ) ( void *pThreadSpecificData ),
    sal_Bool bIsOneway ) SAL_THROW_EXTERN_C()
{
    if (!getThreadPool(hPool)->addJob( pThreadId, bIsOneway, pJob ,doRequest ))
    {
        SAL_WARN(
            "cppu.threadpool",
            "uno_threadpool_putJob in parallel with uno_threadpool_destroy");
    }
}

extern "C" void SAL_CALL
uno_threadpool_dispose( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C()
{
    getThreadPool(hPool)->dispose(
        sal::static_int_cast< sal_Int64 >(
            reinterpret_cast< sal_IntPtr >(hPool)) );
}

extern "C" void SAL_CALL
uno_threadpool_destroy( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C()
{
    ThreadPoolHolder p( getThreadPool(hPool) );
    p->destroy(
        sal::static_int_cast< sal_Int64 >(
            reinterpret_cast< sal_IntPtr >(hPool)) );

    bool empty;
    {
        OSL_ASSERT( g_pThreadpoolHashSet );

        MutexGuard guard( Mutex::getGlobalMutex() );

        ThreadpoolHashSet::iterator ii = g_pThreadpoolHashSet->find( hPool );
        OSL_ASSERT( ii != g_pThreadpoolHashSet->end() );
        g_pThreadpoolHashSet->erase( ii );
        delete hPool;

        empty = g_pThreadpoolHashSet->empty();
        if( empty )
        {
            delete g_pThreadpoolHashSet;
            g_pThreadpoolHashSet = nullptr;
        }
    }

    if( empty )
    {
        p->joinWorkers();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
