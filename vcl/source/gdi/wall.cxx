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
#include <wall2.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/settings.hxx>

ImplWallpaper::ImplWallpaper() :
    maColor( COL_TRANSPARENT ), meStyle( WallpaperStyle::NONE )
{
}

ImplWallpaper::ImplWallpaper( const ImplWallpaper& rImplWallpaper ) :
    maColor( rImplWallpaper.maColor ), meStyle(rImplWallpaper.meStyle)
{
    if ( rImplWallpaper.mpBitmap )
        mpBitmap = std::make_unique<BitmapEx>( *rImplWallpaper.mpBitmap );

    if( rImplWallpaper.mpCache )
        mpCache = std::make_unique<BitmapEx>( *rImplWallpaper.mpCache );

    if ( rImplWallpaper.mpGradient )
        mpGradient = std::make_unique<Gradient>( *rImplWallpaper.mpGradient );

    if ( rImplWallpaper.mpRect )
        mpRect = *rImplWallpaper.mpRect;
}

ImplWallpaper::~ImplWallpaper()
{
}

SvStream& ReadImplWallpaper( SvStream& rIStm, ImplWallpaper& rImplWallpaper )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );

    rImplWallpaper.mpRect.reset();
    rImplWallpaper.mpGradient.reset();
    rImplWallpaper.mpBitmap.reset();

    // version 1
    ReadColor( rIStm, rImplWallpaper.maColor );
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
            rImplWallpaper.mpRect = tools::Rectangle();
            ReadRectangle( rIStm, *rImplWallpaper.mpRect );
        }

        if( bGrad )
        {
            rImplWallpaper.mpGradient = std::make_unique<Gradient>();
            ReadGradient( rIStm, *rImplWallpaper.mpGradient );
        }

        if( bBmp )
        {
            rImplWallpaper.mpBitmap = std::make_unique<BitmapEx>();
            ReadDIBBitmapEx(*rImplWallpaper.mpBitmap, rIStm);
        }

        // version 3 (new color format)
        if( aCompat.GetVersion() >= 3 )
        {
            rImplWallpaper.maColor.Read( rIStm );
        }
    }

    return rIStm;
}

SvStream& WriteImplWallpaper( SvStream& rOStm, const ImplWallpaper& rImplWallpaper )
{
    VersionCompat   aCompat( rOStm, StreamMode::WRITE, 3 );
    bool            bRect = bool(rImplWallpaper.mpRect);
    bool            bGrad = bool(rImplWallpaper.mpGradient);
    bool            bBmp = bool(rImplWallpaper.mpBitmap);
    bool            bDummy = false;

    // version 1
    WriteColor( rOStm, rImplWallpaper.maColor );
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rImplWallpaper.meStyle) );

    // version 2
    rOStm.WriteBool( bRect ).WriteBool( bGrad ).WriteBool( bBmp ).WriteBool( bDummy ).WriteBool( bDummy ).WriteBool( bDummy );

    if( bRect )
        WriteRectangle( rOStm, *rImplWallpaper.mpRect );

    if( bGrad )
        WriteGradient( rOStm, *rImplWallpaper.mpGradient );

    if( bBmp )
        WriteDIBBitmapEx(*rImplWallpaper.mpBitmap, rOStm);

    // version 3 (new color format)
    rImplWallpaper.maColor.Write( rOStm );

    return rOStm;
}

namespace
{
    struct theGlobalDefault :
        public rtl::Static< Wallpaper::ImplType, theGlobalDefault > {};
}

Wallpaper::Wallpaper() : mpImplWallpaper(theGlobalDefault::get())
{
}

Wallpaper::Wallpaper( const Wallpaper& ) = default;

Wallpaper::Wallpaper( Wallpaper&& ) = default;

Wallpaper::Wallpaper( const Color& rColor ) : mpImplWallpaper()
{
    mpImplWallpaper->maColor    = rColor;
    mpImplWallpaper->meStyle    = WallpaperStyle::Tile;
}

Wallpaper::Wallpaper( const BitmapEx& rBmpEx ) : mpImplWallpaper()
{
    mpImplWallpaper->mpBitmap   = std::make_unique<BitmapEx>( rBmpEx );
    mpImplWallpaper->meStyle    = WallpaperStyle::Tile;
}

Wallpaper::Wallpaper( const Gradient& rGradient ) : mpImplWallpaper()
{
    mpImplWallpaper->mpGradient = std::make_unique<Gradient>( rGradient );
    mpImplWallpaper->meStyle    = WallpaperStyle::Tile;
}

Wallpaper::~Wallpaper() = default;

void Wallpaper::ImplSetCachedBitmap( BitmapEx& rBmp ) const
{
   if( !mpImplWallpaper->mpCache )
      const_cast< ImplWallpaper* >(mpImplWallpaper.get())->mpCache = std::make_unique<BitmapEx>( rBmp );
   else
      *const_cast< ImplWallpaper* >(mpImplWallpaper.get())->mpCache = rBmp;
}

const BitmapEx* Wallpaper::ImplGetCachedBitmap() const
{
    return mpImplWallpaper->mpCache.get();
}

void Wallpaper::ImplReleaseCachedBitmap() const
{
    const_cast< ImplWallpaper* >(mpImplWallpaper.get())->mpCache.reset();
}

void Wallpaper::SetColor( const Color& rColor )
{
    ImplReleaseCachedBitmap();
    mpImplWallpaper->maColor = rColor;

    if( WallpaperStyle::NONE == mpImplWallpaper->meStyle || WallpaperStyle::ApplicationGradient == mpImplWallpaper->meStyle )
        mpImplWallpaper->meStyle = WallpaperStyle::Tile;
}

const Color& Wallpaper::GetColor() const
{
    return mpImplWallpaper->maColor;
}

void Wallpaper::SetStyle( WallpaperStyle eStyle )
{
    if( eStyle == WallpaperStyle::ApplicationGradient )
        // set a dummy gradient, the correct gradient
        // will be created dynamically in GetGradient()
        SetGradient( ImplGetApplicationGradient() );

    mpImplWallpaper->meStyle = eStyle;
}

WallpaperStyle Wallpaper::GetStyle() const
{
    return mpImplWallpaper->meStyle;
}

void Wallpaper::SetBitmap( const BitmapEx& rBitmap )
{
    if ( !rBitmap )
    {
        if ( mpImplWallpaper->mpBitmap )
        {
            ImplReleaseCachedBitmap();
            mpImplWallpaper->mpBitmap.reset();
        }
    }
    else
    {
        ImplReleaseCachedBitmap();
        if ( mpImplWallpaper->mpBitmap )
            *(mpImplWallpaper->mpBitmap) = rBitmap;
        else
            mpImplWallpaper->mpBitmap = std::make_unique<BitmapEx>( rBitmap );
    }

    if( WallpaperStyle::NONE == mpImplWallpaper->meStyle || WallpaperStyle::ApplicationGradient == mpImplWallpaper->meStyle)
        mpImplWallpaper->meStyle = WallpaperStyle::Tile;
}

BitmapEx Wallpaper::GetBitmap() const
{
    if ( mpImplWallpaper->mpBitmap )
        return *(mpImplWallpaper->mpBitmap);
    else
        return BitmapEx();
}

bool Wallpaper::IsBitmap() const
{
    return bool(mpImplWallpaper->mpBitmap);
}

void Wallpaper::SetGradient( const Gradient& rGradient )
{
    ImplReleaseCachedBitmap();

    if ( mpImplWallpaper->mpGradient )
        *(mpImplWallpaper->mpGradient) = rGradient;
    else
        mpImplWallpaper->mpGradient = std::make_unique<Gradient>( rGradient );

    if( WallpaperStyle::NONE == mpImplWallpaper->meStyle || WallpaperStyle::ApplicationGradient == mpImplWallpaper->meStyle )
        mpImplWallpaper->meStyle = WallpaperStyle::Tile;
}

Gradient Wallpaper::GetGradient() const
{
    if( WallpaperStyle::ApplicationGradient == mpImplWallpaper->meStyle )
        return ImplGetApplicationGradient();
    else if ( mpImplWallpaper->mpGradient )
        return *(mpImplWallpaper->mpGradient);
    else
        return Gradient();
}

bool Wallpaper::IsGradient() const
{
    return bool(mpImplWallpaper->mpGradient);
}

Gradient Wallpaper::ImplGetApplicationGradient()
{
    Gradient g;
    g.SetAngle( 900 );
    g.SetStyle( GradientStyle::Linear );
    g.SetStartColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    // no 'extreme' gradient when high contrast
    if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    else
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceGradientColor() );
    return g;
}

void Wallpaper::SetRect( const tools::Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        mpImplWallpaper->mpRect.reset();
    }
    else
    {
        mpImplWallpaper->mpRect = rRect;
    }
}

tools::Rectangle Wallpaper::GetRect() const
{
    if ( mpImplWallpaper->mpRect )
        return *mpImplWallpaper->mpRect;
    else
        return tools::Rectangle();
}

bool Wallpaper::IsRect() const
{
    return bool(mpImplWallpaper->mpRect);
}

bool Wallpaper::IsFixed() const
{
    if ( mpImplWallpaper->meStyle == WallpaperStyle::NONE )
        return false;
    else
        return (!mpImplWallpaper->mpBitmap && !mpImplWallpaper->mpGradient);
}

bool Wallpaper::IsScrollable() const
{
    if ( mpImplWallpaper->meStyle == WallpaperStyle::NONE )
        return false;
    else if ( !mpImplWallpaper->mpBitmap && !mpImplWallpaper->mpGradient )
        return true;
    else if ( mpImplWallpaper->mpBitmap )
        return (mpImplWallpaper->meStyle == WallpaperStyle::Tile);
    else
        return false;
}

Wallpaper& Wallpaper::operator=( const Wallpaper& ) = default;

Wallpaper& Wallpaper::operator=( Wallpaper&& ) = default;

bool Wallpaper::operator==( const Wallpaper& rWallpaper ) const
{
    return mpImplWallpaper.same_object(rWallpaper.mpImplWallpaper);
}

SvStream& ReadWallpaper( SvStream& rIStm, Wallpaper& rWallpaper )
{
    return ReadImplWallpaper( rIStm, *rWallpaper.mpImplWallpaper );
}

SvStream& WriteWallpaper( SvStream& rOStm, const Wallpaper& rWallpaper )
{
    return WriteImplWallpaper( rOStm, *rWallpaper.mpImplWallpaper );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
