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

#include <scitems.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/zforlist.hxx>
#include <svtools/scriptedtext.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/processfactory.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

#include <strings.hrc>
#include <zforauto.hxx>
#include <global.hxx>
#include <autoform.hxx>
#include <autofmt.hxx>
#include <scresid.hxx>
#include <document.hxx>
#include <viewdata.hxx>

#define FRAME_OFFSET 4

// ScAutoFmtPreview

ScAutoFmtPreview::ScAutoFmtPreview()
    : pCurData(nullptr)
    , bFitWidth(false)
    , mbRTL(false)
    , aStrJan(ScResId(STR_JAN))
    , aStrFeb(ScResId(STR_FEB))
    , aStrMar(ScResId(STR_MAR))
    , aStrNorth(ScResId(STR_NORTH))
    , aStrMid(ScResId(STR_MID))
    , aStrSouth(ScResId(STR_SOUTH))
    , aStrSum(ScResId(STR_SUM))
    , pNumFmt(new SvNumberFormatter(::comphelper::getProcessComponentContext(), ScGlobal::eLnge))
{
    Init();
}

void ScAutoFmtPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    aVD.disposeAndReset(VclPtr<VirtualDevice>::Create(pDrawingArea->get_ref_device()));
    CustomWidgetController::SetDrawingArea(pDrawingArea);
}

void ScAutoFmtPreview::Resize()
{
    Size aSize(GetOutputSizePixel());
    aPrvSize  = Size(aSize.Width() - 6, aSize.Height() - 30);
    mnLabelColWidth = (aPrvSize.Width() - 4) / 4 - 12;
    mnDataColWidth1 = (aPrvSize.Width() - 4 - 2 * mnLabelColWidth) / 3;
    mnDataColWidth2 = (aPrvSize.Width() - 4 - 2 * mnLabelColWidth) / 4;
    mnRowHeight = (aPrvSize.Height() - 4) / 5;
    NotifyChange(pCurData);
}

ScAutoFmtPreview::~ScAutoFmtPreview()
{
}

static void lcl_SetFontProperties(
        vcl::Font& rFont,
        const SvxFontItem& rFontItem,
        const SvxWeightItem& rWeightItem,
        const SvxPostureItem& rPostureItem )
{
    rFont.SetFamily     ( rFontItem.GetFamily() );
    rFont.SetFamilyName ( rFontItem.GetFamilyName() );
    rFont.SetStyleName  ( rFontItem.GetStyleName() );
    rFont.SetCharSet    ( rFontItem.GetCharSet() );
    rFont.SetPitch      ( rFontItem.GetPitch() );
    rFont.SetWeight     ( rWeightItem.GetValue() );
    rFont.SetItalic     ( rPostureItem.GetValue() );
}

void ScAutoFmtPreview::MakeFonts(vcl::RenderContext const& rRenderContext, sal_uInt16 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont)
{
    if ( pCurData )
    {
        rFont = rCJKFont = rCTLFont = rRenderContext.GetFont();
        Size aFontSize(rFont.GetFontSize().Width(), 10 * rRenderContext.GetDPIScaleFactor());

        const SvxFontItem*        pFontItem       = pCurData->GetItem( nIndex, ATTR_FONT );
        const SvxWeightItem*      pWeightItem     = pCurData->GetItem( nIndex, ATTR_FONT_WEIGHT );
        const SvxPostureItem*     pPostureItem    = pCurData->GetItem( nIndex, ATTR_FONT_POSTURE );
        const SvxFontItem*        pCJKFontItem    = pCurData->GetItem( nIndex, ATTR_CJK_FONT );
        const SvxWeightItem*      pCJKWeightItem  = pCurData->GetItem( nIndex, ATTR_CJK_FONT_WEIGHT );
        const SvxPostureItem*     pCJKPostureItem = pCurData->GetItem( nIndex, ATTR_CJK_FONT_POSTURE );
        const SvxFontItem*        pCTLFontItem    = pCurData->GetItem( nIndex, ATTR_CTL_FONT );
        const SvxWeightItem*      pCTLWeightItem  = pCurData->GetItem( nIndex, ATTR_CTL_FONT_WEIGHT );
        const SvxPostureItem*     pCTLPostureItem = pCurData->GetItem( nIndex, ATTR_CTL_FONT_POSTURE );
        const SvxUnderlineItem*   pUnderlineItem  = pCurData->GetItem( nIndex, ATTR_FONT_UNDERLINE );
        const SvxOverlineItem*    pOverlineItem   = pCurData->GetItem( nIndex, ATTR_FONT_OVERLINE );
        const SvxCrossedOutItem*  pCrossedOutItem = pCurData->GetItem( nIndex, ATTR_FONT_CROSSEDOUT );
        const SvxContourItem*     pContourItem    = pCurData->GetItem( nIndex, ATTR_FONT_CONTOUR );
        const SvxShadowedItem*    pShadowedItem   = pCurData->GetItem( nIndex, ATTR_FONT_SHADOWED );
        const SvxColorItem*       pColorItem      = pCurData->GetItem( nIndex, ATTR_FONT_COLOR );

        lcl_SetFontProperties( rFont, *pFontItem, *pWeightItem, *pPostureItem );
        lcl_SetFontProperties( rCJKFont, *pCJKFontItem, *pCJKWeightItem, *pCJKPostureItem );
        lcl_SetFontProperties( rCTLFont, *pCTLFontItem, *pCTLWeightItem, *pCTLPostureItem );

        Color aColor( pColorItem->GetValue() );
        if( aColor == COL_TRANSPARENT )
            aColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();

#define SETONALLFONTS( MethodName, Value ) \
rFont.MethodName( Value ); rCJKFont.MethodName( Value ); rCTLFont.MethodName( Value );

        SETONALLFONTS( SetUnderline,        pUnderlineItem->GetValue() )
        SETONALLFONTS( SetOverline,         pOverlineItem->GetValue() )
        SETONALLFONTS( SetStrikeout,        pCrossedOutItem->GetValue() )
        SETONALLFONTS( SetOutline,          pContourItem->GetValue() )
        SETONALLFONTS( SetShadow,           pShadowedItem->GetValue() )
        SETONALLFONTS( SetColor,            aColor )
        SETONALLFONTS( SetFontSize,  aFontSize )
        SETONALLFONTS( SetTransparent,      true )

#undef SETONALLFONTS
    }
}

sal_uInt16 ScAutoFmtPreview::GetFormatIndex( size_t nCol, size_t nRow ) const
{
    static const sal_uInt16 pnFmtMap[] =
    {
        0,  1,  2,  1,  3,
        4,  5,  6,  5,  7,
        8,  9,  10, 9,  11,
        4,  5,  6,  5,  7,
        12, 13, 14, 13, 15
    };
    return pnFmtMap[ maArray.GetCellIndex( nCol, nRow, mbRTL ) ];
}

const SvxBoxItem& ScAutoFmtPreview::GetBoxItem( size_t nCol, size_t nRow ) const
{
    OSL_ENSURE( pCurData, "ScAutoFmtPreview::GetBoxItem - no format data found" );
    return * pCurData->GetItem( GetFormatIndex( nCol, nRow ), ATTR_BORDER );
}

const SvxLineItem& ScAutoFmtPreview::GetDiagItem( size_t nCol, size_t nRow, bool bTLBR ) const
{
    OSL_ENSURE( pCurData, "ScAutoFmtPreview::GetDiagItem - no format data found" );
    return * pCurData->GetItem( GetFormatIndex( nCol, nRow ), bTLBR ? ATTR_BORDER_TLBR : ATTR_BORDER_BLTR );
}

void ScAutoFmtPreview::DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow)
{
    if (!pCurData)
    {
        return;
    }

    // Emit the cell text

    OUString cellString;
    bool bNumFormat = pCurData->GetIncludeValueFormat();
    sal_uInt32 nNum;
    double nVal;
    Color* pDummy = nullptr;
    sal_uInt16 nIndex = static_cast<sal_uInt16>(maArray.GetCellIndex(nCol, nRow, mbRTL));

    switch (nIndex)
    {
        case  1: cellString = aStrJan;          break;
        case  2: cellString = aStrFeb;          break;
        case  3: cellString = aStrMar;          break;
        case  5: cellString = aStrNorth;        break;
        case 10: cellString = aStrMid;          break;
        case 15: cellString = aStrSouth;        break;
        case  4:
        case 20: cellString = aStrSum;          break;

        case  6:
        case  8:
        case 16:
        case 18: nVal = nIndex;
                 nNum = 5;
                 goto mknum;
        case 17:
        case  7: nVal = nIndex;
                 nNum = 6;
                 goto mknum;
        case 11:
        case 12:
        case 13: nVal = nIndex;
                 nNum = 12 == nIndex ? 10 : 9;
                 goto mknum;

        case  9: nVal = 21; nNum = 7; goto mknum;
        case 14: nVal = 36; nNum = 11; goto mknum;
        case 19: nVal = 51; nNum = 7; goto mknum;
        case 21: nVal = 33; nNum = 13; goto mknum;
        case 22: nVal = 36; nNum = 14; goto mknum;
        case 23: nVal = 39; nNum = 13; goto mknum;
        case 24: nVal = 108; nNum = 15;
        mknum:
            if (bNumFormat)
            {
                ScNumFormatAbbrev& rNumFormat = const_cast<ScNumFormatAbbrev&>(pCurData->GetNumFormat(sal_uInt16(nNum)));
                nNum = rNumFormat.GetFormatIndex(*pNumFmt);
            }
            else
                nNum = 0;
            pNumFmt->GetOutputString(nVal, nNum, cellString, &pDummy);
            break;
    }

    if (!cellString.isEmpty())
    {

        Size aStrSize;
        sal_uInt16 nFmtIndex = GetFormatIndex( nCol, nRow );
        const basegfx::B2DRange cellRange(maArray.GetCellRange( nCol, nRow, true ));
        Point aPos = Point(basegfx::fround(cellRange.getMinX()), basegfx::fround(cellRange.getMinY()));
        sal_uInt16 nRightX = 0;
        bool bJustify = pCurData->GetIncludeJustify();
        SvxCellHorJustify eJustification;

        SvtScriptedTextHelper aScriptedText(rRenderContext);

        // Justification:

        eJustification  = mbRTL ? SvxCellHorJustify::Right : bJustify ?
            pCurData->GetItem(nFmtIndex, ATTR_HOR_JUSTIFY)->GetValue() :
            SvxCellHorJustify::Standard;

        if (pCurData->GetIncludeFont())
        {
            vcl::Font aFont, aCJKFont, aCTLFont;
            Size theMaxStrSize;

            MakeFonts(rRenderContext, nFmtIndex, aFont, aCJKFont, aCTLFont);

            theMaxStrSize = Size(basegfx::fround(cellRange.getWidth()), basegfx::fround(cellRange.getHeight()));
            theMaxStrSize.AdjustWidth( -(FRAME_OFFSET) );
            theMaxStrSize.AdjustHeight( -(FRAME_OFFSET) );

            aScriptedText.SetFonts( &aFont, &aCJKFont, &aCTLFont );
            aScriptedText.SetText(cellString, xBreakIter);
            aStrSize = aScriptedText.GetTextSize();

            if (theMaxStrSize.Height() < aStrSize.Height())
            {
                // if the string does not fit in the row using this font,
                // the default font is used
                aScriptedText.SetDefaultFont();
                aStrSize = aScriptedText.GetTextSize();
            }
            while((theMaxStrSize.Width() <= aStrSize.Width()) && (cellString.getLength() > 1))
            {
                if( eJustification == SvxCellHorJustify::Right )
                    cellString = cellString.copy(1);
                else
                    cellString = cellString.copy(0, cellString.getLength() - 1 );

                aScriptedText.SetText( cellString, xBreakIter );
                aStrSize = aScriptedText.GetTextSize();
            }
        }
        else
        {
            aScriptedText.SetDefaultFont();
            aScriptedText.SetText( cellString, xBreakIter );
            aStrSize = aScriptedText.GetTextSize();
        }

        nRightX  = sal_uInt16(basegfx::fround(cellRange.getWidth()) - aStrSize.Width() - FRAME_OFFSET);

        // vertical (always center):

        aPos.AdjustY((mnRowHeight - static_cast<sal_uInt16>(aStrSize.Height())) / 2 );

        // horizontal

        if (eJustification != SvxCellHorJustify::Standard)
        {
            sal_uInt16 nHorPos = sal_uInt16((basegfx::fround(cellRange.getWidth())-aStrSize.Width()) / 2);
            //sal_uInt16 nHorPos = sal_uInt16((basegfx::fround(cellRange.getWidth())-aStrSize.Width()) / 2);

            switch (eJustification)
            {
                case SvxCellHorJustify::Left:
                    aPos.AdjustX(FRAME_OFFSET );
                    break;
                case SvxCellHorJustify::Right:
                    aPos.AdjustX(nRightX );
                    break;
                case SvxCellHorJustify::Block:
                case SvxCellHorJustify::Repeat:
                case SvxCellHorJustify::Center:
                    aPos.AdjustX(nHorPos );
                    break;
                // coverity[dead_error_line] - following conditions exist to avoid compiler warning
                case SvxCellHorJustify::Standard:
                default:
                    // Standard is not handled here
                    break;
            }
        }
        else
        {

            // Standard justification

            if (nCol == 0 || nRow == 0)
            {
                // Text label to the left or sum left adjusted
                aPos.AdjustX(FRAME_OFFSET );
            }
            else
            {
                 // Numbers/Dates right adjusted
                aPos.AdjustX(nRightX );
            }
        }
        aScriptedText.DrawText(aPos);
    }
}

#undef FRAME_OFFSET

void ScAutoFmtPreview::DrawBackground(vcl::RenderContext& rRenderContext)
{
    if (pCurData)
    {
        for(size_t nRow = 0; nRow < 5; ++nRow)
        {
            for(size_t nCol = 0; nCol < 5; ++nCol)
            {
                const SvxBrushItem* pItem =
                    pCurData->GetItem( GetFormatIndex( nCol, nRow ), ATTR_BACKGROUND );

                rRenderContext.Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
                rRenderContext.SetLineColor();
                rRenderContext.SetFillColor( pItem->GetColor() );

                const basegfx::B2DRange aCellRange(maArray.GetCellRange( nCol, nRow, true ));
                rRenderContext.DrawRect(
                    tools::Rectangle(
                        basegfx::fround(aCellRange.getMinX()), basegfx::fround(aCellRange.getMinY()),
                        basegfx::fround(aCellRange.getMaxX()), basegfx::fround(aCellRange.getMaxY())));

                rRenderContext.Pop();
            }
        }
    }
}

void ScAutoFmtPreview::PaintCells(vcl::RenderContext& rRenderContext)
{
    if (pCurData)
    {
        // 1) background
        if (pCurData->GetIncludeBackground())
            DrawBackground(rRenderContext);

        // 2) values
        for(size_t nRow = 0; nRow < 5; ++nRow)
            for(size_t nCol = 0; nCol < 5; ++nCol)
                DrawString(rRenderContext, nCol, nRow);

        // 3) border
        if (pCurData->GetIncludeFrame())
        {
            const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
            std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
                drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
                    rRenderContext,
                    aNewViewInformation2D));

            if (pProcessor2D)
            {
                pProcessor2D->process(maArray.CreateB2DPrimitiveArray());
                pProcessor2D.reset();
            }
        }
    }
}

void ScAutoFmtPreview::Init()
{
    maArray.Initialize( 5, 5 );
    mnLabelColWidth = 0;
    mnDataColWidth1 = 0;
    mnDataColWidth2 = 0;
    mnRowHeight = 0;
    CalcCellArray( false );
    CalcLineMap();
}

void ScAutoFmtPreview::DetectRTL(const ScViewData *pViewData)
{
    SCTAB nCurrentTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    mbRTL = pDoc->IsLayoutRTL(nCurrentTab);
    xBreakIter = pDoc->GetBreakIterator();
}

void ScAutoFmtPreview::CalcCellArray( bool bFitWidthP )
{
    maArray.SetXOffset( 2 );
    maArray.SetAllColWidths( bFitWidthP ? mnDataColWidth2 : mnDataColWidth1 );
    maArray.SetColWidth( 0, mnLabelColWidth );
    maArray.SetColWidth( 4, mnLabelColWidth );

    maArray.SetYOffset( 2 );
    maArray.SetAllRowHeights( mnRowHeight );

    aPrvSize.setWidth( maArray.GetWidth() + 4 );
    aPrvSize.setHeight( maArray.GetHeight() + 4 );
}

static void lclSetStyleFromBorder( svx::frame::Style& rStyle, const ::editeng::SvxBorderLine* pBorder )
{
    rStyle.Set( pBorder, 1.0 / TWIPS_PER_POINT, 5 );
}

void ScAutoFmtPreview::CalcLineMap()
{
    if ( pCurData )
    {
        for( size_t nRow = 0; nRow < 5; ++nRow )
        {
            for( size_t nCol = 0; nCol < 5; ++nCol )
            {
                svx::frame::Style aStyle;

                const SvxBoxItem& rItem = GetBoxItem( nCol, nRow );
                lclSetStyleFromBorder( aStyle, rItem.GetLeft() );
                maArray.SetCellStyleLeft( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, rItem.GetRight() );
                maArray.SetCellStyleRight( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, rItem.GetTop() );
                maArray.SetCellStyleTop( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, rItem.GetBottom() );
                maArray.SetCellStyleBottom( nCol, nRow, aStyle );

                lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, true ).GetLine() );
                maArray.SetCellStyleTLBR( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, false ).GetLine() );
                maArray.SetCellStyleBLTR( nCol, nRow, aStyle );
            }
        }
    }
}

void ScAutoFmtPreview::NotifyChange( ScAutoFormatData* pNewData )
{
    if (pNewData)
    {
        pCurData = pNewData;
        bFitWidth = pNewData->GetIncludeWidthHeight();
    }

    CalcCellArray( bFitWidth );
    CalcLineMap();

    Invalidate();
}

void ScAutoFmtPreview::DoPaint(vcl::RenderContext& rRenderContext)
{
    rRenderContext.Push(PushFlags::ALL);
    DrawModeFlags nOldDrawMode = aVD->GetDrawMode();

    Size aWndSize(GetOutputSizePixel());
    vcl::Font aFont(aVD->GetFont());
    Color aBackCol(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    tools::Rectangle aRect(Point(), aWndSize);

    aFont.SetTransparent( true );
    aVD->SetFont(aFont);
    aVD->SetLineColor();
    aVD->SetFillColor(aBackCol);
    aVD->SetOutputSize(aWndSize);
    aVD->DrawRect(aRect);

    PaintCells(*aVD);

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(aBackCol);
    rRenderContext.DrawRect(aRect);

    Point aPos((aWndSize.Width() - aPrvSize.Width()) / 2, (aWndSize.Height() - aPrvSize.Height()) / 2);
    if (AllSettings::GetLayoutRTL())
       aPos.setX( -aPos.X() );
    rRenderContext.DrawOutDev(aPos, aWndSize, Point(), aWndSize, *aVD);
    aVD->SetDrawMode(nOldDrawMode);
    rRenderContext.Pop();
}

void ScAutoFmtPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    DoPaint(rRenderContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
