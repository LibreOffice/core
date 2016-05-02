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
#include <tools/debug.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/gradient.hxx>
#include <vcl/wall.hxx>
#include <vcl/svapp.hxx>
#include <wall2.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/settings.hxx>

ImplWallpaper::ImplWallpaper() :
    maColor( COL_TRANSPARENT )
{
    mpBitmap        = nullptr;
    mpCache         = nullptr;
    mpGradient      = nullptr;
    mpRect          = nullptr;
    meStyle         = WallpaperStyle::NONE;
}

ImplWallpaper::ImplWallpaper( const ImplWallpaper& rImplWallpaper ) :
    maColor( rImplWallpaper.maColor )
{
    meStyle = rImplWallpaper.meStyle;

    if ( rImplWallpaper.mpBitmap )
        mpBitmap = new BitmapEx( *rImplWallpaper.mpBitmap );
    else
        mpBitmap = nullptr;
    if( rImplWallpaper.mpCache )
        mpCache = new BitmapEx( *rImplWallpaper.mpCache );
    else
        mpCache = nullptr;
    if ( rImplWallpaper.mpGradient )
        mpGradient = new Gradient( *rImplWallpaper.mpGradient );
    else
        mpGradient = nullptr;
    if ( rImplWallpaper.mpRect )
        mpRect = new Rectangle( *rImplWallpaper.mpRect );
    else
        mpRect = nullptr;
}

ImplWallpaper::~ImplWallpaper()
{
    delete mpBitmap;
    delete mpCache;
    delete mpGradient;
    delete mpRect;
}

bool ImplWallpaper::operator==( const ImplWallpaper& rImplWallpaper ) const
{
    if ( meStyle == rWallpaper.mpImplWallpaper->meStyle &&
         maColor == rWallpaper.mpImplWallpaper->maColor &&
         mpRect == rWallpaper.mpImplWallpaper->mpRect &&
         mpBitmap == rWallpaper.mpImplWallpaper->mpBitmap &&
         mpGradient == rWallpaper.mpImplWallpaper->mpGradient )
        return true;
    return false;
}

void ImplWallpaper::ImplSetCachedBitmap( BitmapEx& rBmp )
{
    if( !mpCache )
        mpCache = new BitmapEx( rBmp );
    else
        *mpCache = rBmp;
}

void ImplWallpaper::ImplReleaseCachedBitmap()
{
    delete mpCache;
    mpCache = nullptr;
}

namespace
{
    struct theGlobalDefault :
        public rtl::Static< Wallpaper::ImplType, theGlobalDefault > {};
}

Wallpaper::Wallpaper() : mpImplWallpaper(theGlobalDefault::get())
{
}

Wallpaper::Wallpaper( const Wallpaper& rWallpaper )
    : mpImplWallpaper( rWallpaper.mpImplWallpaper)
{
}

Wallpaper::Wallpaper( const Color& rColor ) : mpImplWallpaper()
{
    mpImplWallpaper->maColor    = rColor;
    mpImplWallpaper->meStyle    = WallpaperStyle::Tile;
}

Wallpaper::Wallpaper( const BitmapEx& rBmpEx ) : mpImplWallpaper()
{
    mpImplWallpaper->mpBitmap   = new BitmapEx( rBmpEx );
    mpImplWallpaper->meStyle    = WallpaperStyle::Tile;
}

Wallpaper::Wallpaper( const Gradient& rGradient ) : mpImplWallpaper()
{
    mpImplWallpaper->mpGradient = new Gradient( rGradient );
    mpImplWallpaper->meStyle    = WallpaperStyle::Tile;
}

Wallpaper::~Wallpaper()
{
}

void Wallpaper::SetColor( const Color& rColor )
{
    mpImplWallpaper->ImplReleaseCachedBitmap();
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
            mpImplWallpaper->ImplReleaseCachedBitmap();
            delete mpImplWallpaper->mpBitmap;
            mpImplWallpaper->mpBitmap = nullptr;
        }
    }
    else
    {
        mpImplWallpaper->ImplReleaseCachedBitmap();
        if ( mpImplWallpaper->mpBitmap )
            *(mpImplWallpaper->mpBitmap) = rBitmap;
        else
            mpImplWallpaper->mpBitmap = new BitmapEx( rBitmap );
    }

    if( WallpaperStyle::NONE == mpImplWallpaper->meStyle || WallpaperStyle::ApplicationGradient == mpImplWallpaper->meStyle)
        mpImplWallpaper->meStyle = WallpaperStyle::Tile;
}

BitmapEx Wallpaper::GetBitmap() const
{
    if ( mpImplWallpaper->mpBitmap )
        return *(mpImplWallpaper->mpBitmap);
    else
    {
        BitmapEx aBmp;
        return aBmp;
    }
}

bool Wallpaper::IsBitmap() const
{
    return (mpImplWallpaper->mpBitmap != nullptr);
}

void Wallpaper::SetGradient( const Gradient& rGradient )
{
    mpImplWallpaper->ImplReleaseCachedBitmap();

    if ( mpImplWallpaper->mpGradient )
        *(mpImplWallpaper->mpGradient) = rGradient;
    else
        mpImplWallpaper->mpGradient = new Gradient( rGradient );

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
    {
        Gradient aGradient;
        return aGradient;
    }
}

bool Wallpaper::IsGradient() const
{
    return (mpImplWallpaper->mpGradient != nullptr);
}

Gradient Wallpaper::ImplGetApplicationGradient() const
{
    Gradient g;
    g.SetAngle( 900 );
    g.SetStyle( GradientStyle_LINEAR );
    g.SetStartColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    // no 'extreme' gradient when high contrast
    if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    else
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceGradientColor() );
    return g;
}

void Wallpaper::SetRect( const Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        if ( mpImplWallpaper->mpRect )
        {
            delete mpImplWallpaper->mpRect;
            mpImplWallpaper->mpRect = nullptr;
        }
    }
    else
    {
        if ( mpImplWallpaper->mpRect )
            *(mpImplWallpaper->mpRect) = rRect;
        else
            mpImplWallpaper->mpRect = new Rectangle( rRect );
    }
}

Rectangle Wallpaper::GetRect() const
{
    if ( mpImplWallpaper->mpRect )
        return *(mpImplWallpaper->mpRect);
    else
    {
        Rectangle aRect;
        return aRect;
    }
}

bool Wallpaper::IsRect() const
{

    return (mpImplWallpaper->mpRect != nullptr);
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

Wallpaper& Wallpaper::operator=( const Wallpaper& rWallpaper )
{
    mpImplWallpaper = rWallpaper.mpImplWallpaper;
    return *this;
}

bool Wallpaper::operator==( const Wallpaper& rWallpaper ) const
{
    return mpImplWallpaper == rWallpaper.mpImplWallpaper;
}

SvStream& ReadWallpaper( SvStream& rIStm, Wallpaper& rWallpaper )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16;

    deleterWallpaper.mpImplWallpaper->mpRect;
    rWallpaper.mpImplWallpaper->mpRect = nullptr;

    deleterWallpaper.mpImplWallpaper->mpGradient;
    rWallpaper.mpImplWallpaper->mpGradient = nullptr;

    deleterWallpaper.mpImplWallpaper->mpBitmap;
    rWallpaper.mpImplWallpaper->mpBitmap = nullptr;

    // version 1
    ReadColor( rIStm,rWallpaper.mpImplWallpaper->maColor );
    rIStm.ReadUInt16( nTmp16 );
    rWallpaper.mpImplWallpaper->meStyle = (WallpaperStyle) nTmp16;

    // version 2
    if( aCompat.GetVersion() >= 2 )
    {
        bool bRect, bGrad, bBmp, bDummy;

        rIStm.ReadCharAsBool( bRect )
             .ReadCharAsBool( bGrad )
             .ReadCharAsBool( bBmp )
             .ReadCharAsBool( bDummy )
             .ReadCharAsBool( bDummy )
             .ReadCharAsBool( bDummy );

        if( bRect )
        {
            rWallpaper.mpImplWallpaper->mpRect = new Rectangle;
            ReadRectangle( rIStm, *rWallpaper.mpImplWallpaper->mpRect );
        }

        if( bGrad )
        {
            rWallpaper.mpImplWallpaper->mpGradient = new Gradient;
            ReadGradient( rIStm, *rWallpaper.mpImplWallpaper->mpGradient );
        }

        if( bBmp )
        {
            rWallpaper.mpImplWallpaper->mpBitmap = new BitmapEx;
            ReadDIBBitmapEx(*rWallpaper.mpImplWallpaper->mpBitmap, rIStm);
        }

        // version 3 (new color format)
        if( aCompat.GetVersion() >= 3 )
        {
            rWallpaper.mpImplWallpaper->maColor.Read( rIStm );
        }
    }

    return rIStm;
}

SvStream& WriteWallpaper( SvStream& rOStm, const Wallpaper& rWallpaper )
{
    VersionCompat   aCompat( rOStm, StreamMode::WRITE, 3 );
    bool            bRect = ( rWallpaper.mpImplWallpaper->mpRect != nullptr );
    bool            bGrad = ( rWallpaper.mpImplWallpaper->mpGradient != nullptr );
    bool            bBmp = ( rWallpaper.mpImplWallpaper->mpBitmap != nullptr );
    bool            bDummy = false;

    // version 1
    WriteColor( rOStm, rWallpaper.mpImplWallpaper->maColor );
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rWallpaper.mpImplWallpaper->meStyle) );

    // version 2
    rOStm.WriteBool( bRect )
         .WriteBool( bGrad )
         .WriteBool( bBmp )
         .WriteBool( bDummy )
         .WriteBool( bDummy )
         .WriteBool( bDummy );

    if( bRect )
        WriteRectangle( rOStm, *rWallpaper.mpImplWallpaper->mpRect );

    if( bGrad )
        WriteGradient( rOStm, *rWallpaper.mpImplWallpaper->mpGradient );

    if( bBmp )
        WriteDIBBitmapEx(*rWallpaper.mpImplWallpaper->mpBitmap, rOStm);

    // version 3 (new color format)
    ( (Color&) rWallpaper.mpImplWallpaper->maColor ).Write( rOStm );

    return rOStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
