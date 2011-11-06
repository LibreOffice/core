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


#ifndef _CPPU_THREADPOOL_THREAD_HXX
#define _CPPU_THREADPOOL_THREAD_HXX

#include <list>
#include <sal/types.h>

#include <osl/thread.h>

#include "jobqueue.hxx"

namespace cppu_threadpool {

    class JobQueue;
    class ThreadAdmin;
    typedef boost::shared_ptr<ThreadAdmin> ThreadAdminHolder;

    //-----------------------------------------
    // private thread class for the threadpool
    // independent from vos
    //-----------------------------------------
    class ORequestThread
    {
    public:
        ORequestThread( JobQueue * ,
                        const ::rtl::ByteSequence &aThreadId,
                        sal_Bool bAsynchron );
        ~ORequestThread();

        void setTask( JobQueue * , const ::rtl::ByteSequence & aThreadId , sal_Bool bAsynchron );

        sal_Bool create();
        void join();
        void onTerminated();
        void run();
        inline void setDeleteSelf( sal_Bool b )
            { m_bDeleteSelf = b; }

    private:
        oslThread m_thread;
        ThreadAdminHolder m_aThreadAdmin;
        JobQueue *m_pQueue;
        ::rtl::ByteSequence m_aThreadId;
        sal_Bool m_bAsynchron;
        sal_Bool m_bDeleteSelf;
    };

    class ThreadAdmin
    {
    public:
        ~ThreadAdmin ();
        static ThreadAdminHolder &getInstance();
        void add( ORequestThread * );
        void remove( ORequestThread * );
        void join();

    private:
        ::osl::Mutex m_mutex;
        ::std::list< ORequestThread * > m_lst;
    };

} // end cppu_threadpool


#endif

