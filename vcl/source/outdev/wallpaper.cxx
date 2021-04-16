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

#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>

#include <cassert>

Color OutputDevice::GetReadableFontColor(const Color& rFontColor, const Color& rBgColor) const
{
    if (rBgColor.IsDark() && rFontColor.IsDark())
        return COL_WHITE;
    else if (rBgColor.IsBright() && rFontColor.IsBright())
        return COL_BLACK;
    else
        return rFontColor;
}

void OutputDevice::DrawWallpaper( const tools::Rectangle& rRect,
                                  const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaWallpaperAction( rRect, rWallpaper ) );

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if ( rWallpaper.GetStyle() != WallpaperStyle::NONE )
    {
        tools::Rectangle aRect = LogicToPixel( rRect );
        aRect.Justify();

        if ( !aRect.IsEmpty() )
        {
            DrawWallpaper( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                               rWallpaper );
        }
    }

    if( mpAlphaVDev )
    {
        // Also see logic in OutputDevice::SetBackground
        // Some of these are probably wrong (e.g. if the gradient has transparency),
        // but hopefully nobody uses that. If you do, feel free to implement it properly.
        Wallpaper aAlphaWallpaper;
        if( rWallpaper.GetStyle() == WallpaperStyle::NONE )
            aAlphaWallpaper = Wallpaper( Color( 0xff, 0xff, 0xff )); // fully opaque
        else if( rWallpaper.IsBitmap())
        {
            BitmapEx bitmap = rWallpaper.GetBitmap();
            if( bitmap.IsAlpha())
                aAlphaWallpaper = Wallpaper( BitmapEx( Bitmap( bitmap.GetAlpha())));
            else
                aAlphaWallpaper = Wallpaper( COL_ALPHA_OPAQUE );
        }
        else if( rWallpaper.IsGradient())
            aAlphaWallpaper = Wallpaper( COL_ALPHA_OPAQUE );
        else
        {
            // Color background.
            int alpha = rWallpaper.GetColor().GetAlpha();
            aAlphaWallpaper = Color( alpha, alpha, alpha );
        }
        mpAlphaVDev->DrawWallpaper( rRect, aAlphaWallpaper );
    }
}

void OutputDevice::DrawWallpaper( tools::Long nX, tools::Long nY,
                                  tools::Long nWidth, tools::Long nHeight,
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

void OutputDevice::DrawColorWallpaper( tools::Long nX, tools::Long nY,
                                       tools::Long nWidth, tools::Long nHeight,
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
    DrawRect( tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );
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
        if ( eRasterOp != RasterOp::OverPaint )
            SetRasterOp( RasterOp::OverPaint );
        DrawWallpaper( 0, 0, mnOutWidth, mnOutHeight, maBackground );
        if ( eRasterOp != RasterOp::OverPaint )
            SetRasterOp( eRasterOp );
    }

    if (mpAlphaVDev)
        mpAlphaVDev->Erase();
}

void OutputDevice::Erase(const tools::Rectangle& rRect)
{
    const RasterOp eRasterOp = GetRasterOp();
    if ( eRasterOp != RasterOp::OverPaint )
        SetRasterOp( RasterOp::OverPaint );
    DrawWallpaper(rRect, GetBackground());
    if ( eRasterOp != RasterOp::OverPaint )
        SetRasterOp( eRasterOp );

    if (mpAlphaVDev)
        mpAlphaVDev->Erase(rRect);
}

void OutputDevice::DrawBitmapWallpaper( tools::Long nX, tools::Long nY,
                                            tools::Long nWidth, tools::Long nHeight,
                                            const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    const BitmapEx* pCached = rWallpaper.ImplGetCachedBitmap();

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    const bool bOldMap = mbMap;

    BitmapEx aBmpEx;
    if( pCached )
        aBmpEx = *pCached;
    else
        aBmpEx = rWallpaper.GetBitmap();

    const tools::Long nBmpWidth = aBmpEx.GetSizePixel().Width();
    const tools::Long nBmpHeight = aBmpEx.GetSizePixel().Height();
    const bool bTransparent = aBmpEx.IsAlpha();

    const WallpaperStyle eStyle = rWallpaper.GetStyle();

    bool bDrawGradientBackground = false;
    bool bDrawColorBackground = false;

    // draw background
    if( bTransparent )
    {
        if( rWallpaper.IsGradient() )
            bDrawGradientBackground = true;
        else
        {
            if( !pCached && !rWallpaper.GetColor().IsTransparent() )
            {
                ScopedVclPtrInstance< VirtualDevice > aVDev(  *this  );
                aVDev->SetBackground( rWallpaper.GetColor() );
                aVDev->SetOutputSizePixel( Size( nBmpWidth, nBmpHeight ) );
                aVDev->DrawBitmapEx( Point(), aBmpEx );
                aBmpEx = aVDev->GetBitmapEx( Point(), aVDev->GetOutputSizePixel() );
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
    {
        DrawGradientWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    }
    else if( bDrawColorBackground && bTransparent )
    {
        DrawColorWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
        bDrawColorBackground = false;
    }

    Point aPos;
    Size aSize;

    // calc pos and size
    if( rWallpaper.IsRect() )
    {
        const tools::Rectangle aBound( LogicToPixel( rWallpaper.GetRect() ) );
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
    Push( vcl::PushFlags::CLIPREGION );
    IntersectClipRegion( tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    bool bDrawn = false;

    switch( eStyle )
    {
    case WallpaperStyle::Scale:
        if( !pCached || ( pCached->GetSizePixel() != aSize ) )
        {
            if( pCached )
                rWallpaper.ImplReleaseCachedBitmap();

            aBmpEx = rWallpaper.GetBitmap();
            aBmpEx.Scale( aSize );
            aBmpEx = BitmapEx( aBmpEx.GetBitmap().CreateDisplayBitmap( this ), aBmpEx.GetAlpha() );
        }
        break;

    case WallpaperStyle::TopLeft:
        break;

    case WallpaperStyle::Top:
        aPos.AdjustX(( aSize.Width() - nBmpWidth ) >> 1 );
        break;

    case WallpaperStyle::TopRight:
        aPos.AdjustX( aSize.Width() - nBmpWidth);
        break;

    case WallpaperStyle::Left:
        aPos.AdjustY(( aSize.Height() - nBmpHeight ) >> 1 );
        break;

    case WallpaperStyle::Center:
        aPos.AdjustX(( aSize.Width() - nBmpWidth ) >> 1 );
        aPos.AdjustY(( aSize.Height() - nBmpHeight ) >> 1 );
        break;

    case WallpaperStyle::Right:
        aPos.AdjustX(aSize.Width() - nBmpWidth);
        aPos.AdjustY(( aSize.Height() - nBmpHeight ) >> 1 );
        break;

    case WallpaperStyle::BottomLeft:
        aPos.AdjustY( aSize.Height() - nBmpHeight );
        break;

    case WallpaperStyle::Bottom:
        aPos.AdjustX(( aSize.Width() - nBmpWidth ) >> 1 );
        aPos.AdjustY( aSize.Height() - nBmpHeight );
        break;

    case WallpaperStyle::BottomRight:
        aPos.AdjustX( aSize.Width() - nBmpWidth );
        aPos.AdjustY( aSize.Height() - nBmpHeight );
        break;

    default:
        {
            const tools::Long nRight = nX + nWidth - 1;
            const tools::Long nBottom = nY + nHeight - 1;
            tools::Long nFirstX;
            tools::Long nFirstY;

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

            const tools::Long nOffX = ( nFirstX - nX ) % nBmpWidth;
            const tools::Long nOffY = ( nFirstY - nY ) % nBmpHeight;
            tools::Long nStartX = nX + nOffX;
            tools::Long nStartY = nY + nOffY;

            if( nOffX > 0 )
                nStartX -= nBmpWidth;

            if( nOffY > 0 )
                nStartY -= nBmpHeight;

            for( tools::Long nBmpY = nStartY; nBmpY <= nBottom; nBmpY += nBmpHeight )
            {
                for( tools::Long nBmpX = nStartX; nBmpX <= nRight; nBmpX += nBmpWidth )
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
            const tools::Rectangle aOutRect( aTmpPoint, GetOutputSizePixel() );
            const tools::Rectangle aColRect( Point( nX, nY ), Size( nWidth, nHeight ) );

            tools::Rectangle aWorkRect( 0, 0, aOutRect.Right(), aPos.Y() - 1 );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }

            aWorkRect = tools::Rectangle( 0, aPos.Y(), aPos.X() - 1, aPos.Y() + aBmpSize.Height() - 1 );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }

            aWorkRect = tools::Rectangle( aPos.X() + aBmpSize.Width(), aPos.Y(),
                                   aOutRect.Right(), aPos.Y() + aBmpSize.Height() - 1 );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                DrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                    aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                    rWallpaper );
            }

            aWorkRect = tools::Rectangle( 0, aPos.Y() + aBmpSize.Height(),
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

void OutputDevice::DrawGradientWallpaper( tools::Long nX, tools::Long nY,
                                          tools::Long nWidth, tools::Long nHeight,
                                          const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    tools::Rectangle aBound;
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    const bool bOldMap = mbMap;

    aBound = tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    mpMetaFile = nullptr;
    EnableMapMode( false );
    Push( vcl::PushFlags::CLIPREGION );
    IntersectClipRegion( tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    DrawGradient( aBound, rWallpaper.GetGradient() );

    Pop();
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
