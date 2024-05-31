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
#include <fmtfsize.hxx>
#include <hintids.hxx>
#include <svtools/unitconv.hxx>
#include <vcl/fieldvalues.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/borderline.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/pageitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/grabbagitem.hxx>
#include <uitool.hxx>
#include <pagedesc.hxx>
#include <pgfnote.hxx>
#include <uiitems.hxx>

#include <memory>

using namespace ::com::sun::star;

const WhichRangesContainer SwFootNotePage::s_aPageRg(svl::Items<FN_PARAM_FTN_INFO, FN_PARAM_FTN_INFO>);

// handler to switch between the different possibilities how the footnote
// region's height can be set.
IMPL_LINK_NOARG(SwFootNotePage, HeightPage, weld::Toggleable&, void)
{
    if (m_xMaxHeightPageBtn->get_active())
        m_xMaxHeightEdit->set_sensitive(false);
}

IMPL_LINK_NOARG(SwFootNotePage, HeightMetric, weld::Toggleable&, void)
{
    if (m_xMaxHeightBtn->get_active())
    {
        m_xMaxHeightEdit->set_sensitive(true);
        m_xMaxHeightEdit->grab_focus();
    }
}

// handler limit values
IMPL_LINK_NOARG(SwFootNotePage, HeightModify, weld::MetricSpinButton&, void)
{
    m_xMaxHeightEdit->set_max(m_xMaxHeightEdit->normalize(m_lMaxHeight -
            (m_xDistEdit->denormalize(m_xDistEdit->get_value(FieldUnit::TWIP)) +
            m_xLineDistEdit->denormalize(m_xLineDistEdit->get_value(FieldUnit::TWIP)))),
            FieldUnit::TWIP);
    if (m_xMaxHeightEdit->get_value(FieldUnit::NONE) < 0)
        m_xMaxHeightEdit->set_value(0, FieldUnit::NONE);
    m_xDistEdit->set_max(m_xDistEdit->normalize(m_lMaxHeight -
            (m_xMaxHeightEdit->denormalize(m_xMaxHeightEdit->get_value(FieldUnit::TWIP)) +
            m_xLineDistEdit->denormalize(m_xLineDistEdit->get_value(FieldUnit::TWIP)))),
            FieldUnit::TWIP);
    if (m_xDistEdit->get_value(FieldUnit::NONE) < 0)
        m_xDistEdit->set_value(0, FieldUnit::NONE);
    m_xLineDistEdit->set_max(m_xLineDistEdit->normalize(m_lMaxHeight -
            (m_xMaxHeightEdit->denormalize(m_xMaxHeightEdit->get_value(FieldUnit::TWIP)) +
            m_xDistEdit->denormalize(m_xDistEdit->get_value(FieldUnit::TWIP)))),
            FieldUnit::TWIP);
}

IMPL_LINK_NOARG(SwFootNotePage, LineWidthChanged_Impl, weld::MetricSpinButton&, void)
{
    sal_Int64 nVal = m_xLineWidthEdit->get_value(FieldUnit::NONE);
    nVal = static_cast<sal_Int64>(vcl::ConvertDoubleValue(
                nVal,
                m_xLineWidthEdit->get_digits(),
                m_xLineWidthEdit->get_unit(), MapUnit::MapTwip ));
    m_xLineTypeBox->SetWidth(nVal);
}

IMPL_LINK(SwFootNotePage, LineColorSelected_Impl, ColorListBox&, rColorBox, void)
{
    m_xLineTypeBox->SetColor(rColorBox.GetSelectEntryColor());
}

SwFootNotePage::SwFootNotePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/footnoteareapage.ui", "FootnoteAreaPage", &rSet)
    , m_lMaxHeight(0)
    , m_xMaxHeightPageBtn(m_xBuilder->weld_radio_button("maxheightpage"))
    , m_xMaxHeightBtn(m_xBuilder->weld_radio_button("maxheight"))
    , m_xMaxHeightEdit(m_xBuilder->weld_metric_spin_button("maxheightsb", FieldUnit::CM))
    , m_xDistLabel(m_xBuilder->weld_label("spacetotextlabel"))
    , m_xDistEdit(m_xBuilder->weld_metric_spin_button("spacetotext", FieldUnit::CM))
    , m_xLinePosLabel(m_xBuilder->weld_label("positionlabel"))
    , m_xLinePosBox(m_xBuilder->weld_combo_box("position"))
    , m_xLineTypeBox(new SvtLineListBox(m_xBuilder->weld_menu_button("style")))
    , m_xLineWidthEdit(m_xBuilder->weld_metric_spin_button("thickness", FieldUnit::POINT))
    , m_xLineColorBox(new ColorListBox(m_xBuilder->weld_menu_button("color"),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xLineLengthLabel(m_xBuilder->weld_label("lengthlabel"))
    , m_xLineLengthEdit(m_xBuilder->weld_metric_spin_button("length", FieldUnit::PERCENT))
    , m_xLineDistLabel(m_xBuilder->weld_label("spacingtocontentslabel"))
    , m_xLineDistEdit(m_xBuilder->weld_metric_spin_button("spacingtocontents", FieldUnit::CM))
{
    SetExchangeSupport();
    FieldUnit aMetric = ::GetDfltMetric(false);
    ::SetFieldUnit(*m_xMaxHeightEdit, aMetric);
    ::SetFieldUnit(*m_xDistEdit, aMetric);
    ::SetFieldUnit(*m_xLineDistEdit, aMetric);
    MeasurementSystem eSys = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    tools::Long nHeightValue = MeasurementSystem::Metric != eSys ? 1440 : 1134;
    m_xMaxHeightEdit->set_value(m_xMaxHeightEdit->normalize(nHeightValue),FieldUnit::TWIP);

    bool bContinuousEndnotes = false;
    if (const SfxGrabBagItem* pGragbagItem = rSet.GetItemIfSet(SID_ATTR_CHAR_GRABBAG))
    {
        auto it = pGragbagItem->GetGrabBag().find("ContinuousEndnotes");
        if (it != pGragbagItem->GetGrabBag().end())
        {
            it->second >>= bContinuousEndnotes;
        }
    }

    if (bContinuousEndnotes)
    {
        // These are ignored in SwFootnoteContFrame::Format() and SwFootnoteContFrame::PaintLine(),
        // hide them.
        m_xDistLabel->set_visible(false);
        m_xDistEdit->set_visible(false);
        m_xLinePosLabel->set_visible(false);
        m_xLinePosBox->set_visible(false);
        m_xLineLengthLabel->set_visible(false);
        m_xLineLengthEdit->set_visible(false);
        m_xLineDistLabel->set_visible(false);
        m_xLineDistEdit->set_visible(false);
    }
}

SwFootNotePage::~SwFootNotePage()
{
    m_xLineColorBox.reset();
    m_xLineTypeBox.reset();
}

std::unique_ptr<SfxTabPage> SwFootNotePage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet)
{
    return std::make_unique<SwFootNotePage>(pPage, pController, *rSet);
}

void SwFootNotePage::Reset(const SfxItemSet *rSet)
{
    // if no example exists, otherwise Init here in Activate
    std::optional<SwPageFootnoteInfo> pDefFootnoteInfo;
    const SwPageFootnoteInfo* pFootnoteInfo;
    const SfxPoolItem* pItem = SfxTabPage::GetItem(*rSet, FN_PARAM_FTN_INFO);
    if( pItem )
    {
        pFootnoteInfo = &static_cast<const SwPageFootnoteInfoItem*>(pItem)->GetPageFootnoteInfo();
    }
    else
    {
        // when "standard" is being activated the footnote item is deleted,
        // that's why a footnote structure has to be created here
        pDefFootnoteInfo.emplace();
        pFootnoteInfo = &*pDefFootnoteInfo;
    }
        // footnote area's height
    SwTwips lHeight = pFootnoteInfo->GetHeight();
    if(lHeight)
    {
        m_xMaxHeightEdit->set_value(m_xMaxHeightEdit->normalize(lHeight),FieldUnit::TWIP);
        m_xMaxHeightBtn->set_active(true);
    }
    else
    {
        m_xMaxHeightPageBtn->set_active(true);
        m_xMaxHeightEdit->set_sensitive(false);
    }
    m_xMaxHeightPageBtn->connect_toggled(LINK(this,SwFootNotePage,HeightPage));
    m_xMaxHeightBtn->connect_toggled(LINK(this,SwFootNotePage,HeightMetric));
    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwFootNotePage, HeightModify);
    m_xMaxHeightEdit->connect_value_changed(aLk);
    m_xDistEdit->connect_value_changed(aLk);
    m_xLineDistEdit->connect_value_changed(aLk);

    // Separator width
    m_xLineWidthEdit->connect_value_changed(LINK(this, SwFootNotePage, LineWidthChanged_Impl));

    sal_Int64 nWidthPt = static_cast<sal_Int64>(vcl::ConvertDoubleValue(
            sal_Int64( pFootnoteInfo->GetLineWidth() ), m_xLineWidthEdit->get_digits(),
            MapUnit::MapTwip, m_xLineWidthEdit->get_unit( ) ));
    m_xLineWidthEdit->set_value(nWidthPt, FieldUnit::NONE);

    // Separator style
    m_xLineTypeBox->SetSourceUnit( FieldUnit::TWIP );

    m_xLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::SOLID),
        SvxBorderLineStyle::SOLID );
    m_xLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DOTTED),
        SvxBorderLineStyle::DOTTED );
    m_xLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DASHED),
        SvxBorderLineStyle::DASHED );
    m_xLineTypeBox->SetWidth( pFootnoteInfo->GetLineWidth( ) );
    m_xLineTypeBox->SelectEntry( pFootnoteInfo->GetLineStyle() );

    // Separator Color
    m_xLineColorBox->SelectEntry(pFootnoteInfo->GetLineColor());
    m_xLineColorBox->SetSelectHdl(LINK(this, SwFootNotePage, LineColorSelected_Impl));
    m_xLineTypeBox->SetColor(pFootnoteInfo->GetLineColor());

    // position
    m_xLinePosBox->set_active(static_cast<sal_Int32>(pFootnoteInfo->GetAdj()));

    // width
    Fraction aTmp( 100, 1 );
    aTmp *= pFootnoteInfo->GetWidth();
    m_xLineLengthEdit->set_value(static_cast<tools::Long>(aTmp), FieldUnit::PERCENT);

    // gap footnote area
    m_xDistEdit->set_value(m_xDistEdit->normalize(pFootnoteInfo->GetTopDist()), FieldUnit::TWIP);
    m_xLineDistEdit->set_value(
        m_xLineDistEdit->normalize(pFootnoteInfo->GetBottomDist()), FieldUnit::TWIP);
    ActivatePage( *rSet );
}

// stuff attributes into the set, when OK
bool SwFootNotePage::FillItemSet(SfxItemSet *rSet)
{
    SwPageFootnoteInfoItem aItem(GetItemSet().Get(FN_PARAM_FTN_INFO));

    // that's the original
    SwPageFootnoteInfo &rFootnoteInfo = aItem.GetPageFootnoteInfo();

    // footnote area's height
    if (m_xMaxHeightBtn->get_active())
        rFootnoteInfo.SetHeight( static_cast< SwTwips >(
                m_xMaxHeightEdit->denormalize(m_xMaxHeightEdit->get_value(FieldUnit::TWIP))));
    else
        rFootnoteInfo.SetHeight(0);

        // gap footnote area
    rFootnoteInfo.SetTopDist(  static_cast< SwTwips >(
            m_xDistEdit->denormalize(m_xDistEdit->get_value(FieldUnit::TWIP))));
    rFootnoteInfo.SetBottomDist(  static_cast< SwTwips >(
            m_xLineDistEdit->denormalize(m_xLineDistEdit->get_value(FieldUnit::TWIP))));

    // Separator style
    rFootnoteInfo.SetLineStyle(m_xLineTypeBox->GetSelectEntryStyle());

    // Separator width
    sal_Int64 nWidth = m_xLineWidthEdit->get_value(FieldUnit::NONE);
    nWidth = static_cast<tools::Long>(vcl::ConvertDoubleValue(
                   nWidth,
                   m_xLineWidthEdit->get_digits(),
                   m_xLineWidthEdit->get_unit(), MapUnit::MapTwip ));
    rFootnoteInfo.SetLineWidth( nWidth );

    // Separator color
    rFootnoteInfo.SetLineColor(m_xLineColorBox->GetSelectEntryColor());

    // Position
    rFootnoteInfo.SetAdj(static_cast<css::text::HorizontalAdjust>(m_xLinePosBox->get_active()));

    // Width
    rFootnoteInfo.SetWidth(Fraction(m_xLineLengthEdit->get_value(FieldUnit::PERCENT), 100));

    const SfxPoolItem* pOldItem;
    if(nullptr == (pOldItem = GetOldItem( *rSet, FN_PARAM_FTN_INFO )) ||
                aItem != *pOldItem )
        rSet->Put(aItem);

    return true;
}

void SwFootNotePage::ActivatePage(const SfxItemSet& rSet)
{
    auto const & rSize = rSet.Get( RES_FRM_SIZE );
    m_lMaxHeight = rSize.GetHeight();

    if( const SvxSetItem* pHeaderSetItem = rSet.GetItemIfSet( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_HEADERSET), false ) )
    {
        const SfxItemSet& rHeaderSet = pHeaderSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            rHeaderSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSizeItem =
                rHeaderSet.Get(rSet.GetPool()->GetWhich(SID_ATTR_PAGE_SIZE));
            m_lMaxHeight -= rSizeItem.GetSize().Height();
        }
    }

    if( const SvxSetItem* pFooterSetItem = rSet.GetItemIfSet( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_FOOTERSET),
            false ) )
    {
        const SfxItemSet& rFooterSet = pFooterSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn = rFooterSet.Get( SID_ATTR_PAGE_ON );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSizeItem =
                rFooterSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_SIZE ) );
            m_lMaxHeight -= rSizeItem.GetSize().Height();
        }
    }

    if ( const SvxULSpaceItem* pSpaceItem = rSet.GetItemIfSet( RES_UL_SPACE , false ) )
    {
        m_lMaxHeight -= pSpaceItem->GetUpper() + pSpaceItem->GetLower();
    }

    m_lMaxHeight *= 8;
    m_lMaxHeight /= 10;

    // set maximum values
    HeightModify(*m_xMaxHeightEdit);
}

DeactivateRC SwFootNotePage::DeactivatePage( SfxItemSet* _pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
