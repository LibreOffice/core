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

#pragma once

#include <vector>
#include <unordered_map>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/byteseq.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include "jobqueue.hxx"


namespace cppu_threadpool {
    class ORequestThread;

    struct EqualThreadId
    {
        bool operator () ( const ::rtl::ByteSequence &a , const ::rtl::ByteSequence &b ) const
            {
                return a == b;
            }
    };

    struct HashThreadId
    {
        sal_Int32 operator () ( const ::rtl::ByteSequence &a  )  const
            {
                if( a.getLength() >= 4 )
                {
                    return *reinterpret_cast<sal_Int32 const *>(a.getConstArray());
                }
                return 0;
            }
    };

    typedef std::unordered_map
    <
        ::rtl::ByteSequence, // ThreadID
        std::pair < JobQueue * , JobQueue * >,
        HashThreadId,
        EqualThreadId
    > ThreadIdHashMap;

    struct WaitingThread
    {
        osl::Condition condition;
        rtl::Reference< ORequestThread > thread;

        explicit WaitingThread(
            rtl::Reference<ORequestThread> const & theThread);
    };

    typedef std::deque< struct ::cppu_threadpool::WaitingThread * > WaitingThreadDeque;

    class DisposedCallerAdmin;
    typedef std::shared_ptr<DisposedCallerAdmin> DisposedCallerAdminHolder;

    class DisposedCallerAdmin
    {
    public:
        ~DisposedCallerAdmin();

        static DisposedCallerAdminHolder const & getInstance();

        void dispose( void const * nDisposeId );
        void destroy( void const * nDisposeId );
        bool isDisposed( void const * nDisposeId );

    private:
        ::osl::Mutex m_mutex;
        std::vector< void const * > m_vector;
    };

    class ThreadAdmin
    {
    public:
        ThreadAdmin();
        ~ThreadAdmin ();

        bool add( rtl::Reference< ORequestThread > const & );
        void remove( rtl::Reference< ORequestThread > const & );
        void join();

        void remove_locked( rtl::Reference< ORequestThread > const & );
        ::osl::Mutex m_mutex;

    private:
        std::deque< rtl::Reference< ORequestThread > > m_deque;
        bool m_disposed;
    };

    class ThreadPool;
    typedef rtl::Reference<ThreadPool> ThreadPoolHolder;

    class ThreadPool: public salhelper::SimpleReferenceObject
    {
    public:
        ThreadPool();
        virtual ~ThreadPool() override;

        void dispose( void const * nDisposeId );
        void destroy( void const * nDisposeId );

        bool addJob( const ::rtl::ByteSequence &aThreadId,
                     bool bAsynchron,
                     void *pThreadSpecificData,
                     RequestFun * doRequest,
                     void const * disposeId );

        void prepare( const ::rtl::ByteSequence &aThreadId );
        void * enter( const ::rtl::ByteSequence &aThreadId, void const * nDisposeId );

        /********
         * @return true, if queue could be successfully revoked.
         ********/
        bool revokeQueue( const ::rtl::ByteSequence & aThreadId , bool bAsynchron );

        void waitInPool( rtl::Reference< ORequestThread > const & pThread );

        void joinWorkers();

        ThreadAdmin & getThreadAdmin() { return m_aThreadAdmin; }

    private:
        bool createThread( JobQueue *pQueue, const ::rtl::ByteSequence &aThreadId, bool bAsynchron);


        ThreadIdHashMap m_mapQueue;
        ::osl::Mutex m_mutex;

        ::osl::Mutex m_mutexWaitingThreadList;
        WaitingThreadDeque m_dequeThreads;

        DisposedCallerAdminHolder m_DisposedCallerAdmin;
        ThreadAdmin m_aThreadAdmin;
    };

} // end namespace cppu_threadpool

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
