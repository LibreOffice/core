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

    sal_uInt16 x;
    x = pTTB->GetItemPos( pTTB->GetCurItemId() );
    x = pWinMenu->GetItemId( nFirstWinPos );
    x = pWinMenu->GetItemCount();
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
            pTaskToolBox->UpdateTask( Image(), pWin->GetText(), pWin == pFrame->pList->Last() && !( pFrame->pList->Last()->GetWinState() & TT_WIN_STATE_HIDE ) );

        nFirstWinPos++;
    }

    pTaskToolBox->EndUpdateTask();
    Resize();
    Invalidate();
}


