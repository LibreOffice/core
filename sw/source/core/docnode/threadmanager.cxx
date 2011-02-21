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
#include "precompiled_sw.hxx"
#include <threadmanager.hxx>

#include <algorithm>

using namespace ::com::sun::star;

/** class to manage threads

    #i73788#

    @author OD
*/
const std::deque< ThreadManager::tThreadData >::size_type ThreadManager::mnStartedSize = 10;

ThreadManager::ThreadManager( uno::Reference< util::XJobManager >& rThreadJoiner )
    : maMutex(),
      mrThreadJoiner( rThreadJoiner ),
      mpThreadListener(),
      mnThreadIDCounter( 0 ),
      maWaitingForStartThreads(),
      maStartedThreads(),
      maStartNewThreadTimer(),
      mbStartingOfThreadsSuspended( false )
{
}

void ThreadManager::Init()
{
    mpThreadListener.reset( new ThreadListener( *this ) );

    maStartNewThreadTimer.SetTimeout( 2000 );
    maStartNewThreadTimer.SetTimeoutHdl( LINK( this, ThreadManager, TryToStartNewThread ) );
}

ThreadManager::~ThreadManager()
{
    maWaitingForStartThreads.clear();
    maStartedThreads.clear();
}

boost::weak_ptr< IFinishedThreadListener > ThreadManager::GetThreadListenerWeakRef()
{
    return mpThreadListener;
}

void ThreadManager::NotifyAboutFinishedThread( const oslInterlockedCount nThreadID )
{
    RemoveThread( nThreadID, true );
}

oslInterlockedCount ThreadManager::AddThread(
                            const rtl::Reference< ObservableThread >& rThread )

{
    osl::MutexGuard aGuard(maMutex);

    // create new thread
    tThreadData aThreadData;
    oslInterlockedCount nNewThreadID( RetrieveNewThreadID() );
    {
        aThreadData.nThreadID = nNewThreadID;

        aThreadData.pThread = rThread;
        aThreadData.aJob = new CancellableJob( aThreadData.pThread );

        aThreadData.pThread->setPriority( osl_Thread_PriorityBelowNormal );
        mpThreadListener->ListenToThread( aThreadData.nThreadID,
                                          *(aThreadData.pThread) );
    }

    // add thread to manager
    if ( maStartedThreads.size() < mnStartedSize &&
         !StartingOfThreadsSuspended() )
    {
        // Try to start thread
        if ( !StartThread( aThreadData ) )
        {
            // No success on starting thread
            // If no more started threads exist, but still threads are waiting,
            // setup Timer to start thread from waiting ones
            if ( maStartedThreads.empty() && !maWaitingForStartThreads.empty() )
            {
                maStartNewThreadTimer.Start();
            }
        }
    }
    else
    {
        // Thread will be started later
        maWaitingForStartThreads.push_back( aThreadData );
    }

    return nNewThreadID;
}

void ThreadManager::RemoveThread( const oslInterlockedCount nThreadID,
                                  const bool bThreadFinished )
{
    // --> SAFE ----
    osl::MutexGuard aGuard(maMutex);

    std::deque< tThreadData >::iterator aIter =
                std::find_if( maStartedThreads.begin(), maStartedThreads.end(),
                              ThreadPred( nThreadID ) );

    if ( aIter != maStartedThreads.end() )
    {
        tThreadData aTmpThreadData( (*aIter) );

        maStartedThreads.erase( aIter );

        if ( bThreadFinished )
        {
            // release thread as job from thread joiner instance
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager > rThreadJoiner( mrThreadJoiner );
            if ( rThreadJoiner.is() )
            {
                rThreadJoiner->releaseJob( aTmpThreadData.aJob );
            }
            else
            {
                OSL_ENSURE( false, "<ThreadManager::RemoveThread(..)> - ThreadJoiner already gone!" );
            }
        }

        // Try to start thread from waiting ones
        TryToStartNewThread( 0 );
    }
    else
    {
        aIter = std::find_if( maWaitingForStartThreads.begin(),
                              maWaitingForStartThreads.end(), ThreadPred( nThreadID ) );

        if ( aIter != maWaitingForStartThreads.end() )
        {
            maWaitingForStartThreads.erase( aIter );
        }
    }
    // <-- SAFE ----
}

bool ThreadManager::StartWaitingThread()
{
    if ( !maWaitingForStartThreads.empty() )
    {
        tThreadData aThreadData( maWaitingForStartThreads.front() );
        maWaitingForStartThreads.pop_front();
        return StartThread( aThreadData );
    }
    else
    {
        return false;
    }
}

bool ThreadManager::StartThread( const tThreadData& rThreadData )
{
    bool bThreadStarted( false );

    if ( rThreadData.pThread->create() )
    {
        // start of thread successful.
        bThreadStarted = true;

        maStartedThreads.push_back( rThreadData );

        // register thread as job at thread joiner instance
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager > rThreadJoiner( mrThreadJoiner );
        if ( rThreadJoiner.is() )
        {
            rThreadJoiner->registerJob( rThreadData.aJob );
        }
        else
        {
            OSL_ENSURE( false, "<ThreadManager::StartThread(..)> - ThreadJoiner already gone!" );
        }
    }
    else
    {
        // thread couldn't be started.
        maWaitingForStartThreads.push_front( rThreadData );
    }

    return bThreadStarted;
}

IMPL_LINK( ThreadManager, TryToStartNewThread, Timer *, EMPTYARG )
{
    osl::MutexGuard aGuard(maMutex);

    if ( !StartingOfThreadsSuspended() )
    {
        // Try to start thread from waiting ones
        if ( !StartWaitingThread() )
        {
            // No success on starting thread
            // If no more started threads exist, but still threads are waiting,
            // setup Timer to start thread from waiting ones
            if ( maStartedThreads.empty() && !maWaitingForStartThreads.empty() )
            {
                maStartNewThreadTimer.Start();
            }
        }
    }

    return sal_True;
}

void ThreadManager::ResumeStartingOfThreads()
{
    osl::MutexGuard aGuard(maMutex);

    mbStartingOfThreadsSuspended = false;

    while ( maStartedThreads.size() < mnStartedSize &&
            !maWaitingForStartThreads.empty() )
    {
        if ( !StartWaitingThread() )
        {
            // No success on starting thread
            // If no more started threads exist, but still threads are waiting,
            // setup Timer to start thread from waiting ones
            if ( maStartedThreads.empty() && !maWaitingForStartThreads.empty() )
            {
                maStartNewThreadTimer.Start();
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
