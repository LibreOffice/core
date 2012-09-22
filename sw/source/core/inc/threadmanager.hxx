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

        DECL_LINK( TryToStartNewThread, void* );

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
            return osl_atomic_increment( &mnThreadIDCounter );
        }

        bool StartWaitingThread();

        bool StartThread( const tThreadData& aThreadData );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
