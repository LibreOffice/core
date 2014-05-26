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
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( &mrDoc, false );
    while ( pFrame )
    {
        pFrame->GetWindow().EnterWait();
        if ( mbLockUnlockDispatcher )
        {
            // do not look already locked dispatchers
            SfxDispatcher* pDispatcher = pFrame->GetDispatcher();
            if ( !pDispatcher->IsLocked() )
            {
                pDispatcher->Lock( true );
                mpLockedDispatchers.insert( pDispatcher );
            }
        }

        pFrame = SfxViewFrame::GetNext( *pFrame, &mrDoc, false );
    }
}

void SwWait::LeaveWaitAndUnlockDispatcher()
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( &mrDoc, false );
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
                pDispatcher->Lock( false );
            }
        }

        pFrame = SfxViewFrame::GetNext( *pFrame, &mrDoc, false );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
