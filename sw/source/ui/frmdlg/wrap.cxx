/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "hintids.hxx"
#include <vcl/graph.hxx>
#include <svx/htmlmode.hxx>
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
    SfxSingleTabDialog(pParent, rSet, 0),
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

SwWrapTabPage::SwWrapTabPage(Window *pParent, const SfxItemSet &rSet) :
    SfxTabPage(pParent, SW_RES(TP_FRM_WRAP), rSet),

    aWrapFLC           (this, SW_RES(C_WRAP_FL)),
    aNoWrapRB           (this, SW_RES(RB_NO_WRAP)),
    aWrapLeftRB         (this, SW_RES(RB_WRAP_LEFT)),
    aWrapRightRB        (this, SW_RES(RB_WRAP_RIGHT)),
    aWrapParallelRB     (this, SW_RES(RB_WRAP_PARALLEL)),
    aWrapThroughRB      (this, SW_RES(RB_WRAP_THROUGH)),
    aIdealWrapRB        (this, SW_RES(RB_WRAP_IDEAL)),

    aMarginFL           (this, SW_RES(FL_MARGIN)),
    aLeftMarginFT       (this, SW_RES(FT_LEFT_MARGIN)),
    aLeftMarginED       (this, SW_RES(ED_LEFT_MARGIN)),
    aRightMarginFT      (this, SW_RES(FT_RIGHT_MARGIN)),
    aRightMarginED      (this, SW_RES(ED_RIGHT_MARGIN)),
    aTopMarginFT        (this, SW_RES(FT_TOP_MARGIN)),
    aTopMarginED        (this, SW_RES(ED_TOP_MARGIN)),
    aBottomMarginFT     (this, SW_RES(FT_BOTTOM_MARGIN)),
    aBottomMarginED     (this, SW_RES(ED_BOTTOM_MARGIN)),

    aOptionsSepFL       (this, SW_RES(FL_OPTION_SEP)),
    aOptionsFL          (this, SW_RES(FL_OPTION)),
    aWrapAnchorOnlyCB   (this, SW_RES(CB_ANCHOR_ONLY)),
    aWrapTransparentCB  (this, SW_RES(CB_TRANSPARENT)),
    aWrapOutlineCB      (this, SW_RES(CB_OUTLINE)),
    aWrapOutsideCB      (this, SW_RES(CB_ONLYOUTSIDE)),

    aWrapIL             (SW_RES(IL_WRAP)),

    nAnchorId(FLY_AT_PARA),
    nHtmlMode(0),

    pWrtSh(0),

    bFormat(sal_False),
    bNew(sal_True),
    bHtmlMode(sal_False),
    bContourImage(sal_False)

{
    FreeResource();
    SetExchangeSupport();

    Link aLk = LINK(this, SwWrapTabPage, RangeModifyHdl);
    aLeftMarginED.SetUpHdl(aLk);
    aLeftMarginED.SetDownHdl(aLk);
    aLeftMarginED.SetFirstHdl(aLk);
    aLeftMarginED.SetLastHdl(aLk);
    aLeftMarginED.SetLoseFocusHdl(aLk);

    aRightMarginED.SetUpHdl(aLk);
    aRightMarginED.SetDownHdl(aLk);
    aRightMarginED.SetFirstHdl(aLk);
    aRightMarginED.SetLastHdl(aLk);
    aRightMarginED.SetLoseFocusHdl(aLk);

    aTopMarginED.SetUpHdl(aLk);
    aTopMarginED.SetDownHdl(aLk);
    aTopMarginED.SetFirstHdl(aLk);
    aTopMarginED.SetLastHdl(aLk);
    aTopMarginED.SetLoseFocusHdl(aLk);

    aBottomMarginED.SetUpHdl(aLk);
    aBottomMarginED.SetDownHdl(aLk);
    aBottomMarginED.SetFirstHdl(aLk);
    aBottomMarginED.SetLastHdl(aLk);
    aBottomMarginED.SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwWrapTabPage, WrapTypeHdl);
    aNoWrapRB.SetClickHdl(aLk);
    aWrapLeftRB.SetClickHdl(aLk);
    aWrapRightRB.SetClickHdl(aLk);
    aWrapParallelRB.SetClickHdl(aLk);
    aWrapThroughRB.SetClickHdl(aLk);
    aIdealWrapRB.SetClickHdl(aLk);
    ApplyImageList();
    aWrapOutlineCB.SetClickHdl(LINK(this, SwWrapTabPage, ContourHdl));
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
        aWrapOutlineCB.Show();
        aWrapOutsideCB.Show();

        aWrapTransparentCB.Check( 0 == ((const SfxInt16Item&)rSet.Get(
                                        FN_DRAW_WRAP_DLG)).GetValue() );
        aWrapTransparentCB.SaveValue();
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
            aWrapOutlineCB.Show();
            aWrapOutsideCB.Show();
        }
    }

    nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;

    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    SetMetric( aLeftMarginED  , aMetric );
    SetMetric( aRightMarginED , aMetric );
    SetMetric( aTopMarginED   , aMetric );
    SetMetric( aBottomMarginED, aMetric );

    const SwFmtSurround& rSurround = (const SwFmtSurround&)rSet.Get(RES_SURROUND);

    SwSurround nSur = rSurround.GetSurround();
    const SwFmtAnchor &rAnch = (const SwFmtAnchor&)rSet.Get(RES_ANCHOR);
    nAnchorId = rAnch.GetAnchorId();

    if (((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR))
        && (nSur != SURROUND_NONE))
    {
        aWrapAnchorOnlyCB.Check( rSurround.IsAnchorOnly() );
    }
    else
    {
        aWrapAnchorOnlyCB.Enable( sal_False );
    }

    sal_Bool bContour = rSurround.IsContour();
    aWrapOutlineCB.Check( bContour );
    aWrapOutsideCB.Check( rSurround.IsOutside() );
    aWrapThroughRB.Enable(!aWrapOutlineCB.IsChecked());
    bContourImage = !bContour;

    ImageRadioButton* pBtn = NULL;

    switch (nSur)
    {
        case SURROUND_NONE:
        {
            pBtn = &aNoWrapRB;
            break;
        }

        case SURROUND_THROUGHT:
        {
            // transparent ?
            pBtn = &aWrapThroughRB;

            if (!bDrawMode)
            {
                const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)rSet.Get(RES_OPAQUE);
                aWrapTransparentCB.Check(!rOpaque.GetValue());
            }
            break;
        }

        case SURROUND_PARALLEL:
        {
            pBtn = &aWrapParallelRB;
            break;
        }

        case SURROUND_IDEAL:
        {
            pBtn = &aIdealWrapRB;
            break;
        }

        default:
        {
            if (nSur == SURROUND_LEFT)
                pBtn = &aWrapLeftRB;
            else if (nSur == SURROUND_RIGHT)
                pBtn = &aWrapRightRB;
        }
    }
    if (pBtn)
    {
        pBtn->Check();
        WrapTypeHdl(pBtn);
        // For character objects that currently are in passage, the default
        // "contour on" is prepared here, in case we switch to any other
        // passage later.
        if (bDrawMode && !aWrapOutlineCB.IsEnabled())
            aWrapOutlineCB.Check();
    }
    aWrapTransparentCB.Enable( pBtn == &aWrapThroughRB && !bHtmlMode );

    const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)rSet.Get(RES_UL_SPACE);
    const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);

    // gap to text
    aLeftMarginED.SetValue(aLeftMarginED.Normalize(rLR.GetLeft()), FUNIT_TWIP);
    aRightMarginED.SetValue(aRightMarginED.Normalize(rLR.GetRight()), FUNIT_TWIP);
    aTopMarginED.SetValue(aTopMarginED.Normalize(rUL.GetUpper()), FUNIT_TWIP);
    aBottomMarginED.SetValue(aBottomMarginED.Normalize(rUL.GetLower()), FUNIT_TWIP);

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

    if (aNoWrapRB.IsChecked())
        aSur.SetSurround(SURROUND_NONE);
    else if (aWrapLeftRB.IsChecked())
        aSur.SetSurround(SURROUND_LEFT);
    else if (aWrapRightRB.IsChecked())
        aSur.SetSurround(SURROUND_RIGHT);
    else if (aWrapParallelRB.IsChecked())
        aSur.SetSurround(SURROUND_PARALLEL);
    else if (aWrapThroughRB.IsChecked())
    {
        aSur.SetSurround(SURROUND_THROUGHT);
        if (aWrapTransparentCB.IsChecked() && !bDrawMode)
            aOp.SetValue(sal_False);
    }
    else if (aIdealWrapRB.IsChecked())
        aSur.SetSurround(SURROUND_IDEAL);

    aSur.SetAnchorOnly( aWrapAnchorOnlyCB.IsChecked() );
    sal_Bool bContour = aWrapOutlineCB.IsChecked() && aWrapOutlineCB.IsEnabled();
    aSur.SetContour( bContour );

    if ( bContour )
        aSur.SetOutside(aWrapOutsideCB.IsChecked());

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

    sal_Bool bTopMod = aTopMarginED.IsValueModified();
    sal_Bool bBottomMod = aBottomMarginED.IsValueModified();

    SvxULSpaceItem aUL( RES_UL_SPACE );
    aUL.SetUpper((sal_uInt16)aTopMarginED.Denormalize(aTopMarginED.GetValue(FUNIT_TWIP)));
    aUL.SetLower((sal_uInt16)aBottomMarginED.Denormalize(aBottomMarginED.GetValue(FUNIT_TWIP)));

    if ( bTopMod || bBottomMod )
    {
        if(0 == (pOldItem = GetOldItem(rSet, RES_UL_SPACE)) ||
                aUL != *pOldItem )
        {
            rSet.Put( aUL, RES_UL_SPACE );
            bModified = sal_True;
        }
    }

    sal_Bool bLeftMod = aLeftMarginED.IsValueModified();
    sal_Bool bRightMod = aRightMarginED.IsValueModified();

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    aLR.SetLeft((sal_uInt16)aLeftMarginED.Denormalize(aLeftMarginED.GetValue(FUNIT_TWIP)));
    aLR.SetRight((sal_uInt16)aRightMarginED.Denormalize(aRightMarginED.GetValue(FUNIT_TWIP)));

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
        sal_Bool bChecked = aWrapTransparentCB.IsChecked() & aWrapTransparentCB.IsEnabled();
        if (aWrapTransparentCB.GetSavedValue() != bChecked)
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

        aLeftMarginED.SetMax(aLeftMarginED.Normalize(nLeft), FUNIT_TWIP);
        aRightMarginED.SetMax(aRightMarginED.Normalize(nRight), FUNIT_TWIP);

        aTopMarginED.SetMax(aTopMarginED.Normalize(nTop), FUNIT_TWIP);
        aBottomMarginED.SetMax(aBottomMarginED.Normalize(nBottom), FUNIT_TWIP);

        RangeModifyHdl(&aLeftMarginED);
        RangeModifyHdl(&aTopMarginED);
    }

    const SwFmtSurround& rSurround = (const SwFmtSurround&)rSet.Get(RES_SURROUND);
    SwSurround nSur = rSurround.GetSurround();

    aWrapTransparentCB.Enable( bEnable && !bHtmlMode && nSur == SURROUND_THROUGHT );
    if(bHtmlMode)
    {
        const SwFmtHoriOrient& rHori = (const SwFmtHoriOrient&)rSet.Get(RES_HORI_ORIENT);
        sal_Int16 eHOrient = rHori.GetHoriOrient();
        sal_Int16 eHRelOrient = rHori.GetRelationOrient();
        aWrapOutlineCB.Hide();
        const bool bAllHtmlModes =
            ((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR)) &&
                            (eHOrient == text::HoriOrientation::RIGHT || eHOrient == text::HoriOrientation::LEFT);
        aWrapAnchorOnlyCB.Enable( bAllHtmlModes && nSur != SURROUND_NONE );
        aWrapOutsideCB.Hide();
        aIdealWrapRB.Enable( sal_False );


        aWrapTransparentCB.Enable( sal_False );
        aNoWrapRB.Enable( FLY_AT_PARA == nAnchorId );
        aWrapParallelRB.Enable( sal_False  );
        aWrapLeftRB       .Enable
                    (  (FLY_AT_PARA == nAnchorId)
                    || (   (FLY_AT_CHAR == nAnchorId)
                        && (eHOrient == text::HoriOrientation::RIGHT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));
        aWrapRightRB      .Enable
                    (  (FLY_AT_PARA == nAnchorId)
                    || (   (FLY_AT_CHAR == nAnchorId)
                        && (eHOrient == text::HoriOrientation::LEFT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));

        aWrapThroughRB.Enable
                (   (  (FLY_AT_PAGE == nAnchorId)
                    || (   (FLY_AT_CHAR == nAnchorId)
                        && (eHRelOrient != text::RelOrientation::PRINT_AREA))
                    || (FLY_AT_PARA == nAnchorId))
                && (eHOrient != text::HoriOrientation::RIGHT));
        if(aNoWrapRB.IsChecked() && !aNoWrapRB.IsEnabled())
        {
            if(aWrapThroughRB.IsEnabled())
                aWrapThroughRB.Check(sal_True);
            else if(aWrapLeftRB.IsEnabled())
                aWrapLeftRB.Check();
            else if(aWrapRightRB.IsEnabled())
                aWrapRightRB.Check();

        }
        if(aWrapLeftRB.IsChecked() && !aWrapLeftRB.IsEnabled())
        {
            if(aWrapRightRB.IsEnabled())
                aWrapRightRB.Check();
            else if(aWrapThroughRB.IsEnabled())
                aWrapThroughRB.Check();
        }
        if(aWrapRightRB.IsChecked() && !aWrapRightRB.IsEnabled())
        {
            if(aWrapLeftRB.IsEnabled())
                aWrapLeftRB.Check();
            else if(aWrapThroughRB.IsEnabled())
                aWrapThroughRB.Check();
        }
        if(aWrapThroughRB.IsChecked() && !aWrapThroughRB.IsEnabled())
            if(aNoWrapRB.IsEnabled())
                aNoWrapRB.Check();

        if(aWrapParallelRB.IsChecked() && !aWrapParallelRB.IsEnabled())
            aWrapThroughRB.Check();
    }
    else
    {
        aNoWrapRB.Enable( bEnable );
        aWrapLeftRB.Enable( bEnable );
        aWrapRightRB.Enable( bEnable );
        aIdealWrapRB.Enable( bEnable );
        aWrapThroughRB.Enable( bEnable );
        aWrapParallelRB.Enable( bEnable );
        aWrapAnchorOnlyCB.Enable(
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

        if (pEdit == &aLeftMarginED)
            pOpposite = &aRightMarginED;
        else if (pEdit == &aRightMarginED)
            pOpposite = &aLeftMarginED;
        else if (pEdit == &aTopMarginED)
            pOpposite = &aBottomMarginED;
        else if (pEdit == &aBottomMarginED)
            pOpposite = &aTopMarginED;

        OSL_ASSERT(pOpposite);

        if (pOpposite)
        {
            sal_Int64 nOpposite = pOpposite->GetValue();

            if (nValue + nOpposite > Max(pEdit->GetMax(), pOpposite->GetMax()))
                pOpposite->SetValue(pOpposite->GetMax() - nValue);
        }

    return 0;
}

IMPL_LINK( SwWrapTabPage, WrapTypeHdl, ImageRadioButton *, pBtn )
{
    sal_Bool bWrapThrough = (pBtn == &aWrapThroughRB);
    aWrapTransparentCB.Enable( bWrapThrough && !bHtmlMode );
    bWrapThrough |= ( nAnchorId == FLY_AS_CHAR );
    aWrapOutlineCB.Enable( !bWrapThrough && pBtn != &aNoWrapRB);
    aWrapOutsideCB.Enable( !bWrapThrough && aWrapOutlineCB.IsChecked() );
    aWrapAnchorOnlyCB.Enable(
        ((nAnchorId == FLY_AT_PARA) || (nAnchorId == FLY_AT_CHAR)) &&
        (pBtn != &aNoWrapRB) );

    ContourHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwWrapTabPage, ContourHdl)
{
    sal_Bool bEnable = !(aWrapOutlineCB.IsChecked() && aWrapOutlineCB.IsEnabled());

    aWrapOutsideCB.Enable(!bEnable);

    bEnable =  !aWrapOutlineCB.IsChecked();
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
    ImageList& rImgLst = aWrapIL;

    aWrapThroughRB.SetModeRadioImage(rImgLst.GetImage(IMG_THROUGH));
    sal_Bool bWrapOutline =  !aWrapOutlineCB.IsChecked();
    if(bWrapOutline)
    {
        aNoWrapRB.SetModeRadioImage(       rImgLst.GetImage( IMG_NONE     ));
        aWrapLeftRB.SetModeRadioImage(     rImgLst.GetImage( IMG_LEFT     ));
        aWrapRightRB.SetModeRadioImage(    rImgLst.GetImage( IMG_RIGHT    ));
        aWrapParallelRB.SetModeRadioImage( rImgLst.GetImage( IMG_PARALLEL ));
        aIdealWrapRB.SetModeRadioImage(    rImgLst.GetImage( IMG_IDEAL    ));
    }
    else
    {
        aNoWrapRB.SetModeRadioImage(       rImgLst.GetImage( IMG_KON_NONE     ));
        aWrapLeftRB.SetModeRadioImage(     rImgLst.GetImage( IMG_KON_LEFT     ));
        aWrapRightRB.SetModeRadioImage(    rImgLst.GetImage( IMG_KON_RIGHT    ));
        aWrapParallelRB.SetModeRadioImage( rImgLst.GetImage( IMG_KON_PARALLEL ));
        aIdealWrapRB.SetModeRadioImage(    rImgLst.GetImage( IMG_KON_IDEAL    ));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
