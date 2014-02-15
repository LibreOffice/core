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

#include "jobqueue.hxx"
#include "threadpool.hxx"

#include <osl/diagnose.h>

using namespace ::osl;

namespace cppu_threadpool {

    JobQueue::JobQueue() :
        m_nToDo( 0 ),
        m_bSuspended( false ),
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

    void *JobQueue::enter( sal_Int64 nDisposeId , bool bReturnWhenNoJob )
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


        while( true )
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
                    if( m_lstJob.empty()
                        && (m_lstCallstack.empty()
                            || m_lstCallstack.front() != 0) )
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
                if( m_lstJob.empty()
                    && (m_lstCallstack.empty() || m_lstCallstack.front() != 0) )
                {
                    osl_resetCondition( m_cndWait );
                }
            }

            if( job.doRequest )
            {
                job.doRequest( job.pThreadSpecificData );
                MutexGuard guard( m_mutex );
                m_nToDo --;
            }
            else
            {
                pReturn = job.pThreadSpecificData;
                MutexGuard guard( m_mutex );
                m_nToDo --;
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
        m_bSuspended = true;
    }

    void JobQueue::resume()
    {
        MutexGuard guard( m_mutex );
        m_bSuspended = false;
        if( ! m_lstJob.empty() )
        {
            osl_setCondition( m_cndWait );
        }
    }

    bool JobQueue::isEmpty() const
    {
        MutexGuard guard( m_mutex );
        return m_lstJob.empty();
    }

    bool JobQueue::isCallstackEmpty() const
    {
        MutexGuard guard( m_mutex );
        return m_lstCallstack.empty();
    }

    bool JobQueue::isBusy() const
    {
        MutexGuard guard( m_mutex );
        return m_nToDo > 0;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
