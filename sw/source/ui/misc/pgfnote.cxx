/*************************************************************************
 *
 *  $RCSfile: pgfnote.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:37:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _REF_HXX //to avoid internal compiler errors
#include <tools/ref.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX
#include <svx/pageitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif

#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _PGFNOTE_HXX
#include <pgfnote.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif

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

static const USHORT __FAR_DATA nLines[] = {
    0,
    50,
    100,
    150,
    200,
    500
};

static const USHORT nLineCount = sizeof(nLines) / sizeof(nLines[0]);

static USHORT __FAR_DATA aPageRg[] = {
    FN_PARAM_FTN_INFO, FN_PARAM_FTN_INFO,
    0
};


/*------------------------------------------------------------------------
 Beschreibung:  liefert zurueck, ob die Linienbreite nWidth bereits
                in der Listbox enthalten ist.
------------------------------------------------------------------------*/


BOOL lcl_HasLineWidth(USHORT nWidth)
{
    for(USHORT i = 0; i < nLineCount; ++i) {
        if(nLines[i] == nWidth)
            return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------
 Beschreibung:  Handler fuer umschalten zwischen den unterschiedlichen
                Arten, wie die Hoehe des Fussnotenbereiches angegeben
                werden kann.
------------------------------------------------------------------------*/


IMPL_LINK_INLINE_START( SwFootNotePage, HeightPage, Button *, EMPTYARG )
{
    aMaxHeightEdit.Enable(FALSE);
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
    FieldUnit aMetric = ::GetDfltMetric(FALSE);
    SetMetric( aMaxHeightEdit,  aMetric );
    SetMetric( aDistEdit,       aMetric );
    SetMetric( aLineDistEdit,   aMetric );
    MeasurementSystem eSys = GetAppLocaleData().getMeasurementSystemEnum();
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
        aMaxHeightBtn.Check(TRUE);
    }
    else
    {
        aMaxHeightPageBtn.Check(TRUE);
        aMaxHeightEdit.Enable(FALSE);
    }
    aMaxHeightPageBtn.SetClickHdl(LINK(this,SwFootNotePage,HeightPage));
    aMaxHeightBtn.SetClickHdl(LINK(this,SwFootNotePage,HeightMetric));
    Link aLk = LINK(this, SwFootNotePage, HeightModify);
    aMaxHeightEdit.SetLoseFocusHdl( aLk );
    aDistEdit.SetLoseFocusHdl( aLk );
    aLineDistEdit.SetLoseFocusHdl( aLk );

    // Trennlinie
    for(USHORT i = 0; i < nLineCount; ++i)
        aLineTypeBox.InsertEntry(nLines[i]);

    const USHORT nWidth = (USHORT)pFtnInfo->GetLineWidth() * TWIP_TO_LBOX;
    if ( !lcl_HasLineWidth(nWidth) )
        aLineTypeBox.InsertEntry(nWidth);
    aLineTypeBox.SelectEntry(nWidth);

        // Position
    aLinePosBox.SelectEntryPos(pFtnInfo->GetAdj());

        // Breite
    Fraction aTmp( 100, 1 );
    aTmp *= pFtnInfo->GetWidth();
    aLineWidthEdit.SetValue( aTmp );

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


BOOL SwFootNotePage::FillItemSet(SfxItemSet &rSet)
{
    SwPageFtnInfoItem aItem((const SwPageFtnInfoItem&)GetItemSet().Get(FN_PARAM_FTN_INFO));

    // Das ist das Original
    SwPageFtnInfo &rFtnInfo = aItem.GetPageFtnInfo();

        // Hoehe Fussnotenbereich
    if(aMaxHeightBtn.IsChecked())
        rFtnInfo.SetHeight(aMaxHeightEdit.
            Denormalize(aMaxHeightEdit.GetValue(FUNIT_TWIP)));
    else
        rFtnInfo.SetHeight(0);

        // Abstand Fussnotenbereich
    rFtnInfo.SetTopDist(aDistEdit.Denormalize(aDistEdit.GetValue(FUNIT_TWIP)));
    rFtnInfo.SetBottomDist(
        aLineDistEdit.Denormalize(aLineDistEdit.GetValue(FUNIT_TWIP)));

        // Trennlinie
    const USHORT nPos = aLineTypeBox.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos )
        rFtnInfo.SetLineWidth(nLines[nPos] / TWIP_TO_LBOX);

        // Position
    rFtnInfo.SetAdj((SwFtnAdj)aLinePosBox.GetSelectEntryPos());

        // Breite
    rFtnInfo.SetWidth(Fraction(aLineWidthEdit.GetValue(), 100));

    const SfxPoolItem* pOldItem;
    if(0 == (pOldItem = GetOldItem( rSet, FN_PARAM_FTN_INFO )) ||
                aItem != *pOldItem )
        rSet.Put(aItem);

    return TRUE;
}

void SwFootNotePage::ActivatePage(const SfxItemSet& rSet)
{
    const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get( RES_FRM_SIZE );
    lMaxHeight = rSize.GetSize().Height();

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_HEADERSET), FALSE, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                (const SvxSizeItem&)rHeaderSet.Get(rSet.GetPool()->GetWhich(SID_ATTR_PAGE_SIZE));
            lMaxHeight -= rSize.GetSize().Height();
        }
    }

    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_FOOTERSET),
            FALSE, &pItem ) )
    {
        const SfxItemSet& rFooterSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( SID_ATTR_PAGE_ON );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                (const SvxSizeItem&)rFooterSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_PAGE_SIZE ) );
            lMaxHeight -= rSize.GetSize().Height();
        }
    }

    if ( rSet.GetItemState( RES_UL_SPACE , FALSE ) == SFX_ITEM_SET )
    {
        const SvxULSpaceItem &rUL = (const SvxULSpaceItem&)rSet.Get( RES_UL_SPACE );
        lMaxHeight -= rUL.GetUpper() + rUL.GetLower();
    }

    lMaxHeight *= 8;
    lMaxHeight /= 10;

    // Maximalwerte setzen
    HeightModify(0);
}

int SwFootNotePage::DeactivatePage( SfxItemSet* pSet)
{
    if(pSet)
        FillItemSet(*pSet);

    return TRUE;
}

USHORT* SwFootNotePage::GetRanges()
{
    return aPageRg;
}



