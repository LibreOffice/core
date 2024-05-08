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

#include <config_features.h>
#include <config_fuzzers.h>

#include <cmdid.h>
#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/viewfrm.hxx>
#include <osl/diagnose.h>
#include <viewopt.hxx>
#include <chldwrap.hxx>
#include <docsh.hxx>
#include "flddb.hxx"
#include "flddinf.hxx"
#include "fldvar.hxx"
#include "flddok.hxx"
#include "fldfunc.hxx"
#include "fldref.hxx"
#include <wrtsh.hxx>
#include <view.hxx>
#include <fldtdlg.hxx>
#include <swmodule.hxx>
#include <comphelper/lok.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

// carrier of the dialog
SwFieldDlg::SwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, weld::Window *pParent)
    : SfxTabDialogController(pParent, "modules/swriter/ui/fielddialog.ui", "FieldDialog")
    , m_pChildWin(pCW)
    , m_pBindings(pB)
    , m_bDataBaseMode(false)
    , m_bClosing(false)
{
    m_bHtmlMode = (::GetHtmlMode(static_cast<SwDocShell*>(SfxObjectShell::Current())) & HTMLMODE_ON) != 0;

    GetCancelButton().connect_clicked(LINK(this, SwFieldDlg, CancelHdl));
    GetOKButton().connect_clicked(LINK(this, SwFieldDlg, OKHdl));

    AddTabPage("document", SwFieldDokPage::Create, nullptr);
    AddTabPage("variables", SwFieldVarPage::Create, nullptr);
    AddTabPage("docinfo", SwFieldDokInfPage::Create, nullptr);

    if (!m_bHtmlMode)
    {
        AddTabPage("ref", SwFieldRefPage::Create, nullptr);
        AddTabPage("functions", SwFieldFuncPage::Create, nullptr);

        utl::OConfigurationTreeRoot aCfgRoot
            = utl::OConfigurationTreeRoot::createWithComponentContext(
                ::comphelper::getProcessComponentContext(),
                "/org.openoffice.Office.DataAccess/Policies/Features/Writer",
                -1,
                utl::OConfigurationTreeRoot::CM_READONLY);

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
        bool bDatabaseFields = true;
        aCfgRoot.getNodeValue(u"DatabaseFields"_ustr) >>= bDatabaseFields;

        if (bDatabaseFields)
            AddTabPage("database", SwFieldDBPage::Create, nullptr);
        else
#endif
            RemoveTabPage("database");
    }
    else
    {
        RemoveTabPage("ref");
        RemoveTabPage("functions");
        RemoveTabPage("database");
    }

    if (comphelper::LibreOfficeKit::isActive())
        RemoveTabPage("database");
}

SwFieldDlg::~SwFieldDlg()
{
}

void SwFieldDlg::EndDialog(int nResponse)
{
    m_bClosing = true;
    SfxTabDialogController::EndDialog(nResponse);
    m_bClosing = false;
}

void SwFieldDlg::Close()
{
    if (m_bClosing)
        return;
    const SfxPoolItemHolder aResult(m_pBindings->GetDispatcher()->
        Execute(m_bDataBaseMode ? FN_INSERT_FIELD_DATA_ONLY : FN_INSERT_FIELD,
        SfxCallMode::SYNCHRON|SfxCallMode::RECORD));
    if (!aResult)
    {
        // If Execute action did fail for whatever reason, this means that request
        // to close did fail or wasn't delivered to SwTextShell::ExecField().
        // Just explicitly close dialog in this case.
        SfxTabDialogController::EndDialog(RET_CLOSE);
    }
}

void SwFieldDlg::Initialize(SfxChildWinInfo const *pInfo)
{
    OUString aWinState = pInfo->aWinState;
    if (aWinState.isEmpty())
        return;
    m_xDialog->set_window_state(aWinState);
}

SfxItemSet* SwFieldDlg::CreateInputItemSet(const OUString& rID)
{
    SwDocShell *const pDocSh(static_cast<SwDocShell*>(SfxObjectShell::Current()));
    if (rID == "docinfo" && pDocSh) // might not have a shell if the dialog is restored on startup
    {
        mxInputItemSet = std::make_unique<SfxItemSetFixed<FN_FIELD_DIALOG_DOC_PROPS, FN_FIELD_DIALOG_DOC_PROPS>>( pDocSh->GetPool() );
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocSh->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        uno::Reference< beans::XPropertySet > xUDProps(
            xDocProps->getUserDefinedProperties(),
            uno::UNO_QUERY_THROW);
        mxInputItemSet->Put( SfxUnoAnyItem( FN_FIELD_DIALOG_DOC_PROPS, uno::Any(xUDProps) ) );
        return mxInputItemSet.get();
    }
    else
        return nullptr;
}

// kick off inserting of new fields
IMPL_LINK_NOARG(SwFieldDlg, OKHdl, weld::Button&, void)
{
    if (GetOKButton().get_sensitive())
    {
        SfxTabPage* pPage = GetTabPage(GetCurPageId());
        assert(pPage);
        pPage->FillItemSet(nullptr);

        GetOKButton().grab_focus();  // because of InputField-Dlg
    }
}

IMPL_LINK_NOARG(SwFieldDlg, CancelHdl, weld::Button&, void)
{
    Close();
}

// newly initialise dialog after Doc-Switch
void SwFieldDlg::ReInitDlg()
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    bool bNewMode = (::GetHtmlMode(pDocSh) & HTMLMODE_ON) != 0;

    if (bNewMode != m_bHtmlMode)
    {
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        {
            pViewFrm->GetDispatcher()->
                Execute(FN_INSERT_FIELD, SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
        }
        Close();
    }

    SwView* pActiveView = ::GetActiveView();
    if(!pActiveView)
        return;
    const SwWrtShell& rSh = pActiveView->GetWrtShell();
    GetOKButton().set_sensitive((  !rSh.IsReadOnlyAvailable()
                                || !rSh.HasReadonlySel())
                            &&  !SwCursorShell::PosInsideInputField(*rSh.GetCursor()->GetPoint()));

    ReInitTabPage(u"document");
    ReInitTabPage(u"variables");
    ReInitTabPage(u"docinfo");

    if (!m_bHtmlMode)
    {
        ReInitTabPage(u"ref");
        ReInitTabPage(u"functions");
        ReInitTabPage(u"database");
    }

    m_pChildWin->SetOldDocShell(pDocSh);
}

// newly initialise TabPage after Doc-Switch
void SwFieldDlg::ReInitTabPage(std::u16string_view rPageId, bool bOnlyActivate)
{
    SwFieldPage* pPage = static_cast<SwFieldPage*>(GetTabPage(rPageId));
    if (pPage)
        pPage->EditNewField( bOnlyActivate );   // newly initialise TabPage
}

// newly initialise after activation of a few TabPages
void SwFieldDlg::Activate()
{
    SwView* pView = GetActiveView();
    if( !pView )
        return;

    bool bHtmlMode = (::GetHtmlMode(static_cast<SwDocShell*>(SfxObjectShell::Current())) & HTMLMODE_ON) != 0;
    const SwWrtShell& rSh = pView->GetWrtShell();
    GetOKButton().set_sensitive((  !rSh.IsReadOnlyAvailable()
                                || !rSh.HasReadonlySel())
                            &&  !SwCursorShell::PosInsideInputField(*rSh.GetCursor()->GetPoint()));


    ReInitTabPage(u"variables", true);

    if( !bHtmlMode )
    {
        ReInitTabPage(u"ref", true);
        ReInitTabPage(u"functions", true);
    }
}

void SwFieldDlg::EnableInsert(bool bEnable)
{
    if( bEnable )
    {
        SwView* pView = ::GetActiveView();
        if( !pView ||
                (pView->GetWrtShell().IsReadOnlyAvailable() &&
                    pView->GetWrtShell().HasReadonlySel())
            || SwCursorShell::PosInsideInputField(*pView->GetWrtShell().GetCursor()->GetPoint()))
        {
            bEnable = false;
        }
    }
    GetOKButton().set_sensitive(bEnable);
}

void SwFieldDlg::InsertHdl()
{
    GetOKButton().clicked();
}

void SwFieldDlg::ActivateDatabasePage()
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    m_bDataBaseMode = true;
    ShowPage("database");
    SfxTabPage* pDBPage = GetTabPage(u"database");
    if( pDBPage )
    {
        static_cast<SwFieldDBPage*>(pDBPage)->ActivateMailMergeAddress();
    }
    //remove all other pages
    RemoveTabPage("document");
    RemoveTabPage("variables");
    RemoveTabPage("docinfo");
    RemoveTabPage("ref");
    RemoveTabPage("functions");
#endif
}

void SwFieldDlg::ShowReferencePage()
{
    ShowPage("ref");
}

void SwFieldDlg::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    if (rId != "database")
        return;

    SfxDispatcher* pDispatch = m_pBindings->GetDispatcher();
    SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : nullptr;
    if(pViewFrame)
    {
        SfxViewShell* pViewShell = SfxViewShell::GetFirst( true, checkSfxViewShell<SwView> );
        while(pViewShell && &pViewShell->GetViewFrame() != pViewFrame)
        {
            pViewShell = SfxViewShell::GetNext( *pViewShell, true, checkSfxViewShell<SwView> );
        }
        if(pViewShell)
            static_cast<SwFieldDBPage&>(rPage).SetWrtShell(static_cast<SwView*>(pViewShell)->GetWrtShell());
    }
#else
    (void) rId;
    (void) rPage;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
