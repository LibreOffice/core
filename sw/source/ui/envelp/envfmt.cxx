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
#include <osl/diagnose.h>

#include <cmdid.h>
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

namespace {
    /// Converts a ranges array to a list containing one entry for each
    /// element covered by the ranges.
    /// @param aRanges An array containing zero or more range specifications and
    ///                terminated by one or more zero entries. A range
    ///                specification is two consecutive entries that specify
    ///                the start and end points of the range.
    /// @returns A vector containing one element for each item covered by the
    ///          ranges. This is not guaranteed to be sorted and may contain
    ///          duplicates if the original ranges contained overlaps.
    std::vector<sal_uInt16> lcl_convertRangesToList(const sal_uInt16 aRanges[]) {
        std::vector<sal_uInt16> aVec;
        int i = 0;
        while (aRanges[i])
        {
            for (sal_uInt16 n = aRanges[i]; n <= aRanges[i+1]; ++n)
            {
                aVec.push_back(n);
            }
            i += 2;
        }
        return aVec;
    }

    /// Converts a list of elements to a ranges array.
    /// @param rElements Vector of the initial elements, this need not be sorted,
    ///                  and may contain duplicate items. The vector is sorted
    ///                  on exit from this function but may still contain duplicates.
    /// @returns An array containing zero or more range specifications and
    ///          terminated by one or more zero entries. A range specification
    ///          is two consecutive entries that specify the start and end
    ///          points of the range. This list will be sorted and will not
    ///          contain any overlapping ranges.
    sal_uInt16* lcl_convertListToRanges(std::vector<sal_uInt16> &rElements) {
        std::sort(rElements.begin(), rElements.end());
        std::vector<sal_uInt16> aRanges;
        size_t i;
        for (i = 0; i < rElements.size(); ++i)
        {
            //Push the start of the this range.
            aRanges.push_back(rElements[i]);
            //Seek to the end of this range.
            while (i + 1 < rElements.size() && rElements[i+1] - rElements[i] <= 1)
            {
                ++i;
            }
            //Push the end of this range (may be the same as the start).
            aRanges.push_back( rElements[i] );
        }

        // Convert the vector to an array with terminating zero
        sal_uInt16 *pNewRanges = new sal_uInt16[aRanges.size() + 1];
        for (i = 0; i < aRanges.size(); ++i)
        {
            pNewRanges[i] = aRanges[i];
        }
        pNewRanges[i] = 0;
        return pNewRanges;
    }

}

static tools::Long lUserW = 5669; // 10 cm
static tools::Long lUserH = 5669; // 10 cm

SwEnvFormatPage::SwEnvFormatPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/envformatpage.ui", "EnvFormatPage", &rSet)
    , m_pDialog(nullptr)
    , m_xAddrLeftField(m_xBuilder->weld_metric_spin_button("leftaddr", FieldUnit::CM))
    , m_xAddrTopField(m_xBuilder->weld_metric_spin_button("topaddr", FieldUnit::CM))
    , m_xAddrEditButton(m_xBuilder->weld_menu_button("addredit"))
    , m_xSendLeftField(m_xBuilder->weld_metric_spin_button("leftsender", FieldUnit::CM))
    , m_xSendTopField(m_xBuilder->weld_metric_spin_button("topsender", FieldUnit::CM))
    , m_xSendEditButton(m_xBuilder->weld_menu_button("senderedit"))
    , m_xSizeFormatBox(m_xBuilder->weld_combo_box("format"))
    , m_xSizeWidthField(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM))
    , m_xSizeHeightField(m_xBuilder->weld_metric_spin_button("height", FieldUnit::CM))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreview))
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
            if (m_aIDs[i] == static_cast<sal_uInt16>(ePaper))
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
        FillItem(GetParentSwEnvDlg()->aEnvItem);
        SetMinMax();
        m_xPreview->queue_draw();
    }
}

IMPL_LINK(SwEnvFormatPage, AddrEditHdl, const OString&, rIdent, void)
{
    Edit(rIdent, false);
}

IMPL_LINK(SwEnvFormatPage, SendEditHdl, const OString&, rIdent, void)
{
    Edit(rIdent, true);
}

void SwEnvFormatPage::Edit(const OString& rIdent, bool bSender)
{
    SwWrtShell* pSh = GetParentSwEnvDlg()->pSh;
    OSL_ENSURE(pSh, "Shell missing");

    SwTextFormatColl* pColl = pSh->GetTextCollFromPool( static_cast< sal_uInt16 >(
        bSender ? RES_POOLCOLL_SEND_ADDRESS : RES_POOLCOLL_ENVELOPE_ADDRESS));
    OSL_ENSURE(pColl, "Text collection missing");

    if (rIdent.startsWith("character"))
    {
        SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

        // In order for the background color not to get ironed over:
        SfxAllItemSet aTmpSet(*pCollSet);
        ::ConvertAttrCharToGen(aTmpSet);

        SwAbstractDialogFactory& rFact = swui::GetFactory();

        const OUString sFormatStr = pColl->GetName();
        ScopedVclPtr<SfxAbstractTabDialog> pDlg(rFact.CreateSwCharDlg(GetFrameWeld(), pSh->GetView(), aTmpSet, SwCharDlgMode::Env, &sFormatStr));
        if (pDlg->Execute() == RET_OK)
        {
            SfxItemSet aOutputSet( *pDlg->GetOutputItemSet() );
            ::ConvertAttrGenToChar(aOutputSet, aTmpSet);
            pCollSet->Put(aOutputSet);
        }
    }
    else if (rIdent.startsWith("paragraph"))
    {
        SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

        // In order for the tabulators not to get ironed over:
        SfxAllItemSet aTmpSet(*pCollSet);

        // Insert tabs, default tabs into ItemSet
        const SvxTabStopItem& rDefTabs =
            pSh->GetView().GetCurShell()->GetPool().GetDefaultItem(RES_PARATR_TABSTOP);

        const sal_uInt16 nDefDist = static_cast<sal_uInt16>(::GetTabDist( rDefTabs ));
        SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
        aTmpSet.Put( aDefDistItem );

        // Current tab
        SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
        aTmpSet.Put( aTabPos );

        // left border as offset
        const tools::Long nOff = aTmpSet.Get( RES_LR_SPACE ).GetTextLeft();
        SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
        aTmpSet.Put( aOff );

        // set BoxInfo
        ::PrepareBoxInfo( aTmpSet, *pSh );

        const OUString sFormatStr = pColl->GetName();
        SwParaDlg aDlg(GetFrameWeld(), pSh->GetView(), aTmpSet, DLG_ENVELOP, &sFormatStr);

        if (aDlg.run() == RET_OK)
        {
            // maybe relocate defaults
            const SfxPoolItem* pItem = nullptr;
            SfxItemSet* pOutputSet = const_cast<SfxItemSet*>(aDlg.GetOutputItemSet());
            sal_uInt16 nNewDist;

            if( SfxItemState::SET == pOutputSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS,
                false, &pItem ) &&
                nDefDist != (nNewDist = static_cast<const SfxUInt16Item*>(pItem)->GetValue()) )
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
    std::unique_ptr<SfxItemSet>& pAddrSet = bSender ? GetParentSwEnvDlg()->pSenderSet : GetParentSwEnvDlg()->pAddresseeSet;
    if (!pAddrSet)
    {
        // determine range (merge both Itemsets' ranges)
        const sal_uInt16 *pRanges = pColl->GetAttrSet().GetRanges();

        static sal_uInt16 const aRanges[] =
        {
            RES_PARATR_BEGIN, RES_PARATR_ADJUST,
            RES_PARATR_TABSTOP, RES_PARATR_END-1,
            RES_LR_SPACE, RES_UL_SPACE,
            RES_BACKGROUND, RES_SHADOW,
            SID_ATTR_TABSTOP_POS, SID_ATTR_TABSTOP_POS,
            SID_ATTR_TABSTOP_DEFAULTS, SID_ATTR_TABSTOP_DEFAULTS,
            SID_ATTR_TABSTOP_OFFSET, SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
            0, 0
        };

        // BruteForce merge because MergeRange in SvTools is buggy:
        std::vector<sal_uInt16> aVec2 = ::lcl_convertRangesToList(pRanges);
        std::vector<sal_uInt16> aVec = ::lcl_convertRangesToList(aRanges);
        aVec2.insert(aVec2.end(), aVec.begin(), aVec.end());
        std::unique_ptr<sal_uInt16[]> pNewRanges(::lcl_convertListToRanges(aVec2));

        pAddrSet.reset(new SfxItemSet(GetParentSwEnvDlg()->pSh->GetView().GetCurShell()->GetPool(),
                                  pNewRanges.get()));
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

    FillItem(GetParentSwEnvDlg()->aEnvItem);
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
    aSet.Put(GetParentSwEnvDlg()->aEnvItem);
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
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    rSet->Put(GetParentSwEnvDlg()->aEnvItem);
    return true;
}

void SwEnvFormatPage::Reset(const SfxItemSet* rSet)
{
    const SwEnvItem& rItem = static_cast<const SwEnvItem&>( rSet->Get(FN_ENVELOP));

    Paper ePaper = SvxPaperInfo::GetSvxPaper(
        Size( std::min(rItem.m_nWidth, rItem.m_nHeight),
        std::max(rItem.m_nWidth, rItem.m_nHeight)), MapUnit::MapTwip);
    for (size_t i = 0; i < m_aIDs.size(); ++i)
        if (m_aIDs[i] == static_cast<sal_uInt16>(ePaper))
            m_xSizeFormatBox->set_active(i);

    // Metric fields
    setfieldval(*m_xAddrLeftField, rItem.m_nAddrFromLeft);
    setfieldval(*m_xAddrTopField, rItem.m_nAddrFromTop );
    setfieldval(*m_xSendLeftField, rItem.m_nSendFromLeft);
    setfieldval(*m_xSendTopField, rItem.m_nSendFromTop );
    setfieldval(*m_xSizeWidthField  , std::max(rItem.m_nWidth, rItem.m_nHeight));
    setfieldval(*m_xSizeHeightField , std::min(rItem.m_nWidth, rItem.m_nHeight));
    SetMinMax();

    GetParentSwEnvDlg()->pSenderSet.reset();
    GetParentSwEnvDlg()->pAddresseeSet.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
