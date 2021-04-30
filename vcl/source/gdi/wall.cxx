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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/gradient.hxx>
#include <vcl/wall.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/settings.hxx>
#include <vcl/TypeSerializer.hxx>

SvStream& ReadWallpaper( SvStream& rIStm, Wallpaper& rImplWallpaper )
{
    VersionCompatRead  aCompat(rIStm);

    rImplWallpaper.maRect.SetEmpty();
    rImplWallpaper.mpGradient.reset();
    rImplWallpaper.maBitmap.SetEmpty();

    // version 1
    TypeSerializer aSerializer(rIStm);
    aSerializer.readColor(rImplWallpaper.maColor);
    sal_uInt16 nTmp16(0);
    rIStm.ReadUInt16(nTmp16);
    rImplWallpaper.meStyle = static_cast<WallpaperStyle>(nTmp16);

    // version 2
    if( aCompat.GetVersion() >= 2 )
    {
        bool bRect(false), bGrad(false), bBmp(false), bDummy;

        rIStm.ReadCharAsBool( bRect ).ReadCharAsBool( bGrad ).ReadCharAsBool( bBmp ).ReadCharAsBool( bDummy ).ReadCharAsBool( bDummy ).ReadCharAsBool( bDummy );

        if( bRect )
        {
            rImplWallpaper.maRect = tools::Rectangle();
            aSerializer.readRectangle(rImplWallpaper.maRect);
        }

        if( bGrad )
        {
            rImplWallpaper.mpGradient.emplace();
            aSerializer.readGradient(*rImplWallpaper.mpGradient);
        }

        if( bBmp )
        {
            rImplWallpaper.maBitmap.SetEmpty();
            ReadDIBBitmapEx(rImplWallpaper.maBitmap, rIStm);
        }

        // version 3 (new color format)
        if( aCompat.GetVersion() >= 3 )
        {
            sal_uInt32 nTmp;
            rIStm.ReadUInt32(nTmp);
            rImplWallpaper.maColor = ::Color(ColorTransparency, nTmp);
        }
    }

    return rIStm;
}

SvStream& WriteWallpaper( SvStream& rOStm, const Wallpaper& rImplWallpaper )
{
    VersionCompatWrite aCompat(rOStm, 3);
    bool            bRect = !rImplWallpaper.maRect.IsEmpty();
    bool            bGrad = bool(rImplWallpaper.mpGradient);
    bool            bBmp = !rImplWallpaper.maBitmap.IsEmpty();
    bool            bDummy = false;

    // version 1
    TypeSerializer aSerializer(rOStm);
    aSerializer.writeColor(rImplWallpaper.maColor);

    rOStm.WriteUInt16( static_cast<sal_uInt16>(rImplWallpaper.meStyle) );

    // version 2
    rOStm.WriteBool( bRect ).WriteBool( bGrad ).WriteBool( bBmp ).WriteBool( bDummy ).WriteBool( bDummy ).WriteBool( bDummy );

    if( bRect )
    {
        aSerializer.writeRectangle(rImplWallpaper.maRect);
    }

    if (bGrad)
    {
        aSerializer.writeGradient(*rImplWallpaper.mpGradient);
    }

    if( bBmp )
        WriteDIBBitmapEx(rImplWallpaper.maBitmap, rOStm);

    // version 3 (new color format)
    rOStm.WriteUInt32(static_cast<sal_uInt32>(rImplWallpaper.maColor));

    return rOStm;
}

Wallpaper::Wallpaper() :
    maColor( COL_TRANSPARENT ), meStyle( WallpaperStyle::NONE )
{
}

Wallpaper::Wallpaper( const Wallpaper& ) = default;

Wallpaper::Wallpaper( Wallpaper&& ) = default;

Wallpaper::Wallpaper( const Color& rColor )
{
    maColor    = rColor;
    meStyle    = WallpaperStyle::Tile;
}

Wallpaper::Wallpaper( const BitmapEx& rBmpEx )
{
    maBitmap   = rBmpEx;
    meStyle    = WallpaperStyle::Tile;
}

Wallpaper::~Wallpaper() = default;

void Wallpaper::ImplSetCachedBitmap( BitmapEx& rBmp ) const
{
    maCache = rBmp;
}

const BitmapEx* Wallpaper::ImplGetCachedBitmap() const
{
    return maCache.IsEmpty() ? nullptr : &maCache;
}

void Wallpaper::ImplReleaseCachedBitmap() const
{
    maCache.SetEmpty();
}

void Wallpaper::SetColor( const Color& rColor )
{
    maCache.SetEmpty();
    maColor = rColor;

    if( WallpaperStyle::NONE == meStyle || WallpaperStyle::ApplicationGradient == meStyle )
        meStyle = WallpaperStyle::Tile;
}

void Wallpaper::SetStyle( WallpaperStyle eStyle )
{
    if( eStyle == WallpaperStyle::ApplicationGradient )
        // set a dummy gradient, the correct gradient
        // will be created dynamically in GetGradient()
        SetGradient( ImplGetApplicationGradient() );

    meStyle = eStyle;
}

void Wallpaper::SetBitmap( const BitmapEx& rBitmap )
{
    maCache.SetEmpty();
    maBitmap = rBitmap;

    if( WallpaperStyle::NONE == meStyle || WallpaperStyle::ApplicationGradient == meStyle)
        meStyle = WallpaperStyle::Tile;
}

const BitmapEx & Wallpaper::GetBitmap() const
{
    return maBitmap;
}

bool Wallpaper::IsBitmap() const
{
    return !maBitmap.IsEmpty();
}

void Wallpaper::SetGradient( const Gradient& rGradient )
{
    maCache.SetEmpty();
    mpGradient = rGradient;

    if( WallpaperStyle::NONE == meStyle || WallpaperStyle::ApplicationGradient == meStyle )
        meStyle = WallpaperStyle::Tile;
}

Gradient Wallpaper::GetGradient() const
{
    if( WallpaperStyle::ApplicationGradient == meStyle )
        return ImplGetApplicationGradient();
    else if ( mpGradient )
        return *mpGradient;
    else
        return Gradient();
}

bool Wallpaper::IsGradient() const
{
    return bool(mpGradient);
}

Gradient Wallpaper::ImplGetApplicationGradient()
{
    Gradient g;
    g.SetAngle( 900_deg10 );
    g.SetStyle( GradientStyle::Linear );
    g.SetStartColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    // no 'extreme' gradient when high contrast
    if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    else
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceGradientColor() );
    return g;
}

bool Wallpaper::IsRect() const
{
    return !maRect.IsEmpty();
}

bool Wallpaper::IsFixed() const
{
    if ( meStyle == WallpaperStyle::NONE )
        return false;
    else
        return (maBitmap.IsEmpty() && !mpGradient);
}

bool Wallpaper::IsScrollable() const
{
    if ( meStyle == WallpaperStyle::NONE )
        return false;
    else if ( maBitmap.IsEmpty() && !mpGradient )
        return true;
    else if ( !maBitmap.IsEmpty() )
        return (meStyle == WallpaperStyle::Tile);
    else
        return false;
}

Wallpaper& Wallpaper::operator=( const Wallpaper& ) = default;

Wallpaper& Wallpaper::operator=( Wallpaper&& ) = default;

bool Wallpaper::operator==( const Wallpaper& rOther ) const
{
    return meStyle == rOther.meStyle &&
        maColor == rOther.maColor &&
        maRect == rOther.maRect &&
        maBitmap == rOther.maBitmap &&
        mpGradient == rOther.mpGradient;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
