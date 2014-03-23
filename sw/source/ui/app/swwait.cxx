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
#include "precompiled_sw.hxx"

#include <swwait.hxx>
#include <docsh.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/window.hxx>


SwWait::SwWait(
    SwDocShell &rDocShell,
    const bool bLockUnlockDispatcher )
    : mrDoc ( rDocShell )
    , mbLockUnlockDispatcher( bLockUnlockDispatcher )
    , mpLockedDispatchers()
{
    EnterWaitAndLockDispatcher();
}

SwWait::~SwWait()
{
    LeaveWaitAndUnlockDispatcher();
}

void SwWait::EnterWaitAndLockDispatcher()
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( &mrDoc, sal_False );
    while ( pFrame )
    {
        pFrame->GetWindow().EnterWait();
        if ( mbLockUnlockDispatcher )
        {
            // do not look already locked dispatchers
            SfxDispatcher* pDispatcher = pFrame->GetDispatcher();
            if ( !pDispatcher->IsLocked() )
            {
                pDispatcher->Lock( sal_True );
                mpLockedDispatchers.insert( pDispatcher );
            }
        }

        pFrame = SfxViewFrame::GetNext( *pFrame, &mrDoc, sal_False );
    }
}

void SwWait::LeaveWaitAndUnlockDispatcher()
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( &mrDoc, sal_False );
    while ( pFrame )
    {
        pFrame->GetWindow().LeaveWait();
        if ( mbLockUnlockDispatcher )
        {
            // only unlock dispatchers which had been locked
            SfxDispatcher* pDispatcher = pFrame->GetDispatcher();
            if ( mpLockedDispatchers.find( pDispatcher ) != mpLockedDispatchers.end() )
            {
                mpLockedDispatchers.erase( pDispatcher );
                pDispatcher->Lock( sal_False );
            }
        }

        pFrame = SfxViewFrame::GetNext( *pFrame, &mrDoc, sal_False );
    }
}


