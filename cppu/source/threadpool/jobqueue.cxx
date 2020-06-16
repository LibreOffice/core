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

#include "jobqueue.hxx"
#include "threadpool.hxx"

namespace cppu_threadpool {

    JobQueue::JobQueue() :
        m_nToDo( 0 ),
        m_bSuspended( false ),
        m_DisposedCallerAdmin( DisposedCallerAdmin::getInstance() )
    {
    }

    void JobQueue::add( void *pThreadSpecificData, RequestFun * doRequest )
    {
        std::scoped_lock guard( m_mutex );
        Job job = { pThreadSpecificData , doRequest };
        m_lstJob.push_back( job );
        if( ! m_bSuspended )
        {
            m_cndWait.notify_all();
        }
        m_nToDo ++;
    }

    void *JobQueue::enter( void const * nDisposeId , bool bReturnWhenNoJob )
    {
        void *pReturn = nullptr;
        {
            // synchronize with the dispose calls
            std::scoped_lock guard( m_mutex );
            if( m_DisposedCallerAdmin->isDisposed( nDisposeId ) )
            {
                return nullptr;
            }
            m_lstCallstack.push_front( nDisposeId );
        }


        while( true )
        {
            struct Job job={nullptr,nullptr};
            {
                std::unique_lock guard( m_mutex );

                while (m_bSuspended
                       || (m_lstCallstack.front() != nullptr && !bReturnWhenNoJob
                           && m_lstJob.empty()))
                {
                    m_cndWait.wait(guard);
                }

                if( nullptr == m_lstCallstack.front() )
                {
                    // disposed !
                    if (!m_lstJob.empty() && m_lstJob.front().doRequest == nullptr) {
                        // If this thread was waiting for a remote response, that response may or
                        // may not have been enqueued; if it has not been enqueued, there cannot be
                        // another enqueued response, so it is always correct to remove any enqueued
                        // response here:
                        m_lstJob.pop_front();
                    }
                    break;
                }

                if( m_lstJob.empty() )
                {
                    assert(bReturnWhenNoJob);
                    break;
                }

                job = m_lstJob.front();
                m_lstJob.pop_front();
            }

            if( job.doRequest )
            {
                job.doRequest( job.pThreadSpecificData );
                std::scoped_lock guard( m_mutex );
                m_nToDo --;
            }
            else
            {
                pReturn = job.pThreadSpecificData;
                std::scoped_lock guard( m_mutex );
                m_nToDo --;
                break;
            }
        }

        {
            // synchronize with the dispose calls
            std::scoped_lock guard( m_mutex );
            m_lstCallstack.pop_front();
        }

        return pReturn;
    }

    void JobQueue::dispose( void const * nDisposeId )
    {
        std::scoped_lock guard( m_mutex );
        for( auto& rId : m_lstCallstack )
        {
            if( rId == nDisposeId )
            {
                rId = nullptr;
            }
        }

        if( !m_lstCallstack.empty()  && ! m_lstCallstack.front() )
        {
            // The thread is waiting for a disposed pCallerId, let it go
            m_cndWait.notify_all();
        }
    }

    void JobQueue::suspend()
    {
        std::scoped_lock guard( m_mutex );
        m_bSuspended = true;
    }

    void JobQueue::resume()
    {
        std::scoped_lock guard( m_mutex );
        m_bSuspended = false;
        if( ! m_lstJob.empty() )
        {
            m_cndWait.notify_all();
        }
    }

    bool JobQueue::isEmpty() const
    {
        std::scoped_lock guard( m_mutex );
        return m_lstJob.empty();
    }

    bool JobQueue::isCallstackEmpty() const
    {
        std::scoped_lock guard( m_mutex );
        return m_lstCallstack.empty();
    }

    bool JobQueue::isBusy() const
    {
        std::scoped_lock guard( m_mutex );
        return m_nToDo > 0;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
