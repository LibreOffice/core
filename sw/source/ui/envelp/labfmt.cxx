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

#include <tools/poly.hxx>
#include <viewopt.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "label.hxx"
#include "labimp.hxx"
#include "labimg.hxx"
#include "labfmt.hxx"
#include "uitool.hxx"

#include "label.hrc"
#include "labfmt.hrc"
#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

using ::rtl::OUString;

#define ROUND(x) ((sal_uInt16) ((x) + .5))

SwLabPreview::SwLabPreview( const SwLabFmtPage* pParent, const ResId& rResID ) :

    Window((Window*) pParent, rResID),

    aGrayColor(COL_LIGHTGRAY),

    aHDistStr (SW_RES(STR_HDIST )),
    aVDistStr (SW_RES(STR_VDIST )),
    aWidthStr (SW_RES(STR_WIDTH )),
    aHeightStr(SW_RES(STR_HEIGHT)),
    aLeftStr  (SW_RES(STR_LEFT  )),
    aUpperStr (SW_RES(STR_UPPER )),
    aColsStr  (SW_RES(STR_COLS  )),
    aRowsStr  (SW_RES(STR_ROWS  )),
    aPWidthStr (SW_RES(STR_PWIDTH )),
    aPHeightStr(SW_RES(STR_PHEIGHT))
{
    SetMapMode(MAP_PIXEL);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    SetBackground(Wallpaper(rWinColor));

    Font aFont = GetFont();
    aFont.SetTransparent(sal_True);
    aFont.SetWeight  (WEIGHT_NORMAL);
    SetFont(aFont);

    const Size aSz(GetOutputSizePixel());

    lOutWPix   = aSz.Width ();
    lOutHPix   = aSz.Height();


    lHDistWidth  = GetTextWidth(aHDistStr );
    lVDistWidth  = GetTextWidth(aVDistStr );
    lHeightWidth = GetTextWidth(aHeightStr);
    lLeftWidth   = GetTextWidth(aLeftStr  );
    lUpperWidth  = GetTextWidth(aUpperStr );
    lColsWidth   = GetTextWidth(aColsStr  );
    lPWidthWidth  = GetTextWidth(aPWidthStr);
    lPHeightWidth = GetTextWidth(aPHeightStr);
    lXHeight = GetTextHeight();
    lXWidth  = GetTextWidth(rtl::OUString('X'));

    // Scale factor
    float fx = (float)(lOutWPix - (2 * (lLeftWidth + 15))) / (float)lOutWPix;

    lOutWPix23 = (long)((float)lOutWPix * fx);
    lOutHPix23 = (long)((float)lOutHPix * fx);
}

SwLabPreview::~SwLabPreview()
{
}

void SwLabPreview::Paint(const Rectangle &)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    const Color& rFieldTextColor = SwViewOption::GetFontColor();

    Font aFont = GetFont();
    aFont.SetFillColor( rWinColor );
    aFont.SetColor(rFieldTextColor);
    SetFont(aFont);

    SetBackground(Wallpaper(rWinColor));

    SetLineColor(rWinColor);
    SetFillColor(aGrayColor);
    Font aPaintFont(GetFont());
    aPaintFont.SetTransparent(sal_False);
    SetFont(aPaintFont);

    // size of region to be displayed
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

    // Scale factor
    float fx = (float) lOutWPix23 / Max(1L, lDispW),
          fy = (float) lOutHPix23 / Max(1L, lDispH),
          f  = fx < fy ? fx : fy;

    // zero point
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

    // draw outline (area)
    DrawRect(Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH)));

    // draw outline (border)
    SetLineColor(rFieldTextColor);
    DrawLine(Point(lX0, lY0), Point(lX0 + lOutlineW - 1, lY0)); // Up
    DrawLine(Point(lX0, lY0), Point(lX0, lY0 + lOutlineH - 1)); // Left
    if (aItem.nCols == 1)
        DrawLine(Point(lX0 + lOutlineW - 1, lY0), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Right
    if (aItem.nRows == 1)
        DrawLine(Point(lX0, lY0 + lOutlineH - 1), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Down

    // Labels
    SetClipRegion (Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH)));
    SetFillColor( COL_LIGHTGRAYBLUE );
    for (sal_uInt16 nRow = 0; nRow < Min((sal_uInt16) 2, (sal_uInt16) aItem.nRows); nRow++)
        for (sal_uInt16 nCol = 0; nCol < Min((sal_uInt16) 2, (sal_uInt16) aItem.nCols); nCol++)
            DrawRect(Rectangle(
              Point(ROUND(lX0 + f * (aItem.lLeft  + nCol * aItem.lHDist)),
                    ROUND(lY0 + f * (aItem.lUpper + nRow * aItem.lVDist))),
              Size (ROUND(f * aItem.lWidth ),
                    ROUND(f * aItem.lHeight))));
    SetClipRegion();

    // annotation: left border
    if (aItem.lLeft)
    {
        long lX = (lX0 + lX1) / 2;
        DrawArrow(Point(lX0, lY0 - 5), Point(lX1, lY0 - 5), sal_False);
        DrawArrow(Point(lX, lY0 - 10), Point(lX, lY0 - 5), sal_True);
        DrawText(Point(lX1 - lLeftWidth, lY0 - 10 - lXHeight), aLeftStr);
    }

    // annotation: upper border
    if (aItem.lUpper)
    {
        DrawArrow(Point(lX0 - 5, lY0), Point(lX0 - 5, lY1), sal_False);
        DrawText(Point(lX0 - 10 - lUpperWidth, ROUND(lY0 + f * aItem.lUpper / 2 - lXHeight / 2)), aUpperStr);
    }

    // annotation: width and height
    {
        long lX = lX2 - lXWidth / 2 - lHeightWidth / 2;
        long lY = lY1 + lXHeight;

        DrawLine(Point(lX1, lY), Point(lX2 - 1, lY));
        DrawLine(Point(lX, lY1), Point(lX, lY2 - 1));

        DrawText(Point(lX1 + lXWidth / 2, lY - lXHeight / 2), aWidthStr);
        DrawText(Point(lX - lHeightWidth / 2, lY2 - lXHeight - lXHeight / 2), aHeightStr);
    }

    // annotation: horizontal gap
    if (aItem.nCols > 1)
    {
        long lX = (lX1 + lX3) / 2;
        DrawArrow(Point(lX1, lY0 - 5), Point(lX3, lY0 - 5), sal_False);
        DrawArrow(Point(lX, lY0 - 10), Point(lX, lY0 - 5), sal_True);
        DrawText(Point(lX - lHDistWidth / 2, lY0 - 10 - lXHeight), aHDistStr);
    }

    // annotation: vertical gap
    if (aItem.nRows > 1)
    {
        DrawArrow(Point(lX0 - 5, lY1), Point(lX0 - 5, lY3), sal_False);
        DrawText(Point(lX0 - 10 - lVDistWidth, ROUND(lY1 + f * aItem.lVDist / 2 - lXHeight / 2)), aVDistStr);
    }

    // annotation: columns
    {
        long lY = lY0 + lOutlineH + 4;
        DrawArrow(Point(lX0, lY), Point(lX0 + lOutlineW - 1, lY), sal_True);
        DrawText(Point((lX0 + lX0 + lOutlineW - 1) / 2 - lColsWidth / 2, lY + 5), aColsStr);
    }

    // annotation: lines
    {
        long lX = lX0 + lOutlineW + 4;
        DrawArrow(Point(lX, lY0), Point(lX, lY0 + lOutlineH - 1), sal_True);
        DrawText(Point(lX + 5, (lY0 + lY0 + lOutlineH - 1 - lXHeight / 2) / 2), aRowsStr);
    }
}

// Arror or interval character --------------------------------------------

void SwLabPreview::DrawArrow(const Point &rP1, const Point &rP2, sal_Bool bArrow)
{
    DrawLine(rP1, rP2);

    if (bArrow)
    {
        Point aArr[3];

        // Arrow character
        if (rP1.Y() == rP2.Y())
        {
            // Horizontal
            aArr[0].X() = rP2.X() - 5;
            aArr[0].Y() = rP2.Y() - 2;
            aArr[1].X() = rP2.X();
            aArr[1].Y() = rP2.Y();
            aArr[2].X() = rP2.X() - 5;
            aArr[2].Y() = rP2.Y() + 2;
        }
        else
        {
            // Vertical
            aArr[0].X() = rP2.X() - 2;
            aArr[0].Y() = rP2.Y() - 5;
            aArr[1].X() = rP2.X() + 2;
            aArr[1].Y() = rP2.Y() - 5;
            aArr[2].X() = rP2.X();
            aArr[2].Y() = rP2.Y();
        }

        const Color& rFieldTextColor = SwViewOption::GetFontColor();
        SetFillColor(rFieldTextColor);
        DrawPolygon(Polygon(3, aArr));
    }
    else
    {
        // Interval symbol
        if (rP1.Y() == rP2.Y())
        {
            // Horizontal
            DrawLine(Point(rP1.X(), rP1.Y() - 2), Point(rP1.X(), rP1.Y() + 2));
            DrawLine(Point(rP2.X(), rP2.Y() - 2), Point(rP2.X(), rP2.Y() + 2));
        }
        else
        {
            // Vertical
            DrawLine(Point(rP1.X() - 2, rP1.Y()), Point(rP1.X() + 2, rP1.Y()));
            DrawLine(Point(rP2.X() - 2, rP2.Y()), Point(rP2.X() + 2, rP2.Y()));
        }
    }
}

void SwLabPreview::Update(const SwLabItem& rItem)
{
    aItem = rItem;
    Invalidate();
}

SwLabFmtPage::SwLabFmtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_LAB_FMT), rSet),
    aMakeFI      (this, SW_RES(FI_MAKE)),
    aTypeFI      (this, SW_RES(FI_TYPE)),
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
    aPWidthText   (this, SW_RES(TXT_PWIDTH  )),
    aPWidthField  (this, SW_RES(FLD_PWIDTH  )),
    aPHeightText  (this, SW_RES(TXT_PHEIGHT )),
    aPHeightField (this, SW_RES(FLD_PHEIGHT )),
    aSavePB      (this, SW_RES(PB_SAVE  )),
    bModified(sal_False),
    aItem        ((const SwLabItem&) rSet.Get(FN_LABEL))
{
    FreeResource();
    SetExchangeSupport();

    // Metrics
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(aHDistField , aMetric);
    SetMetric(aVDistField , aMetric);
    SetMetric(aWidthField , aMetric);
    SetMetric(aHeightField, aMetric);
    SetMetric(aLeftField  , aMetric);
    SetMetric(aUpperField , aMetric);
    SetMetric(aPWidthField , aMetric);
    SetMetric(aPHeightField, aMetric);

    // Install handlers
    Link aLk = LINK(this, SwLabFmtPage, ModifyHdl);
    aHDistField .SetModifyHdl( aLk );
    aVDistField .SetModifyHdl( aLk );
    aWidthField .SetModifyHdl( aLk );
    aHeightField.SetModifyHdl( aLk );
    aLeftField  .SetModifyHdl( aLk );
    aUpperField .SetModifyHdl( aLk );
    aColsField  .SetModifyHdl( aLk );
    aRowsField  .SetModifyHdl( aLk );
    aPWidthField .SetModifyHdl( aLk );
    aPHeightField.SetModifyHdl( aLk );

    aLk = LINK(this, SwLabFmtPage, LoseFocusHdl);
    aHDistField .SetLoseFocusHdl( aLk );
    aVDistField .SetLoseFocusHdl( aLk );
    aWidthField .SetLoseFocusHdl( aLk );
    aHeightField.SetLoseFocusHdl( aLk );
    aLeftField  .SetLoseFocusHdl( aLk );
    aUpperField .SetLoseFocusHdl( aLk );
    aColsField  .SetLoseFocusHdl( aLk );
    aRowsField  .SetLoseFocusHdl( aLk );
    aPWidthField .SetLoseFocusHdl( aLk );
    aPHeightField.SetLoseFocusHdl( aLk );

    aSavePB.SetClickHdl( LINK (this, SwLabFmtPage, SaveHdl));
    // Set timer
    aPreviewTimer.SetTimeout(1000);
    aPreviewTimer.SetTimeoutHdl(LINK(this, SwLabFmtPage, PreviewHdl));
}

SwLabFmtPage::~SwLabFmtPage()
{
}

// Modify-handler of MetricFields. start preview timer
IMPL_LINK_NOARG_INLINE_START(SwLabFmtPage, ModifyHdl)
{
    bModified = sal_True;
    aPreviewTimer.Start();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwLabFmtPage, ModifyHdl)

// Invalidate preview
IMPL_LINK_NOARG_INLINE_START(SwLabFmtPage, PreviewHdl)
{
    aPreviewTimer.Stop();
    ChangeMinMax();
    FillItem( aItem );
    aPreview.Update( aItem );

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwLabFmtPage, PreviewHdl)

// LoseFocus-Handler: Update on change --------------------------
IMPL_LINK_INLINE_START( SwLabFmtPage, LoseFocusHdl, Control *, pControl )
{
    if (((Edit*) pControl)->IsModified())
        PreviewHdl(0);
    return 0;
}
IMPL_LINK_INLINE_END( SwLabFmtPage, LoseFocusHdl, Control *, pControl )

void SwLabFmtPage::ChangeMinMax()
{
    long lMax = 31748; // 56 cm
    long nMinSize = 10; // 0,1cm

    // Min and Max

    int nCols   = aColsField.GetValue(),
        nRows   = aRowsField.GetValue();
    long lLeft   = static_cast< long >(GETFLDVAL(aLeftField )),
         lUpper  = static_cast< long >(GETFLDVAL(aUpperField)),
         lHDist  = static_cast< long >(GETFLDVAL(aHDistField)),
         lVDist  = static_cast< long >(GETFLDVAL(aVDistField)),
         lWidth  = static_cast< long >(GETFLDVAL(aWidthField)),
         lHeight = static_cast< long >(GETFLDVAL(aHeightField)),
         lMinPWidth  = lLeft  + (nCols - 1) * lHDist + lWidth,
         lMinPHeight = lUpper + (nRows - 1) * lVDist + lHeight;

    aHDistField .SetMin(nMinSize, FUNIT_CM);
    aVDistField .SetMin(nMinSize, FUNIT_CM);

    aHDistField .SetMax((long) 100 * ((lMax - lLeft ) / Max(1L, (long) nCols)), FUNIT_TWIP);
    aVDistField .SetMax((long) 100 * ((lMax - lUpper) / Max(1L, (long) nRows)), FUNIT_TWIP);

    aWidthField .SetMin(nMinSize, FUNIT_CM);
    aHeightField.SetMin(nMinSize, FUNIT_CM);

    aWidthField .SetMax((long) 100 * (lHDist), FUNIT_TWIP);
    aHeightField.SetMax((long) 100 * (lVDist), FUNIT_TWIP);

    aLeftField  .SetMax((long) 100 * (lMax - nCols * lHDist), FUNIT_TWIP);
    aUpperField .SetMax((long) 100 * (lMax - nRows * lVDist), FUNIT_TWIP);

    aColsField  .SetMin( 1 );
    aRowsField  .SetMin( 1 );

    aColsField  .SetMax((lMax - lLeft ) / Max(1L, lHDist));
    aRowsField  .SetMax((lMax - lUpper) / Max(1L, lVDist));
    aPWidthField .SetMin( (long) 100 * lMinPWidth,  FUNIT_TWIP );
    aPHeightField.SetMin( (long) 100 * lMinPHeight, FUNIT_TWIP );

    aPWidthField .SetMax( (long) 100 * lMax, FUNIT_TWIP);
    aPHeightField.SetMax( (long) 100 * lMax, FUNIT_TWIP);
    // First and Last

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
    aPWidthField .SetFirst(aPWidthField .GetMin());
    aPHeightField.SetFirst(aPHeightField.GetMin());

    aPWidthField .SetLast (aPWidthField .GetMax());
    aPHeightField.SetLast (aPHeightField.GetMax());
    aHDistField .Reformat();
    aVDistField .Reformat();
    aWidthField .Reformat();
    aHeightField.Reformat();
    aLeftField  .Reformat();
    aUpperField .Reformat();
    aColsField  .Reformat();
    aRowsField  .Reformat();
    aPWidthField .Reformat();
    aPHeightField.Reformat();
}

SfxTabPage* SwLabFmtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwLabFmtPage(pParent, rSet);
}

void SwLabFmtPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    Reset(aSet);
}

int SwLabFmtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(*_pSet);

    return sal_True;
}

void SwLabFmtPage::FillItem(SwLabItem& rItem)
{
    if (bModified)
    {
        rItem.aMake = rItem.aType = SW_RESSTR(STR_CUSTOM);

        SwLabRec& rRec = *GetParent()->Recs()[0];
        rItem.lHDist  = rRec.lHDist  = static_cast< long >(GETFLDVAL(aHDistField ));
        rItem.lVDist  = rRec.lVDist  = static_cast< long >(GETFLDVAL(aVDistField ));
        rItem.lWidth  = rRec.lWidth  = static_cast< long >(GETFLDVAL(aWidthField ));
        rItem.lHeight = rRec.lHeight = static_cast< long >(GETFLDVAL(aHeightField));
        rItem.lLeft   = rRec.lLeft   = static_cast< long >(GETFLDVAL(aLeftField  ));
        rItem.lUpper  = rRec.lUpper  = static_cast< long >(GETFLDVAL(aUpperField ));
        rItem.nCols   = rRec.nCols   = (sal_uInt16) aColsField.GetValue();
        rItem.nRows   = rRec.nRows   = (sal_uInt16) aRowsField.GetValue();
        rItem.lPWidth  = rRec.lPWidth  = static_cast< long >(GETFLDVAL(aPWidthField ));
        rItem.lPHeight = rRec.lPHeight = static_cast< long >(GETFLDVAL(aPHeightField));
    }
}

sal_Bool SwLabFmtPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(aItem);
    rSet.Put(aItem);

    return sal_True;
}

void SwLabFmtPage::Reset(const SfxItemSet& )
{
    // Initialise fields
    GetParent()->GetLabItem(aItem);

    aHDistField .SetMax(100 * aItem.lHDist , FUNIT_TWIP);
    aVDistField .SetMax(100 * aItem.lVDist , FUNIT_TWIP);
    aWidthField .SetMax(100 * aItem.lWidth , FUNIT_TWIP);
    aHeightField.SetMax(100 * aItem.lHeight, FUNIT_TWIP);
    aLeftField  .SetMax(100 * aItem.lLeft  , FUNIT_TWIP);
    aUpperField .SetMax(100 * aItem.lUpper , FUNIT_TWIP);
    aPWidthField .SetMax(100 * aItem.lPWidth , FUNIT_TWIP);
    aPHeightField.SetMax(100 * aItem.lPHeight, FUNIT_TWIP);

    SETFLDVAL(aHDistField , aItem.lHDist );
    SETFLDVAL(aVDistField , aItem.lVDist );
    SETFLDVAL(aWidthField , aItem.lWidth );
    SETFLDVAL(aHeightField, aItem.lHeight);
    SETFLDVAL(aLeftField  , aItem.lLeft  );
    SETFLDVAL(aUpperField , aItem.lUpper );
    SETFLDVAL(aPWidthField , aItem.lPWidth );
    SETFLDVAL(aPHeightField, aItem.lPHeight);

    aColsField.SetMax(aItem.nCols);
    aRowsField.SetMax(aItem.nRows);

    aColsField  .SetValue(aItem.nCols);
    aRowsField  .SetValue(aItem.nRows);
    aMakeFI.SetText(aItem.aMake);
    aTypeFI.SetText(aItem.aType);
    PreviewHdl(0);
}

IMPL_LINK_NOARG(SwLabFmtPage, SaveHdl)
{
    SwLabRec aRec;
    aRec.lHDist  = static_cast< long >(GETFLDVAL(aHDistField ));
    aRec.lVDist  = static_cast< long >(GETFLDVAL(aVDistField ));
    aRec.lWidth  = static_cast< long >(GETFLDVAL(aWidthField ));
    aRec.lHeight = static_cast< long >(GETFLDVAL(aHeightField));
    aRec.lLeft   = static_cast< long >(GETFLDVAL(aLeftField  ));
    aRec.lUpper  = static_cast< long >(GETFLDVAL(aUpperField ));
    aRec.nCols   = (sal_uInt16) aColsField.GetValue();
    aRec.nRows   = (sal_uInt16) aRowsField.GetValue();
    aRec.lPWidth  = static_cast< long >(GETFLDVAL(aPWidthField ));
    aRec.lPHeight = static_cast< long >(GETFLDVAL(aPHeightField));
    aRec.bCont = aItem.bCont;
    SwSaveLabelDlg* pSaveDlg = new SwSaveLabelDlg(this, aRec);
    pSaveDlg->SetLabel(aItem.aLstMake, aItem.aLstType);
    pSaveDlg->Execute();
    if(pSaveDlg->GetLabel(aItem))
    {
        bModified = sal_False;
        const Sequence<OUString>& rMan = GetParent()->GetLabelsConfig().GetManufacturers();
        std::vector<rtl::OUString>& rMakes(GetParent()->Makes());
        if(rMakes.size() < (sal_uInt16)rMan.getLength())
        {
            rMakes.clear();
            const OUString* pMan = rMan.getConstArray();
            for(sal_Int32 nMan = 0; nMan < rMan.getLength(); nMan++)
                rMakes.push_back(pMan[nMan]);
        }
        aMakeFI.SetText(aItem.aMake);
        aTypeFI.SetText(aItem.aType);
    }
    delete pSaveDlg;
    return 0;
}

SwSaveLabelDlg::SwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec) :
    ModalDialog(pParent, SW_RES(DLG_SAVE_LABEL)),
    aOptionsFL(this,SW_RES(FL_OPTIONS  )),
    aMakeFT(this,   SW_RES(FT_MAKE     )),
    aMakeCB(this,   SW_RES(CB_MAKE     )),
    aTypeFT(this,   SW_RES(FT_TYPE     )),
    aTypeED(this,   SW_RES(ED_TYPE     )),

    aOKPB(this,     SW_RES(PB_OK     )),
    aCancelPB(this, SW_RES(PB_CANCEL )),
    aHelpPB(this,   SW_RES(PB_HELP      )),

    aQueryMB(this,  SW_RES(MB_QUERY )),

    bSuccess(sal_False),
    pLabPage(pParent),
    rLabRec(rRec)
{
    FreeResource();

    aOKPB.SetClickHdl(LINK(this, SwSaveLabelDlg, OkHdl));
    Link aLk(LINK(this, SwSaveLabelDlg, ModifyHdl));
    aMakeCB.SetModifyHdl(aLk);
    aTypeED.SetModifyHdl(aLk);

    SwLabelConfig& rCfg = pLabPage->GetParent()->GetLabelsConfig();
    const Sequence<OUString>& rMan = rCfg.GetManufacturers();
    const OUString* pMan = rMan.getConstArray();
    for(sal_Int32 i = 0; i < rMan.getLength(); i++)
        aMakeCB.InsertEntry(pMan[i]);
}

IMPL_LINK_NOARG(SwSaveLabelDlg, OkHdl)
{
    SwLabelConfig& rCfg = pLabPage->GetParent()->GetLabelsConfig();
    String sMake(aMakeCB.GetText());
    String sType(aTypeED.GetText());
    if(rCfg.HasLabel(sMake, sType))
    {
        String sTmp(aQueryMB.GetMessText());
        String sQuery(sTmp);
        sQuery.SearchAndReplace(rtl::OUString("%1"), sMake);
        sQuery.SearchAndReplace(rtl::OUString("%2"), sType);
        aQueryMB.SetMessText(sQuery);

        short eRet = aQueryMB.Execute();
        aQueryMB.SetMessText(sTmp);
        if(RET_YES != eRet)
            return 0;
    }
    rLabRec.aType = sType;
    rCfg.SaveLabel(sMake, sType, rLabRec);
    bSuccess = sal_True;
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(SwSaveLabelDlg, ModifyHdl)
{
    aOKPB.Enable(aMakeCB.GetText().Len() && aTypeED.GetText().Len());
    return 0;
}

sal_Bool SwSaveLabelDlg::GetLabel(SwLabItem& rItem)
{
    if(bSuccess)
    {
        rItem.aMake = aMakeCB.GetText();
        rItem.aType = aTypeED.GetText();
        rItem.lHDist  = rLabRec.lHDist;
        rItem.lVDist  = rLabRec.lVDist;
        rItem.lWidth  = rLabRec.lWidth;
        rItem.lHeight = rLabRec.lHeight;
        rItem.lLeft   = rLabRec.lLeft;
        rItem.lUpper  = rLabRec.lUpper;
        rItem.nCols   = rLabRec.nCols;
        rItem.nRows   = rLabRec.nRows;
        rItem.lPWidth  = rLabRec.lPWidth;
        rItem.lPHeight = rLabRec.lPHeight;
    }
    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
