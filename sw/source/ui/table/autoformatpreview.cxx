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
#include <vcl/settings.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>
#include <svtools/scriptedtext.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <strings.hrc>

#include <autoformatpreview.hxx>

#define FRAME_OFFSET 4

AutoFormatPreview::AutoFormatPreview()
    : maCurrentData(OUString())
    , mbFitWidth(false)
    , mbRTL(false)
    , maStringJan(SwResId(STR_JAN))
    , maStringFeb(SwResId(STR_FEB))
    , maStringMar(SwResId(STR_MAR))
    , maStringNorth(SwResId(STR_NORTH))
    , maStringMid(SwResId(STR_MID))
    , maStringSouth(SwResId(STR_SOUTH))
    , maStringSum(SwResId(STR_SUM))
{
    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    m_xBreak = i18n::BreakIterator::create(xContext);
    mxNumFormat.reset(new SvNumberFormatter(xContext, LANGUAGE_SYSTEM));

    Init();
}

void AutoFormatPreview::Resize()
{
    Size aSize = GetOutputSizePixel();
    maPreviousSize = Size(aSize.Width() - 6, aSize.Height() - 30);
    mnLabelColumnWidth = (maPreviousSize.Width() - 4) / 4 - 12;
    mnDataColumnWidth1 = (maPreviousSize.Width() - 4 - 2 * mnLabelColumnWidth) / 3;
    mnDataColumnWidth2 = (maPreviousSize.Width() - 4 - 2 * mnLabelColumnWidth) / 4;
    mnRowHeight = (maPreviousSize.Height() - 4) / 5;
    NotifyChange(maCurrentData);
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

void AutoFormatPreview::MakeFonts(vcl::RenderContext const& rRenderContext, sal_uInt8 nIndex,
                                  vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont)
{
    const SwBoxAutoFormat& rBoxFormat = maCurrentData.GetBoxFormat(nIndex);

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
            cellString = maStringJan;
            break;
        case 2:
            cellString = maStringFeb;
            break;
        case 3:
            cellString = maStringMar;
            break;
        case 5:
            cellString = maStringNorth;
            break;
        case 10:
            cellString = maStringMid;
            break;
        case 15:
            cellString = maStringSouth;
            break;
        case 4:
        case 20:
            cellString = maStringSum;
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
            if (maCurrentData.IsValueFormat())
            {
                OUString sFormat;
                LanguageType eLng, eSys;
                maCurrentData.GetBoxFormat(sal_uInt8(nNum)).GetValueFormat(sFormat, eLng, eSys);

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
    if (maCurrentData.IsFont())
    {
        vcl::Font aFont, aCJKFont, aCTLFont;
        MakeFonts(rRenderContext, nFormatIndex, aFont, aCJKFont, aCTLFont);
        aScriptedText.SetFonts(&aFont, &aCJKFont, &aCTLFont);
    }
    else
        aScriptedText.SetDefaultFont();

    aScriptedText.SetText(cellString, m_xBreak);
    aStrSize = aScriptedText.GetTextSize();

    if (maCurrentData.IsFont() && theMaxStrSize.Height() < aStrSize.Height())
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
    aPos.AdjustY((mnRowHeight - aStrSize.Height()) / 2);

    // horizontal
    if (mbRTL)
        aPos.AdjustX(nRightX);
    else if (maCurrentData.IsJustify())
    {
        const SvxAdjustItem& rAdj = maCurrentData.GetBoxFormat(nFormatIndex).GetAdjust();
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
                maCurrentData.GetBoxFormat(GetFormatIndex(nCol, nRow)).GetBackground());

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
    if (maCurrentData.IsBackground())
        DrawBackground(rRenderContext);

    // 2) values
    for (size_t nRow = 0; nRow < 5; ++nRow)
        for (size_t nCol = 0; nCol < 5; ++nCol)
            DrawString(rRenderContext, nCol, nRow);

    // 3) border
    if (maCurrentData.IsFrame())
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
    mnLabelColumnWidth = 0;
    mnDataColumnWidth1 = 0;
    mnDataColumnWidth2 = 0;
    mnRowHeight = 0;
    CalcCellArray(false);
    CalcLineMap();
}

void AutoFormatPreview::CalcCellArray(bool _bFitWidth)
{
    maArray.SetAllColWidths(_bFitWidth ? mnDataColumnWidth2 : mnDataColumnWidth1);
    maArray.SetColWidth(0, mnLabelColumnWidth);
    maArray.SetColWidth(4, mnLabelColumnWidth);

    maArray.SetAllRowHeights(mnRowHeight);

    maPreviousSize.setWidth(maArray.GetWidth() + 4);
    maPreviousSize.setHeight(maArray.GetHeight() + 4);
}

static void lclSetStyleFromBorder(svx::frame::Style& rStyle,
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

            const SvxBoxItem& rItem
                = maCurrentData.GetBoxFormat(GetFormatIndex(nCol, nRow)).GetBox();
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
    maCurrentData = rNewData;
    mbFitWidth = maCurrentData.IsJustify(); // true;  //???
    CalcCellArray(mbFitWidth);
    CalcLineMap();
    Invalidate();
}

void AutoFormatPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
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
    maArray.SetXOffset(2 + (theWndSize.Width() - maPreviousSize.Width()) / 2);
    maArray.SetYOffset(2 + (theWndSize.Height() - maPreviousSize.Height()) / 2);
    // Draw cells on virtual device
    PaintCells(rRenderContext);

    rRenderContext.SetDrawMode(nOldDrawMode);
    rRenderContext.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
