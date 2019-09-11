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

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

Color OutputDevice::GetReadableFontColor(const Color& rFontColor, const Color& rBgColor) const
{
    if (rBgColor.IsDark() && rFontColor.IsDark())
        return COL_WHITE;
    else if (rBgColor.IsBright() && rFontColor.IsBright())
        return COL_BLACK;
    else
        return rFontColor;
}

Color OutputDevice::GetBackgroundColor() const
{
    return GetBackground().GetColor();
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

    if( mpAlphaVDev )
        mpAlphaVDev->Erase();
}

static bool IsCachedBitmap(Wallpaper const& rWallpaper)
{
    const BitmapEx* pCached = rWallpaper.ImplGetCachedBitmap();
    return (pCached);
}

static BitmapEx GetWallpaperBitmap(Wallpaper const& rWallpaper)
{
    const BitmapEx* pCached = rWallpaper.ImplGetCachedBitmap();
    return (IsCachedBitmap(rWallpaper)) ? *pCached : rWallpaper.GetBitmap();
}

static bool UseColorBackground(Wallpaper const& rWallpaper)
{
    BitmapEx aBmpEx = GetWallpaperBitmap(rWallpaper);

    return ((aBmpEx.IsTransparent() && !rWallpaper.IsGradient()) ||
            ((rWallpaper.GetStyle() != WallpaperStyle::Tile && rWallpaper.GetStyle() != WallpaperStyle::Scale)
                || (!rWallpaper.IsGradient())));
}

static Size CalculateWallpaperSize(OutputDevice* pRenderContext, Wallpaper const& rWallpaper, long nWidth, long nHeight)
{
    if (rWallpaper.IsRect())
    {
        const tools::Rectangle aBound(pRenderContext->LogicToPixel(rWallpaper.GetRect()));
        return aBound.GetSize();
    }

    return Size(nWidth, nHeight);
}

static Point CalculateWallpaperPos(OutputDevice* pRenderContext, Wallpaper const& rWallpaper)
{
    if (rWallpaper.IsRect())
    {
        const tools::Rectangle aBound(pRenderContext->LogicToPixel(rWallpaper.GetRect()));
        return aBound.TopLeft();
    }

    return Point(0, 0);
}

static void AdjustWallpaper(OutputDevice* pRenderContext, Wallpaper const& rWallpaper, BitmapEx& rBitmap, Point& rPos, Size const& rSize)
{
    const long nBmpWidth = rBitmap.GetSizePixel().Width();
    const long nBmpHeight = rBitmap.GetSizePixel().Height();

    switch (rWallpaper.GetStyle())
    {
    case WallpaperStyle::Scale:
        if (!IsCachedBitmap(rWallpaper) || (IsCachedBitmap(rWallpaper) && rBitmap.GetSizePixel() != rSize))
        {
            if (IsCachedBitmap(rWallpaper))
                rWallpaper.ImplReleaseCachedBitmap();

            rBitmap = rWallpaper.GetBitmap();
            rBitmap.Scale( rSize );
            rBitmap = BitmapEx( rBitmap.GetBitmap().CreateDisplayBitmap(pRenderContext), rBitmap.GetMask() );
        }
        break;

    case WallpaperStyle::TopLeft:
        break;

    case WallpaperStyle::Top:
        rPos.AdjustX(( rSize.Width() - nBmpWidth ) >> 1 );
        break;

    case WallpaperStyle::TopRight:
        rPos.AdjustX( rSize.Width() - nBmpWidth);
        break;

    case WallpaperStyle::Left:
        rPos.AdjustY(( rSize.Height() - nBmpHeight ) >> 1 );
        break;

    case WallpaperStyle::Center:
        rPos.AdjustX(( rSize.Width() - nBmpWidth ) >> 1 );
        rPos.AdjustY(( rSize.Height() - nBmpHeight ) >> 1 );
        break;

    case WallpaperStyle::Right:
        rPos.AdjustX(rSize.Width() - nBmpWidth);
        rPos.AdjustY(( rSize.Height() - nBmpHeight ) >> 1 );
        break;

    case WallpaperStyle::BottomLeft:
        rPos.AdjustY( rSize.Height() - nBmpHeight );
        break;

    case WallpaperStyle::Bottom:
        rPos.AdjustX(( rSize.Width() - nBmpWidth ) >> 1 );
        rPos.AdjustY( rSize.Height() - nBmpHeight );
        break;

    case WallpaperStyle::BottomRight:
        rPos.AdjustX( rSize.Width() - nBmpWidth );
        rPos.AdjustY( rSize.Height() - nBmpHeight );
        break;

    default:
        break;
    }
}

static bool DrawWallpaperBitmap(OutputDevice* pRenderContext, Wallpaper const& rWallpaper, BitmapEx const& rBitmap,
                   Point const& rPos, Size const& rSize,
                   long nX, long nY, long nWidth, long nHeight)
{
    const long nBmpWidth = rBitmap.GetSizePixel().Width();
    const long nBmpHeight = rBitmap.GetSizePixel().Height();

    switch (rWallpaper.GetStyle())
    {
    case WallpaperStyle::Scale:
    case WallpaperStyle::TopLeft:
    case WallpaperStyle::Top:
    case WallpaperStyle::TopRight:
    case WallpaperStyle::Left:
    case WallpaperStyle::Center:
    case WallpaperStyle::Right:
    case WallpaperStyle::BottomLeft:
    case WallpaperStyle::Bottom:
    case WallpaperStyle::BottomRight:
        return false;

    default:
        {
            const long nRight = nX + nWidth - 1;
            const long nBottom = nY + nHeight - 1;
            long nFirstX;
            long nFirstY;

            if (rWallpaper.GetStyle() == WallpaperStyle::Tile)
            {
                nFirstX = rPos.X();
                nFirstY = rPos.Y();
            }
            else
            {
                nFirstX = rPos.X() + ( ( rSize.Width() - nBmpWidth ) >> 1 );
                nFirstY = rPos.Y() + ( ( rSize.Height() - nBmpHeight ) >> 1 );
            }

            const long nOffX = ( nFirstX - nX ) % nBmpWidth;
            const long nOffY = ( nFirstY - nY ) % nBmpHeight;
            long nStartX = nX + nOffX;
            long nStartY = nY + nOffY;

            if( nOffX > 0 )
                nStartX -= nBmpWidth;

            if( nOffY > 0 )
                nStartY -= nBmpHeight;

            for( long nBmpY = nStartY; nBmpY <= nBottom; nBmpY += nBmpHeight )
            {
                for( long nBmpX = nStartX; nBmpX <= nRight; nBmpX += nBmpWidth )
                {
                    pRenderContext->DrawBitmapEx( Point( nBmpX, nBmpY ), rBitmap );
                }
            }
            return true;
        }
        break;
    }
}

class MapModeStasher
{
public:
    MapModeStasher(OutputDevice* pRenderContext)
        : mpRenderContext(pRenderContext)
        , mpMetaFile(pRenderContext->GetConnectMetaFile())
        , mbIsMapModeEnabled(pRenderContext->IsMapModeEnabled())
    {
        pRenderContext->EnableMapMode(false);
        pRenderContext->Push(PushFlags::CLIPREGION);
    }

    ~MapModeStasher()
    {
        mpRenderContext->Pop();
        mpRenderContext->EnableMapMode(mbIsMapModeEnabled);
        mpRenderContext->SetConnectMetaFile(mpMetaFile);
    }

private:
    OutputDevice* mpRenderContext;
    GDIMetaFile* mpMetaFile;
    bool mbIsMapModeEnabled;
};

void OutputDevice::DrawBitmapWallpaper(long nX, long nY,
                                       long nWidth, long nHeight,
                                       const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    BitmapEx aBmpEx = GetWallpaperBitmap(rWallpaper);

    const long nBmpWidth = aBmpEx.GetSizePixel().Width();
    const long nBmpHeight = aBmpEx.GetSizePixel().Height();

    // draw background
    if (aBmpEx.IsTransparent())
    {
        if (!rWallpaper.IsGradient())
        {
            if (!IsCachedBitmap(rWallpaper) && !rWallpaper.GetColor().GetTransparency())
            {
                ScopedVclPtrInstance< VirtualDevice > aVDev(  *this  );
                aVDev->SetBackground( rWallpaper.GetColor() );
                aVDev->SetOutputSizePixel( Size( nBmpWidth, nBmpHeight ) );
                aVDev->DrawBitmapEx( Point(), aBmpEx );
                aBmpEx = aVDev->GetBitmapEx( Point(), aVDev->GetOutputSizePixel() );
            }
        }
    }

    bool bIsColorBackgroundDrawn = false;

    // background of bitmap?
    if (rWallpaper.IsGradient())
    {
        DrawGradientWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    }
    else if (UseColorBackground(rWallpaper) && aBmpEx.IsTransparent())
    {
        DrawColorWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
        bIsColorBackgroundDrawn = true;
    }

    Point aPos = CalculateWallpaperPos(this, rWallpaper);
    Size aSize = CalculateWallpaperSize(this, rWallpaper, nWidth, nHeight);

    MapModeStasher aStash(this);
    IntersectClipRegion(tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight)));
    AdjustWallpaper(this, rWallpaper, aBmpEx, aPos, aSize);

    if(!DrawWallpaperBitmap(this, rWallpaper, aBmpEx, aPos, aSize, nX, nY, nWidth, nHeight))
    {
        // optimized for non-transparent bitmaps
        if (!bIsColorBackgroundDrawn)
        {
            const Size aBmpSize( aBmpEx.GetSizePixel() );
            const tools::Rectangle aOutRect( Point(0, 0), GetOutputSizePixel() );
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
}

void OutputDevice::DrawGradientWallpaper( long nX, long nY,
                                          long nWidth, long nHeight,
                                          const Wallpaper& rWallpaper )
{
    assert(!is_double_buffered_window());

    tools::Rectangle aBound;
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    const bool bOldMap = mbMap;

    aBound = tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    mpMetaFile = nullptr;
    EnableMapMode( false );
    Push( PushFlags::CLIPREGION );
    IntersectClipRegion( tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    DrawGradient( aBound, rWallpaper.GetGradient() );

    Pop();
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
