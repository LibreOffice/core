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
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lazydelete.hxx>

#include <tools/helpers.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>
#include <impglyphitem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/WaveLine.hxx>
#include <o3tl/hash_combine.hxx>
#include <o3tl/lru_map.hxx>

#define UNDERLINE_LAST      LINESTYLE_BOLDWAVE
#define STRIKEOUT_LAST      STRIKEOUT_X

namespace {
    struct WavyLineCache final
    {
        WavyLineCache () : m_aItems( 10 ) {}

        bool find( Color aLineColor, size_t nLineWidth, size_t nWaveHeight, size_t nWordWidth, BitmapEx& rOutput )
        {
            Key aKey = { nWaveHeight, sal_uInt32(aLineColor) };
            auto item = m_aItems.find( aKey );
            if ( item == m_aItems.end() )
                return false;
            // needs update
            if ( item->second.m_aLineWidth != nLineWidth || item->second.m_aWordWidth < nWordWidth )
            {
                return false;
            }
            rOutput = item->second.m_Bitmap;
            return true;
        }

        void insert( const BitmapEx& aBitmap, const Color& aLineColor, const size_t nLineWidth, const size_t nWaveHeight, const size_t nWordWidth, BitmapEx& rOutput )
        {
            Key aKey = { nWaveHeight, sal_uInt32(aLineColor) };
            m_aItems.insert( std::pair< Key, WavyLineCacheItem>( aKey, { nLineWidth, nWordWidth, aBitmap } ) );
            rOutput = aBitmap;
        }

        private:
        struct WavyLineCacheItem
        {
            size_t m_aLineWidth;
            size_t m_aWordWidth;
            BitmapEx m_Bitmap;
        };

        struct Key
        {
            size_t m_aFirst;
            size_t m_aSecond;
            bool operator ==( const Key& rOther ) const
            {
                return ( m_aFirst == rOther.m_aFirst && m_aSecond == rOther.m_aSecond );
            }
        };

        struct Hash
        {
            size_t operator() ( const Key& rKey ) const
            {
                size_t aSeed = 0;
                o3tl::hash_combine(aSeed, rKey.m_aFirst);
                o3tl::hash_combine(aSeed, rKey.m_aSecond);
                return aSeed;
            }
        };

        o3tl::lru_map< Key, WavyLineCacheItem, Hash > m_aItems;
    };
}

void OutputDevice::ImplInitTextLineSize()
{
    mpFontInstance->mxFontMetric->ImplInitTextLineSize( this );
}

void OutputDevice::ImplInitAboveTextLineSize()
{
    mpFontInstance->mxFontMetric->ImplInitAboveTextLineSize();
}

void OutputDevice::ImplDrawWavePixel( tools::Long nOriginX, tools::Long nOriginY,
                                      tools::Long nCurX, tools::Long nCurY,
                                      tools::Long nWidth,
                                      Degree10 nOrientation,
                                      SalGraphics* pGraphics,
                                      const OutputDevice& rOutDev,
                                      tools::Long nPixWidth, tools::Long nPixHeight )
{
    if (nOrientation)
    {
        Point aPoint( nOriginX, nOriginY );
        aPoint.RotateAround( nCurX, nCurY, nOrientation );
    }

    if (shouldDrawWavePixelAsRect(nWidth))
    {
        pGraphics->DrawRect( nCurX, nCurY, nPixWidth, nPixHeight, rOutDev );
    }
    else
    {
        pGraphics->DrawPixel( nCurX, nCurY, rOutDev );
    }
}

bool OutputDevice::shouldDrawWavePixelAsRect(tools::Long nLineWidth) const
{
    if (nLineWidth > 1)
        return true;

    return false;
}

void OutputDevice::SetWaveLineColors(Color const& rColor, tools::Long nLineWidth)
{
    // On printers that output pixel via DrawRect()
    if (nLineWidth > 1)
    {
        if (mbLineColor || mbInitLineColor)
        {
            mpGraphics->SetLineColor();
            mbInitLineColor = true;
        }

        mpGraphics->SetFillColor( rColor );
        mbInitFillColor = true;
    }
    else
    {
        mpGraphics->SetLineColor( rColor );
        mbInitLineColor = true;
    }
}

Size OutputDevice::GetWaveLineSize(tools::Long nLineWidth) const
{
    if (nLineWidth > 1)
        return Size(nLineWidth, ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY);

    return Size(1, 1);
}

void OutputDevice::ImplDrawWaveLine( tools::Long nBaseX, tools::Long nBaseY,
                                     tools::Long nDistX, tools::Long nDistY,
                                     tools::Long nWidth, tools::Long nHeight,
                                     tools::Long nLineWidth, Degree10 nOrientation,
                                     const Color& rColor )
{
    if ( !nHeight )
        return;

    tools::Long nStartX = nBaseX + nDistX;
    tools::Long nStartY = nBaseY + nDistY;

    // If the height is 1 pixel, it's enough output a line
    if ( (nLineWidth == 1) && (nHeight == 1) )
    {
        mpGraphics->SetLineColor( rColor );
        mbInitLineColor = true;

        tools::Long nEndX = nStartX+nWidth;
        tools::Long nEndY = nStartY;
        if ( nOrientation )
        {
            Point aOriginPt( nBaseX, nBaseY );
            aOriginPt.RotateAround( nStartX, nStartY, nOrientation );
            aOriginPt.RotateAround( nEndX, nEndY, nOrientation );
        }
        mpGraphics->DrawLine( nStartX, nStartY, nEndX, nEndY, *this );
    }
    else
    {
        tools::Long    nCurX = nStartX;
        tools::Long    nCurY = nStartY;
        tools::Long    nDiffX = 2;
        tools::Long    nDiffY = nHeight-1;
        tools::Long    nCount = nWidth;
        tools::Long    nOffY = -1;

        SetWaveLineColors(rColor, nLineWidth);
        Size aSize(GetWaveLineSize(nLineWidth));

        tools::Long nPixWidth = aSize.Width();
        tools::Long nPixHeight = aSize.Height();

        if ( !nDiffY )
        {
            while ( nWidth )
            {
                ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nLineWidth, nOrientation,
                                   mpGraphics, *this,
                                   nPixWidth, nPixHeight );
                nCurX++;
                nWidth--;
            }
        }
        else
        {
            nCurY += nDiffY;
            tools::Long nFreq = nCount / (nDiffX+nDiffY);
            while ( nFreq-- )
            {
                for( tools::Long i = nDiffY; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nLineWidth, nOrientation,
                                       mpGraphics, *this,
                                       nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;
                }
                for( tools::Long i = nDiffX; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nLineWidth, nOrientation,
                                       mpGraphics, *this,
                                       nPixWidth, nPixHeight );
                    nCurX++;
                }
                nOffY = -nOffY;
            }
            nFreq = nCount % (nDiffX+nDiffY);
            if ( nFreq )
            {
                for( tools::Long i = nDiffY; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nLineWidth, nOrientation,
                                       mpGraphics, *this,
                                       nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;

                }
                for( tools::Long i = nDiffX; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nLineWidth, nOrientation,
                                       mpGraphics, *this,
                                       nPixWidth, nPixHeight );
                    nCurX++;
                }
            }
        }
    }
}

void OutputDevice::ImplDrawWaveTextLine( tools::Long nBaseX, tools::Long nBaseY,
                                         tools::Long nDistX, tools::Long nDistY, tools::Long nWidth,
                                         FontLineStyle eTextLine,
                                         Color aColor,
                                         bool bIsAbove )
{
    LogicalFontInstance* pFontInstance = mpFontInstance.get();
    tools::Long            nLineHeight;
    tools::Long            nLinePos;

    if ( bIsAbove )
    {
        nLineHeight = pFontInstance->mxFontMetric->GetAboveWavelineUnderlineSize();
        nLinePos = pFontInstance->mxFontMetric->GetAboveWavelineUnderlineOffset();
    }
    else
    {
        nLineHeight = pFontInstance->mxFontMetric->GetWavelineUnderlineSize();
        nLinePos = pFontInstance->mxFontMetric->GetWavelineUnderlineOffset();
    }
    if ( (eTextLine == LINESTYLE_SMALLWAVE) && (nLineHeight > 3) )
        nLineHeight = 3;

    tools::Long nLineWidth = mnDPIX / 300;
    if ( !nLineWidth )
        nLineWidth = 1;

    if ( eTextLine == LINESTYLE_BOLDWAVE )
        nLineWidth *= 2;

    nLinePos += nDistY - (nLineHeight / 2);

    tools::Long nLineWidthHeight = ((nLineWidth * mnDPIX) + (mnDPIY / 2)) / mnDPIY;
    if ( eTextLine == LINESTYLE_DOUBLEWAVE )
    {
        tools::Long nOrgLineHeight = nLineHeight;
        nLineHeight /= 3;
        if ( nLineHeight < 2 )
        {
            if ( nOrgLineHeight > 1 )
                nLineHeight = 2;
            else
                nLineHeight = 1;
        }

        tools::Long nLineDY = nOrgLineHeight-(nLineHeight*2);
        if ( nLineDY < nLineWidthHeight )
            nLineDY = nLineWidthHeight;

        tools::Long nLineDY2 = nLineDY/2;
        if ( !nLineDY2 )
            nLineDY2 = 1;

        nLinePos -= nLineWidthHeight-nLineDY2;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontInstance->mnOrientation, aColor );
        nLinePos += nLineWidthHeight+nLineDY;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontInstance->mnOrientation, aColor );
    }
    else
    {
        nLinePos -= nLineWidthHeight/2;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontInstance->mnOrientation, aColor );
    }
}

void OutputDevice::ImplDrawStraightTextLine( tools::Long nBaseX, tools::Long nBaseY,
                                             tools::Long nDistX, tools::Long nDistY, tools::Long nWidth,
                                             FontLineStyle eTextLine,
                                             Color aColor,
                                             bool bIsAbove )
{
    LogicalFontInstance*  pFontInstance = mpFontInstance.get();
    tools::Long            nLineHeight = 0;
    tools::Long            nLinePos  = 0;
    tools::Long            nLinePos2 = 0;

    const tools::Long nY = nDistY;

    if ( eTextLine > UNDERLINE_LAST )
        eTextLine = LINESTYLE_SINGLE;

    switch ( eTextLine )
    {
    case LINESTYLE_SINGLE:
    case LINESTYLE_DOTTED:
    case LINESTYLE_DASH:
    case LINESTYLE_LONGDASH:
    case LINESTYLE_DASHDOT:
    case LINESTYLE_DASHDOTDOT:
        if ( bIsAbove )
        {
            nLineHeight = pFontInstance->mxFontMetric->GetAboveUnderlineSize();
            nLinePos    = nY + pFontInstance->mxFontMetric->GetAboveUnderlineOffset();
        }
        else
        {
            nLineHeight = pFontInstance->mxFontMetric->GetUnderlineSize();
            nLinePos    = nY + pFontInstance->mxFontMetric->GetUnderlineOffset();
        }
        break;
    case LINESTYLE_BOLD:
    case LINESTYLE_BOLDDOTTED:
    case LINESTYLE_BOLDDASH:
    case LINESTYLE_BOLDLONGDASH:
    case LINESTYLE_BOLDDASHDOT:
    case LINESTYLE_BOLDDASHDOTDOT:
        if ( bIsAbove )
        {
            nLineHeight = pFontInstance->mxFontMetric->GetAboveBoldUnderlineSize();
            nLinePos    = nY + pFontInstance->mxFontMetric->GetAboveBoldUnderlineOffset();
        }
        else
        {
            nLineHeight = pFontInstance->mxFontMetric->GetBoldUnderlineSize();
            nLinePos    = nY + pFontInstance->mxFontMetric->GetBoldUnderlineOffset();
        }
        break;
    case LINESTYLE_DOUBLE:
        if ( bIsAbove )
        {
            nLineHeight = pFontInstance->mxFontMetric->GetAboveDoubleUnderlineSize();
            nLinePos    = nY + pFontInstance->mxFontMetric->GetAboveDoubleUnderlineOffset1();
            nLinePos2   = nY + pFontInstance->mxFontMetric->GetAboveDoubleUnderlineOffset2();
        }
        else
        {
            nLineHeight = pFontInstance->mxFontMetric->GetDoubleUnderlineSize();
            nLinePos    = nY + pFontInstance->mxFontMetric->GetDoubleUnderlineOffset1();
            nLinePos2   = nY + pFontInstance->mxFontMetric->GetDoubleUnderlineOffset2();
        }
        break;
    default:
        break;
    }

    if ( !nLineHeight )
        return;

    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = true;
    }
    mpGraphics->SetFillColor( aColor );
    mbInitFillColor = true;

    tools::Long nLeft = nDistX;

    switch ( eTextLine )
    {
    case LINESTYLE_SINGLE:
    case LINESTYLE_BOLD:
        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
        break;
    case LINESTYLE_DOUBLE:
        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos,  nWidth, nLineHeight );
        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
        break;
    case LINESTYLE_DOTTED:
    case LINESTYLE_BOLDDOTTED:
        {
            tools::Long nDotWidth = nLineHeight*mnDPIY;
            nDotWidth += mnDPIY/2;
            nDotWidth /= mnDPIY;

            tools::Long nTempWidth = nDotWidth;
            tools::Long nEnd = nLeft+nWidth;
            while ( nLeft < nEnd )
            {
                if ( nLeft+nTempWidth > nEnd )
                    nTempWidth = nEnd-nLeft;

                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                nLeft += nDotWidth*2;
            }
        }
        break;
    case LINESTYLE_DASH:
    case LINESTYLE_LONGDASH:
    case LINESTYLE_BOLDDASH:
    case LINESTYLE_BOLDLONGDASH:
        {
            tools::Long nDotWidth = nLineHeight*mnDPIY;
            nDotWidth += mnDPIY/2;
            nDotWidth /= mnDPIY;

            tools::Long nMinDashWidth;
            tools::Long nMinSpaceWidth;
            tools::Long nSpaceWidth;
            tools::Long nDashWidth;
            if ( (eTextLine == LINESTYLE_LONGDASH) ||
                 (eTextLine == LINESTYLE_BOLDLONGDASH) )
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

            tools::Long nTempWidth = nDashWidth;
            tools::Long nEnd = nLeft+nWidth;
            while ( nLeft < nEnd )
            {
                if ( nLeft+nTempWidth > nEnd )
                    nTempWidth = nEnd-nLeft;
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                nLeft += nDashWidth+nSpaceWidth;
            }
        }
        break;
    case LINESTYLE_DASHDOT:
    case LINESTYLE_BOLDDASHDOT:
        {
            tools::Long nDotWidth = nLineHeight*mnDPIY;
            nDotWidth += mnDPIY/2;
            nDotWidth /= mnDPIY;

            tools::Long nDashWidth = ((100*mnDPIX)+1270)/2540;
            tools::Long nMinDashWidth = nDotWidth*4;
            // DashWidth will be increased if the line is getting too thick
            // in proportion to the line's length
            if ( nDashWidth < nMinDashWidth )
                nDashWidth = nMinDashWidth;

            tools::Long nTempDotWidth = nDotWidth;
            tools::Long nTempDashWidth = nDashWidth;
            tools::Long nEnd = nLeft+nWidth;
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
    case LINESTYLE_DASHDOTDOT:
    case LINESTYLE_BOLDDASHDOTDOT:
        {
            tools::Long nDotWidth = nLineHeight*mnDPIY;
            nDotWidth += mnDPIY/2;
            nDotWidth /= mnDPIY;

            tools::Long nDashWidth = ((100*mnDPIX)+1270)/2540;
            tools::Long nMinDashWidth = nDotWidth*4;
            // DashWidth will be increased if the line is getting too thick
            // in proportion to the line's length
            if ( nDashWidth < nMinDashWidth )
                nDashWidth = nMinDashWidth;

            tools::Long nTempDotWidth = nDotWidth;
            tools::Long nTempDashWidth = nDashWidth;
            tools::Long nEnd = nLeft+nWidth;
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

void OutputDevice::ImplDrawStrikeoutLine( tools::Long nBaseX, tools::Long nBaseY,
                                          tools::Long nDistX, tools::Long nDistY, tools::Long nWidth,
                                          FontStrikeout eStrikeout,
                                          Color aColor )
{
    LogicalFontInstance*  pFontInstance = mpFontInstance.get();
    tools::Long            nLineHeight = 0;
    tools::Long            nLinePos  = 0;
    tools::Long            nLinePos2 = 0;

    tools::Long nY = nDistY;

    if ( eStrikeout > STRIKEOUT_LAST )
        eStrikeout = STRIKEOUT_SINGLE;

    switch ( eStrikeout )
    {
    case STRIKEOUT_SINGLE:
        nLineHeight = pFontInstance->mxFontMetric->GetStrikeoutSize();
        nLinePos    = nY + pFontInstance->mxFontMetric->GetStrikeoutOffset();
        break;
    case STRIKEOUT_BOLD:
        nLineHeight = pFontInstance->mxFontMetric->GetBoldStrikeoutSize();
        nLinePos    = nY + pFontInstance->mxFontMetric->GetBoldStrikeoutOffset();
        break;
    case STRIKEOUT_DOUBLE:
        nLineHeight = pFontInstance->mxFontMetric->GetDoubleStrikeoutSize();
        nLinePos    = nY + pFontInstance->mxFontMetric->GetDoubleStrikeoutOffset1();
        nLinePos2   = nY + pFontInstance->mxFontMetric->GetDoubleStrikeoutOffset2();
        break;
    default:
        break;
    }

    if ( !nLineHeight )
        return;

    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = true;
    }
    mpGraphics->SetFillColor( aColor );
    mbInitFillColor = true;

    const tools::Long& nLeft = nDistX;

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

void OutputDevice::ImplDrawStrikeoutChar( tools::Long nBaseX, tools::Long nBaseY,
                                          tools::Long nDistX, tools::Long nDistY, tools::Long nWidth,
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
    tools::Long nStrikeoutWidth = 0;
    std::unique_ptr<SalLayout> pLayout = ImplLayout( aStrikeoutTest, 0, nTestStrLen );
    if( pLayout )
    {
        nStrikeoutWidth = pLayout->GetTextWidth() / (nTestStrLen * pLayout->GetUnitsPerPixel());
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

    if( mpFontInstance->mnOrientation )
    {
        Point aOriginPt(0, 0);
        aOriginPt.RotateAround( nDistX, nDistY, mpFontInstance->mnOrientation );
    }

    nBaseX += nDistX;
    nBaseY += nDistY;

    // strikeout text has to be left aligned
    vcl::text::ComplexTextLayoutFlags nOrigTLM = mnTextLayoutMode;
    mnTextLayoutMode = vcl::text::ComplexTextLayoutFlags::BiDiStrong;
    pLayout = ImplLayout( aStrikeoutText, 0, aStrikeoutText.getLength() );
    mnTextLayoutMode = nOrigTLM;

    if( !pLayout )
        return;

    // draw the strikeout text
    const Color aOldColor = GetTextColor();
    SetTextColor( aColor );
    ImplInitTextColor();

    pLayout->DrawBase() = Point( nBaseX+mnTextOffX, nBaseY+mnTextOffY );

    tools::Rectangle aPixelRect;
    aPixelRect.SetLeft( nBaseX+mnTextOffX );
    aPixelRect.SetRight( aPixelRect.Left()+nWidth );
    aPixelRect.SetBottom( nBaseY+mpFontInstance->mxFontMetric->GetDescent() );
    aPixelRect.SetTop( nBaseY-mpFontInstance->mxFontMetric->GetAscent() );

    if (mpFontInstance->mnOrientation)
    {
        tools::Polygon aPoly( aPixelRect );
        aPoly.Rotate( Point(nBaseX+mnTextOffX, nBaseY+mnTextOffY), mpFontInstance->mnOrientation);
        aPixelRect = aPoly.GetBoundRect();
    }

    Push( vcl::PushFlags::CLIPREGION );
    IntersectClipRegion( PixelToLogic(aPixelRect) );
    if( mbInitClipRegion )
        InitClipRegion();

    pLayout->DrawText( *mpGraphics );

    Pop();

    SetTextColor( aOldColor );
    ImplInitTextColor();
}

void OutputDevice::ImplDrawTextLine( tools::Long nX, tools::Long nY,
                                     tools::Long nDistX, DeviceCoordinate nWidth,
                                     FontStrikeout eStrikeout,
                                     FontLineStyle eUnderline,
                                     FontLineStyle eOverline,
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
        tools::Long nXAdd = nWidth - nDistX;
        if( mpFontInstance->mnOrientation )
            nXAdd = FRound( nXAdd * cos( toRadians(mpFontInstance->mnOrientation) ) );

        nX += nXAdd - 1;
    }

    if ( !IsTextLineColor() )
        aUnderlineColor = GetTextColor();

    if ( !IsOverlineColor() )
        aOverlineColor = GetTextColor();

    if ( (eUnderline == LINESTYLE_SMALLWAVE) ||
         (eUnderline == LINESTYLE_WAVE) ||
         (eUnderline == LINESTYLE_DOUBLEWAVE) ||
         (eUnderline == LINESTYLE_BOLDWAVE) )
    {
        ImplDrawWaveTextLine( nX, nY, nDistX, 0, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
        bUnderlineDone = true;
    }
    if ( (eOverline == LINESTYLE_SMALLWAVE) ||
         (eOverline == LINESTYLE_WAVE) ||
         (eOverline == LINESTYLE_DOUBLEWAVE) ||
         (eOverline == LINESTYLE_BOLDWAVE) )
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
                                      FontLineStyle eUnderline, FontLineStyle eOverline,
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
        const GlyphItem* pGlyph;
        int nStart = 0;
        while (rSalLayout.GetNextGlyph(&pGlyph, aPos, nStart))
        {
            // calculate the boundaries of each word
            if (!pGlyph->IsSpacing())
            {
                if( !nWidth )
                {
                    // get the distance to the base point (as projected to baseline)
                    nDist = aPos.X() - aStartPt.X();
                    if( mpFontInstance->mnOrientation )
                    {
                        const tools::Long nDY = aPos.Y() - aStartPt.Y();
                        const double fRad = toRadians(mpFontInstance->mnOrientation);
                        nDist = FRound( nDist*cos(fRad) - nDY*sin(fRad) );
                    }
                }

                // update the length of the textline
                nWidth += pGlyph->m_nNewWidth;
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

void OutputDevice::ImplDrawMnemonicLine( tools::Long nX, tools::Long nY, tools::Long nWidth )
{
    tools::Long nBaseX = nX;
    if( /*HasMirroredGraphics() &&*/ IsRTLEnabled() )
    {
        // add some strange offset
        nX += 2;
        // revert the hack that will be done later in ImplDrawTextLine
        nX = nBaseX - nWidth - (nX - nBaseX - 1);
    }

    ImplDrawTextLine( nX, nY, 0, nWidth, STRIKEOUT_NONE, LINESTYLE_SINGLE, LINESTYLE_NONE, false );
}

void OutputDevice::SetTextLineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( Color(), false ) );

    maTextLineColor = COL_TRANSPARENT;

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor();
}

void OutputDevice::SetTextLineColor( const Color& rColor )
{
    Color aColor(vcl::drawmode::GetTextColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings()));

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( aColor, true ) );

    maTextLineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor( COL_ALPHA_OPAQUE );
}

void OutputDevice::SetOverlineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( Color(), false ) );

    maOverlineColor = COL_TRANSPARENT;

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor();
}

void OutputDevice::SetOverlineColor( const Color& rColor )
{
    Color aColor(vcl::drawmode::GetTextColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings()));

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( aColor, true ) );

    maOverlineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor( COL_ALPHA_OPAQUE );
}

void OutputDevice::DrawTextLine( const Point& rPos, tools::Long nWidth,
                                 FontStrikeout eStrikeout,
                                 FontLineStyle eUnderline,
                                 FontLineStyle eOverline,
                                 bool bUnderlineAbove )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline, eOverline ) );

    if ( ((eUnderline == LINESTYLE_NONE) || (eUnderline == LINESTYLE_DONTKNOW)) &&
         ((eOverline  == LINESTYLE_NONE) || (eOverline  == LINESTYLE_DONTKNOW)) &&
         ((eStrikeout == STRIKEOUT_NONE) || (eStrikeout == STRIKEOUT_DONTKNOW)) )
    {
        return;
    }
    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    // initialize font if needed to get text offsets
    // TODO: only needed for mnTextOff!=(0,0)
    if (!InitFont())
        return;

    Point aPos = ImplLogicToDevicePixel( rPos );
    DeviceCoordinate fWidth;
    fWidth = LogicWidthToDeviceCoordinate( nWidth );
    aPos += Point( mnTextOffX, mnTextOffY );
    ImplDrawTextLine( aPos.X(), aPos.X(), 0, fWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawTextLine( rPos, nWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );
}

void OutputDevice::DrawWaveLine(const Point& rStartPos, const Point& rEndPos, tools::Long nLineWidth, tools::Long nWaveHeight)
{
    assert(!is_double_buffered_window());

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if (!InitFont())
        return;

    Point aStartPt = ImplLogicToDevicePixel(rStartPos);
    Point aEndPt = ImplLogicToDevicePixel(rEndPos);

    tools::Long nStartX = aStartPt.X();
    tools::Long nStartY = aStartPt.Y();
    tools::Long nEndX = aEndPt.X();
    tools::Long nEndY = aEndPt.Y();
    double fOrientation = 0.0;

    // handle rotation
    if (nStartY != nEndY || nStartX > nEndX)
    {
        fOrientation = basegfx::rad2deg(std::atan2(nStartY - nEndY, nEndX - nStartX));
        // un-rotate the end point
        aStartPt.RotateAround(nEndX, nEndY, Degree10(static_cast<sal_Int16>(-fOrientation * 10.0)));
    }

    // Handle HiDPI
    float fScaleFactor = GetDPIScaleFactor();
    if (fScaleFactor > 1.0f)
    {
        nWaveHeight *= fScaleFactor;

        nStartY += fScaleFactor - 1; // Shift down additional pixel(s) to create more visual separation.

        // odd heights look better than even
        if (nWaveHeight % 2 == 0)
        {
            nWaveHeight--;
        }
    }

    // #109280# make sure the waveline does not exceed the descent to avoid paint problems
    LogicalFontInstance* pFontInstance = mpFontInstance.get();
    if (nWaveHeight > pFontInstance->mxFontMetric->GetWavelineUnderlineSize())
    {
        nWaveHeight = pFontInstance->mxFontMetric->GetWavelineUnderlineSize();
        // tdf#124848 hairline
        nLineWidth = 0;
    }

    if ( fOrientation == 0.0 )
    {
        static vcl::DeleteOnDeinit< WavyLineCache > snLineCache {};
        if ( !snLineCache.get() )
            return;
        WavyLineCache& rLineCache = *snLineCache.get();
        BitmapEx aWavylinebmp;
        if ( !rLineCache.find( GetLineColor(), nLineWidth, nWaveHeight, nEndX - nStartX, aWavylinebmp ) )
        {
            size_t nWordLength = nEndX - nStartX;
            // start with something big to avoid updating it frequently
            nWordLength = nWordLength < 1024 ? 1024 : nWordLength;
            ScopedVclPtrInstance< VirtualDevice > pVirtDev( *this, DeviceFormat::DEFAULT,
                                                           DeviceFormat::DEFAULT );
            pVirtDev->SetOutputSizePixel( Size( nWordLength, nWaveHeight * 2 ), false );
            pVirtDev->SetLineColor( GetLineColor() );
            pVirtDev->SetBackground( Wallpaper( COL_TRANSPARENT ) );
            pVirtDev->Erase();
            pVirtDev->SetAntialiasing( AntialiasingFlags::Enable );
            pVirtDev->ImplDrawWaveLineBezier( 0, 0, nWordLength, 0, nWaveHeight, fOrientation, nLineWidth );
            BitmapEx aBitmapEx(pVirtDev->GetBitmapEx(Point(0, 0), pVirtDev->GetOutputSize()));

            // Ideally we don't need this block, but in the split rgb surface + separate alpha surface
            // with Antialiasing enabled and the svp/cairo backend we get both surfaces antialiased
            // so their combination of aliases merge to overly wash-out the color. Hack it by taking just
            // the alpha surface and use it to blend the original solid line color
            Bitmap aSolidColor(aBitmapEx.GetBitmap());
            aSolidColor.Erase(GetLineColor());
            aBitmapEx = BitmapEx(aSolidColor, aBitmapEx.GetAlpha());

            rLineCache.insert( aBitmapEx, GetLineColor(), nLineWidth, nWaveHeight, nWordLength, aWavylinebmp );
        }
        if ( aWavylinebmp.ImplGetBitmapSalBitmap() != nullptr )
        {
            Size _size( nEndX - nStartX, aWavylinebmp.GetSizePixel().Height() );
            DrawBitmapEx(Point( rStartPos.X(), rStartPos.Y() ), PixelToLogic( _size ), Point(), _size, aWavylinebmp);
        }
        return;
    }

    ImplDrawWaveLineBezier( nStartX, nStartY, nEndX, nEndY, nWaveHeight, fOrientation, nLineWidth );
}

void OutputDevice::ImplDrawWaveLineBezier(tools::Long nStartX, tools::Long nStartY, tools::Long nEndX, tools::Long nEndY, tools::Long nWaveHeight, double fOrientation, tools::Long nLineWidth)
{
    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if (!InitFont())
        return;

    const basegfx::B2DRectangle aWaveLineRectangle(nStartX, nStartY, nEndX, nEndY + nWaveHeight);
    const basegfx::B2DPolygon aWaveLinePolygon = basegfx::createWaveLinePolygon(aWaveLineRectangle);
    const basegfx::B2DHomMatrix aRotationMatrix = basegfx::utils::createRotateAroundPoint(nStartX, nStartY, basegfx::deg2rad(-fOrientation));
    const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

    mpGraphics->SetLineColor(GetLineColor());
    mpGraphics->DrawPolyLine(
            aRotationMatrix,
            aWaveLinePolygon,
            0.0,
            nLineWidth,
            nullptr, // MM01
            basegfx::B2DLineJoin::NONE,
            css::drawing::LineCap_BUTT,
            basegfx::deg2rad(15.0),
            bPixelSnapHairline,
            *this);

    if( mpAlphaVDev )
        mpAlphaVDev->ImplDrawWaveLineBezier(nStartX, nStartY, nEndX, nEndY, nWaveHeight, fOrientation, nLineWidth);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
