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


#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/window.hxx>
#include <docsh.hxx>
#include <swwait.hxx>


void SwDocShell::EnterWait( sal_Bool bLockDispatcher )
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this, sal_False );
    while ( pFrame )
    {
        pFrame->GetWindow().EnterWait();
        if ( bLockDispatcher )
            pFrame->GetDispatcher()->Lock( sal_True );
        pFrame = SfxViewFrame::GetNext( *pFrame, this, sal_False );
    }
}

void SwDocShell::LeaveWait( sal_Bool bLockDispatcher )
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this, sal_False );
    while ( pFrame )
    {
        pFrame->GetWindow().LeaveWait();
        if ( bLockDispatcher )
            pFrame->GetDispatcher()->Lock( sal_False );
        pFrame = SfxViewFrame::GetNext( *pFrame, this, sal_False );
    }
}

SwWait::SwWait( SwDocShell &rDocShell, sal_Bool bLockDispatcher ) :
    rDoc ( rDocShell ),
    bLock( bLockDispatcher )
{
    rDoc.EnterWait( bLock );
}

SwWait::~SwWait()
{
    rDoc.LeaveWait( bLock );
}


