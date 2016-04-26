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
#include <editeng/brushitem.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/menu.hxx>

#include <cmdid.h>
#include <frmatr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <drpcps.hxx>
#include <envfmt.hxx>
#include <fmtcol.hxx>
#include "swuipardlg.hxx"
#include <chrdlgmodes.hxx>
#include <pattern.hxx>
#include <poolfmt.hxx>
#include <uitool.hxx>

#include <vector>
#include <algorithm>

#include <memory>

#include "swabstdlg.hxx"
#include "chrdlg.hrc"
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

static long lUserW = 5669; // 10 cm
static long lUserH = 5669; // 10 cm

SwEnvFormatPage::SwEnvFormatPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "EnvFormatPage",
        "modules/swriter/ui/envformatpage.ui", &rSet)
{
    get(m_pAddrLeftField, "leftaddr");
    get(m_pAddrTopField, "topaddr");
    get(m_pSendLeftField,"leftsender");
    get(m_pSendTopField, "topsender");
    get(m_pSizeFormatBox, "format");
    get(m_pSizeWidthField, "width");
    get(m_pSizeHeightField, "height");
    get(m_pPreview, "preview");
    get(m_pAddrEditButton, "addredit");
    get(m_pSendEditButton, "senderedit");
    SetExchangeSupport();

    // Metrics
    FieldUnit aMetric = ::GetDfltMetric(false);
    SetMetric(*m_pAddrLeftField, aMetric);
    SetMetric(*m_pAddrTopField, aMetric);
    SetMetric(*m_pSendLeftField, aMetric);
    SetMetric(*m_pSendTopField, aMetric);
    SetMetric(*m_pSizeWidthField, aMetric);
    SetMetric(*m_pSizeHeightField, aMetric);

    // Install handlers
    Link<SpinField&,void> aLk = LINK(this, SwEnvFormatPage, ModifyHdl);
    m_pAddrLeftField->SetUpHdl( aLk );
    m_pAddrTopField->SetUpHdl( aLk );
    m_pSendLeftField->SetUpHdl( aLk );
    m_pSendTopField->SetUpHdl( aLk );
    m_pSizeWidthField->SetUpHdl( aLk );
    m_pSizeHeightField->SetUpHdl( aLk );

    m_pAddrLeftField->SetDownHdl( aLk );
    m_pAddrTopField->SetDownHdl( aLk );
    m_pSendLeftField->SetDownHdl( aLk );
    m_pSendTopField->SetDownHdl( aLk );
    m_pSizeWidthField->SetDownHdl( aLk );
    m_pSizeHeightField->SetDownHdl( aLk );

    Link<Control&,void> aLk3 = LINK(this, SwEnvFormatPage, LoseFocusHdl);
    m_pAddrLeftField->SetLoseFocusHdl( aLk3 );
    m_pAddrTopField->SetLoseFocusHdl( aLk3 );
    m_pSendLeftField->SetLoseFocusHdl( aLk3 );
    m_pSendTopField->SetLoseFocusHdl( aLk3 );
    m_pSizeWidthField->SetLoseFocusHdl( aLk3 );
    m_pSizeHeightField->SetLoseFocusHdl( aLk3 );

    Link<MenuButton *, void> aLk2 = LINK(this, SwEnvFormatPage, EditHdl );
    m_pAddrEditButton->SetSelectHdl( aLk2 );
    m_pSendEditButton->SetSelectHdl( aLk2 );

    m_pPreview->SetBorderStyle( WindowBorderStyle::MONO );

    m_pSizeFormatBox->SetSelectHdl(LINK(this, SwEnvFormatPage, FormatHdl));

    // m_pSizeFormatBox
    for (sal_uInt16 i = PAPER_A3; i <= PAPER_KAI32BIG; i++)
    {
        if (i != PAPER_USER)
        {
            const OUString aPaperName = SvxPaperInfo::GetName((Paper) i);

            if (aPaperName.isEmpty())
                continue;

            sal_Int32 nPos = 0;
            while (nPos < m_pSizeFormatBox->GetEntryCount() &&
                   m_pSizeFormatBox->GetEntry(nPos) < aPaperName)
            {
                ++nPos;
            }
            m_pSizeFormatBox->InsertEntry(aPaperName, nPos);
            aIDs.insert( aIDs.begin() + nPos, i);
        }
    }
    m_pSizeFormatBox->InsertEntry(SvxPaperInfo::GetName(PAPER_USER));
    aIDs.push_back( (sal_uInt16) PAPER_USER );

}

SwEnvFormatPage::~SwEnvFormatPage()
{
    disposeOnce();
}

void SwEnvFormatPage::dispose()
{
    m_pAddrLeftField.clear();
    m_pAddrTopField.clear();
    m_pAddrEditButton.clear();
    m_pSendLeftField.clear();
    m_pSendTopField.clear();
    m_pSendEditButton.clear();
    m_pSizeFormatBox.clear();
    m_pSizeWidthField.clear();
    m_pSizeHeightField.clear();
    m_pPreview.clear();
    SfxTabPage::dispose();
}


IMPL_LINK_TYPED( SwEnvFormatPage, LoseFocusHdl, Control&, rControl, void )
{
    ModifyHdl(static_cast<SpinField&>(rControl));
}
IMPL_LINK_TYPED( SwEnvFormatPage, ModifyHdl, SpinField&, rEdit, void )
{
    long lWVal = static_cast< long >(GetFieldVal(*m_pSizeWidthField ));
    long lHVal = static_cast< long >(GetFieldVal(*m_pSizeHeightField));

    long lWidth  = std::max(lWVal, lHVal);
    long lHeight = std::min(lWVal, lHVal);

    if (&rEdit == m_pSizeWidthField || &rEdit == m_pSizeHeightField)
    {
        long nRotatedWidth = lHeight;
        long nRotatedHeight = lWidth;
        Paper ePaper = SvxPaperInfo::GetSvxPaper(
            Size(nRotatedWidth, nRotatedHeight), MAP_TWIP, true);
        for (size_t i = 0; i < aIDs.size(); ++i)
            if (aIDs[i] == (sal_uInt16)ePaper)
                m_pSizeFormatBox->SelectEntryPos(static_cast<sal_Int32>(i));

        // remember user size
        if (aIDs[m_pSizeFormatBox->GetSelectEntryPos()] == (sal_uInt16)PAPER_USER)
        {
            lUserW = lWidth ;
            lUserH = lHeight;
        }

        m_pSizeFormatBox->GetSelectHdl().Call(*m_pSizeFormatBox);
    }
    else
    {
        FillItem(GetParentSwEnvDlg()->aEnvItem);
        SetMinMax();
        m_pPreview->Invalidate();
    }
}

IMPL_LINK_TYPED( SwEnvFormatPage, EditHdl, MenuButton *, pButton, void )
{
    SwWrtShell* pSh = GetParentSwEnvDlg()->pSh;
    OSL_ENSURE(pSh, "Shell missing");

    // determine collection-ptr
    bool bSender = pButton != m_pAddrEditButton;

    SwTextFormatColl* pColl = pSh->GetTextCollFromPool( static_cast< sal_uInt16 >(
        bSender ? RES_POOLCOLL_SENDADRESS : RES_POOLCOLL_JAKETADRESS));
    OSL_ENSURE(pColl, "Text collection missing");

    OString sIdent(pButton->GetCurItemIdent());

    if (sIdent == "character")
    {
        SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

        // In order for the background color not to get ironed over:
        SfxAllItemSet aTmpSet(*pCollSet);
        ::ConvertAttrCharToGen(aTmpSet, CONV_ATTR_ENV);

        SwAbstractDialogFactory* pFact = swui::GetFactory();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        const OUString sFormatStr = pColl->GetName();
        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwCharDlg(GetParentSwEnvDlg(), pSh->GetView(), aTmpSet, SwCharDlgMode::Env, &sFormatStr));
        OSL_ENSURE(pDlg, "Dialog creation failed!");
        if (pDlg->Execute() == RET_OK)
        {
            SfxItemSet aOutputSet( *pDlg->GetOutputItemSet() );
            ::ConvertAttrGenToChar(aOutputSet, aTmpSet, CONV_ATTR_ENV);
            pCollSet->Put(aOutputSet);
        }
    }
    else if (sIdent == "paragraph")
    {
        SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

        // In order for the tabulators not to get ironed over:
        SfxAllItemSet aTmpSet(*pCollSet);

        // Insert tabs, default tabs into ItemSet
        const SvxTabStopItem& rDefTabs = static_cast<const SvxTabStopItem&>(
            pSh->GetView().GetCurShell()->GetPool().GetDefaultItem(RES_PARATR_TABSTOP));

        const sal_uInt16 nDefDist = static_cast<sal_uInt16>(::GetTabDist( rDefTabs ));
        SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
        aTmpSet.Put( aDefDistItem );

        // Current tab
        SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
        aTmpSet.Put( aTabPos );

        // left border as offset
        const long nOff = static_cast<const SvxLRSpaceItem&>(aTmpSet.Get( RES_LR_SPACE )).
                                                            GetTextLeft();
        SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
        aTmpSet.Put( aOff );

        // set BoxInfo
        ::PrepareBoxInfo( aTmpSet, *pSh );

        const OUString sFormatStr = pColl->GetName();
        VclPtrInstance< SwParaDlg > pDlg(GetParentSwEnvDlg(), pSh->GetView(), aTmpSet, DLG_ENVELOP, &sFormatStr);

        if ( pDlg->Execute() == RET_OK )
        {
            // maybe relocate defaults
            const SfxPoolItem* pItem = nullptr;
            SfxItemSet* pOutputSet = const_cast<SfxItemSet*>(pDlg->GetOutputItemSet());
            sal_uInt16 nNewDist;

            if( SfxItemState::SET == pOutputSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS,
                false, &pItem ) &&
                nDefDist != (nNewDist = static_cast<const SfxUInt16Item*>(pItem)->GetValue()) )
            {
                SvxTabStopItem aDefTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
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
SfxItemSet *SwEnvFormatPage::GetCollItemSet(SwTextFormatColl* pColl, bool bSender)
{
    SfxItemSet *&pAddrSet = bSender ? GetParentSwEnvDlg()->pSenderSet : GetParentSwEnvDlg()->pAddresseeSet;
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

        pAddrSet = new SfxItemSet(GetParentSwEnvDlg()->pSh->GetView().GetCurShell()->GetPool(),
                                  pNewRanges.get());
        pAddrSet->Put(pColl->GetAttrSet());
    }

    return pAddrSet;
}

IMPL_LINK_NOARG_TYPED(SwEnvFormatPage, FormatHdl, ListBox&, void)
{
    long lWidth;
    long lHeight;
    long lSendFromLeft;
    long lSendFromTop;
    long lAddrFromLeft;
    long lAddrFromTop;

    const sal_uInt16 nPaper = aIDs[m_pSizeFormatBox->GetSelectEntryPos()];
    if (nPaper != (sal_uInt16)PAPER_USER)
    {
        Size aSz = SvxPaperInfo::GetPaperSize((Paper)nPaper);
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

    SetFieldVal(*m_pAddrLeftField, lAddrFromLeft);
    SetFieldVal(*m_pAddrTopField , lAddrFromTop );
    SetFieldVal(*m_pSendLeftField, lSendFromLeft);
    SetFieldVal(*m_pSendTopField , lSendFromTop );

    SetFieldVal(*m_pSizeWidthField , lWidth );
    SetFieldVal(*m_pSizeHeightField, lHeight);

    SetMinMax();

    FillItem(GetParentSwEnvDlg()->aEnvItem);
    m_pPreview->Invalidate();
}

void SwEnvFormatPage::SetMinMax()
{
    long lWVal = static_cast< long >(GetFieldVal(*m_pSizeWidthField ));
    long lHVal = static_cast< long >(GetFieldVal(*m_pSizeHeightField));

    long lWidth  = std::max(lWVal, lHVal),
         lHeight = std::min(lWVal, lHVal);

    // Min and Max
    m_pAddrLeftField->SetMin((long) 100 * (GetFieldVal(*m_pSendLeftField) + 566), FUNIT_TWIP);
    m_pAddrLeftField->SetMax((long) 100 * (lWidth  - 2 * 566), FUNIT_TWIP);
    m_pAddrTopField->SetMin((long) 100 * (GetFieldVal(*m_pSendTopField ) + 2 * 566), FUNIT_TWIP);
    m_pAddrTopField->SetMax((long) 100 * (lHeight - 2 * 566), FUNIT_TWIP);
    m_pSendLeftField->SetMin((long) 100 * (566), FUNIT_TWIP);
    m_pSendLeftField->SetMax((long) 100 * (GetFieldVal(*m_pAddrLeftField) - 566), FUNIT_TWIP);
    m_pSendTopField->SetMin((long) 100 * (566), FUNIT_TWIP);
    m_pSendTopField->SetMax((long) 100 * (GetFieldVal(*m_pAddrTopField ) - 2 * 566), FUNIT_TWIP);

    // First and last
    m_pAddrLeftField->SetFirst(m_pAddrLeftField->GetMin());
    m_pAddrLeftField->SetLast(m_pAddrLeftField->GetMax());
    m_pAddrTopField->SetFirst(m_pAddrTopField->GetMin());
    m_pAddrTopField->SetLast(m_pAddrTopField->GetMax());
    m_pSendLeftField->SetFirst(m_pSendLeftField->GetMin());
    m_pSendLeftField->SetLast(m_pSendLeftField->GetMax());
    m_pSendTopField->SetFirst(m_pSendTopField->GetMin());
    m_pSendTopField->SetLast(m_pSendTopField->GetMax());

    // Reformat fields
    m_pAddrLeftField->Reformat();
    m_pAddrTopField->Reformat();
    m_pSendLeftField->Reformat();
    m_pSendTopField->Reformat();
    m_pSizeWidthField->Reformat();
    m_pSizeHeightField->Reformat();
}

VclPtr<SfxTabPage> SwEnvFormatPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwEnvFormatPage>::Create(pParent, *rSet);
}

void SwEnvFormatPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParentSwEnvDlg()->aEnvItem);
    Reset(&aSet);
}

SfxTabPage::sfxpg SwEnvFormatPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(_pSet);
    return SfxTabPage::LEAVE_PAGE;
}

void SwEnvFormatPage::FillItem(SwEnvItem& rItem)
{
    rItem.lAddrFromLeft = static_cast< sal_Int32 >(GetFieldVal(*m_pAddrLeftField));
    rItem.lAddrFromTop  = static_cast< sal_Int32 >(GetFieldVal(*m_pAddrTopField ));
    rItem.lSendFromLeft = static_cast< sal_Int32 >(GetFieldVal(*m_pSendLeftField));
    rItem.lSendFromTop  = static_cast< sal_Int32 >(GetFieldVal(*m_pSendTopField ));

    const sal_uInt16 nPaper = aIDs[m_pSizeFormatBox->GetSelectEntryPos()];
    if (nPaper == (sal_uInt16)PAPER_USER)
    {
        long lWVal = static_cast< long >(GetFieldVal(*m_pSizeWidthField ));
        long lHVal = static_cast< long >(GetFieldVal(*m_pSizeHeightField));
        rItem.lWidth  = std::max(lWVal, lHVal);
        rItem.lHeight = std::min(lWVal, lHVal);
    }
    else
    {
        long lWVal = SvxPaperInfo::GetPaperSize((Paper)nPaper).Width ();
        long lHVal = SvxPaperInfo::GetPaperSize((Paper)nPaper).Height();
        rItem.lWidth  = std::max(lWVal, lHVal);
        rItem.lHeight = std::min(lWVal, lHVal);
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
        Size( std::min(rItem.lWidth, rItem.lHeight),
        std::max(rItem.lWidth, rItem.lHeight)), MAP_TWIP, true);
    for (size_t i = 0; i < aIDs.size(); ++i)
        if (aIDs[i] == (sal_uInt16)ePaper)
            m_pSizeFormatBox->SelectEntryPos(static_cast<sal_Int32>(i));

    // Metric fields
    SetFieldVal(*m_pAddrLeftField, rItem.lAddrFromLeft);
    SetFieldVal(*m_pAddrTopField, rItem.lAddrFromTop );
    SetFieldVal(*m_pSendLeftField, rItem.lSendFromLeft);
    SetFieldVal(*m_pSendTopField, rItem.lSendFromTop );
    SetFieldVal(*m_pSizeWidthField  , std::max(rItem.lWidth, rItem.lHeight));
    SetFieldVal(*m_pSizeHeightField , std::min(rItem.lWidth, rItem.lHeight));
    SetMinMax();

    DELETEZ(GetParentSwEnvDlg()->pSenderSet);
    DELETEZ(GetParentSwEnvDlg()->pAddresseeSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
