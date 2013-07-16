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
#include <sfx2/htmlmode.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
// #i18732#
#include <fmtfollowtextflow.hxx>
#include <svx/swframevalidation.hxx>


#include "cmdid.h"
#include "uitool.hxx"
#include "wrtsh.hxx"
#include "swmodule.hxx"
#include "viewopt.hxx"
#include "frmatr.hxx"
#include "frmmgr.hxx"
#include "globals.hrc"
#include "frmui.hrc"
#include "wrap.hrc"
#include "wrap.hxx"

using namespace ::com::sun::star;

static sal_uInt16 aWrapPageRg[] = {
    RES_LR_SPACE, RES_UL_SPACE,
    RES_PROTECT, RES_SURROUND,
    RES_PRINT, RES_PRINT,
    0
};

SwWrapDlg::SwWrapDlg(Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, sal_Bool bDrawMode) :
    SfxNoLayoutSingleTabDialog(pParent, rSet, 0),
    pWrtShell(pSh)

{
    // create TabPage
    SwWrapTabPage* pNewPage = (SwWrapTabPage*) SwWrapTabPage::Create(this, rSet);
    pNewPage->SetFormatUsed(sal_False, bDrawMode);
    pNewPage->SetShell(pWrtShell);
    SetTabPage(pNewPage);

    String sTitle(SW_RES(STR_FRMUI_WRAP));
    SetText(sTitle);
}

SwWrapDlg::~SwWrapDlg()
{
}

SwWrapTabPage::SwWrapTabPage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "WrapPage" , "modules/swriter/ui/wrappage.ui", rSet)
    , nAnchorId(FLY_AT_PARA)
    , nHtmlMode(0)
    , pWrtSh(0)
    , bFormat(false)
    , bNew(true)
    , bHtmlMode(false)
    , bContourImage(false)

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

    Link aLk = LINK(this, SwWrapTabPage, RangeModifyHdl);
    m_pLeftMarginED->SetUpHdl(aLk);
    m_pLeftMarginED->SetDownHdl(aLk);
    m_pLeftMarginED->SetFirstHdl(aLk);
    m_pLeftMarginED->SetLastHdl(aLk);
    m_pLeftMarginED->SetLoseFocusHdl(aLk);

    m_pRightMarginED->SetUpHdl(aLk);
    m_pRightMarginED->SetDownHdl(aLk);
    m_pRightMarginED->SetFirstHdl(aLk);
    m_pRightMarginED->SetLastHdl(aLk);
    m_pRightMarginED->SetLoseFocusHdl(aLk);

    m_pTopMarginED->SetUpHdl(aLk);
    m_pTopMarginED->SetDownHdl(aLk);
    m_pTopMarginED->SetFirstHdl(aLk);
    m_pTopMarginED->SetLastHdl(aLk);
    m_pTopMarginED->SetLoseFocusHdl(aLk);

    m_pBottomMarginED->SetUpHdl(aLk);
    m_pBottomMarginED->SetDownHdl(aLk);
    m_pBottomMarginED->SetFirstHdl(aLk);
    m_pBottomMarginED->SetLastHdl(aLk);
    m_pBottomMarginED->SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwWrapTabPage, WrapTypeHdl);
    m_pNoWrapRB->SetClickHdl(aLk);
    m_pWrapLeftRB->SetClickHdl(aLk);
    m_pWrapRightRB->SetClickHdl(aLk);
    m_pWrapParallelRB->SetClickHdl(aLk);
    m_pWrapThroughRB->SetClickHdl(aLk);
    m_pIdealWrapRB->SetClickHdl(aLk);
    ApplyImageList();
    m_pWrapOutlineCB->SetClickHdl(LINK(this, SwWrapTabPage, ContourHdl));
}

SwWrapTabPage::~SwWrapTabPage()
{
}

SfxTabPage* SwWrapTabPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwWrapTabPage(pParent, rSet);
}

void SwWrapTabPage::Reset(const SfxItemSet &rSet)
{
    // contour for Draw, Graphic and OLE (Insert/Graphic/Properties still missing!)
    if( bDrawMode )
    {
        m_pWrapOutlineCB->Show();
        m_pWrapOutsideCB->Show();

        m_pWrapTransparentCB->Check( 0 == ((const SfxInt16Item&)rSet.Get(
                                        FN_DRAW_WRAP_DLG)).GetValue() );
        m_pWrapTransparentCB->SaveValue();
    }
    else
    {
        sal_Bool bShowCB = bFormat;
        if( !bFormat )
        {
            int nSelType = pWrtSh->GetSelectionType();
            if( ( nSelType & nsSelectionType::SEL_GRF ) ||
                ( nSelType & nsSelectionType::SEL_OLE && GRAPHIC_NONE !=
                            pWrtSh->GetIMapGraphic().GetType() ))
                bShowCB = sal_True;
        }
        if( bShowCB )
        {
            m_pWrapOutlineCB->Show();
            m_pWrapOutsideCB->Show();
        }
    }

    nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;

    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    SetMetric(*m_pLeftMarginED, aMetric);
    SetMetric(*m_pRightMarginED, aMetric);
    SetMetric(*m_pTopMarginED, aMetric);
    SetMetric(*m_pBottomMarginED, aMetric);

    const SwFmtSurround& rSurround = (const SwFmtSurround&)rSet.Get(RES_SURROUND);

    SwSurround nSur = rSurround.GetSurround();
    const SwFmtAnchor &rAnch = (const SwFmtAnchor&)rSet.Get(RES_ANCHOR);
    nAnchorId = rAnch.GetAnchorId();

    if (((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR))
        && (nSur != SURROUND_NONE))
    {
        m_pWrapAnchorOnlyCB->Check( rSurround.IsAnchorOnly() );
    }
    else
    {
        m_pWrapAnchorOnlyCB->Enable( sal_False );
    }

    sal_Bool bContour = rSurround.IsContour();
    m_pWrapOutlineCB->Check( bContour );
    m_pWrapOutsideCB->Check( rSurround.IsOutside() );
    m_pWrapThroughRB->Enable(!m_pWrapOutlineCB->IsChecked());
    bContourImage = !bContour;

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

            if (!bDrawMode)
            {
                const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)rSet.Get(RES_OPAQUE);
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
        if (bDrawMode && !m_pWrapOutlineCB->IsEnabled())
            m_pWrapOutlineCB->Check();
    }
    m_pWrapTransparentCB->Enable( pBtn == m_pWrapThroughRB && !bHtmlMode );

    const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)rSet.Get(RES_UL_SPACE);
    const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);

    // gap to text
    m_pLeftMarginED->SetValue(m_pLeftMarginED->Normalize(rLR.GetLeft()), FUNIT_TWIP);
    m_pRightMarginED->SetValue(m_pRightMarginED->Normalize(rLR.GetRight()), FUNIT_TWIP);
    m_pTopMarginED->SetValue(m_pTopMarginED->Normalize(rUL.GetUpper()), FUNIT_TWIP);
    m_pBottomMarginED->SetValue(m_pBottomMarginED->Normalize(rUL.GetLower()), FUNIT_TWIP);

    ContourHdl(0);
    ActivatePage( rSet );
}

/*--------------------------------------------------------------------
    Description:    stuff attributes into the set when OK
 --------------------------------------------------------------------*/
sal_Bool SwWrapTabPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bModified = sal_False;
    const SfxPoolItem* pOldItem;

    const SwFmtSurround& rOldSur = (const SwFmtSurround&)GetItemSet().Get(RES_SURROUND);
    SwFmtSurround aSur( rOldSur );

    SvxOpaqueItem aOp( RES_OPAQUE);

    if (!bDrawMode)
    {
        const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)GetItemSet().Get(RES_OPAQUE);
        aOp = rOpaque;
        aOp.SetValue(sal_True);
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
        if (m_pWrapTransparentCB->IsChecked() && !bDrawMode)
            aOp.SetValue(sal_False);
    }
    else if (m_pIdealWrapRB->IsChecked())
        aSur.SetSurround(SURROUND_IDEAL);

    aSur.SetAnchorOnly( m_pWrapAnchorOnlyCB->IsChecked() );
    sal_Bool bContour = m_pWrapOutlineCB->IsChecked() && m_pWrapOutlineCB->IsEnabled();
    aSur.SetContour( bContour );

    if ( bContour )
        aSur.SetOutside(m_pWrapOutsideCB->IsChecked());

    if(0 == (pOldItem = GetOldItem( rSet, RES_SURROUND )) ||
                aSur != *pOldItem )
    {
        rSet.Put(aSur);
        bModified = sal_True;
    }

    if (!bDrawMode)
    {
        if(0 == (pOldItem = GetOldItem( rSet, FN_OPAQUE )) ||
                    aOp != *pOldItem )
        {
            rSet.Put(aOp);
            bModified = sal_True;
        }
    }

    sal_Bool bTopMod = m_pTopMarginED->IsValueModified();
    sal_Bool bBottomMod = m_pBottomMarginED->IsValueModified();

    SvxULSpaceItem aUL( RES_UL_SPACE );
    aUL.SetUpper((sal_uInt16)m_pTopMarginED->Denormalize(m_pTopMarginED->GetValue(FUNIT_TWIP)));
    aUL.SetLower((sal_uInt16)m_pBottomMarginED->Denormalize(m_pBottomMarginED->GetValue(FUNIT_TWIP)));

    if ( bTopMod || bBottomMod )
    {
        if(0 == (pOldItem = GetOldItem(rSet, RES_UL_SPACE)) ||
                aUL != *pOldItem )
        {
            rSet.Put( aUL, RES_UL_SPACE );
            bModified = sal_True;
        }
    }

    sal_Bool bLeftMod = m_pLeftMarginED->IsValueModified();
    sal_Bool bRightMod = m_pRightMarginED->IsValueModified();

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    aLR.SetLeft((sal_uInt16)m_pLeftMarginED->Denormalize(m_pLeftMarginED->GetValue(FUNIT_TWIP)));
    aLR.SetRight((sal_uInt16)m_pRightMarginED->Denormalize(m_pRightMarginED->GetValue(FUNIT_TWIP)));

    if ( bLeftMod || bRightMod )
    {
        if( 0 == (pOldItem = GetOldItem(rSet, RES_LR_SPACE)) ||
                aLR != *pOldItem )
        {
            rSet.Put(aLR, RES_LR_SPACE);
            bModified = sal_True;
        }
    }

    if ( bDrawMode )
    {
        sal_Bool bChecked = m_pWrapTransparentCB->IsChecked() & m_pWrapTransparentCB->IsEnabled();
        if (m_pWrapTransparentCB->GetSavedValue() != bChecked)
            bModified |= 0 != rSet.Put(SfxInt16Item(FN_DRAW_WRAP_DLG, bChecked ? 0 : 1));
    }

    return bModified;
}

/*--------------------------------------------------------------------
    Description:    example update
 --------------------------------------------------------------------*/
void SwWrapTabPage::ActivatePage(const SfxItemSet& rSet)
{
    // anchor
    const SwFmtAnchor &rAnch = (const SwFmtAnchor&)rSet.Get(RES_ANCHOR);
    nAnchorId = rAnch.GetAnchorId();
    sal_Bool bEnable = (nAnchorId != FLY_AS_CHAR);

    if (!bDrawMode)
    {
        SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell() : pWrtSh;
        SwFlyFrmAttrMgr aMgr( bNew, pSh, (const SwAttrSet&)GetItemSet() );
        SvxSwFrameValidation aVal;

        // size
        const SwFmtFrmSize& rFrmSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);
        Size aSize = rFrmSize.GetSize();

        // margin
        const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)rSet.Get(RES_UL_SPACE);
        const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);
        nOldLeftMargin  = static_cast< sal_uInt16 >(rLR.GetLeft());
        nOldRightMargin = static_cast< sal_uInt16 >(rLR.GetRight());
        nOldUpperMargin = static_cast< sal_uInt16 >(rUL.GetUpper());
        nOldLowerMargin = static_cast< sal_uInt16 >(rUL.GetLower());

        // position
        const SwFmtHoriOrient& rHori = (const SwFmtHoriOrient&)rSet.Get(RES_HORI_ORIENT);
        const SwFmtVertOrient& rVert = (const SwFmtVertOrient&)rSet.Get(RES_VERT_ORIENT);

        aVal.nAnchorType = static_cast< sal_Int16 >(nAnchorId);
        aVal.bAutoHeight = rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE;
        aVal.bAutoWidth = rFrmSize.GetWidthSizeType() == ATT_MIN_SIZE;
        aVal.bMirror = rHori.IsPosToggle();
        // #i18732#
        aVal.bFollowTextFlow =
            static_cast<const SwFmtFollowTextFlow&>(rSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();

        aVal.nHoriOrient = (short)rHori.GetHoriOrient();
        aVal.nVertOrient = (short)rVert.GetVertOrient();

        aVal.nHPos = rHori.GetPos();
        aVal.nHRelOrient = rHori.GetRelationOrient();
        aVal.nVPos = rVert.GetPos();
        aVal.nVRelOrient = rVert.GetRelationOrient();

        if (rFrmSize.GetWidthPercent() && rFrmSize.GetWidthPercent() != 0xff)
            aSize.Width() = aSize.Width() * rFrmSize.GetWidthPercent() / 100;

        if (rFrmSize.GetHeightPercent() && rFrmSize.GetHeightPercent() != 0xff)
            aSize.Height() = aSize.Height() * rFrmSize.GetHeightPercent() / 100;

        aVal.nWidth  = aSize.Width();
        aVal.nHeight = aSize.Height();
        aFrmSize = aSize;

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

        RangeModifyHdl(m_pLeftMarginED);
        RangeModifyHdl(m_pTopMarginED);
    }

    const SwFmtSurround& rSurround = (const SwFmtSurround&)rSet.Get(RES_SURROUND);
    SwSurround nSur = rSurround.GetSurround();

    m_pWrapTransparentCB->Enable( bEnable && !bHtmlMode && nSur == SURROUND_THROUGHT );
    if(bHtmlMode)
    {
        const SwFmtHoriOrient& rHori = (const SwFmtHoriOrient&)rSet.Get(RES_HORI_ORIENT);
        sal_Int16 eHOrient = rHori.GetHoriOrient();
        sal_Int16 eHRelOrient = rHori.GetRelationOrient();
        m_pWrapOutlineCB->Hide();
        const bool bAllHtmlModes =
            ((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR)) &&
                            (eHOrient == text::HoriOrientation::RIGHT || eHOrient == text::HoriOrientation::LEFT);
        m_pWrapAnchorOnlyCB->Enable( bAllHtmlModes && nSur != SURROUND_NONE );
        m_pWrapOutsideCB->Hide();
        m_pIdealWrapRB->Enable( sal_False );


        m_pWrapTransparentCB->Enable( sal_False );
        m_pNoWrapRB->Enable( FLY_AT_PARA == nAnchorId );
        m_pWrapParallelRB->Enable( sal_False  );
        m_pWrapLeftRB->Enable
                    (  (FLY_AT_PARA == nAnchorId)
                    || (   (FLY_AT_CHAR == nAnchorId)
                        && (eHOrient == text::HoriOrientation::RIGHT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));
        m_pWrapRightRB->Enable
                    (  (FLY_AT_PARA == nAnchorId)
                    || (   (FLY_AT_CHAR == nAnchorId)
                        && (eHOrient == text::HoriOrientation::LEFT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));

        m_pWrapThroughRB->Enable
                (   (  (FLY_AT_PAGE == nAnchorId)
                    || (   (FLY_AT_CHAR == nAnchorId)
                        && (eHRelOrient != text::RelOrientation::PRINT_AREA))
                    || (FLY_AT_PARA == nAnchorId))
                && (eHOrient != text::HoriOrientation::RIGHT));
        if(m_pNoWrapRB->IsChecked() && !m_pNoWrapRB->IsEnabled())
        {
            if(m_pWrapThroughRB->IsEnabled())
                m_pWrapThroughRB->Check(sal_True);
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
                ((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR))
                && nSur != SURROUND_NONE );
    }
    ContourHdl(0);
}

int SwWrapTabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);

    return sal_True;
}

/*--------------------------------------------------------------------
    Description:    range check
 --------------------------------------------------------------------*/
IMPL_LINK( SwWrapTabPage, RangeModifyHdl, MetricField *, pEdit )
{
        sal_Int64 nValue = pEdit->GetValue();
        MetricField *pOpposite = 0;

        if (pEdit == m_pLeftMarginED)
            pOpposite = m_pRightMarginED;
        else if (pEdit == m_pRightMarginED)
            pOpposite = m_pLeftMarginED;
        else if (pEdit == m_pTopMarginED)
            pOpposite = m_pBottomMarginED;
        else if (pEdit == m_pBottomMarginED)
            pOpposite = m_pTopMarginED;

        OSL_ASSERT(pOpposite);

        if (pOpposite)
        {
            sal_Int64 nOpposite = pOpposite->GetValue();

            if (nValue + nOpposite > std::max(pEdit->GetMax(), pOpposite->GetMax()))
                pOpposite->SetValue(pOpposite->GetMax() - nValue);
        }

    return 0;
}

IMPL_LINK( SwWrapTabPage, WrapTypeHdl, RadioButton *, pBtn )
{
    sal_Bool bWrapThrough = (pBtn == m_pWrapThroughRB);
    m_pWrapTransparentCB->Enable( bWrapThrough && !bHtmlMode );
    bWrapThrough |= ( nAnchorId == FLY_AS_CHAR );
    m_pWrapOutlineCB->Enable( !bWrapThrough && pBtn != m_pNoWrapRB);
    m_pWrapOutsideCB->Enable( !bWrapThrough && m_pWrapOutlineCB->IsChecked() );
    m_pWrapAnchorOnlyCB->Enable(
        ((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR)) &&
        (pBtn != m_pNoWrapRB) );

    ContourHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwWrapTabPage, ContourHdl)
{
    sal_Bool bEnable = !(m_pWrapOutlineCB->IsChecked() && m_pWrapOutlineCB->IsEnabled());

    m_pWrapOutsideCB->Enable(!bEnable);

    bEnable =  !m_pWrapOutlineCB->IsChecked();
    if (bEnable == bContourImage) // so that it doesn't always flicker
    {
        bContourImage = !bEnable;
        ApplyImageList();
    }

    return 0;
}

sal_uInt16* SwWrapTabPage::GetRanges()
{
    return aWrapPageRg;
}

void SwWrapTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    SfxTabPage::DataChanged( rDCEvt );
}

void SwWrapTabPage::ApplyImageList()
{
    m_pWrapThroughRB->SetModeRadioImage(get<FixedImage>("imgthrough")->GetImage());
    sal_Bool bWrapOutline =  !m_pWrapOutlineCB->IsChecked();
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
