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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "scresid.hxx"
#include "instbdlg.hrc"
#include "globstr.hrc"

#define SC_INSTBDLG_CXX
#include "instbdlg.hxx"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef ScResId
#define ScResId(x) #x
#undef ModalDialog
#define ModalDialog( parent, id ) Dialog( parent, "insert-sheet.xml", id )
#undef ErrorBox
#define ErrorBox( this, bits, message ) ErrorBox (LAYOUT_THIS_WINDOW (this), bits, message)
#endif /* ENABLE_LAYOUT */

//==================================================================

ScInsertTableDlg::ScInsertTableDlg( Window* pParent, ScViewData& rData, SCTAB nTabCount, bool bFromFile )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_INSERT_TABLE ) ),
        //
        aFlPos          ( this, ScResId( FL_POSITION ) ),
        aBtnBefore      ( this, ScResId( RB_BEFORE ) ),
        aBtnBehind      ( this, ScResId( RB_BEHIND ) ),
        aFlTable        ( this, ScResId( FL_TABLE ) ),
        aBtnNew         ( this, ScResId( RB_NEW ) ),
        aBtnFromFile    ( this, ScResId( RB_FROMFILE ) ),
        aFtCount        ( this, ScResId( FT_COUNT ) ),
        aNfCount        ( this, ScResId( NF_COUNT ) ),
        aFtName         ( this, ScResId( FT_NAME ) ),
        aEdName         ( this, ScResId( ED_TABNAME ) ),
        aLbTables       ( this, ScResId( LB_TABLES ) ),
        aFtPath         ( this, ScResId( FT_PATH ) ),
        aBtnBrowse      ( this, ScResId( BTN_BROWSE ) ),
        aBtnLink        ( this, ScResId( CB_LINK ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        rViewData       ( rData ),
        rDoc            ( *rData.GetDocument() ),
        pDocShTables    ( NULL ),
        pDocInserter    ( NULL ),
        bMustClose      ( false ),
        nSelTabIndex    ( 0 ),
        nTableCount     (nTabCount)
{
#if ENABLE_LAYOUT
    SetHelpId (SID_INSERT_TABLE);
    aFtPath.SetText (EMPTY_STRING);
#endif /* ENABLE_LAYOUT */
    Init_Impl( bFromFile );
    FreeResource();
    aLbTables.SetAccessibleName(aBtnFromFile.GetText());
}

//------------------------------------------------------------------------

ScInsertTableDlg::~ScInsertTableDlg()
{
    if (pDocShTables)
        pDocShTables->DoClose();
    delete pDocInserter;
}

//------------------------------------------------------------------------

void ScInsertTableDlg::Init_Impl( bool bFromFile )
{
    aBtnBrowse      .SetClickHdl( LINK( this, ScInsertTableDlg, BrowseHdl_Impl ) );
    aBtnNew         .SetClickHdl( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    aBtnFromFile    .SetClickHdl( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    aLbTables       .SetSelectHdl( LINK( this, ScInsertTableDlg, SelectHdl_Impl ) );
    aNfCount        .SetModifyHdl( LINK( this, ScInsertTableDlg, CountHdl_Impl));
    aBtnOk          .SetClickHdl( LINK( this, ScInsertTableDlg, DoEnterHdl ));
    aBtnBefore.Check();

    aNfCount.SetText( String::CreateFromInt32(nTableCount) );
    aNfCount.SetMax( MAXTAB - rDoc.GetTableCount() + 1 );

    if(nTableCount==1)
    {
        String aName;
        rDoc.CreateValidTabName( aName );
        aEdName.SetText( aName );
    }
    else
    {
        String aName=aFlTable.GetText();
        aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM("..."));
        aEdName.SetText( aName );
        aFtName.Disable();
        aEdName.Disable();
    }

    bool bShared = ( rViewData.GetDocShell() ? rViewData.GetDocShell()->IsDocShared() : false );

    if ( !bFromFile || bShared )
    {
        aBtnNew.Check();
        SetNewTable_Impl();
        if ( bShared )
        {
            aBtnFromFile.Disable();
        }
    }
    else
    {
        aBtnFromFile.Check();
        SetFromTo_Impl();

        aBrowseTimer.SetTimeoutHdl( LINK( this, ScInsertTableDlg, BrowseTimeoutHdl ) );
        aBrowseTimer.SetTimeout( 200 );
    }
}

//------------------------------------------------------------------------

#if ENABLE_LAYOUT
#undef ModalDialog
#define ModalDialog Dialog
#endif /* ENABLE_LAYOUT */

short ScInsertTableDlg::Execute()
{
    // set Parent of DocumentInserter and Doc-Manager
    Window* pOldDefParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( LAYOUT_THIS_WINDOW (this) );

    if ( aBtnFromFile.IsChecked() )
        aBrowseTimer.Start();

    short nRet = ModalDialog::Execute();
    Application::SetDefDialogParent( pOldDefParent );
    return nRet;
}

//------------------------------------------------------------------------

void ScInsertTableDlg::SetNewTable_Impl()
{
    if (aBtnNew.IsChecked() )
    {
        aNfCount    .Enable();
        aFtCount    .Enable();
        aLbTables   .Disable();
        aFtPath     .Disable();
        aBtnBrowse  .Disable();
        aBtnLink    .Disable();

        if(nTableCount==1)
        {
            aEdName.Enable();
            aFtName.Enable();
        }
    }
}

//------------------------------------------------------------------------

void ScInsertTableDlg::SetFromTo_Impl()
{
    if (aBtnFromFile.IsChecked() )
    {
        aEdName     .Disable();
        aFtName     .Disable();
        aFtCount    .Disable();
        aNfCount    .Disable();
        aLbTables   .Enable();
        aFtPath     .Enable();
        aBtnBrowse  .Enable();
        aBtnLink    .Enable();
    }
}

//------------------------------------------------------------------------

void ScInsertTableDlg::FillTables_Impl( ScDocument* pSrcDoc )
{
    aLbTables.SetUpdateMode( false );
    aLbTables.Clear();

    if ( pSrcDoc )
    {
        SCTAB nCount = pSrcDoc->GetTableCount();
        String aName;

        for ( SCTAB i=0; i<nCount; i++ )
        {
            pSrcDoc->GetName( i, aName );
            aLbTables.InsertEntry( aName );
        }
    }

    aLbTables.SetUpdateMode( sal_True );

    if(aLbTables.GetEntryCount()==1)
        aLbTables.SelectEntryPos(0);
}

//------------------------------------------------------------------------

const String* ScInsertTableDlg::GetFirstTable( sal_uInt16* pN )
{
    const String* pStr = NULL;

    if ( aBtnNew.IsChecked() )
    {
        aStrCurSelTable = aEdName.GetText();
        pStr = &aStrCurSelTable;
    }
    else if ( nSelTabIndex < aLbTables.GetSelectEntryCount() )
    {
        aStrCurSelTable = aLbTables.GetSelectEntry( 0 );
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = aLbTables.GetSelectEntryPos( 0 );
        nSelTabIndex = 1;
    }

    return pStr;
}

//------------------------------------------------------------------------

const String* ScInsertTableDlg::GetNextTable( sal_uInt16* pN )
{
    const String* pStr = NULL;

    if ( !aBtnNew.IsChecked() && nSelTabIndex < aLbTables.GetSelectEntryCount() )
    {
        aStrCurSelTable = aLbTables.GetSelectEntry( nSelTabIndex );
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = aLbTables.GetSelectEntryPos( nSelTabIndex );
        nSelTabIndex++;
    }

    return pStr;
}


//------------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------------

IMPL_LINK( ScInsertTableDlg, CountHdl_Impl, NumericField*, EMPTYARG )
{
    nTableCount = static_cast<SCTAB>(aNfCount.GetValue());
    if ( nTableCount==1)
    {
        String aName;
        rDoc.CreateValidTabName( aName );
        aEdName.SetText( aName );
        aFtName.Enable();
        aEdName.Enable();
    }
    else
    {
        String aName=aFlTable.GetText();
        aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM("..."));
        aEdName.SetText( aName );
        aFtName.Disable();
        aEdName.Disable();
    }

    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------
IMPL_LINK( ScInsertTableDlg, ChoiceHdl_Impl, RadioButton*, EMPTYARG )
{
    if ( aBtnNew.IsChecked() )
        SetNewTable_Impl();
    else
        SetFromTo_Impl();

    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScInsertTableDlg, BrowseHdl_Impl, PushButton*, EMPTYARG )
{
    if ( pDocInserter )
        delete pDocInserter;
    pDocInserter = new ::sfx2::DocumentInserter(
            0, String::CreateFromAscii( ScDocShell::Factory().GetShortName() ) );
    pDocInserter->StartExecuteModal( LINK( this, ScInsertTableDlg, DialogClosedHdl ) );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScInsertTableDlg, SelectHdl_Impl, MultiListBox*, EMPTYARG )
{
    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------

void ScInsertTableDlg::DoEnable_Impl()
{
    if ( aBtnNew.IsChecked() || ( pDocShTables && aLbTables.GetSelectEntryCount() ) )
        aBtnOk.Enable();
    else
        aBtnOk.Disable();
}

IMPL_LINK( ScInsertTableDlg, DoEnterHdl, PushButton*, EMPTYARG )
{
    if(nTableCount > 1 || rDoc.ValidTabName(aEdName.GetText()))
    {
        EndDialog(RET_OK);
    }
    else
    {
        String aErrMsg ( ScGlobal::GetRscString( STR_INVALIDTABNAME ) );
        (void)ErrorBox( this,WinBits( WB_OK | WB_DEF_OK ),aErrMsg).Execute();
    }
    return 0;
}

IMPL_LINK( ScInsertTableDlg, BrowseTimeoutHdl, Timer*, EMPTYARG )
{
    bMustClose = true;
    BrowseHdl_Impl( &aBtnBrowse );
    return 0;
}

IMPL_LINK( ScInsertTableDlg, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg )
{
    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        SfxMedium* pMed = pDocInserter->CreateMedium();
        if ( pMed )
        {
            //  ERRCTX_SFX_OPENDOC -> "Fehler beim Laden des Dokumentes"
            SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

            if ( pDocShTables )
                pDocShTables->DoClose();        // delete passiert beim Zuweisen auf die Ref

            pMed->UseInteractionHandler( sal_True );    // to enable the filter options dialog

            pDocShTables = new ScDocShell;
            aDocShTablesRef = pDocShTables;

            Pointer aOldPtr( GetPointer() );
            SetPointer( Pointer( POINTER_WAIT ) );
            pDocShTables->DoLoad( pMed );
            SetPointer( aOldPtr );

            sal_uLong nErr = pDocShTables->GetErrorCode();
            if ( nErr )
                ErrorHandler::HandleError( nErr );              // auch Warnings

            if ( !pDocShTables->GetError() )                    // nur Errors
            {
                FillTables_Impl( pDocShTables->GetDocument() );
                aFtPath.SetText( pDocShTables->GetTitle( SFX_TITLE_FULLNAME ) );
            }
            else
            {
                pDocShTables->DoClose();
                aDocShTablesRef.Clear();
                pDocShTables = NULL;

                FillTables_Impl( NULL );
                aFtPath.SetText( EMPTY_STRING );
            }
        }

        DoEnable_Impl();
    }
    else if ( bMustClose )
        // execute slot FID_INS_TABLE_EXT and cancel file dialog
        EndDialog( RET_CANCEL );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
