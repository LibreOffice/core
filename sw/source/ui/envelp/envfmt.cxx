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

#include <hintids.hxx>

#include <editeng/paperinf.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svtools/unitconv.hxx>
#include <svx/drawitem.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>

#include <cmdid.h>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include "envfmt.hxx"
#include <fmtcol.hxx>
#include <swuipardlg.hxx>
#include <chrdlgmodes.hxx>
#include <pardlg.hxx>
#include <poolfmt.hxx>
#include <uitool.hxx>

#include <vector>
#include <algorithm>

#include <memory>

#include <swabstdlg.hxx>
#include <swuiexp.hxx>

static tools::Long lUserW = 5669; // 10 cm
static tools::Long lUserH = 5669; // 10 cm

SwEnvFormatPage::SwEnvFormatPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/envformatpage.ui"_ustr, u"EnvFormatPage"_ustr, &rSet)
    , m_pDialog(nullptr)
    , m_xAddrLeftField(m_xBuilder->weld_metric_spin_button(u"leftaddr"_ustr, FieldUnit::CM))
    , m_xAddrTopField(m_xBuilder->weld_metric_spin_button(u"topaddr"_ustr, FieldUnit::CM))
    , m_xAddrEditButton(m_xBuilder->weld_menu_button(u"addredit"_ustr))
    , m_xSendLeftField(m_xBuilder->weld_metric_spin_button(u"leftsender"_ustr, FieldUnit::CM))
    , m_xSendTopField(m_xBuilder->weld_metric_spin_button(u"topsender"_ustr, FieldUnit::CM))
    , m_xSendEditButton(m_xBuilder->weld_menu_button(u"senderedit"_ustr))
    , m_xSizeFormatBox(m_xBuilder->weld_combo_box(u"format"_ustr))
    , m_xSizeWidthField(m_xBuilder->weld_metric_spin_button(u"width"_ustr, FieldUnit::CM))
    , m_xSizeHeightField(m_xBuilder->weld_metric_spin_button(u"height"_ustr, FieldUnit::CM))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreview))
{
    SetExchangeSupport();

    // Metrics
    FieldUnit aMetric = ::GetDfltMetric(false);
    ::SetFieldUnit(*m_xAddrLeftField, aMetric);
    ::SetFieldUnit(*m_xAddrTopField, aMetric);
    ::SetFieldUnit(*m_xSendLeftField, aMetric);
    ::SetFieldUnit(*m_xSendTopField, aMetric);
    ::SetFieldUnit(*m_xSizeWidthField, aMetric);
    ::SetFieldUnit(*m_xSizeHeightField, aMetric);

    // Install handlers
    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwEnvFormatPage, ModifyHdl);
    m_xAddrLeftField->connect_value_changed( aLk );
    m_xAddrTopField->connect_value_changed( aLk );
    m_xSendLeftField->connect_value_changed( aLk );
    m_xSendTopField->connect_value_changed( aLk );
    m_xSizeWidthField->connect_value_changed( aLk );
    m_xSizeHeightField->connect_value_changed( aLk );

    m_xAddrEditButton->connect_selected(LINK(this, SwEnvFormatPage, AddrEditHdl));
    m_xSendEditButton->connect_selected(LINK(this, SwEnvFormatPage, SendEditHdl));

    m_xSizeFormatBox->connect_changed(LINK(this, SwEnvFormatPage, FormatHdl));

    // m_xSizeFormatBox
    for (sal_uInt16 i = PAPER_A3; i <= PAPER_KAI32BIG; i++)
    {
        if (i != PAPER_USER)
        {
            const OUString aPaperName = SvxPaperInfo::GetName(static_cast<Paper>(i));

            if (aPaperName.isEmpty())
                continue;

            sal_Int32 nPos = 0;
            while (nPos < m_xSizeFormatBox->get_count() &&
                   m_xSizeFormatBox->get_text(nPos) < aPaperName)
            {
                ++nPos;
            }
            m_xSizeFormatBox->insert_text(nPos, aPaperName);
            m_aIDs.insert( m_aIDs.begin() + nPos, i);
        }
    }
    m_xSizeFormatBox->append_text(SvxPaperInfo::GetName(PAPER_USER));
    m_aIDs.push_back( sal_uInt16(PAPER_USER) );
}

void SwEnvFormatPage::Init(SwEnvDlg* pDialog)
{
    m_pDialog = pDialog;
    m_aPreview.SetDialog(m_pDialog);
}

SwEnvFormatPage::~SwEnvFormatPage()
{
}

IMPL_LINK( SwEnvFormatPage, ModifyHdl, weld::MetricSpinButton&, rEdit, void )
{
    int lWVal = getfieldval(*m_xSizeWidthField);
    int lHVal = getfieldval(*m_xSizeHeightField);

    int lWidth  = std::max(lWVal, lHVal);
    int lHeight = std::min(lWVal, lHVal);

    if (&rEdit == m_xSizeWidthField.get() || &rEdit == m_xSizeHeightField.get())
    {
        int nRotatedWidth = lHeight;
        int nRotatedHeight = lWidth;
        Paper ePaper = SvxPaperInfo::GetSvxPaper(
            Size(nRotatedWidth, nRotatedHeight), MapUnit::MapTwip);
        for (size_t i = 0; i < m_aIDs.size(); ++i)
            if (m_aIDs[i] == o3tl::narrowing<sal_uInt16>(ePaper))
                m_xSizeFormatBox->set_active(i);

        // remember user size
        if (m_aIDs[m_xSizeFormatBox->get_active()] == sal_uInt16(PAPER_USER))
        {
            lUserW = lWidth ;
            lUserH = lHeight;
        }

        FormatHdl(*m_xSizeFormatBox);
    }
    else
    {
        FillItem(GetParentSwEnvDlg()->m_aEnvItem);
        SetMinMax();
        m_xPreview->queue_draw();
    }
}

IMPL_LINK(SwEnvFormatPage, AddrEditHdl, const OUString&, rIdent, void)
{
    Edit(rIdent, false);
}

IMPL_LINK(SwEnvFormatPage, SendEditHdl, const OUString&, rIdent, void)
{
    Edit(rIdent, true);
}

void SwEnvFormatPage::Edit(std::u16string_view rIdent, bool bSender)
{
    SwWrtShell* pSh = GetParentSwEnvDlg()->m_pSh;
    assert(pSh && "Shell missing");

    SwTextFormatColl* pColl = pSh->GetTextCollFromPool( static_cast< sal_uInt16 >(
        bSender ? RES_POOLCOLL_SEND_ADDRESS : RES_POOLCOLL_ENVELOPE_ADDRESS));
    assert(pColl && "Text collection missing");

    if (o3tl::starts_with(rIdent, u"character"))
    {
        SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

        // In order for the background color not to get ironed over:
        auto xTmpSet = std::make_shared<SfxAllItemSet>(*pCollSet);
        ::ConvertAttrCharToGen(*xTmpSet);

        SwAbstractDialogFactory& rFact = swui::GetFactory();

        const OUString sFormatStr = pColl->GetName();
        VclPtr<SfxAbstractTabDialog> pDlg(rFact.CreateSwCharDlg(GetFrameWeld(), pSh->GetView(), *xTmpSet, SwCharDlgMode::Env, &sFormatStr));
        pDlg->StartExecuteAsync(
            [pDlg, xTmpSet, pCollSet] (sal_Int32 nResult)->void
            {
                if (nResult == RET_OK)
                {
                    SfxItemSet aOutputSet( *pDlg->GetOutputItemSet() );
                    ::ConvertAttrGenToChar(aOutputSet, *xTmpSet);
                    pCollSet->Put(aOutputSet);
                }
                pDlg->disposeOnce();
            }
        );
    }
    else if (o3tl::starts_with(rIdent, u"paragraph"))
    {
        SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

        // In order for the tabulators not to get ironed over:
        SfxAllItemSet aTmpSet(*pCollSet);

        // Insert tabs, default tabs into ItemSet
        const SvxTabStopItem& rDefTabs =
            pSh->GetView().GetCurShell()->GetPool().GetUserOrPoolDefaultItem(RES_PARATR_TABSTOP);

        const sal_uInt16 nDefDist = o3tl::narrowing<sal_uInt16>(::GetTabDist( rDefTabs ));
        SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
        aTmpSet.Put( aDefDistItem );

        // Current tab
        SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
        aTmpSet.Put( aTabPos );

        // left border as offset
        const tools::Long nOff = aTmpSet.Get(RES_MARGIN_TEXTLEFT).GetTextLeft();
        SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
        aTmpSet.Put( aOff );

        // set BoxInfo
        ::PrepareBoxInfo( aTmpSet, *pSh );

        SwDrawModel* pDrawModel = pSh->GetView().GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
        aTmpSet.Put(SvxColorListItem(pDrawModel->GetColorList(), SID_COLOR_TABLE));
        aTmpSet.Put(SvxGradientListItem(pDrawModel->GetGradientList(), SID_GRADIENT_LIST));
        aTmpSet.Put(SvxHatchListItem(pDrawModel->GetHatchList(), SID_HATCH_LIST));
        aTmpSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapList(), SID_BITMAP_LIST));
        aTmpSet.Put(SvxPatternListItem(pDrawModel->GetPatternList(), SID_PATTERN_LIST));

        const OUString sFormatStr = pColl->GetName();
        SwParaDlg aDlg(GetFrameWeld(), pSh->GetView(), aTmpSet, DLG_ENVELOP, &sFormatStr);

        if (aDlg.run() == RET_OK)
        {
            // maybe relocate defaults
            const SfxUInt16Item* pDefaultsItem = nullptr;
            SfxItemSet* pOutputSet = const_cast<SfxItemSet*>(aDlg.GetOutputItemSet());
            sal_uInt16 nNewDist;

            if( (pDefaultsItem = pOutputSet->GetItemIfSet( SID_ATTR_TABSTOP_DEFAULTS, false )) &&
                nDefDist != (nNewDist = pDefaultsItem->GetValue()) )
            {
                SvxTabStopItem aDefTabs( 0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP );
                MakeDefTabs( nNewDist, aDefTabs );
                pSh->SetDefault( aDefTabs );
                pOutputSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
            }
            if( pOutputSet->Count() )
            {
                pCollSet->Put(*pOutputSet);
            }
        }
    }
}

// A temporary Itemset that gets discarded at abort
SfxItemSet *SwEnvFormatPage::GetCollItemSet(SwTextFormatColl const * pColl, bool bSender)
{
    std::unique_ptr<SfxItemSet>& pAddrSet = bSender ? GetParentSwEnvDlg()->m_pSenderSet : GetParentSwEnvDlg()->m_pAddresseeSet;
    if (!pAddrSet)
    {
        // determine range (merge both Itemsets' ranges)
        const WhichRangesContainer& pRanges = pColl->GetAttrSet().GetRanges();

        static WhichRangesContainer const aRanges(svl::Items<
            RES_PARATR_BEGIN, RES_PARATR_ADJUST,
            RES_PARATR_TABSTOP, RES_PARATR_END-1,
            RES_MARGIN_FIRSTLINE, RES_MARGIN_RIGHT,
            RES_UL_SPACE, RES_UL_SPACE,
            RES_BACKGROUND, RES_SHADOW,
            SID_ATTR_TABSTOP_DEFAULTS, SID_ATTR_TABSTOP_DEFAULTS,
            SID_ATTR_TABSTOP_POS, SID_ATTR_TABSTOP_POS,
            SID_ATTR_TABSTOP_OFFSET, SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER
        >);

        pAddrSet.reset(new SfxItemSet(GetParentSwEnvDlg()->m_pSh->GetView().GetCurShell()->GetPool(),
                                  pRanges));
        for (const auto& rPair : aRanges)
            pAddrSet->MergeRange(rPair.first, rPair.second);
        pAddrSet->Put(pColl->GetAttrSet());
    }

    return pAddrSet.get();
}

IMPL_LINK_NOARG(SwEnvFormatPage, FormatHdl, weld::ComboBox&, void)
{
    tools::Long lWidth;
    tools::Long lHeight;
    tools::Long lSendFromLeft;
    tools::Long lSendFromTop;
    tools::Long lAddrFromLeft;
    tools::Long lAddrFromTop;

    const sal_uInt16 nPaper = m_aIDs[m_xSizeFormatBox->get_active()];
    if (nPaper != sal_uInt16(PAPER_USER))
    {
        Size aSz = SvxPaperInfo::GetPaperSize(static_cast<Paper>(nPaper));
        lWidth  = std::max(aSz.Width(), aSz.Height());
        lHeight = std::min(aSz.Width(), aSz.Height());
    }
    else
    {
        lWidth  = lUserW;
        lHeight = lUserH;
    }

    lSendFromLeft = 566;            // 1cm
    lSendFromTop  = 566;            // 1cm
    lAddrFromLeft = lWidth  / 2;
    lAddrFromTop  = lHeight / 2;

    setfieldval(*m_xAddrLeftField, lAddrFromLeft);
    setfieldval(*m_xAddrTopField , lAddrFromTop );
    setfieldval(*m_xSendLeftField, lSendFromLeft);
    setfieldval(*m_xSendTopField , lSendFromTop );

    setfieldval(*m_xSizeWidthField , lWidth );
    setfieldval(*m_xSizeHeightField, lHeight);

    SetMinMax();

    FillItem(GetParentSwEnvDlg()->m_aEnvItem);
    m_xPreview->queue_draw();
}

void SwEnvFormatPage::SetMinMax()
{
    tools::Long lWVal = static_cast< tools::Long >(getfieldval(*m_xSizeWidthField ));
    tools::Long lHVal = static_cast< tools::Long >(getfieldval(*m_xSizeHeightField));

    tools::Long lWidth  = std::max(lWVal, lHVal),
         lHeight = std::min(lWVal, lHVal);

    // Min and Max
    m_xAddrLeftField->set_range(100 * (getfieldval(*m_xSendLeftField) + 566),
                                100 * (lWidth  - 2 * 566), FieldUnit::TWIP);
    m_xAddrTopField->set_range(100 * (getfieldval(*m_xSendTopField ) + 2 * 566),
                               100 * (lHeight - 2 * 566), FieldUnit::TWIP);
    m_xSendLeftField->set_range(100 * 566,
                                100 * (getfieldval(*m_xAddrLeftField) - 566), FieldUnit::TWIP);
    m_xSendTopField->set_range(100 * 566,
                               100 * (getfieldval(*m_xAddrTopField ) - 2 * 566), FieldUnit::TWIP);
}

std::unique_ptr<SfxTabPage> SwEnvFormatPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwEnvFormatPage>(pPage, pController, *rSet);
}

void SwEnvFormatPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParentSwEnvDlg()->m_aEnvItem);
    Reset(&aSet);
}

DeactivateRC SwEnvFormatPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;
}

void SwEnvFormatPage::FillItem(SwEnvItem& rItem)
{
    rItem.m_nAddrFromLeft = static_cast< sal_Int32 >(getfieldval(*m_xAddrLeftField));
    rItem.m_nAddrFromTop  = static_cast< sal_Int32 >(getfieldval(*m_xAddrTopField ));
    rItem.m_nSendFromLeft = static_cast< sal_Int32 >(getfieldval(*m_xSendLeftField));
    rItem.m_nSendFromTop  = static_cast< sal_Int32 >(getfieldval(*m_xSendTopField ));

    const sal_uInt16 nPaper = m_aIDs[m_xSizeFormatBox->get_active()];
    if (nPaper == sal_uInt16(PAPER_USER))
    {
        tools::Long lWVal = static_cast< tools::Long >(getfieldval(*m_xSizeWidthField ));
        tools::Long lHVal = static_cast< tools::Long >(getfieldval(*m_xSizeHeightField));
        rItem.m_nWidth  = std::max(lWVal, lHVal);
        rItem.m_nHeight = std::min(lWVal, lHVal);
    }
    else
    {
        tools::Long lWVal = SvxPaperInfo::GetPaperSize(static_cast<Paper>(nPaper)).Width ();
        tools::Long lHVal = SvxPaperInfo::GetPaperSize(static_cast<Paper>(nPaper)).Height();
        rItem.m_nWidth  = std::max(lWVal, lHVal);
        rItem.m_nHeight = std::min(lWVal, lHVal);
    }
}

bool SwEnvFormatPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem(GetParentSwEnvDlg()->m_aEnvItem);
    rSet->Put(GetParentSwEnvDlg()->m_aEnvItem);
    return true;
}

void SwEnvFormatPage::Reset(const SfxItemSet* rSet)
{
    const SwEnvItem& rItem = static_cast<const SwEnvItem&>( rSet->Get(FN_ENVELOP));

    Paper ePaper = SvxPaperInfo::GetSvxPaper(
        Size( std::min(rItem.m_nWidth, rItem.m_nHeight),
        std::max(rItem.m_nWidth, rItem.m_nHeight)), MapUnit::MapTwip);
    for (size_t i = 0; i < m_aIDs.size(); ++i)
        if (m_aIDs[i] == o3tl::narrowing<sal_uInt16>(ePaper))
            m_xSizeFormatBox->set_active(i);

    // Metric fields
    setfieldval(*m_xAddrLeftField, rItem.m_nAddrFromLeft);
    setfieldval(*m_xAddrTopField, rItem.m_nAddrFromTop );
    setfieldval(*m_xSendLeftField, rItem.m_nSendFromLeft);
    setfieldval(*m_xSendTopField, rItem.m_nSendFromTop );
    setfieldval(*m_xSizeWidthField  , std::max(rItem.m_nWidth, rItem.m_nHeight));
    setfieldval(*m_xSizeHeightField , std::min(rItem.m_nWidth, rItem.m_nHeight));
    SetMinMax();

    GetParentSwEnvDlg()->m_pSenderSet.reset();
    GetParentSwEnvDlg()->m_pAddresseeSet.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
