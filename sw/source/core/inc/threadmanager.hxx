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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_THREADMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_THREADMANAGER_HXX

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
*/
class ThreadManager : public IThreadListenerOwner
{
    public:

        explicit ThreadManager( ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager >& rThreadJoiner );
        virtual ~ThreadManager();

        // --> IThreadListenerOwner
        virtual boost::weak_ptr< IFinishedThreadListener > GetThreadListenerWeakRef() SAL_OVERRIDE;
        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) SAL_OVERRIDE;

        /** initialization

            IMPORTANT NOTE: Needs to be called directly after construction
        */
        void Init();

        /** add thread to the thread manager and taking ownership for the thread

            @return unique ID for added thread
        */
        oslInterlockedCount AddThread(
                            const ::rtl::Reference< ObservableThread >& rThread );

        void RemoveThread( const oslInterlockedCount nThreadID,
                           const bool bThreadFinished = false );

        DECL_LINK( TryToStartNewThread, void* );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.
        */
        inline void SuspendStartingOfThreads()
        {
            osl::MutexGuard aGuard(maMutex);

            mbStartingOfThreadsSuspended = true;
        }

        /** continues the starting of threads after it has been suspended
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
            return osl_atomic_increment( &mnThreadIDCounter );
        }

        bool StartWaitingThread();

        bool StartThread( const tThreadData& aThreadData );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
