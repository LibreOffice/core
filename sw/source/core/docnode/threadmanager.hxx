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

#include <vcl/idle.hxx>
#include <osl/mutex.hxx>
#include <osl/interlck.h>
#include <rtl/ref.hxx>

#include <deque>
#include <cppuhelper/weakref.hxx>
#include <observablethread.hxx>

#include <memory>

namespace com::sun::star::util { class XCancellable; }
namespace com::sun::star::util { class XJobManager; }

class IFinishedThreadListener;
class ThreadListener;
class Timer;

/** class to manage threads

    OD 2007-01-29 #i73788#
    An instance of this class takes care of the starting of threads.
    It assures that not more than <mnStartedSize> threads
    are started.
*/
class ThreadManager final
{
    public:

        explicit ThreadManager( css::uno::Reference< css::util::XJobManager > const & rThreadJoiner );
        ~ThreadManager();

        std::weak_ptr< IFinishedThreadListener > GetThreadListenerWeakRef() const;
        void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID );

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

        DECL_LINK( TryToStartNewThread, Timer*, void );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.
        */
        void SuspendStartingOfThreads()
        {
            osl::MutexGuard aGuard(maMutex);

            mbStartingOfThreadsSuspended = true;
        }

        /** continues the starting of threads after it has been suspended
        */
        void ResumeStartingOfThreads();

        bool StartingOfThreadsSuspended()
        {
            osl::MutexGuard aGuard(maMutex);

            return mbStartingOfThreadsSuspended;
        }

        struct tThreadData
        {
            oslInterlockedCount nThreadID;
            ::rtl::Reference< ObservableThread > pThread;
            css::uno::Reference< css::util::XCancellable > aJob;

            tThreadData()
                : nThreadID( 0 ),
                  aJob()
            {}
        };

    private:

        static const std::deque< tThreadData >::size_type snStartedSize;

        osl::Mutex maMutex;

        css::uno::WeakReference< css::util::XJobManager > mrThreadJoiner;

        std::shared_ptr< ThreadListener > mpThreadListener;

        oslInterlockedCount mnThreadIDCounter;

        std::deque< tThreadData > maWaitingForStartThreads;
        std::deque< tThreadData > maStartedThreads;

        Idle maStartNewThreadIdle;

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

        bool StartWaitingThread();

        bool StartThread( const tThreadData& aThreadData );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
