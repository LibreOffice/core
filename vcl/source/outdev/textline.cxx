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

#include <cassert>

#include <sal/types.h>

#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <tools/helpers.hxx>

#include "salgdi.hxx"
#include "impfont.hxx"
#include "outdata.hxx"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

#define UNDERLINE_LAST      UNDERLINE_BOLDWAVE
#define STRIKEOUT_LAST      STRIKEOUT_X

bool OutputDevice::ImplIsUnderlineAbove( const vcl::Font& rFont )
{
    if ( !rFont.IsVertical() )
        return false;

    if( (LANGUAGE_JAPANESE == rFont.GetLanguage()) ||
        (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()) )
    {
        // the underline is right for Japanese only
        return true;
    }
    return false;
}

void OutputDevice::ImplInitTextLineSize()
{
    mpFontEntry->maMetric.ImplInitTextLineSize( this );
}

void OutputDevice::ImplInitAboveTextLineSize()
{
    mpFontEntry->maMetric.ImplInitAboveTextLineSize();
}

void OutputDevice::ImplDrawWavePixel( long nOriginX, long nOriginY,
                                      long nCurX, long nCurY,
                                      short nOrientation,
                                      SalGraphics* pGraphics,
                                      OutputDevice* pOutDev,
                                      bool bDrawPixAsRect,
                                      long nPixWidth, long nPixHeight )
{
    if ( nOrientation )
    {
        Point aPoint( nOriginX, nOriginY );
        aPoint.RotateAround( nCurX, nCurY, nOrientation );
    }

    if ( bDrawPixAsRect )
    {

        pGraphics->DrawRect( nCurX, nCurY, nPixWidth, nPixHeight, pOutDev );
    }
    else
    {
        pGraphics->DrawPixel( nCurX, nCurY, pOutDev );
    }
}

void OutputDevice::ImplDrawWaveLine( long nBaseX, long nBaseY,
                                     long nDistX, long nDistY,
                                     long nWidth, long nHeight,
                                     long nLineWidth, short nOrientation,
                                     const Color& rColor )
{
    if ( !nHeight )
        return;

    long nStartX = nBaseX + nDistX;
    long nStartY = nBaseY + nDistY;

    // If the height is 1 pixel, it's enough ouput a line
    if ( (nLineWidth == 1) && (nHeight == 1) )
    {
        mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
        mbInitLineColor = true;

        long nEndX = nStartX+nWidth;
        long nEndY = nStartY;
        if ( nOrientation )
        {
            Point aOriginPt( nBaseX, nBaseY );
            aOriginPt.RotateAround( nStartX, nStartY, nOrientation );
            aOriginPt.RotateAround( nEndX, nEndY, nOrientation );
        }
        mpGraphics->DrawLine( nStartX, nStartY, nEndX, nEndY, this );
    }
    else
    {
        long    nCurX = nStartX;
        long    nCurY = nStartY;
        long    nDiffX = 2;
        long    nDiffY = nHeight-1;
        long    nCount = nWidth;
        long    nOffY = -1;
        long    nPixWidth;
        long    nPixHeight;
        bool    bDrawPixAsRect;
        // On printers that ouput pixel via DrawRect()
        if ( (GetOutDevType() == OUTDEV_PRINTER) || (nLineWidth > 1) )
        {
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = true;
            }
            mpGraphics->SetFillColor( ImplColorToSal( rColor ) );
            mbInitFillColor = true;
            bDrawPixAsRect  = true;
            nPixWidth       = nLineWidth;
            nPixHeight      = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        }
        else
        {
            mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
            mbInitLineColor = true;
            nPixWidth       = 1;
            nPixHeight      = 1;
            bDrawPixAsRect  = false;
        }

        if ( !nDiffY )
        {
            while ( nWidth )
            {
                ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                   mpGraphics, this,
                                   bDrawPixAsRect, nPixWidth, nPixHeight );
                nCurX++;
                nWidth--;
            }
        }
        else
        {
            nCurY += nDiffY;
            long nFreq = nCount / (nDiffX+nDiffY);
            while ( nFreq-- )
            {
                for( long i = nDiffY; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;
                }
                for( long i = nDiffX; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
                nOffY = -nOffY;
            }
            nFreq = nCount % (nDiffX+nDiffY);
            if ( nFreq )
            {
                for( long i = nDiffY; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;

                }
                for( long i = nDiffX; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
            }
        }
    }
}

void OutputDevice::ImplDrawWaveTextLine( long nBaseX, long nBaseY,
                                         long nDistX, long nDistY, long nWidth,
                                         FontUnderline eTextLine,
                                         Color aColor,
                                         bool bIsAbove )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight;
    long            nLinePos;

    if ( bIsAbove )
    {
        nLineHeight = pFontEntry->maMetric.mnAboveWUnderlineSize;
        nLinePos = pFontEntry->maMetric.mnAboveWUnderlineOffset;
    }
    else
    {
        nLineHeight = pFontEntry->maMetric.mnWUnderlineSize;
        nLinePos = pFontEntry->maMetric.mnWUnderlineOffset;
    }
    if ( (eTextLine == UNDERLINE_SMALLWAVE) && (nLineHeight > 3) )
        nLineHeight = 3;

    long nLineWidth = (mnDPIX / 300);
    if ( !nLineWidth )
        nLineWidth = 1;

    if ( eTextLine == UNDERLINE_BOLDWAVE )
        nLineWidth *= 2;

    nLinePos += nDistY - (nLineHeight / 2);

    long nLineWidthHeight = ((nLineWidth * mnDPIX) + (mnDPIY / 2)) / mnDPIY;
    if ( eTextLine == UNDERLINE_DOUBLEWAVE )
    {
        long nOrgLineHeight = nLineHeight;
        nLineHeight /= 3;
        if ( nLineHeight < 2 )
        {
            if ( nOrgLineHeight > 1 )
                nLineHeight = 2;
            else
                nLineHeight = 1;
        }

        long nLineDY = nOrgLineHeight-(nLineHeight*2);
        if ( nLineDY < nLineWidthHeight )
            nLineDY = nLineWidthHeight;

        long nLineDY2 = nLineDY/2;
        if ( !nLineDY2 )
            nLineDY2 = 1;

        nLinePos -= nLineWidthHeight-nLineDY2;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontEntry->mnOrientation, aColor );
        nLinePos += nLineWidthHeight+nLineDY;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontEntry->mnOrientation, aColor );
    }
    else
    {
        nLinePos -= nLineWidthHeight/2;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontEntry->mnOrientation, aColor );
    }
}

void OutputDevice::ImplDrawStraightTextLine( long nBaseX, long nBaseY,
                                             long nDistX, long nDistY, long nWidth,
                                             FontUnderline eTextLine,
                                             Color aColor,
                                             bool bIsAbove )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight = 0;
    long            nLinePos  = 0;
    long            nLinePos2 = 0;

    const long nY = nDistY;

    if ( eTextLine > UNDERLINE_LAST )
        eTextLine = UNDERLINE_SINGLE;

    switch ( eTextLine )
    {
    case UNDERLINE_SINGLE:
    case UNDERLINE_DOTTED:
    case UNDERLINE_DASH:
    case UNDERLINE_LONGDASH:
    case UNDERLINE_DASHDOT:
    case UNDERLINE_DASHDOTDOT:
        if ( bIsAbove )
        {
            nLineHeight = pFontEntry->maMetric.mnAboveUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnAboveUnderlineOffset;
        }
        else
        {
            nLineHeight = pFontEntry->maMetric.mnUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnUnderlineOffset;
        }
        break;
    case UNDERLINE_BOLD:
    case UNDERLINE_BOLDDOTTED:
    case UNDERLINE_BOLDDASH:
    case UNDERLINE_BOLDLONGDASH:
    case UNDERLINE_BOLDDASHDOT:
    case UNDERLINE_BOLDDASHDOTDOT:
        if ( bIsAbove )
        {
            nLineHeight = pFontEntry->maMetric.mnAboveBUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnAboveBUnderlineOffset;
        }
        else
        {
            nLineHeight = pFontEntry->maMetric.mnBUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnBUnderlineOffset;
        }
        break;
    case UNDERLINE_DOUBLE:
        if ( bIsAbove )
        {
            nLineHeight = pFontEntry->maMetric.mnAboveDUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset1;
            nLinePos2   = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset2;
        }
        else
        {
            nLineHeight = pFontEntry->maMetric.mnDUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnDUnderlineOffset1;
            nLinePos2   = nY + pFontEntry->maMetric.mnDUnderlineOffset2;
        }
        break;
    default:
        break;
    }

    if ( nLineHeight )
    {
        if ( mbLineColor || mbInitLineColor )
        {
            mpGraphics->SetLineColor();
            mbInitLineColor = true;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aColor ) );
        mbInitFillColor = true;

        long nLeft = nDistX;

        switch ( eTextLine )
        {
        case UNDERLINE_SINGLE:
        case UNDERLINE_BOLD:
            ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            break;
        case UNDERLINE_DOUBLE:
            ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos,  nWidth, nLineHeight );
            ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
            break;
        case UNDERLINE_DOTTED:
        case UNDERLINE_BOLDDOTTED:
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;

                long nTempWidth = nDotWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempWidth > nEnd )
                        nTempWidth = nEnd-nLeft;

                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                }
            }
            break;
        case UNDERLINE_DASH:
        case UNDERLINE_LONGDASH:
        case UNDERLINE_BOLDDASH:
        case UNDERLINE_BOLDLONGDASH:
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;

                long nMinDashWidth;
                long nMinSpaceWidth;
                long nSpaceWidth;
                long nDashWidth;
                if ( (eTextLine == UNDERLINE_LONGDASH) ||
                     (eTextLine == UNDERLINE_BOLDLONGDASH) )
                {
                    nMinDashWidth = nDotWidth*6;
                    nMinSpaceWidth = nDotWidth*2;
                    nDashWidth = 200;
                    nSpaceWidth = 100;
                }
                else
                {
                    nMinDashWidth = nDotWidth*4;
                    nMinSpaceWidth = (nDotWidth*150)/100;
                    nDashWidth = 100;
                    nSpaceWidth = 50;
                }
                nDashWidth = ((nDashWidth*mnDPIX)+1270)/2540;
                nSpaceWidth = ((nSpaceWidth*mnDPIX)+1270)/2540;
                // DashWidth will be increased if the line is getting too thick
                // in proportion to the line's length
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                if ( nSpaceWidth < nMinSpaceWidth )
                    nSpaceWidth = nMinSpaceWidth;

                long nTempWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempWidth > nEnd )
                        nTempWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                    nLeft += nDashWidth+nSpaceWidth;
                }
            }
            break;
        case UNDERLINE_DASHDOT:
        case UNDERLINE_BOLDDASHDOT:
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;

                long nDashWidth = ((100*mnDPIX)+1270)/2540;
                long nMinDashWidth = nDotWidth*4;
                // DashWidth will be increased if the line is getting too thick
                // in proportion to the line's length
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;

                long nTempDotWidth = nDotWidth;
                long nTempDashWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;

                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;

                    if ( nLeft+nTempDashWidth > nEnd )
                        nTempDashWidth = nEnd-nLeft;

                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                    nLeft += nDashWidth+nDotWidth;
                }
            }
            break;
        case UNDERLINE_DASHDOTDOT:
        case UNDERLINE_BOLDDASHDOTDOT:
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;

                long nDashWidth = ((100*mnDPIX)+1270)/2540;
                long nMinDashWidth = nDotWidth*4;
                // DashWidth will be increased if the line is getting too thick
                // in proportion to the line's length
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;

                long nTempDotWidth = nDotWidth;
                long nTempDashWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;

                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;

                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;

                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;

                    if ( nLeft+nTempDashWidth > nEnd )
                        nTempDashWidth = nEnd-nLeft;

                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                    nLeft += nDashWidth+nDotWidth;
                }
            }
            break;
        default:
            break;
        }
    }
}

void OutputDevice::ImplDrawStrikeoutLine( long nBaseX, long nBaseY,
                                          long nDistX, long nDistY, long nWidth,
                                          FontStrikeout eStrikeout,
                                          Color aColor )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight = 0;
    long            nLinePos  = 0;
    long            nLinePos2 = 0;

    long nY = nDistY;

    if ( eStrikeout > STRIKEOUT_LAST )
        eStrikeout = STRIKEOUT_SINGLE;

    switch ( eStrikeout )
    {
    case STRIKEOUT_SINGLE:
        nLineHeight = pFontEntry->maMetric.mnStrikeoutSize;
        nLinePos    = nY + pFontEntry->maMetric.mnStrikeoutOffset;
        break;
    case STRIKEOUT_BOLD:
        nLineHeight = pFontEntry->maMetric.mnBStrikeoutSize;
        nLinePos    = nY + pFontEntry->maMetric.mnBStrikeoutOffset;
        break;
    case STRIKEOUT_DOUBLE:
        nLineHeight = pFontEntry->maMetric.mnDStrikeoutSize;
        nLinePos    = nY + pFontEntry->maMetric.mnDStrikeoutOffset1;
        nLinePos2   = nY + pFontEntry->maMetric.mnDStrikeoutOffset2;
        break;
    default:
        break;
    }

    if ( nLineHeight )
    {
        if ( mbLineColor || mbInitLineColor )
        {
            mpGraphics->SetLineColor();
            mbInitLineColor = true;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aColor ) );
        mbInitFillColor = true;

        const long& nLeft = nDistX;

        switch ( eStrikeout )
        {
        case STRIKEOUT_SINGLE:
        case STRIKEOUT_BOLD:
            ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            break;
        case STRIKEOUT_DOUBLE:
            ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
            break;
        default:
            break;
        }
    }
}

void OutputDevice::ImplDrawStrikeoutChar( long nBaseX, long nBaseY,
                                          long nDistX, long nDistY, long nWidth,
                                          FontStrikeout eStrikeout,
                                          Color aColor )
{
    // See qadevOOo/testdocs/StrikeThrough.odt for examples if you need
    // to tweak this
    if (!nWidth)
        return;

    // prepare string for strikeout measurement
    const char cStrikeoutChar =  eStrikeout == STRIKEOUT_SLASH ? '/' : 'X';
    static const int nTestStrLen = 4;
    static const int nMaxStrikeStrLen = 2048;
    sal_Unicode aChars[nMaxStrikeStrLen+1]; // +1 for valgrind...

    for( int i = 0; i < nTestStrLen; ++i)
        aChars[i] = cStrikeoutChar;

    const OUString aStrikeoutTest(aChars, nTestStrLen);

    // calculate approximation of strikeout atom size
    long nStrikeoutWidth = 0;
    SalLayout* pLayout = ImplLayout( aStrikeoutTest, 0, nTestStrLen );
    if( pLayout )
    {
        nStrikeoutWidth = pLayout->GetTextWidth() / (nTestStrLen * pLayout->GetUnitsPerPixel());
        pLayout->Release();
    }
    if( nStrikeoutWidth <= 0 ) // sanity check
        return;

    int nStrikeStrLen = (nWidth+(nStrikeoutWidth-1)) / nStrikeoutWidth;
    if( nStrikeStrLen > nMaxStrikeStrLen )
        nStrikeStrLen = nMaxStrikeStrLen;

    // build the strikeout string
    for( int i = nTestStrLen; i < nStrikeStrLen; ++i)
        aChars[i] = cStrikeoutChar;

    const OUString aStrikeoutText(aChars, nStrikeStrLen);

    if( mpFontEntry->mnOrientation )
    {
        Point aOriginPt(0, 0);
        aOriginPt.RotateAround( nDistX, nDistY, mpFontEntry->mnOrientation );
    }

    nBaseX += nDistX;
    nBaseY += nDistY;

    // strikeout text has to be left aligned
    ComplexTextLayoutMode nOrigTLM = mnTextLayoutMode;
    mnTextLayoutMode = TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_COMPLEX_DISABLED;
    pLayout = ImplLayout( aStrikeoutText, 0, aStrikeoutText.getLength() );
    mnTextLayoutMode = nOrigTLM;

    if( !pLayout )
        return;

    // draw the strikeout text
    const Color aOldColor = GetTextColor();
    SetTextColor( aColor );
    ImplInitTextColor();

    pLayout->DrawBase() = Point( nBaseX+mnTextOffX, nBaseY+mnTextOffY );

    Rectangle aPixelRect;
    aPixelRect.Left() = nBaseX+mnTextOffX;
    aPixelRect.Right() = aPixelRect.Left()+nWidth;
    aPixelRect.Bottom() = nBaseY+mpFontEntry->maMetric.mnDescent;
    aPixelRect.Top() = nBaseY-mpFontEntry->maMetric.mnAscent;

    if (mpFontEntry->mnOrientation)
    {
        tools::Polygon aPoly( aPixelRect );
        aPoly.Rotate( Point(nBaseX+mnTextOffX, nBaseY+mnTextOffY), mpFontEntry->mnOrientation);
        aPixelRect = aPoly.GetBoundRect();
    }

    Push( PushFlags::CLIPREGION );
    IntersectClipRegion( PixelToLogic(aPixelRect) );
    if( mbInitClipRegion )
        InitClipRegion();

    pLayout->DrawText( *mpGraphics );

    pLayout->Release();
    Pop();

    SetTextColor( aOldColor );
    ImplInitTextColor();
}

void OutputDevice::ImplDrawTextLine( long nX, long nY,
                                     long nDistX, DeviceCoordinate nWidth,
                                     FontStrikeout eStrikeout,
                                     FontUnderline eUnderline,
                                     FontUnderline eOverline,
                                     bool bUnderlineAbove )
{
    if ( !nWidth )
        return;

    Color aStrikeoutColor = GetTextColor();
    Color aUnderlineColor = GetTextLineColor();
    Color aOverlineColor  = GetOverlineColor();
    bool bStrikeoutDone = false;
    bool bUnderlineDone = false;
    bool bOverlineDone  = false;

    if ( IsRTLEnabled() )
    {
        // --- RTL --- mirror at basex
        long nXAdd = nWidth - nDistX;
        if( mpFontEntry->mnOrientation )
            nXAdd = FRound( nXAdd * cos( mpFontEntry->mnOrientation * F_PI1800 ) );

        nX += nXAdd - 1;
    }

    if ( !IsTextLineColor() )
        aUnderlineColor = GetTextColor();

    if ( !IsOverlineColor() )
        aOverlineColor = GetTextColor();

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        ImplDrawWaveTextLine( nX, nY, nDistX, 0, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
        bUnderlineDone = true;
    }
    if ( (eOverline == UNDERLINE_SMALLWAVE) ||
         (eOverline == UNDERLINE_WAVE) ||
         (eOverline == UNDERLINE_DOUBLEWAVE) ||
         (eOverline == UNDERLINE_BOLDWAVE) )
    {
        ImplDrawWaveTextLine( nX, nY, nDistX, 0, nWidth, eOverline, aOverlineColor, true );
        bOverlineDone = true;
    }

    if ( (eStrikeout == STRIKEOUT_SLASH) ||
         (eStrikeout == STRIKEOUT_X) )
    {
        ImplDrawStrikeoutChar( nX, nY, nDistX, 0, nWidth, eStrikeout, aStrikeoutColor );
        bStrikeoutDone = true;
    }

    if ( !bUnderlineDone )
        ImplDrawStraightTextLine( nX, nY, nDistX, 0, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );

    if ( !bOverlineDone )
        ImplDrawStraightTextLine( nX, nY, nDistX, 0, nWidth, eOverline, aOverlineColor, true );

    if ( !bStrikeoutDone )
        ImplDrawStrikeoutLine( nX, nY, nDistX, 0, nWidth, eStrikeout, aStrikeoutColor );
}

void OutputDevice::ImplDrawTextLines( SalLayout& rSalLayout, FontStrikeout eStrikeout,
                                      FontUnderline eUnderline, FontUnderline eOverline,
                                      bool bWordLine, bool bUnderlineAbove )
{
    if( bWordLine )
    {
        // draw everything relative to the layout base point
        const Point aStartPt = rSalLayout.DrawBase();

        // calculate distance of each word from the base point
        Point aPos;
        DeviceCoordinate nDist = 0;
        DeviceCoordinate nWidth = 0;
        DeviceCoordinate nAdvance = 0;
        for( int nStart = 0;;)
        {
            // iterate through the layouted glyphs
            sal_GlyphId aGlyphId;
            if( !rSalLayout.GetNextGlyphs( 1, &aGlyphId, aPos, nStart, &nAdvance ) )
                break;

            // calculate the boundaries of each word
            if( !SalLayout::IsSpacingGlyph( aGlyphId ) )
            {
                if( !nWidth )
                {
                    // get the distance to the base point (as projected to baseline)
                    nDist = aPos.X() - aStartPt.X();
                    if( mpFontEntry->mnOrientation )
                    {
                        const long nDY = aPos.Y() - aStartPt.Y();
                        const double fRad = mpFontEntry->mnOrientation * F_PI1800;
                        nDist = FRound( nDist*cos(fRad) - nDY*sin(fRad) );
                    }
                }

                // update the length of the textline
                nWidth += nAdvance;
            }
            else if( nWidth > 0 )
            {
                // draw the textline for each word
                ImplDrawTextLine( aStartPt.X(), aStartPt.Y(), nDist, nWidth,
                                  eStrikeout, eUnderline, eOverline, bUnderlineAbove );
                nWidth = 0;
            }
        }

        // draw textline for the last word
        if( nWidth > 0 )
        {
            ImplDrawTextLine( aStartPt.X(), aStartPt.Y(), nDist, nWidth,
                              eStrikeout, eUnderline, eOverline, bUnderlineAbove );
        }
    }
    else
    {
        Point aStartPt = rSalLayout.GetDrawPosition();
        ImplDrawTextLine( aStartPt.X(), aStartPt.Y(), 0,
                          rSalLayout.GetTextWidth() / rSalLayout.GetUnitsPerPixel(),
                          eStrikeout, eUnderline, eOverline, bUnderlineAbove );
    }
}

void OutputDevice::ImplDrawMnemonicLine( long nX, long nY, long nWidth )
{
    long nBaseX = nX;
    if( /*HasMirroredGraphics() &&*/ IsRTLEnabled() )
    {
        // --- RTL ---
        // add some strange offset
        nX += 2;
        // revert the hack that will be done later in ImplDrawTextLine
        nX = nBaseX - nWidth - (nX - nBaseX - 1);
    }

    ImplDrawTextLine( nX, nY, 0, nWidth, STRIKEOUT_NONE, UNDERLINE_SINGLE, UNDERLINE_NONE, false );
}

void OutputDevice::SetTextLineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( Color(), false ) );

    maTextLineColor = Color( COL_TRANSPARENT );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor();
}

void OutputDevice::SetTextLineColor( const Color& rColor )
{

    Color aColor( rColor );

    if ( mnDrawMode & ( DrawModeFlags::BlackText | DrawModeFlags::WhiteText |
                        DrawModeFlags::GrayText | DrawModeFlags::GhostedText |
                        DrawModeFlags::SettingsText ) )
    {
        if ( mnDrawMode & DrawModeFlags::BlackText )
        {
            aColor = Color( COL_BLACK );
        }
        else if ( mnDrawMode & DrawModeFlags::WhiteText )
        {
            aColor = Color( COL_WHITE );
        }
        else if ( mnDrawMode & DrawModeFlags::GrayText )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DrawModeFlags::SettingsText )
        {
            aColor = GetSettings().GetStyleSettings().GetFontColor();
        }

        if( (mnDrawMode & DrawModeFlags::GhostedText) &&
            (aColor.GetColor() != COL_TRANSPARENT) )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( aColor, true ) );

    maTextLineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor( COL_BLACK );
}

void OutputDevice::SetOverlineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( Color(), false ) );

    maOverlineColor = Color( COL_TRANSPARENT );

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor();
}

void OutputDevice::SetOverlineColor( const Color& rColor )
{

    Color aColor( rColor );

    if ( mnDrawMode & ( DrawModeFlags::BlackText | DrawModeFlags::WhiteText |
                        DrawModeFlags::GrayText | DrawModeFlags::GhostedText |
                        DrawModeFlags::SettingsText ) )
    {
        if ( mnDrawMode & DrawModeFlags::BlackText )
        {
            aColor = Color( COL_BLACK );
        }
        else if ( mnDrawMode & DrawModeFlags::WhiteText )
        {
            aColor = Color( COL_WHITE );
        }
        else if ( mnDrawMode & DrawModeFlags::GrayText )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DrawModeFlags::SettingsText )
        {
            aColor = GetSettings().GetStyleSettings().GetFontColor();
        }

        if( (mnDrawMode & DrawModeFlags::GhostedText) &&
            (aColor.GetColor() != COL_TRANSPARENT) )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( aColor, true ) );

    maOverlineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor( COL_BLACK );
}

void OutputDevice::DrawTextLine( const Point& rPos, long nWidth,
                                 FontStrikeout eStrikeout,
                                 FontUnderline eUnderline,
                                 FontUnderline eOverline,
                                 bool bUnderlineAbove )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline, eOverline ) );

    if ( ((eUnderline == UNDERLINE_NONE) || (eUnderline == UNDERLINE_DONTKNOW)) &&
         ((eOverline  == UNDERLINE_NONE) || (eOverline  == UNDERLINE_DONTKNOW)) &&
         ((eStrikeout == STRIKEOUT_NONE) || (eStrikeout == STRIKEOUT_DONTKNOW)) )
    {
        return;
    }
    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    // initialize font if needed to get text offsets
    // TODO: only needed for mnTextOff!=(0,0)
    if( mbNewFont && !ImplNewFont() )
        return;

    if( mbInitFont )
        InitFont();

    Point aPos = ImplLogicToDevicePixel( rPos );
    DeviceCoordinate fWidth;
    fWidth = LogicWidthToDeviceCoordinate( nWidth );
    aPos += Point( mnTextOffX, mnTextOffY );
    ImplDrawTextLine( aPos.X(), aPos.X(), 0, fWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawTextLine( rPos, nWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );
}

void OutputDevice::DrawWaveLine( const Point& rStartPos, const Point& rEndPos )
{
    assert(!is_double_buffered_window());

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if( mbNewFont && !ImplNewFont() )
        return;

    Point   aStartPt = ImplLogicToDevicePixel( rStartPos );
    Point   aEndPt = ImplLogicToDevicePixel( rEndPos );
    long    nStartX = aStartPt.X();
    long    nStartY = aStartPt.Y();
    long    nEndX = aEndPt.X();
    long    nEndY = aEndPt.Y();
    short   nOrientation = 0;

    // when rotated
    if ( (nStartY != nEndY) || (nStartX > nEndX) )
    {
        long nDX = nEndX - nStartX;
        double nO = atan2( -nEndY + nStartY, ((nDX == 0L) ? 0.000000001 : nDX) );
        nO /= F_PI1800;
        nOrientation = (short)nO;
        aStartPt.RotateAround( nEndX, nEndY, -nOrientation );
    }

    long nWaveHeight;

    nWaveHeight = 3;
    nStartY++;
    nEndY++;

    if (mnDPIScaleFactor > 1)
    {
        nWaveHeight *= mnDPIScaleFactor;

        nStartY += mnDPIScaleFactor - 1; // Shift down additional pixel(s) to create more visual separation.

        // odd heights look better than even
        if (mnDPIScaleFactor % 2 == 0)
        {
            nWaveHeight--;
        }
    }

    // #109280# make sure the waveline does not exceed the descent to avoid paint problems
    ImplFontEntry* pFontEntry = mpFontEntry;
    if( nWaveHeight > pFontEntry->maMetric.mnWUnderlineSize )
    {
        nWaveHeight = pFontEntry->maMetric.mnWUnderlineSize;
    }
    ImplDrawWaveLine(nStartX, nStartY, 0, 0,
                     nEndX-nStartX, nWaveHeight,
                     mnDPIScaleFactor, nOrientation, GetLineColor());

    if( mpAlphaVDev )
        mpAlphaVDev->DrawWaveLine( rStartPos, rEndPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
