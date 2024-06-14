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
#include <vcl/graph.hxx>

#include <sfx2/htmlmode.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtfollowtextflow.hxx>
#include <svtools/unitconv.hxx>
#include <svx/swframevalidation.hxx>

#include <cmdid.h>
#include <docsh.hxx>
#include <uitool.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <fmtsrnd.hxx>
#include <frmmgr.hxx>
#include <wrap.hxx>
#include <bitmaps.hlst>
#include <fmtwrapinfluenceonobjpos.hxx>

using namespace ::com::sun::star;

const WhichRangesContainer SwWrapTabPage::s_aWrapPageRg(svl::Items<
    RES_LR_SPACE, RES_UL_SPACE,
    RES_PRINT, RES_PRINT,
    RES_PROTECT, RES_SURROUND
>);

SwWrapDlg::SwWrapDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtShell, bool bDrawMode)
    : SwWrapDlgBase{rSet}, // this is an async dialog, so we need to store a copy of the rSet
      SfxSingleTabDialogController(pParent, &maInputSet, u"modules/swriter/ui/wrapdialog.ui"_ustr, u"WrapDialog"_ustr)
{
    // create TabPage
    auto xNewPage = SwWrapTabPage::Create(get_content_area(), this, &maInputSet);
    SwWrapTabPage* pWrapPage = static_cast<SwWrapTabPage*>(xNewPage.get());
    pWrapPage->SetFormatUsed(false, bDrawMode);
    pWrapPage->SetShell(pWrtShell);
    SetTabPage(std::move(xNewPage));
}

SwWrapTabPage::SwWrapTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/wrappage.ui"_ustr, u"WrapPage"_ustr, &rSet)
    , m_nAnchorId(RndStdIds::FLY_AT_PARA)
    , m_nHtmlMode(0)
    , m_pWrtSh(nullptr)
    , m_bFormat(false)
    , m_bNew(true)
    , m_bHtmlMode(false)
    , m_bDrawMode(false)
    , m_bContourImage(false)
    , m_xNoWrapImg(m_xBuilder->weld_image(u"noneimg"_ustr))
    , m_xNoWrapRB(m_xBuilder->weld_radio_button(u"none"_ustr))
    , m_xWrapLeftImg(m_xBuilder->weld_image(u"beforeimg"_ustr))
    , m_xWrapLeftRB(m_xBuilder->weld_radio_button(u"before"_ustr))
    , m_xWrapRightImg(m_xBuilder->weld_image(u"afterimg"_ustr))
    , m_xWrapRightRB(m_xBuilder->weld_radio_button(u"after"_ustr))
    , m_xWrapParallelImg(m_xBuilder->weld_image(u"parallelimg"_ustr))
    , m_xWrapParallelRB(m_xBuilder->weld_radio_button(u"parallel"_ustr))
    , m_xWrapThroughImg(m_xBuilder->weld_image(u"throughimg"_ustr))
    , m_xWrapThroughRB(m_xBuilder->weld_radio_button(u"through"_ustr))
    , m_xIdealWrapImg(m_xBuilder->weld_image(u"optimalimg"_ustr))
    , m_xIdealWrapRB(m_xBuilder->weld_radio_button(u"optimal"_ustr))
    , m_xLeftMarginED(m_xBuilder->weld_metric_spin_button(u"left"_ustr, FieldUnit::CM))
    , m_xRightMarginED(m_xBuilder->weld_metric_spin_button(u"right"_ustr, FieldUnit::CM))
    , m_xTopMarginED(m_xBuilder->weld_metric_spin_button(u"top"_ustr, FieldUnit::CM))
    , m_xBottomMarginED(m_xBuilder->weld_metric_spin_button(u"bottom"_ustr, FieldUnit::CM))
    , m_xWrapAnchorOnlyCB(m_xBuilder->weld_check_button(u"anchoronly"_ustr))
    , m_xWrapTransparentCB(m_xBuilder->weld_check_button(u"transparent"_ustr))
    , m_xWrapOutlineCB(m_xBuilder->weld_check_button(u"outline"_ustr))
    , m_xWrapOutsideCB(m_xBuilder->weld_check_button(u"outside"_ustr))
    , m_xAllowOverlapCB(m_xBuilder->weld_check_button(u"allowoverlap"_ustr))
{
    SetExchangeSupport();

    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwWrapTabPage, RangeModifyHdl);
    m_xLeftMarginED->connect_value_changed(aLk);
    m_xRightMarginED->connect_value_changed(aLk);
    m_xTopMarginED->connect_value_changed(aLk);
    m_xBottomMarginED->connect_value_changed(aLk);

    Link<weld::Toggleable&,void> aLk2 = LINK(this, SwWrapTabPage, WrapTypeHdl);
    m_xNoWrapRB->connect_toggled(aLk2);
    m_xWrapLeftRB->connect_toggled(aLk2);
    m_xWrapRightRB->connect_toggled(aLk2);
    m_xWrapParallelRB->connect_toggled(aLk2);
    m_xWrapThroughRB->connect_toggled(aLk2);
    m_xIdealWrapRB->connect_toggled(aLk2);
    SetImages();
    m_xWrapOutlineCB->connect_toggled(LINK(this, SwWrapTabPage, ContourHdl));
}

SwWrapTabPage::~SwWrapTabPage()
{
}

std::unique_ptr<SfxTabPage> SwWrapTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet)
{
    return std::make_unique<SwWrapTabPage>(pPage, pController, *rSet);
}

void SwWrapTabPage::Reset(const SfxItemSet *rSet)
{
    // contour for Draw, Graphic and OLE (Insert/Graphic/Properties still missing!)
    if( m_bDrawMode )
    {
        m_xWrapOutlineCB->show();
        m_xWrapOutsideCB->show();

        m_xWrapTransparentCB->set_active( 0 == rSet->Get(FN_DRAW_WRAP_DLG).GetValue() );
        m_xWrapTransparentCB->save_state();
    }
    else
    {
        bool bShowCB = m_bFormat;
        if( !m_bFormat )
        {
            SelectionType nSelType = m_pWrtSh->GetSelectionType();
            if( ( nSelType & SelectionType::Graphic ) ||
                ( nSelType & SelectionType::Ole && GraphicType::NONE !=
                            m_pWrtSh->GetIMapGraphic().GetType() ))
                bShowCB = true;
        }
        if( bShowCB )
        {
            m_xWrapOutlineCB->show();
            m_xWrapOutsideCB->show();
        }
    }

    m_nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    m_bHtmlMode = (m_nHtmlMode & HTMLMODE_ON) != 0;

    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    SetFieldUnit(*m_xLeftMarginED, aMetric);
    SetFieldUnit(*m_xRightMarginED, aMetric);
    SetFieldUnit(*m_xTopMarginED, aMetric);
    SetFieldUnit(*m_xBottomMarginED, aMetric);

    const SwFormatSurround& rSurround = rSet->Get(RES_SURROUND);

    css::text::WrapTextMode nSur = rSurround.GetSurround();
    const SwFormatAnchor &rAnch = rSet->Get(RES_ANCHOR);
    m_nAnchorId = rAnch.GetAnchorId();

    if (((m_nAnchorId == RndStdIds::FLY_AT_PARA) || (m_nAnchorId == RndStdIds::FLY_AT_CHAR))
        && (nSur != css::text::WrapTextMode_NONE))
    {
        m_xWrapAnchorOnlyCB->set_active(rSurround.IsAnchorOnly());
    }
    else
    {
        m_xWrapAnchorOnlyCB->set_sensitive(false);
    }

    const bool bContour = rSurround.IsContour();
    m_xWrapOutlineCB->set_active(bContour);
    m_xWrapOutsideCB->set_active(rSurround.IsOutside());
    m_xWrapThroughRB->set_sensitive(!m_xWrapOutlineCB->get_active());
    m_bContourImage = !bContour;

    weld::RadioButton* pBtn = nullptr;

    switch (nSur)
    {
        case css::text::WrapTextMode_NONE:
        {
            pBtn = m_xNoWrapRB.get();
            break;
        }

        case css::text::WrapTextMode_THROUGH:
        {
            // transparent ?
            pBtn = m_xWrapThroughRB.get();

            if (!m_bDrawMode)
            {
                const SvxOpaqueItem& rOpaque = rSet->Get(RES_OPAQUE);
                m_xWrapTransparentCB->set_active(!rOpaque.GetValue());
            }
            break;
        }

        case css::text::WrapTextMode_PARALLEL:
        {
            pBtn = m_xWrapParallelRB.get();
            break;
        }

        case css::text::WrapTextMode_DYNAMIC:
        {
            pBtn = m_xIdealWrapRB.get();
            break;
        }

        default:
        {
            if (nSur == css::text::WrapTextMode_LEFT)
                pBtn = m_xWrapLeftRB.get();
            else if (nSur == css::text::WrapTextMode_RIGHT)
                pBtn = m_xWrapRightRB.get();
        }
    }
    if (pBtn)
    {
        pBtn->set_active(true);
        WrapTypeHdl(*pBtn);
        // For character objects that currently are in passage, the default
        // "contour on" is prepared here, in case we switch to any other
        // passage later.
        if (m_bDrawMode && !m_xWrapOutlineCB->get_sensitive())
            m_xWrapOutlineCB->set_active(true);
    }
    m_xWrapTransparentCB->set_sensitive(pBtn == m_xWrapThroughRB.get() && !m_bHtmlMode);

    const SvxULSpaceItem& rUL = rSet->Get(RES_UL_SPACE);
    const SvxLRSpaceItem& rLR = rSet->Get(RES_LR_SPACE);

    // gap to text
    m_xLeftMarginED->set_value(m_xLeftMarginED->normalize(rLR.GetLeft()), FieldUnit::TWIP);
    m_xRightMarginED->set_value(m_xRightMarginED->normalize(rLR.GetRight()), FieldUnit::TWIP);
    m_xTopMarginED->set_value(m_xTopMarginED->normalize(rUL.GetUpper()), FieldUnit::TWIP);
    m_xBottomMarginED->set_value(m_xBottomMarginED->normalize(rUL.GetLower()), FieldUnit::TWIP);

    m_xLeftMarginED->save_value();
    m_xRightMarginED->save_value();
    m_xTopMarginED->save_value();
    m_xBottomMarginED->save_value();

    ContourHdl(*m_xWrapOutlineCB);

    const SwFormatWrapInfluenceOnObjPos& rInfluence = rSet->Get(RES_WRAP_INFLUENCE_ON_OBJPOS);
    m_xAllowOverlapCB->set_active(rInfluence.GetAllowOverlap());

    ActivatePage( *rSet );
}

// stuff attributes into the set when OK
bool SwWrapTabPage::FillItemSet(SfxItemSet *rSet)
{
    bool bModified = false;
    const SfxPoolItem* pOldItem;
    const SwFormatSurround& rOldSur = GetItemSet().Get(RES_SURROUND);
    SwFormatSurround aSur( rOldSur );

    std::shared_ptr<SvxOpaqueItem> aOp(std::make_shared<SvxOpaqueItem>(RES_OPAQUE));

    if (!m_bDrawMode)
    {
        aOp.reset(GetItemSet().Get(RES_OPAQUE).Clone());
        aOp->SetValue(true);
    }

    if (m_xNoWrapRB->get_active())
        aSur.SetSurround(css::text::WrapTextMode_NONE);
    else if (m_xWrapLeftRB->get_active())
        aSur.SetSurround(css::text::WrapTextMode_LEFT);
    else if (m_xWrapRightRB->get_active())
        aSur.SetSurround(css::text::WrapTextMode_RIGHT);
    else if (m_xWrapParallelRB->get_active())
        aSur.SetSurround(css::text::WrapTextMode_PARALLEL);
    else if (m_xWrapThroughRB->get_active())
    {
        aSur.SetSurround(css::text::WrapTextMode_THROUGH);
        if (m_xWrapTransparentCB->get_active() && !m_bDrawMode)
            aOp->SetValue(false);
    }
    else if (m_xIdealWrapRB->get_active())
        aSur.SetSurround(css::text::WrapTextMode_DYNAMIC);

    aSur.SetAnchorOnly( m_xWrapAnchorOnlyCB->get_active() );
    bool bContour = m_xWrapOutlineCB->get_active() && m_xWrapOutlineCB->get_sensitive();
    aSur.SetContour( bContour );

    if ( bContour )
        aSur.SetOutside(m_xWrapOutsideCB->get_active());

    if(nullptr == (pOldItem = GetOldItem( *rSet, RES_SURROUND )) ||
                aSur != *pOldItem )
    {
        rSet->Put(aSur);
        bModified = true;
    }

    if (!m_bDrawMode)
    {
        if(nullptr == (pOldItem = GetOldItem( *rSet, FN_OPAQUE )) ||
                    *aOp != *pOldItem )
        {
            rSet->Put(*aOp);
            bModified = true;
        }
    }

    bool bTopMod = m_xTopMarginED->get_value_changed_from_saved();
    bool bBottomMod = m_xBottomMarginED->get_value_changed_from_saved();

    SvxULSpaceItem aUL( RES_UL_SPACE );
    aUL.SetUpper(o3tl::narrowing<sal_uInt16>(m_xTopMarginED->denormalize(m_xTopMarginED->get_value(FieldUnit::TWIP))));
    aUL.SetLower(o3tl::narrowing<sal_uInt16>(m_xBottomMarginED->denormalize(m_xBottomMarginED->get_value(FieldUnit::TWIP))));

    if ( bTopMod || bBottomMod )
    {
        if(nullptr == (pOldItem = GetOldItem(*rSet, RES_UL_SPACE)) ||
                aUL != *pOldItem )
        {
            rSet->Put( aUL );
            bModified = true;
        }
    }

    bool bLeftMod = m_xLeftMarginED->get_value_changed_from_saved();
    bool bRightMod = m_xRightMarginED->get_value_changed_from_saved();

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    aLR.SetLeft(o3tl::narrowing<sal_uInt16>(m_xLeftMarginED->denormalize(m_xLeftMarginED->get_value(FieldUnit::TWIP))));
    aLR.SetRight(o3tl::narrowing<sal_uInt16>(m_xRightMarginED->denormalize(m_xRightMarginED->get_value(FieldUnit::TWIP))));

    if ( bLeftMod || bRightMod )
    {
        if( nullptr == (pOldItem = GetOldItem(*rSet, RES_LR_SPACE)) ||
                aLR != *pOldItem )
        {
            rSet->Put(aLR);
            bModified = true;
        }
    }

    if ( m_bDrawMode )
    {
        bool bChecked = m_xWrapTransparentCB->get_active() && m_xWrapTransparentCB->get_sensitive();
        if ((m_xWrapTransparentCB->get_saved_state() == TRISTATE_TRUE) != bChecked)
            bModified |= nullptr != rSet->Put(SfxInt16Item(FN_DRAW_WRAP_DLG, bChecked ? 0 : 1));
    }

    const SwFormatWrapInfluenceOnObjPos& rOldInfluence
        = GetItemSet().Get(RES_WRAP_INFLUENCE_ON_OBJPOS);
    SwFormatWrapInfluenceOnObjPos aInfluence(rOldInfluence);
    aInfluence.SetAllowOverlap(m_xAllowOverlapCB->get_active());

    pOldItem = GetOldItem(*rSet, RES_WRAP_INFLUENCE_ON_OBJPOS);
    if (!pOldItem || aInfluence != *pOldItem)
    {
        rSet->Put(aInfluence);
        bModified = true;
    }

    return bModified;
}

// example update
void SwWrapTabPage::ActivatePage(const SfxItemSet& rSet)
{
    // anchor
    const SwFormatAnchor &rAnch = rSet.Get(RES_ANCHOR);
    m_nAnchorId = rAnch.GetAnchorId();
    bool bEnable = (m_nAnchorId != RndStdIds::FLY_AS_CHAR);

    SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell() : m_pWrtSh;
    if (pSh && !m_bDrawMode)
    {
        SwFlyFrameAttrMgr aMgr( m_bNew, pSh, GetItemSet() );
        SvxSwFrameValidation aVal;

        // size
        const SwFormatFrameSize& rFrameSize = rSet.Get(RES_FRM_SIZE);
        Size aSize = rFrameSize.GetSize();

        // position
        const SwFormatHoriOrient& rHori = rSet.Get(RES_HORI_ORIENT);
        const SwFormatVertOrient& rVert = rSet.Get(RES_VERT_ORIENT);

        aVal.nAnchorType = m_nAnchorId;
        aVal.bAutoHeight = rFrameSize.GetHeightSizeType() == SwFrameSize::Minimum;
        aVal.bMirror = rHori.IsPosToggle();
        // #i18732#
        aVal.bFollowTextFlow = rSet.Get(RES_FOLLOW_TEXT_FLOW).GetValue();

        aVal.nHoriOrient = static_cast<short>(rHori.GetHoriOrient());
        aVal.nVertOrient = static_cast<short>(rVert.GetVertOrient());

        aVal.nHPos = rHori.GetPos();
        aVal.nHRelOrient = rHori.GetRelationOrient();
        aVal.nVPos = rVert.GetPos();
        aVal.nVRelOrient = rVert.GetRelationOrient();

        if (rFrameSize.GetWidthPercent() && rFrameSize.GetWidthPercent() != SwFormatFrameSize::SYNCED)
            aSize.setWidth( aSize.Width() * rFrameSize.GetWidthPercent() / 100 );

        if (rFrameSize.GetHeightPercent() && rFrameSize.GetHeightPercent() != SwFormatFrameSize::SYNCED)
            aSize.setHeight( aSize.Height() * rFrameSize.GetHeightPercent() / 100 );

        aVal.nWidth  = aSize.Width();
        aVal.nHeight = aSize.Height();

        aMgr.ValidateMetrics(aVal, nullptr);

        SwTwips nLeft;
        SwTwips nRight;
        SwTwips nTop;
        SwTwips nBottom;

        nLeft   = aVal.nHPos - aVal.nMinHPos;
        nRight  = aVal.nMaxWidth - aVal.nWidth;
        nTop    = aVal.nVPos - aVal.nMinVPos;
        nBottom = aVal.nMaxHeight - aVal.nHeight;

        {
            if (aVal.nAnchorType == RndStdIds::FLY_AS_CHAR)
            {
                nLeft = nRight;

                if (aVal.nVPos < 0)
                {
                    if (aVal.nVPos <= aVal.nMaxHeight)
                        nTop = aVal.nMaxVPos - aVal.nHeight;
                    else
                        nTop = 0; // no passage
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

        m_xLeftMarginED->set_max(m_xLeftMarginED->normalize(nLeft), FieldUnit::TWIP);
        m_xRightMarginED->set_max(m_xRightMarginED->normalize(nRight), FieldUnit::TWIP);

        m_xTopMarginED->set_max(m_xTopMarginED->normalize(nTop), FieldUnit::TWIP);
        m_xBottomMarginED->set_max(m_xBottomMarginED->normalize(nBottom), FieldUnit::TWIP);

        RangeModifyHdl(*m_xLeftMarginED);
        RangeModifyHdl(*m_xTopMarginED);
    }

    const SwFormatSurround& rSurround = rSet.Get(RES_SURROUND);
    css::text::WrapTextMode nSur = rSurround.GetSurround();

    m_xWrapTransparentCB->set_sensitive(bEnable && !m_bHtmlMode && nSur == css::text::WrapTextMode_THROUGH);
    if(m_bHtmlMode)
    {
        const SwFormatHoriOrient& rHori = rSet.Get(RES_HORI_ORIENT);
        sal_Int16 eHOrient = rHori.GetHoriOrient();
        sal_Int16 eHRelOrient = rHori.GetRelationOrient();
        m_xWrapOutlineCB->hide();
        const bool bAllHtmlModes =
            ((m_nAnchorId == RndStdIds::FLY_AT_PARA) || (m_nAnchorId == RndStdIds::FLY_AT_CHAR)) &&
                            (eHOrient == text::HoriOrientation::RIGHT || eHOrient == text::HoriOrientation::LEFT);
        m_xWrapAnchorOnlyCB->set_sensitive(bAllHtmlModes && nSur != css::text::WrapTextMode_NONE);
        m_xWrapOutsideCB->hide();
        m_xIdealWrapRB->set_sensitive(false);

        m_xWrapTransparentCB->set_sensitive(false);
        m_xNoWrapRB->set_sensitive(RndStdIds::FLY_AT_PARA == m_nAnchorId);
        m_xWrapParallelRB->set_sensitive(false);
        m_xWrapLeftRB->set_sensitive
                    (  (RndStdIds::FLY_AT_PARA == m_nAnchorId)
                    || (   (RndStdIds::FLY_AT_CHAR == m_nAnchorId)
                        && (eHOrient == text::HoriOrientation::RIGHT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));
        m_xWrapRightRB->set_sensitive
                    (  (RndStdIds::FLY_AT_PARA == m_nAnchorId)
                    || (   (RndStdIds::FLY_AT_CHAR == m_nAnchorId)
                        && (eHOrient == text::HoriOrientation::LEFT)
                        && (eHRelOrient == text::RelOrientation::PRINT_AREA)));

        m_xWrapThroughRB->set_sensitive
                (   (  (RndStdIds::FLY_AT_PAGE == m_nAnchorId)
                    || (   (RndStdIds::FLY_AT_CHAR == m_nAnchorId)
                        && (eHRelOrient != text::RelOrientation::PRINT_AREA))
                    || (RndStdIds::FLY_AT_PARA == m_nAnchorId))
                && (eHOrient != text::HoriOrientation::RIGHT));
        if (m_xNoWrapRB->get_active() && !m_xNoWrapRB->get_sensitive())
        {
            if(m_xWrapThroughRB->get_sensitive())
                m_xWrapThroughRB->set_active(true);
            else if(m_xWrapLeftRB->get_sensitive())
                m_xWrapLeftRB->set_active(true);
            else if(m_xWrapRightRB->get_sensitive())
                m_xWrapRightRB->set_active(true);

        }
        if (m_xWrapLeftRB->get_active() && !m_xWrapLeftRB->get_sensitive())
        {
            if(m_xWrapRightRB->get_sensitive())
                m_xWrapRightRB->set_active(true);
            else if(m_xWrapThroughRB->get_sensitive())
                m_xWrapThroughRB->set_active(true);
        }
        if (m_xWrapRightRB->get_active() && !m_xWrapRightRB->get_sensitive())
        {
            if(m_xWrapLeftRB->get_sensitive())
                m_xWrapLeftRB->set_active(true);
            else if(m_xWrapThroughRB->get_sensitive())
                m_xWrapThroughRB->set_active(true);
        }
        if (m_xWrapThroughRB->get_active() && !m_xWrapThroughRB->get_sensitive())
            if(m_xNoWrapRB->get_sensitive())
                m_xNoWrapRB->set_active(true);

        if (m_xWrapParallelRB->get_active() && !m_xWrapParallelRB->get_sensitive())
            m_xWrapThroughRB->set_active(true);
    }
    else
    {
        m_xNoWrapRB->set_sensitive(bEnable);
        m_xWrapLeftRB->set_sensitive(bEnable);
        m_xWrapRightRB->set_sensitive(bEnable);
        m_xIdealWrapRB->set_sensitive(bEnable);
        m_xWrapThroughRB->set_sensitive(bEnable);
        m_xWrapParallelRB->set_sensitive(bEnable);
        m_xWrapAnchorOnlyCB->set_sensitive(
                ((m_nAnchorId == RndStdIds::FLY_AT_PARA) || (m_nAnchorId == RndStdIds::FLY_AT_CHAR))
                && nSur != css::text::WrapTextMode_NONE );
    }
    ContourHdl(*m_xWrapOutlineCB);
}

DeactivateRC SwWrapTabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

IMPL_LINK(SwWrapTabPage, RangeModifyHdl, weld::MetricSpinButton&, rEdit, void)
{
    auto nValue = rEdit.get_value(FieldUnit::NONE);
    weld::MetricSpinButton* pOpposite = nullptr;
    if (&rEdit == m_xLeftMarginED.get())
        pOpposite = m_xRightMarginED.get();
    else if (&rEdit == m_xRightMarginED.get())
        pOpposite = m_xLeftMarginED.get();
    else if (&rEdit == m_xTopMarginED.get())
        pOpposite = m_xBottomMarginED.get();
    else if (&rEdit == m_xBottomMarginED.get())
        pOpposite = m_xTopMarginED.get();

    assert(pOpposite);

    if (pOpposite)
    {
        auto nOpposite = pOpposite->get_value(FieldUnit::NONE);

        if (nValue + nOpposite > std::max(rEdit.get_max(FieldUnit::NONE), pOpposite->get_max(FieldUnit::NONE)))
            pOpposite->set_value(pOpposite->get_max(FieldUnit::NONE) - nValue, FieldUnit::NONE);
    }
}

IMPL_LINK_NOARG(SwWrapTabPage, WrapTypeHdl, weld::Toggleable&, void)
{
    bool bWrapThrough = m_xWrapThroughRB->get_active();
    m_xWrapTransparentCB->set_sensitive(bWrapThrough && !m_bHtmlMode);
    bWrapThrough |= ( m_nAnchorId == RndStdIds::FLY_AS_CHAR );
    m_xWrapOutlineCB->set_sensitive(!bWrapThrough && !m_xNoWrapRB->get_active());
    m_xWrapOutsideCB->set_sensitive(!bWrapThrough && m_xWrapOutlineCB->get_active());
    m_xWrapAnchorOnlyCB->set_sensitive(
        ((m_nAnchorId == RndStdIds::FLY_AT_PARA) || (m_nAnchorId == RndStdIds::FLY_AT_CHAR)) &&
        (!m_xNoWrapRB->get_active()) );

    ContourHdl(*m_xWrapOutlineCB);
}

IMPL_LINK_NOARG(SwWrapTabPage, ContourHdl, weld::Toggleable&, void)
{
    bool bEnable = !(m_xWrapOutlineCB->get_active() && m_xWrapOutlineCB->get_sensitive());

    m_xWrapOutsideCB->set_sensitive(!bEnable);

    bEnable =  !m_xWrapOutlineCB->get_active();
    if (bEnable == m_bContourImage) // so that it doesn't always flicker
    {
        m_bContourImage = !bEnable;
        SetImages();
    }
}

void SwWrapTabPage::SetImages()
{
    m_xWrapThroughImg->set_from_icon_name(RID_BMP_WRAP_THROUGH);
    bool bWrapOutline =  !m_xWrapOutlineCB->get_active();
    if (bWrapOutline)
    {
        m_xNoWrapImg->set_from_icon_name(RID_BMP_WRAP_NONE);
        m_xWrapLeftImg->set_from_icon_name(RID_BMP_WRAP_LEFT);
        m_xWrapRightImg->set_from_icon_name(RID_BMP_WRAP_RIGHT);
        m_xWrapParallelImg->set_from_icon_name(RID_BMP_WRAP_PARALLEL);
        m_xIdealWrapImg->set_from_icon_name(RID_BMP_WRAP_IDEAL);
    }
    else
    {
        m_xNoWrapImg->set_from_icon_name(RID_BMP_WRAP_CONTOUR_NONE);
        m_xWrapLeftImg->set_from_icon_name(RID_BMP_WRAP_CONTOUR_LEFT);
        m_xWrapRightImg->set_from_icon_name(RID_BMP_WRAP_CONTOUR_RIGHT);
        m_xWrapParallelImg->set_from_icon_name(RID_BMP_WRAP_CONTOUR_PARALLEL);
        m_xIdealWrapImg->set_from_icon_name(RID_BMP_WRAP_CONTOUR_IDEAL);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
