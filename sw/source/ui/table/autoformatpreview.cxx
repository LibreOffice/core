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

#include <vcl/builderfactory.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <svtools/scriptedtext.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <strings.hrc>

#include <autoformatpreview.hxx>

#define FRAME_OFFSET 4

AutoFormatPreview::AutoFormatPreview(weld::DrawingArea* pDrawingArea)
    : mxDrawingArea(pDrawingArea)
    , aCurData(OUString())
    , bFitWidth(false)
    , mbRTL(false)
    , aStrJan(SwResId(STR_JAN))
    , aStrFeb(SwResId(STR_FEB))
    , aStrMar(SwResId(STR_MAR))
    , aStrNorth(SwResId(STR_NORTH))
    , aStrMid(SwResId(STR_MID))
    , aStrSouth(SwResId(STR_SOUTH))
    , aStrSum(SwResId(STR_SUM))
{
    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    m_xBreak = i18n::BreakIterator::create(xContext);
    mxNumFormat.reset(new SvNumberFormatter(xContext, LANGUAGE_SYSTEM));

    Init();

    mxDrawingArea->connect_size_allocate(LINK(this, AutoFormatPreview, DoResize));
    mxDrawingArea->connect_draw(LINK(this, AutoFormatPreview, DoPaint));
}

IMPL_LINK(AutoFormatPreview, DoResize, const Size&, rSize, void)
{
    aPrvSize = Size(rSize.Width() - 6, rSize.Height() - 30);
    nLabelColWidth = (aPrvSize.Width() - 4) / 4 - 12;
    nDataColWidth1 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 3;
    nDataColWidth2 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 4;
    nRowHeight = (aPrvSize.Height() - 4) / 5;
    NotifyChange(aCurData);
}

void AutoFormatPreview::DetectRTL(SwWrtShell const* pWrtShell)
{
    if (!pWrtShell->IsCursorInTable()) // We haven't created the table yet
        mbRTL = AllSettings::GetLayoutRTL();
    else
        mbRTL = pWrtShell->IsTableRightToLeft();
}

static void lcl_SetFontProperties(vcl::Font& rFont, const SvxFontItem& rFontItem,
                                  const SvxWeightItem& rWeightItem,
                                  const SvxPostureItem& rPostureItem)
{
    rFont.SetFamily(rFontItem.GetFamily());
    rFont.SetFamilyName(rFontItem.GetFamilyName());
    rFont.SetStyleName(rFontItem.GetStyleName());
    rFont.SetCharSet(rFontItem.GetCharSet());
    rFont.SetPitch(rFontItem.GetPitch());
    rFont.SetWeight(rWeightItem.GetValue());
    rFont.SetItalic(rPostureItem.GetValue());
}

#define SETONALLFONTS(MethodName, Value)                                                           \
    rFont.MethodName(Value);                                                                       \
    rCJKFont.MethodName(Value);                                                                    \
    rCTLFont.MethodName(Value);

void AutoFormatPreview::MakeFonts(vcl::RenderContext& rRenderContext, sal_uInt8 nIndex,
                                  vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont)
{
    const SwBoxAutoFormat& rBoxFormat = aCurData.GetBoxFormat(nIndex);

    rFont = rCJKFont = rCTLFont = rRenderContext.GetFont();
    Size aFontSize(rFont.GetFontSize().Width(), 10 * rRenderContext.GetDPIScaleFactor());

    lcl_SetFontProperties(rFont, rBoxFormat.GetFont(), rBoxFormat.GetWeight(),
                          rBoxFormat.GetPosture());
    lcl_SetFontProperties(rCJKFont, rBoxFormat.GetCJKFont(), rBoxFormat.GetCJKWeight(),
                          rBoxFormat.GetCJKPosture());
    lcl_SetFontProperties(rCTLFont, rBoxFormat.GetCTLFont(), rBoxFormat.GetCTLWeight(),
                          rBoxFormat.GetCTLPosture());

    SETONALLFONTS(SetUnderline, rBoxFormat.GetUnderline().GetValue());
    SETONALLFONTS(SetOverline, rBoxFormat.GetOverline().GetValue());
    SETONALLFONTS(SetStrikeout, rBoxFormat.GetCrossedOut().GetValue());
    SETONALLFONTS(SetOutline, rBoxFormat.GetContour().GetValue());
    SETONALLFONTS(SetShadow, rBoxFormat.GetShadowed().GetValue());
    SETONALLFONTS(SetColor, rBoxFormat.GetColor().GetValue());
    SETONALLFONTS(SetFontSize, aFontSize);
    SETONALLFONTS(SetTransparent, true);
}

sal_uInt8 AutoFormatPreview::GetFormatIndex(size_t nCol, size_t nRow) const
{
    static const sal_uInt8 pnFormatMap[]
        = { 0, 1, 2, 1, 3, 4, 5, 6, 5, 7, 8, 9, 10, 9, 11, 4, 5, 6, 5, 7, 12, 13, 14, 13, 15 };
    return pnFormatMap[maArray.GetCellIndex(nCol, nRow, mbRTL)];
}

void AutoFormatPreview::DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow)
{
    // Output of the cell text:
    sal_uLong nNum;
    double nVal;
    OUString cellString;
    sal_uInt8 nIndex = static_cast<sal_uInt8>(maArray.GetCellIndex(nCol, nRow, mbRTL));

    switch (nIndex)
    {
        case 1:
            cellString = aStrJan;
            break;
        case 2:
            cellString = aStrFeb;
            break;
        case 3:
            cellString = aStrMar;
            break;
        case 5:
            cellString = aStrNorth;
            break;
        case 10:
            cellString = aStrMid;
            break;
        case 15:
            cellString = aStrSouth;
            break;
        case 4:
        case 20:
            cellString = aStrSum;
            break;
        case 6:
        case 8:
        case 16:
        case 18:
            nVal = nIndex;
            nNum = 5;
            goto MAKENUMSTR;
        case 17:
        case 7:
            nVal = nIndex;
            nNum = 6;
            goto MAKENUMSTR;
        case 11:
        case 12:
        case 13:
            nVal = nIndex;
            nNum = 12 == nIndex ? 10 : 9;
            goto MAKENUMSTR;
        case 9:
            nVal = 21;
            nNum = 7;
            goto MAKENUMSTR;
        case 14:
            nVal = 36;
            nNum = 11;
            goto MAKENUMSTR;
        case 19:
            nVal = 51;
            nNum = 7;
            goto MAKENUMSTR;
        case 21:
            nVal = 33;
            nNum = 13;
            goto MAKENUMSTR;
        case 22:
            nVal = 36;
            nNum = 14;
            goto MAKENUMSTR;
        case 23:
            nVal = 39;
            nNum = 13;
            goto MAKENUMSTR;
        case 24:
            nVal = 108;
            nNum = 15;
            goto MAKENUMSTR;

        MAKENUMSTR:
            if (aCurData.IsValueFormat())
            {
                OUString sFormat;
                LanguageType eLng, eSys;
                aCurData.GetBoxFormat(sal_uInt8(nNum)).GetValueFormat(sFormat, eLng, eSys);

                SvNumFormatType nType;
                bool bNew;
                sal_Int32 nCheckPos;
                sal_uInt32 nKey = mxNumFormat->GetIndexPuttingAndConverting(sFormat, eLng, eSys,
                                                                            nType, bNew, nCheckPos);
                Color* pDummy;
                mxNumFormat->GetOutputString(nVal, nKey, cellString, &pDummy);
            }
            else
                cellString = OUString::number(sal_Int32(nVal));
            break;
    }

    if (cellString.isEmpty())
        return;

    SvtScriptedTextHelper aScriptedText(rRenderContext);
    Size aStrSize;
    sal_uInt8 nFormatIndex = GetFormatIndex(nCol, nRow);
    const basegfx::B2DRange aCellRange(maArray.GetCellRange(nCol, nRow, true));
    const tools::Rectangle cellRect(
        basegfx::fround(aCellRange.getMinX()), basegfx::fround(aCellRange.getMinY()),
        basegfx::fround(aCellRange.getMaxX()), basegfx::fround(aCellRange.getMaxY()));
    Point aPos = cellRect.TopLeft();
    long nRightX = 0;

    Size theMaxStrSize(cellRect.GetWidth() - FRAME_OFFSET, cellRect.GetHeight() - FRAME_OFFSET);
    if (aCurData.IsFont())
    {
        vcl::Font aFont, aCJKFont, aCTLFont;
        MakeFonts(rRenderContext, nFormatIndex, aFont, aCJKFont, aCTLFont);
        aScriptedText.SetFonts(&aFont, &aCJKFont, &aCTLFont);
    }
    else
        aScriptedText.SetDefaultFont();

    aScriptedText.SetText(cellString, m_xBreak);
    aStrSize = aScriptedText.GetTextSize();

    if (aCurData.IsFont() && theMaxStrSize.Height() < aStrSize.Height())
    {
        // If the string in this font does not
        // fit into the cell, the standard font
        // is taken again:
        aScriptedText.SetDefaultFont();
        aStrSize = aScriptedText.GetTextSize();
    }

    while (theMaxStrSize.Width() <= aStrSize.Width() && cellString.getLength() > 1)
    {
        cellString = cellString.copy(0, cellString.getLength() - 1);
        aScriptedText.SetText(cellString, m_xBreak);
        aStrSize = aScriptedText.GetTextSize();
    }

    nRightX = cellRect.GetWidth() - aStrSize.Width() - FRAME_OFFSET;

    // vertical (always centering):
    aPos.AdjustY((nRowHeight - aStrSize.Height()) / 2);

    // horizontal
    if (mbRTL)
        aPos.AdjustX(nRightX);
    else if (aCurData.IsJustify())
    {
        const SvxAdjustItem& rAdj = aCurData.GetBoxFormat(nFormatIndex).GetAdjust();
        switch (rAdj.GetAdjust())
        {
            case SvxAdjust::Left:
                aPos.AdjustX(FRAME_OFFSET);
                break;
            case SvxAdjust::Right:
                aPos.AdjustX(nRightX);
                break;
            default:
                aPos.AdjustX((cellRect.GetWidth() - aStrSize.Width()) / 2);
                break;
        }
    }
    else
    {
        // Standard align:
        if (nCol == 0 || nIndex == 4)
        {
            // Text-Label left or sum left aligned
            aPos.AdjustX(FRAME_OFFSET);
        }
        else
        {
            // numbers/dates right aligned
            aPos.AdjustX(nRightX);
        }
    }

    aScriptedText.DrawText(aPos);
}

void AutoFormatPreview::DrawBackground(vcl::RenderContext& rRenderContext)
{
    for (size_t nRow = 0; nRow < 5; ++nRow)
    {
        for (size_t nCol = 0; nCol < 5; ++nCol)
        {
            SvxBrushItem aBrushItem(
                aCurData.GetBoxFormat(GetFormatIndex(nCol, nRow)).GetBackground());

            rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(aBrushItem.GetColor());
            const basegfx::B2DRange aCellRange(maArray.GetCellRange(nCol, nRow, true));
            rRenderContext.DrawRect(tools::Rectangle(
                basegfx::fround(aCellRange.getMinX()), basegfx::fround(aCellRange.getMinY()),
                basegfx::fround(aCellRange.getMaxX()), basegfx::fround(aCellRange.getMaxY())));
            rRenderContext.Pop();
        }
    }
}

void AutoFormatPreview::PaintCells(vcl::RenderContext& rRenderContext)
{
    // 1) background
    if (aCurData.IsBackground())
        DrawBackground(rRenderContext);

    // 2) values
    for (size_t nRow = 0; nRow < 5; ++nRow)
        for (size_t nCol = 0; nCol < 5; ++nCol)
            DrawString(rRenderContext, nCol, nRow);

    // 3) border
    if (aCurData.IsFrame())
    {
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
            drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
                rRenderContext, aNewViewInformation2D));

        if (pProcessor2D)
        {
            pProcessor2D->process(maArray.CreateB2DPrimitiveArray());
            pProcessor2D.reset();
        }
    }
}

void AutoFormatPreview::Init()
{
    maArray.Initialize(5, 5);
    nLabelColWidth = 0;
    nDataColWidth1 = 0;
    nDataColWidth2 = 0;
    nRowHeight = 0;
    CalcCellArray(false);
    CalcLineMap();
}

void AutoFormatPreview::CalcCellArray(bool _bFitWidth)
{
    maArray.SetAllColWidths(_bFitWidth ? nDataColWidth2 : nDataColWidth1);
    maArray.SetColWidth(0, nLabelColWidth);
    maArray.SetColWidth(4, nLabelColWidth);

    maArray.SetAllRowHeights(nRowHeight);

    aPrvSize.setWidth(maArray.GetWidth() + 4);
    aPrvSize.setHeight(maArray.GetHeight() + 4);
}

inline void lclSetStyleFromBorder(svx::frame::Style& rStyle,
                                  const ::editeng::SvxBorderLine* pBorder)
{
    rStyle.Set(pBorder, 0.05, 5);
}

void AutoFormatPreview::CalcLineMap()
{
    for (size_t nRow = 0; nRow < 5; ++nRow)
    {
        for (size_t nCol = 0; nCol < 5; ++nCol)
        {
            svx::frame::Style aStyle;

            const SvxBoxItem& rItem = aCurData.GetBoxFormat(GetFormatIndex(nCol, nRow)).GetBox();
            lclSetStyleFromBorder(aStyle, rItem.GetLeft());
            maArray.SetCellStyleLeft(nCol, nRow, aStyle);
            lclSetStyleFromBorder(aStyle, rItem.GetRight());
            maArray.SetCellStyleRight(nCol, nRow, aStyle);
            lclSetStyleFromBorder(aStyle, rItem.GetTop());
            maArray.SetCellStyleTop(nCol, nRow, aStyle);
            lclSetStyleFromBorder(aStyle, rItem.GetBottom());
            maArray.SetCellStyleBottom(nCol, nRow, aStyle);

            // FIXME - uncomment to draw diagonal borders
            //            lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, true ).GetLine() );
            //            maArray.SetCellStyleTLBR( nCol, nRow, aStyle );
            //            lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, false ).GetLine() );
            //            maArray.SetCellStyleBLTR( nCol, nRow, aStyle );
        }
    }
}

void AutoFormatPreview::NotifyChange(const SwTableAutoFormat& rNewData)
{
    aCurData = rNewData;
    bFitWidth = aCurData.IsJustify(); // true;  //???
    CalcCellArray(bFitWidth);
    CalcLineMap();
    mxDrawingArea->queue_draw();
}

IMPL_LINK(AutoFormatPreview, DoPaint, weld::DrawingArea::draw_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = aPayload.first;

    rRenderContext.Push(PushFlags::ALL);

    DrawModeFlags nOldDrawMode = rRenderContext.GetDrawMode();
    if (rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
        rRenderContext.SetDrawMode(DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill
                                   | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient);

    Size theWndSize = rRenderContext.GetOutputSizePixel();

    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetTransparent(true);

    rRenderContext.SetFont(aFont);
    rRenderContext.SetLineColor();
    const Color& rWinColor = rRenderContext.GetSettings().GetStyleSettings().GetWindowColor();
    rRenderContext.SetBackground(Wallpaper(rWinColor));
    rRenderContext.SetFillColor(rWinColor);

    // Draw the Frame
    Color oldColor = rRenderContext.GetLineColor();
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), theWndSize));
    rRenderContext.SetLineColor(oldColor);

    // Center the preview
    maArray.SetXOffset(2 + (theWndSize.Width() - aPrvSize.Width()) / 2);
    maArray.SetYOffset(2 + (theWndSize.Height() - aPrvSize.Height()) / 2);
    // Draw cells on virtual device
    PaintCells(rRenderContext);

    rRenderContext.SetDrawMode(nOldDrawMode);
    rRenderContext.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
