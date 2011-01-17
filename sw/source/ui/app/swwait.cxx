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


