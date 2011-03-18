/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    sal_Bool JobQueue::isBusy() const
    {
        return m_nToDo > 0;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
