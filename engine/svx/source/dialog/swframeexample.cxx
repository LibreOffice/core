/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
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
constexpr OUString DEMOTEXT = u"Ij"_ustr;

namespace {

void DrawRect_Impl(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect,
                   std::optional<Color> oFillColor, std::optional<Color> oLineColor)
{
    if (oFillColor)
        rRenderContext.SetFillColor(*oFillColor);
    else
        rRenderContext.SetFillColor();
    if (oLineColor)
        rRenderContext.SetLineColor(*oLineColor);
    else
        rRenderContext.SetLineColor();
    rRenderContext.DrawRect(rRect);
}

}

SwFrameExample::SwFrameExample()
    : m_nHAlign(HoriOrientation::CENTER)
    , m_nHRel(RelOrientation::FRAME)
    , m_nVAlign(VertOrientation::TOP)
    , m_nVRel(RelOrientation::PRINT_AREA)
    , m_nWrap(WrapTextMode_NONE)
    , m_nAnchor(RndStdIds::FLY_AT_PAGE)
    , m_bTrans(false)
    , m_aRelPos(Point(0,0))
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
    m_aPage.SetSize(GetOutputSizePixel());

    sal_uInt32 nOutWPix = m_aPage.GetWidth();
    if (!nOutWPix)
    {
        nOutWPix = 1;
    }
    sal_uInt32 nOutHPix = m_aPage.GetHeight();
    if (!nOutHPix)
    {
        nOutHPix = 1;
    }

    // PrintArea
    sal_uInt32 nLBorder;
    sal_uInt32 nRBorder;
    sal_uInt32 nTBorder;
    sal_uInt32 nBBorder;

    sal_uInt32 nLTxtBorder;
    sal_uInt32 nRTxtBorder;
    sal_uInt32 nTTxtBorder;
    sal_uInt32 nBTxtBorder;

    if (m_nAnchor != RndStdIds::FLY_AS_CHAR)
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
    m_aPagePrtArea = tools::Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));

    // Example text: Preparing for the text output
    // A line of text
    m_aTextLine = m_aPagePrtArea;
    m_aTextLine.SetSize(Size(m_aTextLine.GetWidth(), 2));
    m_aTextLine.AdjustLeft(nLTxtBorder );
    m_aTextLine.AdjustRight( -sal_Int32(nRTxtBorder) );
    m_aTextLine.Move(0, nTTxtBorder);

    // Rectangle to edges including paragraph
    sal_uInt16 nLines = static_cast<sal_uInt16>((m_aPagePrtArea.GetHeight() / 2 - nTTxtBorder - nBTxtBorder)
             / (m_aTextLine.GetHeight() + 2));
    m_aPara = m_aPagePrtArea;
    m_aPara.SetSize(Size(m_aPara.GetWidth(),
        (m_aTextLine.GetHeight() + 2) * nLines + nTTxtBorder + nBTxtBorder));

    // Rectangle around paragraph without borders
    m_aParaPrtArea = m_aPara;
    m_aParaPrtArea.AdjustLeft(nLTxtBorder );
    m_aParaPrtArea.AdjustRight( -sal_Int32(nRTxtBorder) );
    m_aParaPrtArea.AdjustTop(nTTxtBorder );
    m_aParaPrtArea.AdjustBottom( -sal_Int32(nBTxtBorder) );

    if (m_nAnchor == RndStdIds::FLY_AS_CHAR || m_nAnchor == RndStdIds::FLY_AT_CHAR)
    {
        vcl::Font aFont = OutputDevice::GetDefaultFont(
                                DefaultFontType::LATIN_TEXT, Application::GetSettings().GetLanguageTag().getLanguageType(),
                                GetDefaultFontFlags::OnlyOne, &rRenderContext );
        aFont.SetColor( m_aTxtCol );
        aFont.SetFillColor( m_aBgCol );
        aFont.SetWeight(WEIGHT_NORMAL);

        if (m_nAnchor == RndStdIds::FLY_AS_CHAR)
        {
            aFont.SetFontSize(Size(0, m_aParaPrtArea.GetHeight() - 2));
            rRenderContext.SetFont(aFont);
            m_aParaPrtArea.SetSize(Size(rRenderContext.GetTextWidth(DEMOTEXT), rRenderContext.GetTextHeight()));
        }
        else
        {
            aFont.SetFontSize(Size(0, m_aParaPrtArea.GetHeight() / 2));
            rRenderContext.SetFont(aFont);
            m_aAutoCharFrame.SetSize(Size(rRenderContext.GetTextWidth(OUString('A')), GetTextHeight()));
            m_aAutoCharFrame.SetPos(Point(m_aParaPrtArea.Left() + (m_aParaPrtArea.GetWidth() - m_aAutoCharFrame.GetWidth()) / 2,
                m_aParaPrtArea.Top() + (m_aParaPrtArea.GetHeight() - m_aAutoCharFrame.GetHeight()) / 2));
        }
    }

    // Inner Frame anchored at the Frame
    m_aFrameAtFrame = m_aPara;
    m_aFrameAtFrame.AdjustLeft(9 );
    m_aFrameAtFrame.AdjustRight( -5 );
    m_aFrameAtFrame.AdjustBottom(5 );
    m_aFrameAtFrame.SetPos(Point(m_aFrameAtFrame.Left() + 2, (m_aPagePrtArea.Bottom() - m_aFrameAtFrame.GetHeight()) / 2 + 5));

    // Size of the frame to be positioned
    if (m_nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        sal_uInt32 nLFBorder = m_nAnchor == RndStdIds::FLY_AT_PAGE ? nLBorder : nLTxtBorder;
        sal_uInt32 nRFBorder = m_nAnchor == RndStdIds::FLY_AT_PAGE ? nRBorder : nRTxtBorder;

        switch (m_nHRel)
        {
            case RelOrientation::PAGE_LEFT:
            case RelOrientation::FRAME_LEFT:
                m_aFrmSize = Size(nLFBorder - 4, (m_aTextLine.GetHeight() + 2) * 3);
                break;

            case RelOrientation::PAGE_RIGHT:
            case RelOrientation::FRAME_RIGHT:
                m_aFrmSize = Size(nRFBorder - 4, (m_aTextLine.GetHeight() + 2) * 3);
                break;

            default:
                m_aFrmSize = Size(nLBorder - 3, (m_aTextLine.GetHeight() + 2) * 3);
                break;
        }
        m_aFrmSize.setWidth( std::max(tools::Long(5), m_aFrmSize.Width()) );
        m_aFrmSize.setHeight( std::max(tools::Long(5), m_aFrmSize.Height()) );
    }
    else
    {
        sal_uInt32 nFreeWidth = m_aPagePrtArea.GetWidth() - rRenderContext.GetTextWidth(DEMOTEXT);

        m_aFrmSize = Size(nFreeWidth / 2, (m_aTextLine.GetHeight() + 2) * 3);
        m_aDrawObj.SetSize(Size(std::max(tools::Long(5), tools::Long(nFreeWidth / 3)), std::max(tools::Long(5), m_aFrmSize.Height() * 3)));
        m_aDrawObj.SetPos(Point(m_aParaPrtArea.Right() + 1, m_aParaPrtArea.Bottom() / 2));
        m_aParaPrtArea.SetRight( m_aDrawObj.Right() );
    }
}

void SwFrameExample::CalcBoundRect_Impl(const vcl::RenderContext& rRenderContext, tools::Rectangle &rRect)
{
    switch (m_nAnchor)
    {
        case RndStdIds::FLY_AT_PAGE:
        {
            switch (m_nHRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetLeft( m_aPage.Left() );
                    rRect.SetRight( m_aPage.Right() );
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetLeft( m_aPagePrtArea.Left() );
                    rRect.SetRight( m_aPagePrtArea.Right() );
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.SetLeft( m_aPage.Left() );
                    rRect.SetRight( m_aPagePrtArea.Left() );
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.SetLeft( m_aPagePrtArea.Right() );
                    rRect.SetRight( m_aPage.Right() );
                    break;
            }

            switch (m_nVRel)
            {
                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetTop( m_aPagePrtArea.Top() );
                    rRect.SetBottom( m_aPagePrtArea.Bottom() );
                    break;

                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetTop( m_aPage.Top() );
                    rRect.SetBottom( m_aPage.Bottom() );
                    break;
            }
        }
        break;

        case RndStdIds::FLY_AT_FLY:
        {
            switch (m_nHRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetLeft( m_aFrameAtFrame.Left() );
                    rRect.SetRight( m_aFrameAtFrame.Right() );
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetLeft( m_aFrameAtFrame.Left() + FLYINFLY_BORDER );
                    rRect.SetRight( m_aFrameAtFrame.Right() - FLYINFLY_BORDER );
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.SetLeft( m_aFrameAtFrame.Left() );
                    rRect.SetRight( m_aFrameAtFrame.Left() + FLYINFLY_BORDER );
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.SetLeft( m_aFrameAtFrame.Right() );
                    rRect.SetRight( m_aFrameAtFrame.Right() - FLYINFLY_BORDER );
                    break;
            }

            switch (m_nVRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.SetTop( m_aFrameAtFrame.Top() );
                    rRect.SetBottom( m_aFrameAtFrame.Bottom() );
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetTop( m_aFrameAtFrame.Top() + FLYINFLY_BORDER );
                    rRect.SetBottom( m_aFrameAtFrame.Bottom() - FLYINFLY_BORDER );
                    break;
            }
        }
        break;
        case RndStdIds::FLY_AT_PARA:
        case RndStdIds::FLY_AT_CHAR:
        {
            switch (m_nHRel)
            {
                case RelOrientation::FRAME:
                    rRect.SetLeft( m_aPara.Left() );
                    rRect.SetRight( m_aPara.Right() );
                    break;

                case RelOrientation::PRINT_AREA:
                    rRect.SetLeft( m_aParaPrtArea.Left() );
                    rRect.SetRight( m_aParaPrtArea.Right() );
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.SetLeft( m_aPage.Left() );
                    rRect.SetRight( m_aPagePrtArea.Left() );
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.SetLeft( m_aPagePrtArea.Right() );
                    rRect.SetRight( m_aPage.Right() );
                    break;

                case RelOrientation::PAGE_FRAME:
                    rRect.SetLeft( m_aPage.Left() );
                    rRect.SetRight( m_aPage.Right() );
                    break;

                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.SetLeft( m_aPagePrtArea.Left() );
                    rRect.SetRight( m_aPagePrtArea.Right() );
                    break;

                case RelOrientation::FRAME_LEFT:
                    rRect.SetLeft( m_aPara.Left() );
                    rRect.SetRight( m_aParaPrtArea.Left() );
                    break;

                case RelOrientation::FRAME_RIGHT:
                    rRect.SetLeft( m_aParaPrtArea.Right() );
                    rRect.SetRight( m_aPara.Right() );
                    break;

                case RelOrientation::CHAR:
                    rRect.SetLeft( m_aAutoCharFrame.Left() );
                    rRect.SetRight( m_aAutoCharFrame.Left() );
                    break;
            }

            switch (m_nVRel)
            {
                case RelOrientation::FRAME:
                    rRect.SetTop( m_aPara.Top() );
                    rRect.SetBottom( m_aPara.Bottom() );
                    break;

                case RelOrientation::PRINT_AREA:
                    rRect.SetTop( m_aParaPrtArea.Top() );
                    rRect.SetBottom( m_aParaPrtArea.Bottom() );
                    break;

                case RelOrientation::CHAR:
                    if (m_nVAlign != VertOrientation::NONE &&
                                m_nVAlign != VertOrientation::CHAR_BOTTOM)
                        rRect.SetTop( m_aAutoCharFrame.Top() );
                    else
                        rRect.SetTop( m_aAutoCharFrame.Bottom() );
                    rRect.SetBottom( m_aAutoCharFrame.Bottom() );
                    break;
                // OD 12.11.2003 #i22341#
                case RelOrientation::TEXT_LINE:
                    rRect.SetTop( m_aAutoCharFrame.Top() );
                    rRect.SetBottom( m_aAutoCharFrame.Top() );
                break;
            }
        }
        break;

        case RndStdIds::FLY_AS_CHAR:
            rRect.SetLeft( m_aParaPrtArea.Left() );
            rRect.SetRight( m_aParaPrtArea.Right() );

            switch (m_nVAlign)
            {
                case VertOrientation::NONE:
                case VertOrientation::TOP:
                case VertOrientation::CENTER:
                case VertOrientation::BOTTOM:
                {
                    FontMetric aMetric(rRenderContext.GetFontMetric());

                    rRect.SetTop( m_aParaPrtArea.Bottom() - aMetric.GetDescent() );
                    rRect.SetBottom( rRect.Top() );
                }
                break;

                default:

                case VertOrientation::LINE_TOP:
                case VertOrientation::LINE_CENTER:
                case VertOrientation::LINE_BOTTOM:
                    rRect.SetTop( m_aParaPrtArea.Top() );
                    rRect.SetBottom( m_aDrawObj.Bottom() );
                    break;

                case VertOrientation::CHAR_TOP:
                case VertOrientation::CHAR_CENTER:
                case VertOrientation::CHAR_BOTTOM:
                    rRect.SetTop( m_aParaPrtArea.Top() );
                    rRect.SetBottom( m_aParaPrtArea.Bottom() );
                    break;
            }
            break;

        default:
            break;
    }
}

tools::Rectangle SwFrameExample::DrawInnerFrame_Impl(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect,
                                                 std::optional<Color> oFillColor, const Color &rBorderColor)
{
    DrawRect_Impl(rRenderContext, rRect, oFillColor, rBorderColor);

    // determine the area relative to which the positioning happens
    tools::Rectangle aRect(rRect); // aPagePrtArea = Default
    CalcBoundRect_Impl(rRenderContext, aRect);

    if (m_nAnchor == RndStdIds::FLY_AT_FLY && &rRect == &m_aPagePrtArea)
    {
        // draw text paragraph
        tools::Rectangle aTxt(m_aTextLine);
        sal_Int32 nStep = aTxt.GetHeight() + 2;
        sal_uInt16 nLines = static_cast<sal_uInt16>(m_aParaPrtArea.GetHeight() / (m_aTextLine.GetHeight() + 2));

        for (sal_uInt16 i = 0; i < nLines; i++)
        {
            if (i == nLines - 1)
                aTxt.SetSize(Size(aTxt.GetWidth() / 2, aTxt.GetHeight()));
            DrawRect_Impl(rRenderContext, aTxt, m_aTxtCol, std::nullopt);
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
    DrawRect_Impl(rRenderContext, m_aPage, m_aBgCol, m_aBorderCol);

    // Draw PrintArea
    tools::Rectangle aRect = DrawInnerFrame_Impl(rRenderContext, m_aPagePrtArea, std::nullopt, m_aPrintAreaCol);

    if (m_nAnchor == RndStdIds::FLY_AT_FLY)
        aRect = DrawInnerFrame_Impl(rRenderContext, m_aFrameAtFrame, m_aBgCol, m_aBorderCol);

    tools::Long lXPos = 0;
    tools::Long lYPos = 0;

    // Horizontal alignment
    if (m_nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        switch (m_nHAlign)
        {
            case HoriOrientation::RIGHT:
            {
                lXPos = aRect.Right() - m_aFrmSize.Width() + 1;
                break;
            }
            case HoriOrientation::CENTER:
            {
                lXPos = aRect.Left() + (aRect.GetWidth() - m_aFrmSize.Width()) / 2;
                break;
            }
            case HoriOrientation::NONE:
            {
                lXPos = aRect.Left() + m_aRelPos.X();
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
    if (m_nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        switch (m_nVAlign)
        {
            case VertOrientation::BOTTOM:
            case VertOrientation::LINE_BOTTOM:
            {
                // #i22341#
                if ( m_nVRel != RelOrientation::TEXT_LINE )
                {
                    lYPos = aRect.Bottom() - m_aFrmSize.Height() + 1;
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
                lYPos = aRect.Top() + (aRect.GetHeight() - m_aFrmSize.Height()) / 2;
                break;
            }
            case VertOrientation::NONE:
            {
                // #i22341#
                if ( m_nVRel != RelOrientation::CHAR && m_nVRel != RelOrientation::TEXT_LINE )
                    lYPos = aRect.Top() + m_aRelPos.Y();
                else
                    lYPos = aRect.Top() - m_aRelPos.Y();
                break;
            }
            default:
                // #i22341#
                if ( m_nVRel != RelOrientation::TEXT_LINE )
                {
                    lYPos = aRect.Top();
                }
                else
                {
                    lYPos = aRect.Bottom() - m_aFrmSize.Height() + 1;
                }
                break;
        }
    }
    else
    {
        switch(m_nVAlign)
        {
            case VertOrientation::CENTER:
            case VertOrientation::CHAR_CENTER:
            case VertOrientation::LINE_CENTER:
                lYPos = aRect.Top() + (aRect.GetHeight() - m_aFrmSize.Height()) / 2;
                break;

            case VertOrientation::TOP:
            case VertOrientation::CHAR_BOTTOM:
            case VertOrientation::LINE_BOTTOM:
                lYPos = aRect.Bottom() - m_aFrmSize.Height() + 1;
                break;

            default:
                lYPos = aRect.Top() - m_aRelPos.Y();
                break;
        }
    }

    tools::Rectangle aFrmRect(Point(lXPos, lYPos), m_aFrmSize);

    tools::Rectangle* pOuterFrame = &m_aPage;

    if (m_nAnchor == RndStdIds::FLY_AT_FLY)
        pOuterFrame = &m_aFrameAtFrame;

    if (aFrmRect.Left() < pOuterFrame->Left())
        aFrmRect.Move(pOuterFrame->Left() - aFrmRect.Left(), 0);
    if (aFrmRect.Right() > pOuterFrame->Right())
        aFrmRect.Move(pOuterFrame->Right() - aFrmRect.Right(), 0);

    if (aFrmRect.Top() < pOuterFrame->Top())
        aFrmRect.Move(0, pOuterFrame->Top() - aFrmRect.Top());
    if (aFrmRect.Bottom() > pOuterFrame->Bottom())
        aFrmRect.Move(0, pOuterFrame->Bottom() - aFrmRect.Bottom());

    // Draw Test paragraph
    const tools::Long nTxtLineHeight = m_aTextLine.GetHeight();
    tools::Rectangle aTxt(m_aTextLine);
    sal_Int32 nStep;
    sal_uInt16 nLines;

    if (m_nAnchor == RndStdIds::FLY_AT_FLY)
    {
        aTxt.SetLeft( m_aFrameAtFrame.Left() + FLYINFLY_BORDER );
        aTxt.SetRight( m_aFrameAtFrame.Right() - FLYINFLY_BORDER );
        aTxt.SetTop( m_aFrameAtFrame.Top() + FLYINFLY_BORDER );
        aTxt.SetBottom( aTxt.Top() + m_aTextLine.GetHeight() - 1 );

        nStep = aTxt.GetHeight() + 2;
        nLines = static_cast<sal_uInt16>(((m_aFrameAtFrame.GetHeight() - 2 * FLYINFLY_BORDER) * 2 / 3)
                 / (aTxt.GetHeight() + 2));
    }
    else
    {
        nStep = aTxt.GetHeight() + 2;
        nLines = static_cast<sal_uInt16>(m_aParaPrtArea.GetHeight() / (m_aTextLine.GetHeight() + 2));
    }

    if (m_nAnchor != RndStdIds::FLY_AS_CHAR)
    {
        // Simulate text
        const tools::Long nOldR = aTxt.Right();
        const tools::Long nOldL = aTxt.Left();

        // #i22341#
        const bool bIgnoreWrap = m_nAnchor == RndStdIds::FLY_AT_CHAR &&
                           ( m_nHRel == RelOrientation::CHAR || m_nVRel == RelOrientation::CHAR ||
                             m_nVRel == RelOrientation::TEXT_LINE );

        for (sal_uInt16 i = 0; i < nLines; ++i)
        {
            if (i == (nLines - 1))
                aTxt.SetSize(Size(aTxt.GetWidth() / 2, aTxt.GetHeight()));

            if (aTxt.Overlaps(aFrmRect) && m_nAnchor != RndStdIds::FLY_AS_CHAR && !bIgnoreWrap)
            {
                switch(m_nWrap)
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
            if (pOuterFrame->Contains(aTxt))
                DrawRect_Impl(rRenderContext, aTxt, m_aTxtCol, std::nullopt );

            aTxt.Move(0, nStep);
            aTxt.SetRight( nOldR );
            aTxt.SetLeft( nOldL );
        }
        aTxt.Move(0, -nStep);

        if (m_nAnchor != RndStdIds::FLY_AT_FLY && aTxt.Bottom() > m_aParaPrtArea.Bottom())
        {
            // Text has been replaced by frame, so adjust parameters height
            sal_Int32 nDiff = aTxt.Bottom() - m_aParaPrtArea.Bottom();
            m_aParaPrtArea.AdjustBottom(nDiff );
            m_aPara.AdjustBottom(nDiff );

            CalcBoundRect_Impl(rRenderContext, aRect);

            m_aParaPrtArea.AdjustBottom( -nDiff );
            m_aPara.AdjustBottom( -nDiff );
        }
        if (m_nAnchor == RndStdIds::FLY_AT_CHAR && bIgnoreWrap)
            rRenderContext.DrawText(m_aAutoCharFrame, OUString('A'));
    }
    else
    {
        rRenderContext.DrawText(m_aParaPrtArea, DEMOTEXT);
        DrawRect_Impl(rRenderContext, m_aDrawObj, m_aBlankCol, m_aBlankFrameCol );
    }

    // Draw rectangle on which the frame is aligned:
    DrawRect_Impl(rRenderContext, aRect, std::nullopt, m_aAlignColor);

    // Frame View
    bool bDontFill = (m_nAnchor == RndStdIds::FLY_AT_CHAR && aFrmRect.Overlaps(m_aAutoCharFrame)) || m_bTrans;
    DrawRect_Impl(rRenderContext, aFrmRect, bDontFill ? std::nullopt : std::optional { m_aBgCol }, m_aFrameColor);
}

void SwFrameExample::SetRelPos(const Point& rP)
{
    m_aRelPos = rP;

    if (m_aRelPos.X() > 0)
        m_aRelPos.setX( 5 );
    if (m_aRelPos.X() < 0)
        m_aRelPos.setX( -5 );

    if (m_aRelPos.Y() > 0)
        m_aRelPos.setY( 5 );
    if (m_aRelPos.Y() < 0)
        m_aRelPos.setY( -5 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
