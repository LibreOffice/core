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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <hintids.hxx>
#include <tools/ref.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <vcl/field.hxx>
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/pageitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/ulspitem.hxx>
#include <uitool.hxx>
#include <pagedesc.hxx>
#include <pgfnote.hxx>
#include <uiitems.hxx>

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _PGFNOTE_HRC
#include <pgfnote.hrc>
#endif

#define TWIP_TO_LBOX 5
/*-----------------------------------------------------#---------------
    Beschreibung:   vordefinierte Linien in Point
 --------------------------------------------------------------------*/

static const sal_uInt16 __FAR_DATA nLines[] = {
    0,
    50,
    100,
    150,
    200,
    500
};

static const sal_uInt16 nLineCount = sizeof(nLines) / sizeof(nLines[0]);

static sal_uInt16 __FAR_DATA aPageRg[] = {
    FN_PARAM_FTN_INFO, FN_PARAM_FTN_INFO,
    0
};


/*------------------------------------------------------------------------
 Beschreibung:  liefert zurueck, ob die Linienbreite nWidth bereits
                in der Listbox enthalten ist.
------------------------------------------------------------------------*/


sal_Bool lcl_HasLineWidth(sal_uInt16 nWidth)
{
    for(sal_uInt16 i = 0; i < nLineCount; ++i) {
        if(nLines[i] == nWidth)
            return sal_True;
    }
    return sal_False;
}

/*------------------------------------------------------------------------
 Beschreibung:  Handler fuer umschalten zwischen den unterschiedlichen
                Arten, wie die Hoehe des Fussnotenbereiches angegeben
                werden kann.
------------------------------------------------------------------------*/


IMPL_LINK_INLINE_START( SwFootNotePage, HeightPage, Button *, EMPTYARG )
{
    aMaxHeightEdit.Enable(sal_False);
    return 0;
}
IMPL_LINK_INLINE_END( SwFootNotePage, HeightPage, Button *, EMPTYARG )


IMPL_LINK_INLINE_START( SwFootNotePage, HeightMetric, Button *, EMPTYARG )
{
    aMaxHeightEdit.Enable();
    aMaxHeightEdit.GrabFocus();
    return 0;
}
IMPL_LINK_INLINE_END( SwFootNotePage, HeightMetric, Button *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:  Handler Grenzwerte
------------------------------------------------------------------------*/


IMPL_LINK( SwFootNotePage, HeightModify, MetricField *, EMPTYARG )
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

// CTOR / DTOR -----------------------------------------------------------

SwFootNotePage::SwFootNotePage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage(pParent, SW_RES(TP_FOOTNOTE_PAGE), rSet),

    aMaxHeightPageBtn(this, SW_RES(RB_MAXHEIGHT_PAGE)),
    aMaxHeightBtn(this,     SW_RES(RB_MAXHEIGHT)),
    aMaxHeightEdit(this,    SW_RES(ED_MAXHEIGHT)),
    aDistLbl(this,          SW_RES(FT_DIST)),
    aDistEdit(this,         SW_RES(ED_DIST)),
    aPosHeader(this,        SW_RES(FL_FOOTNOTE_SIZE)),

    aLinePosLbl(this,       SW_RES(FT_LINEPOS)),
    aLinePosBox(this,       SW_RES(DLB_LINEPOS)),
    aLineTypeLbl(this,      SW_RES(FT_LINETYPE)),
    aLineTypeBox(this,      SW_RES(DLB_LINETYPE)),
    aLineWidthLbl(this,     SW_RES(FT_LINEWIDTH)),
    aLineWidthEdit(this,    SW_RES(ED_LINEWIDTH)),
    aLineDistLbl(this,      SW_RES(FT_LINEDIST)),
    aLineDistEdit(this,     SW_RES(ED_LINEDIST)),
    aLineHeader(this,       SW_RES(FL_LINE))
{
    FreeResource();

    SetExchangeSupport();
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric( aMaxHeightEdit,  aMetric );
    SetMetric( aDistEdit,       aMetric );
    SetMetric( aLineDistEdit,   aMetric );
    MeasurementSystem eSys = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    long nHeightValue = MEASURE_METRIC != eSys ? 1440 : 1134;
    aMaxHeightEdit.SetValue(aMaxHeightEdit.Normalize(nHeightValue),FUNIT_TWIP);;
}

SwFootNotePage::~SwFootNotePage()
{
}


SfxTabPage* SwFootNotePage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFootNotePage(pParent, rSet);
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwFootNotePage::Reset(const SfxItemSet &rSet)
{
    // Falls noch kein Bsp vorhanden Init hier sonst im Activate
    //
    SwPageFtnInfo* pDefFtnInfo = 0;
    const SwPageFtnInfo* pFtnInfo;
    const SfxPoolItem* pItem = SfxTabPage::GetItem(rSet, FN_PARAM_FTN_INFO);
    if( pItem )
    {
        pFtnInfo = &((const SwPageFtnInfoItem*)pItem)->GetPageFtnInfo();
    }
    else
    {
        // wenn "Standard" betaetigt wird, wird das Fussnotenitem geloescht,
        // deswegen muss hier eine Fussnotenstruktur erzeugt werden
        pDefFtnInfo = new SwPageFtnInfo();
        pFtnInfo = pDefFtnInfo;
    }
        // Hoehe Fussnotenbereich
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

    // Trennlinie
    for(sal_uInt16 i = 0; i < nLineCount; ++i)
        aLineTypeBox.InsertEntry(nLines[i]);

    const sal_uInt16 nWidth = (sal_uInt16)pFtnInfo->GetLineWidth() * TWIP_TO_LBOX;
    if ( !lcl_HasLineWidth(nWidth) )
        aLineTypeBox.InsertEntry(nWidth);
    aLineTypeBox.SelectEntry(nWidth);

    // Position
    aLinePosBox.SelectEntryPos( static_cast< sal_uInt16 >(pFtnInfo->GetAdj()) );

        // Breite
    Fraction aTmp( 100, 1 );
    aTmp *= pFtnInfo->GetWidth();
    aLineWidthEdit.SetValue( static_cast<long>(aTmp) );

        // Abstand Fussnotenbereich
    aDistEdit.SetValue(aDistEdit.Normalize(pFtnInfo->GetTopDist()),FUNIT_TWIP);
    aLineDistEdit.SetValue(
        aLineDistEdit.Normalize(pFtnInfo->GetBottomDist()), FUNIT_TWIP);
    ActivatePage( rSet );
    delete pDefFtnInfo;
}

/*--------------------------------------------------------------------
    Beschreibung:   Attribute in den Set stopfen bei OK
 --------------------------------------------------------------------*/


sal_Bool SwFootNotePage::FillItemSet(SfxItemSet &rSet)
{
    SwPageFtnInfoItem aItem((const SwPageFtnInfoItem&)GetItemSet().Get(FN_PARAM_FTN_INFO));

    // Das ist das Original
    SwPageFtnInfo &rFtnInfo = aItem.GetPageFtnInfo();

        // Hoehe Fussnotenbereich
    if(aMaxHeightBtn.IsChecked())
        rFtnInfo.SetHeight( static_cast< SwTwips >(
                aMaxHeightEdit.Denormalize(aMaxHeightEdit.GetValue(FUNIT_TWIP))));
    else
        rFtnInfo.SetHeight(0);

        // Abstand Fussnotenbereich
    rFtnInfo.SetTopDist(  static_cast< SwTwips >(
            aDistEdit.Denormalize(aDistEdit.GetValue(FUNIT_TWIP))));
    rFtnInfo.SetBottomDist(  static_cast< SwTwips >(
            aLineDistEdit.Denormalize(aLineDistEdit.GetValue(FUNIT_TWIP))));

        // Trennlinie
    const sal_uInt16 nPos = aLineTypeBox.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos )
        rFtnInfo.SetLineWidth(nLines[nPos] / TWIP_TO_LBOX);

        // Position
    rFtnInfo.SetAdj((SwFtnAdj)aLinePosBox.GetSelectEntryPos());

        // Breite
    rFtnInfo.SetWidth(Fraction( static_cast< long >(aLineWidthEdit.GetValue()), 100));

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

    // Maximalwerte setzen
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



