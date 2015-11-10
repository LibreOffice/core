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

#include "scitems.hxx"
#include <svx/algitem.hxx>
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
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>

#include "sc.hrc"
#include "scmod.hxx"
#include "attrib.hxx"
#include "zforauto.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "autoform.hxx"
#include "miscdlgs.hrc"
#include "autofmt.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"

#define FRAME_OFFSET 4

// ScAutoFmtPreview

ScAutoFmtPreview::ScAutoFmtPreview(vcl::Window* pParent)
    : Window(pParent)
    , pCurData(nullptr)
    , aVD(*this)
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

VCL_BUILDER_FACTORY(ScAutoFmtPreview)

void ScAutoFmtPreview::Resize()
{
    aPrvSize  = Size(GetSizePixel().Width() - 6, GetSizePixel().Height() - 30);
    mnLabelColWidth = (aPrvSize.Width() - 4) / 4 - 12;
    mnDataColWidth1 = (aPrvSize.Width() - 4 - 2 * mnLabelColWidth) / 3;
    mnDataColWidth2 = (aPrvSize.Width() - 4 - 2 * mnLabelColWidth) / 4;
    mnRowHeight = (aPrvSize.Height() - 4) / 5;
    NotifyChange(pCurData);
}

ScAutoFmtPreview::~ScAutoFmtPreview()
{
    disposeOnce();
}

void ScAutoFmtPreview::dispose()
{
    delete pNumFmt;
    vcl::Window::dispose();
}

static void lcl_SetFontProperties(
        vcl::Font& rFont,
        const SvxFontItem& rFontItem,
        const SvxWeightItem& rWeightItem,
        const SvxPostureItem& rPostureItem )
{
    rFont.SetFamily     ( rFontItem.GetFamily() );
    rFont.SetName       ( rFontItem.GetFamilyName() );
    rFont.SetStyleName  ( rFontItem.GetStyleName() );
    rFont.SetCharSet    ( rFontItem.GetCharSet() );
    rFont.SetPitch      ( rFontItem.GetPitch() );
    rFont.SetWeight     ( (FontWeight)rWeightItem.GetValue() );
    rFont.SetItalic     ( (FontItalic)rPostureItem.GetValue() );
}

void ScAutoFmtPreview::MakeFonts( sal_uInt16 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont )
{
    if ( pCurData )
    {
        rFont = rCJKFont = rCTLFont = GetFont();
        Size aFontSize( rFont.GetSize().Width(), 10 * GetDPIScaleFactor() );

        const SvxFontItem*        pFontItem       = static_cast<const SvxFontItem*>      (pCurData->GetItem( nIndex, ATTR_FONT ));
        const SvxWeightItem*      pWeightItem     = static_cast<const SvxWeightItem*>    (pCurData->GetItem( nIndex, ATTR_FONT_WEIGHT ));
        const SvxPostureItem*     pPostureItem    = static_cast<const SvxPostureItem*>   (pCurData->GetItem( nIndex, ATTR_FONT_POSTURE ));
        const SvxFontItem*        pCJKFontItem    = static_cast<const SvxFontItem*>      (pCurData->GetItem( nIndex, ATTR_CJK_FONT ));
        const SvxWeightItem*      pCJKWeightItem  = static_cast<const SvxWeightItem*>    (pCurData->GetItem( nIndex, ATTR_CJK_FONT_WEIGHT ));
        const SvxPostureItem*     pCJKPostureItem = static_cast<const SvxPostureItem*>   (pCurData->GetItem( nIndex, ATTR_CJK_FONT_POSTURE ));
        const SvxFontItem*        pCTLFontItem    = static_cast<const SvxFontItem*>      (pCurData->GetItem( nIndex, ATTR_CTL_FONT ));
        const SvxWeightItem*      pCTLWeightItem  = static_cast<const SvxWeightItem*>    (pCurData->GetItem( nIndex, ATTR_CTL_FONT_WEIGHT ));
        const SvxPostureItem*     pCTLPostureItem = static_cast<const SvxPostureItem*>   (pCurData->GetItem( nIndex, ATTR_CTL_FONT_POSTURE ));
        const SvxUnderlineItem*   pUnderlineItem  = static_cast<const SvxUnderlineItem*> (pCurData->GetItem( nIndex, ATTR_FONT_UNDERLINE ));
        const SvxOverlineItem*    pOverlineItem   = static_cast<const SvxOverlineItem*>  (pCurData->GetItem( nIndex, ATTR_FONT_OVERLINE ));
        const SvxCrossedOutItem*  pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pCurData->GetItem( nIndex, ATTR_FONT_CROSSEDOUT ));
        const SvxContourItem*     pContourItem    = static_cast<const SvxContourItem*>   (pCurData->GetItem( nIndex, ATTR_FONT_CONTOUR ));
        const SvxShadowedItem*    pShadowedItem   = static_cast<const SvxShadowedItem*>  (pCurData->GetItem( nIndex, ATTR_FONT_SHADOWED ));
        const SvxColorItem*       pColorItem      = static_cast<const SvxColorItem*>     (pCurData->GetItem( nIndex, ATTR_FONT_COLOR ));

        lcl_SetFontProperties( rFont, *pFontItem, *pWeightItem, *pPostureItem );
        lcl_SetFontProperties( rCJKFont, *pCJKFontItem, *pCJKWeightItem, *pCJKPostureItem );
        lcl_SetFontProperties( rCTLFont, *pCTLFontItem, *pCTLWeightItem, *pCTLPostureItem );

        Color aColor( pColorItem->GetValue() );
        if( aColor.GetColor() == COL_TRANSPARENT )
            aColor = GetSettings().GetStyleSettings().GetWindowTextColor();

#define SETONALLFONTS( MethodName, Value ) \
rFont.MethodName( Value ); rCJKFont.MethodName( Value ); rCTLFont.MethodName( Value );

        SETONALLFONTS( SetUnderline,    (FontUnderline)pUnderlineItem->GetValue() )
        SETONALLFONTS( SetOverline,     (FontUnderline)pOverlineItem->GetValue() )
        SETONALLFONTS( SetStrikeout,    (FontStrikeout)pCrossedOutItem->GetValue() )
        SETONALLFONTS( SetOutline,      pContourItem->GetValue() )
        SETONALLFONTS( SetShadow,       pShadowedItem->GetValue() )
        SETONALLFONTS( SetColor,        aColor )
        SETONALLFONTS( SetSize,         aFontSize )
        SETONALLFONTS( SetTransparent,  true )

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
    return *static_cast< const SvxBoxItem* >( pCurData->GetItem( GetFormatIndex( nCol, nRow ), ATTR_BORDER ) );
}

const SvxLineItem& ScAutoFmtPreview::GetDiagItem( size_t nCol, size_t nRow, bool bTLBR ) const
{
    OSL_ENSURE( pCurData, "ScAutoFmtPreview::GetDiagItem - no format data found" );
    return *static_cast< const SvxLineItem* >( pCurData->GetItem( GetFormatIndex( nCol, nRow ), bTLBR ? ATTR_BORDER_TLBR : ATTR_BORDER_BLTR ) );
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
    sal_uLong nNum;
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
                ScNumFormatAbbrev& rNumFormat = (ScNumFormatAbbrev&) pCurData->GetNumFormat(sal_uInt16(nNum));
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
        Rectangle cellRect = maArray.GetCellRect( nCol, nRow );
        Point aPos = cellRect.TopLeft();
        sal_uInt16 nRightX = 0;
        bool bJustify = pCurData->GetIncludeJustify();
        SvxHorJustifyItem aHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY );
        SvxCellHorJustify eJustification;

        SvtScriptedTextHelper aScriptedText(rRenderContext);

        // Justification:

        eJustification  = mbRTL ? SVX_HOR_JUSTIFY_RIGHT : bJustify ?
            (SvxCellHorJustify) (static_cast<const SvxHorJustifyItem*>(pCurData->GetItem(nFmtIndex, ATTR_HOR_JUSTIFY))->GetValue()) :
            SVX_HOR_JUSTIFY_STANDARD;

        if (pCurData->GetIncludeFont())
        {
            vcl::Font aFont, aCJKFont, aCTLFont;
            Size theMaxStrSize;

            MakeFonts( nFmtIndex, aFont, aCJKFont, aCTLFont );

            theMaxStrSize           = cellRect.GetSize();
            theMaxStrSize.Width()  -= FRAME_OFFSET;
            theMaxStrSize.Height() -= FRAME_OFFSET;

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
                if( eJustification == SVX_HOR_JUSTIFY_RIGHT )
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

        nRightX  = sal_uInt16(cellRect.GetWidth() - aStrSize.Width() - FRAME_OFFSET);

        // vertical (always center):

        aPos.Y() += (mnRowHeight - (sal_uInt16)aStrSize.Height()) / 2;

        // horizontal

        if (eJustification != SVX_HOR_JUSTIFY_STANDARD)
        {
            sal_uInt16 nHorPos = sal_uInt16((cellRect.GetWidth()-aStrSize.Width()) / 2);

            switch (eJustification)
            {
                case SVX_HOR_JUSTIFY_LEFT:
                    aPos.X() += FRAME_OFFSET;
                    break;
                case SVX_HOR_JUSTIFY_RIGHT:
                    aPos.X() += nRightX;
                    break;
                case SVX_HOR_JUSTIFY_BLOCK:
                case SVX_HOR_JUSTIFY_REPEAT:
                case SVX_HOR_JUSTIFY_CENTER:
                    aPos.X() += nHorPos;
                    break;
                // coverity[dead_error_line] - following conditions exist to avoid compiler warning
                case SVX_HOR_JUSTIFY_STANDARD:
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
                aPos.X() += FRAME_OFFSET;
            }
            else
            {
                 // Numbers/Dates right adjusted
                aPos.X() += nRightX;
            }
        }
        aScriptedText.DrawText(aPos);
    }
}

#undef FRAME_OFFSET

void ScAutoFmtPreview::DrawStrings(vcl::RenderContext& rRenderContext)
{
    for(size_t nRow = 0; nRow < 5; ++nRow)
        for(size_t nCol = 0; nCol < 5; ++nCol)
            DrawString(rRenderContext, nCol, nRow);
}

void ScAutoFmtPreview::DrawBackground(vcl::RenderContext& rRenderContext)
{
    if (pCurData)
    {
        for(size_t nRow = 0; nRow < 5; ++nRow)
        {
            for(size_t nCol = 0; nCol < 5; ++nCol)
            {
                const SvxBrushItem* pItem = static_cast< const SvxBrushItem* >(
                    pCurData->GetItem( GetFormatIndex( nCol, nRow ), ATTR_BACKGROUND ) );

                rRenderContext.Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
                rRenderContext.SetLineColor();
                rRenderContext.SetFillColor( pItem->GetColor() );
                rRenderContext.DrawRect( maArray.GetCellRect( nCol, nRow ) );
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
        DrawStrings(rRenderContext);

        // 3) border
        if (pCurData->GetIncludeFrame())
            maArray.DrawArray(rRenderContext);
    }
}

void ScAutoFmtPreview::Init()
{
    SetBorderStyle( WindowBorderStyle::MONO );
    maArray.Initialize( 5, 5 );
    maArray.SetUseDiagDoubleClipping( false );
    CalcCellArray( false );
    CalcLineMap();
}

void ScAutoFmtPreview::DetectRTL(ScViewData *pViewData)
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

    aPrvSize.Width() = maArray.GetWidth() + 4;
    aPrvSize.Height() = maArray.GetHeight() + 4;
}

inline void lclSetStyleFromBorder( svx::frame::Style& rStyle, const ::editeng::SvxBorderLine* pBorder )
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

    Invalidate(Rectangle(Point(0,0), GetSizePixel()));
}

void ScAutoFmtPreview::DoPaint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    DrawModeFlags nOldDrawMode = aVD->GetDrawMode();

    Size aWndSize(GetSizePixel());
    vcl::Font aFont(aVD->GetFont());
    Color aBackCol(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    Point aTmpPoint;
    Rectangle aRect(aTmpPoint, aWndSize);

    aFont.SetTransparent( true );
    aVD->SetFont(aFont);
    aVD->SetLineColor();
    aVD->SetFillColor(aBackCol);
    aVD->SetOutputSize(aWndSize);
    aVD->DrawRect(aRect);

    PaintCells(*aVD.get());

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(aBackCol);
    rRenderContext.DrawRect(aRect);

    Point aPos((aWndSize.Width() - aPrvSize.Width()) / 2, (aWndSize.Height() - aPrvSize.Height()) / 2);
    if (AllSettings::GetLayoutRTL())
       aPos.X() = -aPos.X();
    rRenderContext.DrawOutDev(aPos, aWndSize, Point(), aWndSize, *aVD.get());
    aVD->SetDrawMode(nOldDrawMode);
}

void ScAutoFmtPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    DoPaint(rRenderContext, rRect);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
