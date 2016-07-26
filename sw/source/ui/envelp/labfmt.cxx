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
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <viewopt.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "label.hxx"
#include "labimp.hxx"
#include "labimg.hxx"
#include "labfmt.hxx"
#include "uitool.hxx"

#include "../../uibase/envelp/label.hrc"
#include "labfmt.hrc"
#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define ROUND(x) static_cast<long>((x) + .5)

namespace {

// Arrow or interval character
void DrawArrow(vcl::RenderContext& rRenderContext, const Point &rP1, const Point &rP2, bool bArrow)
{
    rRenderContext.DrawLine(rP1, rP2);
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
        rRenderContext.SetFillColor(rFieldTextColor);
        rRenderContext.DrawPolygon( tools::Polygon(3, aArr));
    }
    else
    {
        // Interval symbol
        if (rP1.Y() == rP2.Y())
        {
            // Horizontal
            rRenderContext.DrawLine(Point(rP1.X(), rP1.Y() - 2), Point(rP1.X(), rP1.Y() + 2));
            rRenderContext.DrawLine(Point(rP2.X(), rP2.Y() - 2), Point(rP2.X(), rP2.Y() + 2));
        }
        else
        {
            // Vertical
            rRenderContext.DrawLine(Point(rP1.X() - 2, rP1.Y()), Point(rP1.X() + 2, rP1.Y()));
            rRenderContext.DrawLine(Point(rP2.X() - 2, rP2.Y()), Point(rP2.X() + 2, rP2.Y()));
        }
    }
}

}

SwLabPreview::SwLabPreview(vcl::Window* pParent)
    : Window(pParent, 0)
    , m_aGrayColor(COL_LIGHTGRAY)
    , m_aHDistStr(SW_RESSTR(STR_HDIST))
    , m_aVDistStr(SW_RESSTR(STR_VDIST))
    , m_aWidthStr(SW_RESSTR(STR_WIDTH))
    , m_aHeightStr(SW_RESSTR(STR_HEIGHT))
    , m_aLeftStr(SW_RESSTR(STR_LEFT))
    , m_aUpperStr(SW_RESSTR(STR_UPPER))
    , m_aColsStr(SW_RESSTR(STR_COLS))
    , m_aRowsStr(SW_RESSTR(STR_ROWS))
    , m_aPWidthStr(SW_RESSTR(STR_PWIDTH))
    , m_aPHeightStr(SW_RESSTR(STR_PHEIGHT))
{
    SetMapMode(MAP_PIXEL);

    // FIXME RenderContext

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    SetBackground(Wallpaper(rWinColor));

    vcl::Font aFont = GetFont();
    aFont.SetTransparent(true);
    aFont.SetWeight(WEIGHT_NORMAL);
    SetFont(aFont);

    m_lHDistWidth  = GetTextWidth(m_aHDistStr );
    m_lVDistWidth  = GetTextWidth(m_aVDistStr );
    m_lHeightWidth = GetTextWidth(m_aHeightStr);
    m_lLeftWidth   = GetTextWidth(m_aLeftStr  );
    m_lUpperWidth  = GetTextWidth(m_aUpperStr );
    m_lColsWidth   = GetTextWidth(m_aColsStr  );
    m_lPWidthWidth  = GetTextWidth(m_aPWidthStr);
    m_lPHeightWidth = GetTextWidth(m_aPHeightStr);
    m_lXHeight = GetTextHeight();
    m_lXWidth  = GetTextWidth(OUString('X'));
}

Size SwLabPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(146 , 161), MapMode(MAP_APPFONT));
}

VCL_BUILDER_FACTORY(SwLabPreview)

void SwLabPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    const Size aSz(GetOutputSizePixel());

    const long lOutWPix = aSz.Width ();
    const long lOutHPix = aSz.Height();

    // Scale factor
    const double fxpix = double(lOutWPix - (2 * (m_lLeftWidth + 15))) / double(lOutWPix);

    const long lOutWPix23 = long(double(lOutWPix) * fxpix);
    const long lOutHPix23 = long(double(lOutHPix) * fxpix);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    const Color& rFieldTextColor = SwViewOption::GetFontColor();

    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetFillColor(rWinColor);
    aFont.SetColor(rFieldTextColor);
    rRenderContext.SetFont(aFont);

    rRenderContext.SetBackground(Wallpaper(rWinColor));

    rRenderContext.SetLineColor(rWinColor);
    rRenderContext.SetFillColor(m_aGrayColor);
    vcl::Font aPaintFont(rRenderContext.GetFont());
    aPaintFont.SetTransparent(false);
    rRenderContext.SetFont(aPaintFont);

    // size of region to be displayed
    const long lDispW = m_aItem.m_lLeft + m_aItem.m_lHDist + ((m_aItem.m_nCols == 1)
                            ? m_aItem.m_lLeft
                            : ROUND(m_aItem.m_lHDist / 10.0));

    const long lDispH = m_aItem.m_lUpper + m_aItem.m_lVDist + ((m_aItem.m_nRows == 1)
                            ? m_aItem.m_lUpper
                            : ROUND(m_aItem.m_lVDist / 10.0));

    // Scale factor
    const float fx = float(lOutWPix23) / std::max(1L, lDispW);
    const float fy = float(lOutHPix23) / std::max(1L, lDispH);
    const float f  = fx < fy ? fx : fy;

    // zero point
    const long lOutlineW = ROUND(f * lDispW);
    const long lOutlineH = ROUND(f * lDispH);

    const long lX0 = (lOutWPix - lOutlineW) / 2;
    const long lY0 = (lOutHPix - lOutlineH) / 2;
    const long lX1 = lX0 + ROUND(f *  m_aItem.m_lLeft );
    const long lY1 = lY0 + ROUND(f *  m_aItem.m_lUpper);
    const long lX2 = lX0 + ROUND(f * (m_aItem.m_lLeft  + m_aItem.m_lWidth ));
    const long lY2 = lY0 + ROUND(f * (m_aItem.m_lUpper + m_aItem.m_lHeight));
    const long lX3 = lX0 + ROUND(f * (m_aItem.m_lLeft  + m_aItem.m_lHDist ));
    const long lY3 = lY0 + ROUND(f * (m_aItem.m_lUpper + m_aItem.m_lVDist ));

    // draw outline (area)
    rRenderContext.DrawRect(Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH)));

    // draw outline (border)
    rRenderContext.SetLineColor(rFieldTextColor);
    rRenderContext.DrawLine(Point(lX0, lY0), Point(lX0 + lOutlineW - 1, lY0)); // Up
    rRenderContext.DrawLine(Point(lX0, lY0), Point(lX0, lY0 + lOutlineH - 1)); // Left
    if (m_aItem.m_nCols == 1)
        rRenderContext.DrawLine(Point(lX0 + lOutlineW - 1, lY0), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Right
    if (m_aItem.m_nRows == 1)
        rRenderContext.DrawLine(Point(lX0, lY0 + lOutlineH - 1), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Down

    // Labels
    rRenderContext.SetClipRegion(vcl::Region(Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH))));
    rRenderContext.SetFillColor(COL_LIGHTGRAYBLUE);
    const sal_Int32 nRows = std::min<sal_Int32>(2, m_aItem.m_nRows);
    const sal_Int32 nCols = std::min<sal_Int32>(2, m_aItem.m_nCols);
    for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
        for (sal_Int32 nCol = 0; nCol < nCols; ++nCol)
            rRenderContext.DrawRect(Rectangle(Point(lX0 + ROUND(f * (m_aItem.m_lLeft  + nCol * m_aItem.m_lHDist)),
                                                    lY0 + ROUND(f * (m_aItem.m_lUpper + nRow * m_aItem.m_lVDist))),
                                              Size(ROUND(f * m_aItem.m_lWidth),
                                                   ROUND(f * m_aItem.m_lHeight))));
    rRenderContext.SetClipRegion();

    // annotation: left border
    if (m_aItem.m_lLeft)
    {
        long lX = (lX0 + lX1) / 2;
        DrawArrow(rRenderContext, Point(lX0, lY0 - 5), Point(lX1, lY0 - 5), false);
        DrawArrow(rRenderContext, Point(lX, lY0 - 10), Point(lX, lY0 - 5), true);
        rRenderContext.DrawText(Point(lX1 - m_lLeftWidth, lY0 - 10 - m_lXHeight), m_aLeftStr);
    }

    // annotation: upper border
    if (m_aItem.m_lUpper)
    {
        DrawArrow(rRenderContext, Point(lX0 - 5, lY0), Point(lX0 - 5, lY1), false);
        rRenderContext.DrawText(Point(lX0 - 10 - m_lUpperWidth, lY0 + ROUND(f*m_aItem.m_lUpper/2.0 - m_lXHeight/2.0)), m_aUpperStr);
    }

    // annotation: width and height
    {
        long lX = lX2 - m_lXWidth / 2 - m_lHeightWidth / 2;
        long lY = lY1 + m_lXHeight;

        rRenderContext.DrawLine(Point(lX1, lY), Point(lX2 - 1, lY));
        rRenderContext.DrawLine(Point(lX, lY1), Point(lX, lY2 - 1));

        rRenderContext.DrawText(Point(lX1 + m_lXWidth / 2, lY - m_lXHeight / 2), m_aWidthStr);
        rRenderContext.DrawText(Point(lX - m_lHeightWidth / 2, lY2 - m_lXHeight - m_lXHeight / 2), m_aHeightStr);
    }

    // annotation: horizontal gap
    if (m_aItem.m_nCols > 1)
    {
        long lX = (lX1 + lX3) / 2;
        DrawArrow(rRenderContext, Point(lX1, lY0 - 5), Point(lX3, lY0 - 5), false);
        DrawArrow(rRenderContext, Point(lX, lY0 - 10), Point(lX, lY0 - 5), true);
        rRenderContext.DrawText(Point(lX - m_lHDistWidth / 2, lY0 - 10 - m_lXHeight), m_aHDistStr);
    }

    // annotation: vertical gap
    if (m_aItem.m_nRows > 1)
    {
        DrawArrow(rRenderContext, Point(lX0 - 5, lY1), Point(lX0 - 5, lY3), false);
        rRenderContext.DrawText(Point(lX0 - 10 - m_lVDistWidth, lY1 + ROUND(f*m_aItem.m_lVDist/2.0 - m_lXHeight/2.0)), m_aVDistStr);
    }

    // annotation: columns
    {
        long lY = lY0 + lOutlineH + 4;
        DrawArrow(rRenderContext, Point(lX0, lY), Point(lX0 + lOutlineW - 1, lY), true);
        rRenderContext.DrawText(Point((lX0 + lX0 + lOutlineW - 1) / 2 - m_lColsWidth / 2, lY + 5), m_aColsStr);
    }

    // annotation: lines
    {
        long lX = lX0 + lOutlineW + 4;
        DrawArrow(rRenderContext, Point(lX, lY0), Point(lX, lY0 + lOutlineH - 1), true);
        rRenderContext.DrawText(Point(lX + 5, (lY0 + lY0 + lOutlineH - 1 - m_lXHeight / 2) / 2), m_aRowsStr);
    }
}

void SwLabPreview::UpdateItem(const SwLabItem& rItem)
{
    m_aItem = rItem;
    Invalidate();
}

SwLabFormatPage::SwLabFormatPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "LabelFormatPage",
        "modules/swriter/ui/labelformatpage.ui", &rSet)
    , aPreviewIdle("SwLabFormatPage Preview")
    , bModified(false)
    , aItem(static_cast<const SwLabItem&>( rSet.Get(FN_LABEL) ))
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
    FieldUnit aMetric = ::GetDfltMetric(false);
    SetMetric(*m_pHDistField, aMetric);
    SetMetric(*m_pVDistField , aMetric);
    SetMetric(*m_pWidthField , aMetric);
    SetMetric(*m_pHeightField, aMetric);
    SetMetric(*m_pLeftField  , aMetric);
    SetMetric(*m_pUpperField , aMetric);
    SetMetric(*m_pPWidthField , aMetric);
    SetMetric(*m_pPHeightField, aMetric);

    // Install handlers
    Link<Edit&,void> aLk = LINK(this, SwLabFormatPage, ModifyHdl);
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

    Link<Control&,void> aLk2 = LINK(this, SwLabFormatPage, LoseFocusHdl);
    m_pHDistField->SetLoseFocusHdl( aLk2 );
    m_pVDistField->SetLoseFocusHdl( aLk2 );
    m_pWidthField->SetLoseFocusHdl( aLk2 );
    m_pHeightField->SetLoseFocusHdl( aLk2 );
    m_pLeftField->SetLoseFocusHdl( aLk2 );
    m_pUpperField->SetLoseFocusHdl( aLk2 );
    m_pColsField->SetLoseFocusHdl( aLk2 );
    m_pRowsField->SetLoseFocusHdl( aLk2 );
    m_pPWidthField->SetLoseFocusHdl( aLk2 );
    m_pPHeightField->SetLoseFocusHdl( aLk2 );

    m_pSavePB->SetClickHdl( LINK (this, SwLabFormatPage, SaveHdl));
    // Set timer
    aPreviewIdle.SetPriority(SchedulerPriority::LOWEST);
    aPreviewIdle.SetIdleHdl(LINK(this, SwLabFormatPage, PreviewHdl));
}

SwLabFormatPage::~SwLabFormatPage()
{
    disposeOnce();
}

void SwLabFormatPage::dispose()
{
    m_pMakeFI.clear();
    m_pTypeFI.clear();
    m_pPreview.clear();
    m_pHDistField.clear();
    m_pVDistField.clear();
    m_pWidthField.clear();
    m_pHeightField.clear();
    m_pLeftField.clear();
    m_pUpperField.clear();
    m_pColsField.clear();
    m_pRowsField.clear();
    m_pPWidthField.clear();
    m_pPHeightField.clear();
    m_pSavePB.clear();
    SfxTabPage::dispose();
}


// Modify-handler of MetricFields. start preview timer
IMPL_LINK_NOARG_TYPED(SwLabFormatPage, ModifyHdl, Edit&, void)
{
    bModified = true;
    aPreviewIdle.Start();
}

// Invalidate preview
IMPL_LINK_NOARG_TYPED(SwLabFormatPage, PreviewHdl, Idle *, void)
{
    aPreviewIdle.Stop();
    ChangeMinMax();
    FillItem( aItem );
    m_pPreview->UpdateItem( aItem );
}

// LoseFocus-Handler: Update on change
IMPL_LINK_TYPED( SwLabFormatPage, LoseFocusHdl, Control&, rControl, void )
{
    if (static_cast<Edit*>( &rControl)->IsModified())
        PreviewHdl(nullptr);
}

void SwLabFormatPage::ChangeMinMax()
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

VclPtr<SfxTabPage> SwLabFormatPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwLabFormatPage>::Create(pParent, *rSet);
}

void SwLabFormatPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    Reset(&aSet);
}

SfxTabPage::sfxpg SwLabFormatPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(_pSet);

    return LEAVE_PAGE;
}

void SwLabFormatPage::FillItem(SwLabItem& rItem)
{
    if (bModified)
    {
        rItem.m_aMake = rItem.m_aType = SW_RESSTR(STR_CUSTOM);

        SwLabRec& rRec = *GetParentSwLabDlg()->Recs()[0];
        rItem.m_lHDist  = rRec.lHDist  = static_cast< long >(GETFLDVAL(*m_pHDistField ));
        rItem.m_lVDist  = rRec.lVDist  = static_cast< long >(GETFLDVAL(*m_pVDistField ));
        rItem.m_lWidth  = rRec.lWidth  = static_cast< long >(GETFLDVAL(*m_pWidthField ));
        rItem.m_lHeight = rRec.lHeight = static_cast< long >(GETFLDVAL(*m_pHeightField));
        rItem.m_lLeft   = rRec.lLeft   = static_cast< long >(GETFLDVAL(*m_pLeftField  ));
        rItem.m_lUpper  = rRec.lUpper  = static_cast< long >(GETFLDVAL(*m_pUpperField ));
        rItem.m_nCols   = rRec.nCols   = static_cast< sal_Int32 >(m_pColsField->GetValue());
        rItem.m_nRows   = rRec.nRows   = static_cast< sal_Int32 >(m_pRowsField->GetValue());
        rItem.m_lPWidth  = rRec.lPWidth  = static_cast< long >(GETFLDVAL(*m_pPWidthField ));
        rItem.m_lPHeight = rRec.lPHeight = static_cast< long >(GETFLDVAL(*m_pPHeightField));
    }
}

bool SwLabFormatPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem(aItem);
    rSet->Put(aItem);

    return true;
}

void SwLabFormatPage::Reset(const SfxItemSet* )
{
    // Initialise fields
    GetParentSwLabDlg()->GetLabItem(aItem);

    m_pHDistField->SetMax(100 * aItem.m_lHDist , FUNIT_TWIP);
    m_pVDistField->SetMax(100 * aItem.m_lVDist , FUNIT_TWIP);
    m_pWidthField->SetMax(100 * aItem.m_lWidth , FUNIT_TWIP);
    m_pHeightField->SetMax(100 * aItem.m_lHeight, FUNIT_TWIP);
    m_pLeftField->SetMax(100 * aItem.m_lLeft  , FUNIT_TWIP);
    m_pUpperField->SetMax(100 * aItem.m_lUpper , FUNIT_TWIP);
    m_pPWidthField->SetMax(100 * aItem.m_lPWidth , FUNIT_TWIP);
    m_pPHeightField->SetMax(100 * aItem.m_lPHeight, FUNIT_TWIP);

    SETFLDVAL(*m_pHDistField, aItem.m_lHDist );
    SETFLDVAL(*m_pVDistField , aItem.m_lVDist );
    SETFLDVAL(*m_pWidthField , aItem.m_lWidth );
    SETFLDVAL(*m_pHeightField, aItem.m_lHeight);
    SETFLDVAL(*m_pLeftField  , aItem.m_lLeft  );
    SETFLDVAL(*m_pUpperField , aItem.m_lUpper );
    SETFLDVAL(*m_pPWidthField , aItem.m_lPWidth );
    SETFLDVAL(*m_pPHeightField, aItem.m_lPHeight);

    m_pColsField->SetMax(aItem.m_nCols);
    m_pRowsField->SetMax(aItem.m_nRows);

    m_pColsField->SetValue(aItem.m_nCols);
    m_pRowsField->SetValue(aItem.m_nRows);
    m_pMakeFI->SetText(aItem.m_aMake);
    m_pTypeFI->SetText(aItem.m_aType);
    PreviewHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED(SwLabFormatPage, SaveHdl, Button*, void)
{
    SwLabRec aRec;
    aRec.lHDist  = static_cast< long >(GETFLDVAL(*m_pHDistField));
    aRec.lVDist  = static_cast< long >(GETFLDVAL(*m_pVDistField ));
    aRec.lWidth  = static_cast< long >(GETFLDVAL(*m_pWidthField ));
    aRec.lHeight = static_cast< long >(GETFLDVAL(*m_pHeightField));
    aRec.lLeft   = static_cast< long >(GETFLDVAL(*m_pLeftField  ));
    aRec.lUpper  = static_cast< long >(GETFLDVAL(*m_pUpperField ));
    aRec.nCols   = static_cast< sal_Int32 >(m_pColsField->GetValue());
    aRec.nRows   = static_cast< sal_Int32 >(m_pRowsField->GetValue());
    aRec.lPWidth  = static_cast< long >(GETFLDVAL(*m_pPWidthField ));
    aRec.lPHeight = static_cast< long >(GETFLDVAL(*m_pPHeightField));
    aRec.bCont = aItem.m_bCont;
    ScopedVclPtrInstance< SwSaveLabelDlg > pSaveDlg(this, aRec);
    pSaveDlg->SetLabel(aItem.m_aLstMake, aItem.m_aLstType);
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
        m_pMakeFI->SetText(aItem.m_aMake);
        m_pTypeFI->SetText(aItem.m_aType);
    }
}

SwSaveLabelDlg::SwSaveLabelDlg(SwLabFormatPage* pParent, SwLabRec& rRec)
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
    Link<Edit&,void> aLk(LINK(this, SwSaveLabelDlg, ModifyHdl));
    m_pMakeCB->SetModifyHdl(aLk);
    m_pTypeED->SetModifyHdl(aLk);

    SwLabelConfig& rCfg = pLabPage->GetParentSwLabDlg()->GetLabelsConfig();
    const std::vector<OUString>& rMan = rCfg.GetManufacturers();
    for (const auto & i : rMan)
    {
        m_pMakeCB->InsertEntry(i);
    }
}

SwSaveLabelDlg::~SwSaveLabelDlg()
{
    disposeOnce();
}

void SwSaveLabelDlg::dispose()
{
    m_pMakeCB.clear();
    m_pTypeED.clear();
    m_pOKPB.clear();
    pLabPage.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SwSaveLabelDlg, OkHdl, Button*, void)
{
    SwLabelConfig& rCfg = pLabPage->GetParentSwLabDlg()->GetLabelsConfig();
    OUString sMake(m_pMakeCB->GetText());
    OUString sType(m_pTypeED->GetText());
    if(rCfg.HasLabel(sMake, sType))
    {
        if ( rCfg.IsPredefinedLabel(sMake, sType) )
        {
            SAL_WARN( "sw.envelp", "label is predefined and cannot be overwritten" );
            ScopedVclPtrInstance<MessageDialog>(this, "CannotSaveLabelDialog", "modules/swriter/ui/cannotsavelabeldialog.ui")->Execute();
            return;
        }

        ScopedVclPtrInstance<MessageDialog> aQuery(this, "QuerySaveLabelDialog",
                                                   "modules/swriter/ui/querysavelabeldialog.ui");

        aQuery->set_primary_text(aQuery->get_primary_text().
            replaceAll("%1", sMake).replaceAll("%2", sType));
        aQuery->set_secondary_text(aQuery->get_secondary_text().
            replaceAll("%1", sMake).replaceAll("%2", sType));

        if (RET_YES != aQuery->Execute())
            return;
    }
    rLabRec.aType = sType;
    rCfg.SaveLabel(sMake, sType, rLabRec);
    bSuccess = true;
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG_TYPED(SwSaveLabelDlg, ModifyHdl, Edit&, void)
{
    m_pOKPB->Enable(!m_pMakeCB->GetText().isEmpty() && !m_pTypeED->GetText().isEmpty());
}

bool SwSaveLabelDlg::GetLabel(SwLabItem& rItem)
{
    if(bSuccess)
    {
        rItem.m_aMake = m_pMakeCB->GetText();
        rItem.m_aType = m_pTypeED->GetText();
        rItem.m_lHDist  = rLabRec.lHDist;
        rItem.m_lVDist  = rLabRec.lVDist;
        rItem.m_lWidth  = rLabRec.lWidth;
        rItem.m_lHeight = rLabRec.lHeight;
        rItem.m_lLeft   = rLabRec.lLeft;
        rItem.m_lUpper  = rLabRec.lUpper;
        rItem.m_nCols   = rLabRec.nCols;
        rItem.m_nRows   = rLabRec.nRows;
        rItem.m_lPWidth  = rLabRec.lPWidth;
        rItem.m_lPHeight = rLabRec.lPHeight;
    }
    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
