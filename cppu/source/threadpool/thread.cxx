/*************************************************************************
 *
 *  $RCSfile: thread.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2001-02-20 12:43:51 $
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
#include <uno/threadpool.h>

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
#ifdef DEBUG
        if( m_lst.size() )
        {
            fprintf( stderr, "%d Threads left\n" , m_lst.size() );
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

    ThreadAdmin* ThreadAdmin::getInstance()
    {
        static ThreadAdmin *pThreadAdmin = 0;
        if( ! pThreadAdmin )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pThreadAdmin )
            {
                static ThreadAdmin admin;
                pThreadAdmin = &admin;
            }
        }
        return pThreadAdmin;

    }

// ----------------------------------------------------------------------------------
    ORequestThread::ORequestThread( JobQueue *pQueue,
                                    const ByteSequence &aThreadId,
                                    sal_Bool bAsynchron )
        : m_pQueue( pQueue )
        , m_thread( 0 )
        , m_aThreadId( aThreadId )
        , m_bAsynchron( bAsynchron )
        , m_bDeleteSelf( sal_True )
    {
        ThreadAdmin::getInstance()->add( this );
    }


    ORequestThread::~ORequestThread()
    {
        if (m_thread != 0)
        {
            osl_freeThreadHandle(m_thread);
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

        if ( m_thread = osl_createSuspendedThread( cppu_requestThreadWorker, (void*)this))
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
        ThreadAdmin::getInstance()->remove( this );
        if( m_bDeleteSelf )
        {
            delete this;
        }
    }

    void ORequestThread::run()
    {
        while ( m_pQueue )
        {
            if( ! m_bAsynchron )
            {
                sal_Bool bReturn = uno_bindIdToCurrentThread( m_aThreadId.getHandle() );
                OSL_ASSERT( bReturn );
            }

            while( ! m_pQueue->isEmpty() )
            {
                // Note : Oneways should not get a disposable disposeid,
                //        It does not make sense to dispose a call in this state.
                //        That's way we put it an disposeid, that can't be used otherwise.
                m_pQueue->enter( (sal_Int64 ) this , sal_True );

                if( m_pQueue->isEmpty() )
                {
                    ThreadPool::getInstance()->revokeQueue( m_aThreadId , m_bAsynchron );
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

            cppu_threadpool::ThreadPool::getInstance()->waitInPool( this );
        }
    }
}
