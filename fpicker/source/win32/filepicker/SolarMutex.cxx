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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <osl/thread.hxx>

int ReleaseSolarMutexOnMainThreadContext(unsigned nThreadId)
{
    int nAcquireCount = 0;
    vos::IMutex& rSolarMutex = Application::GetSolarMutex();
    vos::OThread::TThreadIdentifier nMainThreadId = Application::GetMainThreadIdentifier();

    if ( nMainThreadId == nThreadId )
    {
        ::vos::IMutex& rMutex = Application::GetSolarMutex();
        if ( rMutex.tryToAcquire() )
            nAcquireCount = Application::ReleaseSolarMutex() - 1;
    }

    return nAcquireCount;
}

void AcquireSolarMutex(int nAcquireCount)
{
    if ( nAcquireCount )
        Application::AcquireSolarMutex( nAcquireCount );
}
