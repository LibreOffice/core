/*************************************************************************
 *
 *  $RCSfile: status.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "app.hxx"
#include "basic.hrc"
#include "appwin.hxx"
#include "status.hxx"

#include <vcl/system.hxx>

#include <vcl/decoview.hxx>

StatusLine::StatusLine( BasicFrame* p )
: TaskBar( p )
, pFrame( p )
{
    Show();
}

TaskToolBox* StatusLine::CreateTaskToolBox()
{
    TaskToolBox *pTTB = new TaskToolBox( this );
    pTTB->SetActivateTaskHdl( LINK( this, StatusLine, ActivateTask ) );
//  pTTB->Show();
    return pTTB;
}

TaskStatusBar* StatusLine::CreateTaskStatusBar()
{
    TaskStatusBar* pBar = new TaskStatusBar( this, WB_3DLOOK | WB_BORDER | WB_LEFT );
    USHORT nCharWidth = GetTextWidth( '0' );    // Angenommen, alle Zahlen sind gleich breit

    pBar->InsertItem( ST_MESSAGE, GetTextWidth( 'X' ) * 20, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    pBar->InsertItem( ST_LINE, 5*nCharWidth );
    pBar->InsertStatusField();
    return pBar;
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
    AppWin* pWin = pFrame->FindWin( pWinMenu->GetItemText( pWinMenu->GetItemId( nFirstWinPos ) ) );
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
}


