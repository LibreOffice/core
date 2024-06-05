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

#include <cmdid.h>
#include <hintids.hxx>
#include <svx/colorbox.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/ruler.hxx>
#include <pggrid.hxx>
#include <tgrditem.hxx>
#include <svx/pageitem.hxx>

#include <wrtsh.hxx>
#include <doc.hxx>
#include <swmodule.hxx>
#include <view.hxx>

constexpr tools::Long constTwips_5mm = o3tl::toTwips(5, o3tl::Length::mm);

SwTextGridPage::SwTextGridPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/textgridpage.ui"_ustr, u"TextGridPage"_ustr, &rSet)
    , m_nRubyUserValue(0)
    , m_bRubyUserValue(false)
    , m_aPageSize(constTwips_5mm, constTwips_5mm)
    , m_bVertical(false)
    , m_bSquaredMode(false)
    , m_bHRulerChanged(false)
    , m_bVRulerChanged(false)
    , m_xNoGridRB(m_xBuilder->weld_radio_button(u"radioRB_NOGRID"_ustr))
    , m_xLinesGridRB(m_xBuilder->weld_radio_button(u"radioRB_LINESGRID"_ustr))
    , m_xCharsGridRB(m_xBuilder->weld_radio_button(u"radioRB_CHARSGRID"_ustr))
    , m_xSnapToCharsCB(m_xBuilder->weld_check_button(u"checkCB_SNAPTOCHARS"_ustr))
    , m_xExampleWN(new weld::CustomWeld(*m_xBuilder, u"drawingareaWN_EXAMPLE"_ustr, m_aExampleWN))
    , m_xLayoutFL(m_xBuilder->weld_widget(u"frameFL_LAYOUT"_ustr))
    , m_xLinesPerPageNF(m_xBuilder->weld_spin_button(u"spinNF_LINESPERPAGE"_ustr))
    , m_xLinesRangeFT(m_xBuilder->weld_label(u"labelFT_LINERANGE"_ustr))
    , m_xTextSizeMF(m_xBuilder->weld_metric_spin_button(u"spinMF_TEXTSIZE"_ustr, FieldUnit::POINT))
    , m_xCharsPerLineFT(m_xBuilder->weld_label(u"labelFT_CHARSPERLINE"_ustr))
    , m_xCharsPerLineNF(m_xBuilder->weld_spin_button(u"spinNF_CHARSPERLINE"_ustr))
    , m_xCharsRangeFT(m_xBuilder->weld_label(u"labelFT_CHARRANGE"_ustr))
    , m_xCharWidthFT(m_xBuilder->weld_label(u"labelFT_CHARWIDTH"_ustr))
    , m_xCharWidthMF(m_xBuilder->weld_metric_spin_button(u"spinMF_CHARWIDTH"_ustr, FieldUnit::POINT))
    , m_xRubySizeFT(m_xBuilder->weld_label(u"labelFT_RUBYSIZE"_ustr))
    , m_xRubySizeMF(m_xBuilder->weld_metric_spin_button(u"spinMF_RUBYSIZE"_ustr, FieldUnit::POINT))
    , m_xRubyBelowCB(m_xBuilder->weld_check_button(u"checkCB_RUBYBELOW"_ustr))
    , m_xDisplayFL(m_xBuilder->weld_widget(u"frameFL_DISPLAY"_ustr))
    , m_xDisplayCB(m_xBuilder->weld_check_button(u"checkCB_DISPLAY"_ustr))
    , m_xPrintCB(m_xBuilder->weld_check_button(u"checkCB_PRINT"_ustr))
    , m_xColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"listLB_COLOR"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
{
    Link<weld::SpinButton&,void> aLink = LINK(this, SwTextGridPage, CharorLineChangedHdl);
    m_xCharsPerLineNF->connect_value_changed(aLink);
    m_xLinesPerPageNF->connect_value_changed(aLink);

    Link<weld::MetricSpinButton&,void> aSizeLink = LINK(this, SwTextGridPage, TextSizeChangedHdl);
    m_xTextSizeMF->connect_value_changed(aSizeLink);
    m_xRubySizeMF->connect_value_changed(aSizeLink);
    m_xCharWidthMF->connect_value_changed(aSizeLink);

    Link<weld::Toggleable&,void> aGridTypeHdl = LINK(this, SwTextGridPage, GridTypeHdl);
    m_xNoGridRB->connect_toggled(aGridTypeHdl);
    m_xLinesGridRB->connect_toggled(aGridTypeHdl);
    m_xCharsGridRB->connect_toggled(aGridTypeHdl);

    m_xColorLB->SetSelectHdl(LINK(this, SwTextGridPage, ColorModifyHdl));
    m_xPrintCB->connect_toggled(LINK(this, SwTextGridPage, GridModifyClickHdl));
    m_xRubyBelowCB->connect_toggled(LINK(this, SwTextGridPage, GridModifyClickHdl));

    m_xDisplayCB->connect_toggled(LINK(this, SwTextGridPage, DisplayGridHdl));

    //Get the default paper mode
    if (SwView *pView = GetActiveView())
    {
        if (SwWrtShell* pSh = pView->GetWrtShellPtr())
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
    m_xColorLB.reset();
}

std::unique_ptr<SfxTabPage> SwTextGridPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet)
{
    return std::make_unique<SwTextGridPage>(pPage, pController, *rSet);
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
    if (SwView * pView = GetActiveView())
    {
        if ( m_bHRulerChanged )
            pView->GetHRuler().DrawTicks();
        if ( m_bVRulerChanged )
            pView->GetVRuler().DrawTicks();
    }
    return bRet;
}

void    SwTextGridPage::Reset(const SfxItemSet *rSet)
{
    sal_Int32 nLinesPerPage = 0;

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
        nLinesPerPage = rGridItem.GetLines();

        SetLinesOrCharsRanges(*m_xLinesRangeFT , m_xLinesPerPageNF->get_max());
        m_nRubyUserValue = rGridItem.GetBaseHeight();
        m_bRubyUserValue = true;
        m_xTextSizeMF->set_value(m_xTextSizeMF->normalize(m_nRubyUserValue), FieldUnit::TWIP);
        m_xRubySizeMF->set_value(m_xRubySizeMF->normalize(rGridItem.GetRubyHeight()), FieldUnit::TWIP);
        m_xCharWidthMF->set_value(m_xCharWidthMF->normalize(rGridItem.GetBaseWidth()), FieldUnit::TWIP);
        m_xRubyBelowCB->set_active(rGridItem.IsRubyTextBelow());
        m_xPrintCB->set_active(rGridItem.IsPrintGrid());
        m_xColorLB->SelectEntry(rGridItem.GetColor());
    }
    UpdatePageSize(*rSet);

    if (nLinesPerPage > 0)
        m_xLinesPerPageNF->set_value(nLinesPerPage);

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
    aGridItem.SetLines( static_cast< sal_Int32 >(m_xLinesPerPageNF->get_value()) );
    aGridItem.SetBaseHeight( static_cast< sal_uInt16 >(
        m_bRubyUserValue ? m_nRubyUserValue :
            m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP))) );
    // Tdf#151544: set ruby height from the value get from UI only when in square page mode.
    // When in normal mode, the ruby height should be zero.
    if (m_bSquaredMode)
        aGridItem.SetRubyHeight(static_cast<sal_uInt16>(m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FieldUnit::TWIP))));
    else
        aGridItem.SetRubyHeight(0);
    aGridItem.SetBaseWidth( static_cast< sal_uInt16 >(m_xCharWidthMF->denormalize(m_xCharWidthMF->get_value(FieldUnit::TWIP))) );
    aGridItem.SetRubyTextBelow(m_xRubyBelowCB->get_active());
    aGridItem.SetSquaredMode(m_bSquaredMode);
    aGridItem.SetDisplayGrid(m_xDisplayCB->get_active());
    aGridItem.SetPrintGrid(m_xPrintCB->get_active());
    aGridItem.SetColor(m_xColorLB->GetSelectEntryColor());
    rSet.Put(aGridItem);

    SwView * pView = ::GetActiveView();
    if (pView && aGridItem.GetGridType() != GRID_NONE)
    {
        if ( aGridItem.GetGridType() == GRID_LINES_CHARS )
        {
            m_bHRulerChanged = true;
        }
        m_bVRulerChanged = true;
        pView->GetHRuler().SetCharWidth(m_xCharWidthMF->get_value(FieldUnit::MM));
        pView->GetVRuler().SetLineHeight(m_xTextSizeMF->get_value(FieldUnit::MM));
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

    for( const TypedWhichId<SvxSetItem> & nId : { SID_ATTR_PAGE_HEADERSET, SID_ATTR_PAGE_FOOTERSET })
    {
        if( const SvxSetItem* pItem = rSet.GetItemIfSet( nId, false ) )
        {
            const SfxItemSet& rExtraSet = pItem->GetItemSet();
            const SfxBoolItem& rOn =
                rExtraSet.Get( rSet.GetPool()->GetWhichIDFromSlotID( SID_ATTR_PAGE_ON ) );

            if ( rOn.GetValue() )
            {
                const SvxSizeItem& rSizeItem =
                    rExtraSet.Get(rSet.GetPool()->GetWhichIDFromSlotID(SID_ATTR_PAGE_SIZE));
                nDistanceUL += rSizeItem.GetSize().Height();
            }
        }
    }

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
                    m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP)));

    if ( m_bSquaredMode )
    {
        sal_Int32 nCharsPerLine = m_aPageSize.Width() / nTextSize;
        m_xCharsPerLineNF->set_max(nCharsPerLine);
        m_xCharsPerLineNF->set_sensitive(nCharsPerLine != 0);
        m_xCharsPerLineNF->set_value(nCharsPerLine);
        sal_Int32 nMaxLines = m_aPageSize.Height() /
        (   m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP)) +
                    m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FieldUnit::TWIP)));
        m_xLinesPerPageNF->set_max(nMaxLines);
        m_xLinesPerPageNF->set_sensitive(nMaxLines != 0);
        SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
        SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
    }
    else
    {
        sal_Int32 nTextWidth = static_cast< sal_Int32 >(m_xCharWidthMF->denormalize(m_xCharWidthMF->get_value(FieldUnit::TWIP)));
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
    OUString aFieldStr = "( 1 -" + OUString::number(nValue) + " )";
    rField.set_label(aFieldStr);
}

WhichRangesContainer SwTextGridPage::GetRanges()
{
    return WhichRangesContainer(svl::Items<RES_TEXTGRID, RES_TEXTGRID>);
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
            m_xTextSizeMF->set_value(m_xTextSizeMF->normalize(nWidth), FieldUnit::TWIP);
            //prevent rounding errors in the MetricField by saving the used value
            m_nRubyUserValue = nWidth;
            m_bRubyUserValue = true;

        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP)) +
                    m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FieldUnit::TWIP))));
            m_xLinesPerPageNF->set_max(nMaxLines);
            m_xLinesPerPageNF->set_sensitive(nMaxLines != 0);
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
            m_xTextSizeMF->set_value(m_xTextSizeMF->normalize(nHeight), FieldUnit::TWIP);
            SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );

            m_nRubyUserValue = nHeight;
            m_bRubyUserValue = true;
        }
        else if (m_xCharsPerLineNF.get() == &rField)
        {
            auto nValue = m_xCharsPerLineNF->get_value();
            assert(nValue && "div-by-zero");
            auto nWidth = m_aPageSize.Width() / nValue;
            m_xCharWidthMF->set_value(m_xCharWidthMF->normalize(nWidth), FieldUnit::TWIP);
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
            sal_Int32 nTextSize = static_cast< sal_Int32 >(m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP)));
            if (nTextSize > 0)
            {
                sal_Int32 nMaxChars = m_aPageSize.Width() / nTextSize;
                m_xCharsPerLineNF->set_value(nMaxChars);
                m_xCharsPerLineNF->set_max(nMaxChars);
                m_xCharsPerLineNF->set_sensitive(nMaxChars != 0);
                SetLinesOrCharsRanges( *m_xCharsRangeFT , m_xCharsPerLineNF->get_max() );
            }
        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP)) +
                    m_xRubySizeMF->denormalize(m_xRubySizeMF->get_value(FieldUnit::TWIP))));
            m_xLinesPerPageNF->set_max(nMaxLines);
            m_xLinesPerPageNF->set_sensitive(nMaxLines != 0);
            SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
        }
    }
    else
    {
        if (m_xTextSizeMF.get() == &rField)
        {
            sal_Int32 nTextSize = static_cast< sal_Int32 >(m_xTextSizeMF->denormalize(m_xTextSizeMF->get_value(FieldUnit::TWIP)));
            m_xLinesPerPageNF->set_value(m_aPageSize.Height() / nTextSize);
            m_bRubyUserValue = false;
            SetLinesOrCharsRanges( *m_xLinesRangeFT , m_xLinesPerPageNF->get_max() );
        }
        else if (m_xCharWidthMF.get() == &rField)
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(m_xCharWidthMF->denormalize(m_xCharWidthMF->get_value(FieldUnit::TWIP)));
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

IMPL_LINK(SwTextGridPage, GridTypeHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    if (m_xNoGridRB.get() == &rButton)
    {
        // GRID_NONE mode:
        //   "Layout" and "Display" sections should all be disabled.
        m_xLayoutFL->set_sensitive(false);
        m_xDisplayFL->set_sensitive(false);
    }
    else
    {
        // GRID_LINES_ONLY or GRID_LINES_CHARS mode:
        //   "Layout" and "Display" sections should all be enabled;
        //   DisplayGridHdl should be executed;
        m_xLayoutFL->set_sensitive(true);
        m_xDisplayFL->set_sensitive(true);
        DisplayGridHdl(*m_xDisplayCB);
    }

    if (m_xCharsGridRB.get() == &rButton)
    {
        // GRID_LINES_CHARS mode:
        //   "Snap to character" should be enabled;
        //   "Characters per line" should be enabled;
        //   "Characters range" should be enabled;
        m_xSnapToCharsCB->set_sensitive(true);
        m_xCharsPerLineFT->set_sensitive(true);
        m_xCharsPerLineNF->set_sensitive(true);
        m_xCharsRangeFT->set_sensitive(true);
        m_xCharWidthFT->set_sensitive(true);
        m_xCharWidthMF->set_sensitive(true);
    }
    else
    {
        // GRID_NONE or GRID_LINES_ONLY mode:
        //   "Snap to character" should be disabled;
        //   "Characters per line" should be disabled;
        //   "Characters range" should be disabled;
        m_xSnapToCharsCB->set_sensitive(false);
        m_xCharsPerLineFT->set_sensitive(false);
        m_xCharsPerLineNF->set_sensitive(false);
        m_xCharsRangeFT->set_sensitive(false);
        m_xCharWidthFT->set_sensitive(false);
        m_xCharWidthMF->set_sensitive(false);
    }

    if (m_xNoGridRB.get() != &rButton)
    {
        // GRID_LINES_ONLY or GRID_LINES_CHARS mode: (additionally)
        //   TextSizeChangedHdl should be executed to recalculate which dependencies are sensitive.
        TextSizeChangedHdl(*m_xTextSizeMF);
    }

    GridModifyHdl();
}

IMPL_LINK_NOARG(SwTextGridPage, DisplayGridHdl, weld::Toggleable&, void)
{
    bool bChecked = m_xDisplayCB->get_active();
    m_xPrintCB->set_sensitive(bChecked);
    m_xPrintCB->set_active(bChecked);
}

IMPL_LINK_NOARG(SwTextGridPage, GridModifyClickHdl, weld::Toggleable&, void)
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
    const SfxItemSet* pExSet = GetDialogExampleSet();
    if(pExSet)
        aSet.Put(*pExSet);
    PutGridItem(aSet);
    m_aExampleWN.UpdateExample(aSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
