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
