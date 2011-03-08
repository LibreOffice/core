/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"


#include <tools/poly.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <svtools/colorcfg.hxx>
#include <swframeexample.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>

using namespace ::com::sun::star::text;

#define FLYINFLY_BORDER 3
#define DEMOTEXT        "Ij"
#define C2S(cChar) UniString::CreateFromAscii(cChar)


SvxSwFrameExample::SvxSwFrameExample( Window *pParent, const ResId& rResID ) :

    Window(pParent, rResID),

    nHAlign     (HoriOrientation::CENTER),
    nHRel       (RelOrientation::FRAME),
    nVAlign     (VertOrientation::TOP),
    nVRel       (RelOrientation::PRINT_AREA),
    nWrap       (WrapTextMode_NONE),
    nAnchor     (TextContentAnchorType_AT_PAGE),
    bTrans      (FALSE),
    aRelPos     (Point(0,0))
{
    InitColors_Impl();
    SetMapMode(MAP_PIXEL);
}

SvxSwFrameExample::~SvxSwFrameExample()
{
}

void SvxSwFrameExample::InitColors_Impl( void )
{
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    m_aBgCol = Color( rSettings.GetWindowColor() );

    BOOL bHC = rSettings.GetHighContrastMode();

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

void SvxSwFrameExample::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if( rDCEvt.GetType() == DATACHANGED_SETTINGS && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitColors_Impl();
}

void SvxSwFrameExample::InitAllRects_Impl()
{
    aPage.SetSize( GetOutputSizePixel() );

    ULONG nOutWPix = aPage.GetWidth();
    ULONG nOutHPix = aPage.GetHeight();

    // PrintArea
    ULONG nLBorder;
    ULONG nRBorder;
    ULONG nTBorder;
    ULONG nBBorder;

    ULONG nLTxtBorder;
    ULONG nRTxtBorder;
    ULONG nTTxtBorder;
    ULONG nBTxtBorder;

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
    USHORT nLines = (USHORT)((aPagePrtArea.GetHeight() / 2 - nTTxtBorder - nBTxtBorder)
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
        Font aFont = OutputDevice::GetDefaultFont(
                                DEFAULTFONT_LATIN_TEXT, Application::GetSettings().GetLanguage(),
                                DEFAULTFONT_FLAGS_ONLYONE, this );
        aFont.SetColor( m_aTxtCol );
        aFont.SetFillColor( m_aBgCol );
        aFont.SetWeight(WEIGHT_NORMAL);

        if (nAnchor == TextContentAnchorType_AS_CHARACTER)
        {
            aFont.SetSize(Size(0, aParaPrtArea.GetHeight() - 2));
            SetFont(aFont);
            aParaPrtArea.SetSize(Size(GetTextWidth(C2S(DEMOTEXT)), GetTextHeight()));
        }
        else
        {
            aFont.SetSize(Size(0, aParaPrtArea.GetHeight() / 2));
            SetFont(aFont);
            aAutoCharFrame.SetSize(Size(GetTextWidth('A'), GetTextHeight()));
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
        ULONG nLFBorder = nAnchor == TextContentAnchorType_AT_PAGE ? nLBorder : nLTxtBorder;
        ULONG nRFBorder = nAnchor == TextContentAnchorType_AT_PAGE ? nRBorder : nRTxtBorder;

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
        aFrmSize.Width() = Max(5L, aFrmSize.Width());
        aFrmSize.Height() = Max(5L, aFrmSize.Height());
    }
    else
    {
        ULONG nFreeWidth = aPagePrtArea.GetWidth() - GetTextWidth(C2S(DEMOTEXT));

        aFrmSize = Size(nFreeWidth / 2, (aTextLine.GetHeight() + 2) * 3);
        aDrawObj.SetSize(Size(Max(5L, (long)nFreeWidth / 3L), Max(5L, aFrmSize.Height() * 3L)));
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

Rectangle SvxSwFrameExample::DrawInnerFrame_Impl(const Rectangle &rRect, const Color &rFillColor, const Color &rBorderColor)
{
    DrawRect_Impl(rRect, rFillColor, rBorderColor);

    // Bereich, zu dem relativ positioniert wird, bestimmen
    Rectangle aRect(rRect); // aPagePrtArea = Default
    CalcBoundRect_Impl(aRect);

    if (nAnchor == TextContentAnchorType_AT_FRAME && &rRect == &aPagePrtArea)
    {
        // Testabsatz zeichnen
        Rectangle aTxt(aTextLine);
        sal_Int32 nStep = aTxt.GetHeight() + 2;
        USHORT nLines = (USHORT)(aParaPrtArea.GetHeight() / (aTextLine.GetHeight() + 2));

        for (USHORT i = 0; i < nLines; i++)
        {
            if (i == nLines - 1)
                aTxt.SetSize(Size(aTxt.GetWidth() / 2, aTxt.GetHeight()));
            DrawRect_Impl(aTxt, m_aTxtCol, m_aTransColor);
            aTxt.Move(0, nStep);
        }
    }

    return aRect;
}

void SvxSwFrameExample::Paint(const Rectangle&)
{
    InitAllRects_Impl();

    // Draw page
    DrawRect_Impl( aPage, m_aBgCol, m_aBorderCol );

    // Draw PrintArea
    Rectangle aRect = DrawInnerFrame_Impl( aPagePrtArea, m_aTransColor, m_aPrintAreaCol );

    if (nAnchor == TextContentAnchorType_AT_FRAME)
        aRect = DrawInnerFrame_Impl( aFrameAtFrame, m_aBgCol, m_aBorderCol );

    long lXPos    = 0;
    long lYPos    = 0;

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
       lXPos = aRect.Right() + 2;

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

    Rectangle *pOuterFrame = &aPage;

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
    USHORT nLines;

    if (nAnchor == TextContentAnchorType_AT_FRAME)
    {
        aTxt.Left() = aFrameAtFrame.Left() + FLYINFLY_BORDER;
        aTxt.Right() = aFrameAtFrame.Right() - FLYINFLY_BORDER;
        aTxt.Top() = aFrameAtFrame.Top() + FLYINFLY_BORDER;
        aTxt.Bottom() = aTxt.Top() + aTextLine.GetHeight() - 1;

        nStep = aTxt.GetHeight() + 2;
        nLines = (USHORT)(((aFrameAtFrame.GetHeight() - 2 * FLYINFLY_BORDER) * 2 / 3)
                 / (aTxt.GetHeight() + 2));
    }
    else
    {
        nStep = aTxt.GetHeight() + 2;
        nLines = (USHORT)(aParaPrtArea.GetHeight() / (aTextLine.GetHeight() + 2));
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

        for (USHORT i = 0; i < nLines; ++i)
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
                DrawRect_Impl( aTxt, m_aTxtCol, m_aTransColor );

            aTxt.Move(0, nStep);
            aTxt.Right() = nOldR;
            aTxt.Left()  = nOldL;
        }
        aTxt.Move(0, -nStep);

        if (nAnchor != TextContentAnchorType_AT_FRAME && aTxt.Bottom() > aParaPrtArea.Bottom())
        {
            // Text has been replaced by frame, so adjust parameters height
            ULONG nDiff = aTxt.Bottom() - aParaPrtArea.Bottom();
            aParaPrtArea.Bottom() += nDiff;
            aPara.Bottom() += nDiff;

            CalcBoundRect_Impl(aRect);

            aParaPrtArea.Bottom() -= nDiff;
            aPara.Bottom() -= nDiff;
        }
        if (nAnchor == TextContentAnchorType_AT_CHARACTER && bIgnoreWrap)
            DrawText(aAutoCharFrame, 'A');
    }
    else
    {
        DrawText(aParaPrtArea, C2S(DEMOTEXT));
        DrawRect_Impl(aDrawObj, m_aBlankCol, m_aBlankFrameCol );
    }

    // Draw rectangle on which the frame is aligned:
    DrawRect_Impl(aRect, m_aTransColor, m_aAlignColor);

    // Frame View
    BOOL bDontFill = (nAnchor == TextContentAnchorType_AT_CHARACTER && aFrmRect.IsOver(aAutoCharFrame)) ? TRUE : bTrans;
    DrawRect_Impl( aFrmRect, bDontFill? m_aTransColor : m_aBgCol, m_aFrameColor );
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

void SvxSwFrameExample::DrawRect_Impl(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor)
{
    SetFillColor(rFillColor);
    SetLineColor(rLineColor);
    Window::DrawRect(rRect);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
