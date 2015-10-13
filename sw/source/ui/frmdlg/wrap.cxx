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

#include "hintids.hxx"
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>

#include <sfx2/htmlmode.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtfollowtextflow.hxx>
#include <svx/swframevalidation.hxx>

#include "cmdid.h"
#include <docsh.hxx>
#include "uitool.hxx"
#include "wrtsh.hxx"
#include "swmodule.hxx"
#include "viewopt.hxx"
#include "frmatr.hxx"
#include "frmmgr.hxx"
#include "globals.hrc"
#include "frmui.hrc"
#include "wrap.hxx"

using namespace ::com::sun::star;

const sal_uInt16 SwWrapTabPage::m_aWrapPageRg[] = {
    RES_LR_SPACE, RES_UL_SPACE,
    RES_PROTECT, RES_SURROUND,
    RES_PRINT, RES_PRINT,
    0
};

SwWrapDlg::SwWrapDlg(vcl::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, bool bDrawMode)
    : SfxSingleTabDialog(pParent, rSet, "WrapDialog", "modules/swriter/ui/wrapdialog.ui")
    , pWrtShell(pSh)

{
    // create TabPage
    VclPtr<SwWrapTabPage> pNewPage = static_cast<SwWrapTabPage*>( SwWrapTabPage::Create(get_content_area(), &rSet).get() );
    pNewPage->SetFormatUsed(false, bDrawMode);
    pNewPage->SetShell(pWrtShell);
    SetTabPage(pNewPage);
}

SwWrapTabPage::SwWrapTabPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "WrapPage" , "modules/swriter/ui/wrappage.ui", &rSet)
    , m_nOldLeftMargin(0)
    , m_nOldRightMargin(0)
    , m_nOldUpperMargin(0)
    , m_nOldLowerMargin(0)
    , m_nAnchorId(FLY_AT_PARA)
    , m_nHtmlMode(0)
    , m_pWrtSh(0)
    , m_bFormat(false)
    , m_bNew(true)
    , m_bHtmlMode(false)
    , m_bDrawMode(false)
    , m_bContourImage(false)
{
    get(m_pNoWrapRB, "none");
    get(m_pWrapLeftRB, "before");
    get(m_pWrapRightRB, "after");
    get(m_pWrapParallelRB, "parallel");
    get(m_pWrapThroughRB, "through");
    get(m_pIdealWrapRB, "optimal");
    get(m_pLeftMarginED, "left");
    get(m_pRightMarginED, "right");
    get(m_pTopMarginED, "top");
    get(m_pBottomMarginED, "bottom");
    get(m_pWrapAnchorOnlyCB, "anchoronly");
    get(m_pWrapTransparentCB, "transparent");
    get(m_pWrapOutlineCB, "outline");
    get(m_pWrapOutsideCB, "outside");

    SetExchangeSupport();

    Link<SpinField&,void> aLk = LINK(this, SwWrapTabPage, RangeModifyHdl);
    Link<Control&,void> aLk3 = LINK(this, SwWrapTabPage, RangeLoseFocusHdl);
    m_pLeftMarginED->SetUpHdl(aLk);
    m_pLeftMarginED->SetDownHdl(aLk);
    m_pLeftMarginED->SetFirstHdl(aLk);
    m_pLeftMarginED->SetLastHdl(aLk);
    m_pLeftMarginED->SetLoseFocusHdl(aLk3);

    m_pRightMarginED->SetUpHdl(aLk);
    m_pRightMarginED->SetDownHdl(aLk);
    m_pRightMarginED->SetFirstHdl(aLk);
    m_pRightMarginED->SetLastHdl(aLk);
    m_pRightMarginED->SetLoseFocusHdl(aLk3);

    m_pTopMarginED->SetUpHdl(aLk);
    m_pTopMarginED->SetDownHdl(aLk);
    m_pTopMarginED->SetFirstHdl(aLk);
    m_pTopMarginED->SetLastHdl(aLk);
    m_pTopMarginED->SetLoseFocusHdl(aLk3);

    m_pBottomMarginED->SetUpHdl(aLk);
    m_pBottomMarginED->SetDownHdl(aLk);
    m_pBottomMarginED->SetFirstHdl(aLk);
    m_pBottomMarginED->SetLastHdl(aLk);
    m_pBottomMarginED->SetLoseFocusHdl(aLk3);

    Link<Button*,void> aLk2 = LINK(this, SwWrapTabPage, WrapTypeHdl);
    m_pNoWrapRB->SetClickHdl(aLk2);
    m_pWrapLeftRB->SetClickHdl(aLk2);
    m_pWrapRightRB->SetClickHdl(aLk2);
    m_pWrapParallelRB->SetClickHdl(aLk2);
    m_pWrapThroughRB->SetClickHdl(aLk2);
    m_pIdealWrapRB->SetClickHdl(aLk2);
    ApplyImageList();
    m_pWrapOutlineCB->SetClickHdl(LINK(this, SwWrapTabPage, ContourHdl));
}

SwWrapTabPage::~SwWrapTabPage()
{
    disposeOnce();
}

void SwWrapTabPage::dispose()
{
    m_pNoWrapRB.clear();
    m_pWrapLeftRB.clear();
    m_pWrapRightRB.clear();
    m_pWrapParallelRB.clear();
    m_pWrapThroughRB.clear();
    m_pIdealWrapRB.clear();
    m_pLeftMarginED.clear();
    m_pRightMarginED.clear();
    m_pTopMarginED.clear();
    m_pBottomMarginED.clear();
    m_pWrapAnchorOnlyCB.clear();
    m_pWrapTransparentCB.clear();
    m_pWrapOutlineCB.clear();
    m_pWrapOutsideCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwWrapTabPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwWrapTabPage>::Create(pParent, *rSet);
}

void SwWrapTabPage::Reset(const SfxItemSet *rSet)
{
    // contour for Draw, Graphic and OLE (Insert/Graphic/Properties still missing!)
    if( m_bDrawMode )
    {
        m_pWrapOutlineCB->Show();
        m_pWrapOutsideCB->Show();

        m_pWrapTransparentCB->Check( 0 == static_cast<const SfxInt16Item&>(rSet->Get(
                                        FN_DRAW_WRAP_DLG)).GetValue() );
        m_pWrapTransparentCB->SaveValue();
    }
    else
    {
        bool bShowCB = m_bFormat;
        if( !m_bFormat )
        {
            int nSelType = m_pWrtSh->GetSelectionType();
            if( ( nSelType & nsSelectionType::SEL_GRF ) ||
                ( nSelType & nsSelectionType::SEL_OLE && GRAPHIC_NONE !=
                            m_pWrtSh->GetIMapGraphic().GetType() ))
                bShowCB = true;
        }
        if( bShowCB )
        {
            m_pWrapOutlineCB->Show();
            m_pWrapOutsideCB->Show();
        }
    }

    m_nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    m_bHtmlMode = (m_nHtmlMode & HTMLMODE_ON) != 0;

    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    SetMetric(*m_pLeftMarginED, aMetric);
    SetMetric(*m_pRightMarginED, aMetric);
    SetMetric(*m_pTopMarginED, aMetric);
    SetMetric(*m_pBottomMarginED, aMetric);

    const SwFormatSurround& rSurround = static_cast<const SwFormatSurround&>(rSet->Get(RES_SURROUND));

    SwSurround nSur = rSurround.GetSurround();
    const SwFormatAnchor &rAnch = static_cast<const SwFormatAnchor&>(rSet->Get(RES_ANCHOR));
    m_nAnchorId = rAnch.GetAnchorId();

    if (((m_nAnchorId == FLY_AT_PARA) || (m_nAnchorId == FLY_AT_CHAR))
        && (nSur != SURROUND_NONE))
    {
        m_pWrapAnchorOnlyCB->Check( rSurround.IsAnchorOnly() );
    }
    else
    {
        m_pWrapAnchorOnlyCB->Enable( false );
    }

    bool bContour = rSurround.IsContour();
    m_pWrapOutlineCB->Check( bContour );
    m_pWrapOutsideCB->Check( rSurround.IsOutside() );
    m_pWrapThroughRB->Enable(!m_pWrapOutlineCB->IsChecked());
    m_bContourImage = !bContour;

    RadioButton* pBtn = NULL;

    switch (nSur)
    {
        case SURROUND_NONE:
        {
            pBtn = m_pNoWrapRB;
            break;
        }

        case SURROUND_THROUGHT:
        {
            // transparent ?
            pBtn = m_pWrapThroughRB;

            if (!m_bDrawMode)
            {
                const SvxOpaqueItem& rOpaque = static_cast<const SvxOpaqueItem&>(rSet->Get(RES_OPAQUE));
                m_pWrapTransparentCB->Check(!rOpaque.GetValue());
            }
            break;
        }

        case SURROUND_PARALLEL:
        {
            pBtn = m_pWrapParallelRB;
            break;
        }

        case SURROUND_IDEAL:
        {
            pBtn = m_pIdealWrapRB;
            break;
        }

        default:
        {
            if (nSur == SURROUND_LEFT)
                pBtn = m_pWrapLeftRB;
            else if (nSur == SURROUND_RIGHT)
                pBtn = m_pWrapRightRB;
        }
    }
    if (pBtn)
    {
        pBtn->Check();
        WrapTypeHdl(pBtn);
        // For character objects that currently are in passage, the default
        // "contour on" is prepared here, in case we switch to any other
        // passage later.
        if (m_bDrawMode && !m_pWrapOutlineCB->IsEnabled())
            m_pWrapOutlineCB->Check();
    }
    m_pWrapTransparentCB->Enable( pBtn == m_pWrapThroughRB && !m_bHtmlMode );

    const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(rSet->Get(RES_UL_SPACE));
    const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(rSet->Get(RES_LR_SPACE));

    // gap to text
    m_pLeftMarginED->SetValue(m_pLeftMarginED->Normalize(rLR.GetLeft()), FUNIT_TWIP);
    m_pRightMarginED->SetValue(m_pRightMarginED->Normalize(rLR.GetRight()), FUNIT_TWIP);
    m_pTopMarginED->SetValue(m_pTopMarginED->Normalize(rUL.GetUpper()), FUNIT_TWIP);
    m_pBottomMarginED->SetValue(m_pBottomMarginED->Normalize(rUL.GetLower()), FUNIT_TWIP);

    ContourHdl(0);
    ActivatePage( *rSet );
}

// stuff attributes into the set when OK
bool SwWrapTabPage::FillItemSet(SfxItemSet *rSet)
{
    bool bModified = false;
    const SfxPoolItem* pOldItem;
    const SwFormatSurround& rOldSur = static_cast<const SwFormatSurround&>(GetItemSet().Get(RES_SURROUND));
    SwFormatSurround aSur( rOldSur );

    SvxOpaqueItem aOp( RES_OPAQUE);

    if (!m_bDrawMode)
    {
        const SvxOpaqueItem& rOpaque = static_cast<const SvxOpaqueItem&>(GetItemSet().Get(RES_OPAQUE));
        aOp = rOpaque;
        aOp.SetValue(true);
    }

    if (m_pNoWrapRB->IsChecked())
        aSur.SetSurround(SURROUND_NONE);
    else if (m_pWrapLeftRB->IsChecked())
        aSur.SetSurround(SURROUND_LEFT);
    else if (m_pWrapRightRB->IsChecked())
        aSur.SetSurround(SURROUND_RIGHT);
    else if (m_pWrapParallelRB->IsChecked())
        aSur.SetSurround(SURROUND_PARALLEL);
    else if (m_pWrapThroughRB->IsChecked())
    {
        aSur.SetSurround(SURROUND_THROUGHT);
        if (m_pWrapTransparentCB->IsChecked() && !m_bDrawMode)
            aOp.SetValue(false);
    }
    else if (m_pIdealWrapRB->IsChecked())
        aSur.SetSurround(SURROUND_IDEAL);

    aSur.SetAnchorOnly( m_pWrapAnchorOnlyCB->IsChecked() );
    bool bContour = m_pWrapOutlineCB->IsChecked() && m_pWrapOutlineCB->IsEnabled();
    aSur.SetContour( bContour );

    if ( bContour )
        aSur.SetOutside(m_pWrapOutsideCB->IsChecked());

    if(0 == (pOldItem = GetOldItem( *rSet, RES_SURROUND )) ||
                aSur != *pOldItem )
    {
        rSet->Put(aSur);
        bModified = true;
    }

    if (!m_bDrawMode)
    {
        if(0 == (pOldItem = GetOldItem( *rSet, FN_OPAQUE )) ||
                    aOp != *pOldItem )
        {
            rSet->Put(aOp);
            bModified = true;
        }
    }

    bool bTopMod = m_pTopMarginED->IsValueModified();
    bool bBottomMod = m_pBottomMarginED->IsValueModified();

    SvxULSpaceItem aUL( RES_UL_SPACE );
    aUL.SetUpper((sal_uInt16)m_pTopMarginED->Denormalize(m_pTopMarginED->GetValue(FUNIT_TWIP)));
    aUL.SetLower((sal_uInt16)m_pBottomMarginED->Denormalize(m_pBottomMarginED->GetValue(FUNIT_TWIP)));

    if ( bTopMod || bBottomMod )
    {
        if(0 == (pOldItem = GetOldItem(*rSet, RES_UL_SPACE)) ||
                aUL != *pOldItem )
        {
            rSet->Put( aUL, RES_UL_SPACE );
            bModified = true;
        }
    }

    bool bLeftMod = m_pLeftMarginED->IsValueModified();
    bool bRightMod = m_pRightMarginED->IsValueModified();

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    aLR.SetLeft((sal_uInt16)m_pLeftMarginED->Denormalize(m_pLeftMarginED->GetValue(FUNIT_TWIP)));
    aLR.SetRight((sal_uInt16)m_pRightMarginED->Denormalize(m_pRightMarginED->GetValue(FUNIT_TWIP)));

    if ( bLeftMod || bRightMod )
    {
        if( 0 == (pOldItem = GetOldItem(*rSet, RES_LR_SPACE)) ||
                aLR != *pOldItem )
        {
            rSet->Put(aLR, RES_LR_SPACE);
            bModified = true;
        }
    }

    if ( m_bDrawMode )
    {
        bool bChecked = m_pWrapTransparentCB->IsChecked() && m_pWrapTransparentCB->IsEnabled();
        if ((m_pWrapTransparentCB->GetSavedValue() == 1) != bChecked)
            bModified |= 0 != rSet->Put(SfxInt16Item(FN_DRAW_WRAP_DLG, bChecked ? 0 : 1));
    }

    return bModified;
}

// example update
void SwWrapTabPage::ActivatePage(const SfxItemSet& rSet)
{
    // anchor
    const SwFormatAnchor &rAnch = static_cast<const SwFormatAnchor&>(rSet.Get(RES_ANCHOR));
    m_nAnchorId = rAnch.GetAnchorId();
    bool bEnable = (m_nAnchorId != FLY_AS_CHAR);

    if (!m_bDrawMode)
    {
        SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell() : m_pWrtSh;
        SwFlyFrmAttrMgr aMgr( m_bNew, pSh, static_cast<const SwAttrSet&>(GetItemSet()) );
        SvxSwFrameValidation aVal;

        // size
        const SwFormatFrmSize& rFrmSize = static_cast<const SwFormatFrmSize&>(rSet.Get(RES_FRM_SIZE));
        Size aSize = rFrmSize.GetSize();

        // margin
        const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(rSet.Get(RES_UL_SPACE));
        const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(rSet.Get(RES_LR_SPACE));
        m_nOldLeftMargin  = static_cast< sal_uInt16 >(rLR.GetLeft());
        m_nOldRightMargin = static_cast< sal_uInt16 >(rLR.GetRight());
        m_nOldUpperMargin = static_cast< sal_uInt16 >(rUL.GetUpper());
        m_nOldLowerMargin = static_cast< sal_uInt16 >(rUL.GetLower());

        // position
        const SwFormatHoriOrient& rHori = static_cast<const SwFormatHoriOrient&>(rSet.Get(RES_HORI_ORIENT));
        const SwFormatVertOrient& rVert = static_cast<const SwFormatVertOrient&>(rSet.Get(RES_VERT_ORIENT));

        aVal.nAnchorType = static_cast< sal_Int16 >(m_nAnchorId);
        aVal.bAutoHeight = rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE;
        aVal.bAutoWidth = rFrmSize.GetWidthSizeType() == ATT_MIN_SIZE;
        aVal.bMirror = rHori.IsPosToggle();
        // #i18732#
        aVal.bFollowTextFlow =
            static_cast<const SwFormatFollowTextFlow&>(rSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();

        aVal.nHoriOrient = (short)rHori.GetHoriOrient();
        aVal.nVertOrient = (short)rVert.GetVertOrient();

        aVal.nHPos = rHori.GetPos();
        aVal.nHRelOrient = rHori.GetRelationOrient();
        aVal.nVPos = rVert.GetPos();
        aVal.nVRelOrient = rVert.GetRelationOrient();

        if (rFrmSize.GetWidthPercent() && rFrmSize.GetWidthPercent() != SwFormatFrmSize::SYNCED)
            aSize.Width() = aSize.Width() * rFrmSize.GetWidthPercent() / 100;

        if (rFrmSize.GetHeightPercent() && rFrmSize.GetHeightPercent() != SwFormatFrmSize::SYNCED)
            aSize.Height() = aSize.Height() * rFrmSize.GetHeightPercent() / 100;

        aVal.nWidth  = aSize.Width();
        aVal.nHeight = aSize.Height();
        m_aFrmSize = aSize;

        aMgr.ValidateMetrics(aVal, 0);

        SwTwips nLeft;
        SwTwips nRight;
        SwTwips nTop;
        SwTwips nBottom;

        nLeft   = aVal.nHPos - aVal.nMinHPos;
        nRight  = aVal.nMaxWidth - aVal.nWidth;
        nTop    = aVal.nVPos - aVal.nMinVPos;
        nBottom = aVal.nMaxHeight - aVal.nHeight;

        {
            if (aVal.nAnchorType == FLY_AS_CHAR)
            {
                nLeft = nRight;

                if (aVal.nVPos < 0)
                {
                    if (aVal.nVPos <= aVal.nMaxHeight)
                        nTop = aVal.nMaxVPos - aVal.nHeight;
                    else
                        nTop = nBottom = 0; // no passage
                }
                else
                    nTop = aVal.nMaxVPos - aVal.nHeight - aVal.nVPos;
            }
            else
            {
                nLeft += nRight;
                nTop += nBottom;
            }

            nBottom = nTop;
            nRight = nLeft;
        }

        m_pLeftMarginED->SetMax(m_pLeftMarginED->Normalize(nLeft), FUNIT_TWIP);
        m_pRightMarginED->SetMax(m_pRightMarginED->Normalize(nRight), FUNIT_TWIP);

        m_pTopMarginED->SetMax(m_pTopMarginED->Normalize(nTop), FUNIT_TWIP);
        m_pBottomMarginED->SetMax(m_pBottomMarginED->Normalize(nBottom), FUNIT_TWIP);

        RangeModifyHdl(*m_pLeftMarginED);
        RangeModifyHdl(*m_pTopMarginED);
    }

    const SwFormatSurround& rSurround = static_cast<const SwFormatSurround&>(rSet.Get(RES_SURROUND));
    SwSurround nSur = rSurround.GetSurround();

    m_pWrapTransparentCB->Enable( bEnable && !m_bHtmlMode && nSur == SURROUND_THROUGHT );
    if(m_bHtmlMode)
    {
        const SwFormatHoriOrient& rHori = static_cast<const SwFormatHoriOrient&>(rSet.Get(RES_HORI_ORIENT));
        sal_Int16 eHOrient = rHori.GetHoriOrient();
        sal_Int16 eHRelOrient = rHori.GetRelationOrient();
        m_pWrapOutlineCB->Hide();
        const bool bAllHtmlModes =
            ((m_nAnchorId == FLY_AT_PARA) || (m_nAnchorId == FLY_AT_CHAR)) &&
                            (eHOrient == text::HoriOrientation::RIGHT || eHOrient == text::HoriOrientation::LEFT);
        m_pWrapAnchorOnlyCB->Enable( bAllHtmlModes && nSur != SURROUND_NONE );
        m_pWrapOutsideCB->Hide();
        m_pIdealWrapRB->Enable( false );

        m_pWrapTransparentCB->Enable( false );
        m_pNoWrapRB->Enable( FLY_AT_PARA == m_nAnchorId );
        m_pWrapParallelRB->Enable( false  );
        m_pWrapLeftRB->Enable
                    (  (FLY_AT_PARA == m_nAnchorId)
                    || (   (FLY_AT_CHAR == m_nAnchorId)
                        && (eHOrient == text::HoriOrientation::RIGHT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));
        m_pWrapRightRB->Enable
                    (  (FLY_AT_PARA == m_nAnchorId)
                    || (   (FLY_AT_CHAR == m_nAnchorId)
                        && (eHOrient == text::HoriOrientation::LEFT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));

        m_pWrapThroughRB->Enable
                (   (  (FLY_AT_PAGE == m_nAnchorId)
                    || (   (FLY_AT_CHAR == m_nAnchorId)
                        && (eHRelOrient != text::RelOrientation::PRINT_AREA))
                    || (FLY_AT_PARA == m_nAnchorId))
                && (eHOrient != text::HoriOrientation::RIGHT));
        if(m_pNoWrapRB->IsChecked() && !m_pNoWrapRB->IsEnabled())
        {
            if(m_pWrapThroughRB->IsEnabled())
                m_pWrapThroughRB->Check();
            else if(m_pWrapLeftRB->IsEnabled())
                m_pWrapLeftRB->Check();
            else if(m_pWrapRightRB->IsEnabled())
                m_pWrapRightRB->Check();

        }
        if(m_pWrapLeftRB->IsChecked() && !m_pWrapLeftRB->IsEnabled())
        {
            if(m_pWrapRightRB->IsEnabled())
                m_pWrapRightRB->Check();
            else if(m_pWrapThroughRB->IsEnabled())
                m_pWrapThroughRB->Check();
        }
        if(m_pWrapRightRB->IsChecked() && !m_pWrapRightRB->IsEnabled())
        {
            if(m_pWrapLeftRB->IsEnabled())
                m_pWrapLeftRB->Check();
            else if(m_pWrapThroughRB->IsEnabled())
                m_pWrapThroughRB->Check();
        }
        if(m_pWrapThroughRB->IsChecked() && !m_pWrapThroughRB->IsEnabled())
            if(m_pNoWrapRB->IsEnabled())
                m_pNoWrapRB->Check();

        if(m_pWrapParallelRB->IsChecked() && !m_pWrapParallelRB->IsEnabled())
            m_pWrapThroughRB->Check();
    }
    else
    {
        m_pNoWrapRB->Enable( bEnable );
        m_pWrapLeftRB->Enable( bEnable );
        m_pWrapRightRB->Enable( bEnable );
        m_pIdealWrapRB->Enable( bEnable );
        m_pWrapThroughRB->Enable( bEnable );
        m_pWrapParallelRB->Enable( bEnable );
        m_pWrapAnchorOnlyCB->Enable(
                ((m_nAnchorId == FLY_AT_PARA) || (m_nAnchorId == FLY_AT_CHAR))
                && nSur != SURROUND_NONE );
    }
    ContourHdl(0);
}

SfxTabPage::sfxpg SwWrapTabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return LEAVE_PAGE;
}

// range check
IMPL_LINK_TYPED( SwWrapTabPage, RangeLoseFocusHdl, Control&, rControl, void )
{
    RangeModifyHdl( static_cast<SpinField&>(rControl) );
}
IMPL_LINK_TYPED( SwWrapTabPage, RangeModifyHdl, SpinField&, rSpin, void )
{
    MetricField& rEdit = static_cast<MetricField&>(rSpin);
    sal_Int64 nValue = rEdit.GetValue();
    MetricField *pOpposite = 0;
    if (&rEdit == m_pLeftMarginED)
        pOpposite = m_pRightMarginED;
    else if (&rEdit == m_pRightMarginED)
        pOpposite = m_pLeftMarginED;
    else if (&rEdit == m_pTopMarginED)
        pOpposite = m_pBottomMarginED;
    else if (&rEdit == m_pBottomMarginED)
        pOpposite = m_pTopMarginED;

    OSL_ASSERT(pOpposite);

    if (pOpposite)
    {
        sal_Int64 nOpposite = pOpposite->GetValue();

        if (nValue + nOpposite > std::max(rEdit.GetMax(), pOpposite->GetMax()))
            pOpposite->SetValue(pOpposite->GetMax() - nValue);
    }
}

IMPL_LINK_TYPED( SwWrapTabPage, WrapTypeHdl, Button *, pBtn, void )
{
    bool bWrapThrough = (pBtn == m_pWrapThroughRB);
    m_pWrapTransparentCB->Enable( bWrapThrough && !m_bHtmlMode );
    bWrapThrough |= ( m_nAnchorId == FLY_AS_CHAR );
    m_pWrapOutlineCB->Enable( !bWrapThrough && pBtn != m_pNoWrapRB);
    m_pWrapOutsideCB->Enable( !bWrapThrough && m_pWrapOutlineCB->IsChecked() );
    m_pWrapAnchorOnlyCB->Enable(
        ((m_nAnchorId == FLY_AT_PARA) || (m_nAnchorId == FLY_AT_CHAR)) &&
        (pBtn != m_pNoWrapRB) );

    ContourHdl(0);
}

IMPL_LINK_NOARG_TYPED(SwWrapTabPage, ContourHdl, Button*, void)
{
    bool bEnable = !(m_pWrapOutlineCB->IsChecked() && m_pWrapOutlineCB->IsEnabled());

    m_pWrapOutsideCB->Enable(!bEnable);

    bEnable =  !m_pWrapOutlineCB->IsChecked();
    if (bEnable == m_bContourImage) // so that it doesn't always flicker
    {
        m_bContourImage = !bEnable;
        ApplyImageList();
    }
}

void SwWrapTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
            ApplyImageList();

    SfxTabPage::DataChanged( rDCEvt );
}

void SwWrapTabPage::ApplyImageList()
{
    m_pWrapThroughRB->SetModeRadioImage(get<FixedImage>("imgthrough")->GetImage());
    bool bWrapOutline =  !m_pWrapOutlineCB->IsChecked();
    if(bWrapOutline)
    {
        m_pNoWrapRB->SetModeRadioImage(get<FixedImage>("imgnone")->GetImage());
        m_pWrapLeftRB->SetModeRadioImage(get<FixedImage>("imgleft")->GetImage());
        m_pWrapRightRB->SetModeRadioImage(get<FixedImage>("imgright")->GetImage());
        m_pWrapParallelRB->SetModeRadioImage(get<FixedImage>("imgparallel")->GetImage());
        m_pIdealWrapRB->SetModeRadioImage(get<FixedImage>("imgideal")->GetImage());
    }
    else
    {
        m_pNoWrapRB->SetModeRadioImage(get<FixedImage>("imgkonnone")->GetImage());
        m_pWrapLeftRB->SetModeRadioImage(get<FixedImage>("imgkonleft")->GetImage());
        m_pWrapRightRB->SetModeRadioImage(get<FixedImage>("imgkonright")->GetImage());
        m_pWrapParallelRB->SetModeRadioImage(get<FixedImage>("imgkonparallel")->GetImage());
        m_pIdealWrapRB->SetModeRadioImage(get<FixedImage>("imgkonideal")->GetImage());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
