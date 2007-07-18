/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swthreadmanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:32:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_sw.hxx"

#ifndef _SWTHREADMANAGER_HXX
#include <swthreadmanager.hxx>
#endif

#ifndef _SWTHREADJOINER_HXX
#include <swthreadjoiner.hxx>
#endif

#ifndef _OBSERVABLETHREAD_HXX
#include <observablethread.hxx>
#endif
#ifndef _THREADMANAGER_HXX
#include <threadmanager.hxx>
#endif

/** class to manage threads in Writer - it conforms the singleton pattern

    OD 2007-04-13 #i73788#

    @author OD
*/
SwThreadManager* SwThreadManager::mpThreadManager = 0;
osl::Mutex* SwThreadManager::mpGetManagerMutex = new osl::Mutex();

SwThreadManager::SwThreadManager()
    : mpThreadManagerImpl( new ThreadManager( SwThreadJoiner::GetThreadJoiner() ) )
{
    mpThreadManagerImpl->Init();
}

SwThreadManager::~SwThreadManager()
{
    delete mpThreadManagerImpl;
}

SwThreadManager& SwThreadManager::GetThreadManager()
{
    osl::MutexGuard aGuard(*mpGetManagerMutex);

    if ( mpThreadManager == 0 )
    {
        mpThreadManager = new SwThreadManager();
    }

    return *mpThreadManager;
}

bool SwThreadManager::ExistsThreadManager()
{
    return (mpThreadManager != 0);
}

oslInterlockedCount SwThreadManager::AddThread( const rtl::Reference< ObservableThread >& rThread )
{
    return mpThreadManagerImpl->AddThread( rThread );
}

void SwThreadManager::RemoveThread( const oslInterlockedCount nThreadID )
{
    mpThreadManagerImpl->RemoveThread( nThreadID );
}

void SwThreadManager::SuspendStartingOfThreads()
{
    mpThreadManagerImpl->SuspendStartingOfThreads();
}

void SwThreadManager::ResumeStartingOfThreads()
{
    mpThreadManagerImpl->ResumeStartingOfThreads();
}

bool SwThreadManager::StartingOfThreadsSuspended()
{
    return mpThreadManagerImpl->StartingOfThreadsSuspended();
}
