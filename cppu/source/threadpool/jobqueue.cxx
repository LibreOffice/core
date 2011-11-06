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
#include "jobqueue.hxx"
#include "threadpool.hxx"

#include <osl/diagnose.h>

using namespace ::osl;

namespace cppu_threadpool {

    JobQueue::JobQueue() :
        m_nToDo( 0 ),
        m_bSuspended( sal_False ),
        m_cndWait( osl_createCondition() )
    {
        osl_resetCondition( m_cndWait );
        m_DisposedCallerAdmin = DisposedCallerAdmin::getInstance();
    }

    JobQueue::~JobQueue()
    {
        osl_destroyCondition( m_cndWait );
    }


    void JobQueue::add( void *pThreadSpecificData, RequestFun * doRequest )
    {
        MutexGuard guard( m_mutex );
        Job job = { pThreadSpecificData , doRequest };
        m_lstJob.push_back( job );
        if( ! m_bSuspended )
        {
            osl_setCondition( m_cndWait );
        }
        m_nToDo ++;
    }

    void *JobQueue::enter( sal_Int64 nDisposeId , sal_Bool bReturnWhenNoJob )
    {
        void *pReturn = 0;
        {
            // synchronize with the dispose calls
            MutexGuard guard( m_mutex );
            if( m_DisposedCallerAdmin->isDisposed( nDisposeId ) )
            {
                return 0;
            }
            m_lstCallstack.push_front( nDisposeId );
        }


        while( sal_True )
        {
            if( bReturnWhenNoJob )
            {
                MutexGuard guard( m_mutex );
                if( m_lstJob.empty() )
                {
                    break;
                }
            }

            osl_waitCondition( m_cndWait , 0 );

            struct Job job={0,0};
            {
                // synchronize with add and dispose calls
                MutexGuard guard( m_mutex );

                if( 0 == m_lstCallstack.front() )
                {
                    // disposed !
                    if( m_lstJob.empty() )
                    {
                        osl_resetCondition( m_cndWait );
                    }
                    break;
                }

                OSL_ASSERT( ! m_lstJob.empty() );
                if( ! m_lstJob.empty() )
                {
                    job = m_lstJob.front();
                    m_lstJob.pop_front();
                }
                if( m_lstJob.empty() )
                {
                    osl_resetCondition( m_cndWait );
                }
            }

            if( job.doRequest )
            {
                job.doRequest( job.pThreadSpecificData );
                m_nToDo --;
            }
            else
            {
                m_nToDo --;
                pReturn = job.pThreadSpecificData;
                break;
            }
        }

        {
            // synchronize with the dispose calls
            MutexGuard guard( m_mutex );
            m_lstCallstack.pop_front();
        }

        return pReturn;
    }

    void JobQueue::dispose( sal_Int64 nDisposeId )
    {
        MutexGuard guard( m_mutex );
        for( CallStackList::iterator ii = m_lstCallstack.begin() ;
             ii != m_lstCallstack.end() ;
             ++ii )
        {
            if( (*ii) == nDisposeId )
            {
                (*ii) = 0;
            }
        }

        if( !m_lstCallstack.empty()  && ! m_lstCallstack.front() )
        {
            // The thread is waiting for a disposed pCallerId, let it go
            osl_setCondition( m_cndWait );
        }
    }

    void JobQueue::suspend()
    {
        MutexGuard guard( m_mutex );
        m_bSuspended = sal_True;
    }

    void JobQueue::resume()
    {
        MutexGuard guard( m_mutex );
        m_bSuspended = sal_False;
        if( ! m_lstJob.empty() )
        {
            osl_setCondition( m_cndWait );
        }
    }

    sal_Bool JobQueue::isEmpty()
    {
        MutexGuard guard( m_mutex );
        return m_lstJob.empty();
    }

    sal_Bool JobQueue::isCallstackEmpty()
    {
        MutexGuard guard( m_mutex );
        return m_lstCallstack.empty();
    }

    sal_Bool JobQueue::isBusy()
    {
        return m_nToDo > 0;
    }


}
