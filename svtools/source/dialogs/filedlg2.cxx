/*************************************************************************
 *
 *  $RCSfile: filedlg2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <svtdata.hxx>

#include <filedlg2.hxx>
#include <filedlg.hxx>
#include <filedlg2.hrc>

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif

#include <vcl/rcid.h>

#include <stdctrl.hxx>

#if defined (WNT) && defined (MSC)
#pragma optimize ("", off)
#endif

#include <helpid.hrc>


DECLARE_LIST( UniStringList, UniString* );

#define STD_BTN_WIDTH   80
#define STD_BTN_HEIGHT  26

#ifndef UNX
    #define ALLFILES                "*.*"
#else
    #define ALLFILES                "*"
#endif
//      #define STD_BTN_WIDTH   90
//      #define STD_BTN_HEIGHT  35

#define INITCONTROL( p, ControlClass, nBits, aPos, aSize, aTitel, nHelpId ) \
    p = new ControlClass( GetPathDialog(), WinBits( nBits ) ); \
    p->SetHelpId( nHelpId ); \
    p->SetPosSizePixel( aPos, aSize ); \
    p->SetText( aTitel ); \
    p->Show();


inline BOOL IsPrintable( sal_Unicode c )
{
    return c >= 32 && c != 127 ? TRUE : FALSE;
}

long
KbdListBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent aKeyEvt    = *rNEvt.GetKeyEvent();
        sal_Unicode  cCharCode  = aKeyEvt.GetCharCode();

        if ( IsPrintable ( cCharCode ) )
        {
            USHORT nCurrentPos = GetSelectEntryPos();
            USHORT nEntries    = GetEntryCount();

            for ( USHORT i = 1; i < nEntries; i++ )
            {
                UniString aEntry = GetEntry ( (i + nCurrentPos) % nEntries );
                aEntry.EraseLeadingChars( ' ' );
                aEntry.ToUpperAscii();
                UniString aCompare( cCharCode );
                aCompare.ToUpperAscii();

                if ( aEntry.CompareTo( aCompare, 1 ) == COMPARE_EQUAL )
                {
                    SelectEntryPos ( (i + nCurrentPos) % nEntries );
                    break;
                }
            }
        }
        else
        if ( aKeyEvt.GetKeyCode().GetCode() == KEY_RETURN )
        {
            DoubleClick();
        }
    }

    return ListBox::PreNotify ( rNEvt );
}

ImpPathDialog::ImpPathDialog( PathDialog* pDlg, WinBits nWinBits, RESOURCE_TYPE nType, BOOL bCreateDir )
{
    pSvPathDialog = pDlg;
    nDirCount = 0;

    // initialize Controls if not used as a base class
    if ( nType == WINDOW_PATHDIALOG )
    {
        InitControls();
        if( pNewDirBtn )
            pNewDirBtn->Enable( bCreateDir );
    }

    pDlg->SetHelpId( HID_FILEDLG_PATHDLG );
}

ImpPathDialog::~ImpPathDialog()
{
    delete pEdit;
    delete pDirTitel;
    delete pDirList;
    delete pDirPath;
    delete pDriveList;
    delete pDriveTitle;
    delete pLoadBtn;
    delete pOkBtn;
    delete pCancelBtn;
    delete pNewDirBtn;
#   ifdef UNX
    delete pHomeBtn;
#   endif
}

void ImpPathDialog::InitControls()
{
    PathDialog* pDlg = GetPathDialog();
    pDlg->SetText( UniString( SvtResId( STR_FILEDLG_SELECT ) ) );

    Size a3Siz = pDlg->LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
    Size a6Siz = pDlg->LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    Size aBtnSiz = pDlg->LogicToPixel( Size( 70, 14 ), MAP_APPFONT );
    Size aFTSiz = pDlg->LogicToPixel( Size( 142, 10 ), MAP_APPFONT );
    Size aEDSiz = pDlg->LogicToPixel( Size( 142, 12 ), MAP_APPFONT );
    Point aPnt( a6Siz.Width(), a6Siz.Height() );
    long nLbH1 = pDlg->LogicToPixel( Size( 0, 93 ), MAP_APPFONT ).Height();
    long nLbH2 = pDlg->LogicToPixel( Size( 0, 60 ), MAP_APPFONT ).Height();
    long nH = 0;
    UniString aEmptyStr;

    INITCONTROL( pDirTitel, FixedText, 0,
                 aPnt, aFTSiz, UniString( SvtResId( STR_FILEDLG_DIR ) ), HID_FILEDLG_DIR );
    aPnt.Y() += aFTSiz.Height() + a3Siz.Height();

    INITCONTROL( pEdit, Edit, WB_BORDER, aPnt, aEDSiz, aPath.GetFull(), HID_FILEDLG_EDIT );

    aPnt.Y() += aEDSiz.Height() + a3Siz.Height();
#ifndef UNX
    INITCONTROL( pDirList, KbdListBox, WB_AUTOHSCROLL | WB_BORDER,
        aPnt, Size( aEDSiz.Width(), nLbH1 ), aEmptyStr, HID_FILEDLG_DIRS );
    aPnt.Y() += nLbH1 + a6Siz.Height();
    INITCONTROL( pDriveTitle, FixedText, 0,
                 aPnt, aFTSiz, UniString( SvtResId( STR_FILEDLG_DRIVES ) ), HID_FILEDLG_DRIVE );
    aPnt.Y() += aFTSiz.Height() + a3Siz.Height();
    INITCONTROL( pDriveList, ListBox, WB_DROPDOWN,
                 aPnt, Size( aEDSiz.Width(), nLbH2 ), aEmptyStr, HID_FILEDLG_DRIVES );
    nH = aPnt.Y() + aEDSiz.Height() + a6Siz.Height();
#else
    long nNewH = nLbH1 + 3 * a3Siz.Height() +
                 aFTSiz.Height() + aEDSiz.Height();
    INITCONTROL( pDirList, KbdListBox, WB_AUTOHSCROLL | WB_BORDER,
                 aPnt, Size( aEDSiz.Width(), nNewH ), aEmptyStr, HID_FILEDLG_DIRS );
    nH = aPnt.Y() + nNewH + a6Siz.Height();
    pDriveTitle = NULL;
    pDriveList = NULL;
#endif

    aPnt.X() = 2 * a6Siz.Width() + aEDSiz.Width();
    aPnt.Y() = a6Siz.Height();
    INITCONTROL( pOkBtn, PushButton, WB_DEFBUTTON,
                 aPnt, aBtnSiz, Button::GetStandardText( BUTTON_OK ), 0 );
    aPnt.Y() += aBtnSiz.Height() + a3Siz.Height();
    INITCONTROL( pCancelBtn, CancelButton, 0,
                 aPnt, aBtnSiz, Button::GetStandardText( BUTTON_CANCEL ), 0 );
    aPnt.Y() += aBtnSiz.Height() + a3Siz.Height();
    INITCONTROL( pNewDirBtn, PushButton, WB_DEFBUTTON,
                 aPnt, aBtnSiz, UniString( SvtResId( STR_FILEDLG_NEWDIR ) ), HID_FILEDLG_NEWDIR );
#ifdef UNX
    aPnt.Y() += aBtnSiz.Height() + a3Siz.Height();
    INITCONTROL( pHomeBtn, PushButton, WB_DEFBUTTON,
                 aPnt, aBtnSiz, UniString( SvtResId( STR_FILEDLG_HOME ) ), HID_FILEDLG_HOME );
#else
    pHomeBtn = NULL;
#endif

    pDirPath = 0;
    pLoadBtn = 0;
    // Dialogbreite == OKBtn-Position + OKBtn-Breite + Rand
    long nW = aPnt.X() + aBtnSiz.Width() + a6Siz.Width();

    pDlg->SetOutputSizePixel( Size( nW, nH ) );  // Groesse ggf. auch Resource wird geplaettet?

    if (pDirList)
        pDirList->SetDoubleClickHdl(LINK( this, ImpPathDialog, DblClickHdl) );

    if (pDirList)
        pDirList->SetSelectHdl( LINK( this, ImpPathDialog, SelectHdl ) );

    if (pDriveList)
        pDriveList->SetSelectHdl( LINK( this, ImpPathDialog, SelectHdl ) );

    if (pOkBtn)
        pOkBtn->SetClickHdl( LINK( this, ImpPathDialog, ClickHdl) );

    if (pCancelBtn)
        pCancelBtn->SetClickHdl( LINK( this, ImpPathDialog, ClickHdl) );

    if (pHomeBtn)
        pHomeBtn->SetClickHdl( LINK( this, ImpPathDialog, ClickHdl) );

    if (pNewDirBtn)
        pNewDirBtn->SetClickHdl( LINK( this, ImpPathDialog, ClickHdl) );

    nOwnChilds = pDlg->GetChildCount();
}



IMPL_LINK( ImpPathDialog, SelectHdl, ListBox *, p )
{
    if( p == pDriveList )
    {
        UniString aDrive( pDriveList->GetSelectEntry(), 0, 2);
        aDrive += '\\';
        SetPath( aDrive );
    }
    else
    if( p == pDirList )
    {
        // isolate the pure name of the entry
        // removing trainling stuff and leading spaces
        UniString aEntry( pDirList->GetSelectEntry() );

        aEntry.EraseLeadingChars( ' ' );
        USHORT nPos = aEntry.Search( '/' );
        aEntry.Erase( nPos );

        // build the absolute path to the selected item
        DirEntry aNewPath;
        aNewPath.ToAbs();

        USHORT nCurPos = pDirList->GetSelectEntryPos();

        // Wird nach oben gewechselt
        if( nCurPos < nDirCount )
            aNewPath = aNewPath[nDirCount-nCurPos-1];
        else
            aNewPath += aEntry;

        pEdit->SetText( aNewPath.GetFull() );
    }

    return 0;
}


IMPL_LINK( ImpPathDialog, ClickHdl, Button*, pBtn )
{
    if ( pBtn == pOkBtn || pBtn == pLoadBtn )
    {
        DirEntry aFile( pEdit->GetText() );

        // Existiert File / File ueberschreiben
        if( IsFileOk( aFile ) )
        {
            // Ja, dann kompletten Pfad mit Filenamen merken und Dialog beenden
            aPath = aFile;
            aPath.ToAbs();
            GetPathDialog()->EndDialog( TRUE );
        }
        else
        {
            DirEntry aCheck( aPath );
            aCheck += aFile;
            if( aCheck.Exists() )
            {
                aCheck.ToAbs();
                SetPath( aCheck.GetFull() );
                pEdit->SetSelection( Selection( 0x7FFFFFFF, 0x7FFFFFFF ) );
            }
        }
    }
    else
    if ( pBtn == pCancelBtn )
    {
        GetPathDialog()->EndDialog( FALSE );
    }
    else
    if ( pBtn == pHomeBtn )
    {
        NAMESPACE_RTL( OUString )aHomeDir;
        NAMESPACE_VOS( OSecurity ) aSecurity;
        if ( aSecurity.getHomeDir( aHomeDir ) )
        {
            DirEntry aFile ( aHomeDir );
            if ( IsFileOk( aFile ) )
            {
                aFile.ToAbs();
                SetPath( aFile.GetFull() );
            }
        }
    }
    else
    if ( pBtn == pNewDirBtn )
    {
        DirEntry aFile( pEdit->GetText() );
        if( ! aFile.Exists() && ! FileStat( aFile ).IsKind( FSYS_KIND_WILD ) )
            aFile.MakeDir();

        if( IsFileOk ( aFile ) )
        {
            aFile.ToAbs();
            SetPath( aFile.GetFull() );
        }
    }

    return 0;
}


IMPL_LINK( ImpPathDialog, DblClickHdl, ListBox*, pBox )
{
    // isolate the pure name of the entry
    // removing trainling stuff and leading spaces
    UniString aEntry( pBox->GetSelectEntry() );

    aEntry.EraseLeadingChars( ' ' );
    USHORT nPos = aEntry.Search( '/' );
    aEntry.Erase( nPos );

    // build the absolute path to the selected item
    DirEntry aNewPath;
    aNewPath.ToAbs();
    if( pBox == pDirList )
    {
        USHORT nCurPos = pDirList->GetSelectEntryPos();

        // Wenn es schon das aktuelle ist, dann mache nichts
        if( nCurPos == nDirCount-1 )
            return 0;

        // Wird nach oben gewechselt
        if( nCurPos < nDirCount )
            aNewPath = aNewPath[nDirCount-nCurPos-1];
        else
            aNewPath += aEntry;
    }
    else
        aNewPath += aEntry;

    USHORT nCurPos = pBox->GetSelectEntryPos();

    pSvPathDialog->EnterWait();

    if( FileStat( aNewPath ).GetKind() & FSYS_KIND_DIR )
    {
        // Neuen Pfad setzen und Listboxen updaten
        aPath = aNewPath;
        if( !aPath.SetCWD( TRUE ) )
        {
            ErrorBox aBox( GetPathDialog(),
                           WB_OK_CANCEL | WB_DEF_OK,
                           UniString( SvtResId( STR_FILEDLG_CANTCHDIR ) ) );
            if( aBox.Execute() == RET_CANCEL )
                GetPathDialog()->EndDialog( FALSE );
        }
        UpdateEntries( TRUE );
    }

    pSvPathDialog->LeaveWait();
    return 0;
}

void ImpPathDialog::UpdateEntries( const BOOL dummy_bWithDirs )
{
    UniString aTabString;
    DirEntry aTmpPath;
    aTmpPath.ToAbs();

    nDirCount = aTmpPath.Level();

    pDirList->SetUpdateMode( FALSE );
    pDirList->Clear();

    for( USHORT i = nDirCount; i > 0; i-- )
    {
        UniString aName( aTabString );
        aName += aTmpPath[i-1].GetName();
        pDirList->InsertEntry( aName );
        aTabString.AppendAscii( "  ", 2 );
    }

    // scan the directory
    DirEntry aCurrent;
    aCurrent.ToAbs();

    Dir aDir( aCurrent, FSYS_KIND_DIR|FSYS_KIND_FILE );

    USHORT nEntries = aDir.Count();
    if( nEntries )
    {
        UniStringList aSortDirList;
        for ( USHORT n = 0; n < nEntries; n++ )
        {
            DirEntry& rEntry = aDir[n];
            UniString aName( rEntry.GetName() );
            if( aName.Len() && ( aName.GetChar(0) != '.' ) && rEntry.Exists() )
            {
                if( FileStat( rEntry ).GetKind() & FSYS_KIND_DIR )
                {
                    ULONG l;
                    for( l = 0; l < aSortDirList.Count(); l++ )
                        if( *aSortDirList.GetObject(l) > aName )
                            break;
                    aSortDirList.Insert( new UniString( aName ), l );
                }
            }
        }

        for( ULONG l = 0; l < aSortDirList.Count(); l++ )
        {
            UniString aEntryStr( aTabString );
            aEntryStr += *aSortDirList.GetObject(l);
            pDirList->InsertEntry( aEntryStr );
            delete aSortDirList.GetObject(l);
        }
    }

    UpdateDirs( aTmpPath );
}

void ImpPathDialog::UpdateDirs( const DirEntry& rTmpPath )
{
    pDirList->SelectEntryPos( nDirCount-1 );
    pDirList->SetTopEntry( nDirCount > 1
                           ? nDirCount - 2
                           : nDirCount - 1 );
    pDirList->SetUpdateMode( TRUE );
    pDirList->Invalidate();
    pDirList->Update();

    UniString aDirName = rTmpPath.GetFull();
    if( pDirPath )
        pDirPath->SetText( aDirName );
    else
        pEdit->SetText( aDirName );
}

BOOL ImpPathDialog::IsFileOk( const DirEntry& rDirEntry )
{
    if( FileStat( rDirEntry ).GetKind() & (FSYS_KIND_WILD | FSYS_KIND_DEV) )
        return FALSE;
    else
    {
        // Datei vorhanden ?
        if( ! rDirEntry.Exists() )
        {
            UniString aQueryTxt( SvtResId( STR_FILEDLG_ASKNEWDIR ) );
            aQueryTxt.SearchAndReplaceAscii( "%s", rDirEntry.GetFull() );
            QueryBox aQuery( GetPathDialog(),
                             WB_YES_NO | WB_DEF_YES,
                             aQueryTxt  );
            if( aQuery.Execute() == RET_YES )
                rDirEntry.MakeDir();
            else
                return FALSE;
        }
        if( !FileStat( rDirEntry ).IsKind( FSYS_KIND_DIR ) )
        {
            UniString aBoxText( SvtResId( STR_FILEDLG_CANTOPENDIR ) );
            aBoxText.AppendAscii( "\n[" );
            aBoxText += rDirEntry.GetFull();
            aBoxText.AppendAscii( "]" );
            InfoBox aBox( GetPathDialog(), aBoxText );
            aBox.Execute();
            return FALSE;
        }
    }
    return GetPathDialog()->OK() != 0;
}


void ImpPathDialog::PreExecute()
{
    // Neues Verzeichnis setzen und Listboxen updaten
    aPath.SetCWD( TRUE );
    UpdateEntries( TRUE );

    // Zusaetzliche Buttons anordnen
    Point   aPos;
    Size    aSize;
    long    nDY;
    if( pLoadBtn )
    {
        aPos  = pLoadBtn->GetPosPixel();
        aSize = pLoadBtn->GetSizePixel();
        nDY   = pLoadBtn->GetSizePixel().Height() * 2;
    }
    else
    {
        aPos  = pCancelBtn->GetPosPixel();
        aSize = pCancelBtn->GetSizePixel();
        nDY   = pCancelBtn->GetPosPixel().Y() - pOkBtn->GetPosPixel().Y();
    }

    // Standard-Controls anpassen
    long nMaxWidth = 0;

    // Maximale Breite ermitteln
    USHORT nChilds = GetPathDialog()->GetChildCount();
    USHORT n;
    for ( n = nOwnChilds; n < nChilds; n++ )
    {
        Window* pChild = GetPathDialog()->GetChild( n );
        pChild = pChild->GetWindow( WINDOW_CLIENT );
        if( pChild->GetType() != WINDOW_WINDOW )
        {
            long nWidth = pChild->GetTextWidth( pChild->GetText() ) + 12;
            if( nMaxWidth < nWidth )
                nMaxWidth = nWidth;
            nWidth = pChild->GetSizePixel().Width();
            if( nMaxWidth < nWidth )
                nMaxWidth = nWidth;
        }
    }

    if( nMaxWidth > aSize.Width() )
    {
        Size aDlgSize = GetPathDialog()->GetOutputSizePixel();
        GetPathDialog()->SetOutputSizePixel( Size( aDlgSize.Width()+nMaxWidth-aSize.Width(), aDlgSize.Height() ) );
        aSize.Width() = nMaxWidth;

        if( pOkBtn )
            pOkBtn->SetSizePixel( aSize );
        if( pCancelBtn )
            pCancelBtn->SetSizePixel( aSize );
        if( pLoadBtn )
            pLoadBtn->SetSizePixel( aSize );
    }
    else
        nMaxWidth = aSize.Width();

    for ( n = nOwnChilds; n < nChilds; n++ )
    {
        Window* pChild = GetPathDialog()->GetChild( n );
        pChild = pChild->GetWindow( WINDOW_CLIENT );
        if( pChild->GetType() != WINDOW_WINDOW )
        {
            aPos.Y() += nDY;
            pChild->SetPosSizePixel( aPos, aSize );
        }
        else
        {
            Size aDlgSize = GetPathDialog()->GetOutputSizePixel();
            long nExtra = Min( aDlgSize.Height(), (long)160);
            GetPathDialog()->SetOutputSizePixel( Size( aDlgSize.Width()+nExtra, aDlgSize.Height() ) );
            Size aSz( nExtra, nExtra );
            aSz.Width() -= 8;
            aSz.Height() -= 8;
            Point aCtrlPos( aDlgSize.Width() + 2, (aDlgSize.Height()-aSz.Height())/2 );
            pChild->SetPosSizePixel( aCtrlPos, aSz );
        }
    }

    // Laufwerke-LB fuellen
    if( pDriveList )
    {
        DirEntry aTmpDirEntry;
        Dir aDir( aTmpDirEntry, FSYS_KIND_BLOCK );

        USHORT nCount = aDir.Count(), i;
        for( i = 0; i < nCount; ++i )
        {
            DirEntry& rEntry = aDir[i];
            UniString aStr    = rEntry.GetFull( FSYS_STYLE_HOST, FALSE );

            UniString aVolume = rEntry.GetVolume() ;
            aStr.ToUpperAscii();
            if ( aVolume.Len() )
            {
                aStr += ' ';
                aStr += aVolume;
            }
            pDriveList->InsertEntry( aStr );

        }
        UniString aPathStr = aPath.GetFull();

        for ( i = 0; i < pDriveList->GetEntryCount(); ++i )
        {
            UniString aEntry = pDriveList->GetEntry(i);
            int    nLen   = aEntry.Len();
            nLen = nLen > 2 ? 2 : nLen;
            if ( aEntry.CompareIgnoreCaseToAscii( aPathStr, nLen ) == COMPARE_EQUAL )
            {
                pDriveList->SelectEntryPos(i);
                break;
            }
        }
    }
}

void ImpPathDialog::PostExecute()
{
}

void ImpPathDialog::SetPath( UniString const & rPath )
{
    aPath = DirEntry( rPath );

    pSvPathDialog->EnterWait();

    DirEntry aFile( rPath );
    // Falls der Pfad eine Wildcard oder einen Filenamen enthaelt
    // -> abschneiden und merken
    if( FileStat( aFile ).GetKind() & (FSYS_KIND_FILE | FSYS_KIND_WILD) || !aFile.Exists() )
        aFile.CutName();

    // Neue Maske und neues Verzeichnis setzen, und Listboxen updaten
    pEdit->SetText( rPath );
    aFile.SetCWD( TRUE );
    UpdateEntries( TRUE );

    pSvPathDialog->LeaveWait();
}

void ImpPathDialog::SetPath( Edit const & rEdit )
{
    UniString aPresetText = rEdit.GetText();
    if( aPresetText.Len() )
        SetPath( aPresetText );
}


UniString ImpPathDialog::GetPath() const
{
    DirEntry aFile( pEdit->GetText() );
    aFile.ToAbs();
    return aFile.GetFull();
}


ImpFileDialog::ImpFileDialog( PathDialog* pDlg, WinBits nWinBits, RESOURCE_TYPE nType ) :
  ImpPathDialog( pDlg, nWinBits, nType, FALSE )
{
    bOpen = (nWinBits & WB_SAVEAS) == 0;

    SvtResId aSvtResId = bOpen ? STR_FILEDLG_OPEN : STR_FILEDLG_SAVE;

    // Titel setzen
    GetFileDialog()->SetText( UniString( aSvtResId ) );
    nDirCount = 0;

    // initialize Controls if not used as a base class
    if ( nType == WINDOW_FILEDIALOG )
        InitControls();

    pDlg->SetHelpId( HID_FILEDLG_OPENDLG );

}

ImpFileDialog::~ImpFileDialog()
{
    ImpFilterItem* pItem = aFilterList.First();
    while( pItem )
    {
        delete pItem;
        pItem = aFilterList.Next();
    }

    delete pFileTitel;
    if (pFileList && ( pFileList != pDirList ) )
        delete pFileList;

    delete pTypeTitel;
    delete pTypeList;
}

void ImpFileDialog::InitControls()
{
    UniString aEmptyStr;

    const nW = 160;
    const nH = 48; // Um den Dialog in eine akzeptable Form zu bringen

    INITCONTROL( pFileTitel, FixedText, 0,
        Point(10, 12), Size(nW, 18), UniString( SvtResId( STR_FILEDLG_FILE ) ), HID_FILEDLG_FILE );
    INITCONTROL( pEdit, Edit, WB_BORDER,
        Point(10, 31), Size(nW, 20), aEmptyStr, HID_FILEDLG_EDIT ); // aMask()
    INITCONTROL( pFileList, ListBox, WB_SORT | WB_AUTOHSCROLL | WB_BORDER,
        Point(10, 58), Size(nW, 180-nH), aEmptyStr, HID_FILEDLG_FILES );

    INITCONTROL( pDirTitel, FixedText, 0,
        Point(nW+20, 12), Size(nW, 18), UniString( SvtResId( STR_FILEDLG_DIR ) ), HID_FILEDLG_DIR );
    INITCONTROL( pDirPath, FixedInfo, WB_PATHELLIPSIS,
        Point(nW+20, 33), Size(nW, 20), aPath.GetFull(), HID_FILEDLG_PATH );
    INITCONTROL( pDirList, KbdListBox, WB_AUTOHSCROLL | WB_BORDER,
        Point(nW+20, 58), Size(nW, 180-nH ), aEmptyStr, HID_FILEDLG_DIRS );

    INITCONTROL( pTypeTitel, FixedText, 0,
        Point(10, 246-nH), Size(nW, 18), UniString( SvtResId( STR_FILEDLG_TYPE ) ), HID_FILEDLG_TYPE );

#ifndef UNX
    INITCONTROL( pTypeList, ListBox, WB_DROPDOWN,
        Point(10, 265-nH ), Size(nW, 100 ), aEmptyStr, HID_FILEDLG_TYPES );

    INITCONTROL( pDriveTitle, FixedText, 0,
        Point(nW+20, 246-nH), Size(nW, 18), UniString( SvtResId( STR_FILEDLG_DRIVES ) ), HID_FILEDLG_DRIVE );
    INITCONTROL( pDriveList, ListBox, WB_DROPDOWN,
        Point(nW+20, 265-nH ), Size(nW, 100 ), aEmptyStr, HID_FILEDLG_DRIVES );
    pNewDirBtn = NULL;
    pHomeBtn   = NULL;
#else
    INITCONTROL( pTypeList, ListBox, WB_DROPDOWN,
        Point(10, 265-nH ), Size(2*nW+20, 100 ), aEmptyStr, HID_FILEDLG_TYPES );

    pDriveTitle = NULL;
    pDriveList = NULL;
    pNewDirBtn = NULL;
    pHomeBtn   = NULL;
#endif

    const long nButtonStartX = 2*nW+20+15;
    INITCONTROL( pOkBtn, PushButton, WB_DEFBUTTON,
        Point(nButtonStartX, 10), Size(STD_BTN_WIDTH, STD_BTN_HEIGHT),
        Button::GetStandardText( BUTTON_OK ), 0 );
    INITCONTROL( pCancelBtn, CancelButton, 0,
        Point(nButtonStartX, 45 ), Size(STD_BTN_WIDTH, STD_BTN_HEIGHT),
        Button::GetStandardText( BUTTON_CANCEL ), 0 );

    pLoadBtn = 0;

    GetFileDialog()->SetOutputSizePixel( Size(nButtonStartX+STD_BTN_WIDTH+10, 298-nH) );

    nOwnChilds = GetPathDialog()->GetChildCount();

    // Handler setzen
    if (pDriveList)
        pDriveList->SetSelectHdl( LINK( this, ImpFileDialog, SelectHdl ) );

    if (pDirList)
        pDirList->SetDoubleClickHdl(LINK( this, ImpFileDialog, DblClickHdl) );

    if (pOkBtn)
        pOkBtn->SetClickHdl( LINK( this, ImpFileDialog, ClickHdl) );

    if (pCancelBtn)
        pCancelBtn->SetClickHdl( LINK( this, ImpFileDialog, ClickHdl) );

    if( pFileList )
    {
        pFileList->SetSelectHdl( LINK( this, ImpFileDialog, SelectHdl ) );
        pFileList->SetDoubleClickHdl( LINK( this, ImpFileDialog, DblClickHdl ) );
    }

    if( pTypeList )
        pTypeList->SetSelectHdl( LINK( this, ImpFileDialog, DblClickHdl ) );
}

IMPL_LINK( ImpFileDialog, SelectHdl, ListBox *, p )
{
    if( p == pDriveList )
    {
        UniString aDrive ( pDriveList->GetSelectEntry(), 0, 2);
        aDrive += '\\';
        SetPath( aDrive );
    }
    else if (p == pFileList)
    {
        // Ausgewaehltes File in das Edit stellen
        pEdit->SetText( pFileList->GetSelectEntry() );
        GetFileDialog()->FileSelect();
    }
    return 0;
}


IMPL_LINK( ImpFileDialog, DblClickHdl, ListBox *, pBox )
{
  // isolate the pure name of the entry
  // removing trailing stuff and leading spaces
    UniString aEntry( pBox->GetSelectEntry() );

    aEntry.EraseLeadingChars( ' ' );
    USHORT nPos = aEntry.Search( '/' );
    aEntry.Erase( nPos );

    // build the absolute path to the selected item
    DirEntry aNewPath;
    aNewPath.ToAbs();

    if( ( pDirList != pFileList ) && ( pBox == pDirList ) )
    {
        // SVLOOK
        USHORT nCurPos = pDirList->GetSelectEntryPos();

        // Wenn es schon das aktuelle ist, dann mache nichts
        if( nCurPos == nDirCount-1 )
            return 0;

        // Wird nach oben gewechselt
        if( nCurPos < nDirCount )
            aNewPath = aNewPath[nDirCount-nCurPos-1];
        else
            aNewPath += aEntry;
    }
    else
    {
        // non-SVLOOK
        if( aEntry == UniString( SvtResId( STR_FILEDLG_GOUP ) ) )
            aEntry.AssignAscii( ".." );
        aNewPath += aEntry;
    }

    if( pBox == pFileList )
    {
        DirEntry aFile( aEntry );

        // Abfrage, ob File ueberschrieben werden soll...
        if( !FileStat(aFile).IsKind(FSYS_KIND_DIR) && IsFileOk( aFile ) )
        {
            // dann kompletten Pfad mit Filenamen merken und Dialog beenden
            aPath = aNewPath;
            GetFileDialog()->EndDialog( TRUE );
        }
    }

    USHORT nCurPos = pBox->GetSelectEntryPos();

    GetFileDialog()->EnterWait();

    UniString aFull = aNewPath.GetFull();

    if( ( ( pBox == pDirList ) && ( pDirList != pFileList ) ) ||
        ( ( pDirList == pFileList ) && FileStat( aNewPath ).GetKind() & FSYS_KIND_DIR ) )
    {
        // Neuen Pfad setzen und Listboxen updaten
        aPath = aNewPath;
        if( !aPath.SetCWD( TRUE ) )
        {
            if( ErrorBox( GetFileDialog(), WB_OK_CANCEL|WB_DEF_OK,
                                UniString( SvtResId( STR_FILEDLG_CANTCHDIR ) ) ).Execute() == RET_CANCEL )
            {
                GetFileDialog()->EndDialog( FALSE );
            }
        }
        UpdateEntries( TRUE );
        GetFileDialog()->FileSelect();
    }

    if( pBox == pTypeList )
    {
        // Neue Maske setzen, und Listboxen updaten
        USHORT nCurPos = pTypeList->GetSelectEntryPos();
        if( nCurPos+1 > (USHORT)aFilterList.Count() )
            aMask = UniString::CreateFromAscii( ALLFILES );
        else
        {
            UniString aFilterListMask = aFilterList.GetObject( nCurPos )->aMask;
//                      if( aFilterListMask.Search( ';' ) == STRING_NOTFOUND ) // kein ; in der Maske
//                              aMask = WildCard( aFilterListMask, '\0' );
//                      else // ; muss beruecksichtigt werden
                aMask = WildCard( aFilterListMask, ';' );
        }

        pEdit->SetText( aMask() );
        UpdateEntries( FALSE );
        GetFileDialog()->FilterSelect();
    }

  GetFileDialog()->LeaveWait();

  return 0;
}

IMPL_LINK( ImpFileDialog, ClickHdl, Button*, pBtn )
{
    if( ( pBtn == pOkBtn ) || ( pBtn == pLoadBtn ) )
    {
        DirEntry aFile( pEdit->GetText() );

        // Existiert File / File ueberschreiben
        if( IsFileOk( aFile ) )
        {
            // Ja, dann kompletten Pfad mit Filenamen merken und Dialog beenden
            aPath = aFile;
            aPath.ToAbs();
            GetFileDialog()->EndDialog( TRUE );
        }
        else
        {
            GetFileDialog()->EnterWait();

            // Falls der Pfad eine Wildcard oder einen Filenamen enthaelt
            // -> abschneiden und merken
            if( FileStat( aFile ).GetKind() & (FSYS_KIND_FILE | FSYS_KIND_WILD) || !aFile.Exists() )
            {
                aMask = aFile.CutName();
            }

            // Neue Maske und neues Verzeichnis setzen, und Listboxen updaten
            pEdit->SetText( aMask() );
            aFile.SetCWD( TRUE );
            UpdateEntries( TRUE );

            GetFileDialog()->LeaveWait();
        }
    }
    else if( pBtn == pCancelBtn )
        GetFileDialog()->EndDialog( FALSE );

    return 0;
}

void ImpFileDialog::UpdateEntries( const BOOL bWithDirs )
{
    GetFileDialog()->EnterWait();

    UniString aTabString;
    DirEntry aTmpPath;
    aTmpPath.ToAbs();
    nDirCount = aTmpPath.Level();

    if( pFileList )
    {
    pFileList->SetUpdateMode( FALSE );
        pFileList->Clear();
    }

    if( bWithDirs && (pDirList != pFileList) )
    {
        pDirList->SetUpdateMode( FALSE );
        pDirList->Clear();

        for( USHORT i = nDirCount; i > 0; i-- )
        {
            UniString aEntryStr( aTabString );
            aEntryStr += aTmpPath[i-1].GetName();
            pDirList->InsertEntry( aEntryStr );
            aTabString.AppendAscii( "  ", 2 );
        }
    }

    // for the combined box insert a '..'
    // (this happens only if WB_SVLOOK is not set)

    if( pDirList == pFileList && nDirCount != 1 )
        pFileList->InsertEntry( UniString( SvtResId( STR_FILEDLG_GOUP ) ) );

    // scan the directory
    DirEntry aCurrent;
    aCurrent.ToAbs();
    Dir aDir( aCurrent, FSYS_KIND_DIR|FSYS_KIND_FILE );
    USHORT nEntries = aDir.Count();

    // TempMask, weil Vergleich case-sensitiv
    BOOL bMatchCase = FALSE; //aCurrent.IsCaseSensitive();
    UniString aWildCard( aMask.GetWildCard() );
    if ( !bMatchCase )
        aWildCard.ToLowerAscii();
    WildCard aTmpMask( aWildCard, ';' );
    if ( nEntries )
    {
        UniStringList   aSortDirList;
    for ( USHORT n = 0; n < nEntries; n++ )
        {
            DirEntry& rEntry = aDir[n];
        UniString aName( rEntry.GetName() );

            if( aName.Len() &&
                ( ( ( aName.GetChar(0) != '.' ) ||
                  ( ( aName.GetChar(0) == '.' ) && ( aMask.GetWildCard() ).GetChar(0) == '.' ) )
                        && rEntry.Exists() ) )
            {
                FileStat aFileStat( rEntry );
                UniString aTmpName( aName );
                if ( !bMatchCase )
                    aTmpName.ToLowerAscii();
                if( ( aFileStat.GetKind() & FSYS_KIND_FILE ) && aTmpMask.Matches( aTmpName ) )
                {
                    ULONG n = aFileStat.GetKind();
                    if( pFileList )
            pFileList->InsertEntry( aName );
                }
        else if( bWithDirs && ( aFileStat.GetKind() & FSYS_KIND_DIR ) )
                {
                    if( pDirList == pFileList )
                    {
                        UniString aEntryStr( aName );
                        aEntryStr += '/';
                        pDirList->InsertEntry( aEntryStr );
                    }
                    else
                    {
            ULONG l;
            for( l = 0; l < aSortDirList.Count(); l++ )
                if( *aSortDirList.GetObject(l) > aName )
                                break;
            aSortDirList.Insert( new UniString( aName ), l );
            }
        }
        }
    }
    for( ULONG l = 0; l < aSortDirList.Count(); l++ )
        {
            UniString aEntryStr( aTabString );
            aEntryStr += *aSortDirList.GetObject(l);
        pDirList->InsertEntry( aEntryStr );
        delete aSortDirList.GetObject(l);
    }
    }

    if( bWithDirs )
        UpdateDirs( aTmpPath );

    if( pFileList )
    {
    if ( pDirList == pFileList && nDirCount > 1 )
        pFileList->SelectEntryPos( 1 );
    else
        pFileList->SetNoSelection();
    pFileList->SetUpdateMode( TRUE );
    pFileList->Invalidate();
    pFileList->Update();
    }

    if( pDriveList )
    {
        if( pDirList->GetEntryCount() > 0 )
        {
            UniString aStr( pDirList->GetEntry( 0 ) );
            aStr.Erase( 2 );
            aStr.ToLowerAscii();
            pDriveList->SelectEntry( aStr );
        }
    }

  GetFileDialog()->LeaveWait();
}

BOOL ImpFileDialog::IsFileOk( const DirEntry& rDirEntry )
{
    if( FileStat( rDirEntry ).GetKind() & (FSYS_KIND_WILD | FSYS_KIND_DEV) )
    return FALSE;
    if( FileStat( rDirEntry ).GetKind() & FSYS_KIND_DIR )
    {
        if( pFileList )
            return FALSE;
    }
    else if( bOpen )
    {
    // Datei vorhanden ?
    if( !FileStat( rDirEntry ).IsKind( FSYS_KIND_FILE ) )
        {
            UniString aErrorString( SvtResId( STR_FILEDLG_CANTOPENFILE ) );
            aErrorString.AppendAscii( "\n[" );
            aErrorString += rDirEntry.GetFull();
            aErrorString += ']';
            InfoBox aBox( GetFileDialog(),
                          aErrorString );
            aBox.Execute();
        return FALSE;
    }
    }
    else
    {
    // Datei vorhanden ?
        if( FileStat( ExtendFileName( rDirEntry ) ).IsKind( FSYS_KIND_FILE ) )
        {
            UniString aQueryString( SvtResId( STR_FILEDLG_OVERWRITE ) );
            aQueryString.AppendAscii( "\n[" );
            aQueryString += rDirEntry.GetFull();
            aQueryString += ']';
            QueryBox aBox( GetFileDialog(),
                           WinBits( WB_YES_NO | WB_DEF_NO ),
                           aQueryString );
            if( aBox.Execute() != RET_YES )
                return FALSE;
    }
    }
    return GetFileDialog()->OK() != 0;
}

void ImpFileDialog::SetPath( UniString const & rPath )
{
    aPath = DirEntry( rPath );
    GetFileDialog()->EnterWait();

    DirEntry aFile( rPath );

    // Falls der Pfad eine Wildcard oder einen Filenamen enthaelt
    // -> abschneiden und merken
    if( FileStat( aFile ).GetKind() & (FSYS_KIND_FILE | FSYS_KIND_WILD)     || !aFile.Exists() )
    {
        aMask = aFile.CutName();

        // Neue Maske und neues Verzeichnis setzen, und Listboxen updaten
        if( pDirList )
        {
            UniString aWildCard( aMask.GetWildCard() );
            pEdit->SetText( aWildCard );
        }
        else
            pEdit->SetText( rPath );
    }

    aFile.SetCWD( TRUE );

    UpdateEntries( TRUE );

    GetFileDialog()->LeaveWait();
}

void ImpFileDialog::SetPath( Edit const& rEdit )
{
    UniString aPresetText = rEdit.GetText();
    if( aPresetText.Len() )
        SetPath( aPresetText );
}


void ImpFileDialog::AddFilter( const UniString& rFilter, const UniString& rMask )
{
    aFilterList.Insert( new ImpFilterItem( rFilter, rMask ), LIST_APPEND );
    if( pTypeList )
        pTypeList->InsertEntry( rFilter, LISTBOX_APPEND );

    if( !GetCurFilter().Len() )
        SetCurFilter( rFilter );
}

void ImpFileDialog::RemoveFilter( const UniString& rFilter )
{
    ImpFilterItem* pItem = aFilterList.First();
    while( pItem && pItem->aName != rFilter )
        pItem = aFilterList.Next();

    if( pItem )
    {
        delete aFilterList.Remove();
        if( pTypeList )
            pTypeList->RemoveEntry( rFilter );
    }
}

void ImpFileDialog::RemoveAllFilter()
{
    ImpFilterItem* pItem = aFilterList.First();
    while( pItem )
    {
        delete pItem;
        pItem = aFilterList.Next();
    }
    aFilterList.Clear();

    if( pTypeList )
        pTypeList->Clear();
}

void ImpFileDialog::SetCurFilter( const UniString& rFilter )
{
    if( !pTypeList )
        return;

    ImpFilterItem* pItem = aFilterList.First();
    while( pItem && pItem->aName != rFilter )
        pItem = aFilterList.Next();

    if( pItem )
        pTypeList->SelectEntryPos( (USHORT)aFilterList.GetCurPos() );
    else
        pTypeList->SetNoSelection();
}

UniString ImpFileDialog::GetCurFilter() const
{
    UniString aFilter;
    if ( pTypeList )
        aFilter = pTypeList->GetSelectEntry();
    return aFilter;
}

void ImpFileDialog::PreExecute()
{
    // ListBoxen erst unmittelbar vor Execute fuellen
    // (damit vor Execute der Pfad umgesetzt werden kann, ohne das immer die
    //  Listboxen sofort upgedatet werden)

    GetFileDialog()->EnterWait();

    // Wenn kein Filter vorhanden, dann auch keine FilterBox
    if( pTypeList && !pTypeList->GetEntryCount() )
    {
        // pTypeList->InsertEntry( "* (all files)" );
        pTypeTitel->Disable();
        pTypeList->Disable();
    }

    if( pTypeList )
    {
        USHORT nCurType = pTypeList->GetSelectEntryPos();
        if( nCurType < aFilterList.Count() )
        {
            UniString aFilterListMask = aFilterList.GetObject( nCurType )->aMask;
            if( aFilterListMask.Search( ';' ) == STRING_NOTFOUND ) // kein ; in der Maske
                aMask = WildCard( aFilterListMask, '\0' );
            else // ; in der Maske, muss in der Wildcard beruecksichtigt werden
                aMask = WildCard( aFilterListMask, ';' );
        }
        else
            aMask = UniString::CreateFromAscii( ALLFILES );
    }
    else
        aMask = UniString::CreateFromAscii( ALLFILES );

    // Neue Maske setzen
    if( pEdit->GetText().Len() == 0 )
        pEdit->SetText( aMask() );

    ImpPathDialog::PreExecute();

    GetFileDialog()->LeaveWait();
}

UniString ImpFileDialog::GetPath() const
{
    DirEntry aFile( pEdit->GetText() );
    return ExtendFileName( aFile );
}

UniString ImpFileDialog::ExtendFileName( DirEntry aEntry ) const
{
    aEntry.ToAbs();
    // das ganze Theater hier ohnehin nur machen, wenn Dateiname
    // ohne Extension angegeben wurde
    if( !aEntry.GetExtension().Len() )
    {
        UniString aPostfix; // hier kommt die ausgesuchte Extension herein

        // ist ein Filter mit Extension gesetzt?
        USHORT nChosenFilterPos = pTypeList->GetSelectEntryPos();
        if( nChosenFilterPos != LISTBOX_ENTRY_NOTFOUND )
        {
            UniString aExtensionMask = GetFileDialog()->GetFilterType( nChosenFilterPos );
            // aExtension ist z.B. *.sdw, alles bis einschliesslich Punkt abschneiden
            UniString aExtension = aExtensionMask.Copy( aExtensionMask.Search( '.' )+1 );

            // hat der Filter ueberhaupt eine Extension
            if( aExtension.Len() )
            {
                // keine Wildcards enthalten?
                if( ( aExtension.Search( '*' ) == STRING_NOTFOUND ) &&
                    ( aExtension.Search( '?' ) == STRING_NOTFOUND ) )
                {
                    // OK, Filter hat Extension ohne Wildcards -> verwenden
                    aPostfix = aExtension;
                }
                else
                {
                    // Filter hat Extension mit Wildcards (z.B. *.*) -> nicht verwenden
                    aPostfix.Erase();
                }
            }
            else
            {
                // Filter hatte keine Extension (schwer vorstellbar) -> nichts anhaengen
                aPostfix.Erase();
            }
        }
        else
        {
            // kein Filter gefunden (merkwÆrdig) -> Default-Extension anhaengen
            aPostfix = GetFileDialog()->GetDefaultExt();
        }

        // jetzt kann es mit dem Anhaengen losgehen
        const sal_Unicode* pExt = aPostfix.GetBuffer();
        while( *pExt == '*' || *pExt == '?' )
            pExt++;

        if( *pExt )
        {
            UniString aName = aEntry.GetName();
            if( *pExt != '.' )
                aName += '.';
            aName += pExt;
            aEntry.SetName( aName );
        }
    }
    return aEntry.GetFull();
}


void ImpSvFileDlg::CreateDialog( PathDialog* pSvDlg, WinBits nStyle, RESOURCE_TYPE nType, BOOL bCreate )
{
    delete pDlg;
    if ( nType == WINDOW_PATHDIALOG )
        pDlg = new ImpPathDialog( pSvDlg, nStyle, nType, bCreate );
    else
        pDlg = new ImpFileDialog( pSvDlg, nStyle, nType );
}


