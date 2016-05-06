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
    if ( meStyle == rImplWallpaper.meStyle &&
         maColor == rImplWallpaper.maColor &&
         mpRect == rImplWallpaper.mpRect &&
         mpBitmap == rImplWallpaper.mpBitmap &&
         mpGradient == rImplWallpaper.mpGradient )
        return true;
    return false;
}

SvStream& ReadImplWallpaper( SvStream& rIStm, ImplWallpaper& rImplWallpaper )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16;

    delete rImplWallpaper.mpRect;
    rImplWallpaper.mpRect = nullptr;

    delete rImplWallpaper.mpGradient;
    rImplWallpaper.mpGradient = nullptr;

    delete rImplWallpaper.mpBitmap;
    rImplWallpaper.mpBitmap = nullptr;

    // version 1
    ReadColor( rIStm, rImplWallpaper.maColor );
    rIStm.ReadUInt16( nTmp16 ); rImplWallpaper.meStyle = (WallpaperStyle) nTmp16;

    // version 2
    if( aCompat.GetVersion() >= 2 )
    {
        bool bRect, bGrad, bBmp, bDummy;

        rIStm.ReadCharAsBool( bRect ).ReadCharAsBool( bGrad ).ReadCharAsBool( bBmp ).ReadCharAsBool( bDummy ).ReadCharAsBool( bDummy ).ReadCharAsBool( bDummy );

        if( bRect )
        {
            rImplWallpaper.mpRect = new Rectangle;
            ReadRectangle( rIStm, *rImplWallpaper.mpRect );
        }

        if( bGrad )
        {
            rImplWallpaper.mpGradient = new Gradient;
            ReadGradient( rIStm, *rImplWallpaper.mpGradient );
        }

        if( bBmp )
        {
            rImplWallpaper.mpBitmap = new BitmapEx;
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
    bool            bRect = ( rImplWallpaper.mpRect != nullptr );
    bool            bGrad = ( rImplWallpaper.mpGradient != nullptr );
    bool            bBmp = ( rImplWallpaper.mpBitmap != nullptr );
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
    ( (Color&) rImplWallpaper.maColor ).Write( rOStm );

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

void Wallpaper::ImplSetCachedBitmap( BitmapEx& rBmp ) const
{
   if( !mpImplWallpaper->mpCache )
      const_cast< ImplWallpaper* >(mpImplWallpaper.get())->mpCache = new BitmapEx( rBmp );
   else
      *const_cast< ImplWallpaper* >(mpImplWallpaper.get())->mpCache = rBmp;
}

const BitmapEx* Wallpaper::ImplGetCachedBitmap() const
{
    return mpImplWallpaper->mpCache;
}

void Wallpaper::ImplReleaseCachedBitmap() const
{
    delete mpImplWallpaper->mpCache;
    const_cast< ImplWallpaper* >(mpImplWallpaper.get())->mpCache = nullptr;
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
            delete mpImplWallpaper->mpBitmap;
            mpImplWallpaper->mpBitmap = nullptr;
        }
    }
    else
    {
        ImplReleaseCachedBitmap();
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
    ImplReleaseCachedBitmap();

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
    return ReadImplWallpaper( rIStm, *rWallpaper.mpImplWallpaper );
}

SvStream& WriteWallpaper( SvStream& rOStm, const Wallpaper& rWallpaper )
{
    return WriteImplWallpaper( rOStm, *rWallpaper.mpImplWallpaper );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
