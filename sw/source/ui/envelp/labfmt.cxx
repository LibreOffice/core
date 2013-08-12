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

#include <tools/poly.hxx>
#include <vcl/layout.hxx>
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


#define ROUND(x) ((sal_uInt16) ((x) + .5))

SwLabPreview::SwLabPreview(Window* pParent)
    : Window(pParent, 0)
    , aGrayColor(COL_LIGHTGRAY)
    , aHDistStr(SW_RESSTR(STR_HDIST))
    , aVDistStr(SW_RESSTR(STR_VDIST))
    , aWidthStr(SW_RESSTR(STR_WIDTH))
    , aHeightStr(SW_RESSTR(STR_HEIGHT))
    , aLeftStr(SW_RESSTR(STR_LEFT))
    , aUpperStr(SW_RESSTR(STR_UPPER))
    , aColsStr(SW_RESSTR(STR_COLS))
    , aRowsStr(SW_RESSTR(STR_ROWS))
    , aPWidthStr(SW_RESSTR(STR_PWIDTH))
    , aPHeightStr(SW_RESSTR(STR_PHEIGHT))
{
    SetMapMode(MAP_PIXEL);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    SetBackground(Wallpaper(rWinColor));

    Font aFont = GetFont();
    aFont.SetTransparent(sal_True);
    aFont.SetWeight  (WEIGHT_NORMAL);
    SetFont(aFont);

    lHDistWidth  = GetTextWidth(aHDistStr );
    lVDistWidth  = GetTextWidth(aVDistStr );
    lHeightWidth = GetTextWidth(aHeightStr);
    lLeftWidth   = GetTextWidth(aLeftStr  );
    lUpperWidth  = GetTextWidth(aUpperStr );
    lColsWidth   = GetTextWidth(aColsStr  );
    lPWidthWidth  = GetTextWidth(aPWidthStr);
    lPHeightWidth = GetTextWidth(aPHeightStr);
    lXHeight = GetTextHeight();
    lXWidth  = GetTextWidth(OUString('X'));
}

Size SwLabPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(146 , 161), MapMode(MAP_APPFONT));
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwLabPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new SwLabPreview(pParent);
}

void SwLabPreview::Paint(const Rectangle &)
{
    const Size aSz(GetOutputSizePixel());

    long lOutWPix   = aSz.Width ();
    long lOutHPix   = aSz.Height();

    // Scale factor
    float fxpix = (float)(lOutWPix - (2 * (lLeftWidth + 15))) / (float)lOutWPix;

    long lOutWPix23 = (long)((float)lOutWPix * fxpix);
    long lOutHPix23 = (long)((float)lOutHPix * fxpix);

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
    float fx = (float) lOutWPix23 / std::max(1L, lDispW),
          fy = (float) lOutHPix23 / std::max(1L, lDispH),
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
    SetClipRegion(Region(Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH))));
    SetFillColor( COL_LIGHTGRAYBLUE );
    for (sal_uInt16 nRow = 0; nRow < std::min((sal_uInt16) 2, (sal_uInt16) aItem.nRows); nRow++)
        for (sal_uInt16 nCol = 0; nCol < std::min((sal_uInt16) 2, (sal_uInt16) aItem.nCols); nCol++)
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
        DrawArrow(Point(lX0, lY0 - 5), Point(lX1, lY0 - 5), false);
        DrawArrow(Point(lX, lY0 - 10), Point(lX, lY0 - 5), true);
        DrawText(Point(lX1 - lLeftWidth, lY0 - 10 - lXHeight), aLeftStr);
    }

    // annotation: upper border
    if (aItem.lUpper)
    {
        DrawArrow(Point(lX0 - 5, lY0), Point(lX0 - 5, lY1), false);
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
        DrawArrow(Point(lX1, lY0 - 5), Point(lX3, lY0 - 5), false);
        DrawArrow(Point(lX, lY0 - 10), Point(lX, lY0 - 5), true);
        DrawText(Point(lX - lHDistWidth / 2, lY0 - 10 - lXHeight), aHDistStr);
    }

    // annotation: vertical gap
    if (aItem.nRows > 1)
    {
        DrawArrow(Point(lX0 - 5, lY1), Point(lX0 - 5, lY3), false);
        DrawText(Point(lX0 - 10 - lVDistWidth, ROUND(lY1 + f * aItem.lVDist / 2 - lXHeight / 2)), aVDistStr);
    }

    // annotation: columns
    {
        long lY = lY0 + lOutlineH + 4;
        DrawArrow(Point(lX0, lY), Point(lX0 + lOutlineW - 1, lY), true);
        DrawText(Point((lX0 + lX0 + lOutlineW - 1) / 2 - lColsWidth / 2, lY + 5), aColsStr);
    }

    // annotation: lines
    {
        long lX = lX0 + lOutlineW + 4;
        DrawArrow(Point(lX, lY0), Point(lX, lY0 + lOutlineH - 1), true);
        DrawText(Point(lX + 5, (lY0 + lY0 + lOutlineH - 1 - lXHeight / 2) / 2), aRowsStr);
    }
}

// Arror or interval character --------------------------------------------

void SwLabPreview::DrawArrow(const Point &rP1, const Point &rP2, bool bArrow)
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

void SwLabPreview::UpdateItem(const SwLabItem& rItem)
{
    aItem = rItem;
    Invalidate();
}

SwLabFmtPage::SwLabFmtPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "LabelFormatPage",
        "modules/swriter/ui/labelformatpage.ui", rSet)
    , bModified(false)
    , aItem((const SwLabItem&) rSet.Get(FN_LABEL))
{
    SetExchangeSupport();

    get(m_pMakeFI, "make");
    get(m_pTypeFI, "type");
    get(m_pPreview, "preview");
    get(m_pHDistField, "hori");
    get(m_pVDistField, "vert");
    get(m_pWidthField, "width");
    get(m_pHeightField, "height");
    get(m_pLeftField, "left");
    get(m_pUpperField, "top");
    get(m_pColsField, "cols");
    get(m_pRowsField, "rows");
    get(m_pPWidthField, "pagewidth");
    get(m_pPHeightField, "pageheight");
    get(m_pSavePB, "save");

    // Metrics
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(*m_pHDistField, aMetric);
    SetMetric(*m_pVDistField , aMetric);
    SetMetric(*m_pWidthField , aMetric);
    SetMetric(*m_pHeightField, aMetric);
    SetMetric(*m_pLeftField  , aMetric);
    SetMetric(*m_pUpperField , aMetric);
    SetMetric(*m_pPWidthField , aMetric);
    SetMetric(*m_pPHeightField, aMetric);

    // Install handlers
    Link aLk = LINK(this, SwLabFmtPage, ModifyHdl);
    m_pHDistField->SetModifyHdl( aLk );
    m_pVDistField->SetModifyHdl( aLk );
    m_pWidthField->SetModifyHdl( aLk );
    m_pHeightField->SetModifyHdl( aLk );
    m_pLeftField->SetModifyHdl( aLk );
    m_pUpperField->SetModifyHdl( aLk );
    m_pColsField->SetModifyHdl( aLk );
    m_pRowsField->SetModifyHdl( aLk );
    m_pPWidthField->SetModifyHdl( aLk );
    m_pPHeightField->SetModifyHdl( aLk );

    aLk = LINK(this, SwLabFmtPage, LoseFocusHdl);
    m_pHDistField->SetLoseFocusHdl( aLk );
    m_pVDistField->SetLoseFocusHdl( aLk );
    m_pWidthField->SetLoseFocusHdl( aLk );
    m_pHeightField->SetLoseFocusHdl( aLk );
    m_pLeftField->SetLoseFocusHdl( aLk );
    m_pUpperField->SetLoseFocusHdl( aLk );
    m_pColsField->SetLoseFocusHdl( aLk );
    m_pRowsField->SetLoseFocusHdl( aLk );
    m_pPWidthField->SetLoseFocusHdl( aLk );
    m_pPHeightField->SetLoseFocusHdl( aLk );

    m_pSavePB->SetClickHdl( LINK (this, SwLabFmtPage, SaveHdl));
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
    bModified = true;
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
    m_pPreview->UpdateItem( aItem );

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

    int nCols   = m_pColsField->GetValue(),
        nRows   = m_pRowsField->GetValue();
    long lLeft   = static_cast< long >(GETFLDVAL(*m_pLeftField )),
         lUpper  = static_cast< long >(GETFLDVAL(*m_pUpperField)),
         lHDist  = static_cast< long >(GETFLDVAL(*m_pHDistField)),
         lVDist  = static_cast< long >(GETFLDVAL(*m_pVDistField)),
         lWidth  = static_cast< long >(GETFLDVAL(*m_pWidthField)),
         lHeight = static_cast< long >(GETFLDVAL(*m_pHeightField)),
         lMinPWidth  = lLeft  + (nCols - 1) * lHDist + lWidth,
         lMinPHeight = lUpper + (nRows - 1) * lVDist + lHeight;

    m_pHDistField->SetMin(nMinSize, FUNIT_CM);
    m_pVDistField->SetMin(nMinSize, FUNIT_CM);

    m_pHDistField->SetMax((long) 100 * ((lMax - lLeft ) / std::max(1L, (long) nCols)), FUNIT_TWIP);
    m_pVDistField->SetMax((long) 100 * ((lMax - lUpper) / std::max(1L, (long) nRows)), FUNIT_TWIP);

    m_pWidthField->SetMin(nMinSize, FUNIT_CM);
    m_pHeightField->SetMin(nMinSize, FUNIT_CM);

    m_pWidthField->SetMax((long) 100 * (lHDist), FUNIT_TWIP);
    m_pHeightField->SetMax((long) 100 * (lVDist), FUNIT_TWIP);

    m_pLeftField->SetMax((long) 100 * (lMax - nCols * lHDist), FUNIT_TWIP);
    m_pUpperField->SetMax((long) 100 * (lMax - nRows * lVDist), FUNIT_TWIP);

    m_pColsField->SetMin( 1 );
    m_pRowsField->SetMin( 1 );

    m_pColsField->SetMax((lMax - lLeft ) / std::max(1L, lHDist));
    m_pRowsField->SetMax((lMax - lUpper) / std::max(1L, lVDist));
    m_pPWidthField->SetMin( (long) 100 * lMinPWidth,  FUNIT_TWIP );
    m_pPHeightField->SetMin( (long) 100 * lMinPHeight, FUNIT_TWIP );

    m_pPWidthField->SetMax( (long) 100 * lMax, FUNIT_TWIP);
    m_pPHeightField->SetMax( (long) 100 * lMax, FUNIT_TWIP);
    // First and Last

    m_pHDistField->SetFirst(m_pHDistField->GetMin());
    m_pVDistField->SetFirst(m_pVDistField->GetMin());

    m_pHDistField->SetLast (m_pHDistField->GetMax());
    m_pVDistField->SetLast (m_pVDistField->GetMax());

    m_pWidthField->SetFirst(m_pWidthField->GetMin());
    m_pHeightField->SetFirst(m_pHeightField->GetMin());

    m_pWidthField->SetLast (m_pWidthField->GetMax());
    m_pHeightField->SetLast (m_pHeightField->GetMax());

    m_pLeftField->SetLast (m_pLeftField->GetMax());
    m_pUpperField->SetLast (m_pUpperField->GetMax());

    m_pColsField->SetLast (m_pColsField->GetMax());
    m_pRowsField->SetLast (m_pRowsField->GetMax());
    m_pPWidthField->SetFirst(m_pPWidthField->GetMin());
    m_pPHeightField->SetFirst(m_pPHeightField->GetMin());

    m_pPWidthField->SetLast (m_pPWidthField->GetMax());
    m_pPHeightField->SetLast (m_pPHeightField->GetMax());
    m_pHDistField->Reformat();
    m_pVDistField->Reformat();
    m_pWidthField->Reformat();
    m_pHeightField->Reformat();
    m_pLeftField->Reformat();
    m_pUpperField->Reformat();
    m_pColsField->Reformat();
    m_pRowsField->Reformat();
    m_pPWidthField->Reformat();
    m_pPHeightField->Reformat();
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

        SwLabRec& rRec = *GetParentSwLabDlg()->Recs()[0];
        rItem.lHDist  = rRec.lHDist  = static_cast< long >(GETFLDVAL(*m_pHDistField ));
        rItem.lVDist  = rRec.lVDist  = static_cast< long >(GETFLDVAL(*m_pVDistField ));
        rItem.lWidth  = rRec.lWidth  = static_cast< long >(GETFLDVAL(*m_pWidthField ));
        rItem.lHeight = rRec.lHeight = static_cast< long >(GETFLDVAL(*m_pHeightField));
        rItem.lLeft   = rRec.lLeft   = static_cast< long >(GETFLDVAL(*m_pLeftField  ));
        rItem.lUpper  = rRec.lUpper  = static_cast< long >(GETFLDVAL(*m_pUpperField ));
        rItem.nCols   = rRec.nCols   = (sal_uInt16) m_pColsField->GetValue();
        rItem.nRows   = rRec.nRows   = (sal_uInt16) m_pRowsField->GetValue();
        rItem.lPWidth  = rRec.lPWidth  = static_cast< long >(GETFLDVAL(*m_pPWidthField ));
        rItem.lPHeight = rRec.lPHeight = static_cast< long >(GETFLDVAL(*m_pPHeightField));
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
    GetParentSwLabDlg()->GetLabItem(aItem);

    m_pHDistField->SetMax(100 * aItem.lHDist , FUNIT_TWIP);
    m_pVDistField->SetMax(100 * aItem.lVDist , FUNIT_TWIP);
    m_pWidthField->SetMax(100 * aItem.lWidth , FUNIT_TWIP);
    m_pHeightField->SetMax(100 * aItem.lHeight, FUNIT_TWIP);
    m_pLeftField->SetMax(100 * aItem.lLeft  , FUNIT_TWIP);
    m_pUpperField->SetMax(100 * aItem.lUpper , FUNIT_TWIP);
    m_pPWidthField->SetMax(100 * aItem.lPWidth , FUNIT_TWIP);
    m_pPHeightField->SetMax(100 * aItem.lPHeight, FUNIT_TWIP);

    SETFLDVAL(*m_pHDistField, aItem.lHDist );
    SETFLDVAL(*m_pVDistField , aItem.lVDist );
    SETFLDVAL(*m_pWidthField , aItem.lWidth );
    SETFLDVAL(*m_pHeightField, aItem.lHeight);
    SETFLDVAL(*m_pLeftField  , aItem.lLeft  );
    SETFLDVAL(*m_pUpperField , aItem.lUpper );
    SETFLDVAL(*m_pPWidthField , aItem.lPWidth );
    SETFLDVAL(*m_pPHeightField, aItem.lPHeight);

    m_pColsField->SetMax(aItem.nCols);
    m_pRowsField->SetMax(aItem.nRows);

    m_pColsField->SetValue(aItem.nCols);
    m_pRowsField->SetValue(aItem.nRows);
    m_pMakeFI->SetText(aItem.aMake);
    m_pTypeFI->SetText(aItem.aType);
    PreviewHdl(0);
}

IMPL_LINK_NOARG(SwLabFmtPage, SaveHdl)
{
    SwLabRec aRec;
    aRec.lHDist  = static_cast< long >(GETFLDVAL(*m_pHDistField));
    aRec.lVDist  = static_cast< long >(GETFLDVAL(*m_pVDistField ));
    aRec.lWidth  = static_cast< long >(GETFLDVAL(*m_pWidthField ));
    aRec.lHeight = static_cast< long >(GETFLDVAL(*m_pHeightField));
    aRec.lLeft   = static_cast< long >(GETFLDVAL(*m_pLeftField  ));
    aRec.lUpper  = static_cast< long >(GETFLDVAL(*m_pUpperField ));
    aRec.nCols   = (sal_uInt16) m_pColsField->GetValue();
    aRec.nRows   = (sal_uInt16) m_pRowsField->GetValue();
    aRec.lPWidth  = static_cast< long >(GETFLDVAL(*m_pPWidthField ));
    aRec.lPHeight = static_cast< long >(GETFLDVAL(*m_pPHeightField));
    aRec.bCont = aItem.bCont;
    SwSaveLabelDlg* pSaveDlg = new SwSaveLabelDlg(this, aRec);
    pSaveDlg->SetLabel(aItem.aLstMake, aItem.aLstType);
    pSaveDlg->Execute();
    if(pSaveDlg->GetLabel(aItem))
    {
        bModified = false;
        const std::vector<OUString>& rMan = GetParentSwLabDlg()->GetLabelsConfig().GetManufacturers();
        std::vector<OUString>& rMakes(GetParentSwLabDlg()->Makes());
        if(rMakes.size() < rMan.size())
        {
            rMakes = rMan;
        }
        m_pMakeFI->SetText(aItem.aMake);
        m_pTypeFI->SetText(aItem.aType);
    }
    delete pSaveDlg;
    return 0;
}

SwSaveLabelDlg::SwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec)
    : ModalDialog(pParent, "SaveLabelDialog",
        "modules/swriter/ui/savelabeldialog.ui")
    , bSuccess(false)
    , pLabPage(pParent)
    , rLabRec(rRec)
{
    get(m_pMakeCB, "brand");
    get(m_pTypeED, "type");
    get(m_pOKPB, "ok");

    m_pOKPB->SetClickHdl(LINK(this, SwSaveLabelDlg, OkHdl));
    Link aLk(LINK(this, SwSaveLabelDlg, ModifyHdl));
    m_pMakeCB->SetModifyHdl(aLk);
    m_pTypeED->SetModifyHdl(aLk);

    SwLabelConfig& rCfg = pLabPage->GetParentSwLabDlg()->GetLabelsConfig();
    const std::vector<OUString>& rMan = rCfg.GetManufacturers();
    for (sal_uInt16 i = 0; i < rMan.size(); i++)
    {
        m_pMakeCB->InsertEntry(rMan[i]);
    }
}

IMPL_LINK_NOARG(SwSaveLabelDlg, OkHdl)
{
    SwLabelConfig& rCfg = pLabPage->GetParentSwLabDlg()->GetLabelsConfig();
    String sMake(m_pMakeCB->GetText());
    String sType(m_pTypeED->GetText());
    if(rCfg.HasLabel(sMake, sType))
    {
        if ( rCfg.IsPredefinedLabel(sMake, sType) )
        {
            SAL_WARN( "sw.envelp", "label is predefined and cannot be overwritten" );
            MessageDialog(this, "CannotSaveLabelDialog", "modules/swriter/ui/cannotsavelabeldialog.ui").Execute();
            return 0;
        }

        MessageDialog aQuery(this, "QuerySaveLabelDialog",
            "modules/swriter/ui/querysavelabeldialog.ui");

        aQuery.set_primary_text(aQuery.get_primary_text().
            replaceAll("%1", sMake).replaceAll("%2", sType));
        aQuery.set_secondary_text(aQuery.get_secondary_text().
            replaceAll("%1", sMake).replaceAll("%2", sType));
        sal_uInt16 eRet = aQuery.Execute();

        if (RET_YES != eRet)
            return 0;
    }
    rLabRec.aType = sType;
    rCfg.SaveLabel(sMake, sType, rLabRec);
    bSuccess = true;
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(SwSaveLabelDlg, ModifyHdl)
{
    m_pOKPB->Enable(!m_pMakeCB->GetText().isEmpty() && !m_pTypeED->GetText().isEmpty());
    return 0;
}

bool SwSaveLabelDlg::GetLabel(SwLabItem& rItem)
{
    if(bSuccess)
    {
        rItem.aMake = m_pMakeCB->GetText();
        rItem.aType = m_pTypeED->GetText();
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
