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

#undef SC_DLLIMPLEMENTATION

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
#include "globstr.hrc"
#include "instbdlg.hxx"

ScInsertTableDlg::ScInsertTableDlg( vcl::Window* pParent, ScViewData& rData, SCTAB nTabCount, bool bFromFile )
    : ModalDialog(pParent, "InsertSheetDialog", "modules/scalc/ui/insertsheet.ui")
    , rViewData(rData)
    , rDoc(*rData.GetDocument())
    , pDocShTables(nullptr)
    , pDocInserter(nullptr)
    , bMustClose(false)
    , nSelTabIndex(0)
    , nTableCount(nTabCount)
{
    get(m_pBtnBefore, "before");
    get(m_pBtnBehind, "after");
    get(m_pBtnNew, "new");
    get(m_pBtnFromFile, "fromfile");
    get(m_pFtCount, "countft");
    get(m_pNfCount, "countnf");
    get(m_pFtName, "nameft");
    get(m_pEdName, "nameed");
    m_sSheetDotDotDot = m_pEdName->GetText();
    get(m_pLbTables, "tables");
    m_pLbTables->SetDropDownLineCount(8);
    get(m_pFtPath, "path");
    get(m_pBtnBrowse, "browse");
    get(m_pBtnLink, "link");
    get(m_pBtnOk, "ok");
    Init_Impl( bFromFile );
}

ScInsertTableDlg::~ScInsertTableDlg()
{
    disposeOnce();
}

void ScInsertTableDlg::dispose()
{
    if (pDocShTables)
        pDocShTables->DoClose();
    delete pDocInserter;
    m_pBtnBefore.clear();
    m_pBtnBehind.clear();
    m_pBtnNew.clear();
    m_pBtnFromFile.clear();
    m_pFtCount.clear();
    m_pNfCount.clear();
    m_pFtName.clear();
    m_pEdName.clear();
    m_pLbTables.clear();
    m_pFtPath.clear();
    m_pBtnBrowse.clear();
    m_pBtnLink.clear();
    m_pBtnOk.clear();
    ModalDialog::dispose();
}

void ScInsertTableDlg::Init_Impl( bool bFromFile )
{
    m_pLbTables->EnableMultiSelection(true);
    m_pBtnBrowse->SetClickHdl( LINK( this, ScInsertTableDlg, BrowseHdl_Impl ) );
    m_pBtnNew->SetClickHdl( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    m_pBtnFromFile->SetClickHdl( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    m_pLbTables->SetSelectHdl( LINK( this, ScInsertTableDlg, SelectHdl_Impl ) );
    m_pNfCount->SetModifyHdl( LINK( this, ScInsertTableDlg, CountHdl_Impl));
    m_pBtnOk->SetClickHdl( LINK( this, ScInsertTableDlg, DoEnterHdl ));
    m_pBtnBefore->Check();

    m_pNfCount->SetText( OUString::number(nTableCount) );
    m_pNfCount->SetMax( MAXTAB - rDoc.GetTableCount() + 1 );

    if(nTableCount==1)
    {
        OUString aName;
        rDoc.CreateValidTabName( aName );
        m_pEdName->SetText( aName );
    }
    else
    {
        m_pEdName->SetText(m_sSheetDotDotDot);
        m_pFtName->Disable();
        m_pEdName->Disable();
    }

    bool bShared = rViewData.GetDocShell() && rViewData.GetDocShell()->IsDocShared();

    if ( !bFromFile || bShared )
    {
        m_pBtnNew->Check();
        SetNewTable_Impl();
        if ( bShared )
        {
            m_pBtnFromFile->Disable();
        }
    }
    else
    {
        m_pBtnFromFile->Check();
        SetFromTo_Impl();

        aBrowseTimer.SetTimeoutHdl( LINK( this, ScInsertTableDlg, BrowseTimeoutHdl ) );
        aBrowseTimer.SetTimeout( 200 );
    }
}

short ScInsertTableDlg::Execute()
{
    if ( m_pBtnFromFile->IsChecked() )
        aBrowseTimer.Start();

    return ModalDialog::Execute();
}

void ScInsertTableDlg::SetNewTable_Impl()
{
    if (m_pBtnNew->IsChecked() )
    {
        m_pNfCount->Enable();
        m_pFtCount->Enable();
        m_pLbTables->Disable();
        m_pFtPath->Disable();
        m_pBtnBrowse->Disable();
        m_pBtnLink->Disable();

        if(nTableCount==1)
        {
            m_pEdName->Enable();
            m_pFtName->Enable();
        }
    }
}

void ScInsertTableDlg::SetFromTo_Impl()
{
    if (m_pBtnFromFile->IsChecked() )
    {
        m_pEdName->Disable();
        m_pFtName->Disable();
        m_pFtCount->Disable();
        m_pNfCount->Disable();
        m_pLbTables->Enable();
        m_pFtPath->Enable();
        m_pBtnBrowse->Enable();
        m_pBtnLink->Enable();
    }
}

void ScInsertTableDlg::FillTables_Impl( ScDocument* pSrcDoc )
{
    m_pLbTables->SetUpdateMode( false );
    m_pLbTables->Clear();

    if ( pSrcDoc )
    {
        SCTAB nCount = pSrcDoc->GetTableCount();
        OUString aName;

        for ( SCTAB i=0; i<nCount; i++ )
        {
            pSrcDoc->GetName( i, aName );
            m_pLbTables->InsertEntry( aName );
        }
    }

    m_pLbTables->SetUpdateMode( true );

    if(m_pLbTables->GetEntryCount()==1)
        m_pLbTables->SelectEntryPos(0);
}

const OUString* ScInsertTableDlg::GetFirstTable( sal_uInt16* pN )
{
    const OUString* pStr = nullptr;

    if ( m_pBtnNew->IsChecked() )
    {
        aStrCurSelTable = m_pEdName->GetText();
        pStr = &aStrCurSelTable;
    }
    else if ( nSelTabIndex < m_pLbTables->GetSelectEntryCount() )
    {
        aStrCurSelTable = m_pLbTables->GetSelectEntry();
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = m_pLbTables->GetSelectEntryPos();
        nSelTabIndex = 1;
    }

    return pStr;
}

const OUString* ScInsertTableDlg::GetNextTable( sal_uInt16* pN )
{
    const OUString* pStr = nullptr;

    if ( !m_pBtnNew->IsChecked() && nSelTabIndex < m_pLbTables->GetSelectEntryCount() )
    {
        aStrCurSelTable = m_pLbTables->GetSelectEntry( nSelTabIndex );
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = m_pLbTables->GetSelectEntryPos( nSelTabIndex );
        nSelTabIndex++;
    }

    return pStr;
}

// Handler:

IMPL_LINK_NOARG_TYPED(ScInsertTableDlg, CountHdl_Impl, Edit&, void)
{
    nTableCount = static_cast<SCTAB>(m_pNfCount->GetValue());
    if ( nTableCount==1)
    {
        OUString aName;
        rDoc.CreateValidTabName( aName );
        m_pEdName->SetText( aName );
        m_pFtName->Enable();
        m_pEdName->Enable();
    }
    else
    {
        m_pEdName->SetText(m_sSheetDotDotDot);
        m_pFtName->Disable();
        m_pEdName->Disable();
    }

    DoEnable_Impl();
}

IMPL_LINK_NOARG_TYPED(ScInsertTableDlg, ChoiceHdl_Impl, Button*, void)
{
    if ( m_pBtnNew->IsChecked() )
        SetNewTable_Impl();
    else
        SetFromTo_Impl();

    DoEnable_Impl();
}

IMPL_LINK_NOARG_TYPED(ScInsertTableDlg, BrowseHdl_Impl, Button*, void)
{
    delete pDocInserter;
    pDocInserter = new ::sfx2::DocumentInserter(
            OUString::createFromAscii( ScDocShell::Factory().GetShortName() ) );
    pDocInserter->StartExecuteModal( LINK( this, ScInsertTableDlg, DialogClosedHdl ) );
}

IMPL_LINK_NOARG_TYPED(ScInsertTableDlg, SelectHdl_Impl, ListBox&, void)
{
    DoEnable_Impl();
}

void ScInsertTableDlg::DoEnable_Impl()
{
    if ( m_pBtnNew->IsChecked() || ( pDocShTables && m_pLbTables->GetSelectEntryCount() ) )
        m_pBtnOk->Enable();
    else
        m_pBtnOk->Disable();
}

IMPL_LINK_NOARG_TYPED(ScInsertTableDlg, DoEnterHdl, Button*, void)
{
    if(nTableCount > 1 || ScDocument::ValidTabName(m_pEdName->GetText()))
    {
        EndDialog(RET_OK);
    }
    else
    {
        OUString aErrMsg ( ScGlobal::GetRscString( STR_INVALIDTABNAME ) );
        (void)ScopedVclPtrInstance<MessageDialog>(this, aErrMsg)->Execute();
    }
}

IMPL_LINK_NOARG_TYPED(ScInsertTableDlg, BrowseTimeoutHdl, Timer *, void)
{
    bMustClose = true;
    BrowseHdl_Impl(m_pBtnBrowse);
}

IMPL_LINK_TYPED( ScInsertTableDlg, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        SfxMedium* pMed = pDocInserter->CreateMedium();
        if ( pMed )
        {
            //  ERRCTX_SFX_OPENDOC -> "Error loading document"
            SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

            if ( pDocShTables )
                pDocShTables->DoClose();        // deletion is done when assigning to the reference

            pMed->UseInteractionHandler( true );    // to enable the filter options dialog

            pDocShTables = new ScDocShell;
            aDocShTablesRef = pDocShTables;

            Pointer aOldPtr( GetPointer() );
            SetPointer( Pointer( PointerStyle::Wait ) );
            pDocShTables->DoLoad( pMed );
            SetPointer( aOldPtr );

            sal_uLong nErr = pDocShTables->GetErrorCode();
            if ( nErr )
                ErrorHandler::HandleError( nErr );              // warnings, too

            if ( !pDocShTables->GetError() )                    // errors only
            {
                FillTables_Impl( &pDocShTables->GetDocument() );
                m_pFtPath->SetText( pDocShTables->GetTitle( SFX_TITLE_FULLNAME ) );
            }
            else
            {
                pDocShTables->DoClose();
                aDocShTablesRef.Clear();
                pDocShTables = nullptr;

                FillTables_Impl( nullptr );
                m_pFtPath->SetText( EMPTY_OUSTRING );
            }
        }

        DoEnable_Impl();
    }
    else if ( bMustClose )
        // execute slot FID_INS_TABLE_EXT and cancel file dialog
        EndDialog();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
