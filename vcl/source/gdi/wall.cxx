/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>

#include <vcl/bitmapex.hxx>
#include <vcl/gradient.hxx>
#include <vcl/wall.hxx>
#include <vcl/svapp.hxx>

#include <wall2.hxx>


DBG_NAME( Wallpaper )

// -----------------------------------------------------------------------

ImplWallpaper::ImplWallpaper() :
    maColor( COL_TRANSPARENT )
{
    mnRefCount      = 1;
    mpBitmap        = NULL;
    mpCache         = NULL;
    mpGradient      = NULL;
    mpRect          = NULL;
    meStyle         = WALLPAPER_NULL;
}

// -----------------------------------------------------------------------

ImplWallpaper::ImplWallpaper( const ImplWallpaper& rImplWallpaper ) :
    maColor( rImplWallpaper.maColor )
{
    mnRefCount = 1;
    meStyle = rImplWallpaper.meStyle;

    if ( rImplWallpaper.mpBitmap )
        mpBitmap = new BitmapEx( *rImplWallpaper.mpBitmap );
    else
        mpBitmap = NULL;
    if( rImplWallpaper.mpCache )
        mpCache = new BitmapEx( *rImplWallpaper.mpCache );
    else
        mpCache = NULL;
    if ( rImplWallpaper.mpGradient )
        mpGradient = new Gradient( *rImplWallpaper.mpGradient );
    else
        mpGradient = NULL;
    if ( rImplWallpaper.mpRect )
        mpRect = new Rectangle( *rImplWallpaper.mpRect );
    else
        mpRect = NULL;
}

// -----------------------------------------------------------------------

ImplWallpaper::~ImplWallpaper()
{
    delete mpBitmap;
    delete mpCache;
    delete mpGradient;
    delete mpRect;
}

// -----------------------------------------------------------------------

void ImplWallpaper::ImplSetCachedBitmap( BitmapEx& rBmp )
{
    if( !mpCache )
        mpCache = new BitmapEx( rBmp );
    else
        *mpCache = rBmp;
}

// -----------------------------------------------------------------------

void ImplWallpaper::ImplReleaseCachedBitmap()
{
    delete mpCache;
    mpCache = NULL;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImplWallpaper& rImplWallpaper )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;

    delete rImplWallpaper.mpRect;
    rImplWallpaper.mpRect = NULL;

    delete rImplWallpaper.mpGradient;
    rImplWallpaper.mpGradient = NULL;

    delete rImplWallpaper.mpBitmap;
    rImplWallpaper.mpBitmap = NULL;

    // version 1
    rIStm >> rImplWallpaper.maColor;
    rIStm >> nTmp16; rImplWallpaper.meStyle = (WallpaperStyle) nTmp16;

    // version 2
    if( aCompat.GetVersion() >= 2 )
    {
        sal_Bool bRect, bGrad, bBmp, bDummy;

        rIStm >> bRect >> bGrad >> bBmp >> bDummy >> bDummy >> bDummy;

        if( bRect )
        {
            rImplWallpaper.mpRect = new Rectangle;
            rIStm >> *rImplWallpaper.mpRect;
        }

        if( bGrad )
        {
            rImplWallpaper.mpGradient = new Gradient;
            rIStm >> *rImplWallpaper.mpGradient;
        }

        if( bBmp )
        {
            rImplWallpaper.mpBitmap = new BitmapEx;
            rIStm >> *rImplWallpaper.mpBitmap;
        }

        // version 3 (new color format)
        if( aCompat.GetVersion() >= 3 )
        {
            rImplWallpaper.maColor.Read( rIStm, sal_True );
        }
    }

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplWallpaper& rImplWallpaper )
{
    VersionCompat   aCompat( rOStm, STREAM_WRITE, 3 );
    sal_Bool            bRect = ( rImplWallpaper.mpRect != NULL );
    sal_Bool            bGrad = ( rImplWallpaper.mpGradient != NULL );
    sal_Bool            bBmp = ( rImplWallpaper.mpBitmap != NULL );
    sal_Bool            bDummy = sal_False;

    // version 1
    rOStm << rImplWallpaper.maColor << (sal_uInt16) rImplWallpaper.meStyle;

    // version 2
    rOStm << bRect << bGrad << bBmp << bDummy << bDummy << bDummy;

    if( bRect )
        rOStm << *rImplWallpaper.mpRect;

    if( bGrad )
        rOStm << *rImplWallpaper.mpGradient;

    if( bBmp )
        rOStm << *rImplWallpaper.mpBitmap;

    // version 3 (new color format)
    ( (Color&) rImplWallpaper.maColor ).Write( rOStm, sal_True );

    return rOStm;
}

// -----------------------------------------------------------------------

inline void Wallpaper::ImplMakeUnique( sal_Bool bReleaseCache )
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpImplWallpaper->mnRefCount != 1 )
    {
        if ( mpImplWallpaper->mnRefCount )
            mpImplWallpaper->mnRefCount--;
        mpImplWallpaper = new ImplWallpaper( *(mpImplWallpaper) );
    }

    if( bReleaseCache )
        mpImplWallpaper->ImplReleaseCachedBitmap();
}

// -----------------------------------------------------------------------

Wallpaper::Wallpaper()
{
    DBG_CTOR( Wallpaper, NULL );

    static ImplWallpaper aStaticImplWallpaper;

    aStaticImplWallpaper.mnRefCount = 0;
    mpImplWallpaper = &aStaticImplWallpaper;
}

// -----------------------------------------------------------------------

Wallpaper::Wallpaper( const Wallpaper& rWallpaper )
{
    DBG_CTOR( Wallpaper, NULL );
    DBG_CHKOBJ( &rWallpaper, Wallpaper, NULL );
    DBG_ASSERT( rWallpaper.mpImplWallpaper->mnRefCount < 0xFFFFFFFE, "Wallpaper: RefCount overflow" );

    // Instance Daten uebernehmen und Referenzcounter erhoehen
    mpImplWallpaper = rWallpaper.mpImplWallpaper;
    // RefCount == 0 fuer statische Objekte
    if ( mpImplWallpaper->mnRefCount )
        mpImplWallpaper->mnRefCount++;
}

// -----------------------------------------------------------------------

Wallpaper::Wallpaper( const Color& rColor )
{
    DBG_CTOR( Wallpaper, NULL );

    mpImplWallpaper             = new ImplWallpaper;
    mpImplWallpaper->maColor    = rColor;
    mpImplWallpaper->meStyle    = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

Wallpaper::Wallpaper( const BitmapEx& rBmpEx )
{
    DBG_CTOR( Wallpaper, NULL );

    mpImplWallpaper             = new ImplWallpaper;
    mpImplWallpaper->mpBitmap   = new BitmapEx( rBmpEx );
    mpImplWallpaper->meStyle    = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

Wallpaper::Wallpaper( const Gradient& rGradient )
{
    DBG_CTOR( Wallpaper, NULL );

    mpImplWallpaper             = new ImplWallpaper;
    mpImplWallpaper->mpGradient = new Gradient( rGradient );
    mpImplWallpaper->meStyle    = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

Wallpaper::~Wallpaper()
{
    DBG_DTOR( Wallpaper, NULL );

    // Wenn es keine statischen ImpDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
    if ( mpImplWallpaper->mnRefCount )
    {
        if ( mpImplWallpaper->mnRefCount == 1 )
            delete mpImplWallpaper;
        else
            mpImplWallpaper->mnRefCount--;
    }
}

// -----------------------------------------------------------------------

void Wallpaper::SetColor( const Color& rColor )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique();
    mpImplWallpaper->maColor = rColor;

    if( WALLPAPER_NULL == mpImplWallpaper->meStyle || WALLPAPER_APPLICATIONGRADIENT == mpImplWallpaper->meStyle )
        mpImplWallpaper->meStyle = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

const Color& Wallpaper::GetColor() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    return mpImplWallpaper->maColor;
}

// -----------------------------------------------------------------------

void Wallpaper::SetStyle( WallpaperStyle eStyle )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique( sal_False );

    if( eStyle == WALLPAPER_APPLICATIONGRADIENT )
        // set a dummy gradient, the correct gradient
        // will be created dynamically in GetGradient()
        SetGradient( ImplGetApplicationGradient() );

    mpImplWallpaper->meStyle = eStyle;
}

// -----------------------------------------------------------------------

WallpaperStyle Wallpaper::GetStyle() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    return mpImplWallpaper->meStyle;
}

// -----------------------------------------------------------------------

void Wallpaper::SetBitmap( const BitmapEx& rBitmap )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if ( !rBitmap )
    {
        if ( mpImplWallpaper->mpBitmap )
        {
            ImplMakeUnique();
            delete mpImplWallpaper->mpBitmap;
            mpImplWallpaper->mpBitmap = NULL;
        }
    }
    else
    {
        ImplMakeUnique();
        if ( mpImplWallpaper->mpBitmap )
            *(mpImplWallpaper->mpBitmap) = rBitmap;
        else
            mpImplWallpaper->mpBitmap = new BitmapEx( rBitmap );
    }

    if( WALLPAPER_NULL == mpImplWallpaper->meStyle || WALLPAPER_APPLICATIONGRADIENT == mpImplWallpaper->meStyle)
        mpImplWallpaper->meStyle = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

void Wallpaper::SetBitmap()
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if ( mpImplWallpaper->mpBitmap )
    {
        ImplMakeUnique();
        delete mpImplWallpaper->mpBitmap;
        mpImplWallpaper->mpBitmap = NULL;
    }
}

// -----------------------------------------------------------------------

BitmapEx Wallpaper::GetBitmap() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if ( mpImplWallpaper->mpBitmap )
        return *(mpImplWallpaper->mpBitmap);
    else
    {
        BitmapEx aBmp;
        return aBmp;
    }
}

// -----------------------------------------------------------------------

sal_Bool Wallpaper::IsBitmap() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    return (mpImplWallpaper->mpBitmap != 0);
}


// -----------------------------------------------------------------------

void Wallpaper::SetGradient( const Gradient& rGradient )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique();

    if ( mpImplWallpaper->mpGradient )
        *(mpImplWallpaper->mpGradient) = rGradient;
    else
        mpImplWallpaper->mpGradient = new Gradient( rGradient );

    if( WALLPAPER_NULL == mpImplWallpaper->meStyle || WALLPAPER_APPLICATIONGRADIENT == mpImplWallpaper->meStyle )
        mpImplWallpaper->meStyle = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

void Wallpaper::SetGradient()
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if ( mpImplWallpaper->mpGradient )
    {
        ImplMakeUnique();
        delete mpImplWallpaper->mpGradient;
        mpImplWallpaper->mpGradient = NULL;
    }
}

// -----------------------------------------------------------------------

Gradient Wallpaper::GetGradient() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if( WALLPAPER_APPLICATIONGRADIENT == mpImplWallpaper->meStyle )
        return ImplGetApplicationGradient();
    else if ( mpImplWallpaper->mpGradient )
        return *(mpImplWallpaper->mpGradient);
    else
    {
        Gradient aGradient;
        return aGradient;
    }
}

// -----------------------------------------------------------------------

sal_Bool Wallpaper::IsGradient() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    return (mpImplWallpaper->mpGradient != 0);
}


// -----------------------------------------------------------------------

Gradient Wallpaper::ImplGetApplicationGradient() const
{
    Gradient g;
    g.SetAngle( 900 );
    g.SetStyle( GRADIENT_LINEAR );
    g.SetStartColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    // no 'extreme' gradient when high contrast
    if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    else
        g.SetEndColor( Application::GetSettings().GetStyleSettings().GetFaceGradientColor() );
    return g;
}

// -----------------------------------------------------------------------

void Wallpaper::SetRect( const Rectangle& rRect )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique( sal_False );

    if ( rRect.IsEmpty() )
    {
        if ( mpImplWallpaper->mpRect )
        {
            delete mpImplWallpaper->mpRect;
            mpImplWallpaper->mpRect = NULL;
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

// -----------------------------------------------------------------------

void Wallpaper::SetRect()
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if ( mpImplWallpaper->mpRect )
    {
        ImplMakeUnique( sal_False );
        delete mpImplWallpaper->mpRect;
        mpImplWallpaper->mpRect = NULL;
    }
}

// -----------------------------------------------------------------------

Rectangle Wallpaper::GetRect() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    if ( mpImplWallpaper->mpRect )
        return *(mpImplWallpaper->mpRect);
    else
    {
        Rectangle aRect;
        return aRect;
    }
}

// -----------------------------------------------------------------------

sal_Bool Wallpaper::IsRect() const
{
    DBG_CHKTHIS( Wallpaper, NULL );

    return (mpImplWallpaper->mpRect != 0);
}


// -----------------------------------------------------------------------

sal_Bool Wallpaper::IsFixed() const
{
    if ( mpImplWallpaper->meStyle == WALLPAPER_NULL )
        return sal_False;
    else
        return (!mpImplWallpaper->mpBitmap && !mpImplWallpaper->mpGradient);
}

// -----------------------------------------------------------------------

sal_Bool Wallpaper::IsScrollable() const
{
    if ( mpImplWallpaper->meStyle == WALLPAPER_NULL )
        return sal_False;
    else if ( !mpImplWallpaper->mpBitmap && !mpImplWallpaper->mpGradient )
        return sal_True;
    else if ( mpImplWallpaper->mpBitmap )
        return (mpImplWallpaper->meStyle == WALLPAPER_TILE);
    else
        return sal_False;
}

// -----------------------------------------------------------------------

Wallpaper& Wallpaper::operator=( const Wallpaper& rWallpaper )
{
    DBG_CHKTHIS( Wallpaper, NULL );
    DBG_CHKOBJ( &rWallpaper, Wallpaper, NULL );
    DBG_ASSERT( rWallpaper.mpImplWallpaper->mnRefCount < 0xFFFFFFFE, "Wallpaper: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    if ( rWallpaper.mpImplWallpaper->mnRefCount )
        rWallpaper.mpImplWallpaper->mnRefCount++;

    // Wenn es keine statischen ImpDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
    if ( mpImplWallpaper->mnRefCount )
    {
        if ( mpImplWallpaper->mnRefCount == 1 )
            delete mpImplWallpaper;
        else
            mpImplWallpaper->mnRefCount--;
    }

    mpImplWallpaper = rWallpaper.mpImplWallpaper;

    return *this;
}

// -----------------------------------------------------------------------

sal_Bool Wallpaper::operator==( const Wallpaper& rWallpaper ) const
{
    DBG_CHKTHIS( Wallpaper, NULL );
    DBG_CHKOBJ( &rWallpaper, Wallpaper, NULL );

    if ( mpImplWallpaper == rWallpaper.mpImplWallpaper )
        return sal_True;

    if ( ( mpImplWallpaper->meStyle != rWallpaper.mpImplWallpaper->meStyle ) ||
         ( mpImplWallpaper->maColor != rWallpaper.mpImplWallpaper->maColor ) )
        return sal_False;

    if ( mpImplWallpaper->mpRect != rWallpaper.mpImplWallpaper->mpRect
         && ( !mpImplWallpaper->mpRect
              || !rWallpaper.mpImplWallpaper->mpRect
              || *(mpImplWallpaper->mpRect) != *(rWallpaper.mpImplWallpaper->mpRect) ) )
        return sal_False;

    if ( mpImplWallpaper->mpBitmap != rWallpaper.mpImplWallpaper->mpBitmap
         && ( !mpImplWallpaper->mpBitmap
              || !rWallpaper.mpImplWallpaper->mpBitmap
              || *(mpImplWallpaper->mpBitmap) != *(rWallpaper.mpImplWallpaper->mpBitmap) ) )
        return sal_False;

    if ( mpImplWallpaper->mpGradient != rWallpaper.mpImplWallpaper->mpGradient
         && ( !mpImplWallpaper->mpGradient
              || !rWallpaper.mpImplWallpaper->mpGradient
              || *(mpImplWallpaper->mpGradient) != *(rWallpaper.mpImplWallpaper->mpGradient) ) )
        return sal_False;

    return sal_True;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Wallpaper& rWallpaper )
{
    rWallpaper.ImplMakeUnique();
    return( rIStm >> *rWallpaper.mpImplWallpaper );
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Wallpaper& rWallpaper )
{
    return( rOStm << *rWallpaper.mpImplWallpaper );
}
