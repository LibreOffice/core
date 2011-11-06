/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"
#include <stdio.h>
#include <osl/diagnose.h>
#include <uno/threadpool.h>

#include <rtl/instance.hxx>

#include "thread.hxx"
#include "jobqueue.hxx"
#include "threadpool.hxx"


using namespace osl;
extern "C" {

void SAL_CALL cppu_requestThreadWorker( void *pVoid )
{
    ::cppu_threadpool::ORequestThread *pThread = ( ::cppu_threadpool::ORequestThread * ) pVoid;

    pThread->run();
    pThread->onTerminated();
}

}

namespace cppu_threadpool {

// ----------------------------------------------------------------------------------
    ThreadAdmin::~ThreadAdmin()
    {
#if OSL_DEBUG_LEVEL > 1
        if( m_lst.size() )
        {
            fprintf( stderr, "%lu Threads left\n" , static_cast<unsigned long>(m_lst.size()) );
        }
#endif
    }

    void ThreadAdmin::add( ORequestThread *p )
    {
        MutexGuard aGuard( m_mutex );
        m_lst.push_back( p );
    }

    void ThreadAdmin::remove( ORequestThread * p )
    {
        MutexGuard aGuard( m_mutex );
        ::std::list< ORequestThread * >::iterator ii = ::std::find( m_lst.begin(), m_lst.end(), p );
        OSL_ASSERT( ii != m_lst.end() );
        m_lst.erase( ii );
    }

    void ThreadAdmin::join()
    {
        ORequestThread *pCurrent;
        do
        {
            pCurrent = 0;
            {
                MutexGuard aGuard( m_mutex );
                if( ! m_lst.empty() )
                {
                    pCurrent = m_lst.front();
                    pCurrent->setDeleteSelf( sal_False );
                }
            }
            if ( pCurrent )
            {
                pCurrent->join();
                delete pCurrent;
            }
        } while( pCurrent );
    }

    struct theThreadAdmin : public rtl::StaticWithInit< ThreadAdminHolder, theThreadAdmin >
    {
        ThreadAdminHolder operator () () {
            ThreadAdminHolder aRet(new ThreadAdmin());
            return aRet;
        }
    };

    ThreadAdminHolder& ThreadAdmin::getInstance()
    {
        return theThreadAdmin::get();
    }

// ----------------------------------------------------------------------------------
    ORequestThread::ORequestThread( JobQueue *pQueue,
                                    const ByteSequence &aThreadId,
                                    sal_Bool bAsynchron )
        : m_thread( 0 )
        , m_aThreadAdmin( ThreadAdmin::getInstance() )
        , m_pQueue( pQueue )
        , m_aThreadId( aThreadId )
        , m_bAsynchron( bAsynchron )
        , m_bDeleteSelf( sal_True )
    {
        m_aThreadAdmin->add( this );
    }


    ORequestThread::~ORequestThread()
    {
        if (m_thread != 0)
        {
            osl_destroyThread(m_thread);
        }
    }


    void ORequestThread::setTask( JobQueue *pQueue,
                                  const ByteSequence &aThreadId,
                                  sal_Bool bAsynchron )
    {
        m_pQueue = pQueue;
        m_aThreadId = aThreadId;
        m_bAsynchron = bAsynchron;
    }

    sal_Bool ORequestThread::create()
    {
        OSL_ASSERT(m_thread == 0);  // only one running thread per instance

        m_thread = osl_createSuspendedThread( cppu_requestThreadWorker, (void*)this);
        if ( m_thread )
        {
            osl_resumeThread( m_thread );
        }

        return m_thread != 0;
    }

    void ORequestThread::join()
    {
        osl_joinWithThread( m_thread );
    }

    void ORequestThread::onTerminated()
    {
        m_aThreadAdmin->remove( this );
        if( m_bDeleteSelf )
        {
            delete this;
        }
    }

    void ORequestThread::run()
    {
        ThreadPoolHolder theThreadPool = cppu_threadpool::ThreadPool::getInstance();

        while ( m_pQueue )
        {
            if( ! m_bAsynchron )
            {
                if ( !uno_bindIdToCurrentThread( m_aThreadId.getHandle() ) )
                {
                    OSL_ASSERT( false );
                }
            }

            while( ! m_pQueue->isEmpty() )
            {
                // Note : Oneways should not get a disposable disposeid,
                //        It does not make sense to dispose a call in this state.
                //        That's way we put it an disposeid, that can't be used otherwise.
                m_pQueue->enter(
                    sal::static_int_cast< sal_Int64 >(
                        reinterpret_cast< sal_IntPtr >(this)),
                    sal_True );

                if( m_pQueue->isEmpty() )
                {
                    theThreadPool->revokeQueue( m_aThreadId , m_bAsynchron );
                    // Note : revokeQueue might have failed because m_pQueue.isEmpty()
                    //        may be false (race).
                }
            }

            delete m_pQueue;
            m_pQueue = 0;

            if( ! m_bAsynchron )
            {
                uno_releaseIdFromCurrentThread();
            }

            theThreadPool->waitInPool( this );
        }
    }
}
