/*************************************************************************
 *
 *  $RCSfile: labfmt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif


#include "swtypes.hxx"
#include "cmdid.h"
#include "label.hxx"
#include "labimp.hxx"
#include "labimg.hxx"
#include "labfmt.hxx"
#include "uitool.hxx"

#include "label.hrc"
#include "labfmt.hrc"


// #define ------------------------------------------------------------------

#define ROUND(x) ((USHORT) ((x) + .5))

// --------------------------------------------------------------------------



SwLabPreview::SwLabPreview( const SwLabFmtPage* pParent, const ResId& rResID ) :

    Window((Window*) pParent, rResID),

    aHDistStr (SW_RES(STR_HDIST )),
    aVDistStr (SW_RES(STR_VDIST )),
    aWidthStr (SW_RES(STR_WIDTH )),
    aHeightStr(SW_RES(STR_HEIGHT)),
    aLeftStr  (SW_RES(STR_LEFT  )),
    aUpperStr (SW_RES(STR_UPPER )),
    aColsStr  (SW_RES(STR_COLS  )),
    aRowsStr  (SW_RES(STR_ROWS  )),
    aBlackColor(COL_BLACK),
    aGrayColor(COL_LIGHTGRAY),
    aWhiteColor(COL_WHITE)

{
    SetMapMode(MAP_PIXEL);

    const Size aSz(GetOutputSizePixel());

    lOutWPix   = aSz.Width ();
    lOutHPix   = aSz.Height();


    SetBackground(Wallpaper(aWhiteColor));

    Font aFont = GetFont();
    aFont.SetFillColor(Color(COL_WHITE));
    aFont.SetWeight  (WEIGHT_NORMAL);
    SetFont(aFont);

    lHDistWidth  = GetTextWidth(aHDistStr );
    lVDistWidth  = GetTextWidth(aVDistStr );
    lHeightWidth = GetTextWidth(aHeightStr);
    lLeftWidth   = GetTextWidth(aLeftStr  );
    lUpperWidth  = GetTextWidth(aUpperStr );
    lColsWidth   = GetTextWidth(aColsStr  );

    lXHeight = GetTextHeight();
    lXWidth  = GetTextWidth('X');

    // Skalierungsfaktor
    float fx = (float)(lOutWPix - (2 * (lLeftWidth + 15))) / (float)lOutWPix;

    lOutWPix23 = (long)((float)lOutWPix * fx);
    lOutHPix23 = (long)((float)lOutHPix * fx);
}

// --------------------------------------------------------------------------



SwLabPreview::~SwLabPreview()
{
}

// --------------------------------------------------------------------------



void SwLabPreview::Paint(const Rectangle &rRect)
{
    SetLineColor(aWhiteColor);
    SetFillColor(aGrayColor);
    Font aPaintFont(GetFont());
    aPaintFont.SetTransparent(FALSE);
    SetFont(aPaintFont);

    // Groesse des darzustellenden Bereichs
    long lDispW = ROUND(aItem.lLeft  + aItem.lHDist);
    long lDispH = ROUND(aItem.lUpper + aItem.lVDist);
    if (aItem.nCols == 1)
        lDispW += ROUND(aItem.lLeft );
    else
        lDispW += ROUND(aItem.lHDist / 10);
    if (aItem.nRows == 1)
        lDispH += ROUND(aItem.lUpper);
    else
        lDispH += ROUND(aItem.lVDist / 10);

    // Skalierungsfaktor
    float fx = (float) lOutWPix23 / Max(1L, lDispW),
          fy = (float) lOutHPix23 / Max(1L, lDispH),
          f  = fx < fy ? fx : fy;

    // Nullpunkt
    long lOutlineW = ROUND(f * lDispW);
    long lOutlineH = ROUND(f * lDispH);

    long lX0 = (lOutWPix - lOutlineW) / 2;
    long lY0 = (lOutHPix - lOutlineH) / 2;
    long lX1 = ROUND(lX0 + f *  aItem.lLeft );
    long lY1 = ROUND(lY0 + f *  aItem.lUpper);
    long lX2 = ROUND(lX0 + f * (aItem.lLeft  + aItem.lWidth ));
    long lY2 = ROUND(lY0 + f * (aItem.lUpper + aItem.lHeight));
    long lX3 = ROUND(lX0 + f * (aItem.lLeft  + aItem.lHDist ));
    long lY3 = ROUND(lY0 + f * (aItem.lUpper + aItem.lVDist ));

    // Umriss zeichnen (Flaeche)
    DrawRect(Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH)));

    // Umriss zeichnen (Umrandung)
    SetLineColor(aBlackColor);
    DrawLine(Point(lX0, lY0), Point(lX0 + lOutlineW - 1, lY0)); // Oben
    DrawLine(Point(lX0, lY0), Point(lX0, lY0 + lOutlineH - 1)); // Links
    if (aItem.nCols == 1)
        DrawLine(Point(lX0 + lOutlineW - 1, lY0), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Rechts
    if (aItem.nRows == 1)
        DrawLine(Point(lX0, lY0 + lOutlineH - 1), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Unten

    // Etiketten
    SetClipRegion (Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH)));
    SetFillColor(aWhiteColor);
    for (USHORT nRow = 0; nRow < Min((USHORT) 2, (USHORT) aItem.nRows); nRow++)
        for (USHORT nCol = 0; nCol < Min((USHORT) 2, (USHORT) aItem.nCols); nCol++)
            DrawRect(Rectangle(
              Point(ROUND(lX0 + f * (aItem.lLeft  + nCol * aItem.lHDist)),
                    ROUND(lY0 + f * (aItem.lUpper + nRow * aItem.lVDist))),
              Size (ROUND(f * aItem.lWidth ),
                    ROUND(f * aItem.lHeight))));
    SetClipRegion();

    // Beschritung: Rand links
    if (aItem.lLeft)
    {
        long lX = (lX0 + lX1) / 2;
        DrawArrow(Point(lX0, lY0 - 5), Point(lX1, lY0 - 5), FALSE);
        DrawArrow(Point(lX, lY0 - 10), Point(lX, lY0 - 5), TRUE);
        DrawText(Point(lX1 - lLeftWidth, lY0 - 10 - lXHeight), aLeftStr);
    }

    // Beschriftung: Rand oben
    if (aItem.lUpper)
    {
        DrawArrow(Point(lX0 - 5, lY0), Point(lX0 - 5, lY1), FALSE);
        DrawText(Point(lX0 - 10 - lUpperWidth, ROUND(lY0 + f * aItem.lUpper / 2 - lXHeight / 2)), aUpperStr);
    }

    // Beschriftung: Breite und Hoehe
    {
        long lX = lX2 - lXWidth / 2 - lHeightWidth / 2;
        long lY = lY1 + lXHeight;

        DrawLine(Point(lX1, lY), Point(lX2 - 1, lY));
        DrawLine(Point(lX, lY1), Point(lX, lY2 - 1));

        DrawText(Point(lX1 + lXWidth / 2, lY - lXHeight / 2), aWidthStr);
        DrawText(Point(lX - lHeightWidth / 2, lY2 - lXHeight - lXHeight / 2), aHeightStr);
    }

    // Beschriftung: Horz. Abstand
    if (aItem.nCols > 1)
    {
        long lX = (lX1 + lX3) / 2;
        DrawArrow(Point(lX1, lY0 - 5), Point(lX3, lY0 - 5), FALSE);
        DrawArrow(Point(lX, lY0 - 10), Point(lX, lY0 - 5), TRUE);
        DrawText(Point(lX - lHDistWidth / 2, lY0 - 10 - lXHeight), aHDistStr);
    }

    // Beschriftung: Vertikaler Abstand
    if (aItem.nRows > 1)
    {
        DrawArrow(Point(lX0 - 5, lY1), Point(lX0 - 5, lY3), FALSE);
        DrawText(Point(lX0 - 10 - lVDistWidth, ROUND(lY1 + f * aItem.lVDist / 2 - lXHeight / 2)), aVDistStr);
    }

    // Beschriftung: Spalten
    {
        long lY = lY0 + lOutlineH + 4;
        DrawArrow(Point(lX0, lY), Point(lX0 + lOutlineW - 1, lY), TRUE);
        DrawText(Point((lX0 + lX0 + lOutlineW - 1) / 2 - lColsWidth / 2, lY + 5), aColsStr);
    }

    // Beschriftung: Zeilen
    {
        long lX = lX0 + lOutlineW + 4;
        DrawArrow(Point(lX, lY0), Point(lX, lY0 + lOutlineH - 1), TRUE);
        DrawText(Point(lX + 5, (lY0 + lY0 + lOutlineH - 1 - lXHeight / 2) / 2), aRowsStr);
    }
}

// Pfeil bzw. Intervall zeichnen --------------------------------------------



void SwLabPreview::DrawArrow(const Point &rP1, const Point &rP2, BOOL bArrow)
{
    DrawLine(rP1, rP2);

    if (bArrow)
    {
        Point aArr[3];

        // Pfeil zeichnen
        if (rP1.Y() == rP2.Y())
        {
            // Waagerecht
            aArr[0].X() = rP2.X() - 5;
            aArr[0].Y() = rP2.Y() - 2;
            aArr[1].X() = rP2.X();
            aArr[1].Y() = rP2.Y();
            aArr[2].X() = rP2.X() - 5;
            aArr[2].Y() = rP2.Y() + 2;
        }
        else
        {
            // Senkrecht
            aArr[0].X() = rP2.X() - 2;
            aArr[0].Y() = rP2.Y() - 5;
            aArr[1].X() = rP2.X() + 2;
            aArr[1].Y() = rP2.Y() - 5;
            aArr[2].X() = rP2.X();
            aArr[2].Y() = rP2.Y();
        }

        SetFillColor(aBlackColor);
        DrawPolygon(Polygon(3, aArr));
    }
    else
    {
        // Intervall zeichnen
        if (rP1.Y() == rP2.Y())
        {
            // Waagerecht
            DrawLine(Point(rP1.X(), rP1.Y() - 2), Point(rP1.X(), rP1.Y() + 2));
            DrawLine(Point(rP2.X(), rP2.Y() - 2), Point(rP2.X(), rP2.Y() + 2));
        }
        else
        {
            // Senkrecht
            DrawLine(Point(rP1.X() - 2, rP1.Y()), Point(rP1.X() + 2, rP1.Y()));
            DrawLine(Point(rP2.X() - 2, rP2.Y()), Point(rP2.X() + 2, rP2.Y()));
        }
    }
}

// --------------------------------------------------------------------------

void SwLabPreview::Update(const SwLabItem& rItem)
{
    aItem = rItem;
    Invalidate();
}

// --------------------------------------------------------------------------



SwLabFmtPage::SwLabFmtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_LAB_FMT), rSet),
    aItem        ((const SwLabItem&) rSet.Get(FN_LABEL)),

    aPreview     (this, SW_RES(WIN_PREVIEW)),
    aHDistText   (this, SW_RES(TXT_HDIST  )),
    aHDistField  (this, SW_RES(FLD_HDIST  )),
    aVDistText   (this, SW_RES(TXT_VDIST  )),
    aVDistField  (this, SW_RES(FLD_VDIST  )),
    aWidthText   (this, SW_RES(TXT_WIDTH  )),
    aWidthField  (this, SW_RES(FLD_WIDTH  )),
    aHeightText  (this, SW_RES(TXT_HEIGHT )),
    aHeightField (this, SW_RES(FLD_HEIGHT )),
    aLeftText    (this, SW_RES(TXT_LEFT  )),
    aLeftField   (this, SW_RES(FLD_LEFT  )),
    aUpperText   (this, SW_RES(TXT_UPPER  )),
    aUpperField  (this, SW_RES(FLD_UPPER  )),
    aColsText    (this, SW_RES(TXT_COLUMNS)),
    aColsField   (this, SW_RES(FLD_COLUMNS)),
    aRowsText    (this, SW_RES(TXT_ROWS  )),
    aRowsField   (this, SW_RES(FLD_ROWS  )),

    bModified(FALSE)

{
    FreeResource();
    SetExchangeSupport();

    // Metriken
    FieldUnit aMetric = ::GetDfltMetric(FALSE);
    SetMetric(aHDistField , aMetric);
    SetMetric(aVDistField , aMetric);
    SetMetric(aWidthField , aMetric);
    SetMetric(aHeightField, aMetric);
    SetMetric(aLeftField  , aMetric);
    SetMetric(aUpperField , aMetric);

    // Handler installieren
    Link aLk = LINK(this, SwLabFmtPage, ModifyHdl);
    aHDistField .SetModifyHdl( aLk );
    aVDistField .SetModifyHdl( aLk );
    aWidthField .SetModifyHdl( aLk );
    aHeightField.SetModifyHdl( aLk );
    aLeftField  .SetModifyHdl( aLk );
    aUpperField .SetModifyHdl( aLk );
    aColsField  .SetModifyHdl( aLk );
    aRowsField  .SetModifyHdl( aLk );

    aLk = LINK(this, SwLabFmtPage, LoseFocusHdl);
    aHDistField .SetLoseFocusHdl( aLk );
    aVDistField .SetLoseFocusHdl( aLk );
    aWidthField .SetLoseFocusHdl( aLk );
    aHeightField.SetLoseFocusHdl( aLk );
    aLeftField  .SetLoseFocusHdl( aLk );
    aUpperField .SetLoseFocusHdl( aLk );
    aColsField  .SetLoseFocusHdl( aLk );
    aRowsField  .SetLoseFocusHdl( aLk );

    // Timer einstellen
    aPreviewTimer.SetTimeout(1000);
    aPreviewTimer.SetTimeoutHdl(LINK(this, SwLabFmtPage, PreviewHdl));
}

// --------------------------------------------------------------------------



SwLabFmtPage::~SwLabFmtPage()
{
}

// Modify-Handler der MetricFields. Preview-Timer starten -------------------



IMPL_LINK_INLINE_START( SwLabFmtPage, ModifyHdl, Edit *, EMPTYARG )
{
    bModified = TRUE;
    aPreviewTimer.Start();
    return 0;
}
IMPL_LINK_INLINE_END( SwLabFmtPage, ModifyHdl, Edit *, EMPTYARG )

// Preview invalidaten ------------------------------------------------------



IMPL_LINK_INLINE_START( SwLabFmtPage, PreviewHdl, Timer *, EMPTYARG )
{
    aPreviewTimer.Stop();
    ChangeMinMax();
    FillItem( aItem );
    aPreview.Update( aItem );

    return 0;
}
IMPL_LINK_INLINE_END( SwLabFmtPage, PreviewHdl, Timer *, EMPTYARG )

// LoseFocus-Handler: Bei Aenderung sofort updaten --------------------------



IMPL_LINK_INLINE_START( SwLabFmtPage, LoseFocusHdl, Control *, pControl )
{
    if (((Edit*) pControl)->IsModified())
        PreviewHdl(0);
    return 0;
}
IMPL_LINK_INLINE_END( SwLabFmtPage, LoseFocusHdl, Control *, pControl )

// Minima und Maxima fuer Fields festlegen ----------------------------------


void SwLabFmtPage::ChangeMinMax()
{
    long lMax = 31748; // 56 cm

    // Min und Max

    long lLeft  = GETFLDVAL(aLeftField ),
         lUpper = GETFLDVAL(aUpperField),
         lHDist = GETFLDVAL(aHDistField),
         lVDist = GETFLDVAL(aVDistField);

    long nMinSize = 10; // 0,1cm

    aHDistField .SetMin(nMinSize, FUNIT_CM);
    aVDistField .SetMin(nMinSize, FUNIT_CM);

    aHDistField .SetMax((long) 100 * ((lMax - lLeft ) / Max(1L, (long) aColsField.GetValue())), FUNIT_TWIP);
    aVDistField .SetMax((long) 100 * ((lMax - lUpper) / Max(1L, (long) aRowsField.GetValue())), FUNIT_TWIP);

    aWidthField .SetMin(nMinSize, FUNIT_CM);
    aHeightField.SetMin(nMinSize, FUNIT_CM);

    aWidthField .SetMax((long) 100 * (lHDist), FUNIT_TWIP);
    aHeightField.SetMax((long) 100 * (lVDist), FUNIT_TWIP);

    aLeftField  .SetMax((long) 100 * (lMax - (long) aColsField.GetValue() * GETFLDVAL(aHDistField)), FUNIT_TWIP);
    aUpperField .SetMax((long) 100 * (lMax - (long) aRowsField.GetValue() * GETFLDVAL(aVDistField)), FUNIT_TWIP);

    aColsField  .SetMax((lMax - lLeft ) / Max(1L, lHDist));
    aRowsField  .SetMax((lMax - lUpper) / Max(1L, lVDist));

    // First und Last

    aHDistField .SetFirst(aHDistField .GetMin());
    aVDistField .SetFirst(aVDistField .GetMin());

    aHDistField .SetLast (aHDistField .GetMax());
    aVDistField .SetLast (aVDistField .GetMax());

    aWidthField .SetFirst(aWidthField .GetMin());
    aHeightField.SetFirst(aHeightField.GetMin());

    aWidthField .SetLast (aWidthField .GetMax());
    aHeightField.SetLast (aHeightField.GetMax());

    aLeftField  .SetLast (aLeftField  .GetMax());
    aUpperField .SetLast (aUpperField .GetMax());

    aColsField  .SetLast (aColsField  .GetMax());
    aRowsField  .SetLast (aRowsField  .GetMax());

    aHDistField .Reformat();
    aVDistField .Reformat();
    aWidthField .Reformat();
    aHeightField.Reformat();
    aLeftField  .Reformat();
    aUpperField .Reformat();
    aColsField  .Reformat();
    aRowsField  .Reformat();
}

// --------------------------------------------------------------------------



SfxTabPage* SwLabFmtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwLabFmtPage(pParent, rSet);
}

// ----------------------------------------------------------------------------



void SwLabFmtPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    Reset(aSet);
}

// ----------------------------------------------------------------------------



int SwLabFmtPage::DeactivatePage(SfxItemSet* pSet)
{
    if (pSet)
        FillItemSet(*pSet);

    return TRUE;
}

// --------------------------------------------------------------------------



void SwLabFmtPage::FillItem(SwLabItem& rItem)
{
    if (bModified)
    {
        rItem.aMake = rItem.aType = SW_RESSTR(STR_CUSTOM);

        SwLabRec& rRec = *GetParent()->Recs()[0];
        rItem.lHDist  = rRec.lHDist  = GETFLDVAL(aHDistField );
        rItem.lVDist  = rRec.lVDist  = GETFLDVAL(aVDistField );
        rItem.lWidth  = rRec.lWidth  = GETFLDVAL(aWidthField );
        rItem.lHeight = rRec.lHeight = GETFLDVAL(aHeightField);
        rItem.lLeft   = rRec.lLeft   = GETFLDVAL(aLeftField  );
        rItem.lUpper  = rRec.lUpper  = GETFLDVAL(aUpperField );
        rItem.nCols   = rRec.nCols   = (USHORT) aColsField.GetValue();
        rItem.nRows   = rRec.nRows   = (USHORT) aRowsField.GetValue();
    }
}

// --------------------------------------------------------------------------



BOOL SwLabFmtPage::FillItemSet(SfxItemSet& rSet)
{
    SwLabItem aItem;
    GetParent()->GetLabItem(aItem);
    FillItem(aItem);
    rSet.Put(aItem);

    return TRUE;
}

// --------------------------------------------------------------------------



void SwLabFmtPage::Reset(const SfxItemSet& rSet)
{
    // Fields initialisieren
    GetParent()->GetLabItem(aItem);

    aHDistField .SetMax(100 * aItem.lHDist , FUNIT_TWIP);
    aVDistField .SetMax(100 * aItem.lVDist , FUNIT_TWIP);
    aWidthField .SetMax(100 * aItem.lWidth , FUNIT_TWIP);
    aHeightField.SetMax(100 * aItem.lHeight, FUNIT_TWIP);
    aLeftField  .SetMax(100 * aItem.lLeft  , FUNIT_TWIP);
    aUpperField .SetMax(100 * aItem.lUpper , FUNIT_TWIP);

    SETFLDVAL(aHDistField , aItem.lHDist );
    SETFLDVAL(aVDistField , aItem.lVDist );
    SETFLDVAL(aWidthField , aItem.lWidth );
    SETFLDVAL(aHeightField, aItem.lHeight);
    SETFLDVAL(aLeftField  , aItem.lLeft  );
    SETFLDVAL(aUpperField , aItem.lUpper );

    aColsField.SetMax(aItem.nCols);
    aRowsField.SetMax(aItem.nRows);

    aColsField  .SetValue(aItem.nCols);
    aRowsField  .SetValue(aItem.nRows);

    PreviewHdl(0);
}



/*
$Log: not supported by cvs2svn $
Revision 1.60  2000/09/18 16:05:26  willem.vandorp
OpenOffice header added.

Revision 1.59  2000/04/18 15:31:35  os
UNICODE

Revision 1.58  2000/03/03 15:17:00  os
StarView remainders removed

Revision 1.57  2000/02/11 14:45:37  hr
#70473# changes for unicode ( patched by automated patchtool )

Revision 1.56  1999/07/23 08:00:30  OS
#67818# set font transparent


      Rev 1.55   23 Jul 1999 10:00:30   OS
   #67818# set font transparent

      Rev 1.54   23 Sep 1998 12:42:28   OM
   #56852# Minimale Etikettengroesse 1mm

      Rev 1.53   11 Sep 1998 12:29:46   OM
   #56319# Richtiges Etikettenformat verwenden

      Rev 1.52   08 Sep 1998 16:52:34   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.51   07 Sep 1998 16:59:32   OM
   #55930# Einzelnes Etikett an der korrekten Position drucken

      Rev 1.50   08 Jul 1998 17:16:48   OM
   #52127# os2icci3-Compiler Optimierungsbug umpropelt

      Rev 1.49   14 Mar 1998 14:31:36   OM
   ExchangeSupport repariert/implementiert

      Rev 1.48   14 Mar 1998 14:10:24   OM
   ExchangeSupport repariert/implementiert

      Rev 1.47   24 Nov 1997 11:52:12   MA
   includes

      Rev 1.46   03 Nov 1997 13:17:10   MA
   precomp entfernt

      Rev 1.45   07 Apr 1997 14:46:28   MH
   chg: header

      Rev 1.44   11 Nov 1996 09:44:16   MA
   ResMgr

      Rev 1.43   26 Jul 1996 20:36:38   MA
   includes

      Rev 1.42   07 Mar 1996 14:14:44   HJS
   line 270: &aArr => aArr

      Rev 1.41   06 Mar 1996 10:52:36   MA
   chg: SV309

      Rev 1.40   06 Feb 1996 15:19:04   JP
   Link Umstellung 305

      Rev 1.39   25 Jan 1996 16:58:16   OM
   Dialogfelder richtig initialisieren

      Rev 1.38   24 Jan 1996 18:25:08   OM
   #24534# Redraw-Bug in Etiketten Preview gefixt

      Rev 1.37   22 Jan 1996 12:36:50   OM
   Berechnungsfehler in Labels-Preview gefixt

      Rev 1.36   28 Nov 1995 21:14:56   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

      Rev 1.35   27 Nov 1995 19:36:32   OM
   HasExchangeSupport->303a

      Rev 1.34   24 Nov 1995 16:59:42   OM
   PCH->PRECOMPILED

      Rev 1.33   23 Nov 1995 18:02:08   OM
   In DeactivatePage ItemSet fuellen

      Rev 1.32   08 Nov 1995 13:48:22   OM
   Change->Set

      Rev 1.31   10 Jul 1995 09:50:56   MA
   LabelDlg optimiert und etwas aufgeraeumt.

      Rev 1.30   24 May 1995 18:15:58   ER
   Segmentierung

      Rev 1.29   23 Apr 1995 17:33:32   PK
   bugfix: etik.-fmt. wurde manchm. falsch angezeigt

      Rev 1.28   05 Apr 1995 19:35:18   PK
   fertig zur beta

      Rev 1.27   05 Apr 1995 09:26:58   JP
   Benutzung vom Link-Makro eingeschraenkt

      Rev 1.26   04 Apr 1995 18:38:42   PK
   geht immer weiter

      Rev 1.25   24 Mar 1995 20:30:40   PK
   geht immer weiter

      Rev 1.24   23 Mar 1995 18:33:48   PK
   geht immer weiter ...

      Rev 1.23   17 Mar 1995 17:10:30   PK
   geht immer weiter

      Rev 1.22   15 Mar 1995 13:27:04   PK
   geht immer weiter

      Rev 1.21   06 Mar 1995 00:08:18   PK
   linkbarer envelp-zustand

      Rev 1.20   21 Feb 1995 15:39:32   PK
   erstmal eingecheckt

      Rev 1.19   25 Jan 1995 17:55:56   OS
   Timerinstrumentierung

      Rev 1.18   18 Nov 1994 15:54:42   MA
   min -> Min, max -> Max

      Rev 1.17   25 Oct 1994 17:33:36   ER
   add: PCH

*/

