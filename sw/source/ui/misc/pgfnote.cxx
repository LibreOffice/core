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
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <sal/macros.h>
#include <vcl/field.hxx>
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/borderline.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/pageitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/ulspitem.hxx>
#include <uitool.hxx>
#include <pagedesc.hxx>
#include <pgfnote.hxx>
#include <uiitems.hxx>
#include <sfx2/objsh.hxx>

#include <globals.hrc>
#include <strings.hrc>
#include <misc.hrc>
#include <memory>

using namespace ::com::sun::star;

const sal_uInt16 SwFootNotePage::aPageRg[] = {
    FN_PARAM_FTN_INFO, FN_PARAM_FTN_INFO,
    0
};

// handler to switch between the different possibilities how the footnote
// region's height can be set.
IMPL_LINK_NOARG(SwFootNotePage, HeightPage, Button*, void)
{
    m_pMaxHeightEdit->Enable(false);
}

IMPL_LINK_NOARG(SwFootNotePage, HeightMetric, Button*, void)
{
    m_pMaxHeightEdit->Enable();
    m_pMaxHeightEdit->GrabFocus();
}

// handler limit values
IMPL_LINK_NOARG(SwFootNotePage, HeightModify, Control&, void)
{
    m_pMaxHeightEdit->SetMax(m_pMaxHeightEdit->Normalize(lMaxHeight -
            (m_pDistEdit->Denormalize(m_pDistEdit->GetValue(FUNIT_TWIP)) +
            m_pLineDistEdit->Denormalize(m_pLineDistEdit->GetValue(FUNIT_TWIP)))),
            FUNIT_TWIP);
    if(m_pMaxHeightEdit->GetValue() < 0)
        m_pMaxHeightEdit->SetValue(0);
    m_pDistEdit->SetMax(m_pDistEdit->Normalize(lMaxHeight -
            (m_pMaxHeightEdit->Denormalize(m_pMaxHeightEdit->GetValue(FUNIT_TWIP)) +
            m_pLineDistEdit->Denormalize(m_pLineDistEdit->GetValue(FUNIT_TWIP)))),
            FUNIT_TWIP);
    if(m_pDistEdit->GetValue() < 0)
        m_pDistEdit->SetValue(0);
    m_pLineDistEdit->SetMax(m_pLineDistEdit->Normalize(lMaxHeight -
            (m_pMaxHeightEdit->Denormalize(m_pMaxHeightEdit->GetValue(FUNIT_TWIP)) +
            m_pDistEdit->Denormalize(m_pDistEdit->GetValue(FUNIT_TWIP)))),
            FUNIT_TWIP);
}

IMPL_LINK_NOARG(SwFootNotePage, LineWidthChanged_Impl, Edit&, void)
{
    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                m_pLineWidthEdit->GetValue( ),
                m_pLineWidthEdit->GetDecimalDigits( ),
                m_pLineWidthEdit->GetUnit(), MapUnit::MapTwip ));
    m_pLineTypeBox->SetWidth( nVal );
}

IMPL_LINK(SwFootNotePage, LineColorSelected_Impl, SvxColorListBox&, rColorBox, void)
{
    m_pLineTypeBox->SetColor(rColorBox.GetSelectEntryColor());
}

SwFootNotePage::SwFootNotePage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "FootnoteAreaPage",
        "modules/swriter/ui/footnoteareapage.ui", &rSet)
    , lMaxHeight(0)
{
    get(m_pMaxHeightPageBtn, "maxheightpage");
    get(m_pMaxHeightBtn, "maxheight");
    get(m_pMaxHeightEdit, "maxheightsb");
    get(m_pDistEdit, "spacetotext");

    get(m_pLinePosBox, "position");
    get(m_pLineTypeBox, "style");
    get(m_pLineWidthEdit, "thickness");
    get(m_pLineColorBox, "color");
    get(m_pLineLengthEdit, "length");
    get(m_pLineDistEdit, "spacingtocontents");

    SetExchangeSupport();
    FieldUnit aMetric = ::GetDfltMetric(false);
    SetMetric(*m_pMaxHeightEdit, aMetric);
    SetMetric(*m_pDistEdit, aMetric);
    SetMetric(*m_pLineDistEdit, aMetric);
    MeasurementSystem eSys = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    long nHeightValue = MeasurementSystem::Metric != eSys ? 1440 : 1134;
    m_pMaxHeightEdit->SetValue(m_pMaxHeightEdit->Normalize(nHeightValue),FUNIT_TWIP);
}

SwFootNotePage::~SwFootNotePage()
{
    disposeOnce();
}

void SwFootNotePage::dispose()
{
    m_pMaxHeightPageBtn.clear();
    m_pMaxHeightBtn.clear();
    m_pMaxHeightEdit.clear();
    m_pDistEdit.clear();
    m_pLinePosBox.clear();
    m_pLineTypeBox.clear();
    m_pLineWidthEdit.clear();
    m_pLineColorBox.clear();
    m_pLineLengthEdit.clear();
    m_pLineDistEdit.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwFootNotePage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFootNotePage>::Create(pParent, *rSet);
}

void SwFootNotePage::Reset(const SfxItemSet *rSet)
{
    // if no example exists, otherwise Init here in Activate
    std::unique_ptr<SwPageFootnoteInfo> pDefFootnoteInfo;
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
        pDefFootnoteInfo.reset(new SwPageFootnoteInfo());
        pFootnoteInfo = pDefFootnoteInfo.get();
    }
        // footnote area's height
    SwTwips lHeight = pFootnoteInfo->GetHeight();
    if(lHeight)
    {
        m_pMaxHeightEdit->SetValue(m_pMaxHeightEdit->Normalize(lHeight),FUNIT_TWIP);
        m_pMaxHeightBtn->Check();
    }
    else
    {
        m_pMaxHeightPageBtn->Check();
        m_pMaxHeightEdit->Enable(false);
    }
    m_pMaxHeightPageBtn->SetClickHdl(LINK(this,SwFootNotePage,HeightPage));
    m_pMaxHeightBtn->SetClickHdl(LINK(this,SwFootNotePage,HeightMetric));
    Link<Control&,void> aLk = LINK(this, SwFootNotePage, HeightModify);
    m_pMaxHeightEdit->SetLoseFocusHdl( aLk );
    m_pDistEdit->SetLoseFocusHdl( aLk );
    m_pLineDistEdit->SetLoseFocusHdl( aLk );

    // Separator width
    m_pLineWidthEdit->SetModifyHdl( LINK( this, SwFootNotePage, LineWidthChanged_Impl ) );

    sal_Int64 nWidthPt = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
            sal_Int64( pFootnoteInfo->GetLineWidth() ), m_pLineWidthEdit->GetDecimalDigits(),
            MapUnit::MapTwip, m_pLineWidthEdit->GetUnit( ) ));
    m_pLineWidthEdit->SetValue( nWidthPt );

    // Separator style
    m_pLineTypeBox->SetSourceUnit( FUNIT_TWIP );

    m_pLineTypeBox->SetNone(SwResId(SW_STR_NONE));
    m_pLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::SOLID),
        SvxBorderLineStyle::SOLID );
    m_pLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DOTTED),
        SvxBorderLineStyle::DOTTED );
    m_pLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DASHED),
        SvxBorderLineStyle::DASHED );
    m_pLineTypeBox->SetWidth( pFootnoteInfo->GetLineWidth( ) );
    m_pLineTypeBox->SelectEntry( pFootnoteInfo->GetLineStyle() );

    // Separator Color
    m_pLineColorBox->SelectEntry(pFootnoteInfo->GetLineColor());
    m_pLineColorBox->SetSelectHdl(LINK(this, SwFootNotePage, LineColorSelected_Impl));
    m_pLineTypeBox->SetColor(pFootnoteInfo->GetLineColor());

    // position
    m_pLinePosBox->SelectEntryPos( static_cast< sal_Int32 >(pFootnoteInfo->GetAdj()) );

        // width
    Fraction aTmp( 100, 1 );
    aTmp *= pFootnoteInfo->GetWidth();
    m_pLineLengthEdit->SetValue( static_cast<long>(aTmp) );

        // gap footnote area
    m_pDistEdit->SetValue(m_pDistEdit->Normalize(pFootnoteInfo->GetTopDist()),FUNIT_TWIP);
    m_pLineDistEdit->SetValue(
        m_pLineDistEdit->Normalize(pFootnoteInfo->GetBottomDist()), FUNIT_TWIP);
    ActivatePage( *rSet );
}

// stuff attributes into the set, when OK
bool SwFootNotePage::FillItemSet(SfxItemSet *rSet)
{
    SwPageFootnoteInfoItem aItem(static_cast<const SwPageFootnoteInfoItem&>(GetItemSet().Get(FN_PARAM_FTN_INFO)));

    // that's the original
    SwPageFootnoteInfo &rFootnoteInfo = aItem.GetPageFootnoteInfo();

        // footnote area's height
    if(m_pMaxHeightBtn->IsChecked())
        rFootnoteInfo.SetHeight( static_cast< SwTwips >(
                m_pMaxHeightEdit->Denormalize(m_pMaxHeightEdit->GetValue(FUNIT_TWIP))));
    else
        rFootnoteInfo.SetHeight(0);

        // gap footnote area
    rFootnoteInfo.SetTopDist(  static_cast< SwTwips >(
            m_pDistEdit->Denormalize(m_pDistEdit->GetValue(FUNIT_TWIP))));
    rFootnoteInfo.SetBottomDist(  static_cast< SwTwips >(
            m_pLineDistEdit->Denormalize(m_pLineDistEdit->GetValue(FUNIT_TWIP))));

    // Separator style
    rFootnoteInfo.SetLineStyle( m_pLineTypeBox->GetSelectEntryStyle() );

    // Separator width
    long nWidth = static_cast<long>(MetricField::ConvertDoubleValue(
                   m_pLineWidthEdit->GetValue( ),
                   m_pLineWidthEdit->GetDecimalDigits( ),
                   m_pLineWidthEdit->GetUnit(), MapUnit::MapTwip ));
    rFootnoteInfo.SetLineWidth( nWidth );

    // Separator color
    rFootnoteInfo.SetLineColor( m_pLineColorBox->GetSelectEntryColor() );

    // Position
    rFootnoteInfo.SetAdj((css::text::HorizontalAdjust)m_pLinePosBox->GetSelectEntryPos());

    // Width
    rFootnoteInfo.SetWidth(Fraction( static_cast< long >(m_pLineLengthEdit->GetValue()), 100));

    const SfxPoolItem* pOldItem;
    if(nullptr == (pOldItem = GetOldItem( *rSet, FN_PARAM_FTN_INFO )) ||
                aItem != *pOldItem )
        rSet->Put(aItem);

    return true;
}

void SwFootNotePage::ActivatePage(const SfxItemSet& rSet)
{
    auto const & rSize = static_cast<SwFormatFrameSize const &>(rSet.Get( RES_FRM_SIZE ));
    lMaxHeight = rSize.GetHeight();

    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_HEADERSET), false, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            static_cast<const SfxBoolItem&>(rHeaderSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_ON ) ));

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSizeItem =
                static_cast<const SvxSizeItem&>(rHeaderSet.Get(rSet.GetPool()->GetWhich(SID_ATTR_PAGE_SIZE)));
            lMaxHeight -= rSizeItem.GetSize().Height();
        }
    }

    if( SfxItemState::SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_FOOTERSET),
            false, &pItem ) )
    {
        const SfxItemSet& rFooterSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn =
            static_cast<const SfxBoolItem&>(rFooterSet.Get( SID_ATTR_PAGE_ON ));

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSizeItem =
                static_cast<const SvxSizeItem&>(rFooterSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_SIZE ) ));
            lMaxHeight -= rSizeItem.GetSize().Height();
        }
    }

    if ( rSet.GetItemState( RES_UL_SPACE , false ) == SfxItemState::SET )
    {
        const SvxULSpaceItem &rUL = static_cast<const SvxULSpaceItem&>(rSet.Get( RES_UL_SPACE ));
        lMaxHeight -= rUL.GetUpper() + rUL.GetLower();
    }

    lMaxHeight *= 8;
    lMaxHeight /= 10;

    // set maximum values
    HeightModify(*m_pMaxHeightEdit);
}

DeactivateRC SwFootNotePage::DeactivatePage( SfxItemSet* _pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
