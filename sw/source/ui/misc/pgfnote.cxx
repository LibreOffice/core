/*************************************************************************
 *
 *  $RCSfile: pgfnote.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:45 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _REF_HXX //to avoid internal compiler errors
#include <tools/ref.hxx>
#endif

#include "cmdid.h"
#include "hintids.hxx"
#include "uiparam.hxx"

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


#include "uitool.hxx"
#include "globals.hrc"
#include "pagedesc.hxx"
#include "pgfnote.hxx"
#include "uiitems.hxx"

#include "misc.hrc"
#include "pgfnote.hrc"

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
    aPosFrm(this,           SW_RES(GB_FOOTNOTE_SIZE)),

    aLinePosLbl(this,       SW_RES(FT_LINEPOS)),
    aLinePosBox(this,       SW_RES(DLB_LINEPOS)),
    aLineTypeLbl(this,      SW_RES(FT_LINETYPE)),
    aLineTypeBox(this,      SW_RES(DLB_LINETYPE)),
    aLineWidthLbl(this,     SW_RES(FT_LINEWIDTH)),
    aLineWidthEdit(this,    SW_RES(ED_LINEWIDTH)),
    aLineDistLbl(this,      SW_RES(FT_LINEDIST)),
    aLineDistEdit(this,     SW_RES(ED_LINEDIST)),
    aLineFrm(this,          SW_RES(GB_LINE)),
    aBspWin(this,           SW_RES(WN_BSP)),
    aBspFrm(this,           SW_RES(GB_BSP))
{
    FreeResource();

    SetExchangeSupport();
    FieldUnit aMetric = ::GetDfltMetric(FALSE);
    SetMetric( aMaxHeightEdit,  aMetric );
    SetMetric( aDistEdit,       aMetric );
    SetMetric( aLineDistEdit,   aMetric );
    MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
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
    if(lcl_HasLineWidth(nWidth))
        aLineTypeBox.SelectEntry(nWidth);
    else
    {
        aLineTypeBox.InsertEntry(nWidth);
        aLineTypeBox.SelectEntry(nWidth);
    }

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
    //Pen aPen(rFtnInfo.GetPen());

    const USHORT nPos = aLineTypeBox.GetSelectEntryPos();
        if( LISTBOX_ENTRY_NOTFOUND != nPos )
            rFtnInfo.SetLineWidth(nLines[nPos] / TWIP_TO_LBOX);
    //rFtnInfo.SetPen(aPen);

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

/*--------------------------------------------------------------------
    Beschreibung:   Bsp Uebernehmen
 --------------------------------------------------------------------*/


void SwFootNotePage::ActivatePage(const SfxItemSet& rSet)
{
    aBspWin.UpdateExample( rSet );
    lMaxHeight = aBspWin.GetSize().Height() -
                 aBspWin.GetHdDist() - aBspWin.GetHdHeight() -
                 aBspWin.GetFtDist() - aBspWin.GetFtHeight()-
                 aBspWin.GetTop() - aBspWin.GetBottom();
    lMaxHeight *= 8;
    lMaxHeight /= 10;

    // Maximalwerte setzen
    HeightModify(0);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.79  2000/09/18 16:05:59  willem.vandorp
    OpenOffice header added.

    Revision 1.78  2000/03/03 15:17:03  os
    StarView remainders removed

    Revision 1.77  2000/02/11 14:56:44  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.76  1999/09/20 11:23:04  os
    set initial value for max footnote height

    Revision 1.75  1999/07/29 11:40:06  MA
    avoid internal compiler errors


      Rev 1.74   29 Jul 1999 13:40:06   MA
   avoid internal compiler errors

      Rev 1.73   08 Sep 1998 17:01:46   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.72   17 Apr 1998 16:24:44   OS
   Design verbessert, Linienstaerken berichtigt

      Rev 1.71   28 Nov 1997 20:11:46   MA
   includes

      Rev 1.70   24 Nov 1997 16:47:42   MA
   includes

      Rev 1.69   03 Nov 1997 13:22:44   MA
   precomp entfernt

      Rev 1.68   10 Oct 1997 12:24:12   OS
   Index fuer LineType berichtigt

      Rev 1.67   09 Sep 1997 16:11:18   OS
   fuer VCL LineListBox benutzen #43663#

      Rev 1.66   09 Aug 1997 13:31:40   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.65   05 Feb 1997 10:19:18   OM
   FillItemSet in DeactivatePage rufen

      Rev 1.64   11 Nov 1996 11:05:46   MA
   ResMgr

      Rev 1.63   02 Oct 1996 18:28:58   MA
   Umstellung Enable/Disable

      Rev 1.62   29 Aug 1996 16:09:16   OS
   Korrektur: max. Hoehe des Fussnotenbereichs

      Rev 1.61   28 Aug 1996 14:12:12   OS
   includes

      Rev 1.60   02 Jul 1996 10:09:56   NF
   includes

      Rev 1.59   06 Feb 1996 15:21:14   JP
   Link Umstellung 305

      Rev 1.58   23 Jan 1996 11:27:10   OS
   richtige Reaktion auf Standard-Button im Formatdialog

      Rev 1.57   29 Nov 1995 17:51:26   OS
   im Reset Default-Fussnote erzeugen

      Rev 1.56   28 Nov 1995 21:13:18   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

      Rev 1.55   24 Nov 1995 16:58:46   OM
   PCH->PRECOMPILED

      Rev 1.54   13 Nov 1995 10:51:34   OM
   static entfernt

      Rev 1.53   08 Nov 1995 13:31:44   JP
   Umstellung zur 301: Change -> Set

      Rev 1.52   05 Oct 1995 17:31:02   JP
   eigene TabPages mit GetRanges versehen (fuer Vorlagen editieren)

      Rev 1.51   22 Aug 1995 09:34:22   MA
   svxitems-header entfernt

------------------------------------------------------------------------*/


