/*************************************************************************
 *
 *  $RCSfile: appwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gh $ $Date: 2001-04-04 13:18:57 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#include <svtools/stringtransfer.hxx>
#endif

#include "basic.hrc"
#include "app.hxx"
#include "mybasic.hxx"
#include "status.hxx"
#include "appwin.hxx"
#include "dataedit.hxx"
#include "dialogs.hxx"

String *AppWin::pNoName = NULL;     // enthaelt den "Untitled"-String
short AppWin::nNumber = 0;          // fortlaufende Nummer
short AppWin::nCount = 0;           // Anzahl Editfenster

TYPEINIT0(AppWin);
AppWin::AppWin( BasicFrame* pParent )
: DockingWindow( pParent, WB_SIZEMOVE | WB_CLOSEABLE | WB_PINABLE )
, pDataEdit(NULL)
, nSkipReload(0)
, bHasFile( FALSE )
, bReloadAborted( FALSE )
, bFind( TRUE )
, pFrame( pParent )
{
    // evtl. den Untitled-String laden:
    if( !pNoName )
        pNoName = new String( ResId( IDS_NONAME ) );
    nCount++;

    // Maximized Status von aktuellem Fenster holen
    USHORT nInitialWinState;
    if ( pFrame->pWork )
    {
        nInitialWinState = pFrame->pWork->GetWinState();
        nInitialWinState &= TT_WIN_STATE_MAX | TT_WIN_STATE_FLOAT;
    }
    else
        nInitialWinState = TT_WIN_STATE_MAX;

    StartListening( *pFrame );
    pFrame->AddWindow( this );

    ShowTitleButton( TITLE_BUTTON_DOCKING );
    ShowTitleButton( TITLE_BUTTON_HIDE );
    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );

    Cascade( nCount );
    if ( TT_WIN_STATE_MAX == nInitialWinState )
        Maximize();
}

AppWin::~AppWin()
{
    nCount--;
    pFrame->RemoveWindow( this );
    pFrame = NULL;      // So daß nach BasicRun nicht mehr versucht wird Fenstertext zu setzen
}

void AppWin::SetText( const XubString& rStr )
{
    DockingWindow::SetText( rStr );
    pFrame->WindowRenamed( this );
}

void AppWin::TitleButtonClick( USHORT nButton )
{
    if ( TITLE_BUTTON_DOCKING == nButton )
        if ( TT_WIN_STATE_MAX != nWinState )
            Maximize();
        else
            Restore();
    else // if ( TITLE_BUTTON_HIDE == nButton )
        Minimize( TRUE );
}

void AppWin::Maximize()
{
    if ( TT_WIN_STATE_MAX != nWinState )
    {
        nNormalPos = GetPosPixel();
        nNormalSize = GetSizePixel();

        SetFloatingMode( FALSE );

        pFrame->nMaximizedWindows++;
        nWinState = TT_WIN_STATE_MAX;
    }
    long nTitleHeight;
    {
        long nDummy1, nDummy2, nDummy3;
        pFrame->GetBorder( nDummy1, nTitleHeight, nDummy2, nDummy3 );
    }

    Size aSize = pFrame->GetOutputSizePixel();
    aSize.Height() -= nTitleHeight;
    aSize.Height() -= 2;
    aSize.Width() -= 2;
    SetSizePixel( aSize );
    SetPosPixel( Point( 1,1 ) );
    pFrame->WinMax_Restore();
}

void AppWin::Restore()
{
    SetFloatingMode( TRUE );
    SetPosSizePixel( nNormalPos, nNormalSize );

    if ( TT_WIN_STATE_MAX == nWinState )
        pFrame->nMaximizedWindows--;

    nWinState = TT_WIN_STATE_FLOAT;
    pFrame->WinMax_Restore();
}

void AppWin::Minimize( BOOL bMinimize )
{
    if ( bMinimize )
        nWinState |= TT_WIN_STATE_HIDE;
    else
        nWinState &= ~TT_WIN_STATE_HIDE;
    pFrame->WinMax_Restore();
}

void AppWin::Cascade( USHORT nNr )
{
    Restore();

    nNr--;
    nNr %= 10;
    nNr++;

    long nTitleHeight;
    {
        long nDummy1, nDummy2, nDummy3;
        pFrame->GetBorder( nDummy1, nTitleHeight, nDummy2, nDummy3 );
    }

    Size aWinSize = pFrame->GetOutputSizePixel();
    aWinSize.Width() -= aWinSize.Width() / 5;       // auf 80 % reduzieren
    aWinSize.Height() -= nTitleHeight * nNr;        // Unten entsprechen kürzen
    aWinSize.Height() -= 2;

    Point nPos( nTitleHeight * nNr, nTitleHeight * nNr );

    SetPosSizePixel( nPos, aWinSize );
}

void AppWin::RequestHelp( const HelpEvent& )
{
    Help();
}

void AppWin::Help()
{
    String s = pDataEdit->GetSelected();
    if( s.Len() ) {
        // Leerstellen davor weg:
        while( s.GetChar(0) == ' ' ) s.Erase( 0, 1 );
//      aBasicApp.pHelp->Start( s );
    } else {
//      aBasicApp.pHelp->Start( HELP_INDEX );
    }
}

void AppWin::Resize()
{
    if( pDataEdit )
    {
        pDataEdit->SetPosPixel( Point( 0, 0 ) );
        pDataEdit->SetSizePixel( GetOutputSizePixel() );
    }
}

void AppWin::GetFocus()
{
    pFrame->FocusWindow( this );
    if( pDataEdit )     // Im Destruktor wird GetFocus gerufen, daher diese Abfrage
    {
        pDataEdit->GrabFocus();
//      InitMenu(GetpApp()->GetAppMenu()->GetPopupMenu( RID_APPEDIT )); // So daß Delete richtig ist
    }
}

long AppWin::PreNotify( NotifyEvent& rNEvt )
{

    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        Activate();
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        if ( pFrame->pList->Last() != this )
            Activate();
    return FALSE;       // Der event soll weiter verarbeitet werden
}

void AppWin::Activate()
{
    GrabFocus();
}

// Set up the menu
long AppWin::InitMenu( Menu* pMenu )
{

    ::rtl::OUString aTemp;
    TextSelection r = pDataEdit->GetSelection();
    BOOL bMarked = r.HasRange();
    pMenu->EnableItem( RID_EDITREPEAT,  (aFind.Len() != 0 ) );
    pMenu->EnableItem( RID_EDITCUT,     bMarked );
    pMenu->EnableItem( RID_EDITCOPY,    bMarked );
    pMenu->EnableItem( RID_EDITPASTE,   ( ::svt::OStringTransfer::PasteString( aTemp ) ) );
    pMenu->EnableItem( RID_EDITDEL,     bMarked );
//  pMenu->EnableItem( RID_HELPTOPIC,   bMarked );

    BOOL bHasText = pDataEdit->HasText();
    BOOL bRunning = pFrame->Basic().IsRunning();
    BOOL bCanExecute = BOOL( (!bRunning && bHasText) || pFrame->bInBreak );
    pMenu->EnableItem( RID_RUNSTART,    bCanExecute );
    pMenu->EnableItem( RID_RUNBREAK,    bRunning && !pFrame->bInBreak);
    pMenu->EnableItem( RID_RUNSTOP,     bRunning );
    pMenu->EnableItem( RID_RUNTOCURSOR, bCanExecute );
    pMenu->EnableItem( RID_RUNSTEPINTO, bCanExecute );
    pMenu->EnableItem( RID_RUNSTEPOVER, bCanExecute );
    return TRUE;
}

long AppWin::DeInitMenu( Menu* pMenu )
{
    pMenu->EnableItem( RID_EDITREPEAT );
    pMenu->EnableItem( RID_EDITCUT );
    pMenu->EnableItem( RID_EDITCOPY );
    pMenu->EnableItem( RID_EDITPASTE );
    pMenu->EnableItem( RID_EDITDEL );

    pMenu->EnableItem( RID_RUNSTART );
    pMenu->EnableItem( RID_RUNBREAK );
    pMenu->EnableItem( RID_RUNSTOP );
    pMenu->EnableItem( RID_RUNTOCURSOR );
    pMenu->EnableItem( RID_RUNSTEPINTO );
    pMenu->EnableItem( RID_RUNSTEPOVER );
    return TRUE;
}

// Menu Handler

void AppWin::Command( const CommandEvent& rCEvt )
{
    TextSelection r  = pDataEdit->GetSelection();
    BOOL bHasMark = r.HasRange();
    switch( rCEvt.GetCommand() ) {
        case RID_FILESAVE:
            QuerySave( QUERY_DISK_CHANGED | SAVE_NOT_DIRTY ); break;
        case RID_FILESAVEAS:
            SaveAs(); break;
        case RID_EDITSEARCH:
            Find(); break;
        case RID_EDITREPLACE:
            Replace(); break;
        case RID_EDITREPEAT:
            Repeat(); break;
        case RID_EDITCUT:
            if( bHasMark )
                pDataEdit->Cut();
            break;
        case RID_EDITCOPY:
            if( bHasMark )
                pDataEdit->Copy();
            break;
        case RID_EDITPASTE:
            {
                ::rtl::OUString aTemp;
                if( ::svt::OStringTransfer::PasteString( aTemp ) )
                    pDataEdit->Paste();
            }
            break;
        case RID_EDITDEL:
            /*if( bHasMark ) */pDataEdit->Delete();
            break;
        case RID_EDITUNDO:
            pDataEdit->Undo();
            break;
        case RID_EDITREDO:
            pDataEdit->Redo();
            break;
        case COMMAND_CONTEXTMENU:
            {
                PopupMenu *pKontext = NULL;
                pDataEdit->BuildKontextMenu( pKontext );
                if ( pKontext )
                {
                    USHORT nRes = pKontext->Execute( this, GetPointerPosPixel() );
                    if ( nRes )
                        pFrame->Command( nRes );
                    delete pKontext;
                }
            }
    }
}


BOOL AppWin::IsSkipReload()
{
    return nSkipReload != 0;
}

void AppWin::SkipReload( BOOL bSkip )
{
    DBG_ASSERT( bSkip || nSkipReload, "SkipReload aufgehoben ohne es zu aktivieren");
    if ( bSkip )
        nSkipReload++;
    else
        nSkipReload--;
}

BOOL AppWin::DiskFileChanged( USHORT nWhat )
{
    if ( !bHasFile )
        return FALSE;

    switch ( nWhat )
    {
        case SINCE_LAST_LOAD:
            {
                if ( bReloadAborted )
                    return TRUE;
                else
                    return DiskFileChanged( SINCE_LAST_ASK_RELOAD );
            }
            break;
        case SINCE_LAST_ASK_RELOAD:
            {
                String aFilename( GetText() );

                DirEntry aFile( aFilename );
                FileStat aStat( aFile );

                return ( !aLastAccess.GetError() != !aStat.GetError() )
                    || aLastAccess.IsYounger( aStat ) || aStat.IsYounger( aLastAccess );
            }
            break;
        default:
            DBG_ERROR("Not Implemented in AppWin::DiskFileChanged");
    }
    return TRUE;
}

void AppWin::UpdateFileInfo( USHORT nWhat )
{
    switch ( nWhat )
    {
        case HAS_BEEN_LOADED:
            {
                bReloadAborted = FALSE;
                UpdateFileInfo( ASKED_RELOAD );

            }
            break;
        case ASKED_RELOAD:
            {
                String aFilename( GetText() );

                DirEntry aFile( aFilename );
                aLastAccess.Update( aFile );
            }
            break;
        default:
            DBG_ERROR("Not Implemented in AppWin::UpdateFileInfo");
    }
}

void AppWin::CheckReload()
{
    if ( IsSkipReload() || !bHasFile )
        return;

    String aFilename( GetText() );
    DirEntry aFile( aFilename );
    if ( !aFilename.Len() )
        return;

    if ( !aFile.Exists() )
        return;

//  FileStat aStat( aFile );

    if ( DiskFileChanged( SINCE_LAST_ASK_RELOAD ) && ReloadAllowed() )
    {
        UpdateFileInfo( ASKED_RELOAD );
        ToTop();
        Update();
        if ( (IsModified() && QueryBox( this, ResId( IDS_ASKDIRTYRELOAD ) ).Execute() == RET_YES )
            || ( !IsModified() && ( pFrame->IsAutoReload() || QueryBox( this, ResId( IDS_ASKRELOAD ) ).Execute() == RET_YES ) ) )
        {
            Reload();
        }
        else
        {
            bReloadAborted = TRUE;
        }
    }
}

void AppWin::Reload()
{
    SkipReload();
    TextSelection aSelMemo = pDataEdit->GetSelection();
    Load( GetText() );
    pDataEdit->SetSelection( aSelMemo );
    SkipReload( FALSE );
}

// Datei laden

void AppWin::Load( const String& aName )
{
    SkipReload();
    BOOL bErr;

//  if( !QuerySave() )
//      return;
    bErr = !pDataEdit->Load( aName );
    if( bErr )
        ErrorBox( this, ResId( IDS_READERROR ) ).Execute();
    else
    {
        DirEntry aEntry( aName );
        String aModName = aEntry.GetFull();
        SetText( aModName );
        UpdateFileInfo( HAS_BEEN_LOADED );
        PostLoad();
        bHasFile = TRUE;
    }
    SkipReload( FALSE );
}

// Datei speichern

USHORT AppWin::ImplSave()
{
    SkipReload();
    USHORT nResult = SAVE_RES_NOT_SAVED;
    String s1 = *pNoName;
    String s2 = GetText().Copy( 0, s1.Len() );
    if( s1 == s2 )
        nResult = SaveAs();
    else {
        String aName = GetText();
        if ( pDataEdit->Save( aName ) )
        {
            nResult = SAVE_RES_SAVED;
            bHasFile = TRUE;
        }
        else
        {
            nResult = SAVE_RES_ERROR;
            ErrorBox( this, ResId( IDS_WRITEERROR ) ).Execute();
        }
        UpdateFileInfo( HAS_BEEN_LOADED );
    }
    SkipReload( FALSE );
    return nResult;
}

// mit neuem Namen speichern

USHORT AppWin::SaveAs()
{
    SkipReload();
    String s1 = *pNoName;
    String s2 = GetText().Copy( 0, s1.Len() );
    if( s1 == s2 ) s2.Erase();
    else s2 = GetText();
    if( pFrame->QueryFileName( s2, GetFileType(), TRUE ) )
    {
        SetText( s2 );
        PostSaveAs();
        SkipReload( FALSE );
        return ImplSave();
    }
    else
    {
        SkipReload( FALSE );
        return SAVE_RES_CANCEL;
    }
}

// Soll gespeichert werden?

USHORT AppWin::QuerySave( QueryBits nBits )
{
    BOOL bQueryDirty = ( nBits & QUERY_DIRTY ) != 0;
    BOOL bQueryDiskChanged = ( nBits & QUERY_DISK_CHANGED ) != 0;
    BOOL bSaveNotDirty = ( nBits & SAVE_NOT_DIRTY ) != 0;

    SkipReload();
    short nResult;
    if ( IsModified() || bSaveNotDirty )
        nResult = RET_YES;
    else
        nResult = RET_NO;

    if( ( ( IsModified() || bSaveNotDirty ) && bQueryDirty ) || ( DiskFileChanged( SINCE_LAST_LOAD ) && bQueryDiskChanged ) )
    {
        ToTop();
        if ( ( ( IsModified() && bQueryDirty ) && DiskFileChanged( SINCE_LAST_LOAD ) )
            || ( IsModified() && ( DiskFileChanged( SINCE_LAST_LOAD ) && bQueryDiskChanged ) ) )
            nResult = QueryBox( this, ResId( IDS_ASK_DIRTY_AND_DISKCHANGE_SAVE ) ).Execute();
        else if ( ( IsModified() && bQueryDirty ) )
            nResult = QueryBox( this, ResId( IDS_ASK_DIRTY_SAVE ) ).Execute();
        else
            nResult = QueryBox( this, ResId( IDS_ASK_DISKCHANGE_SAVE ) ).Execute();
    }


    USHORT nReturn;
    switch( nResult )
    {
        case RET_YES:
            nReturn = ImplSave();
            break;
        case RET_NO:
            nReturn = SAVE_RES_NOT_SAVED;
            break;
        case RET_CANCEL:
            nReturn = SAVE_RES_CANCEL;
            break;
    }
    SkipReload( FALSE );
    return nReturn;
}

BOOL AppWin::Close()
{
    switch ( QuerySave( QUERY_DIRTY ) )
    {
    case SAVE_RES_NOT_SAVED:
    case SAVE_RES_SAVED:
        {
            DockingWindow::Close();
            delete this;
            return TRUE;
        }
    break;
    case SAVE_RES_ERROR:    // Fehlermeldung schon ausgegeben
        return FALSE;
        break;
    case SAVE_RES_CANCEL:
        return FALSE;
        break;
    default:
        DBG_ERROR("Not Implemented in AppWin::Close")
        return FALSE;
    }
}

// Text suchen

void AppWin::Find()
{
    ResId aResId( IDD_FIND_DIALOG );
    FindDialog aDlg( this, aResId, aFind );
    if( aDlg.Execute() ) {
        bFind = TRUE;
        Repeat();
    }
}

// Text ersetzen

void AppWin::Replace()
{
    ResId aResId( IDD_REPLACE_DIALOG );
    ReplaceDialog* pDlg = new ReplaceDialog
                        (this, aResId, aFind, aReplace );
    if( pDlg->Execute() ) {
        bFind = FALSE;
        Repeat();
    }
}

// Suchen/ersetzen wiederholen

void AppWin::Repeat()
{
    if( (aFind.Len() != 0 ) && ( pDataEdit->Find( aFind ) || (ErrorBox(this,ResId(IDS_PATTERNNOTFOUND)).Execute() && FALSE) ) && !bFind )
      pDataEdit->ReplaceSelected( aReplace );
}

