/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: threadpool.cxx,v $
 * $Revision: 1.18 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"
#include <hash_set>
#include <stdio.h>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <uno/threadpool.h>

#include "threadpool.hxx"
#include "thread.hxx"

using namespace ::std;
using namespace ::osl;

namespace cppu_threadpool
{
    DisposedCallerAdmin *DisposedCallerAdmin::getInstance()
    {
        static DisposedCallerAdmin *pDisposedCallerAdmin = 0;
        if( ! pDisposedCallerAdmin )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pDisposedCallerAdmin )
            {
                static DisposedCallerAdmin admin;
                pDisposedCallerAdmin = &admin;
            }
        }
        return pDisposedCallerAdmin;
    }

    DisposedCallerAdmin::~DisposedCallerAdmin()
    {
#if OSL_DEBUG_LEVEL > 1
        if( !m_lst.empty() )
        {
            printf( "DisposedCallerList : %lu left\n" , static_cast<unsigned long>(m_lst.size( )));
        }
#endif
    }

    void DisposedCallerAdmin::dispose( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        m_lst.push_back( nDisposeId );
    }

    void DisposedCallerAdmin::stopDisposing( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        for( DisposedCallerList::iterator ii = m_lst.begin() ;
             ii != m_lst.end() ;
             ++ ii )
        {
            if( (*ii) == nDisposeId )
            {
                m_lst.erase( ii );
                break;
            }
        }
    }

    sal_Bool DisposedCallerAdmin::isDisposed( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        for( DisposedCallerList::iterator ii = m_lst.begin() ;
             ii != m_lst.end() ;
             ++ ii )
        {
            if( (*ii) == nDisposeId )
            {
                return sal_True;
            }
        }
        return sal_False;
    }


    //-------------------------------------------------------------------------------
    ThreadPool::~ThreadPool()
    {
#if OSL_DEBUG_LEVEL > 1
        if( m_mapQueue.size() )
        {
            printf( "ThreadIdHashMap : %lu left\n" , static_cast<unsigned long>(m_mapQueue.size()) );
        }
#endif
    }
    ThreadPool *ThreadPool::getInstance()
    {
        static ThreadPool *pThreadPool = 0;
        if( ! pThreadPool )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pThreadPool )
            {
                static ThreadPool pool;
                pThreadPool = &pool;
            }
        }
        return pThreadPool;
    }


    void ThreadPool::dispose( sal_Int64 nDisposeId )
    {
        if( nDisposeId )
        {
            DisposedCallerAdmin::getInstance()->dispose( nDisposeId );

            MutexGuard guard( m_mutex );
            for( ThreadIdHashMap::iterator ii = m_mapQueue.begin() ;
                 ii != m_mapQueue.end();
                 ++ii)
            {
                if( (*ii).second.first )
                {
                    (*ii).second.first->dispose( nDisposeId );
                }
                if( (*ii).second.second )
                {
                    (*ii).second.second->dispose( nDisposeId );
                }
            }
        }
        else
        {
            {
                MutexGuard guard( m_mutexWaitingThreadList );
                for( WaitingThreadList::iterator ii = m_lstThreads.begin() ;
                     ii != m_lstThreads.end() ;
                     ++ ii )
                {
                    // wake the threads up
                    osl_setCondition( (*ii)->condition );
                }
            }
            ThreadAdmin::getInstance()->join();
        }
    }

    void ThreadPool::stopDisposing( sal_Int64 nDisposeId )
    {
        DisposedCallerAdmin::getInstance()->stopDisposing( nDisposeId );
    }

    /******************
     * This methods lets the thread wait a certain amount of time. If within this timespan
     * a new request comes in, this thread is reused. This is done only to improve performance,
     * it is not required for threadpool functionality.
     ******************/
    void ThreadPool::waitInPool( ORequestThread * pThread )
    {
        struct WaitingThread waitingThread;
        waitingThread.condition = osl_createCondition();
        waitingThread.thread = pThread;
        {
            MutexGuard guard( m_mutexWaitingThreadList );
            m_lstThreads.push_front( &waitingThread );
        }

        // let the thread wait 2 seconds
        TimeValue time = { 2 , 0 };
        osl_waitCondition( waitingThread.condition , &time );

        {
            MutexGuard guard ( m_mutexWaitingThreadList );
            if( waitingThread.thread )
            {
                // thread wasn't reused, remove it from the list
                WaitingThreadList::iterator ii = find(
                    m_lstThreads.begin(), m_lstThreads.end(), &waitingThread );
                OSL_ASSERT( ii != m_lstThreads.end() );
                m_lstThreads.erase( ii );
            }
        }

        osl_destroyCondition( waitingThread.condition );
    }

    void ThreadPool::createThread( JobQueue *pQueue ,
                                   const ByteSequence &aThreadId,
                                   sal_Bool bAsynchron )
    {
        sal_Bool bCreate = sal_True;
        {
            // Can a thread be reused ?
            MutexGuard guard( m_mutexWaitingThreadList );
            if( ! m_lstThreads.empty() )
            {
                // inform the thread and let it go
                struct WaitingThread *pWaitingThread = m_lstThreads.back();
                pWaitingThread->thread->setTask( pQueue , aThreadId , bAsynchron );
                pWaitingThread->thread = 0;

                // remove from list
                m_lstThreads.pop_back();

                // let the thread go
                osl_setCondition( pWaitingThread->condition );
                bCreate = sal_False;
            }
        }

        if( bCreate )
        {
            ORequestThread *pThread =
                new ORequestThread( pQueue , aThreadId, bAsynchron);
            // deletes itself !
            pThread->create();
        }
    }

    sal_Bool ThreadPool::revokeQueue( const ByteSequence &aThreadId, sal_Bool bAsynchron )
    {
        MutexGuard guard( m_mutex );

        ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );
        OSL_ASSERT( ii != m_mapQueue.end() );

        if( bAsynchron )
        {
            if( ! (*ii).second.second->isEmpty() )
            {
                // another thread has put something into the queue
                return sal_False;
            }

            (*ii).second.second = 0;
            if( (*ii).second.first )
            {
                // all oneway request have been processed, now
                // synchronus requests may go on
                (*ii).second.first->resume();
            }
        }
        else
        {
            if( ! (*ii).second.first->isEmpty() )
            {
                // another thread has put something into the queue
                return sal_False;
            }
            (*ii).second.first = 0;
        }

        if( 0 == (*ii).second.first && 0 == (*ii).second.second )
        {
            m_mapQueue.erase( ii );
        }

        return sal_True;
    }


    void ThreadPool::addJob(
        const ByteSequence &aThreadId ,
        sal_Bool bAsynchron,
        void *pThreadSpecificData,
        RequestFun * doRequest )
    {
        sal_Bool bCreateThread = sal_False;
        JobQueue *pQueue = 0;
        {
            MutexGuard guard( m_mutex );

            ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );

            if( ii == m_mapQueue.end() )
            {
                m_mapQueue[ aThreadId ] = pair < JobQueue * , JobQueue * > ( 0 , 0 );
                ii = m_mapQueue.find( aThreadId );
                OSL_ASSERT( ii != m_mapQueue.end() );
            }

            if( bAsynchron )
            {
                if( ! (*ii).second.second )
                {
                    (*ii).second.second = new JobQueue();
                    bCreateThread = sal_True;
                }
                pQueue = (*ii).second.second;
            }
            else
            {
                if( ! (*ii).second.first )
                {
                    (*ii).second.first = new JobQueue();
                    bCreateThread = sal_True;
                }
                pQueue = (*ii).second.first;

                if( (*ii).second.second && ( (*ii).second.second->isBusy() ) )
                {
                    pQueue->suspend();
                }
            }
            pQueue->add( pThreadSpecificData , doRequest );
        }

        if( bCreateThread )
        {
            createThread( pQueue , aThreadId , bAsynchron);
        }
    }

    void ThreadPool::prepare( const ByteSequence &aThreadId )
    {
        MutexGuard guard( m_mutex );

        ThreadIdHashMap::iterator ii = m_mapQueue.find( aThreadId );

        if( ii == m_mapQueue.end() )
        {
            JobQueue *p = new JobQueue();
            m_mapQueue[ aThreadId ] = pair< JobQueue * , JobQueue * > ( p , 0 );
        }
        else if( 0 == (*ii).second.first )
        {
            (*ii).second.first = new JobQueue();
        }
    }

    void * ThreadPool::enter( const ByteSequence & aThreadId , sal_Int64 nDisposeId )
    {
        JobQueue *pQueue = 0;
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
            if( revokeQueue( aThreadId , sal_False) )
            {
                // remove queue
                delete pQueue;
            }
        }
        return pReturn;
    }
}


using namespace cppu_threadpool;

struct uno_ThreadPool_Equal
{
    sal_Bool operator () ( const uno_ThreadPool &a , const uno_ThreadPool &b ) const
        {
            return a == b;
        }
};

struct uno_ThreadPool_Hash
{
    sal_Size operator () ( const uno_ThreadPool &a  )  const
        {
            return (sal_Size) a;
        }
};



typedef ::std::hash_set< uno_ThreadPool, uno_ThreadPool_Hash, uno_ThreadPool_Equal > ThreadpoolHashSet;

static ThreadpoolHashSet *g_pThreadpoolHashSet;

struct _uno_ThreadPool
{
    sal_Int32 dummy;
};

extern "C" uno_ThreadPool SAL_CALL
uno_threadpool_create() SAL_THROW_EXTERN_C()
{
    MutexGuard guard( Mutex::getGlobalMutex() );
    if( ! g_pThreadpoolHashSet )
    {
        g_pThreadpoolHashSet = new ThreadpoolHashSet();
    }

    // Just ensure that the handle is unique in the process (via heap)
    uno_ThreadPool h = new struct _uno_ThreadPool;
    g_pThreadpoolHashSet->insert( h );
    return h;
}

extern "C" void SAL_CALL
uno_threadpool_attach( uno_ThreadPool ) SAL_THROW_EXTERN_C()
{
    sal_Sequence *pThreadId = 0;
    uno_getIdOfCurrentThread( &pThreadId );
    ThreadPool::getInstance()->prepare( pThreadId );
    rtl_byte_sequence_release( pThreadId );
    uno_releaseIdFromCurrentThread();
}

extern "C" void SAL_CALL
uno_threadpool_enter( uno_ThreadPool hPool , void **ppJob )
    SAL_THROW_EXTERN_C()
{
    sal_Sequence *pThreadId = 0;
    uno_getIdOfCurrentThread( &pThreadId );
    *ppJob =
        ThreadPool::getInstance()->enter(
            pThreadId,
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(hPool)) );
    rtl_byte_sequence_release( pThreadId );
    uno_releaseIdFromCurrentThread();
}

extern "C" void SAL_CALL
uno_threadpool_detach( uno_ThreadPool ) SAL_THROW_EXTERN_C()
{
    // we might do here some tiding up in case a thread called attach but never detach
}

extern "C" void SAL_CALL
uno_threadpool_putJob(
    uno_ThreadPool,
    sal_Sequence *pThreadId,
    void *pJob,
    void ( SAL_CALL * doRequest ) ( void *pThreadSpecificData ),
    sal_Bool bIsOneway ) SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->addJob( pThreadId, bIsOneway, pJob ,doRequest );
}

extern "C" void SAL_CALL
uno_threadpool_dispose( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->dispose(
        sal::static_int_cast< sal_Int64 >(
            reinterpret_cast< sal_IntPtr >(hPool)) );
}

extern "C" void SAL_CALL
uno_threadpool_destroy( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->stopDisposing(
        sal::static_int_cast< sal_Int64 >(
            reinterpret_cast< sal_IntPtr >(hPool)) );

    if( hPool )
    {
        // special treatment for 0 !
        OSL_ASSERT( g_pThreadpoolHashSet );

        MutexGuard guard( Mutex::getGlobalMutex() );

        ThreadpoolHashSet::iterator ii = g_pThreadpoolHashSet->find( hPool );
        OSL_ASSERT( ii != g_pThreadpoolHashSet->end() );
        g_pThreadpoolHashSet->erase( ii );
        delete hPool;

        if( g_pThreadpoolHashSet->empty() )
        {
            delete g_pThreadpoolHashSet;
            g_pThreadpoolHashSet = 0;
        }
    }
}
