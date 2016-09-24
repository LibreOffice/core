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
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>

using namespace ::com::sun::star::text;

#define FLYINFLY_BORDER 3
#define DEMOTEXT        "Ij"

namespace {

void DrawRect_Impl(vcl::RenderContext& rRenderContext, const Rectangle &rRect,
                   const Color &rFillColor, const Color &rLineColor)
{
    rRenderContext.SetFillColor(rFillColor);
    rRenderContext.SetLineColor(rLineColor);
    rRenderContext.DrawRect(rRect);
}

}

SvxSwFrameExample::SvxSwFrameExample( vcl::Window *pParent, WinBits nStyle ) :

    Window(pParent, nStyle),

    nHAlign     (HoriOrientation::CENTER),
    nHRel       (RelOrientation::FRAME),
    nVAlign     (VertOrientation::TOP),
    nVRel       (RelOrientation::PRINT_AREA),
    nWrap       (WrapTextMode_NONE),
    nAnchor     (TextContentAnchorType_AT_PAGE),
    bTrans      (false),
    aRelPos     (Point(0,0))
{
    InitColors_Impl();
}

VCL_BUILDER_FACTORY_ARGS(SvxSwFrameExample, 0)

Size SvxSwFrameExample::GetOptimalSize() const
{
    return LogicToPixel(Size(52, 86), MapMode(MAP_APPFONT));
}

void SvxSwFrameExample::InitColors_Impl()
{
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    m_aBgCol = Color( rSettings.GetWindowColor() );

    bool bHC = rSettings.GetHighContrastMode();

    m_aFrameColor = Color( COL_LIGHTGREEN );
    m_aAlignColor = Color( COL_LIGHTRED );
    m_aTransColor = Color( COL_TRANSPARENT );

    m_aTxtCol = bHC?
        svtools::ColorConfig().GetColorValue(svtools::FONTCOLOR).nColor :
        Color( COL_GRAY );
    m_aPrintAreaCol = bHC? m_aTxtCol : Color( COL_GRAY );
    m_aBorderCol = m_aTxtCol;
    m_aBlankCol = bHC? m_aTxtCol : Color( COL_LIGHTGRAY );
    m_aBlankFrameCol = bHC? m_aTxtCol : Color( COL_GRAY );
}

void SvxSwFrameExample::DataChanged(const DataChangedEvent& rDCEvt)
{
    Window::DataChanged(rDCEvt);

    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
        InitColors_Impl();
}

void SvxSwFrameExample::InitAllRects_Impl(vcl::RenderContext& rRenderContext)
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

    if (nAnchor != TextContentAnchorType_AS_CHARACTER)
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
    aPagePrtArea = Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));

    // Example text: Preparing for the text output
    // A line of text
    aTextLine = aPagePrtArea;
    aTextLine.SetSize(Size(aTextLine.GetWidth(), 2));
    aTextLine.Left()    += nLTxtBorder;
    aTextLine.Right()   -= nRTxtBorder;
    aTextLine.Move(0, nTTxtBorder);

    // Rectangle to edges including paragraph
    sal_uInt16 nLines = (sal_uInt16)((aPagePrtArea.GetHeight() / 2 - nTTxtBorder - nBTxtBorder)
             / (aTextLine.GetHeight() + 2));
    aPara = aPagePrtArea;
    aPara.SetSize(Size(aPara.GetWidth(),
        (aTextLine.GetHeight() + 2) * nLines + nTTxtBorder + nBTxtBorder));

    // Rectangle around paragraph without borders
    aParaPrtArea = aPara;
    aParaPrtArea.Left()     += nLTxtBorder;
    aParaPrtArea.Right()    -= nRTxtBorder;
    aParaPrtArea.Top()      += nTTxtBorder;
    aParaPrtArea.Bottom()   -= nBTxtBorder;

    if (nAnchor == TextContentAnchorType_AS_CHARACTER || nAnchor == TextContentAnchorType_AT_CHARACTER)
    {
        vcl::Font aFont = OutputDevice::GetDefaultFont(
                                DefaultFontType::LATIN_TEXT, Application::GetSettings().GetLanguageTag().getLanguageType(),
                                GetDefaultFontFlags::OnlyOne, this );
        aFont.SetColor( m_aTxtCol );
        aFont.SetFillColor( m_aBgCol );
        aFont.SetWeight(WEIGHT_NORMAL);

        if (nAnchor == TextContentAnchorType_AS_CHARACTER)
        {
            aFont.SetFontSize(Size(0, aParaPrtArea.GetHeight() - 2));
            SetFont(aFont);
            aParaPrtArea.SetSize(Size(GetTextWidth(DEMOTEXT), GetTextHeight()));
        }
        else
        {
            aFont.SetFontSize(Size(0, aParaPrtArea.GetHeight() / 2));
            rRenderContext.SetFont(aFont);
            aAutoCharFrame.SetSize(Size(GetTextWidth(OUString('A')), GetTextHeight()));
            aAutoCharFrame.SetPos(Point(aParaPrtArea.Left() + (aParaPrtArea.GetWidth() - aAutoCharFrame.GetWidth()) / 2,
                aParaPrtArea.Top() + (aParaPrtArea.GetHeight() - aAutoCharFrame.GetHeight()) / 2));
        }
    }

    // Inner Frame anchored at the Frame
    aFrameAtFrame = aPara;
    aFrameAtFrame.Left() += 9;
    aFrameAtFrame.Right() -= 5;
    aFrameAtFrame.Bottom() += 5;
    aFrameAtFrame.SetPos(Point(aFrameAtFrame.Left() + 2, (aPagePrtArea.Bottom() - aFrameAtFrame.GetHeight()) / 2 + 5));

    // Size of the frame to be positioned
    if (nAnchor != TextContentAnchorType_AS_CHARACTER)
    {
        sal_uInt32 nLFBorder = nAnchor == TextContentAnchorType_AT_PAGE ? nLBorder : nLTxtBorder;
        sal_uInt32 nRFBorder = nAnchor == TextContentAnchorType_AT_PAGE ? nRBorder : nRTxtBorder;

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
        aFrmSize.Width() = std::max(5L, aFrmSize.Width());
        aFrmSize.Height() = std::max(5L, aFrmSize.Height());
    }
    else
    {
        sal_uInt32 nFreeWidth = aPagePrtArea.GetWidth() - GetTextWidth(DEMOTEXT);

        aFrmSize = Size(nFreeWidth / 2, (aTextLine.GetHeight() + 2) * 3);
        aDrawObj.SetSize(Size(std::max(5L, (long)nFreeWidth / 3L), std::max(5L, aFrmSize.Height() * 3L)));
        aDrawObj.SetPos(Point(aParaPrtArea.Right() + 1, aParaPrtArea.Bottom() / 2));
        aParaPrtArea.Right() = aDrawObj.Right();
    }
}

void SvxSwFrameExample::CalcBoundRect_Impl(Rectangle &rRect)
{
    switch (nAnchor)
    {
        case TextContentAnchorType_AT_PAGE:
        {
            switch (nHRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.Left() = aPage.Left();
                    rRect.Right() = aPage.Right();
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.Left() = aPagePrtArea.Left();
                    rRect.Right() = aPagePrtArea.Right();
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.Left() = aPage.Left();
                    rRect.Right() = aPagePrtArea.Left();
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.Left() = aPagePrtArea.Right();
                    rRect.Right() = aPage.Right();
                    break;
            }

            switch (nVRel)
            {
                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.Top() = aPagePrtArea.Top();
                    rRect.Bottom() = aPagePrtArea.Bottom();
                    break;

                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.Top() = aPage.Top();
                    rRect.Bottom() = aPage.Bottom();
                    break;
            }
        }
        break;

        case TextContentAnchorType_AT_FRAME:
        {
            switch (nHRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.Left() = aFrameAtFrame.Left();
                    rRect.Right() = aFrameAtFrame.Right();
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.Left() = aFrameAtFrame.Left() + FLYINFLY_BORDER;
                    rRect.Right() = aFrameAtFrame.Right() - FLYINFLY_BORDER;
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.Left() = aFrameAtFrame.Left();
                    rRect.Right() = aFrameAtFrame.Left() + FLYINFLY_BORDER;
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.Left() = aFrameAtFrame.Right();
                    rRect.Right() = aFrameAtFrame.Right() - FLYINFLY_BORDER;
                    break;
            }

            switch (nVRel)
            {
                case RelOrientation::FRAME:
                case RelOrientation::PAGE_FRAME:
                    rRect.Top() = aFrameAtFrame.Top();
                    rRect.Bottom() = aFrameAtFrame.Bottom();
                    break;

                case RelOrientation::PRINT_AREA:
                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.Top() = aFrameAtFrame.Top() + FLYINFLY_BORDER;
                    rRect.Bottom() = aFrameAtFrame.Bottom() - FLYINFLY_BORDER;
                    break;
            }
        }
        break;
        case TextContentAnchorType_AT_PARAGRAPH:
        case TextContentAnchorType_AT_CHARACTER:
        {
            switch (nHRel)
            {
                case RelOrientation::FRAME:
                    rRect.Left() = aPara.Left();
                    rRect.Right() = aPara.Right();
                    break;

                case RelOrientation::PRINT_AREA:
                    rRect.Left() = aParaPrtArea.Left();
                    rRect.Right() = aParaPrtArea.Right();
                    break;

                case RelOrientation::PAGE_LEFT:
                    rRect.Left() = aPage.Left();
                    rRect.Right() = aPagePrtArea.Left();
                    break;

                case RelOrientation::PAGE_RIGHT:
                    rRect.Left() = aPagePrtArea.Right();
                    rRect.Right() = aPage.Right();
                    break;

                case RelOrientation::PAGE_FRAME:
                    rRect.Left() = aPage.Left();
                    rRect.Right() = aPage.Right();
                    break;

                case RelOrientation::PAGE_PRINT_AREA:
                    rRect.Left() = aPagePrtArea.Left();
                    rRect.Right() = aPagePrtArea.Right();
                    break;

                case RelOrientation::FRAME_LEFT:
                    rRect.Left() = aPara.Left();
                    rRect.Right() = aParaPrtArea.Left();
                    break;

                case RelOrientation::FRAME_RIGHT:
                    rRect.Left() = aParaPrtArea.Right();
                    rRect.Right() = aPara.Right();
                    break;

                case RelOrientation::CHAR:
                    rRect.Left() = aAutoCharFrame.Left();
                    rRect.Right() = aAutoCharFrame.Left();
                    break;
            }

            switch (nVRel)
            {
                case RelOrientation::FRAME:
                    rRect.Top() = aPara.Top();
                    rRect.Bottom() = aPara.Bottom();
                    break;

                case RelOrientation::PRINT_AREA:
                    rRect.Top() = aParaPrtArea.Top();
                    rRect.Bottom() = aParaPrtArea.Bottom();
                    break;

                case RelOrientation::CHAR:
                    if (nVAlign != VertOrientation::NONE &&
                                nVAlign != VertOrientation::CHAR_BOTTOM)
                        rRect.Top() = aAutoCharFrame.Top();
                    else
                        rRect.Top() = aAutoCharFrame.Bottom();
                    rRect.Bottom() = aAutoCharFrame.Bottom();
                    break;
                // OD 12.11.2003 #i22341#
                case RelOrientation::TEXT_LINE:
                    rRect.Top() = aAutoCharFrame.Top();
                    rRect.Bottom() = aAutoCharFrame.Top();
                break;
            }
        }
        break;

        case TextContentAnchorType_AS_CHARACTER:
            rRect.Left() = aParaPrtArea.Left();
            rRect.Right() = aParaPrtArea.Right();

            switch (nVAlign)
            {
                case VertOrientation::NONE:
                case VertOrientation::TOP:
                case VertOrientation::CENTER:
                case VertOrientation::BOTTOM:
                {
                    FontMetric aMetric(GetFontMetric());

                    rRect.Top() = aParaPrtArea.Bottom() - aMetric.GetDescent();
                    rRect.Bottom() = rRect.Top();
                }
                break;

                default:

                case VertOrientation::LINE_TOP:
                case VertOrientation::LINE_CENTER:
                case VertOrientation::LINE_BOTTOM:
                    rRect.Top() = aParaPrtArea.Top();
                    rRect.Bottom() = aDrawObj.Bottom();
                    break;

                case VertOrientation::CHAR_TOP:
                case VertOrientation::CHAR_CENTER:
                case VertOrientation::CHAR_BOTTOM:
                    rRect.Top() = aParaPrtArea.Top();
                    rRect.Bottom() = aParaPrtArea.Bottom();
                    break;
            }
            break;

        default:
            break;
    }
}

Rectangle SvxSwFrameExample::DrawInnerFrame_Impl(vcl::RenderContext& rRenderContext, const Rectangle &rRect,
                                                 const Color &rFillColor, const Color &rBorderColor)
{
    DrawRect_Impl(rRenderContext, rRect, rFillColor, rBorderColor);

    // Bereich, zu dem relativ positioniert wird, bestimmen
    Rectangle aRect(rRect); // aPagePrtArea = Default
    CalcBoundRect_Impl(aRect);

    if (nAnchor == TextContentAnchorType_AT_FRAME && &rRect == &aPagePrtArea)
    {
        // Testabsatz zeichnen
        Rectangle aTxt(aTextLine);
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

void SvxSwFrameExample::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    rRenderContext.SetMapMode(MAP_PIXEL);

    InitAllRects_Impl(rRenderContext);

    // Draw page
    DrawRect_Impl(rRenderContext, aPage, m_aBgCol, m_aBorderCol);

    // Draw PrintArea
    Rectangle aRect = DrawInnerFrame_Impl(rRenderContext, aPagePrtArea, m_aTransColor, m_aPrintAreaCol);

    if (nAnchor == TextContentAnchorType_AT_FRAME)
        aRect = DrawInnerFrame_Impl(rRenderContext, aFrameAtFrame, m_aBgCol, m_aBorderCol);

    long lXPos = 0;
    long lYPos = 0;

    // Horizontal alignment
    if (nAnchor != TextContentAnchorType_AS_CHARACTER)
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
    if (nAnchor != TextContentAnchorType_AS_CHARACTER)
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

    Rectangle aFrmRect(Point(lXPos, lYPos), aFrmSize);

    Rectangle* pOuterFrame = &aPage;

    if (nAnchor == TextContentAnchorType_AT_FRAME)
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
    Rectangle aTxt(aTextLine);
    sal_Int32 nStep;
    sal_uInt16 nLines;

    if (nAnchor == TextContentAnchorType_AT_FRAME)
    {
        aTxt.Left() = aFrameAtFrame.Left() + FLYINFLY_BORDER;
        aTxt.Right() = aFrameAtFrame.Right() - FLYINFLY_BORDER;
        aTxt.Top() = aFrameAtFrame.Top() + FLYINFLY_BORDER;
        aTxt.Bottom() = aTxt.Top() + aTextLine.GetHeight() - 1;

        nStep = aTxt.GetHeight() + 2;
        nLines = (sal_uInt16)(((aFrameAtFrame.GetHeight() - 2 * FLYINFLY_BORDER) * 2 / 3)
                 / (aTxt.GetHeight() + 2));
    }
    else
    {
        nStep = aTxt.GetHeight() + 2;
        nLines = (sal_uInt16)(aParaPrtArea.GetHeight() / (aTextLine.GetHeight() + 2));
    }

    if (nAnchor != TextContentAnchorType_AS_CHARACTER)
    {
        // Simulate text
        const long nOldR = aTxt.Right();
        const long nOldL = aTxt.Left();

        // #i22341#
        const bool bIgnoreWrap = nAnchor == TextContentAnchorType_AT_CHARACTER &&
                           ( nHRel == RelOrientation::CHAR || nVRel == RelOrientation::CHAR ||
                             nVRel == RelOrientation::TEXT_LINE );

        for (sal_uInt16 i = 0; i < nLines; ++i)
        {
            if (i == (nLines - 1))
                aTxt.SetSize(Size(aTxt.GetWidth() / 2, aTxt.GetHeight()));

            if (aTxt.IsOver(aFrmRect) && nAnchor != TextContentAnchorType_AS_CHARACTER && !bIgnoreWrap)
            {
                switch(nWrap)
                {
                    case WrapTextMode_NONE:
                        aTxt.Top()    = aFrmRect.Bottom() + nTxtLineHeight;
                        aTxt.Bottom() = aTxt.Top() + nTxtLineHeight - 1;
                        break;

                    case WrapTextMode_LEFT:
                        aTxt.Right() = aFrmRect.Left();
                        break;

                    case WrapTextMode_RIGHT:
                        aTxt.Left() = aFrmRect.Right();
                        break;
                }
            }
            if (pOuterFrame->IsInside(aTxt))
                DrawRect_Impl(rRenderContext, aTxt, m_aTxtCol, m_aTransColor );

            aTxt.Move(0, nStep);
            aTxt.Right() = nOldR;
            aTxt.Left()  = nOldL;
        }
        aTxt.Move(0, -nStep);

        if (nAnchor != TextContentAnchorType_AT_FRAME && aTxt.Bottom() > aParaPrtArea.Bottom())
        {
            // Text has been replaced by frame, so adjust parameters height
            sal_uInt32 nDiff = aTxt.Bottom() - aParaPrtArea.Bottom();
            aParaPrtArea.Bottom() += nDiff;
            aPara.Bottom() += nDiff;

            CalcBoundRect_Impl(aRect);

            aParaPrtArea.Bottom() -= nDiff;
            aPara.Bottom() -= nDiff;
        }
        if (nAnchor == TextContentAnchorType_AT_CHARACTER && bIgnoreWrap)
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
    bool bDontFill = (nAnchor == TextContentAnchorType_AT_CHARACTER && aFrmRect.IsOver(aAutoCharFrame)) || bTrans;
    DrawRect_Impl(rRenderContext, aFrmRect, bDontFill? m_aTransColor : m_aBgCol, m_aFrameColor);
}

void SvxSwFrameExample::SetRelPos(const Point& rP)
{
    aRelPos = rP;

    if (aRelPos.X() > 0)
        aRelPos.X() = 5;
    if (aRelPos.X() < 0)
        aRelPos.X() = -5;

    if (aRelPos.Y() > 0)
        aRelPos.Y() = 5;
    if (aRelPos.Y() < 0)
        aRelPos.Y() = -5;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
