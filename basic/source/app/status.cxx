/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: status.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:19:50 $
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
    USHORT nCharWidth = GetTextWidth( '0' );    // Angenommen, alle Zahlen sind gleich breit
    pTempStatusBar->InsertItem( ST_MESSAGE, GetTextWidth( 'X' ) * 20, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    pTempStatusBar->InsertItem( ST_LINE, 5*nCharWidth );
    pTempStatusBar->InsertStatusField();

    Show();
}

void StatusLine::Message( String& s )
{
    GetStatusBar()->SetItemText( ST_MESSAGE, s );
}

void StatusLine::Pos( String& s )
{
    GetStatusBar()->SetItemText( ST_LINE, s );
}

IMPL_LINK( StatusLine, ActivateTask, TaskToolBox*, pTTB )
{
    USHORT nFirstWinPos=0;
    MenuBar* pMenu = pFrame->GetMenuBar();
    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    while ( pWinMenu->GetItemId( nFirstWinPos ) < RID_WIN_FILE1 && nFirstWinPos < pWinMenu->GetItemCount() )
        nFirstWinPos++;

    nFirstWinPos += pTTB->GetItemPos( pTTB->GetCurItemId() ) / 2;

    USHORT x;
    x = pTTB->GetItemPos( pTTB->GetCurItemId() );
    x = pWinMenu->GetItemId( nFirstWinPos );
    x = pWinMenu->GetItemCount();
    AppWin* pWin = pFrame->FindWin( pWinMenu->GetItemText( pWinMenu->GetItemId( nFirstWinPos ) ).EraseAllChars( L'~' ) );
    if ( pWin )
    {
        pWin->Minimize( FALSE );
        pWin->ToTop();
    }
    return 0;
}

void StatusLine::LoadTaskToolBox()
{
    USHORT nFirstWinPos=0;
    MenuBar* pMenu = pFrame->GetMenuBar();
    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    while ( pWinMenu->GetItemId( nFirstWinPos ) < RID_WIN_FILE1 && nFirstWinPos < pWinMenu->GetItemCount() )
        nFirstWinPos++;

    TaskToolBox* pTaskToolBox = GetTaskToolBox();

    pTaskToolBox->StartUpdateTask();

    while ( nFirstWinPos < pWinMenu->GetItemCount() )
    {   // Es gibt Fenster
        Window* pWin = pFrame->FindWin( pWinMenu->GetItemId( nFirstWinPos ) );

        if ( pWin )
            pTaskToolBox->UpdateTask( Image(), pWin->GetText(), pWin == pFrame->pList->Last() && !( pFrame->pList->Last()->GetWinState() & TT_WIN_STATE_HIDE ) );

        nFirstWinPos++;
    }

    pTaskToolBox->EndUpdateTask();
    Resize();
    Invalidate();
}


