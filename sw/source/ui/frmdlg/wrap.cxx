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
#include <vcl/settings.hxx>

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
#include <frmatr.hxx>
#include <frmmgr.hxx>
#include <globals.hrc>
#include <wrap.hxx>
#include <bitmaps.hlst>

using namespace ::com::sun::star;

const sal_uInt16 SwWrapTabPage::m_aWrapPageRg[] = {
    RES_LR_SPACE, RES_UL_SPACE,
    RES_PROTECT, RES_SURROUND,
    RES_PRINT, RES_PRINT,
    0
};

SwWrapDlg::SwWrapDlg(weld::Window* pParent, SfxItemSet& rSet, SwWrtShell* pWrtShell, bool bDrawMode)
    : SfxSingleTabDialogController(pParent, rSet, "modules/swriter/ui/wrapdialog.ui", "WrapDialog")
{
    // create TabPage
    TabPageParent pPageParent(get_content_area(), this);
    VclPtr<SwWrapTabPage> xNewPage = static_cast<SwWrapTabPage*>(SwWrapTabPage::Create(pPageParent, &rSet).get());
    xNewPage->SetFormatUsed(false, bDrawMode);
    xNewPage->SetShell(pWrtShell);
    SetTabPage(xNewPage);
}

SwWrapTabPage::SwWrapTabPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/wrappage.ui", "WrapPage", &rSet)
    , m_nAnchorId(RndStdIds::FLY_AT_PARA)
    , m_nHtmlMode(0)
    , m_pWrtSh(nullptr)
    , m_bFormat(false)
    , m_bNew(true)
    , m_bHtmlMode(false)
    , m_bDrawMode(false)
    , m_bContourImage(false)
    , m_xNoWrapRB(m_xBuilder->weld_radio_button("none"))
    , m_xWrapLeftRB(m_xBuilder->weld_radio_button("before"))
    , m_xWrapRightRB(m_xBuilder->weld_radio_button("after"))
    , m_xWrapParallelRB(m_xBuilder->weld_radio_button("parallel"))
    , m_xWrapThroughRB(m_xBuilder->weld_radio_button("through"))
    , m_xIdealWrapRB(m_xBuilder->weld_radio_button("optimal"))
    , m_xLeftMarginED(m_xBuilder->weld_metric_spin_button("left", FieldUnit::CM))
    , m_xRightMarginED(m_xBuilder->weld_metric_spin_button("right", FieldUnit::CM))
    , m_xTopMarginED(m_xBuilder->weld_metric_spin_button("top", FieldUnit::CM))
    , m_xBottomMarginED(m_xBuilder->weld_metric_spin_button("bottom", FieldUnit::CM))
    , m_xWrapAnchorOnlyCB(m_xBuilder->weld_check_button("anchoronly"))
    , m_xWrapTransparentCB(m_xBuilder->weld_check_button("transparent"))
    , m_xWrapOutlineCB(m_xBuilder->weld_check_button("outline"))
    , m_xWrapOutsideCB(m_xBuilder->weld_check_button("outside"))
{
    SetExchangeSupport();

    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwWrapTabPage, RangeModifyHdl);
    m_xLeftMarginED->connect_value_changed(aLk);
    m_xRightMarginED->connect_value_changed(aLk);
    m_xTopMarginED->connect_value_changed(aLk);
    m_xBottomMarginED->connect_value_changed(aLk);

    Link<weld::ToggleButton&,void> aLk2 = LINK(this, SwWrapTabPage, WrapTypeHdl);
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

VclPtr<SfxTabPage> SwWrapTabPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwWrapTabPage>::Create(pParent, *rSet);
}

void SwWrapTabPage::Reset(const SfxItemSet *rSet)
{
    // contour for Draw, Graphic and OLE (Insert/Graphic/Properties still missing!)
    if( m_bDrawMode )
    {
        m_xWrapOutlineCB->show();
        m_xWrapOutsideCB->show();

        m_xWrapTransparentCB->set_active( 0 == static_cast<const SfxInt16Item&>(rSet->Get(
                                        FN_DRAW_WRAP_DLG)).GetValue() );
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
    ActivatePage( *rSet );
}

// stuff attributes into the set when OK
bool SwWrapTabPage::FillItemSet(SfxItemSet *rSet)
{
    bool bModified = false;
    const SfxPoolItem* pOldItem;
    const SwFormatSurround& rOldSur = GetItemSet().Get(RES_SURROUND);
    SwFormatSurround aSur( rOldSur );

    SvxOpaqueItem aOp( RES_OPAQUE);

    if (!m_bDrawMode)
    {
        const SvxOpaqueItem& rOpaque = GetItemSet().Get(RES_OPAQUE);
        aOp = rOpaque;
        aOp.SetValue(true);
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
            aOp.SetValue(false);
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
                    aOp != *pOldItem )
        {
            rSet->Put(aOp);
            bModified = true;
        }
    }

    bool bTopMod = m_xTopMarginED->get_value_changed_from_saved();
    bool bBottomMod = m_xBottomMarginED->get_value_changed_from_saved();

    SvxULSpaceItem aUL( RES_UL_SPACE );
    aUL.SetUpper(static_cast<sal_uInt16>(m_xTopMarginED->denormalize(m_xTopMarginED->get_value(FieldUnit::TWIP))));
    aUL.SetLower(static_cast<sal_uInt16>(m_xBottomMarginED->denormalize(m_xBottomMarginED->get_value(FieldUnit::TWIP))));

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
    aLR.SetLeft(static_cast<sal_uInt16>(m_xLeftMarginED->denormalize(m_xLeftMarginED->get_value(FieldUnit::TWIP))));
    aLR.SetRight(static_cast<sal_uInt16>(m_xRightMarginED->denormalize(m_xRightMarginED->get_value(FieldUnit::TWIP))));

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

    return bModified;
}

// example update
void SwWrapTabPage::ActivatePage(const SfxItemSet& rSet)
{
    // anchor
    const SwFormatAnchor &rAnch = rSet.Get(RES_ANCHOR);
    m_nAnchorId = rAnch.GetAnchorId();
    bool bEnable = (m_nAnchorId != RndStdIds::FLY_AS_CHAR);

    if (!m_bDrawMode)
    {
        SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell() : m_pWrtSh;
        SwFlyFrameAttrMgr aMgr( m_bNew, pSh, GetItemSet() );
        SvxSwFrameValidation aVal;

        // size
        const SwFormatFrameSize& rFrameSize = rSet.Get(RES_FRM_SIZE);
        Size aSize = rFrameSize.GetSize();

        // position
        const SwFormatHoriOrient& rHori = rSet.Get(RES_HORI_ORIENT);
        const SwFormatVertOrient& rVert = rSet.Get(RES_VERT_ORIENT);

        aVal.nAnchorType = m_nAnchorId;
        aVal.bAutoHeight = rFrameSize.GetHeightSizeType() == ATT_MIN_SIZE;
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

IMPL_LINK_NOARG(SwWrapTabPage, WrapTypeHdl, weld::ToggleButton&, void)
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

IMPL_LINK_NOARG(SwWrapTabPage, ContourHdl, weld::ToggleButton&, void)
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
    m_xWrapThroughRB->set_from_icon_name(RID_BMP_WRAP_THROUGH);
    bool bWrapOutline =  !m_xWrapOutlineCB->get_active();
    if (bWrapOutline)
    {
        m_xNoWrapRB->set_from_icon_name(RID_BMP_WRAP_NONE);
        m_xWrapLeftRB->set_from_icon_name(RID_BMP_WRAP_LEFT);
        m_xWrapRightRB->set_from_icon_name(RID_BMP_WRAP_RIGHT);
        m_xWrapParallelRB->set_from_icon_name(RID_BMP_WRAP_PARALLEL);
        m_xIdealWrapRB->set_from_icon_name(RID_BMP_WRAP_IDEAL);
    }
    else
    {
        m_xNoWrapRB->set_from_icon_name(RID_BMP_WRAP_CONTOUR_NONE);
        m_xWrapLeftRB->set_from_icon_name(RID_BMP_WRAP_CONTOUR_LEFT);
        m_xWrapRightRB->set_from_icon_name(RID_BMP_WRAP_CONTOUR_RIGHT);
        m_xWrapParallelRB->set_from_icon_name(RID_BMP_WRAP_CONTOUR_PARALLEL);
        m_xIdealWrapRB->set_from_icon_name(RID_BMP_WRAP_CONTOUR_IDEAL);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
