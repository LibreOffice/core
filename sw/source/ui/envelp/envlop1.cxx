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

#include <dbmgr.hxx>
#include <tools/lineend.hxx>
#include <vcl/print.hxx>
#include <vcl/settings.hxx>

#include <swwait.hxx>
#include <viewopt.hxx>

#include <wrtsh.hxx>
#include <cmdid.h>
#include "envfmt.hxx"
#include <envlop.hxx>
#include "envprt.hxx"
#include <fmtcol.hxx>
#include <poolfmt.hxx>
#include <view.hxx>

#include <comphelper/string.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

SwEnvPreview::SwEnvPreview()
    : m_pDialog(nullptr)
{
}

void SwEnvPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 20,
                                   pDrawingArea->get_text_height() * 8);
}

void SwEnvPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetBackground(rRenderContext.GetSettings().GetStyleSettings().GetDialogColor());
    rRenderContext.Erase();

    const SwEnvItem& rItem = m_pDialog->aEnvItem;

    const tools::Long nPageW = std::max(rItem.m_nWidth, rItem.m_nHeight);
    const tools::Long nPageH = std::min(rItem.m_nWidth, rItem.m_nHeight);

    Size aSize(GetOutputSizePixel());

    const double f = 0.8 * std::min(
        double(aSize.Width()) / double(nPageW),
        double(aSize.Height()) / double(nPageH));

    Color aBack = rSettings.GetWindowColor();
    Color aFront = SwViewOption::GetFontColor();
    Color aMedium((aBack.GetRed() + aFront.GetRed()) / 2,
                  (aBack.GetGreen() + aFront.GetGreen()) / 2,
                  (aBack.GetBlue() + aFront.GetBlue()) / 2);

    rRenderContext.SetLineColor(aFront);

    // Envelope
    const tools::Long nW = static_cast<tools::Long>(f * nPageW);
    const tools::Long nH = static_cast<tools::Long>(f * nPageH);
    const tools::Long nX = (aSize.Width () - nW) / 2;
    const tools::Long nY = (aSize.Height() - nH) / 2;
    rRenderContext.SetFillColor(aBack);
    rRenderContext.DrawRect(tools::Rectangle(Point(nX, nY), Size(nW, nH)));

    // Sender
    if (rItem.m_bSend)
    {
        const tools::Long nSendX = nX + static_cast<tools::Long>(f * rItem.m_nSendFromLeft);
        const tools::Long nSendY = nY + static_cast<tools::Long>(f * rItem.m_nSendFromTop );
        const tools::Long nSendW = static_cast<tools::Long>(f * (rItem.m_nAddrFromLeft - rItem.m_nSendFromLeft));
        const tools::Long nSendH = static_cast<tools::Long>(f * (rItem.m_nAddrFromTop  - rItem.m_nSendFromTop  - 566));
        rRenderContext.SetFillColor(aMedium);

        rRenderContext.DrawRect(tools::Rectangle(Point(nSendX, nSendY), Size(nSendW, nSendH)));
    }

    // Addressee
    const tools::Long nAddrX = nX + static_cast<tools::Long>(f * rItem.m_nAddrFromLeft);
    const tools::Long nAddrY = nY + static_cast<tools::Long>(f * rItem.m_nAddrFromTop );
    const tools::Long nAddrW = static_cast<tools::Long>(f * (nPageW - rItem.m_nAddrFromLeft - 566));
    const tools::Long nAddrH = static_cast<tools::Long>(f * (nPageH - rItem.m_nAddrFromTop  - 566));
    rRenderContext.SetFillColor(aMedium);
    rRenderContext.DrawRect(tools::Rectangle(Point(nAddrX, nAddrY), Size(nAddrW, nAddrH)));

    // Stamp
    const tools::Long nStmpW = static_cast<tools::Long>(f * 1417 /* 2,5 cm */);
    const tools::Long nStmpH = static_cast<tools::Long>(f * 1701 /* 3,0 cm */);
    const tools::Long nStmpX = nX + nW - static_cast<tools::Long>(f * 566) - nStmpW;
    const tools::Long nStmpY = nY + static_cast<tools::Long>(f * 566);

    rRenderContext.SetFillColor(aBack);
    rRenderContext.DrawRect(tools::Rectangle(Point(nStmpX, nStmpY), Size(nStmpW, nStmpH)));
}

SwEnvDlg::SwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert)
    : SfxTabDialogController(pParent, "modules/swriter/ui/envdialog.ui", "EnvDialog", &rSet)
    , aEnvItem(static_cast<const SwEnvItem&>( rSet.Get(FN_ENVELOP)))
    , pSh(pWrtSh)
    , pPrinter(pPrt)
    , m_xModify(m_xBuilder->weld_button("modify"))
{
    if (!bInsert)
    {
        GetUserButton()->set_label(m_xModify->get_label());
    }

    AddTabPage("envelope", SwEnvPage::Create, nullptr);
    AddTabPage("format", SwEnvFormatPage::Create, nullptr);
    AddTabPage("printer", SwEnvPrtPage::Create, nullptr);
}

SwEnvDlg::~SwEnvDlg()
{
    pAddresseeSet.reset();
    pSenderSet.reset();
}

void SwEnvDlg::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "printer")
    {
        static_cast<SwEnvPrtPage*>(&rPage)->SetPrt(pPrinter);
    }
    else if (rId == "envelope")
    {
        static_cast<SwEnvPage*>(&rPage)->Init(this);
    }
    else if (rId == "format")
    {
        static_cast<SwEnvFormatPage*>(&rPage)->Init(this);
    }
}

short SwEnvDlg::Ok()
{
    short nRet = SfxTabDialogController::Ok();

    if (nRet == RET_OK || nRet == RET_USER)
    {
        if (pAddresseeSet)
        {
            SwTextFormatColl* pColl = pSh->GetTextCollFromPool(RES_POOLCOLL_ENVELOPE_ADDRESS);
            pColl->SetFormatAttr(*pAddresseeSet);
        }
        if (pSenderSet)
        {
            SwTextFormatColl* pColl = pSh->GetTextCollFromPool(RES_POOLCOLL_SEND_ADDRESS);
            pColl->SetFormatAttr(*pSenderSet);
        }
    }

    return nRet;
}

SwEnvPage::SwEnvPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/envaddresspage.ui", "EnvAddressPage", &rSet)
    , m_pDialog(nullptr)
    , m_pSh(nullptr)
    , m_xAddrEdit(m_xBuilder->weld_text_view("addredit"))
    , m_xDatabaseLB(m_xBuilder->weld_combo_box("database"))
    , m_xTableLB(m_xBuilder->weld_combo_box("table"))
    , m_xDBFieldLB(m_xBuilder->weld_combo_box("field"))
    , m_xInsertBT(m_xBuilder->weld_button("insert"))
    , m_xSenderBox(m_xBuilder->weld_check_button("sender"))
    , m_xSenderEdit(m_xBuilder->weld_text_view("senderedit"))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreview))
{
    auto nTextBoxHeight(m_xAddrEdit->get_height_rows(10));
    auto nTextBoxWidth(m_xAddrEdit->get_approximate_digit_width() * 25);

    m_xAddrEdit->set_size_request(nTextBoxWidth, nTextBoxHeight);
    m_xSenderEdit->set_size_request(nTextBoxWidth, nTextBoxHeight);

    auto nListBoxWidth = m_xTableLB->get_approximate_digit_width() * 25;
    m_xTableLB->set_size_request(nListBoxWidth, -1);
    m_xDatabaseLB->set_size_request(nListBoxWidth, -1);
    m_xDBFieldLB->set_size_request(nListBoxWidth, -1);

    SetExchangeSupport();
}

void SwEnvPage::Init(SwEnvDlg* pDialog)
{
    m_pDialog = pDialog;
    m_pSh = m_pDialog->pSh;
    m_aPreview.SetDialog(pDialog);

    // Install handlers
    m_xDatabaseLB->connect_changed(LINK(this, SwEnvPage, DatabaseHdl));
    m_xTableLB->connect_changed(LINK(this, SwEnvPage, DatabaseHdl));
    m_xInsertBT->connect_clicked(LINK(this, SwEnvPage, FieldHdl));
    m_xSenderBox->connect_clicked(LINK(this, SwEnvPage, SenderHdl));

    SwDBData aData = m_pSh->GetDBData();
    m_sActDBName = aData.sDataSource + OUStringChar(DB_DELIM) + aData.sCommand;
    InitDatabaseBox();
}

SwEnvPage::~SwEnvPage()
{
}

IMPL_LINK( SwEnvPage, DatabaseHdl, weld::ComboBox&, rListBox, void )
{
    SwWait aWait( *m_pSh->GetView().GetDocShell(), true );

    if (&rListBox == m_xDatabaseLB.get())
    {
        m_sActDBName = rListBox.get_active_text();
        m_pSh->GetDBManager()->GetTableNames(*m_xTableLB, m_sActDBName);
        m_sActDBName += OUStringChar(DB_DELIM);
    }
    else
    {
        m_sActDBName = comphelper::string::setToken(m_sActDBName, 1, DB_DELIM, m_xTableLB->get_active_text());
    }
    m_pSh->GetDBManager()->GetColumnNames(*m_xDBFieldLB, m_xDatabaseLB->get_active_text(),
                                          m_xTableLB->get_active_text());
}

IMPL_LINK_NOARG(SwEnvPage, FieldHdl, weld::Button&, void)
{
    OUString aStr("<" + m_xDatabaseLB->get_active_text() + "." +
                  m_xTableLB->get_active_text() + "." +
                  m_xTableLB->get_active_id() + "." +
                  m_xDBFieldLB->get_active_text() + ">");
    m_xAddrEdit->replace_selection(aStr);
    int nStartPos, nEndPos;
    m_xAddrEdit->get_selection_bounds(nStartPos, nEndPos);
    m_xAddrEdit->grab_focus();
    m_xAddrEdit->select_region(nStartPos, nEndPos);
}

IMPL_LINK_NOARG(SwEnvPage, SenderHdl, weld::Button&, void)
{
    const bool bEnable = m_xSenderBox->get_active();
    GetParentSwEnvDlg()->aEnvItem.m_bSend = bEnable;
    m_xSenderEdit->set_sensitive(bEnable);
    if (bEnable)
    {
        m_xSenderEdit->grab_focus();
        if (m_xSenderEdit->get_text().isEmpty())
            m_xSenderEdit->set_text(MakeSender());
    }
    m_xPreview->queue_draw();
}

void SwEnvPage::InitDatabaseBox()
{
    if (!m_pSh->GetDBManager())
        return;

    m_xDatabaseLB->clear();
    const Sequence<OUString> aDataNames = SwDBManager::GetExistingDatabaseNames();

    for (const OUString& rDataName : aDataNames)
        m_xDatabaseLB->append_text(rDataName);

    sal_Int32 nIdx{ 0 };
    OUString sDBName = m_sActDBName.getToken( 0, DB_DELIM, nIdx );
    OUString sTableName = m_sActDBName.getToken( 0, DB_DELIM, nIdx );
    m_xDatabaseLB->set_active_text(sDBName);
    if (m_pSh->GetDBManager()->GetTableNames(*m_xTableLB, sDBName))
    {
        m_xTableLB->append_text(sTableName);
        m_pSh->GetDBManager()->GetColumnNames(*m_xDBFieldLB, sDBName, sTableName);
    }
    else
        m_xDBFieldLB->clear();
}

std::unique_ptr<SfxTabPage> SwEnvPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwEnvPage>(pPage, pController, *rSet);
}

void SwEnvPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParentSwEnvDlg()->aEnvItem);
    Reset(&aSet);
}

DeactivateRC SwEnvPage::DeactivatePage(SfxItemSet* _pSet)
{
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    if( _pSet )
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;
}

void SwEnvPage::FillItem(SwEnvItem& rItem)
{
    rItem.m_aAddrText = m_xAddrEdit->get_text();
    rItem.m_bSend     = m_xSenderBox->get_active();
    rItem.m_aSendText = m_xSenderEdit->get_text();
}

bool SwEnvPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    rSet->Put(GetParentSwEnvDlg()->aEnvItem);
    return true;
}

void SwEnvPage::Reset(const SfxItemSet* rSet)
{
    SwEnvItem aItem = static_cast<const SwEnvItem&>( rSet->Get(FN_ENVELOP));
    m_xAddrEdit->set_text(convertLineEnd(aItem.m_aAddrText, GetSystemLineEnd()));
    m_xSenderEdit->set_text(convertLineEnd(aItem.m_aSendText, GetSystemLineEnd()));
    m_xSenderBox->set_active(aItem.m_bSend);
    SenderHdl(*m_xSenderBox);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
