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

/*
 The Table preview in Writer and Calc is almost similar but have different
 implementations 'SvxAutoFmtPreview' replaces those module specific previews
 */

#include <svx/svxtableitems.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <svx/dialog/TableAutoFmtPreview.hxx>
#include <comphelper/processfactory.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <editeng/adjustitem.hxx>
#include <svtools/scriptedtext.hxx>
#include <svx/dialmgr.hxx>
#include <svx/framelink.hxx>
#include <svx/strings.hrc>
#include <vcl/outdev.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/rendercontext/DrawModeFlags.hxx>

using namespace Autoformat;

SvxAutoFmtPreview::SvxAutoFmtPreview(bool bRTL)
    : mpCurrentData(nullptr)
    , mbFitWidth(false)
    , mbRTL(bRTL)
    , maStrJan(SvxResId(RID_SVXSTR_JAN))
    , maStrFeb(SvxResId(RID_SVXSTR_FEB))
    , maStrMar(SvxResId(RID_SVXSTR_MAR))
    , maStrNorth(SvxResId(RID_SVXSTR_NORTH))
    , maStrMid(SvxResId(RID_SVXSTR_MID))
    , maStrSouth(SvxResId(RID_SVXSTR_SOUTH))
    , maStrSum(SvxResId(RID_SVXSTR_SUM))
{
    const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext
        = comphelper::getProcessComponentContext();
    mxBreakIter = com::sun::star::i18n::BreakIterator::create(xContext);
    mxNumFormat.reset(new SvNumberFormatter(xContext, LANGUAGE_SYSTEM));

    Init();
}

void SvxAutoFmtPreview::Resize()
{
    Size aSize = GetOutputSizePixel();
    maPreviousSize = Size(aSize.Width() - 6, aSize.Height() - 30);
    mnLabelColWidth = (maPreviousSize.Width() - 4) / 4 - 12;
    mnDataColWidth1 = (maPreviousSize.Width() - 4 - 2 * mnLabelColWidth) / 3;
    mnDataColWidth2 = (maPreviousSize.Width() - 4 - 2 * mnLabelColWidth) / 4;
    mnRowHeight = (maPreviousSize.Height() - 4) / 5;
    NotifyChange(mpCurrentData);
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

void SvxAutoFmtPreview::MakeFonts(vcl::RenderContext const& rRenderContext, sal_uInt8 nIndex,
                                  vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont)
{
    if (!mpCurrentData)
        return;

    const SvxAutoFormatDataField rData = *mpCurrentData->GetField(nIndex);

    rFont = rCJKFont = rCTLFont = rRenderContext.GetFont();
    Size aFontSize(rFont.GetFontSize().Width(), 10 * rRenderContext.GetDPIScaleFactor());

    lcl_SetFontProperties(rFont, rData.GetFont(), rData.GetWeight(), rData.GetPosture());
    lcl_SetFontProperties(rCJKFont, rData.GetCJKFont(), rData.GetCJKWeight(),
                          rData.GetCJKPosture());
    lcl_SetFontProperties(rCTLFont, rData.GetCTLFont(), rData.GetCTLWeight(),
                          rData.GetCTLPosture());

    SETONALLFONTS(SetUnderline, rData.GetUnderline().GetValue());
    SETONALLFONTS(SetOverline, rData.GetOverline().GetValue());
    SETONALLFONTS(SetStrikeout, rData.GetCrossedOut().GetValue());
    SETONALLFONTS(SetOutline, rData.GetContour().GetValue());
    SETONALLFONTS(SetShadow, rData.GetShadowed().GetValue());
    SETONALLFONTS(SetColor, rData.GetColor().GetValue());
    SETONALLFONTS(SetFontSize, aFontSize);
    SETONALLFONTS(SetTransparent, true);
}

bool SvxAutoFmtPreview::compareTextAttr(int nIndex)
{
    SvxAutoFormatDataField aField = *mpCurrentData->GetField(nIndex);
    SvxAutoFormatDataField aDefault = *mpCurrentData->GetField(9);
    if (aField.GetFont() != aDefault.GetFont() || aField.GetWeight() != aDefault.GetWeight()
        || aField.GetUnderline() != aDefault.GetUnderline()
        || aField.GetPosture() != aDefault.GetPosture() || aField.GetColor() != aDefault.GetColor()
        || aField.GetHeight().GetHeight() != aDefault.GetHeight().GetHeight()
        || aField.GetAdjust() != aDefault.GetAdjust())
        return true;

    return false;
}

sal_uInt8 SvxAutoFmtPreview::GetFormatIndex(size_t nCol, size_t nRow)
{
    bool bFirstRow = nRow == 0;
    bool bLastRow = nRow == 4;
    bool bFirstCol = nCol == 0;
    bool bLastCol = nCol == 4;
    bool bEvenRow = nRow % 2 != 0;
    bool bEvenCol = nCol % 2 != 0;

    if (mpCurrentData->UseLastRowStyles())
    {
        if (bLastRow && bFirstCol && compareTextAttr(LAST_ROW_START_COL))
            return LAST_ROW_START_COL;
        if (bLastRow && bLastCol && compareTextAttr(LAST_ROW_END_COL))
            return LAST_ROW_END_COL;
        if (bLastRow && bEvenCol && compareTextAttr(LAST_ROW_EVEN_COL))
            return LAST_ROW_EVEN_COL;
        if (bLastRow && compareTextAttr(LAST_ROW))
            return LAST_ROW;
    }

    if (mpCurrentData->UseFirstRowStyles())
    {
        if (bFirstRow && bFirstCol && compareTextAttr(FIRST_ROW_START_COL))
            return FIRST_ROW_START_COL;
        if (bFirstRow && bLastCol && compareTextAttr(FIRST_ROW_END_COL))
            return FIRST_ROW_END_COL;
        if (bFirstRow && bEvenCol && compareTextAttr(FIRST_ROW_EVEN_COL))
            return FIRST_ROW_EVEN_COL;
        if (bFirstRow && compareTextAttr(FIRST_ROW))
            return FIRST_ROW;
    }

    if (bFirstCol && compareTextAttr(FIRST_COL) && mpCurrentData->UseFirstColStyles())
        return FIRST_COL;
    if (bLastCol && compareTextAttr(LAST_COL) && mpCurrentData->UseLastColStyles())
        return LAST_COL;

    if (mpCurrentData->UseBandedRowStyles())
    {
        if (bEvenRow && compareTextAttr(EVEN_ROW))
            return EVEN_ROW;
        if (!bEvenRow && compareTextAttr(ODD_ROW))
            return ODD_ROW;
    }
    if (mpCurrentData->UseBandedColStyles())
    {
        if (bEvenCol && compareTextAttr(EVEN_COL))
            return EVEN_COL;
        if (!bEvenCol && compareTextAttr(ODD_COL))
            return ODD_COL;
    }

    if (compareTextAttr(BODY))
        return BODY;

    return BACKGROUND;
}

void SvxAutoFmtPreview::DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow)
{
    if (!mpCurrentData)
        return;

    // Output of the cell text:
    sal_uInt8 nNum;
    double nVal;
    OUString cellString;
    sal_uInt8 nIndex = static_cast<sal_uInt8>(maArray.GetCellIndex(nCol, nRow, mbRTL));

    switch (nIndex)
    {
        case 1:
            cellString = maStrJan;
            break;
        case 2:
            cellString = maStrFeb;
            break;
        case 3:
            cellString = maStrMar;
            break;
        case 5:
            cellString = maStrNorth;
            break;
        case 10:
            cellString = maStrMid;
            break;
        case 15:
            cellString = maStrSouth;
            break;
        case 4:
        case 20:
            cellString = maStrSum;
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
            if (mpCurrentData->IsValueFormat())
            {
                OUString sFormat;
                LanguageType eLng, eSys;
                mpCurrentData->GetField(nNum)->GetValueFormat(sFormat, eLng, eSys);

                SvNumFormatType nType;
                bool bNew;
                sal_Int32 nCheckPos;
                sal_uInt32 nKey = mxNumFormat->GetIndexPuttingAndConverting(sFormat, eLng, eSys,
                                                                            nType, bNew, nCheckPos);
                const Color* pDummy;
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
    const basegfx::B2DRange aCellRange(maArray.GetCellRange(nCol, nRow));
    const tools::Rectangle cellRect(basegfx::fround<tools::Long>(aCellRange.getMinX()),
                                    basegfx::fround<tools::Long>(aCellRange.getMinY()),
                                    basegfx::fround<tools::Long>(aCellRange.getMaxX()),
                                    basegfx::fround<tools::Long>(aCellRange.getMaxY()));
    Point aPos = cellRect.TopLeft();
    tools::Long nRightX = 0;

    Size theMaxStrSize(cellRect.GetWidth() - FRAME_OFFSET, cellRect.GetHeight() - FRAME_OFFSET);
    if (mpCurrentData->IsFont())
    {
        vcl::Font aFont, aCJKFont, aCTLFont;
        MakeFonts(rRenderContext, nFormatIndex, aFont, aCJKFont, aCTLFont);
        aScriptedText.SetFonts(&aFont, &aCJKFont, &aCTLFont);
    }
    else
        aScriptedText.SetDefaultFont();

    aScriptedText.SetText(cellString, mxBreakIter);
    aStrSize = aScriptedText.GetTextSize();

    if (mpCurrentData->IsFont() && theMaxStrSize.Height() < aStrSize.Height())
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
        aScriptedText.SetText(cellString, mxBreakIter);
        aStrSize = aScriptedText.GetTextSize();
    }

    nRightX = cellRect.GetWidth() - aStrSize.Width() - FRAME_OFFSET;

    // vertical (always centering):
    aPos.AdjustY((mnRowHeight - aStrSize.Height()) / 2);

    // horizontal
    if (mbRTL)
        aPos.AdjustX(nRightX);
    else if (mpCurrentData->IsJustify())
    {
        const SvxAdjustItem& rAdj = mpCurrentData->GetField(nFormatIndex)->GetAdjust();
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

static SvxBrushItem lcl_GetBackgroundColor(const SvxAutoFormatData* pData, size_t nCol, size_t nRow)
{
    bool bFirstRow = nRow == 0;
    bool bEvenRow = nRow % 2 != 0;
    bool bLastRow = nRow == 4;
    bool bFirstCol = nCol == 0;
    bool bEvenCol = nCol % 2 != 0;
    bool bLastCol = nCol == 4;

    // Default Background Color
    SvxBrushItem aDefault(0);
    SvxBrushItem aBrushItem(COL_WHITE, SVX_TABLE_BACKGROUND);

    auto hasBackground
        = [&](int index) { return pData->GetField(index)->GetBackground() != aDefault; };
    auto getColor = [&](int index) { return pData->GetField(index)->GetBackground().GetColor(); };

    if (hasBackground(BACKGROUND))
        aBrushItem.SetColor(getColor(BACKGROUND));
    if (hasBackground(BODY))
        aBrushItem.SetColor(getColor(BODY));

    if (pData->UseBandedColStyles())
    {
        if (bEvenCol && hasBackground(EVEN_COL))
            aBrushItem.SetColor(getColor(EVEN_COL));
        if (!bEvenCol && hasBackground(ODD_COL))
            aBrushItem.SetColor(getColor(ODD_COL));
    }

    if (pData->UseBandedRowStyles())
    {
        if (bEvenRow && hasBackground(EVEN_ROW))
            aBrushItem.SetColor(getColor(EVEN_ROW));
        if (!bEvenRow && hasBackground(ODD_ROW))
            aBrushItem.SetColor(getColor(ODD_ROW));
    }

    if (pData->UseLastColStyles())
    {
        if (bLastCol && hasBackground(LAST_COL))
            aBrushItem.SetColor(getColor(LAST_COL));
    }
    if (pData->UseFirstColStyles())
    {
        if (bFirstCol && hasBackground(FIRST_COL))
            aBrushItem.SetColor(getColor(FIRST_COL));
    }
    if (pData->UseLastRowStyles())
    {
        if (bLastRow && hasBackground(LAST_ROW))
            aBrushItem.SetColor(getColor(LAST_ROW));
        if (bLastRow && bEvenCol && hasBackground(LAST_ROW_EVEN_COL))
            aBrushItem.SetColor(getColor(LAST_ROW_EVEN_COL));
        if (bLastRow && bLastCol && hasBackground(LAST_ROW_END_COL))
            aBrushItem.SetColor(getColor(LAST_ROW_END_COL));
        if (bLastRow && bFirstCol && hasBackground(LAST_ROW_START_COL))
            aBrushItem.SetColor(getColor(LAST_ROW_START_COL));
    }
    if (pData->UseFirstRowStyles())
    {
        if (bFirstRow && hasBackground(FIRST_ROW))
            aBrushItem.SetColor(getColor(FIRST_ROW));
        if (bFirstRow && bEvenCol && hasBackground(FIRST_ROW_EVEN_COL))
            aBrushItem.SetColor(getColor(FIRST_ROW_EVEN_COL));
        if (bFirstRow && bLastCol && hasBackground(FIRST_ROW_END_COL))
            aBrushItem.SetColor(getColor(FIRST_ROW_END_COL));
        if (bFirstRow && bFirstCol && hasBackground(FIRST_ROW_START_COL))
            aBrushItem.SetColor(getColor(FIRST_ROW_START_COL));
    }

    return aBrushItem;
}

void SvxAutoFmtPreview::DrawBackground(vcl::RenderContext& rRenderContext)
{
    if (!mpCurrentData)
        return;

    for (size_t nRow = 0; nRow < GRID_SIZE; ++nRow)
    {
        for (size_t nCol = 0; nCol < GRID_SIZE; ++nCol)
        {
            SvxBrushItem aBrushItem = lcl_GetBackgroundColor(mpCurrentData, nCol, nRow);
            rRenderContext.Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(aBrushItem.GetColor());
            const basegfx::B2DRange aCellRange(maArray.GetCellRange(nCol, nRow));
            rRenderContext.DrawRect(
                tools::Rectangle(basegfx::fround<tools::Long>(aCellRange.getMinX()),
                                 basegfx::fround<tools::Long>(aCellRange.getMinY()),
                                 basegfx::fround<tools::Long>(aCellRange.getMaxX()),
                                 basegfx::fround<tools::Long>(aCellRange.getMaxY())));
            rRenderContext.Pop();
        }
    }
}

void SvxAutoFmtPreview::PaintCells(vcl::RenderContext& rRenderContext)
{
    if (!mpCurrentData)
        return;

    // 1) background
    if (mpCurrentData->IsBackground())
        DrawBackground(rRenderContext);

    // 2) values
    for (size_t nRow = 0; nRow < GRID_SIZE; ++nRow)
        for (size_t nCol = 0; nCol < GRID_SIZE; ++nCol)
            DrawString(rRenderContext, nCol, nRow);

    // 3) border
    if (!mpCurrentData->IsFrame())
        return;

    const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
        drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(rRenderContext,
                                                                          aNewViewInformation2D));

    if (pProcessor2D)
    {
        pProcessor2D->process(maArray.CreateB2DPrimitiveArray());
        pProcessor2D.reset();
    }
}

void SvxAutoFmtPreview::Init()
{
    maArray.Initialize(GRID_SIZE, GRID_SIZE);
    mnLabelColWidth = 0;
    mnDataColWidth1 = 0;
    mnDataColWidth2 = 0;
    mnRowHeight = 0;
    CalcCellArray(false);
    CalcLineMap();
}

void SvxAutoFmtPreview::CalcCellArray(bool _bFitWidth)
{
    maArray.SetAllColWidths(_bFitWidth ? mnDataColWidth2 : mnDataColWidth1);
    maArray.SetColWidth(0, mnLabelColWidth);
    maArray.SetColWidth(4, mnLabelColWidth);

    maArray.SetAllRowHeights(mnRowHeight);

    maPreviousSize.setWidth(maArray.GetWidth() + 4);
    maPreviousSize.setHeight(maArray.GetHeight() + 4);
}

static void lclSetStyleFromBorder(svx::frame::Style& rStyle,
                                  const ::editeng::SvxBorderLine* pBorder)
{
    rStyle.Set(pBorder, 0.05, 5);
}

void SvxAutoFmtPreview::setStyleFromBorder(sal_uInt8 nElement, size_t nCol, size_t nRow,
                                           bool reset = false)
{
    svx::frame::Style rStyle;
    SvxBoxItem rItem = mpCurrentData->GetField(nElement)->GetBox();

    if (rItem.GetLeft() || reset)
    {
        lclSetStyleFromBorder(rStyle, rItem.GetLeft());
        maArray.SetCellStyleLeft(nCol, nRow, rStyle);
    }
    if (rItem.GetRight() || reset)
    {
        lclSetStyleFromBorder(rStyle, rItem.GetRight());
        maArray.SetCellStyleRight(nCol, nRow, rStyle);
    }
    if (rItem.GetTop() || reset)
    {
        lclSetStyleFromBorder(rStyle, rItem.GetTop());
        maArray.SetCellStyleTop(nCol, nRow, rStyle);
    }
    if (rItem.GetBottom() || reset)
    {
        lclSetStyleFromBorder(rStyle, rItem.GetBottom());
        maArray.SetCellStyleBottom(nCol, nRow, rStyle);
    }
}

void SvxAutoFmtPreview::setStyleFromBorderPriority(size_t nCol, size_t nRow)
{
    bool bFirstRow = nRow == 0;
    bool bEvenRow = nRow % 2 != 0;
    bool bLastRow = nRow == 4;
    bool bFirstCol = nCol == 0;
    bool bEvenCol = nCol % 2 != 0;
    bool bLastCol = nCol == 4;

    setStyleFromBorder(BACKGROUND, nCol, nRow, true);
    setStyleFromBorder(BODY, nCol, nRow);

    if (mpCurrentData->UseBandedColStyles())
    {
        if (!bEvenCol)
            setStyleFromBorder(ODD_COL, nCol, nRow);
        else
            setStyleFromBorder(EVEN_COL, nCol, nRow);
    }

    if (mpCurrentData->UseBandedRowStyles())
    {
        if (!bEvenRow)
            setStyleFromBorder(ODD_ROW, nCol, nRow);
        else
            setStyleFromBorder(EVEN_ROW, nCol, nRow);
    }

    if (bLastCol && mpCurrentData->UseLastColStyles())
        setStyleFromBorder(LAST_COL, nCol, nRow);
    if (bFirstCol && mpCurrentData->UseFirstColStyles())
        setStyleFromBorder(FIRST_COL, nCol, nRow);

    if (mpCurrentData->UseLastRowStyles())
    {
        if (bLastRow)
            setStyleFromBorder(LAST_ROW, nCol, nRow);
        if (bLastRow && bEvenCol)
            setStyleFromBorder(LAST_ROW_EVEN_COL, nCol, nRow);
        if (bLastRow && bLastCol)
            setStyleFromBorder(LAST_ROW_END_COL, nCol, nRow);
        if (bLastRow && bFirstCol)
            setStyleFromBorder(LAST_ROW_START_COL, nCol, nRow);
    }

    if (mpCurrentData->UseFirstRowStyles())
    {
        if (bFirstRow)
            setStyleFromBorder(FIRST_ROW, nCol, nRow);
        if (bFirstRow && bEvenCol)
            setStyleFromBorder(FIRST_ROW_EVEN_COL, nCol, nRow);
        if (bFirstRow && bLastCol)
            setStyleFromBorder(FIRST_ROW_END_COL, nCol, nRow);
        if (bFirstRow && bFirstCol)
            setStyleFromBorder(FIRST_ROW_START_COL, nCol, nRow);
    }
}

void SvxAutoFmtPreview::CalcLineMap()
{
    if (!mpCurrentData)
        return;

    for (size_t nRow = 0; nRow < GRID_SIZE; ++nRow)
    {
        for (size_t nCol = 0; nCol < GRID_SIZE; ++nCol)
        {
            setStyleFromBorderPriority(nCol, nRow);
        }
    }

    // maArray styles has some default border behaviour below code fixes that
    const SvxBoxItem& rFirstCol = mpCurrentData->GetField(FIRST_COL)->GetBox();
    const SvxBoxItem& rLastCol = mpCurrentData->GetField(LAST_COL)->GetBox();
    const SvxBoxItem& rEvenCol = mpCurrentData->GetField(EVEN_COL)->GetBox();
    const SvxBoxItem& rOddCol = mpCurrentData->GetField(ODD_COL)->GetBox();
    const SvxBoxItem& rFirstRow = mpCurrentData->GetField(FIRST_ROW)->GetBox();
    const SvxBoxItem& rLastRow = mpCurrentData->GetField(LAST_ROW)->GetBox();
    svx::frame::Style aStyle;

    if (mpCurrentData->UseFirstColStyles())
    {
        if (rFirstCol.GetRight())
            maArray.SetColumnStyleLeft(1, aStyle);
    }

    if (mpCurrentData->UseLastColStyles())
    {
        if (rLastCol.GetLeft())
            maArray.SetColumnStyleRight(3, aStyle);
    }

    if (mpCurrentData->UseBandedColStyles())
    {
        if (rEvenCol.GetLeft())
        {
            if (!rFirstCol.GetRight())
                maArray.SetColumnStyleRight(0, aStyle);
            maArray.SetColumnStyleRight(2, aStyle);
        }
        if (rEvenCol.GetRight())
        {
            if (!rLastCol.GetLeft())
                maArray.SetColumnStyleLeft(4, aStyle);
            maArray.SetColumnStyleLeft(2, aStyle);
        }

        if (rOddCol.GetRight() && !rEvenCol.GetLeft())
        {
            maArray.SetColumnStyleLeft(1, aStyle);
            maArray.SetColumnStyleLeft(3, aStyle);
        }
        if (rOddCol.GetLeft() && !rEvenCol.GetRight())
        {
            maArray.SetColumnStyleRight(1, aStyle);
            maArray.SetColumnStyleRight(3, aStyle);
        }
    }

    if (mpCurrentData->UseFirstRowStyles())
    {
        if (rFirstRow.GetBottom())
            maArray.SetRowStyleTop(1, aStyle);

        const SvxBoxItem& rFirstRowStart = mpCurrentData->GetField(FIRST_ROW_START_COL)->GetBox();
        if (rFirstRowStart.GetBottom())
            maArray.SetCellStyleTop(0, 1, aStyle);
        if (rFirstRowStart.GetRight())
            maArray.SetCellStyleLeft(1, 0, aStyle);
        const SvxBoxItem& rFirstRowEnd = mpCurrentData->GetField(FIRST_ROW_END_COL)->GetBox();
        if (rFirstRowEnd.GetBottom())
            maArray.SetCellStyleTop(4, 1, aStyle);
        if (rFirstRowEnd.GetLeft())
            maArray.SetCellStyleRight(3, 0, aStyle);

        const SvxBoxItem& rFirstRowEven = mpCurrentData->GetField(FIRST_ROW_EVEN_COL)->GetBox();
        if (!rFirstRow.GetBottom() && rFirstRowEven.GetBottom())
        {
            maArray.SetCellStyleTop(1, 1, aStyle);
            maArray.SetCellStyleTop(3, 1, aStyle);
        }
        if (rFirstRowEven.GetRight())
        {
            if (!rFirstRowEnd.GetLeft())
                maArray.SetCellStyleLeft(4, 0, aStyle);
            maArray.SetCellStyleLeft(2, 0, aStyle);
        }
        if (rFirstRowEven.GetLeft())
        {
            if (!rFirstRow.GetRight() && !rFirstRowStart.GetRight())
                maArray.SetCellStyleRight(0, 0, aStyle);
            maArray.SetCellStyleRight(2, 0, aStyle);
        }
    }

    if (mpCurrentData->UseLastRowStyles())
    {
        if (rLastRow.GetTop())
            maArray.SetRowStyleBottom(3, aStyle);

        const SvxBoxItem& rLastRowStart = mpCurrentData->GetField(LAST_ROW_START_COL)->GetBox();
        if (rLastRowStart.GetTop())
            maArray.SetCellStyleBottom(0, 3, aStyle);
        if (rLastRowStart.GetRight())
            maArray.SetCellStyleLeft(1, 4, aStyle);
        const SvxBoxItem& rLastRowEnd = mpCurrentData->GetField(LAST_ROW_END_COL)->GetBox();
        if (rLastRowEnd.GetTop())
            maArray.SetCellStyleBottom(4, 3, aStyle);
        if (rLastRowEnd.GetLeft())
            maArray.SetCellStyleRight(3, 4, aStyle);

        const SvxBoxItem& rLastRowEven = mpCurrentData->GetField(LAST_ROW_EVEN_COL)->GetBox();
        if (!rLastRow.GetTop() && rLastRowEven.GetTop())
        {
            maArray.SetCellStyleBottom(1, 3, aStyle);
            maArray.SetCellStyleBottom(3, 3, aStyle);
        }
        if (rLastRowEven.GetRight())
        {
            if (!rLastRowEnd.GetLeft())
                maArray.SetCellStyleLeft(4, 4, aStyle);
            maArray.SetCellStyleLeft(2, 4, aStyle);
        }
        if (rLastRowEven.GetLeft())
        {
            if (!rLastRow.GetRight() && !rLastRowStart.GetRight())
                maArray.SetCellStyleRight(0, 4, aStyle);
            maArray.SetCellStyleRight(2, 4, aStyle);
        }
    }

    if (mpCurrentData->UseBandedRowStyles())
    {
        const SvxBoxItem& rEvenRow = mpCurrentData->GetField(EVEN_ROW)->GetBox();
        if (rEvenRow.GetTop())
        {
            if (!rFirstRow.GetBottom())
                maArray.SetRowStyleBottom(0, aStyle);
            maArray.SetRowStyleBottom(2, aStyle);
        }
        if (rEvenRow.GetBottom())
        {
            if (!rLastRow.GetTop())
                maArray.SetRowStyleTop(4, aStyle);
            maArray.SetRowStyleTop(2, aStyle);
        }

        const SvxBoxItem& rOddRow = mpCurrentData->GetField(ODD_ROW)->GetBox();
        if (!rEvenRow.GetTop() && rOddRow.GetBottom())
        {
            maArray.SetRowStyleTop(1, aStyle);
            maArray.SetRowStyleTop(3, aStyle);
        }
        if (!rEvenRow.GetBottom() && rOddRow.GetTop())
        {
            maArray.SetRowStyleBottom(1, aStyle);
            maArray.SetRowStyleBottom(3, aStyle);
        }
    }
}

void SvxAutoFmtPreview::NotifyChange(const SvxAutoFormatData* rNewData)
{
    if (rNewData)
    {
        mpCurrentData = rNewData->MakeCopy();
        mbFitWidth = mpCurrentData->IsJustify();
    }

    CalcCellArray(mbFitWidth);
    CalcLineMap();
    Invalidate();
}

void SvxAutoFmtPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(vcl::PushFlags::ALL);

    const Color& rWinColor = svtools::ColorConfig().GetColorValue(::svtools::DOCCOLOR).nColor;
    rRenderContext.SetBackground(Wallpaper(rWinColor));
    rRenderContext.Erase();

    DrawModeFlags nOldDrawMode = rRenderContext.GetDrawMode();
    if (rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
        rRenderContext.SetDrawMode(DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill
                                   | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient);

    Size theWndSize = rRenderContext.GetOutputSizePixel();

    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetTransparent(true);
    rRenderContext.SetFont(aFont);

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
