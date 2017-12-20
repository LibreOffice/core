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
#include <sfx2/app.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/msgbox.hxx>
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

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <unomid.h>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

SwEnvPreview::SwEnvPreview(vcl::Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
{
    SetMapMode(MapMode(MapUnit::MapPixel));
}

Size SwEnvPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(84 , 63), MapMode(MapUnit::MapAppFont));
}

VCL_BUILDER_FACTORY_ARGS(SwEnvPreview, 0)

void SwEnvPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if (DataChangedEventType::SETTINGS == rDCEvt.GetType())
        Invalidate();
}

void SwEnvPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &)
{
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    SetBackground(rRenderContext.GetSettings().GetStyleSettings().GetDialogColor());

    const SwEnvItem& rItem = static_cast<SwEnvDlg*>(GetParentDialog())->aEnvItem;

    const long nPageW = std::max(rItem.m_nWidth, rItem.m_nHeight);
    const long nPageH = std::min(rItem.m_nWidth, rItem.m_nHeight);

    const double f = 0.8 * std::min(
        double(GetOutputSizePixel().Width()) / double(nPageW),
        double(GetOutputSizePixel().Height()) / double(nPageH));

    Color aBack = rSettings.GetWindowColor();
    Color aFront = SwViewOption::GetFontColor();
    Color aMedium = Color((aBack.GetRed() + aFront.GetRed()) / 2,
                          (aBack.GetGreen() + aFront.GetGreen()) / 2,
                          (aBack.GetBlue() + aFront.GetBlue()) / 2);

    rRenderContext.SetLineColor(aFront);

    // Envelope
    const long nW = static_cast<long>(f * nPageW);
    const long nH = static_cast<long>(f * nPageH);
    const long nX = (GetOutputSizePixel().Width () - nW) / 2;
    const long nY = (GetOutputSizePixel().Height() - nH) / 2;
    rRenderContext.SetFillColor(aBack);
    rRenderContext.DrawRect(tools::Rectangle(Point(nX, nY), Size(nW, nH)));

    // Sender
    if (rItem.m_bSend)
    {
        const long nSendX = nX + static_cast<long>(f * rItem.m_nSendFromLeft);
        const long nSendY = nY + static_cast<long>(f * rItem.m_nSendFromTop );
        const long nSendW = static_cast<long>(f * (rItem.m_nAddrFromLeft - rItem.m_nSendFromLeft));
        const long nSendH = static_cast<long>(f * (rItem.m_nAddrFromTop  - rItem.m_nSendFromTop  - 566));
        rRenderContext.SetFillColor(aMedium);

        rRenderContext.DrawRect(tools::Rectangle(Point(nSendX, nSendY), Size(nSendW, nSendH)));
    }

    // Addressee
    const long nAddrX = nX + static_cast<long>(f * rItem.m_nAddrFromLeft);
    const long nAddrY = nY + static_cast<long>(f * rItem.m_nAddrFromTop );
    const long nAddrW = static_cast<long>(f * (nPageW - rItem.m_nAddrFromLeft - 566));
    const long nAddrH = static_cast<long>(f * (nPageH - rItem.m_nAddrFromTop  - 566));
    rRenderContext.SetFillColor(aMedium);
    rRenderContext.DrawRect(tools::Rectangle(Point(nAddrX, nAddrY), Size(nAddrW, nAddrH)));

    // Stamp
    const long nStmpW = static_cast<long>(f * 1417 /* 2,5 cm */);
    const long nStmpH = static_cast<long>(f * 1701 /* 3,0 cm */);
    const long nStmpX = nX + nW - static_cast<long>(f * 566) - nStmpW;
    const long nStmpY = nY + static_cast<long>(f * 566);

    rRenderContext.SetFillColor(aBack);
    rRenderContext.DrawRect(tools::Rectangle(Point(nStmpX, nStmpY), Size(nStmpW, nStmpH)));
}

SwEnvDlg::SwEnvDlg(vcl::Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert)
    : SfxTabDialog(pParent, "EnvDialog",
        "modules/swriter/ui/envdialog.ui", &rSet)
    , aEnvItem(static_cast<const SwEnvItem&>( rSet.Get(FN_ENVELOP)))
    , pSh(pWrtSh)
    , pPrinter(pPrt)
    , pAddresseeSet(nullptr)
    , pSenderSet(nullptr)
    , m_nEnvPrintId(0)
{
    if (!bInsert)
    {
        GetUserButton()->SetText(get<PushButton>("modify")->GetText());
    }

    AddTabPage("envelope", SwEnvPage   ::Create, nullptr);
    AddTabPage("format", SwEnvFormatPage::Create, nullptr);
    m_nEnvPrintId = AddTabPage("printer", SwEnvPrtPage::Create, nullptr);
}

SwEnvDlg::~SwEnvDlg()
{
    disposeOnce();
}

void SwEnvDlg::dispose()
{
    delete pAddresseeSet;
    delete pSenderSet;
    pPrinter.clear();
    SfxTabDialog::dispose();
}

void SwEnvDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId == m_nEnvPrintId)
    {
        static_cast<SwEnvPrtPage*>(&rPage)->SetPrt(pPrinter);
    }
}

short SwEnvDlg::Ok()
{
    short nRet = SfxTabDialog::Ok();

    if (nRet == RET_OK || nRet == RET_USER)
    {
        if (pAddresseeSet)
        {
            SwTextFormatColl* pColl = pSh->GetTextCollFromPool(RES_POOLCOLL_JAKETADRESS);
            pColl->SetFormatAttr(*pAddresseeSet);
        }
        if (pSenderSet)
        {
            SwTextFormatColl* pColl = pSh->GetTextCollFromPool(RES_POOLCOLL_SENDADRESS);
            pColl->SetFormatAttr(*pSenderSet);
        }
    }

    return nRet;
}

SwEnvPage::SwEnvPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "EnvAddressPage",
        "modules/swriter/ui/envaddresspage.ui", &rSet)
{
    get(m_pAddrEdit, "addredit");
    get(m_pDatabaseLB, "database");
    get(m_pTableLB, "table");
    get(m_pDBFieldLB, "field");
    get(m_pInsertBT, "insert");
    get(m_pSenderBox, "sender");
    get(m_pSenderEdit, "senderedit");
    get(m_pPreview, "preview");

    long nTextBoxHeight(m_pAddrEdit->GetTextHeight() * 10);
    long nTextBoxWidth(m_pAddrEdit->approximate_char_width() * 25);

    m_pAddrEdit->set_height_request(nTextBoxHeight);
    m_pAddrEdit->set_width_request(nTextBoxWidth);
    m_pSenderEdit->set_height_request(nTextBoxHeight);
    m_pSenderEdit->set_width_request(nTextBoxWidth);

    long nListBoxWidth = approximate_char_width() * 30;
    m_pTableLB->set_width_request(nListBoxWidth);
    m_pDatabaseLB->set_width_request(nListBoxWidth);
    m_pDBFieldLB->set_width_request(nListBoxWidth);

    SetExchangeSupport();
    pSh = GetParentSwEnvDlg()->pSh;

    // Install handlers
    m_pDatabaseLB->SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    m_pTableLB->SetSelectHdl(LINK(this, SwEnvPage, DatabaseHdl     ));
    m_pInsertBT->SetClickHdl (LINK(this, SwEnvPage, FieldHdl        ));
    m_pSenderBox->SetClickHdl (LINK(this, SwEnvPage, SenderHdl       ));
    m_pPreview->SetBorderStyle( WindowBorderStyle::MONO );

    SwDBData aData = pSh->GetDBData();
    sActDBName = aData.sDataSource + OUStringLiteral1(DB_DELIM) + aData.sCommand;
    InitDatabaseBox();
}

SwEnvPage::~SwEnvPage()
{
    disposeOnce();
}

void SwEnvPage::dispose()
{
    m_pAddrEdit.clear();
    m_pDatabaseLB.clear();
    m_pTableLB.clear();
    m_pDBFieldLB.clear();
    m_pInsertBT.clear();
    m_pSenderBox.clear();
    m_pSenderEdit.clear();
    m_pPreview.clear();
    SfxTabPage::dispose();
}

IMPL_LINK( SwEnvPage, DatabaseHdl, ListBox&, rListBox, void )
{
    SwWait aWait( *pSh->GetView().GetDocShell(), true );

    if (&rListBox == m_pDatabaseLB)
    {
        sActDBName = rListBox.GetSelectedEntry();
        pSh->GetDBManager()->GetTableNames(m_pTableLB, sActDBName);
        sActDBName += OUStringLiteral1(DB_DELIM);
    }
    else
    {
        sActDBName = comphelper::string::setToken(sActDBName, 1, DB_DELIM, m_pTableLB->GetSelectedEntry());
    }
    pSh->GetDBManager()->GetColumnNames(m_pDBFieldLB, m_pDatabaseLB->GetSelectedEntry(),
                                       m_pTableLB->GetSelectedEntry());
}

IMPL_LINK_NOARG(SwEnvPage, FieldHdl, Button*, void)
{
    OUString aStr("<" + m_pDatabaseLB->GetSelectedEntry() + "." +
                  m_pTableLB->GetSelectedEntry() + "." +
                  OUString(m_pTableLB->GetSelectedEntryData() == nullptr ? '0' : '1') + "." +
                  m_pDBFieldLB->GetSelectedEntry() + ">");
    m_pAddrEdit->ReplaceSelected(aStr);
    Selection aSel = m_pAddrEdit->GetSelection();
    m_pAddrEdit->GrabFocus();
    m_pAddrEdit->SetSelection(aSel);
}

IMPL_LINK_NOARG(SwEnvPage, SenderHdl, Button*, void)
{
    const bool bEnable = m_pSenderBox->IsChecked();
    GetParentSwEnvDlg()->aEnvItem.m_bSend = bEnable;
    m_pSenderEdit->Enable(bEnable);
    if ( bEnable )
    {
        m_pSenderEdit->GrabFocus();
        if(m_pSenderEdit->GetText().isEmpty())
            m_pSenderEdit->SetText(MakeSender());
    }
    m_pPreview->Invalidate();
}

void SwEnvPage::InitDatabaseBox()
{
    if (pSh->GetDBManager())
    {
        m_pDatabaseLB->Clear();
        Sequence<OUString> aDataNames = SwDBManager::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();

        for (sal_Int32 i = 0; i < aDataNames.getLength(); i++)
            m_pDatabaseLB->InsertEntry(pDataNames[i]);

        OUString sDBName = sActDBName.getToken( 0, DB_DELIM );
        OUString sTableName = sActDBName.getToken( 1, DB_DELIM );
        m_pDatabaseLB->SelectEntry(sDBName);
        if (pSh->GetDBManager()->GetTableNames(m_pTableLB, sDBName))
        {
            m_pTableLB->SelectEntry(sTableName);
            pSh->GetDBManager()->GetColumnNames(m_pDBFieldLB, sDBName, sTableName);
        }
        else
            m_pDBFieldLB->Clear();

    }
}

VclPtr<SfxTabPage> SwEnvPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwEnvPage>::Create(pParent, *rSet);
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
    rItem.m_aAddrText = m_pAddrEdit->GetText();
    rItem.m_bSend     = m_pSenderBox->IsChecked();
    rItem.m_aSendText = m_pSenderEdit->GetText();
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
    m_pAddrEdit->SetText(convertLineEnd(aItem.m_aAddrText, GetSystemLineEnd()));
    m_pSenderEdit->SetText(convertLineEnd(aItem.m_aSendText, GetSystemLineEnd()));
    m_pSenderBox->Check  (aItem.m_bSend);
    m_pSenderBox->GetClickHdl().Call(m_pSenderBox);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
