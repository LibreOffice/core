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


#include <hash_map>

#include <osl/conditn.h>

#include <rtl/byteseq.hxx>

#include <boost/shared_ptr.hpp>

#include "jobqueue.hxx"


using namespace ::rtl;
namespace cppu_threadpool {
    class ORequestThread;

    struct EqualThreadId
    {
        sal_Int32 operator () ( const ::rtl::ByteSequence &a , const ::rtl::ByteSequence &b ) const
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
                    return *(sal_Int32 *)a.getConstArray();
                }
                return 0;
            }
    };

    typedef ::std::hash_map
    <
        ByteSequence, // ThreadID
        ::std::pair < JobQueue * , JobQueue * >,
        HashThreadId,
        EqualThreadId
    > ThreadIdHashMap;

    typedef ::std::list < sal_Int64 > DisposedCallerList;


    struct WaitingThread
    {
        oslCondition condition;
        ORequestThread *thread;
    };

    typedef ::std::list < struct ::cppu_threadpool::WaitingThread * > WaitingThreadList;

    class DisposedCallerAdmin;
    typedef boost::shared_ptr<DisposedCallerAdmin> DisposedCallerAdminHolder;

    class DisposedCallerAdmin
    {
    public:
        ~DisposedCallerAdmin();

        static DisposedCallerAdminHolder getInstance();

        void dispose( sal_Int64 nDisposeId );
        void stopDisposing( sal_Int64 nDisposeId );
        sal_Bool isDisposed( sal_Int64 nDisposeId );

    private:
        ::osl::Mutex m_mutex;
        DisposedCallerList m_lst;
    };

    class ThreadPool;
    typedef boost::shared_ptr<ThreadPool> ThreadPoolHolder;

    class ThreadPool
    {
    public:
        ThreadPool();
        ~ThreadPool();
        static ThreadPoolHolder getInstance();

        void dispose( sal_Int64 nDisposeId );
        void stopDisposing( sal_Int64 nDisposeId );

        void addJob( const ByteSequence &aThreadId,
                     sal_Bool bAsynchron,
                     void *pThreadSpecificData,
                     RequestFun * doRequest );

        void prepare( const ByteSequence &aThreadId );
        void * enter( const ByteSequence &aThreadId, sal_Int64 nDisposeId );

        /********
         * @return true, if queue could be successfully revoked.
         ********/
        sal_Bool revokeQueue( const ByteSequence & aThreadId , sal_Bool bAsynchron );

        void waitInPool( ORequestThread *pThread );
    private:
        void createThread( JobQueue *pQueue, const ByteSequence &aThreadId, sal_Bool bAsynchron);


        ThreadIdHashMap m_mapQueue;
        ::osl::Mutex m_mutex;

        ::osl::Mutex m_mutexWaitingThreadList;
        WaitingThreadList m_lstThreads;

        DisposedCallerAdminHolder m_DisposedCallerAdmin;
    };

} // end namespace cppu_threadpool
