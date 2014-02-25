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
#include <misc.hrc>

using namespace ::com::sun::star;

static sal_uInt16 aPageRg[] = {
    FN_PARAM_FTN_INFO, FN_PARAM_FTN_INFO,
    0
};

/*------------------------------------------------------------------------
 Description:  handler to switch between the different possibilities
               how the footnote region's height can be set.
------------------------------------------------------------------------*/
IMPL_LINK_NOARG_INLINE_START(SwFootNotePage, HeightPage)
{
    m_pMaxHeightEdit->Enable(false);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwFootNotePage, HeightPage)

IMPL_LINK_NOARG_INLINE_START(SwFootNotePage, HeightMetric)
{
    m_pMaxHeightEdit->Enable();
    m_pMaxHeightEdit->GrabFocus();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwFootNotePage, HeightMetric)

/*------------------------------------------------------------------------
 Description:   handler limit values
------------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFootNotePage, HeightModify)
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
    return 0;
}

IMPL_LINK_NOARG(SwFootNotePage, LineWidthChanged_Impl)
{
    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                m_pLineWidthEdit->GetValue( ),
                m_pLineWidthEdit->GetDecimalDigits( ),
                m_pLineWidthEdit->GetUnit(), MAP_TWIP ));
    m_pLineTypeBox->SetWidth( nVal );

    return 0;
}

IMPL_LINK_NOARG(SwFootNotePage, LineColorSelected_Impl)
{
    m_pLineTypeBox->SetColor( m_pLineColorBox->GetSelectEntryColor() );
    return 0;
}

SwFootNotePage::SwFootNotePage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "FootnoteAreaPage",
        "modules/swriter/ui/footnoteareapage.ui", rSet)
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
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(*m_pMaxHeightEdit, aMetric);
    SetMetric(*m_pDistEdit, aMetric);
    SetMetric(*m_pLineDistEdit, aMetric);
    MeasurementSystem eSys = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    long nHeightValue = MEASURE_METRIC != eSys ? 1440 : 1134;
    m_pMaxHeightEdit->SetValue(m_pMaxHeightEdit->Normalize(nHeightValue),FUNIT_TWIP);
}

SwFootNotePage::~SwFootNotePage()
{
}

SfxTabPage* SwFootNotePage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFootNotePage(pParent, rSet);
}

void SwFootNotePage::Reset(const SfxItemSet &rSet)
{
    // if no example exists, otherwise Init here in Activate
    SwPageFtnInfo* pDefFtnInfo = 0;
    const SwPageFtnInfo* pFtnInfo;
    const SfxPoolItem* pItem = SfxTabPage::GetItem(rSet, FN_PARAM_FTN_INFO);
    if( pItem )
    {
        pFtnInfo = &((const SwPageFtnInfoItem*)pItem)->GetPageFtnInfo();
    }
    else
    {
        // when "standard" is being activated the footnote item is deleted,
        // that's why a footnote structure has to be created here
        pDefFtnInfo = new SwPageFtnInfo();
        pFtnInfo = pDefFtnInfo;
    }
        // footnote area's height
    SwTwips lHeight = pFtnInfo->GetHeight();
    if(lHeight)
    {
        m_pMaxHeightEdit->SetValue(m_pMaxHeightEdit->Normalize(lHeight),FUNIT_TWIP);
        m_pMaxHeightBtn->Check(true);
    }
    else
    {
        m_pMaxHeightPageBtn->Check(true);
        m_pMaxHeightEdit->Enable(false);
    }
    m_pMaxHeightPageBtn->SetClickHdl(LINK(this,SwFootNotePage,HeightPage));
    m_pMaxHeightBtn->SetClickHdl(LINK(this,SwFootNotePage,HeightMetric));
    Link aLk = LINK(this, SwFootNotePage, HeightModify);
    m_pMaxHeightEdit->SetLoseFocusHdl( aLk );
    m_pDistEdit->SetLoseFocusHdl( aLk );
    m_pLineDistEdit->SetLoseFocusHdl( aLk );

    // Separator width
    m_pLineWidthEdit->SetModifyHdl( LINK( this, SwFootNotePage, LineWidthChanged_Impl ) );

    sal_Int64 nWidthPt = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
            sal_Int64( pFtnInfo->GetLineWidth() ), m_pLineWidthEdit->GetDecimalDigits(),
            MAP_TWIP, m_pLineWidthEdit->GetUnit( ) ));
    m_pLineWidthEdit->SetValue( nWidthPt );

    // Separator style
    m_pLineTypeBox->SetSourceUnit( FUNIT_TWIP );

    m_pLineTypeBox->SetNone(SW_RESSTR(SW_STR_NONE));
    m_pLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::SOLID),
        table::BorderLineStyle::SOLID );
    m_pLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::DOTTED),
        table::BorderLineStyle::DOTTED );
    m_pLineTypeBox->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::DASHED),
        table::BorderLineStyle::DASHED );
    m_pLineTypeBox->SetWidth( pFtnInfo->GetLineWidth( ) );
    m_pLineTypeBox->SelectEntry( pFtnInfo->GetLineStyle() );

    // Separator Color
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef pColorList;

    OSL_ENSURE( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem* pColorItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pColorItem != NULL )
            pColorList = ( (SvxColorListItem*)pColorItem )->GetColorList();
    }

    OSL_ENSURE( pColorList.is(), "ColorTable not found!" );

    if ( pColorList.is() )
    {
        m_pLineColorBox->SetUpdateMode( false );

        for ( long i = 0; i < pColorList->Count(); ++i )
        {
            XColorEntry* pEntry = pColorList->GetColor(i);
            m_pLineColorBox->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        m_pLineColorBox->SetUpdateMode( true );
    }

    // select color in the list or add it as a user color
    sal_uInt16 nSelPos = m_pLineColorBox->GetEntryPos( pFtnInfo->GetLineColor() );
    if( nSelPos == LISTBOX_ENTRY_NOTFOUND )
        nSelPos = m_pLineColorBox->InsertEntry( pFtnInfo->GetLineColor(),
                OUString( SW_RES( RID_SVXSTR_COLOR_USER ) ) );

    m_pLineColorBox->SetSelectHdl( LINK( this, SwFootNotePage, LineColorSelected_Impl ) );
    m_pLineColorBox->SelectEntryPos( nSelPos );
    m_pLineTypeBox->SetColor( pFtnInfo->GetLineColor() );

    // position
    m_pLinePosBox->SelectEntryPos( static_cast< sal_uInt16 >(pFtnInfo->GetAdj()) );

        // width
    Fraction aTmp( 100, 1 );
    aTmp *= pFtnInfo->GetWidth();
    m_pLineLengthEdit->SetValue( static_cast<long>(aTmp) );

        // gap footnote area
    m_pDistEdit->SetValue(m_pDistEdit->Normalize(pFtnInfo->GetTopDist()),FUNIT_TWIP);
    m_pLineDistEdit->SetValue(
        m_pLineDistEdit->Normalize(pFtnInfo->GetBottomDist()), FUNIT_TWIP);
    ActivatePage( rSet );
    delete pDefFtnInfo;
}

/*--------------------------------------------------------------------
    Description:    stuff attributes into the set, when OK
 --------------------------------------------------------------------*/
sal_Bool SwFootNotePage::FillItemSet(SfxItemSet &rSet)
{
    SwPageFtnInfoItem aItem((const SwPageFtnInfoItem&)GetItemSet().Get(FN_PARAM_FTN_INFO));

    // that's the original
    SwPageFtnInfo &rFtnInfo = aItem.GetPageFtnInfo();

        // footnote area's height
    if(m_pMaxHeightBtn->IsChecked())
        rFtnInfo.SetHeight( static_cast< SwTwips >(
                m_pMaxHeightEdit->Denormalize(m_pMaxHeightEdit->GetValue(FUNIT_TWIP))));
    else
        rFtnInfo.SetHeight(0);

        // gap footnote area
    rFtnInfo.SetTopDist(  static_cast< SwTwips >(
            m_pDistEdit->Denormalize(m_pDistEdit->GetValue(FUNIT_TWIP))));
    rFtnInfo.SetBottomDist(  static_cast< SwTwips >(
            m_pLineDistEdit->Denormalize(m_pLineDistEdit->GetValue(FUNIT_TWIP))));

    // Separator style
    rFtnInfo.SetLineStyle( ::editeng::SvxBorderStyle( m_pLineTypeBox->GetSelectEntryStyle() ) );

    // Separator width
    long nWidth = static_cast<long>(MetricField::ConvertDoubleValue(
                   m_pLineWidthEdit->GetValue( ),
                   m_pLineWidthEdit->GetDecimalDigits( ),
                   m_pLineWidthEdit->GetUnit(), MAP_TWIP ));
    rFtnInfo.SetLineWidth( nWidth );

    // Separator color
    rFtnInfo.SetLineColor( m_pLineColorBox->GetSelectEntryColor() );

        // Position
    rFtnInfo.SetAdj((SwFtnAdj)m_pLinePosBox->GetSelectEntryPos());

        // Breite
    rFtnInfo.SetWidth(Fraction( static_cast< long >(m_pLineLengthEdit->GetValue()), 100));

    const SfxPoolItem* pOldItem;
    if(0 == (pOldItem = GetOldItem( rSet, FN_PARAM_FTN_INFO )) ||
                aItem != *pOldItem )
        rSet.Put(aItem);

    return sal_True;
}

void SwFootNotePage::ActivatePage(const SfxItemSet& rSet)
{
    const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get( RES_FRM_SIZE );
    lMaxHeight = rSize.GetSize().Height();

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_HEADERSET), false, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSizeItem =
                (const SvxSizeItem&)rHeaderSet.Get(rSet.GetPool()->GetWhich(SID_ATTR_PAGE_SIZE));
            lMaxHeight -= rSizeItem.GetSize().Height();
        }
    }

    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_FOOTERSET),
            false, &pItem ) )
    {
        const SfxItemSet& rFooterSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( SID_ATTR_PAGE_ON );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSizeItem =
                (const SvxSizeItem&)rFooterSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_SIZE ) );
            lMaxHeight -= rSizeItem.GetSize().Height();
        }
    }

    if ( rSet.GetItemState( RES_UL_SPACE , false ) == SFX_ITEM_SET )
    {
        const SvxULSpaceItem &rUL = (const SvxULSpaceItem&)rSet.Get( RES_UL_SPACE );
        lMaxHeight -= rUL.GetUpper() + rUL.GetLower();
    }

    lMaxHeight *= 8;
    lMaxHeight /= 10;

    // set maximum values
    HeightModify(0);
}

int SwFootNotePage::DeactivatePage( SfxItemSet* _pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);

    return sal_True;
}

sal_uInt16* SwFootNotePage::GetRanges()
{
    return aPageRg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
