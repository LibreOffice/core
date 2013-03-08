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

#include <vcl/svapp.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svtresid.hxx>
#include <filedlg2.hxx>
#include <svtools/filedlg.hxx>
#include <svtools/filedlg2.hrc>
#include <vcl/msgbox.hxx>
#include <osl/security.hxx>
#include <comphelper/string.hxx>

#include <svtools/stdctrl.hxx>

#include <svtools/helpid.hrc>

using namespace com::sun::star;
using namespace com::sun::star::uno;

typedef ::std::vector< OUString > OUStringList;

#define INITCONTROL( p, ControlClass, nBits, aPos, aSize, aTitel, rHelpId ) \
    p = new ControlClass( GetPathDialog(), WinBits( nBits ) ); \
    p->SetHelpId( rHelpId ); \
    p->SetPosSizePixel( aPos, aSize ); \
    p->SetText( aTitel ); \
    p->Show();


inline sal_Bool IsPrintable( sal_Unicode c )
{
    return c >= 32 && c != 127 ? sal_True : sal_False;
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
            sal_uInt16 nCurrentPos = GetSelectEntryPos();
            sal_uInt16 nEntries    = GetEntryCount();

            for ( sal_uInt16 i = 1; i < nEntries; i++ )
            {
                OUString aEntry = GetEntry ( (i + nCurrentPos) % nEntries );
                aEntry = comphelper::string::stripStart(aEntry, ' ').toAsciiUpperCase();
                OUString aCompare = OUString(cCharCode).toAsciiUpperCase();

                if ( aEntry.startsWith( aCompare ) )
                {
                    SelectEntryPos ( (i + nCurrentPos) % nEntries );
                    break;
                }
            }
        }
        else if ( aKeyEvt.GetKeyCode().GetCode() == KEY_RETURN )
        {
            DoubleClick();
        }
    }

    return ListBox::PreNotify ( rNEvt );
}

ImpPathDialog::ImpPathDialog( PathDialog* pDlg, RESOURCE_TYPE nType, sal_Bool bCreateDir )
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
#   if defined(UNX)
    delete pHomeBtn;
#   endif
}

void ImpPathDialog::InitControls()
{
    PathDialog* pDlg = GetPathDialog();
    pDlg->SetText( SVT_RESSTR(STR_FILEDLG_SELECT) );

    Size a3Siz = pDlg->LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
    Size a6Siz = pDlg->LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    Size aBtnSiz = pDlg->LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
    Size aFTSiz = pDlg->LogicToPixel( Size( 142, 10 ), MAP_APPFONT );
    Size aEDSiz = pDlg->LogicToPixel( Size( 142, 12 ), MAP_APPFONT );
    Point aPnt( a6Siz.Width(), a6Siz.Height() );
    long nLbH1 = pDlg->LogicToPixel( Size( 0, 93 ), MAP_APPFONT ).Height();
    long nH = 0;
    OUString aEmptyStr;

    INITCONTROL( pDirTitel, FixedText, 0,
                 aPnt, aFTSiz, SVT_RESSTR( STR_FILEDLG_DIR ), HID_FILEDLG_DIR );
    aPnt.Y() += aFTSiz.Height() + a3Siz.Height();

    INITCONTROL( pEdit, Edit, WB_BORDER, aPnt, aEDSiz, aPath.GetFull(), HID_FILEDLG_EDIT );

    aPnt.Y() += aEDSiz.Height() + a3Siz.Height();
#ifndef UNX
    long nLbH2 = pDlg->LogicToPixel( Size( 0, 60 ), MAP_APPFONT ).Height();
    INITCONTROL( pDirList, KbdListBox, WB_AUTOHSCROLL | WB_BORDER,
        aPnt, Size( aEDSiz.Width(), nLbH1 ), aEmptyStr, HID_FILEDLG_DIRS );
    aPnt.Y() += nLbH1 + a6Siz.Height();
    INITCONTROL( pDriveTitle, FixedText, 0,
                 aPnt, aFTSiz, SVT_RESSTR( STR_FILEDLG_DRIVES ), HID_FILEDLG_DRIVE );
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

    long nExtraWidth = pDlg->GetTextWidth( String( RTL_CONSTASCII_USTRINGPARAM( "(W)" ) ) )+10;
    String aOkStr = Button::GetStandardText( BUTTON_OK );
    long nTextWidth = pDlg->GetTextWidth( aOkStr )+nExtraWidth;
    if( nTextWidth > aBtnSiz.Width() )
        aBtnSiz.Width() = nTextWidth;

    String aCancelStr = Button::GetStandardText( BUTTON_CANCEL );
    nTextWidth = pDlg->GetTextWidth( aCancelStr )+nExtraWidth;
    if( nTextWidth > aBtnSiz.Width() )
        aBtnSiz.Width() = nTextWidth;

    String aNewDirStr( SVT_RESSTR( STR_FILEDLG_NEWDIR ) );
    nTextWidth = pDlg->GetTextWidth( aNewDirStr )+nExtraWidth;
    if( nTextWidth > aBtnSiz.Width() )
        aBtnSiz.Width() = nTextWidth;
#if defined(UNX)
    String aHomeDirStr( SVT_RESSTR( STR_FILEDLG_HOME ) );
    nTextWidth = pDlg->GetTextWidth( aHomeDirStr )+nExtraWidth;
    if( nTextWidth > aBtnSiz.Width() )
        aBtnSiz.Width() = nTextWidth;
#endif

    aPnt.X() = 2 * a6Siz.Width() + aEDSiz.Width();
    aPnt.Y() = a6Siz.Height();
    INITCONTROL( pOkBtn, PushButton, WB_DEFBUTTON,
                 aPnt, aBtnSiz, aOkStr, "" );
    aPnt.Y() += aBtnSiz.Height() + a3Siz.Height();
    INITCONTROL( pCancelBtn, CancelButton, 0,
                 aPnt, aBtnSiz, aCancelStr, "" );
    aPnt.Y() += aBtnSiz.Height() + a3Siz.Height();
    INITCONTROL( pNewDirBtn, PushButton, WB_DEFBUTTON,
                 aPnt, aBtnSiz, aNewDirStr, HID_FILEDLG_NEWDIR );
#if defined(UNX)
    aPnt.Y() += aBtnSiz.Height() + a3Siz.Height();
    INITCONTROL( pHomeBtn, PushButton, WB_DEFBUTTON,
                 aPnt, aBtnSiz, aHomeDirStr, HID_FILEDLG_HOME );
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

    nOwnChildren = pDlg->GetChildCount();
}



IMPL_LINK( ImpPathDialog, SelectHdl, ListBox *, p )
{
    if( p == pDriveList )
    {
        OUString aDrive = OUString( pDriveList->GetSelectEntry().Copy(0, 2) ) + "\\";
        SetPath( aDrive );
    }
    else if( p == pDirList )
    {
        // isolate the pure name of the entry
        // removing trainling stuff and leading spaces
        OUString aEntry( pDirList->GetSelectEntry() );
        aEntry = comphelper::string::stripStart(aEntry, ' ');

        sal_Int32 nPos = aEntry.indexOf( '/' );
        if (nPos >= 0)
            aEntry = aEntry.replaceAt( nPos, aEntry.getLength()-nPos, "" );

        // build the absolute path to the selected item
        DirEntry aNewPath;
        aNewPath.ToAbs();

        sal_uInt16 nCurPos = pDirList->GetSelectEntryPos();

        // Wird nach oben gewechselt
        if( nCurPos < nDirCount )
            aNewPath = aNewPath[nDirCount-nCurPos-1];
        else
            aNewPath += String(aEntry);

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
            GetPathDialog()->EndDialog( sal_True );
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
    else if ( pBtn == pCancelBtn )
    {
        GetPathDialog()->EndDialog( sal_False );
    }
    else if ( pBtn == pHomeBtn )
    {
        ::rtl::OUString aHomeDir;
        osl::Security aSecurity;
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
    else if ( pBtn == pNewDirBtn )
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
    OUString aEntry( pBox->GetSelectEntry() );

    aEntry = comphelper::string::stripStart(aEntry, ' ');
    sal_Int32 nPos = aEntry.indexOf( '/' );
    if (nPos >= 0)
        aEntry = aEntry.replaceAt( nPos, aEntry.getLength()-nPos, "" );

    // build the absolute path to the selected item
    DirEntry aNewPath;
    aNewPath.ToAbs();
    if( pBox == pDirList )
    {
        sal_uInt16 nCurPos = pDirList->GetSelectEntryPos();

        // Wenn es schon das aktuelle ist, dann mache nichts
        if( nCurPos == nDirCount-1 )
            return 0;

        // Wird nach oben gewechselt
        if( nCurPos < nDirCount )
            aNewPath = aNewPath[nDirCount-nCurPos-1];
        else
            aNewPath += String(aEntry);
    }
    else
        aNewPath += String(aEntry);

    pSvPathDialog->EnterWait();

    if( FileStat( aNewPath ).GetKind() & FSYS_KIND_DIR )
    {
        // Neuen Pfad setzen und Listboxen updaten
        aPath = aNewPath;
        if( !aPath.SetCWD( sal_True ) )
        {
            ErrorBox aBox( GetPathDialog(),
                           WB_OK_CANCEL | WB_DEF_OK,
                           SVT_RESSTR( STR_FILEDLG_CANTCHDIR ) );
            if( aBox.Execute() == RET_CANCEL )
                GetPathDialog()->EndDialog( sal_False );
        }
        UpdateEntries( sal_True );
    }

    pSvPathDialog->LeaveWait();
    return 0;
}

void ImpPathDialog::UpdateEntries( const sal_Bool )
{
    OUString aTabString;
    DirEntry aTmpPath;
    aTmpPath.ToAbs();

    nDirCount = aTmpPath.Level();

    pDirList->SetUpdateMode( sal_False );
    pDirList->Clear();

    for( sal_uInt16 i = nDirCount; i > 0; i-- )
    {
        OUString aName = aTabString + aTmpPath[i-1].GetName();
        pDirList->InsertEntry( aName );
        aTabString += "  ";
    }

    // scan the directory
    DirEntry aCurrent;
    aCurrent.ToAbs();

    Dir aDir( aCurrent, FSYS_KIND_DIR|FSYS_KIND_FILE );

    sal_uInt16 nEntries = aDir.Count();
    if( nEntries )
    {
        OUStringList aSortDirList;
        const comphelper::string::NaturalStringSorter& rSorter =
            ::vcl::unohelper::getNaturalStringSorterForAppLocale();
        for ( sal_uInt16 n = 0; n < nEntries; n++ )
        {
            DirEntry& rEntry = aDir[n];
            OUString aName( rEntry.GetName() );
            if( aName.getLength() && ( aName[0] != '.' ) && rEntry.Exists() )
            {
                if( FileStat( rEntry ).GetKind() & FSYS_KIND_DIR )
                {
                    size_t l = 0;
                    for( l = 0; l < aSortDirList.size(); l++ )
                        if( rSorter.compare( aSortDirList[l], aName ) > 0 )
                            break;
                    if ( l < aSortDirList.size() ) {
                        OUStringList::iterator it = aSortDirList.begin();
                        ::std::advance( it, l );
                        aSortDirList.insert( it, aName );
                    } else {
                        aSortDirList.push_back( aName );
                    }
                }
            }
        }

        for( size_t l = 0; l < aSortDirList.size(); l++ )
        {
            OUString aEntryStr = aTabString + aSortDirList[l];
            pDirList->InsertEntry( aEntryStr );
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
    pDirList->SetUpdateMode( sal_True );
    pDirList->Invalidate();
    pDirList->Update();

    OUString aDirName = rTmpPath.GetFull();
    if( pDirPath )
        pDirPath->SetText( aDirName );
    else
        pEdit->SetText( aDirName );
}

sal_Bool ImpPathDialog::IsFileOk( const DirEntry& rDirEntry )
{
    if( FileStat( rDirEntry ).GetKind() & (FSYS_KIND_WILD | FSYS_KIND_DEV) )
        return sal_False;
    else
    {
        // Datei vorhanden ?
        if( ! rDirEntry.Exists() )
        {
            OUString aQueryTxt( SVT_RESSTR( STR_FILEDLG_ASKNEWDIR ) );
            aQueryTxt = aQueryTxt.replaceFirst( "%s", rDirEntry.GetFull() );
            QueryBox aQuery( GetPathDialog(),
                             WB_YES_NO | WB_DEF_YES,
                             aQueryTxt  );
            if( aQuery.Execute() == RET_YES )
                rDirEntry.MakeDir();
            else
                return sal_False;
        }
        if( !FileStat( rDirEntry ).IsKind( FSYS_KIND_DIR ) )
        {
            OUString aBoxText = SVT_RESSTR( STR_FILEDLG_CANTOPENDIR ) + "\n[" + rDirEntry.GetFull() + "]" ;
            InfoBox aBox( GetPathDialog(), aBoxText );
            aBox.Execute();
            return sal_False;
        }
    }
    return GetPathDialog()->OK() != 0;
}


void ImpPathDialog::PreExecute()
{
    // Neues Verzeichnis setzen und Listboxen updaten
    aPath.SetCWD( sal_True );
    UpdateEntries( sal_True );

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
    sal_uInt16 nChildren = GetPathDialog()->GetChildCount();
    sal_uInt16 n;
    for ( n = nOwnChildren; n < nChildren; n++ )
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
        aSize.setWidth(nMaxWidth);

        if( pOkBtn )
            pOkBtn->SetSizePixel( aSize );
        if( pCancelBtn )
            pCancelBtn->SetSizePixel( aSize );
        if( pLoadBtn )
            pLoadBtn->SetSizePixel( aSize );
    }

    for ( n = nOwnChildren; n < nChildren; n++ )
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

        sal_uInt16 nCount = aDir.Count(), i;
        for( i = 0; i < nCount; ++i )
        {
            DirEntry& rEntry = aDir[i];
            OUString aStr    = rEntry.GetFull( FSYS_STYLE_HOST, sal_False );

            OUString aVolume = rEntry.GetVolume() ;
            aStr = aStr.toAsciiUpperCase();
            if ( aVolume.getLength() )
            {
                aStr = aStr + " " + aVolume;
            }
            pDriveList->InsertEntry( String(aStr) );

        }
        OUString aPathStr = aPath.GetFull();
        aPathStr = aPathStr.toAsciiUpperCase();

        for ( i = 0; i < pDriveList->GetEntryCount(); ++i )
        {
            OUString aEntry = pDriveList->GetEntry(i);
            aEntry = aEntry.toAsciiUpperCase();
            sal_Int32 nLen = aEntry.getLength();
            nLen = nLen > 2 ? 2 : nLen;
            if ( aEntry.compareTo( aPathStr, nLen ) == 0 )
            {
                pDriveList->SelectEntryPos(i);
                break;
            }
        }
    }
}

void ImpPathDialog::SetPath( OUString const & rPath )
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
    aFile.SetCWD( sal_True );
    UpdateEntries( sal_True );

    pSvPathDialog->LeaveWait();
}

OUString ImpPathDialog::GetPath() const
{
    DirEntry aFile( pEdit->GetText() );
    aFile.ToAbs();
    return aFile.GetFull();
}

void ImpSvFileDlg::CreatePathDialog( PathDialog* pSvDlg, bool bCreate )
{
    delete pDlg;
    pDlg = new ImpPathDialog( pSvDlg, WINDOW_PATHDIALOG, bCreate );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
