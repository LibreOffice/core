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
#include <vcl/builderfactory.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/swframeexample.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

using namespace ::com::sun::star::text;

#define FLYINFLY_BORDER 3
#define DEMOTEXT        "Ij"

namespace {

void DrawRect_Impl(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect,
                   const Color &rFillColor, const Color &rLineColor)
{
    rRenderContext.SetFillColor(rFillColor);
    rRenderContext.SetLineColor(rLineColor);
    rRenderContext.DrawRect(rRect);
}

}

SwFrameExample::SwFrameExample()
    : nHAlign(HoriOrientation::CENTER)
    , nHRel(RelOrientation::FRAME)
    , nVAlign(VertOrientation::TOP)
    , nVRel(RelOrientation::PRINT_AREA)
    , nWrap(WrapTextMode_NONE)
    , nAnchor(RndStdIds::FLY_AT_PAGE)
    , bTrans(false)
    , aRelPos(Point(0,0))
{
    InitColors_Impl();
}

void SwFrameExample::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 16,
                                   pDrawingArea->get_text_height() * 12);
}

void SwFrameExample::InitColors_Impl()
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_aBgCol = rSettings.GetWindowColor();

    bool bHC = rSettings.GetHighContrastMode();

    m_aFrameColor = COL_LIGHTGREEN;
    m_aAlignColor = COL_LIGHTRED;
    m_aTransColor = COL_TRANSPARENT;

    m_aTxtCol = bHC?
        svtools::ColorConfig().GetColorValue(svtools::FONTCOLOR).nColor :
        COL_GRAY;
    m_aPrintAreaCol = bHC? m_aTxtCol : COL_GRAY;
    m_aBorderCol = m_aTxtCol;
    m_aBlankCol = bHC? m_aTxtCol : COL_LIGHTGRAY;
    m_aBlankFrameCol = bHC? m_aTxtCol : COL_GRAY;
}

void SwFrameExample::StyleUpdated()
{
    InitColors_Impl();
    CustomWidgetController::StyleUpdated();
}

void SwFrameExample::InitAllRects_Impl(vcl::RenderContext& rRenderContext)
{
    aPage.SetSize(GetOutputSizePixel());

    sal_uInt32 nOutWPix = aPage.GetWidth();
    sal_uInt32 nOutHPix = aPage.GetHeight();

    // PrintArea
    sal_uInt32 nLBorder;
    sal_uInt32 nRBorder;
    sal_uInt32 nTBorder;
    sal_uInt32 nBBorder;

    sal_uInt32 nLTxtBorder;
    sal_uInt32 nRTxtBorder;
    sal_uInt32 nTTxtBorder;
    sal_uInt32 nBTxtBorder;

    if (nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        nLBorder = 14;
        nRBorder = 10;
        nTBorder = 10;
        nBBorder = 15;

        nLTxtBorder = 8;
        nRTxtBorder = 4;
        nTTxtBorder = 2;
        nBTxtBorder = 2;
    }
    else
    {
        nLBorder = 2;
        nRBorder = 2;
        nTBorder = 2;
        nBBorder = 2;

        nLTxtBorder = 2;
        nRTxtBorder = 2;
        nTTxtBorder = 2;
        nBTxtBorder = 2;
    }
    aPagePrtArea = tools::Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));

    // Example text: Preparing for the text output
    // A line of text
    aTextLine = aPagePrtArea;
    aTextLine.SetSize(Size(aTextLine.GetWidth(), 2));
    aTextLine.AdjustLeft(nLTxtBorder );
    aTextLine.AdjustRight( -sal_Int32(nRTxtBorder) );
    aTextLine.Move(0, nTTxtBorder);

    // Rectangle to edges including paragraph
    sal_uInt16 nLines = static_cast<sal_uInt16>((aPagePrtArea.GetHeight() / 2 - nTTxtBorder - nBTxtBorder)
             / (aTextLine.GetHeight() + 2));
    aPara = aPagePrtArea;
    aPara.SetSize(Size(aPara.GetWidth(),
        (aTextLine.GetHeight() + 2) * nLines + nTTxtBorder + nBTxtBorder));

    // Rectangle around paragraph without borders
    aParaPrtArea = aPara;
    aParaPrtArea.AdjustLeft(nLTxtBorder );
    aParaPrtArea.AdjustRight( -sal_Int32(nRTxtBorder) );
    aParaPrtArea.AdjustTop(nTTxtBorder );
    aParaPrtArea.AdjustBottom( -sal_Int32(nBTxtBorder) );

    if (nAnchor == RndStdIds::FLY_AS_CHAR || nAnchor == RndStdIds::FLY_AT_CHAR)
    {
        vcl::Font aFont = OutputDevice::GetDefaultFont(
                                DefaultFontType::LATIN_TEXT, Application::GetSettings().GetLanguageTag().getLanguageType(),
                                GetDefaultFontFlags::OnlyOne, &rRenderContext );
        aFont.SetColor( m_aTxtCol );
        aFont.SetFillColor( m_aBgCol );
        aFont.SetWeight(WEIGHT_NORMAL);

        if (nAnchor == RndStdIds::FLY_AS_CHAR)
        {
            aFont.SetFontSize(Size(0, aParaPrtArea.GetHeight() - 2));
            rRenderContext.SetFont(aFont);
            aParaPrtArea.SetSize(Size(rRenderContext.GetTextWidth(DEMOTEXT), rRenderContext.GetTextHeight()));
        }
        else
        {
            aFont.SetFontSize(Size(0, aParaPrtArea.GetHeight() / 2));
            rRenderContext.SetFont(aFont);
            aAutoCharFrame.SetSize(Size(rRenderContext.GetTextWidth(OUString('A')), GetTextHeight()));
            aAutoCharFrame.SetPos(Point(aParaPrtArea.Left() + (aParaPrtArea.GetWidth() - aAutoCharFrame.GetWidth()) / 2,
                aParaPrtArea.Top() + (aParaPrtArea.GetHeight() - aAutoCharFrame.GetHeight()) / 2));
        }
    }

    // Inner Frame anchored at the Frame
    aFrameAtFrame = aPara;
    aFrameAtFrame.AdjustLeft(9 );
    aFrameAtFrame.AdjustRight( -5 );
    aFrameAtFrame.AdjustBottom(5 );
    aFrameAtFrame.SetPos(Point(aFrameAtFrame.Left() + 2, (aPagePrtArea.Bottom() - aFrameAtFrame.GetHeight()) / 2 + 5));

    // Size of the frame to be positioned
    if (nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        sal_uInt32 nLFBorder = nAnchor == RndStdIds::FLY_AT_PAGE ? nLBorder : nLTxtBorder;
        sal_uInt32 nRFBorder = nAnchor == RndStdIds::FLY_AT_PAGE ? nRBorder : nRTxtBorder;

        switch (nHRel)
        {
            case RelOrientation::PAGE_LEFT:
            case RelOrientation::FRAME_LEFT:
                aFrmSize = Size(nLFBorder - 4, (aTextLine.GetHeight() + 2) * 3);
                break;

            case RelOrientation::PAGE_RIGHT:
            case RelOrientation::FRAME_RIGHT:
                aFrmSize = Size(nRFBorder - 4, (aTextLine.GetHeight() + 2) * 3);
                break;

            default:
                aFrmSize = Size(nLBorder - 3, (aTextLine.GetHeight() + 2) * 3);
                break;
        }
        aFrmSize.setWidth( std::max(5L, aFrmSize.Width()) );
        aFrmSize.setHeight( std::max(5L, aFrmSize.Height()) );
    }
    else
    {
        sal_uInt32 nFreeWidth = aPagePrtArea.GetWidth() - rRenderContext.GetTextWidth(DEMOTEXT);

        aFrmSize = Size(nFreeWidth / 2, (aTextLine.GetHeight() + 2) * 3);
        aDrawObj.SetSize(Size(std::max(5L, static_cast<long>(nFreeWidth) / 3L), std::max(5L, aFrmSize.Height() * 3L)));
        aDrawObj.SetPos(Point(aParaPrtArea.Right() + 1, aParaPrtArea.Bottom() / 2));
        aParaPrtArea.SetRight( aDrawObj.Right() );
    }
}

void SwFrameExample::CalcBoundRect_Impl(const vcl::RenderContext& rRenderContext, tools::Rectangle &rRect)
{
    switch (nAnchor)
    {
        case RndStdIds::FLY_AT_PAGE:
        {
            switch (nHRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetLeft( aPage.Left() );
                    rRect.SetRight( aPage.Right() );
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetLeft( aPagePrtArea.Left() );
                    rRect.SetRight( aPagePrtArea.Right() );
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.SetLeft( aPage.Left() );
                    rRect.SetRight( aPagePrtArea.Left() );
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.SetLeft( aPagePrtArea.Right() );
                    rRect.SetRight( aPage.Right() );
                    break;
            }

            switch (nVRel)
            {
                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetTop( aPagePrtArea.Top() );
                    rRect.SetBottom( aPagePrtArea.Bottom() );
                    break;

                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetTop( aPage.Top() );
                    rRect.SetBottom( aPage.Bottom() );
                    break;
            }
        }
        break;

        case RndStdIds::FLY_AT_FLY:
        {
            switch (nHRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetLeft( aFrameAtFrame.Left() );
                    rRect.SetRight( aFrameAtFrame.Right() );
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetLeft( aFrameAtFrame.Left() + FLYINFLY_BORDER );
                    rRect.SetRight( aFrameAtFrame.Right() - FLYINFLY_BORDER );
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.SetLeft( aFrameAtFrame.Left() );
                    rRect.SetRight( aFrameAtFrame.Left() + FLYINFLY_BORDER );
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.SetLeft( aFrameAtFrame.Right() );
                    rRect.SetRight( aFrameAtFrame.Right() - FLYINFLY_BORDER );
                    break;
            }

            switch (nVRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetTop( aFrameAtFrame.Top() );
                    rRect.SetBottom( aFrameAtFrame.Bottom() );
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetTop( aFrameAtFrame.Top() + FLYINFLY_BORDER );
                    rRect.SetBottom( aFrameAtFrame.Bottom() - FLYINFLY_BORDER );
                    break;
            }
        }
        break;
        case RndStdIds::FLY_AT_PARA:
        case RndStdIds::FLY_AT_CHAR:
        {
            switch (nHRel)
            {
                case RelOrientation::FRAME:
                    rRect.SetLeft( aPara.Left() );
                    rRect.SetRight( aPara.Right() );
                    break;

                case RelOrientation::PRINT_AREA:
                    rRect.SetLeft( aParaPrtArea.Left() );
                    rRect.SetRight( aParaPrtArea.Right() );
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.SetLeft( aPage.Left() );
                    rRect.SetRight( aPagePrtArea.Left() );
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.SetLeft( aPagePrtArea.Right() );
                    rRect.SetRight( aPage.Right() );
                    break;

                case RelOrientation::PAGE_FRAME:
                    rRect.SetLeft( aPage.Left() );
                    rRect.SetRight( aPage.Right() );
                    break;

                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetLeft( aPagePrtArea.Left() );
                    rRect.SetRight( aPagePrtArea.Right() );
                    break;

                case RelOrientation::FRAME_LEFT:
                    rRect.SetLeft( aPara.Left() );
                    rRect.SetRight( aParaPrtArea.Left() );
                    break;

                case RelOrientation::FRAME_RIGHT:
                    rRect.SetLeft( aParaPrtArea.Right() );
                    rRect.SetRight( aPara.Right() );
                    break;

                case RelOrientation::CHAR:
                    rRect.SetLeft( aAutoCharFrame.Left() );
                    rRect.SetRight( aAutoCharFrame.Left() );
                    break;
            }

            switch (nVRel)
            {
                case RelOrientation::FRAME:
                    rRect.SetTop( aPara.Top() );
                    rRect.SetBottom( aPara.Bottom() );
                    break;

                case RelOrientation::PRINT_AREA:
                    rRect.SetTop( aParaPrtArea.Top() );
                    rRect.SetBottom( aParaPrtArea.Bottom() );
                    break;

                case RelOrientation::CHAR:
                    if (nVAlign != VertOrientation::NONE &&
                                nVAlign != VertOrientation::CHAR_BOTTOM)
                        rRect.SetTop( aAutoCharFrame.Top() );
                    else
                        rRect.SetTop( aAutoCharFrame.Bottom() );
                    rRect.SetBottom( aAutoCharFrame.Bottom() );
                    break;
                // OD 12.11.2003 #i22341#
                case RelOrientation::TEXT_LINE:
                    rRect.SetTop( aAutoCharFrame.Top() );
                    rRect.SetBottom( aAutoCharFrame.Top() );
                break;
            }
        }
        break;

        case RndStdIds::FLY_AS_CHAR:
            rRect.SetLeft( aParaPrtArea.Left() );
            rRect.SetRight( aParaPrtArea.Right() );

            switch (nVAlign)
            {
                case VertOrientation::NONE:
                case VertOrientation::TOP:
                case VertOrientation::CENTER:
                case VertOrientation::BOTTOM:
                {
                    FontMetric aMetric(rRenderContext.GetFontMetric());

                    rRect.SetTop( aParaPrtArea.Bottom() - aMetric.GetDescent() );
                    rRect.SetBottom( rRect.Top() );
                }
                break;

                default:

                case VertOrientation::LINE_TOP:
                case VertOrientation::LINE_CENTER:
                case VertOrientation::LINE_BOTTOM:
                    rRect.SetTop( aParaPrtArea.Top() );
                    rRect.SetBottom( aDrawObj.Bottom() );
                    break;

                case VertOrientation::CHAR_TOP:
                case VertOrientation::CHAR_CENTER:
                case VertOrientation::CHAR_BOTTOM:
                    rRect.SetTop( aParaPrtArea.Top() );
                    rRect.SetBottom( aParaPrtArea.Bottom() );
                    break;
            }
            break;

        default:
            break;
    }
}

tools::Rectangle SwFrameExample::DrawInnerFrame_Impl(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect,
                                                 const Color &rFillColor, const Color &rBorderColor)
{
    DrawRect_Impl(rRenderContext, rRect, rFillColor, rBorderColor);

    // determine the area relative to which the positioning happens
    tools::Rectangle aRect(rRect); // aPagePrtArea = Default
    CalcBoundRect_Impl(rRenderContext, aRect);

    if (nAnchor == RndStdIds::FLY_AT_FLY && &rRect == &aPagePrtArea)
    {
        // draw text paragraph
        tools::Rectangle aTxt(aTextLine);
        sal_Int32 nStep = aTxt.GetHeight() + 2;
        sal_uInt16 nLines = static_cast<sal_uInt16>(aParaPrtArea.GetHeight() / (aTextLine.GetHeight() + 2));

        for (sal_uInt16 i = 0; i < nLines; i++)
        {
            if (i == nLines - 1)
                aTxt.SetSize(Size(aTxt.GetWidth() / 2, aTxt.GetHeight()));
            DrawRect_Impl(rRenderContext, aTxt, m_aTxtCol, m_aTransColor);
            aTxt.Move(0, nStep);
        }
    }

    return aRect;
}

void SwFrameExample::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));

    InitAllRects_Impl(rRenderContext);

    // Draw page
    DrawRect_Impl(rRenderContext, aPage, m_aBgCol, m_aBorderCol);

    // Draw PrintArea
    tools::Rectangle aRect = DrawInnerFrame_Impl(rRenderContext, aPagePrtArea, m_aTransColor, m_aPrintAreaCol);

    if (nAnchor == RndStdIds::FLY_AT_FLY)
        aRect = DrawInnerFrame_Impl(rRenderContext, aFrameAtFrame, m_aBgCol, m_aBorderCol);

    long lXPos = 0;
    long lYPos = 0;

    // Horizontal alignment
    if (nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        switch (nHAlign)
        {
            case HoriOrientation::RIGHT:
            {
                lXPos = aRect.Right() - aFrmSize.Width() + 1;
                break;
            }
            case HoriOrientation::CENTER:
            {
                lXPos = aRect.Left() + (aRect.GetWidth() - aFrmSize.Width()) / 2;
                break;
            }
            case HoriOrientation::NONE:
            {
                lXPos = aRect.Left() + aRelPos.X();
                break;
            }

            default: // HoriOrientation::LEFT
                lXPos = aRect.Left();
                break;
        }
    }
    else
    {
       lXPos = aRect.Right() + 2;
    }

    // Vertical Alignment
    if (nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        switch (nVAlign)
        {
            case VertOrientation::BOTTOM:
            case VertOrientation::LINE_BOTTOM:
            {
                // #i22341#
                if ( nVRel != RelOrientation::TEXT_LINE )
                {
                    lYPos = aRect.Bottom() - aFrmSize.Height() + 1;
                }
                else
                {
                    lYPos = aRect.Top();
                }
                break;
            }
            case VertOrientation::CENTER:
            case VertOrientation::LINE_CENTER:
            {
                lYPos = aRect.Top() + (aRect.GetHeight() - aFrmSize.Height()) / 2;
                break;
            }
            case VertOrientation::NONE:
            {
                // #i22341#
                if ( nVRel != RelOrientation::CHAR && nVRel != RelOrientation::TEXT_LINE )
                    lYPos = aRect.Top() + aRelPos.Y();
                else
                    lYPos = aRect.Top() - aRelPos.Y();
                break;
            }
            default:
                // #i22341#
                if ( nVRel != RelOrientation::TEXT_LINE )
                {
                    lYPos = aRect.Top();
                }
                else
                {
                    lYPos = aRect.Bottom() - aFrmSize.Height() + 1;
                }
                break;
        }
    }
    else
    {
        switch(nVAlign)
        {
            case VertOrientation::CENTER:
            case VertOrientation::CHAR_CENTER:
            case VertOrientation::LINE_CENTER:
                lYPos = aRect.Top() + (aRect.GetHeight() - aFrmSize.Height()) / 2;
                break;

            case VertOrientation::TOP:
            case VertOrientation::CHAR_BOTTOM:
            case VertOrientation::LINE_BOTTOM:
                lYPos = aRect.Bottom() - aFrmSize.Height() + 1;
                break;

            default:
                lYPos = aRect.Top() - aRelPos.Y();
                break;
        }
    }

    tools::Rectangle aFrmRect(Point(lXPos, lYPos), aFrmSize);

    tools::Rectangle* pOuterFrame = &aPage;

    if (nAnchor == RndStdIds::FLY_AT_FLY)
        pOuterFrame = &aFrameAtFrame;

    if (aFrmRect.Left() < pOuterFrame->Left())
        aFrmRect.Move(pOuterFrame->Left() - aFrmRect.Left(), 0);
    if (aFrmRect.Right() > pOuterFrame->Right())
        aFrmRect.Move(pOuterFrame->Right() - aFrmRect.Right(), 0);

    if (aFrmRect.Top() < pOuterFrame->Top())
        aFrmRect.Move(0, pOuterFrame->Top() - aFrmRect.Top());
    if (aFrmRect.Bottom() > pOuterFrame->Bottom())
        aFrmRect.Move(0, pOuterFrame->Bottom() - aFrmRect.Bottom());

    // Draw Test paragraph
    const long nTxtLineHeight = aTextLine.GetHeight();
    tools::Rectangle aTxt(aTextLine);
    sal_Int32 nStep;
    sal_uInt16 nLines;

    if (nAnchor == RndStdIds::FLY_AT_FLY)
    {
        aTxt.SetLeft( aFrameAtFrame.Left() + FLYINFLY_BORDER );
        aTxt.SetRight( aFrameAtFrame.Right() - FLYINFLY_BORDER );
        aTxt.SetTop( aFrameAtFrame.Top() + FLYINFLY_BORDER );
        aTxt.SetBottom( aTxt.Top() + aTextLine.GetHeight() - 1 );

        nStep = aTxt.GetHeight() + 2;
        nLines = static_cast<sal_uInt16>(((aFrameAtFrame.GetHeight() - 2 * FLYINFLY_BORDER) * 2 / 3)
                 / (aTxt.GetHeight() + 2));
    }
    else
    {
        nStep = aTxt.GetHeight() + 2;
        nLines = static_cast<sal_uInt16>(aParaPrtArea.GetHeight() / (aTextLine.GetHeight() + 2));
    }

    if (nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        // Simulate text
        const long nOldR = aTxt.Right();
        const long nOldL = aTxt.Left();

        // #i22341#
        const bool bIgnoreWrap = nAnchor == RndStdIds::FLY_AT_CHAR &&
                           ( nHRel == RelOrientation::CHAR || nVRel == RelOrientation::CHAR ||
                             nVRel == RelOrientation::TEXT_LINE );

        for (sal_uInt16 i = 0; i < nLines; ++i)
        {
            if (i == (nLines - 1))
                aTxt.SetSize(Size(aTxt.GetWidth() / 2, aTxt.GetHeight()));

            if (aTxt.IsOver(aFrmRect) && nAnchor != RndStdIds::FLY_AS_CHAR && !bIgnoreWrap)
            {
                switch(nWrap)
                {
                    case WrapTextMode_NONE:
                        aTxt.SetTop( aFrmRect.Bottom() + nTxtLineHeight );
                        aTxt.SetBottom( aTxt.Top() + nTxtLineHeight - 1 );
                        break;

                    case WrapTextMode_LEFT:
                        aTxt.SetRight( aFrmRect.Left() );
                        break;

                    case WrapTextMode_RIGHT:
                        aTxt.SetLeft( aFrmRect.Right() );
                        break;
                    default: break;
                }
            }
            if (pOuterFrame->IsInside(aTxt))
                DrawRect_Impl(rRenderContext, aTxt, m_aTxtCol, m_aTransColor );

            aTxt.Move(0, nStep);
            aTxt.SetRight( nOldR );
            aTxt.SetLeft( nOldL );
        }
        aTxt.Move(0, -nStep);

        if (nAnchor != RndStdIds::FLY_AT_FLY && aTxt.Bottom() > aParaPrtArea.Bottom())
        {
            // Text has been replaced by frame, so adjust parameters height
            sal_Int32 nDiff = aTxt.Bottom() - aParaPrtArea.Bottom();
            aParaPrtArea.AdjustBottom(nDiff );
            aPara.AdjustBottom(nDiff );

            CalcBoundRect_Impl(rRenderContext, aRect);

            aParaPrtArea.AdjustBottom( -nDiff );
            aPara.AdjustBottom( -nDiff );
        }
        if (nAnchor == RndStdIds::FLY_AT_CHAR && bIgnoreWrap)
            rRenderContext.DrawText(aAutoCharFrame, OUString('A'));
    }
    else
    {
        rRenderContext.DrawText(aParaPrtArea, OUString(DEMOTEXT));
        DrawRect_Impl(rRenderContext, aDrawObj, m_aBlankCol, m_aBlankFrameCol );
    }

    // Draw rectangle on which the frame is aligned:
    DrawRect_Impl(rRenderContext, aRect, m_aTransColor, m_aAlignColor);

    // Frame View
    bool bDontFill = (nAnchor == RndStdIds::FLY_AT_CHAR && aFrmRect.IsOver(aAutoCharFrame)) || bTrans;
    DrawRect_Impl(rRenderContext, aFrmRect, bDontFill? m_aTransColor : m_aBgCol, m_aFrameColor);
}

void SwFrameExample::SetRelPos(const Point& rP)
{
    aRelPos = rP;

    if (aRelPos.X() > 0)
        aRelPos.setX( 5 );
    if (aRelPos.X() < 0)
        aRelPos.setX( -5 );

    if (aRelPos.Y() > 0)
        aRelPos.setY( 5 );
    if (aRelPos.Y() < 0)
        aRelPos.setY( -5 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
