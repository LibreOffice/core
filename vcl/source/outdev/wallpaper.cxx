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

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <wall2.hxx>

void OutputDevice::DrawWallpaper( const Rectangle& rRect,
                                  const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaWallpaperAction( rRect, rWallpaper ) );

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if ( rWallpaper.GetStyle() != WallpaperStyle::NONE )
    {
        Rectangle aRect = LogicToPixel( rRect );
        aRect.Justify();

        if ( !aRect.IsEmpty() )
        {
            DrawWallpaper( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                               rWallpaper );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawWallpaper( rRect, rWallpaper );
}

void OutputDevice::DrawWallpaper( long nX, long nY,
                                  long nWidth, long nHeight,
                                  const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    if( rWallpaper.IsBitmap() )
        DrawBitmapWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    else if( rWallpaper.IsGradient() )
        DrawGradientWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    else
        DrawColorWallpaper(  nX, nY, nWidth, nHeight, rWallpaper );
}

void OutputDevice::DrawColorWallpaper( long nX, long nY,
                                       long nWidth, long nHeight,
                                       const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    // draw wallpaper without border
    Color aOldLineColor = GetLineColor();
    Color aOldFillColor = GetFillColor();
    SetLineColor();
    SetFillColor( rWallpaper.GetColor() );

    bool bMap = mbMap;
    EnableMapMode( false );
    DrawRect( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );
    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
    EnableMapMode( bMap );
}

void OutputDevice::Erase()
{
    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if ( mbBackground )
    {
        RasterOp eRasterOp = GetRasterOp();
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( ROP_OVERPAINT );
        DrawWallpaper( 0, 0, mnOutWidth, mnOutHeight, maBackground );
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( eRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->Erase();
}

void OutputDevice::DrawBitmapWallpaper( long nX, long nY,
                                            long nWidth, long nHeight,
                                            const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    BitmapEx aBmpEx;
    const BitmapEx* pCached = rWallpaper.ImplGetCachedBitmap();
    Point aPos;
    Size aSize;
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    const WallpaperStyle eStyle = rWallpaper.GetStyle();
    const bool bOldMap = mbMap;
    bool bDrawn = false;
    bool bDrawGradientBackground = false;
    bool bDrawColorBackground = false;

    if( pCached )
        aBmpEx = *pCached;
    else
        aBmpEx = rWallpaper.GetBitmap();

    const long nBmpWidth = aBmpEx.GetSizePixel().Width();
    const long nBmpHeight = aBmpEx.GetSizePixel().Height();
    const bool bTransparent = aBmpEx.IsTransparent();

    // draw background
    if( bTransparent )
    {
        if( rWallpaper.IsGradient() )
            bDrawGradientBackground = true;
        else
        {
            if( !pCached && !rWallpaper.GetColor().GetTransparency() )
            {
                ScopedVclPtrInstance< VirtualDevice > aVDev(  *this  );
                aVDev->SetBackground( rWallpaper.GetColor() );
                aVDev->SetOutputSizePixel( Size( nBmpWidth, nBmpHeight ) );
                aVDev->DrawBitmapEx( Point(), aBmpEx );
                aBmpEx = aVDev->GetBitmap( Point(), aVDev->GetOutputSizePixel() );
            }

            bDrawColorBackground = true;
        }
    }
    else if( eStyle != WallpaperStyle::Tile && eStyle != WallpaperStyle::Scale )
    {
        if( rWallpaper.IsGradient() )
            bDrawGradientBackground = true;
        else
            bDrawColorBackground = true;
    }

    // background of bitmap?
    if( bDrawGradientBackground )
        DrawGradientWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    else if( bDrawColorBackground && bTransparent )
    {
        DrawColorWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
        bDrawColorBackground = false;
    }

    // calc pos and size
    if( rWallpaper.IsRect() )
    {
        const Rectangle aBound( LogicToPixel( rWallpaper.GetRect() ) );
        aPos = aBound.TopLeft();
        aSize = aBound.GetSize();
    }
    else
    {
        aPos = Point( 0, 0 );
        aSize = Size( nWidth, nHeight );
    }

    mpMetaFile = nullptr;
    EnableMapMode( false );
    Push( PushFlags::CLIPREGION );
    IntersectClipRegion( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    switch( eStyle )
    {
    case( WallpaperStyle::Scale ):
        if( !pCached || ( pCached->GetSizePixel() != aSize ) )
        {
            if( pCached )
                rWallpaper.ImplReleaseCachedBitmap();

            aBmpEx = rWallpaper.GetBitmap();
            aBmpEx.Scale( aSize );
            aBmpEx = BitmapEx( aBmpEx.GetBitmap().CreateDisplayBitmap( this ), aBmpEx.GetMask() );
        }
        break;

    case( WallpaperStyle::TopLeft ):
        break;

    case( WallpaperStyle::Top ):
        aPos.X() += ( aSize.Width() - nBmpWidth ) >> 1;
        break;

    case( WallpaperStyle::TopRight ):
        aPos.X() += ( aSize.Width() - nBmpWidth );
        break;

    case( WallpaperStyle::Left ):
        aPos.Y() += ( aSize.Height() - nBmpHeight ) >> 1;
        break;

    case( WallpaperStyle::Center ):
        aPos.X() += ( aSize.Width() - nBmpWidth ) >> 1;
        aPos.Y() += ( aSize.Height() - nBmpHeight ) >> 1;
        break;

    case( WallpaperStyle::Right ):
        aPos.X() += ( aSize.Width() - nBmpWidth );
        aPos.Y() += ( aSize.Height() - nBmpHeight ) >> 1;
        break;

    case( WallpaperStyle::BottomLeft ):
        aPos.Y() += ( aSize.Height() - nBmpHeight );
        break;

    case( WallpaperStyle::Bottom ):
        aPos.X() += ( aSize.Width() - nBmpWidth ) >> 1;
        aPos.Y() += ( aSize.Height() - nBmpHeight );
        break;

    case( WallpaperStyle::BottomRight ):
        aPos.X() += ( aSize.Width() - nBmpWidth );
        aPos.Y() += ( aSize.Height() - nBmpHeight );
        break;

    default:
        {
            const long nRight = nX + nWidth - 1L;
            const long nBottom = nY + nHeight - 1L;
            long nFirstX;
            long nFirstY;

            if( eStyle == WallpaperStyle::Tile )
            {
                nFirstX = aPos.X();
                nFirstY = aPos.Y();
            }
            else
            {
                nFirstX = aPos.X() + ( ( aSize.Width() - nBmpWidth ) >> 1 );
                nFirstY = aPos.Y() + ( ( aSize.Height() - nBmpHeight ) >> 1 );
            }

            const long nOffX = ( nFirstX - nX ) % nBmpWidth;
            const long nOffY = ( nFirstY - nY ) % nBmpHeight;
            long nStartX = nX + nOffX;
            long nStartY = nY + nOffY;

            if( nOffX > 0L )
                nStartX -= nBmpWidth;

            if( nOffY > 0L )
                nStartY -= nBmpHeight;

            for( long nBmpY = nStartY; nBmpY <= nBottom; nBmpY += nBmpHeight )
            {
                for( long nBmpX = nStartX; nBmpX <= nRight; nBmpX += nBmpWidth )
                {
                    DrawBitmapEx( Point( nBmpX, nBmpY ), aBmpEx );
                }
            }
            bDrawn = true;
        }
        break;
    }

    if( !bDrawn )
    {
        // optimized for non-transparent bitmaps
        if( bDrawColorBackground )
        {
            const Size aBmpSize( aBmpEx.GetSizePixel() );
            const Point aTmpPoint;
            const Rectangle aOutRect( aTmpPoint, GetOutputSizePixel() );
            const Rectangle aColRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            Rectangle aWorkRect;

            aWorkRect = Rectangle( 0, 0, aOutRect.Right(), aPos.Y() - 1L );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }

            aWorkRect = Rectangle( 0, aPos.Y(), aPos.X() - 1L, aPos.Y() + aBmpSize.Height() - 1L );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }

            aWorkRect = Rectangle( aPos.X() + aBmpSize.Width(), aPos.Y(),
                                   aOutRect.Right(), aPos.Y() + aBmpSize.Height() - 1L );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }

            aWorkRect = Rectangle( 0, aPos.Y() + aBmpSize.Height(),
                                   aOutRect.Right(), aOutRect.Bottom() );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }
        }

        DrawBitmapEx( aPos, aBmpEx );
    }

    rWallpaper.ImplSetCachedBitmap( aBmpEx );

    Pop();
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

void OutputDevice::DrawGradientWallpaper( long nX, long nY,
                                          long nWidth, long nHeight,
                                          const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    Rectangle aBound;
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    const bool bOldMap = mbMap;

    aBound = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    mpMetaFile = nullptr;
    EnableMapMode( false );
    Push( PushFlags::CLIPREGION );
    IntersectClipRegion( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    DrawGradient( aBound, rWallpaper.GetGradient() );

    Pop();
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
