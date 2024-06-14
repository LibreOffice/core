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

#include <svtools/unitconv.hxx>
#include <tools/poly.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>

#include <viewopt.hxx>

#include <swtypes.hxx>
#include <cmdid.h>
#include <label.hxx>
#include <labimp.hxx>
#include <labimg.hxx>
#include "labfmt.hxx"
#include <uitool.hxx>

#include <strings.hrc>

using namespace utl;

#define ROUND(x) static_cast<tools::Long>((x) + .5)

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
            aArr[0].setX( rP2.X() - 5 );
            aArr[0].setY( rP2.Y() - 2 );
            aArr[1].setX( rP2.X() );
            aArr[1].setY( rP2.Y() );
            aArr[2].setX( rP2.X() - 5 );
            aArr[2].setY( rP2.Y() + 2 );
        }
        else
        {
            // Vertical
            aArr[0].setX( rP2.X() - 2 );
            aArr[0].setY( rP2.Y() - 5 );
            aArr[1].setX( rP2.X() + 2 );
            aArr[1].setY( rP2.Y() - 5 );
            aArr[2].setX( rP2.X() );
            aArr[2].setY( rP2.Y() );
        }

        const Color& rFieldTextColor = SwViewOption::GetCurrentViewOptions().GetFontColor();
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

SwLabPreview::SwLabPreview()
    : m_aGrayColor(COL_LIGHTGRAY)
    , m_aHDistStr(SwResId(STR_HDIST))
    , m_aVDistStr(SwResId(STR_VDIST))
    , m_aWidthStr(SwResId(STR_WIDTH))
    , m_aHeightStr(SwResId(STR_HEIGHT))
    , m_aLeftStr(SwResId(STR_LEFT))
    , m_aUpperStr(SwResId(STR_UPPER))
    , m_aColsStr(SwResId(STR_COLS))
    , m_aRowsStr(SwResId(STR_ROWS))
    , m_lHDistWidth(0)
    , m_lVDistWidth(0)
    , m_lHeightWidth(0)
    , m_lLeftWidth(0)
    , m_lUpperWidth(0)
    , m_lColsWidth(0)
    , m_lXWidth(0)
    , m_lXHeight(0)
{
}

void SwLabPreview::SetDrawingArea(weld::DrawingArea* pWidget)
{
    CustomWidgetController::SetDrawingArea(pWidget);

    pWidget->set_size_request(pWidget->get_approximate_digit_width() * 54,
                              pWidget->get_text_height() * 15);

    m_lHDistWidth  = pWidget->get_pixel_size(m_aHDistStr).Width();
    m_lVDistWidth  = pWidget->get_pixel_size(m_aVDistStr).Width();
    m_lHeightWidth = pWidget->get_pixel_size(m_aHeightStr).Width();
    m_lLeftWidth   = pWidget->get_pixel_size(m_aLeftStr).Width();
    m_lUpperWidth  = pWidget->get_pixel_size(m_aUpperStr).Width();
    m_lColsWidth   = pWidget->get_pixel_size(m_aColsStr).Width();
    m_lXWidth  = pWidget->get_pixel_size(OUString('X')).Width();
    m_lXHeight = pWidget->get_text_height();
}

void SwLabPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const Size aSize(GetOutputSizePixel());
    const tools::Long lOutWPix = aSize.Width();
    const tools::Long lOutHPix = aSize.Height();

    // Scale factor
    const double fxpix = double(lOutWPix - (2 * (m_lLeftWidth + 15))) / double(lOutWPix);

    const tools::Long lOutWPix23 = tools::Long(double(lOutWPix) * fxpix);
    const tools::Long lOutHPix23 = tools::Long(double(lOutHPix) * fxpix);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    const Color& rFieldTextColor = SwViewOption::GetCurrentViewOptions().GetFontColor();

    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetFillColor(rWinColor);
    aFont.SetColor(rFieldTextColor);
    rRenderContext.SetFont(aFont);

    rRenderContext.SetBackground(Wallpaper(rWinColor));
    rRenderContext.Erase();

    rRenderContext.SetLineColor(rWinColor);
    rRenderContext.SetFillColor(m_aGrayColor);
    vcl::Font aPaintFont(rRenderContext.GetFont());
    aPaintFont.SetTransparent(false);
    rRenderContext.SetFont(aPaintFont);

    // size of region to be displayed
    const tools::Long lDispW = m_aItem.m_lLeft + m_aItem.m_lHDist + ((m_aItem.m_nCols == 1)
                            ? m_aItem.m_lLeft
                            : ROUND(m_aItem.m_lHDist / 10.0));

    const tools::Long lDispH = m_aItem.m_lUpper + m_aItem.m_lVDist + ((m_aItem.m_nRows == 1)
                            ? m_aItem.m_lUpper
                            : ROUND(m_aItem.m_lVDist / 10.0));

    // Scale factor
    const float fx = float(lOutWPix23) / std::max(tools::Long(1), lDispW);
    const float fy = float(lOutHPix23) / std::max(tools::Long(1), lDispH);
    const float f  = std::min(fx, fy);

    // zero point
    const tools::Long lOutlineW = ROUND(f * lDispW);
    const tools::Long lOutlineH = ROUND(f * lDispH);

    const tools::Long lX0 = (lOutWPix - lOutlineW) / 2;
    const tools::Long lY0 = (lOutHPix - lOutlineH) / 2;
    const tools::Long lX1 = lX0 + ROUND(f *  m_aItem.m_lLeft );
    const tools::Long lY1 = lY0 + ROUND(f *  m_aItem.m_lUpper);
    const tools::Long lX2 = lX0 + ROUND(f * (m_aItem.m_lLeft  + m_aItem.m_lWidth ));
    const tools::Long lY2 = lY0 + ROUND(f * (m_aItem.m_lUpper + m_aItem.m_lHeight));
    const tools::Long lX3 = lX0 + ROUND(f * (m_aItem.m_lLeft  + m_aItem.m_lHDist ));
    const tools::Long lY3 = lY0 + ROUND(f * (m_aItem.m_lUpper + m_aItem.m_lVDist ));

    // draw outline (area)
    rRenderContext.DrawRect(tools::Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH)));

    // draw outline (border)
    rRenderContext.SetLineColor(rFieldTextColor);
    rRenderContext.DrawLine(Point(lX0, lY0), Point(lX0 + lOutlineW - 1, lY0)); // Up
    rRenderContext.DrawLine(Point(lX0, lY0), Point(lX0, lY0 + lOutlineH - 1)); // Left
    if (m_aItem.m_nCols == 1)
        rRenderContext.DrawLine(Point(lX0 + lOutlineW - 1, lY0), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Right
    if (m_aItem.m_nRows == 1)
        rRenderContext.DrawLine(Point(lX0, lY0 + lOutlineH - 1), Point(lX0 + lOutlineW - 1, lY0 + lOutlineH - 1)); // Down

    // Labels
    rRenderContext.SetClipRegion(vcl::Region(tools::Rectangle(Point(lX0, lY0), Size(lOutlineW, lOutlineH))));
    rRenderContext.SetFillColor(COL_LIGHTGRAYBLUE);
    const sal_Int32 nRows = std::min<sal_Int32>(2, m_aItem.m_nRows);
    const sal_Int32 nCols = std::min<sal_Int32>(2, m_aItem.m_nCols);
    for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
        for (sal_Int32 nCol = 0; nCol < nCols; ++nCol)
            rRenderContext.DrawRect(tools::Rectangle(Point(lX0 + ROUND(f * (m_aItem.m_lLeft  + nCol * m_aItem.m_lHDist)),
                                                    lY0 + ROUND(f * (m_aItem.m_lUpper + nRow * m_aItem.m_lVDist))),
                                              Size(ROUND(f * m_aItem.m_lWidth),
                                                   ROUND(f * m_aItem.m_lHeight))));
    rRenderContext.SetClipRegion();

    // annotation: left border
    if (m_aItem.m_lLeft)
    {
        tools::Long lX = (lX0 + lX1) / 2;
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
        tools::Long lX = lX2 - m_lXWidth / 2 - m_lHeightWidth / 2;
        tools::Long lY = lY1 + m_lXHeight;

        rRenderContext.DrawLine(Point(lX1, lY), Point(lX2 - 1, lY));
        rRenderContext.DrawLine(Point(lX, lY1), Point(lX, lY2 - 1));

        rRenderContext.DrawText(Point(lX1 + m_lXWidth / 2, lY - m_lXHeight / 2), m_aWidthStr);
        rRenderContext.DrawText(Point(lX - m_lHeightWidth / 2, lY2 - m_lXHeight - m_lXHeight / 2), m_aHeightStr);
    }

    // annotation: horizontal gap
    if (m_aItem.m_nCols > 1)
    {
        tools::Long lX = (lX1 + lX3) / 2;
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
        tools::Long lY = lY0 + lOutlineH + 4;
        DrawArrow(rRenderContext, Point(lX0, lY), Point(lX0 + lOutlineW - 1, lY), true);
        rRenderContext.DrawText(Point((lX0 + lX0 + lOutlineW - 1) / 2 - m_lColsWidth / 2, lY + 5), m_aColsStr);
    }

    // annotation: lines
    {
        tools::Long lX = lX0 + lOutlineW + 4;
        DrawArrow(rRenderContext, Point(lX, lY0), Point(lX, lY0 + lOutlineH - 1), true);
        rRenderContext.DrawText(Point(lX + 5, (lY0 + lY0 + lOutlineH - 1 - m_lXHeight / 2) / 2), m_aRowsStr);
    }
}

void SwLabPreview::UpdateItem(const SwLabItem& rItem)
{
    m_aItem = rItem;
    Invalidate();
}

SwLabFormatPage::SwLabFormatPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/labelformatpage.ui"_ustr, u"LabelFormatPage"_ustr, &rSet)
    , m_aPreviewIdle("SwLabFormatPage Preview")
    , m_aItem(static_cast<const SwLabItem&>( rSet.Get(FN_LABEL) ))
    , m_bModified(false)
    , m_xMakeFI(m_xBuilder->weld_label(u"make"_ustr))
    , m_xTypeFI(m_xBuilder->weld_label(u"type"_ustr))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreview))
    , m_xHDistField(m_xBuilder->weld_metric_spin_button(u"hori"_ustr, FieldUnit::CM))
    , m_xVDistField(m_xBuilder->weld_metric_spin_button(u"vert"_ustr, FieldUnit::CM))
    , m_xWidthField(m_xBuilder->weld_metric_spin_button(u"width"_ustr, FieldUnit::CM))
    , m_xHeightField(m_xBuilder->weld_metric_spin_button(u"height"_ustr, FieldUnit::CM))
    , m_xLeftField(m_xBuilder->weld_metric_spin_button(u"left"_ustr, FieldUnit::CM))
    , m_xUpperField(m_xBuilder->weld_metric_spin_button(u"top"_ustr, FieldUnit::CM))
    , m_xColsField(m_xBuilder->weld_spin_button(u"cols"_ustr))
    , m_xRowsField(m_xBuilder->weld_spin_button(u"rows"_ustr))
    , m_xPWidthField(m_xBuilder->weld_metric_spin_button(u"pagewidth"_ustr, FieldUnit::CM))
    , m_xPHeightField(m_xBuilder->weld_metric_spin_button(u"pageheight"_ustr, FieldUnit::CM))
    , m_xSavePB(m_xBuilder->weld_button(u"save"_ustr))
{
    SetExchangeSupport();

    // Metrics
    FieldUnit aMetric = ::GetDfltMetric(false);
    ::SetFieldUnit(*m_xHDistField, aMetric);
    ::SetFieldUnit(*m_xVDistField , aMetric);
    ::SetFieldUnit(*m_xWidthField , aMetric);
    ::SetFieldUnit(*m_xHeightField, aMetric);
    ::SetFieldUnit(*m_xLeftField  , aMetric);
    ::SetFieldUnit(*m_xUpperField , aMetric);
    ::SetFieldUnit(*m_xPWidthField , aMetric);
    ::SetFieldUnit(*m_xPHeightField, aMetric);

    // Install handlers
    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwLabFormatPage, MetricModifyHdl);
    m_xHDistField->connect_value_changed( aLk );
    m_xVDistField->connect_value_changed( aLk );
    m_xWidthField->connect_value_changed( aLk );
    m_xHeightField->connect_value_changed( aLk );
    m_xLeftField->connect_value_changed( aLk );
    m_xUpperField->connect_value_changed( aLk );
    m_xPWidthField->connect_value_changed( aLk );
    m_xPHeightField->connect_value_changed( aLk );

    m_xColsField->connect_value_changed(LINK(this, SwLabFormatPage, ModifyHdl));
    m_xRowsField->connect_value_changed(LINK(this, SwLabFormatPage, ModifyHdl));

    m_xSavePB->connect_clicked( LINK (this, SwLabFormatPage, SaveHdl));
    // Set timer
    m_aPreviewIdle.SetPriority(TaskPriority::LOWEST);
    m_aPreviewIdle.SetInvokeHandler(LINK(this, SwLabFormatPage, PreviewHdl));
}

SwLabFormatPage::~SwLabFormatPage()
{
}

// Modify-handler of MetricFields. start preview timer
IMPL_LINK_NOARG(SwLabFormatPage, MetricModifyHdl, weld::MetricSpinButton&, void)
{
    m_bModified = true;
    m_aPreviewIdle.Start();
}

IMPL_LINK_NOARG(SwLabFormatPage, ModifyHdl, weld::SpinButton&, void)
{
    m_bModified = true;
    m_aPreviewIdle.Start();
}

// Invalidate preview
IMPL_LINK_NOARG(SwLabFormatPage, PreviewHdl, Timer *, void)
{
    m_aPreviewIdle.Stop();
    ChangeMinMax();
    FillItem( m_aItem );
    m_aPreview.UpdateItem(m_aItem);
}

void SwLabFormatPage::ChangeMinMax()
{
    tools::Long lMax = 31748; // 56 cm
    tools::Long nMinSize = 10; // 0,1cm

    // Min and Max

    int nCols   = m_xColsField->get_value(),
        nRows   = m_xRowsField->get_value();
    tools::Long lLeft   = static_cast< tools::Long >(getfldval(*m_xLeftField )),
         lUpper  = static_cast< tools::Long >(getfldval(*m_xUpperField)),
         lHDist  = static_cast< tools::Long >(getfldval(*m_xHDistField)),
         lVDist  = static_cast< tools::Long >(getfldval(*m_xVDistField)),
         lWidth  = static_cast< tools::Long >(getfldval(*m_xWidthField)),
         lHeight = static_cast< tools::Long >(getfldval(*m_xHeightField)),
         lMinPWidth  = lLeft  + (nCols - 1) * lHDist + lWidth,
         lMinPHeight = lUpper + (nRows - 1) * lVDist + lHeight;

    m_xHDistField->set_min(nMinSize, FieldUnit::CM);
    m_xVDistField->set_min(nMinSize, FieldUnit::CM);

    m_xHDistField->set_max(100 * ((lMax - lLeft ) / std::max(1, nCols)), FieldUnit::TWIP);
    m_xVDistField->set_max(100 * ((lMax - lUpper) / std::max(1, nRows)), FieldUnit::TWIP);

    m_xWidthField->set_min(nMinSize, FieldUnit::CM);
    m_xHeightField->set_min(nMinSize, FieldUnit::CM);

    m_xWidthField->set_max(tools::Long(100) * lHDist, FieldUnit::TWIP);
    m_xHeightField->set_max(tools::Long(100) * lVDist, FieldUnit::TWIP);

    m_xLeftField->set_max(tools::Long(100) * (lMax - nCols * lHDist), FieldUnit::TWIP);
    m_xUpperField->set_max(tools::Long(100) * (lMax - nRows * lVDist), FieldUnit::TWIP);

    m_xColsField->set_range(1, (lMax - lLeft ) / std::max(tools::Long(1), lHDist));
    m_xRowsField->set_range(1, (lMax - lUpper) / std::max(tools::Long(1), lVDist));

    m_xPWidthField->set_range(tools::Long(100) * lMinPWidth, tools::Long(100) * lMax, FieldUnit::TWIP);
    m_xPHeightField->set_range(tools::Long(100) * lMinPHeight, tools::Long(100) * lMax, FieldUnit::TWIP);
}

std::unique_ptr<SfxTabPage> SwLabFormatPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwLabFormatPage>(pPage, pController, *rSet);
}

void SwLabFormatPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    Reset(&aSet);
}

DeactivateRC SwLabFormatPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

void SwLabFormatPage::FillItem(SwLabItem& rItem)
{
    if (!m_bModified)
        return;

    rItem.m_aMake = rItem.m_aType = SwResId(STR_CUSTOM_LABEL);

    SwLabRec& rRec = *GetParentSwLabDlg()->Recs()[0];
    rItem.m_lHDist  = rRec.m_nHDist  = static_cast< tools::Long >(getfldval(*m_xHDistField ));
    rItem.m_lVDist  = rRec.m_nVDist  = static_cast< tools::Long >(getfldval(*m_xVDistField ));
    rItem.m_lWidth  = rRec.m_nWidth  = static_cast< tools::Long >(getfldval(*m_xWidthField ));
    rItem.m_lHeight = rRec.m_nHeight = static_cast< tools::Long >(getfldval(*m_xHeightField));
    rItem.m_lLeft   = rRec.m_nLeft   = static_cast< tools::Long >(getfldval(*m_xLeftField  ));
    rItem.m_lUpper  = rRec.m_nUpper  = static_cast< tools::Long >(getfldval(*m_xUpperField ));
    rItem.m_nCols   = rRec.m_nCols   = static_cast< sal_Int32 >(m_xColsField->get_value());
    rItem.m_nRows   = rRec.m_nRows   = static_cast< sal_Int32 >(m_xRowsField->get_value());
    rItem.m_lPWidth  = rRec.m_nPWidth  = static_cast< tools::Long >(getfldval(*m_xPWidthField ));
    rItem.m_lPHeight = rRec.m_nPHeight = static_cast< tools::Long >(getfldval(*m_xPHeightField));

}

bool SwLabFormatPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem(m_aItem);
    rSet->Put(m_aItem);

    return true;
}

void SwLabFormatPage::Reset(const SfxItemSet* )
{
    // Initialise fields
    GetParentSwLabDlg()->GetLabItem(m_aItem);

    m_xHDistField->set_max(100 * m_aItem.m_lHDist , FieldUnit::TWIP);
    m_xVDistField->set_max(100 * m_aItem.m_lVDist , FieldUnit::TWIP);
    m_xWidthField->set_max(100 * m_aItem.m_lWidth , FieldUnit::TWIP);
    m_xHeightField->set_max(100 * m_aItem.m_lHeight, FieldUnit::TWIP);
    m_xLeftField->set_max(100 * m_aItem.m_lLeft  , FieldUnit::TWIP);
    m_xUpperField->set_max(100 * m_aItem.m_lUpper , FieldUnit::TWIP);
    m_xPWidthField->set_max(100 * m_aItem.m_lPWidth , FieldUnit::TWIP);
    m_xPHeightField->set_max(100 * m_aItem.m_lPHeight, FieldUnit::TWIP);

    setfldval(*m_xHDistField, m_aItem.m_lHDist );
    setfldval(*m_xVDistField , m_aItem.m_lVDist );
    setfldval(*m_xWidthField , m_aItem.m_lWidth );
    setfldval(*m_xHeightField, m_aItem.m_lHeight);
    setfldval(*m_xLeftField  , m_aItem.m_lLeft  );
    setfldval(*m_xUpperField , m_aItem.m_lUpper );
    setfldval(*m_xPWidthField , m_aItem.m_lPWidth );
    setfldval(*m_xPHeightField, m_aItem.m_lPHeight);

    m_xColsField->set_max(m_aItem.m_nCols);
    m_xRowsField->set_max(m_aItem.m_nRows);

    m_xColsField->set_value(m_aItem.m_nCols);
    m_xRowsField->set_value(m_aItem.m_nRows);
    m_xMakeFI->set_label(m_aItem.m_aMake);
    m_xTypeFI->set_label(m_aItem.m_aType);
    PreviewHdl(nullptr);
}

IMPL_LINK_NOARG(SwLabFormatPage, SaveHdl, weld::Button&, void)
{
    SwLabRec aRec;
    aRec.m_nHDist  = static_cast< tools::Long >(getfldval(*m_xHDistField));
    aRec.m_nVDist  = static_cast< tools::Long >(getfldval(*m_xVDistField ));
    aRec.m_nWidth  = static_cast< tools::Long >(getfldval(*m_xWidthField ));
    aRec.m_nHeight = static_cast< tools::Long >(getfldval(*m_xHeightField));
    aRec.m_nLeft   = static_cast< tools::Long >(getfldval(*m_xLeftField  ));
    aRec.m_nUpper  = static_cast< tools::Long >(getfldval(*m_xUpperField ));
    aRec.m_nCols   = static_cast< sal_Int32 >(m_xColsField->get_value());
    aRec.m_nRows   = static_cast< sal_Int32 >(m_xRowsField->get_value());
    aRec.m_nPWidth  = static_cast< tools::Long >(getfldval(*m_xPWidthField ));
    aRec.m_nPHeight = static_cast< tools::Long >(getfldval(*m_xPHeightField));
    aRec.m_bCont = m_aItem.m_bCont;
    SwSaveLabelDlg aSaveDlg(GetParentSwLabDlg(), aRec);
    aSaveDlg.SetLabel(m_aItem.m_aLstMake, m_aItem.m_aLstType);
    aSaveDlg.run();
    if (aSaveDlg.GetLabel(m_aItem))
    {
        m_bModified = false;
        const std::vector<OUString>& rMan = GetParentSwLabDlg()->GetLabelsConfig().GetManufacturers();
        std::vector<OUString>& rMakes(GetParentSwLabDlg()->Makes());
        if(rMakes.size() < rMan.size())
        {
            rMakes = rMan;
        }
        m_xMakeFI->set_label(m_aItem.m_aMake);
        m_xTypeFI->set_label(m_aItem.m_aType);
    }
}

SwSaveLabelDlg::SwSaveLabelDlg(SwLabDlg* pParent, SwLabRec& rRec)
    : GenericDialogController(pParent->getDialog(), u"modules/swriter/ui/savelabeldialog.ui"_ustr, u"SaveLabelDialog"_ustr)
    , m_bSuccess(false)
    , m_pLabDialog(pParent)
    , m_rLabRec(rRec)
    , m_xMakeCB(m_xBuilder->weld_combo_box(u"brand"_ustr))
    , m_xTypeED(m_xBuilder->weld_entry(u"type"_ustr))
    , m_xOKPB(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xOKPB->connect_clicked(LINK(this, SwSaveLabelDlg, OkHdl));
    m_xMakeCB->connect_changed(LINK(this, SwSaveLabelDlg, ModifyComboHdl));
    m_xTypeED->connect_changed(LINK(this, SwSaveLabelDlg, ModifyEntryHdl));

    SwLabelConfig& rCfg = m_pLabDialog->GetLabelsConfig();
    const std::vector<OUString>& rMan = rCfg.GetManufacturers();
    for (const auto & i : rMan)
    {
        m_xMakeCB->append_text(i);
    }
}

SwSaveLabelDlg::~SwSaveLabelDlg()
{
}

IMPL_LINK_NOARG(SwSaveLabelDlg, OkHdl, weld::Button&, void)
{
    SwLabelConfig& rCfg = m_pLabDialog->GetLabelsConfig();
    OUString sMake(m_xMakeCB->get_active_text());
    OUString sType(m_xTypeED->get_text());
    if(rCfg.HasLabel(sMake, sType))
    {
        if ( rCfg.IsPredefinedLabel(sMake, sType) )
        {
            SAL_WARN( "sw.envelp", "label is predefined and cannot be overwritten" );
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), u"modules/swriter/ui/cannotsavelabeldialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"CannotSaveLabelDialog"_ustr));
            xBox->run();
            return;
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), u"modules/swriter/ui/querysavelabeldialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog(u"QuerySaveLabelDialog"_ustr));
        xQuery->set_primary_text(xQuery->get_primary_text().
            replaceAll("%1", sMake).replaceAll("%2", sType));
        xQuery->set_secondary_text(xQuery->get_secondary_text().
            replaceAll("%1", sMake).replaceAll("%2", sType));

        if (RET_YES != xQuery->run())
            return;
    }
    m_rLabRec.m_aType = sType;
    rCfg.SaveLabel(sMake, sType, m_rLabRec);
    m_bSuccess = true;
    m_xDialog->response(RET_OK);
}

void SwSaveLabelDlg::Modify()
{
    m_xOKPB->set_sensitive(!m_xMakeCB->get_active_text().isEmpty() && !m_xTypeED->get_text().isEmpty());
}

IMPL_LINK_NOARG(SwSaveLabelDlg, ModifyComboHdl, weld::ComboBox&, void)
{
    Modify();
}

IMPL_LINK_NOARG(SwSaveLabelDlg, ModifyEntryHdl, weld::Entry&, void)
{
    Modify();
}

bool SwSaveLabelDlg::GetLabel(SwLabItem& rItem)
{
    if(m_bSuccess)
    {
        rItem.m_aMake = m_xMakeCB->get_active_text();
        rItem.m_aType = m_xTypeED->get_text();
        rItem.m_lHDist  = m_rLabRec.m_nHDist;
        rItem.m_lVDist  = m_rLabRec.m_nVDist;
        rItem.m_lWidth  = m_rLabRec.m_nWidth;
        rItem.m_lHeight = m_rLabRec.m_nHeight;
        rItem.m_lLeft   = m_rLabRec.m_nLeft;
        rItem.m_lUpper  = m_rLabRec.m_nUpper;
        rItem.m_nCols   = m_rLabRec.m_nCols;
        rItem.m_nRows   = m_rLabRec.m_nRows;
        rItem.m_lPWidth  = m_rLabRec.m_nPWidth;
        rItem.m_lPHeight = m_rLabRec.m_nPHeight;
    }
    return m_bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
