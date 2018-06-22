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

#include <sfx2/app.hxx>

#include <cmdid.h>
#include <hintids.hxx>
#include <swtypes.hxx>
#include <globals.hrc>
#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <uitool.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/ruler.hxx>
#include <pggrid.hxx>
#include <tgrditem.hxx>

#include <wrtsh.hxx>
#include <doc.hxx>
#include <uiitems.hxx>
#include <swmodule.hxx>
#include <view.hxx>

SwTextGridPage::SwTextGridPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/textgridpage.ui", "TextGridPage", &rSet)
    , m_nRubyUserValue(0)
    , m_bRubyUserValue(false)
    , m_aPageSize(MM50, MM50)
    , m_bVertical(false)
    , m_bSquaredMode(false)
    , m_bHRulerChanged(false)
    , m_bVRulerChanged(false)
    , m_xNoGridRB(m_xBuilder->weld_radio_button("radioRB_NOGRID"))
    , m_xLinesGridRB(m_xBuilder->weld_radio_button("radioRB_LINESGRID"))
    , m_xCharsGridRB(m_xBuilder->weld_radio_button("radioRB_CHARSGRID"))
    , m_xSnapToCharsCB(m_xBuilder->weld_check_button("checkCB_SNAPTOCHARS"))
    , m_xExampleWN(new weld::CustomWeld(*m_xBuilder, "drawingareaWN_EXAMPLE", m_aExampleWN))
    , m_xLayoutFL(m_xBuilder->weld_widget("frameFL_LAYOUT"))
    , m_xLinesPerPageNF(m_xBuilder->weld_spin_button("spinNF_LINESPERPAGE"))
    , m_xLinesRangeFT(m_xBuilder->weld_label("labelFT_LINERANGE"))
    , m_xTextSizeMF(m_xBuilder->weld_metric_spin_button("spinMF_TEXTSIZE", FUNIT_POINT))
    , m_xCharsPerLineFT(m_xBuilder->weld_label("labelFT_CHARSPERLINE"))
    , m_xCharsPerLineNF(m_xBuilder->weld_spin_button("spinNF_CHARSPERLINE"))
    , m_xCharsRangeFT(m_xBuilder->weld_label("labelFT_CHARRANGE"))
    , m_xCharWidthFT(m_xBuilder->weld_label("labelFT_CHARWIDTH"))
    , m_xCharWidthMF(m_xBuilder->weld_metric_spin_button("spinMF_CHARWIDTH", FUNIT_POINT))
    , m_xRubySizeFT(m_xBuilder->weld_label("labelFT_RUBYSIZE"))
    , m_xRubySizeMF(m_xBuilder->weld_metric_spin_button("spinMF_RUBYSIZE", FUNIT_POINT))
    , m_xRubyBelowCB(m_xBuilder->weld_check_button("checkCB_RUBYBELOW"))
    , m_xDisplayFL(m_xBuilder->weld_widget("frameFL_DISPLAY"))
    , m_xDisplayCB(m_xBuilder->weld_check_button("checkCB_DISPLAY"))
    , m_xPrintCB(m_xBuilder->weld_check_button("checkCB_PRINT"))
    , m_xColorLB(new ColorListBox(m_xBuilder->weld_menu_button("listLB_COLOR"), GetFrameWeld(), true))
{
    Link<weld::SpinButton&,void> aLink = LINK(this, SwTextGridPage, CharorLineChangedHdl);
    m_xCharsPerLineNF->connect_value_changed(aLink);
    m_xLinesPerPageNF->connect_value_changed(aLink);

    Link<weld::MetricSpinButton&,void> aSizeLink = LINK(this, SwTextGridPage, TextSizeChangedHdl);
    m_xTextSizeMF->connect_value_changed(aSizeLink);
    m_xRubySizeMF->connect_value_changed(aSizeLink);
    m_xCharWidthMF->connect_value_changed(aSizeLink);

    Link<weld::ToggleButton&,void> aGridTypeHdl = LINK(this, SwTextGridPage, GridTypeHdl);
    m_xNoGridRB->connect_toggled(aGridTypeHdl);
    m_xLinesGridRB->connect_toggled(aGridTypeHdl);
    m_xCharsGridRB->connect_toggled(aGridTypeHdl);

    m_xColorLB->SetSelectHdl(LINK(this, SwTextGridPage, ColorModifyHdl));
    m_xPrintCB->connect_toggled(LINK(this, SwTextGridPage, GridModifyClickHdl));
    m_xRubyBelowCB->connect_toggled(LINK(this, SwTextGridPage, GridModifyClickHdl));

    m_xDisplayCB->connect_toggled(LINK(this, SwTextGridPage, DisplayGridHdl));

    //Get the default paper mode
    SwView *pView   = ::GetActiveView();
    if( pView )
    {
        SwWrtShell* pSh = pView->GetWrtShellPtr();
        if( pSh )
        {
            m_bSquaredMode = pSh->GetDoc()->IsSquaredPageMode();
        }
    }
    if( m_bSquaredMode )
    {

        m_xRubySizeFT->show();
        m_xRubySizeMF->show();
        m_xRubyBelowCB->show();
        m_xSnapToCharsCB->hide();
        m_xCharWidthFT->hide();
        m_xCharWidthMF->hide();
    }
    else
    {
        m_xRubySizeFT->hide();
        m_xRubySizeMF->hide();
        m_xRubyBelowCB->hide();
        m_xSnapToCharsCB->show();
        m_xCharWidthFT->show();
        m_xCharWidthMF->show();
    }
}

SwTextGridPage::~SwTextGridPage()
{
    disposeOnce();
}

void SwTextGridPage::dispose()
{
    m_xColorLB.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwTextGridPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwTextGridPage>::Create(pParent, *rSet);
}

bool SwTextGridPage::FillItemSet(SfxItemSet *rSet)
{
    bool bRet = false;
    if (m_xNoGridRB->get_state_changed_from_saved() ||
        m_xLinesGridRB->get_state_changed_from_saved() ||
        m_xLinesPerPageNF->get_value_changed_from_saved() ||
        m_xTextSizeMF->get_value_changed_from_saved() ||
        m_xCharsPerLineNF->get_value_changed_from_saved() ||
        m_xSnapToCharsCB->get_state_changed_from_saved() ||
        m_xRubySizeMF->get_value_changed_from_saved() ||
        m_xCharWidthMF->get_value_changed_from_saved() ||
        m_xRubyBelowCB->get_state_changed_from_saved() ||
        m_xDisplayCB->get_state_changed_from_saved() ||
        m_xPrintCB->get_state_changed_from_saved() ||
        m_xColorLB->IsValueChangedFromSaved())
    {
        PutGridItem(*rSet);
        bRet = true;
    }

    // draw ticks of ruler
    SwView * pView = ::GetActiveView();
    if ( m_bHRulerChanged )
        pView->GetHRuler().DrawTicks();
    if ( m_bVRulerChanged )
        pView->GetVRuler().DrawTicks();
    return bRet;
}

void    SwTextGridPage::Reset(const SfxItemSet *rSet)
{
    if(SfxItemState::DEFAULT <= rSet->GetItemState(RES_TEXTGRID))
    {
        const SwTextGridItem& rGridItem = rSet->Get(RES_TEXTGRID);
        weld::RadioButton* pButton = nullptr;
        switch(rGridItem.GetGridType())
        {
            case GRID_NONE :        pButton = m_xNoGridRB.get();    break;
            case GRID_LINES_ONLY  : pButton = m_xLinesGridRB.get(); break;
            default:                pButton = m_xCharsGridRB.get();
        }
        pButton->set_active(true);
        m_xDisplayCB->set_active(rGridItem.IsDisplayGrid());
        GridTypeHdl(*pButton);
        m_xSnapToCharsCB->set_active(rGridItem.IsSnapToChars());
        m_xLinesPerPageNF->set_value(rGridItem.GetLines());
        SetLinesOrCharsRanges(*m_xLinesRangeFT , m_xLinesPerPageNF->get_max());
        m_nRubyUserValue = rGridItem.GetBaseHeight();
        m_bRubyUserValue = true;
        m_xTextSizeMF->set_value(m_xTextSizeMF->normalize(m_nRubyUserValue), FUNIT_TWIP);
        m_xRubySizeMF->set_value(m_xRubySizeMF->normalize(rGridItem.GetRubyHeight()), FUNIT_TWIP);
        m_xCharWidthMF->set_value(m_xCharWidthMF->normalize(rGridItem.GetBaseWidth()), FUNIT_TWIP);
        m_xRubyBelowCB->set_active(rGridItem.IsRubyTextBelow());
        m_xPrintCB->set_active(rGridItem.IsPrintGrid());
        m_xColorLB->SelectEntry(rGridItem.GetColor());
    }
    UpdatePageSize(*rSet);

    m_xNoGridRB->save_state();
    m_xLinesGridRB->save_state();
    m_xSnapToCharsCB->save_state();
    m_xLinesPerPageNF->save_value();
    m_xTextSizeMF->save_value();
    m_xCharsPerLineNF->save_value();
    m_xRubySizeMF->save_value();
    m_xCharWidthMF->save_value();
    m_xRubyBelowCB->save_state();
    m_xDisplayCB->save_state();
    m_xPrintCB->save_state();
    m_xColorLB->SaveValue();
}

void    SwTextGridPage::ActivatePage( const SfxItemSet& rSet )
{
    m_aExampleWN.Hide();
    m_aExampleWN.UpdateExample(rSet);
    UpdatePageSize(rSet);
    m_aExampleWN.Show();
    m_aExampleWN.Invalidate();
}

DeactivateRC SwTextGridPage::DeactivatePage( SfxItemSet* )
{
    return DeactivateRC::LeavePage;
}

void SwTextGridPage::PutGridItem(SfxItemSet& rSet)
{
        SwTextGridItem aGridItem;
        aGridItem.SetGridType(m_xNoGridRB->get_active() ? GRID_NONE :
            m_xLinesGridRB->get_active() ? GRID_LINES_ONLY : GRID_LINES_CHARS );
        aGridItem.SetSnapToChars(m_xSnapToCharsCB->get_active());
        aGridItem.SetLines( static_cast< sal_uInt16 >(m_xLinesPerPageNF->get_value()) );
        aGridItem.SetBaseHeight( static_cast< sal_uInt16 >(
            m_bRubyUserValue ? m_nRubyUserValue :
                m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP))) );
        aGridItem.SetRubyHeight( static_cast< sal_uInt16 >(m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FUNIT_TWIP))) );
        aGridItem.SetBaseWidth( static_cast< sal_uInt16 >(m_xCharWidthMF->denormalize(m_xCharWidthMF->get_value(FUNIT_TWIP))) );
        aGridItem.SetRubyTextBelow(m_xRubyBelowCB->get_active());
        aGridItem.SetSquaredMode(m_bSquaredMode);
        aGridItem.SetDisplayGrid(m_xDisplayCB->get_active());
        aGridItem.SetPrintGrid(m_xPrintCB->get_active());
        aGridItem.SetColor(m_xColorLB->GetSelectEntryColor());
        rSet.Put(aGridItem);

        SwView * pView = ::GetActiveView();
        if ( aGridItem.GetGridType() != GRID_NONE )
        {
            if ( aGridItem.GetGridType() == GRID_LINES_CHARS )
            {
                m_bHRulerChanged = true;
            }
            m_bVRulerChanged = true;
            pView->GetHRuler().SetCharWidth(static_cast<long>(m_xCharWidthMF->get_value(FUNIT_TWIP)/56.7));
            pView->GetVRuler().SetLineHeight(static_cast<long>(m_xTextSizeMF->get_value(FUNIT_TWIP)/56.7));
        }
}

void SwTextGridPage::UpdatePageSize(const SfxItemSet& rSet)
{
    if( SfxItemState::UNKNOWN !=  rSet.GetItemState( RES_FRAMEDIR ))
    {
        const SvxFrameDirectionItem& rDirItem =
                    rSet.Get(RES_FRAMEDIR);
        m_bVertical = rDirItem.GetValue() == SvxFrameDirection::Vertical_RL_TB||
                    rDirItem.GetValue() == SvxFrameDirection::Vertical_LR_TB;
    }

    if( SfxItemState::SET != rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
        return;

    const SvxSizeItem& rSize = rSet.Get(SID_ATTR_PAGE_SIZE);
    const SvxLRSpaceItem& rLRSpace = rSet.Get( RES_LR_SPACE );
    const SvxULSpaceItem& rULSpace = rSet.Get( RES_UL_SPACE );
    const SvxBoxItem& rBox = rSet.Get(RES_BOX);
    sal_Int32 nDistanceLR = rLRSpace.GetLeft() + rLRSpace.GetRight();
    sal_Int32 nDistanceUL = rULSpace.GetUpper() + rULSpace.GetLower();

    sal_Int32 nValue1 = rSize.GetSize().Height() - nDistanceUL -
            rBox.GetDistance(SvxBoxItemLine::TOP) -
                                rBox.GetDistance(SvxBoxItemLine::BOTTOM);
    sal_Int32 nValue2 = rSize.GetSize().Width() - nDistanceLR -
            rBox.GetDistance(SvxBoxItemLine::LEFT) -
                                rBox.GetDistance(SvxBoxItemLine::RIGHT);
    if(m_bVertical)
    {
        m_aPageSize.setWidth( nValue1 );
        m_aPageSize.setHeight( nValue2 );
    }
    else
    {
        m_aPageSize.setWidth( nValue2 );
        m_aPageSize.setHeight( nValue1 );
    }

    sal_Int32 nTextSize = static_cast< sal_Int32 >(m_bRubyUserValue ?
                m_nRubyUserValue :
                    m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP)));

    if ( m_bSquaredMode )
    {
        m_xCharsPerLineNF->set_value(m_aPageSize.Width() / nTextSize);
        m_xCharsPerLineNF->set_max(m_xCharsPerLineNF->get_value());
        m_xLinesPerPageNF->set_max(m_aPageSize.Height() /
        (   m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP)) +
                    m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FUNIT_TWIP))));
        SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
        SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
    }
    else
    {
        sal_Int32 nTextWidth = static_cast< sal_Int32 >(m_xCharWidthMF->denormalize(m_xCharWidthMF->get_value(FUNIT_TWIP)));
        m_xLinesPerPageNF->set_value(m_aPageSize.Height() / nTextSize);
        if (nTextWidth)
            m_xCharsPerLineNF->set_value(m_aPageSize.Width() / nTextWidth);
        else
            m_xCharsPerLineNF->set_value(45);
        SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
        SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
    }
}

void SwTextGridPage::SetLinesOrCharsRanges(weld::Label& rField, const sal_Int32 nValue )
{
    OUString aFieldStr("( 1 -");
    aFieldStr += OUString::number(nValue);
    aFieldStr += " )";
    rField.set_label(aFieldStr);
}

const sal_uInt16* SwTextGridPage::GetRanges()
{
    static const sal_uInt16 aPageRg[] = {
        RES_TEXTGRID, RES_TEXTGRID,
        0};
    return aPageRg;
}

IMPL_LINK(SwTextGridPage, CharorLineChangedHdl, weld::SpinButton&, rField, void)
{
    //if in squared mode
    if ( m_bSquaredMode )
    {
        if (m_xCharsPerLineNF.get() == &rField)
        {
            auto nValue = m_xCharsPerLineNF->get_value();
            assert(nValue && "div-by-zero");
            auto nWidth = m_aPageSize.Width() / nValue;
            m_xTextSizeMF->set_value(m_xTextSizeMF->normalize(nWidth), FUNIT_TWIP);
            //prevent rounding errors in the MetricField by saving the used value
            m_nRubyUserValue = nWidth;
            m_bRubyUserValue = true;

        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP)) +
                    m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FUNIT_TWIP))));
            m_xLinesPerPageNF->set_max(nMaxLines);
        }
        SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
        SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
    }
    else//in normal mode
    {
        if (m_xLinesPerPageNF.get() == &rField)
        {
            auto nValue = m_xLinesPerPageNF->get_value();
            assert(nValue && "div-by-zero");
            auto nHeight = m_aPageSize.Height() / nValue;
            m_xTextSizeMF->set_value(m_xTextSizeMF->normalize(nHeight), FUNIT_TWIP);
            m_xRubySizeMF->set_value(0, FUNIT_TWIP);
            SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );

            m_nRubyUserValue = nHeight;
            m_bRubyUserValue = true;
        }
        else if (m_xCharsPerLineNF.get() == &rField)
        {
            auto nValue = m_xCharsPerLineNF->get_value();
            assert(nValue && "div-by-zero");
            auto nWidth = m_aPageSize.Width() / nValue;
            m_xCharWidthMF->set_value(m_xCharWidthMF->normalize(nWidth), FUNIT_TWIP);
            SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
        }
    }
    GridModifyHdl();
}

IMPL_LINK(SwTextGridPage, TextSizeChangedHdl, weld::MetricSpinButton&, rField, void)
{
    //if in squared mode
    if( m_bSquaredMode )
    {
        if (m_xTextSizeMF.get() == &rField)
        {
            m_bRubyUserValue = false;

            // fdo#50941: set maximum characters per line
            sal_Int32 nTextSize = static_cast< sal_Int32 >(m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP)));
            if (nTextSize > 0)
            {
                sal_Int32 nMaxChars = m_aPageSize.Width() / nTextSize;
                m_xCharsPerLineNF->set_value(nMaxChars);
                m_xCharsPerLineNF->set_max(nMaxChars);
                SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
            }
        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP)) +
                    m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FUNIT_TWIP))));
            m_xLinesPerPageNF->set_max(nMaxLines);
            SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
        }
    }
    else
    {
        if (m_xTextSizeMF.get() == &rField)
        {
            sal_Int32 nTextSize = static_cast< sal_Int32 >(m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FUNIT_TWIP)));
            m_xLinesPerPageNF->set_value(m_aPageSize.Height() / nTextSize);
            m_bRubyUserValue = false;
            SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
        }
        else if (m_xCharWidthMF.get() == &rField)
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(m_xCharWidthMF->denormalize(m_xCharWidthMF->get_value(FUNIT_TWIP)));
            sal_Int32 nMaxChar = 45 ;
            if (nTextWidth)
                nMaxChar = m_aPageSize.Width() / nTextWidth;
            m_xCharsPerLineNF->set_value( nMaxChar );
            SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
        }
        //rubySize is disabled
    }
    GridModifyHdl();
}

IMPL_LINK(SwTextGridPage, GridTypeHdl, weld::ToggleButton&, rButton, void)
{
    bool bEnable = m_xNoGridRB.get() != &rButton;
    m_xLayoutFL->set_sensitive(bEnable);
    m_xDisplayFL->set_sensitive(bEnable);

    //one special case
    if (bEnable)
        DisplayGridHdl(*m_xDisplayCB);

    bEnable = m_xCharsGridRB.get() == &rButton;
    m_xSnapToCharsCB->set_sensitive(bEnable);

    bEnable = m_xLinesGridRB.get() == &rButton;
    if (bEnable && !m_bSquaredMode)
    {
        m_xCharsPerLineFT->set_sensitive(false);
        m_xCharsPerLineNF->set_sensitive(false);
        m_xCharsRangeFT->set_sensitive(false);
        m_xCharWidthFT->set_sensitive(false);
        m_xCharWidthMF->set_sensitive(false);
    }

    GridModifyHdl();
}

IMPL_LINK_NOARG(SwTextGridPage, DisplayGridHdl, weld::ToggleButton&, void)
{
    bool bChecked = m_xDisplayCB->get_active();
    m_xPrintCB->set_sensitive(bChecked);
    m_xPrintCB->set_active(bChecked);
}

IMPL_LINK_NOARG(SwTextGridPage, GridModifyClickHdl, weld::ToggleButton&, void)
{
    GridModifyHdl();
}

IMPL_LINK_NOARG(SwTextGridPage, ColorModifyHdl, ColorListBox&, void)
{
    GridModifyHdl();
}

void SwTextGridPage::GridModifyHdl()
{
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemSet aSet(rOldSet);
    const SfxItemSet* pExSet = GetTabDialog()->GetExampleSet();
    if(pExSet)
        aSet.Put(*pExSet);
    PutGridItem(aSet);
    m_aExampleWN.UpdateExample(aSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
