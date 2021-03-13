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

#include <algorithm>
#include <cstdlib>
#include <osl/diagnose.h>
#include <uno/threadpool.h>
#include <sal/log.hxx>

#include "thread.hxx"
#include "jobqueue.hxx"
#include "threadpool.hxx"

using namespace osl;
using namespace rtl;

namespace cppu_threadpool {


    ThreadAdmin::ThreadAdmin(): m_disposed(false) {}

    ThreadAdmin::~ThreadAdmin()
    {
        SAL_WARN_IF(m_deque.size(), "cppu.threadpool", m_deque.size() << "Threads left");
    }

    bool ThreadAdmin::add_locked( rtl::Reference< ORequestThread > const & p )
    {
        if( m_disposed )
        {
            return false;
        }
        m_deque.push_back( p );
        return true;
    }

    void ThreadAdmin::remove_locked( rtl::Reference< ORequestThread > const & p )
    {
        m_deque.erase(std::find( m_deque.begin(), m_deque.end(), p ), m_deque.end());
    }

    void ThreadAdmin::remove( rtl::Reference< ORequestThread > const & p )
    {
        std::scoped_lock aGuard( m_mutex );
        remove_locked( p );
    }

    void ThreadAdmin::join()
    {
        {
            std::scoped_lock aGuard( m_mutex );
            m_disposed = true;
        }
        for (;;)
        {
            rtl::Reference< ORequestThread > pCurrent;
            {
                std::scoped_lock aGuard( m_mutex );
                if( m_deque.empty() )
                {
                    break;
                }
                pCurrent = m_deque.front();
                m_deque.pop_front();
            }
            if (pCurrent->getIdentifier()
                != osl::Thread::getCurrentIdentifier())
            {
                pCurrent->join();
            }
        }
    }


    ORequestThread::ORequestThread( ThreadPoolHolder const &aThreadPool,
                                    JobQueue *pQueue,
                                    const ByteSequence &aThreadId,
                                    bool bAsynchron )
        : m_aThreadPool( aThreadPool )
        , m_pQueue( pQueue )
        , m_aThreadId( aThreadId )
        , m_bAsynchron( bAsynchron )
    {}

    ORequestThread::~ORequestThread() {}

    void ORequestThread::setTask( JobQueue *pQueue,
                                  const ByteSequence &aThreadId,
                                  bool bAsynchron )
    {
        m_pQueue = pQueue;
        m_aThreadId = aThreadId;
        m_bAsynchron = bAsynchron;
    }

    bool ORequestThread::launch()
    {
        // Assumption is that osl::Thread::create returns normally with a true
        // return value iff it causes osl::Thread::run to start executing:
        acquire();
        ThreadAdmin & rThreadAdmin = m_aThreadPool->getThreadAdmin();
        std::unique_lock g(rThreadAdmin.m_mutex);
        if (!rThreadAdmin.add_locked( this )) {
            return false;
        }
        try {
            if (!create()) {
                std::abort();
            }
        } catch (...) {
            rThreadAdmin.remove_locked( this );
            g.release();
            release();
            throw;
        }
        return true;
    }

    void ORequestThread::onTerminated()
    {
        m_aThreadPool->getThreadAdmin().remove( this );
        release();
    }

    void ORequestThread::run()
    {
        osl_setThreadName("cppu_threadpool::ORequestThread");

        try
        {
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
                    //        That's way we put it a disposeid, that can't be used otherwise.
                    m_pQueue->enter(
                        this,
                        true );

                    if( m_pQueue->isEmpty() )
                    {
                        m_aThreadPool->revokeQueue( m_aThreadId , m_bAsynchron );
                        // Note : revokeQueue might have failed because m_pQueue.isEmpty()
                        //        may be false (race).
                    }
                }

                delete m_pQueue;
                m_pQueue = nullptr;

                if( ! m_bAsynchron )
                {
                    uno_releaseIdFromCurrentThread();
                }

                m_aThreadPool->waitInPool( this );
            }
        }
        catch (...)
        {
            // Work around the problem that onTerminated is not called if run
            // throws an exception:
            onTerminated();
            throw;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
