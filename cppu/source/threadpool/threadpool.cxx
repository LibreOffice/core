/*************************************************************************
 *
 *  $RCSfile: threadpool.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <uno/threadpool.h>

#include "threadpool.hxx"
#include "thread.hxx"

using namespace ::std;
using namespace ::osl;

struct uno_threadpool_Handle
{
    /**
     * Global Threadidentifier of the waiting thread
     **/
    uno_threadpool_Handle( const ByteSequence &aThreadId_ , sal_Int64 nDisposeId_ )
        : aThreadId( aThreadId_ )
        , nDisposeId( nDisposeId_ )
        {}

    ByteSequence aThreadId;
    sal_Int64 nDisposeId;
};

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
#ifdef DEBUG
        if( !m_lst.empty() )
        {
            printf( "DisposedCallerList : %d left\n" , m_lst.size( ));
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
#ifdef DEBUG
        if( m_mapQueue.size() )
        {
            printf( "ThreadIdHashMap : %d left\n" , m_mapQueue.size() );
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
        osl_resetCondition( waitingThread.condition );
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
        void ( SAL_CALL * doRequest ) ( void * ) )
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
                    (*ii).second.second = new JobQueue( bAsynchron );
                    bCreateThread = sal_True;
                }
                pQueue = (*ii).second.second;
            }
            else
            {
                if( ! (*ii).second.first )
                {
                    (*ii).second.first = new JobQueue( bAsynchron );
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
            JobQueue *p = new JobQueue( sal_False );
            m_mapQueue[ aThreadId ] = pair< JobQueue * , JobQueue * > ( p , 0 );
        }
        else if( 0 == (*ii).second.first )
        {
            (*ii).second.first = new JobQueue( sal_False );
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


//------------------------------
//
// The C-Interface
//
//-------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL uno_threadpool_putRequest(
    sal_Sequence *pThreadId, void *pThreadSpecificData,
    void ( SAL_CALL * doRequest ) ( void *pThreadSpecificData ), sal_Bool bIsOneway )
    SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->addJob( pThreadId, bIsOneway, pThreadSpecificData,doRequest );
}



extern "C" SAL_DLLEXPORT void SAL_CALL uno_threadpool_putReply(
    sal_Sequence *pThreadId, void *pThreadSpecificData )
    SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->addJob( pThreadId, sal_False, pThreadSpecificData, 0 );
}


extern "C" SAL_DLLEXPORT  struct uno_threadpool_Handle * SAL_CALL
uno_threadpool_createHandle( sal_Int64 nDisposeId )
    SAL_THROW_EXTERN_C()
{
    sal_Sequence *pThreadId = 0;
    uno_getIdOfCurrentThread( &pThreadId );

    struct uno_threadpool_Handle *pHandle = new uno_threadpool_Handle( pThreadId, nDisposeId );
    ThreadPool::getInstance()->prepare( pThreadId );

    rtl_byte_sequence_release( pThreadId );

    return pHandle;
}

extern "C" SAL_DLLEXPORT void SAL_CALL uno_threadpool_enter(
    struct uno_threadpool_Handle *pHandle , void **ppThreadSpecificData )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( ppThreadSpecificData );

    *ppThreadSpecificData =
        ThreadPool::getInstance()->enter( pHandle->aThreadId , pHandle->nDisposeId );

    uno_releaseIdFromCurrentThread();
    delete pHandle;
}


extern "C" SAL_DLLEXPORT void SAL_CALL
uno_threadpool_disposeThreads( sal_Int64 nDisposeId )
    SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->dispose( nDisposeId );
}

extern "C" SAL_DLLEXPORT void SAL_CALL
uno_threadpool_stopDisposeThreads( sal_Int64 nDisposeId )
    SAL_THROW_EXTERN_C()
{
    ThreadPool::getInstance()->stopDisposing( nDisposeId );
}
