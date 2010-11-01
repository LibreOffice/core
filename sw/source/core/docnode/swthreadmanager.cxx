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
#include <swthreadmanager.hxx>
#include <swthreadjoiner.hxx>
#include <observablethread.hxx>
#include <threadmanager.hxx>

/** class to manage threads in Writer - it conforms the singleton pattern

    OD 2007-04-13 #i73788#

    @author OD
*/
bool SwThreadManager::mbThreadManagerInstantiated = false;

SwThreadManager::SwThreadManager()
    : mpThreadManagerImpl( new ThreadManager( SwThreadJoiner::GetThreadJoiner() ) )
{
    mpThreadManagerImpl->Init();
    mbThreadManagerInstantiated = true;
}

SwThreadManager::~SwThreadManager()
{
}

struct InitInstance : public rtl::Static<SwThreadManager, InitInstance> {};

SwThreadManager& SwThreadManager::GetThreadManager()
{
    return InitInstance::get();
}

bool SwThreadManager::ExistsThreadManager()
{
    return mbThreadManagerInstantiated;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
