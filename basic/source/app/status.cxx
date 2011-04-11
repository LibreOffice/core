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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include "app.hxx"
#include "basic.hrc"
#include "appwin.hxx"
#include "status.hxx"

#include <vcl/decoview.hxx>

StatusLine::StatusLine( BasicFrame* p )
: TaskBar( p )
, pFrame( p )
{
    // initialize TaskToolBox
    TaskToolBox*    pTempTaskToolBox = GetTaskToolBox();
    pTempTaskToolBox->SetActivateTaskHdl( LINK( this, StatusLine, ActivateTask ) );

    // initialize TaskStatusBar
    TaskStatusBar*  pTempStatusBar = GetStatusBar();
    long nCharWidth = GetTextWidth( '0' );  // We state: All numbers has the same width
    pTempStatusBar->InsertItem( ST_MESSAGE, GetTextWidth( 'X' ) * 20, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    pTempStatusBar->InsertItem( ST_LINE, 5*nCharWidth );
    pTempStatusBar->InsertItem( ST_PROF, GetTextWidth( 'X' ) * 10 );
    pTempStatusBar->InsertStatusField();

    Show();
}

void StatusLine::Message( const String& s )
{
    GetStatusBar()->SetItemText( ST_MESSAGE, s );
}

void StatusLine::Pos( const String& s )
{
    GetStatusBar()->SetItemText( ST_LINE, s );
}

void StatusLine::SetProfileName( const String& s )
{
    GetStatusBar()->SetItemText( ST_PROF, s );
}


IMPL_LINK( StatusLine, ActivateTask, TaskToolBox*, pTTB )
{
    sal_uInt16 nFirstWinPos=0;
    MenuBar* pMenu = pFrame->GetMenuBar();
    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    while ( pWinMenu->GetItemId( nFirstWinPos ) < RID_WIN_FILE1 && nFirstWinPos < pWinMenu->GetItemCount() )
        nFirstWinPos++;

    nFirstWinPos += pTTB->GetItemPos( pTTB->GetCurItemId() ) / 2;

    AppWin* pWin = pFrame->FindWin( pWinMenu->GetItemText( pWinMenu->GetItemId( nFirstWinPos ) ).EraseAllChars( L'~' ) );
    if ( pWin )
    {
        pWin->Minimize( sal_False );
        pWin->ToTop();
    }
    return 0;
}

void StatusLine::LoadTaskToolBox()
{
    sal_uInt16 nFirstWinPos=0;
    MenuBar* pMenu = pFrame->GetMenuBar();
    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    while ( pWinMenu->GetItemId( nFirstWinPos ) < RID_WIN_FILE1 && nFirstWinPos < pWinMenu->GetItemCount() )
        nFirstWinPos++;

    TaskToolBox* pTaskToolBox = GetTaskToolBox();

    pTaskToolBox->StartUpdateTask();

    while ( nFirstWinPos < pWinMenu->GetItemCount() )
    {   // There are windows
        Window* pWin = pFrame->FindWin( pWinMenu->GetItemId( nFirstWinPos ) );

        if ( pWin )
            pTaskToolBox->UpdateTask( Image(), pWin->GetText(), pWin == pFrame->pList->back() && !( pFrame->pList->back()->GetWinState() & TT_WIN_STATE_HIDE ) );

        nFirstWinPos++;
    }

    pTaskToolBox->EndUpdateTask();
    Resize();
    Invalidate();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
