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



#ifndef _CPPU_THREADPOOL_JOBQUEUE_HXX_
#define _CPPU_THREADPOOL_JOBQUEUE_HXX_

#include <list>
#include <sal/types.h>

#include <osl/conditn.h>
#include <osl/mutex.hxx>

#include <boost/shared_ptr.hpp>

namespace cppu_threadpool
{
    extern "C" typedef void (SAL_CALL RequestFun)(void *);

    struct Job
    {
        void *pThreadSpecificData;
        RequestFun * doRequest;
    };

    typedef ::std::list < struct Job > JobList;

    typedef ::std::list < sal_Int64 > CallStackList;

    class DisposedCallerAdmin;
    typedef boost::shared_ptr<DisposedCallerAdmin> DisposedCallerAdminHolder;

    class JobQueue
    {
    public:
        JobQueue();
        ~JobQueue();

        void add( void *pThreadSpecificData, RequestFun * doRequest );

        void *enter( sal_Int64 nDisposeId , sal_Bool bReturnWhenNoJob = sal_False );
        void dispose( sal_Int64 nDisposeId );

        void suspend();
        void resume();

        sal_Bool isEmpty();
        sal_Bool isCallstackEmpty();
        sal_Bool isBusy();

    private:
        ::osl::Mutex m_mutex;
        JobList      m_lstJob;
        CallStackList m_lstCallstack;
        sal_Int32 m_nToDo;
        sal_Bool m_bSuspended;
        oslCondition m_cndWait;
        DisposedCallerAdminHolder m_DisposedCallerAdmin;
    };
}

#endif
