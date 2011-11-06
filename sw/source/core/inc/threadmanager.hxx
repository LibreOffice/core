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


#ifndef _THREADMANAGER_HXX
#define _THREADMANAGER_HXX

#include <ithreadlistenerowner.hxx>
#include <vcl/timer.hxx>
#include <osl/mutex.hxx>
#include <osl/interlck.h>
#include <rtl/ref.hxx>

#include <deque>
#include <list>
#include <cppuhelper/weak.hxx>
#include "com/sun/star/util/XJobManager.hpp"
#include <observablethread.hxx>
#include <cancellablejob.hxx>
#include <threadlistener.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

/** class to manage threads

    OD 2007-01-29 #i73788#
    An instance of this class takes care of the starting of threads.
    It assures that not more than <mnStartedSize> threads
    are started.

    @author OD
*/
class ThreadManager : public IThreadListenerOwner
{
    public:

        explicit ThreadManager( ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager >& rThreadJoiner );
        ~ThreadManager();

        // --> IThreadListenerOwner
        virtual boost::weak_ptr< IFinishedThreadListener > GetThreadListenerWeakRef();
        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID );
        // <--

        /** initialization

            IMPORTANT NOTE: Needs to be called directly after construction

            @author OD
        */
        void Init();

        /** add thread to the thread manager and taking ownership for the thread

            @author OD

            @return unique ID for added thread
        */
        oslInterlockedCount AddThread(
                            const ::rtl::Reference< ObservableThread >& rThread );

        void RemoveThread( const oslInterlockedCount nThreadID,
                           const bool bThreadFinished = false );

        DECL_LINK( TryToStartNewThread, Timer* );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.

            @author OD
        */
        inline void SuspendStartingOfThreads()
        {
            osl::MutexGuard aGuard(maMutex);

            mbStartingOfThreadsSuspended = true;
        }

        /** continues the starting of threads after it has been suspended

            @author OD
        */
        void ResumeStartingOfThreads();

        inline bool StartingOfThreadsSuspended()
        {
            osl::MutexGuard aGuard(maMutex);

            return mbStartingOfThreadsSuspended;
        }

        struct tThreadData
        {
            oslInterlockedCount nThreadID;
            ::rtl::Reference< ObservableThread > pThread;
            com::sun::star::uno::Reference< com::sun::star::util::XCancellable > aJob;

            tThreadData()
                : nThreadID( 0 ),
                  pThread( 0 ),
                  aJob()
            {}
        };

    private:

        static const std::deque< tThreadData >::size_type mnStartedSize;

        osl::Mutex maMutex;

        ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XJobManager > mrThreadJoiner;

        boost::shared_ptr< ThreadListener > mpThreadListener;

        oslInterlockedCount mnThreadIDCounter;

        std::deque< tThreadData > maWaitingForStartThreads;
        std::deque< tThreadData > maStartedThreads;

        Timer maStartNewThreadTimer;

        bool mbStartingOfThreadsSuspended;

        struct ThreadPred
        {
            oslInterlockedCount mnThreadID;
            explicit ThreadPred( oslInterlockedCount nThreadID )
                : mnThreadID( nThreadID )
            {}

            bool operator() ( const tThreadData& rThreadData ) const
            {
                return rThreadData.nThreadID == mnThreadID;
            }
        };


        inline oslInterlockedCount RetrieveNewThreadID()
        {
            return osl_incrementInterlockedCount( &mnThreadIDCounter );
        }

        bool StartWaitingThread();

        bool StartThread( const tThreadData& aThreadData );
};
#endif
