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

#include <sfx2/docfile.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/weld.hxx>

#include <global.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <instbdlg.hxx>

ScInsertTableDlg::ScInsertTableDlg(weld::Window* pParent, ScViewData& rData, SCTAB nTabCount, bool bFromFile)
    : GenericDialogController(pParent, "modules/scalc/ui/insertsheet.ui", "InsertSheetDialog")
    , rViewData(rData)
    , rDoc(*rData.GetDocument())
    , pDocShTables(nullptr)
    , bMustClose(false)
    , nSelTabIndex(0)
    , nTableCount(nTabCount)
    , m_xBtnBefore(m_xBuilder->weld_radio_button("before"))
    , m_xBtnBehind(m_xBuilder->weld_radio_button("after"))
    , m_xBtnNew(m_xBuilder->weld_radio_button("new"))
    , m_xBtnFromFile(m_xBuilder->weld_radio_button("fromfile"))
    , m_xFtCount(m_xBuilder->weld_label("countft"))
    , m_xNfCount(m_xBuilder->weld_spin_button("countnf"))
    , m_xFtName(m_xBuilder->weld_label("nameft"))
    , m_xEdName(m_xBuilder->weld_entry("nameed"))
    , m_xLbTables(m_xBuilder->weld_tree_view("tables"))
    , m_xFtPath(m_xBuilder->weld_label("path"))
    , m_xBtnBrowse(m_xBuilder->weld_button("browse"))
    , m_xBtnLink(m_xBuilder->weld_check_button("link"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
{
    m_sSheetDotDotDot = m_xEdName->get_text();
    m_xLbTables->set_size_request(-1, m_xLbTables->get_height_rows(8));
    Init_Impl(bFromFile);
}

ScInsertTableDlg::~ScInsertTableDlg()
{
    if (pDocShTables)
        pDocShTables->DoClose();
    pDocInserter.reset();
}

void ScInsertTableDlg::Init_Impl( bool bFromFile )
{
    m_xLbTables->set_selection_mode(SelectionMode::Multiple);
    m_xBtnBrowse->connect_clicked( LINK( this, ScInsertTableDlg, BrowseHdl_Impl ) );
    m_xBtnNew->connect_clicked( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    m_xBtnFromFile->connect_clicked( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    m_xLbTables->connect_changed( LINK( this, ScInsertTableDlg, SelectHdl_Impl ) );
    m_xNfCount->connect_value_changed( LINK( this, ScInsertTableDlg, CountHdl_Impl));
    m_xBtnOk->connect_clicked( LINK( this, ScInsertTableDlg, DoEnterHdl ));
    m_xBtnBefore->set_active(true);

    m_xNfCount->set_max(MAXTAB - rDoc.GetTableCount() + 1);
    m_xNfCount->set_value(nTableCount);

    if(nTableCount==1)
    {
        OUString aName;
        rDoc.CreateValidTabName( aName );
        m_xEdName->set_text( aName );
    }
    else
    {
        m_xEdName->set_text(m_sSheetDotDotDot);
        m_xFtName->set_sensitive(false);
        m_xEdName->set_sensitive(false);
    }

    bool bShared = rViewData.GetDocShell() && rViewData.GetDocShell()->IsDocShared();

    if ( !bFromFile || bShared )
    {
        m_xBtnNew->set_active(true);
        SetNewTable_Impl();
        if ( bShared )
        {
            m_xBtnFromFile->set_sensitive(false);
        }
    }
    else
    {
        m_xBtnFromFile->set_active(true);
        SetFromTo_Impl();

        aBrowseTimer.SetInvokeHandler( LINK( this, ScInsertTableDlg, BrowseTimeoutHdl ) );
        aBrowseTimer.SetTimeout( 200 );
    }
}

short ScInsertTableDlg::run()
{
    if (m_xBtnFromFile->get_active())
        aBrowseTimer.Start();

    return GenericDialogController::run();
}

void ScInsertTableDlg::SetNewTable_Impl()
{
    if (m_xBtnNew->get_active() )
    {
        m_xNfCount->set_sensitive(true);
        m_xFtCount->set_sensitive(true);
        m_xLbTables->set_sensitive(false);
        m_xFtPath->set_sensitive(false);
        m_xBtnBrowse->set_sensitive(false);
        m_xBtnLink->set_sensitive(false);

        if(nTableCount==1)
        {
            m_xEdName->set_sensitive(true);
            m_xFtName->set_sensitive(true);
        }
    }
}

void ScInsertTableDlg::SetFromTo_Impl()
{
    if (m_xBtnFromFile->get_active() )
    {
        m_xEdName->set_sensitive(false);
        m_xFtName->set_sensitive(false);
        m_xFtCount->set_sensitive(false);
        m_xNfCount->set_sensitive(false);
        m_xLbTables->set_sensitive(true);
        m_xFtPath->set_sensitive(true);
        m_xBtnBrowse->set_sensitive(true);
        m_xBtnLink->set_sensitive(true);
    }
}

void ScInsertTableDlg::FillTables_Impl( const ScDocument* pSrcDoc )
{
    m_xLbTables->freeze();
    m_xLbTables->clear();

    if ( pSrcDoc )
    {
        SCTAB nCount = pSrcDoc->GetTableCount();
        OUString aName;

        for (SCTAB i=0; i<nCount; ++i)
        {
            pSrcDoc->GetName( i, aName );
            m_xLbTables->append_text(aName);
        }
    }

    m_xLbTables->thaw();

    if (m_xLbTables->n_children() == 1)
        m_xLbTables->select(0);
}

const OUString* ScInsertTableDlg::GetFirstTable( sal_uInt16* pN )
{
    const OUString* pStr = nullptr;

    if ( m_xBtnNew->get_active() )
    {
        aStrCurSelTable = m_xEdName->get_text();
        pStr = &aStrCurSelTable;
    }
    else
    {
        std::vector<int> aRows(m_xLbTables->get_selected_rows());
        if (nSelTabIndex < aRows.size())
        {
            aStrCurSelTable = m_xLbTables->get_text(aRows[0]);
            pStr = &aStrCurSelTable;
            if ( pN )
                *pN = aRows[0];
            nSelTabIndex = 1;
        }
    }

    return pStr;
}

const OUString* ScInsertTableDlg::GetNextTable( sal_uInt16* pN )
{
    if (m_xBtnNew->get_active())
        return nullptr;

    std::vector<int> aRows(m_xLbTables->get_selected_rows());

    const OUString* pStr = nullptr;
    if (nSelTabIndex < aRows.size())
    {
        aStrCurSelTable = m_xLbTables->get_text(aRows[nSelTabIndex]);
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = aRows[nSelTabIndex];
        nSelTabIndex++;
    }

    return pStr;
}

// Handler:

IMPL_LINK_NOARG(ScInsertTableDlg, CountHdl_Impl, weld::SpinButton&, void)
{
    nTableCount = static_cast<SCTAB>(m_xNfCount->get_value());
    if ( nTableCount==1)
    {
        OUString aName;
        rDoc.CreateValidTabName( aName );
        m_xEdName->set_text( aName );
        m_xFtName->set_sensitive(true);
        m_xEdName->set_sensitive(true);
    }
    else
    {
        m_xEdName->set_text(m_sSheetDotDotDot);
        m_xFtName->set_sensitive(false);
        m_xEdName->set_sensitive(false);
    }

    DoEnable_Impl();
}

IMPL_LINK_NOARG(ScInsertTableDlg, ChoiceHdl_Impl, weld::Button&, void)
{
    if ( m_xBtnNew->get_active() )
        SetNewTable_Impl();
    else
        SetFromTo_Impl();

    DoEnable_Impl();
}

IMPL_LINK_NOARG(ScInsertTableDlg, BrowseHdl_Impl, weld::Button&, void)
{
    pDocInserter.reset();
    pDocInserter.reset( new ::sfx2::DocumentInserter(m_xDialog.get(), ScDocShell::Factory().GetFactoryName()) );
    pDocInserter->StartExecuteModal( LINK( this, ScInsertTableDlg, DialogClosedHdl ) );
}

IMPL_LINK_NOARG(ScInsertTableDlg, SelectHdl_Impl, weld::TreeView&, void)
{
    DoEnable_Impl();
}

void ScInsertTableDlg::DoEnable_Impl()
{
    if ( m_xBtnNew->get_active() || ( pDocShTables && m_xLbTables->count_selected_rows() ) )
        m_xBtnOk->set_sensitive(true);
    else
        m_xBtnOk->set_sensitive(false);
}

IMPL_LINK_NOARG(ScInsertTableDlg, DoEnterHdl, weld::Button&, void)
{
    if (nTableCount > 1 || ScDocument::ValidTabName(m_xEdName->get_text()))
    {
        m_xDialog->response(RET_OK);
    }
    else
    {
        OUString aErrMsg ( ScResId( STR_INVALIDTABNAME ) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                    VclButtonsType::Ok, aErrMsg));
        xBox->run();
    }
}

IMPL_LINK_NOARG(ScInsertTableDlg, BrowseTimeoutHdl, Timer *, void)
{
    bMustClose = true;
    BrowseHdl_Impl(*m_xBtnBrowse);
}

IMPL_LINK( ScInsertTableDlg, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        std::unique_ptr<SfxMedium> pMed = pDocInserter->CreateMedium();
        if ( pMed )
        {
            //  ERRCTX_SFX_OPENDOC -> "Error loading document"
            SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

            if ( pDocShTables )
                pDocShTables->DoClose();        // deletion is done when assigning to the reference

            pMed->UseInteractionHandler( true );    // to enable the filter options dialog

            pDocShTables = new ScDocShell;
            aDocShTablesRef = pDocShTables;

            {
                weld::WaitObject aWait(m_xDialog.get());
                pDocShTables->DoLoad(pMed.release());
            }

            ErrCode nErr = pDocShTables->GetErrorCode();
            if ( nErr )
                ErrorHandler::HandleError( nErr );              // warnings, too

            if ( !pDocShTables->GetError() )                    // errors only
            {
                FillTables_Impl( &pDocShTables->GetDocument() );
                m_xFtPath->set_label(pDocShTables->GetTitle(SFX_TITLE_FULLNAME));
            }
            else
            {
                pDocShTables->DoClose();
                aDocShTablesRef.clear();
                pDocShTables = nullptr;

                FillTables_Impl( nullptr );
                m_xFtPath->set_label(EMPTY_OUSTRING);
            }
        }

        DoEnable_Impl();
    }
    else if ( bMustClose )
        // execute slot FID_INS_TABLE_EXT and cancel file dialog
        m_xDialog->response(RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
