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

#include <tools/poly.hxx>
#include <tools/helpers.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

void OutputDevice::DrawBorder(tools::Rectangle aBorderRect)
{
    sal_uInt16 nPixel = static_cast<sal_uInt16>(PixelToLogic(Size(1, 1)).Width());

    aBorderRect.AdjustLeft(nPixel);
    aBorderRect.AdjustTop(nPixel);

    SetLineColor(COL_LIGHTGRAY);
    DrawRect(aBorderRect);

    aBorderRect.AdjustLeft(-nPixel);
    aBorderRect.AdjustTop(-nPixel);
    aBorderRect.AdjustRight(-nPixel);
    aBorderRect.AdjustBottom(-nPixel);
    SetLineColor(COL_GRAY);

    DrawRect(aBorderRect);
}

void OutputDevice::DrawRect( const tools::Rectangle& rRect )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRectAction( rRect ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    tools::Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;

    aRect.Justify();

    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    if ( mbInitFillColor )
        InitFillColor();

    mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), *this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( rRect );
}

void OutputDevice::DrawRect( const tools::Rectangle& rRect,
                             sal_uLong nHorzRound, sal_uLong nVertRound )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRoundRectAction( rRect, nHorzRound, nVertRound ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    const tools::Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;

    nHorzRound = ImplLogicWidthToDevicePixel( nHorzRound );
    nVertRound = ImplLogicHeightToDevicePixel( nVertRound );

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    if ( mbInitFillColor )
        InitFillColor();

    if ( !nHorzRound && !nVertRound )
    {
        mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), *this );
    }
    else
    {
        tools::Polygon aRoundRectPoly( aRect, nHorzRound, nVertRound );

        if ( aRoundRectPoly.GetSize() >= 2 )
        {
            Point* pPtAry = aRoundRectPoly.GetPointAry();

            if ( !mbFillColor )
                mpGraphics->DrawPolyLine( aRoundRectPoly.GetSize(), pPtAry, *this );
            else
                mpGraphics->DrawPolygon( aRoundRectPoly.GetSize(), pPtAry, *this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( rRect, nHorzRound, nVertRound );
}

void OutputDevice::Invert( const tools::Rectangle& rRect, InvertFlags nFlags )
{
    assert(!is_double_buffered_window());
    if ( !IsDeviceOutputNecessary() )
        return;

    tools::Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    SalInvert nSalFlags = SalInvert::NONE;
    if ( nFlags & InvertFlags::N50 )
        nSalFlags |= SalInvert::N50;
    if ( nFlags & InvertFlags::TrackFrame )
        nSalFlags |= SalInvert::TrackFrame;
    mpGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), nSalFlags, *this );
}

void OutputDevice::Invert( const tools::Polygon& rPoly, InvertFlags nFlags )
{
    assert(!is_double_buffered_window());
    if ( !IsDeviceOutputNecessary() )
        return;

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    tools::Polygon aPoly( ImplLogicToDevicePixel( rPoly ) );

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    SalInvert nSalFlags = SalInvert::NONE;
    if ( nFlags & InvertFlags::N50 )
        nSalFlags |= SalInvert::N50;
    if ( nFlags & InvertFlags::TrackFrame )
        nSalFlags |= SalInvert::TrackFrame;
    const Point* pPtAry = aPoly.GetConstPointAry();
    mpGraphics->Invert( nPoints, pPtAry, nSalFlags, *this );
}

void OutputDevice::DrawCheckered(const Point& rPos, const Size& rSize, sal_uInt32 nLen, Color aStart, Color aEnd)
{
    assert(!is_double_buffered_window());

    const sal_uInt32 nMaxX(rPos.X() + rSize.Width());
    const sal_uInt32 nMaxY(rPos.Y() + rSize.Height());

    Push(PushFlags::LINECOLOR|PushFlags::FILLCOLOR);
    SetLineColor();

    for(sal_uInt32 x(0), nX(rPos.X()); nX < nMaxX; x++, nX += nLen)
    {
        const sal_uInt32 nRight(std::min(nMaxX, nX + nLen));

        for(sal_uInt32 y(0), nY(rPos.Y()); nY < nMaxY; y++, nY += nLen)
        {
            const sal_uInt32 nBottom(std::min(nMaxY, nY + nLen));

            SetFillColor(((x & 0x0001) ^ (y & 0x0001)) ? aStart : aEnd);
            DrawRect(tools::Rectangle(nX, nY, nRight, nBottom));
        }
    }

    Pop();
}

void OutputDevice::DrawGrid( const tools::Rectangle& rRect, const Size& rDist, DrawGridFlags nFlags )
{
    assert(!is_double_buffered_window());

    tools::Rectangle aDstRect( PixelToLogic( Point() ), GetOutputSize() );
    aDstRect.Intersection( rRect );

    if( aDstRect.IsEmpty() || ImplIsRecordLayout() )
        return;

    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    const tools::Long nDistX = std::max( rDist.Width(), tools::Long(1) );
    const tools::Long nDistY = std::max( rDist.Height(), tools::Long(1) );
    tools::Long nX = ( rRect.Left() >= aDstRect.Left() ) ? rRect.Left() : ( rRect.Left() + ( ( aDstRect.Left() - rRect.Left() ) / nDistX ) * nDistX );
    tools::Long nY = ( rRect.Top() >= aDstRect.Top() ) ? rRect.Top() : ( rRect.Top() + ( ( aDstRect.Top() - rRect.Top() ) / nDistY ) * nDistY );
    const tools::Long nRight = aDstRect.Right();
    const tools::Long nBottom = aDstRect.Bottom();
    const tools::Long nStartX = ImplLogicXToDevicePixel( nX );
    const tools::Long nEndX = ImplLogicXToDevicePixel( nRight );
    const tools::Long nStartY = ImplLogicYToDevicePixel( nY );
    const tools::Long nEndY = ImplLogicYToDevicePixel( nBottom );
    tools::Long nHorzCount = 0;
    tools::Long nVertCount = 0;

    std::vector< sal_Int32 > aVertBuf;
    std::vector< sal_Int32 > aHorzBuf;

    if( ( nFlags & DrawGridFlags::Dots ) || ( nFlags & DrawGridFlags::HorzLines ) )
    {
        aVertBuf.resize( aDstRect.GetHeight() / nDistY + 2 );
        aVertBuf[ nVertCount++ ] = nStartY;
        while( ( nY += nDistY ) <= nBottom )
        {
            aVertBuf[ nVertCount++ ] = ImplLogicYToDevicePixel( nY );
        }
    }

    if( ( nFlags & DrawGridFlags::Dots ) || ( nFlags & DrawGridFlags::VertLines ) )
    {
        aHorzBuf.resize( aDstRect.GetWidth() / nDistX + 2 );
        aHorzBuf[ nHorzCount++ ] = nStartX;
        while( ( nX += nDistX ) <= nRight )
        {
            aHorzBuf[ nHorzCount++ ] = ImplLogicXToDevicePixel( nX );
        }
    }

    if( mbInitLineColor )
        InitLineColor();

    if( mbInitFillColor )
        InitFillColor();

    const bool bOldMap = mbMap;
    EnableMapMode( false );

    if( nFlags & DrawGridFlags::Dots )
    {
        for( tools::Long i = 0; i < nVertCount; i++ )
        {
            for( tools::Long j = 0, Y = aVertBuf[ i ]; j < nHorzCount; j++ )
            {
                mpGraphics->DrawPixel( aHorzBuf[ j ], Y, *this );
            }
        }
    }
    else
    {
        if( nFlags & DrawGridFlags::HorzLines )
        {
            for( tools::Long i = 0; i < nVertCount; i++ )
            {
                nY = aVertBuf[ i ];
                mpGraphics->DrawLine( nStartX, nY, nEndX, nY, *this );
            }
        }

        if( nFlags & DrawGridFlags::VertLines )
        {
            for( tools::Long i = 0; i < nHorzCount; i++ )
            {
                nX = aHorzBuf[ i ];
                mpGraphics->DrawLine( nX, nStartY, nX, nEndY, *this );
            }
        }
    }

    EnableMapMode( bOldMap );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawGrid( rRect, rDist, nFlags );
}

BmpMirrorFlags AdjustTwoRect( SalTwoRect& rTwoRect, const Size& rSizePix )
{
    BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

    if ( rTwoRect.mnDestWidth < 0 )
    {
        rTwoRect.mnSrcX = rSizePix.Width() - rTwoRect.mnSrcX - rTwoRect.mnSrcWidth;
        rTwoRect.mnDestWidth = -rTwoRect.mnDestWidth;
        rTwoRect.mnDestX -= rTwoRect.mnDestWidth-1;
        nMirrFlags |= BmpMirrorFlags::Horizontal;
    }

    if ( rTwoRect.mnDestHeight < 0 )
    {
        rTwoRect.mnSrcY = rSizePix.Height() - rTwoRect.mnSrcY - rTwoRect.mnSrcHeight;
        rTwoRect.mnDestHeight = -rTwoRect.mnDestHeight;
        rTwoRect.mnDestY -= rTwoRect.mnDestHeight-1;
        nMirrFlags |= BmpMirrorFlags::Vertical;
    }

    if( ( rTwoRect.mnSrcX < 0 ) || ( rTwoRect.mnSrcX >= rSizePix.Width() ) ||
        ( rTwoRect.mnSrcY < 0 ) || ( rTwoRect.mnSrcY >= rSizePix.Height() ) ||
        ( ( rTwoRect.mnSrcX + rTwoRect.mnSrcWidth ) > rSizePix.Width() ) ||
        ( ( rTwoRect.mnSrcY + rTwoRect.mnSrcHeight ) > rSizePix.Height() ) )
    {
        const tools::Rectangle aSourceRect( Point( rTwoRect.mnSrcX, rTwoRect.mnSrcY ),
                                     Size( rTwoRect.mnSrcWidth, rTwoRect.mnSrcHeight ) );
        tools::Rectangle aCropRect( aSourceRect );

        aCropRect.Intersection( tools::Rectangle( Point(), rSizePix ) );

        if( aCropRect.IsEmpty() )
        {
            rTwoRect.mnSrcWidth = rTwoRect.mnSrcHeight = rTwoRect.mnDestWidth = rTwoRect.mnDestHeight = 0;
        }
        else
        {
            const double fFactorX = ( rTwoRect.mnSrcWidth > 1 ) ? static_cast<double>( rTwoRect.mnDestWidth - 1 ) / ( rTwoRect.mnSrcWidth - 1 ) : 0.0;
            const double fFactorY = ( rTwoRect.mnSrcHeight > 1 ) ? static_cast<double>( rTwoRect.mnDestHeight - 1 ) / ( rTwoRect.mnSrcHeight - 1 ) : 0.0;

            const tools::Long nDstX1 = rTwoRect.mnDestX + FRound( fFactorX * ( aCropRect.Left() - rTwoRect.mnSrcX ) );
            const tools::Long nDstY1 = rTwoRect.mnDestY + FRound( fFactorY * ( aCropRect.Top() - rTwoRect.mnSrcY ) );
            const tools::Long nDstX2 = rTwoRect.mnDestX + FRound( fFactorX * ( aCropRect.Right() - rTwoRect.mnSrcX ) );
            const tools::Long nDstY2 = rTwoRect.mnDestY + FRound( fFactorY * ( aCropRect.Bottom() - rTwoRect.mnSrcY ) );

            rTwoRect.mnSrcX = aCropRect.Left();
            rTwoRect.mnSrcY = aCropRect.Top();
            rTwoRect.mnSrcWidth = aCropRect.GetWidth();
            rTwoRect.mnSrcHeight = aCropRect.GetHeight();
            rTwoRect.mnDestX = nDstX1;
            rTwoRect.mnDestY = nDstY1;
            rTwoRect.mnDestWidth = nDstX2 - nDstX1 + 1;
            rTwoRect.mnDestHeight = nDstY2 - nDstY1 + 1;
        }
    }

    return nMirrFlags;
}

void AdjustTwoRect( SalTwoRect& rTwoRect, const tools::Rectangle& rValidSrcRect )
{
    if( !(( rTwoRect.mnSrcX < rValidSrcRect.Left() ) || ( rTwoRect.mnSrcX >= rValidSrcRect.Right() ) ||
        ( rTwoRect.mnSrcY < rValidSrcRect.Top() ) || ( rTwoRect.mnSrcY >= rValidSrcRect.Bottom() ) ||
        ( ( rTwoRect.mnSrcX + rTwoRect.mnSrcWidth ) > rValidSrcRect.Right() ) ||
        ( ( rTwoRect.mnSrcY + rTwoRect.mnSrcHeight ) > rValidSrcRect.Bottom() )) )
        return;

    const tools::Rectangle aSourceRect( Point( rTwoRect.mnSrcX, rTwoRect.mnSrcY ),
                                 Size( rTwoRect.mnSrcWidth, rTwoRect.mnSrcHeight ) );
    tools::Rectangle aCropRect( aSourceRect );

    aCropRect.Intersection( rValidSrcRect );

    if( aCropRect.IsEmpty() )
    {
        rTwoRect.mnSrcWidth = rTwoRect.mnSrcHeight = rTwoRect.mnDestWidth = rTwoRect.mnDestHeight = 0;
    }
    else
    {
        const double fFactorX = ( rTwoRect.mnSrcWidth > 1 ) ? static_cast<double>( rTwoRect.mnDestWidth - 1 ) / ( rTwoRect.mnSrcWidth - 1 ) : 0.0;
        const double fFactorY = ( rTwoRect.mnSrcHeight > 1 ) ? static_cast<double>( rTwoRect.mnDestHeight - 1 ) / ( rTwoRect.mnSrcHeight - 1 ) : 0.0;

        const tools::Long nDstX1 = rTwoRect.mnDestX + FRound( fFactorX * ( aCropRect.Left() - rTwoRect.mnSrcX ) );
        const tools::Long nDstY1 = rTwoRect.mnDestY + FRound( fFactorY * ( aCropRect.Top() - rTwoRect.mnSrcY ) );
        const tools::Long nDstX2 = rTwoRect.mnDestX + FRound( fFactorX * ( aCropRect.Right() - rTwoRect.mnSrcX ) );
        const tools::Long nDstY2 = rTwoRect.mnDestY + FRound( fFactorY * ( aCropRect.Bottom() - rTwoRect.mnSrcY ) );

        rTwoRect.mnSrcX = aCropRect.Left();
        rTwoRect.mnSrcY = aCropRect.Top();
        rTwoRect.mnSrcWidth = aCropRect.GetWidth();
        rTwoRect.mnSrcHeight = aCropRect.GetHeight();
        rTwoRect.mnDestX = nDstX1;
        rTwoRect.mnDestY = nDstY1;
        rTwoRect.mnDestWidth = nDstX2 - nDstX1 + 1;
        rTwoRect.mnDestHeight = nDstY2 - nDstY1 + 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
