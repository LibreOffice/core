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

#include <cmdid.h>
#include <hintids.hxx>
#include <svx/dialogs.hrc>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>               // XColorList
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
#include <pgfnote.hrc>


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
    aMaxHeightEdit.Enable(sal_False);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwFootNotePage, HeightPage)


IMPL_LINK_NOARG_INLINE_START(SwFootNotePage, HeightMetric)
{
    aMaxHeightEdit.Enable();
    aMaxHeightEdit.GrabFocus();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwFootNotePage, HeightMetric)

/*------------------------------------------------------------------------
 Description:   handler limit values
------------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFootNotePage, HeightModify)
{
    aMaxHeightEdit.SetMax(aMaxHeightEdit.Normalize(lMaxHeight -
            (aDistEdit.Denormalize(aDistEdit.GetValue(FUNIT_TWIP)) +
            aLineDistEdit.Denormalize(aLineDistEdit.GetValue(FUNIT_TWIP)))),
            FUNIT_TWIP);
    if(aMaxHeightEdit.GetValue() < 0)
        aMaxHeightEdit.SetValue(0);
    aDistEdit.SetMax(aDistEdit.Normalize(lMaxHeight -
            (aMaxHeightEdit.Denormalize(aMaxHeightEdit.GetValue(FUNIT_TWIP)) +
            aLineDistEdit.Denormalize(aLineDistEdit.GetValue(FUNIT_TWIP)))),
            FUNIT_TWIP);
    if(aDistEdit.GetValue() < 0)
        aDistEdit.SetValue(0);
    aLineDistEdit.SetMax(aLineDistEdit.Normalize(lMaxHeight -
            (aMaxHeightEdit.Denormalize(aMaxHeightEdit.GetValue(FUNIT_TWIP)) +
            aDistEdit.Denormalize(aDistEdit.GetValue(FUNIT_TWIP)))),
            FUNIT_TWIP);
    return 0;
}

IMPL_LINK_NOARG(SwFootNotePage, LineWidthChanged_Impl)
{
    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                aLineWidthEdit.GetValue( ),
                aLineWidthEdit.GetDecimalDigits( ),
                aLineWidthEdit.GetUnit(), MAP_TWIP ));
    aLineTypeBox.SetWidth( nVal );

    return 0;
}

IMPL_LINK_NOARG(SwFootNotePage, LineColorSelected_Impl)
{
    aLineTypeBox.SetColor( aLineColorBox.GetSelectEntryColor() );
    return 0;
}

// CTOR / DTOR -----------------------------------------------------------

SwFootNotePage::SwFootNotePage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage(pParent, SW_RES(TP_FOOTNOTE_PAGE), rSet),
    aPosHeader(this,        SW_RES(FL_FOOTNOTE_SIZE)),
    aMaxHeightPageBtn(this, SW_RES(RB_MAXHEIGHT_PAGE)),
    aMaxHeightBtn(this,     SW_RES(RB_MAXHEIGHT)),
    aMaxHeightEdit(this,    SW_RES(ED_MAXHEIGHT)),
    aDistLbl(this,          SW_RES(FT_DIST)),
    aDistEdit(this,         SW_RES(ED_DIST)),

    aLineHeader(this,       SW_RES(FL_LINE)),
    aLinePosLbl(this,       SW_RES(FT_LINEPOS)),
    aLinePosBox(this,       SW_RES(DLB_LINEPOS)),
    aLineTypeLbl(this,      SW_RES(FT_LINETYPE)),
    aLineTypeBox(this,      SW_RES(DLB_LINETYPE)),
    aLineWidthLbl(this,     SW_RES(FT_LINEWIDTH)),
    aLineWidthEdit(this,    SW_RES(ED_LINEWIDTH)),
    aLineColorLbl(this,     SW_RES(FT_LINECOLOR)),
    aLineColorBox(this,     SW_RES(DLB_LINECOLOR)),
    aLineLengthLbl(this,    SW_RES(FT_LINELENGTH)),
    aLineLengthEdit(this,   SW_RES(ED_LINELENGTH)),
    aLineDistLbl(this,      SW_RES(FT_LINEDIST)),
    aLineDistEdit(this,     SW_RES(ED_LINEDIST))
    {
    FreeResource();

    SetExchangeSupport();
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric( aMaxHeightEdit,  aMetric );
    SetMetric( aDistEdit,       aMetric );
    SetMetric( aLineDistEdit,   aMetric );
    MeasurementSystem eSys = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    long nHeightValue = MEASURE_METRIC != eSys ? 1440 : 1134;
    aMaxHeightEdit.SetValue(aMaxHeightEdit.Normalize(nHeightValue),FUNIT_TWIP);
    aMaxHeightEdit.SetAccessibleRelationLabeledBy(&aMaxHeightBtn);
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
        aMaxHeightEdit.SetValue(aMaxHeightEdit.Normalize(lHeight),FUNIT_TWIP);
        aMaxHeightBtn.Check(sal_True);
    }
    else
    {
        aMaxHeightPageBtn.Check(sal_True);
        aMaxHeightEdit.Enable(sal_False);
    }
    aMaxHeightPageBtn.SetClickHdl(LINK(this,SwFootNotePage,HeightPage));
    aMaxHeightBtn.SetClickHdl(LINK(this,SwFootNotePage,HeightMetric));
    Link aLk = LINK(this, SwFootNotePage, HeightModify);
    aMaxHeightEdit.SetLoseFocusHdl( aLk );
    aDistEdit.SetLoseFocusHdl( aLk );
    aLineDistEdit.SetLoseFocusHdl( aLk );

    // Separator width
    aLineWidthEdit.SetModifyHdl( LINK( this, SwFootNotePage, LineWidthChanged_Impl ) );

    sal_Int64 nWidthPt = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
            sal_Int64( pFtnInfo->GetLineWidth() ), aLineWidthEdit.GetDecimalDigits(),
            MAP_TWIP, aLineWidthEdit.GetUnit( ) ));
    aLineWidthEdit.SetValue( nWidthPt );

    // Separator style
    aLineTypeBox.SetSourceUnit( FUNIT_TWIP );

    aLineTypeBox.SetNone( String( SW_RES( STR_NONE ) ) );
    aLineTypeBox.InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::SOLID),
        table::BorderLineStyle::SOLID );
    aLineTypeBox.InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::DOTTED),
        table::BorderLineStyle::DOTTED );
    aLineTypeBox.InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::DASHED),
        table::BorderLineStyle::DASHED );
    aLineTypeBox.SetWidth( pFtnInfo->GetLineWidth( ) );
    aLineTypeBox.SelectEntry( pFtnInfo->GetLineStyle() );

    // Separator Color
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pColorItem  = NULL;
    XColorListRef pColorList;

    OSL_ENSURE( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        pColorItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pColorItem != NULL )
            pColorList = ( (SvxColorListItem*)pColorItem )->GetColorList();
    }

    OSL_ENSURE( pColorList.is(), "ColorTable not found!" );

    if ( pColorList.is() )
    {
        aLineColorBox.SetUpdateMode( sal_False );

        for ( long i = 0; i < pColorList->Count(); ++i )
        {
            XColorEntry* pEntry = pColorList->GetColor(i);
            aLineColorBox.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        aLineColorBox.SetUpdateMode( sal_True );
    }

    // select color in the list or add it as a user color
    sal_uInt16 nSelPos = aLineColorBox.GetEntryPos( pFtnInfo->GetLineColor() );
    if( nSelPos == LISTBOX_ENTRY_NOTFOUND )
        nSelPos = aLineColorBox.InsertEntry( pFtnInfo->GetLineColor(),
                String( SW_RES( RID_SVXSTR_COLOR_USER ) ) );

    aLineColorBox.SetSelectHdl( LINK( this, SwFootNotePage, LineColorSelected_Impl ) );
    aLineColorBox.SelectEntryPos( nSelPos );
    aLineTypeBox.SetColor( pFtnInfo->GetLineColor() );


    // position
    aLinePosBox.SelectEntryPos( static_cast< sal_uInt16 >(pFtnInfo->GetAdj()) );

        // width
    Fraction aTmp( 100, 1 );
    aTmp *= pFtnInfo->GetWidth();
    aLineLengthEdit.SetValue( static_cast<long>(aTmp) );

        // gap footnote area
    aDistEdit.SetValue(aDistEdit.Normalize(pFtnInfo->GetTopDist()),FUNIT_TWIP);
    aLineDistEdit.SetValue(
        aLineDistEdit.Normalize(pFtnInfo->GetBottomDist()), FUNIT_TWIP);
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
    if(aMaxHeightBtn.IsChecked())
        rFtnInfo.SetHeight( static_cast< SwTwips >(
                aMaxHeightEdit.Denormalize(aMaxHeightEdit.GetValue(FUNIT_TWIP))));
    else
        rFtnInfo.SetHeight(0);

        // gap footnote area
    rFtnInfo.SetTopDist(  static_cast< SwTwips >(
            aDistEdit.Denormalize(aDistEdit.GetValue(FUNIT_TWIP))));
    rFtnInfo.SetBottomDist(  static_cast< SwTwips >(
            aLineDistEdit.Denormalize(aLineDistEdit.GetValue(FUNIT_TWIP))));

    // Separator style
    rFtnInfo.SetLineStyle( ::editeng::SvxBorderStyle( aLineTypeBox.GetSelectEntryStyle() ) );

    // Separator width
    long nWidth = static_cast<long>(MetricField::ConvertDoubleValue(
                   aLineWidthEdit.GetValue( ),
                   aLineWidthEdit.GetDecimalDigits( ),
                   aLineWidthEdit.GetUnit(), MAP_TWIP ));
    rFtnInfo.SetLineWidth( nWidth );

    // Separator color
    rFtnInfo.SetLineColor( aLineColorBox.GetSelectEntryColor() );

        // Position
    rFtnInfo.SetAdj((SwFtnAdj)aLinePosBox.GetSelectEntryPos());

        // Breite
    rFtnInfo.SetWidth(Fraction( static_cast< long >(aLineLengthEdit.GetValue()), 100));

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
    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_HEADERSET), sal_False, &pItem ) )
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
            sal_False, &pItem ) )
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

    if ( rSet.GetItemState( RES_UL_SPACE , sal_False ) == SFX_ITEM_SET )
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
