/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <threadmanager.hxx>

#include <algorithm>

using namespace ::com::sun::star;

/** class to manage threads

    #i73788#
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

    
    if ( maStartedThreads.size() < mnStartedSize &&
         !StartingOfThreadsSuspended() )
    {
        
        if ( !StartThread( aThreadData ) )
        {
            
            
            
            if ( maStartedThreads.empty() && !maWaitingForStartThreads.empty() )
            {
                maStartNewThreadTimer.Start();
            }
        }
    }
    else
    {
        
        maWaitingForStartThreads.push_back( aThreadData );
    }

    return nNewThreadID;
}

void ThreadManager::RemoveThread( const oslInterlockedCount nThreadID,
                                  const bool bThreadFinished )
{
    
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
            
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager > rThreadJoiner( mrThreadJoiner );
            if ( rThreadJoiner.is() )
            {
                rThreadJoiner->releaseJob( aTmpThreadData.aJob );
            }
            else
            {
                OSL_FAIL( "<ThreadManager::RemoveThread(..)> - ThreadJoiner already gone!" );
            }
        }

        
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
        
        bThreadStarted = true;

        maStartedThreads.push_back( rThreadData );

        
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager > rThreadJoiner( mrThreadJoiner );
        if ( rThreadJoiner.is() )
        {
            rThreadJoiner->registerJob( rThreadData.aJob );
        }
        else
        {
            OSL_FAIL( "<ThreadManager::StartThread(..)> - ThreadJoiner already gone!" );
        }
    }
    else
    {
        
        maWaitingForStartThreads.push_front( rThreadData );
    }

    return bThreadStarted;
}

IMPL_LINK_NOARG(ThreadManager, TryToStartNewThread)
{
    osl::MutexGuard aGuard(maMutex);

    if ( !StartingOfThreadsSuspended() )
    {
        
        if ( !StartWaitingThread() )
        {
            
            
            
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
            
            
            
            if ( maStartedThreads.empty() && !maWaitingForStartThreads.empty() )
            {
                maStartNewThreadTimer.Start();
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
