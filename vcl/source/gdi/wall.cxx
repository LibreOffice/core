/*************************************************************************
 *
 *  $RCSfile: wall.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_WALL_CXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <wall.hxx>
#endif

// =======================================================================

DBG_NAME( Wallpaper );

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
    UINT16          nTmp16;

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
        BOOL bRect, bGrad, bBmp, bDummy;

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
            rImplWallpaper.maColor.Read( rIStm, TRUE );
        }
    }

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplWallpaper& rImplWallpaper )
{
    VersionCompat   aCompat( rOStm, STREAM_WRITE, 3 );
    BOOL            bRect = ( rImplWallpaper.mpRect != NULL );
    BOOL            bGrad = ( rImplWallpaper.mpGradient != NULL );
    BOOL            bBmp = ( rImplWallpaper.mpBitmap != NULL );
    BOOL            bDummy = FALSE;

    // version 1
    rOStm << rImplWallpaper.maColor << (UINT16) rImplWallpaper.meStyle;

    // version 2
    rOStm << bRect << bGrad << bBmp << bDummy << bDummy << bDummy;

    if( bRect )
        rOStm << *rImplWallpaper.mpRect;

    if( bGrad )
        rOStm << *rImplWallpaper.mpGradient;

    if( bBmp )
        rOStm << *rImplWallpaper.mpBitmap;

    // version 3 (new color format)
    ( (Color&) rImplWallpaper.maColor ).Write( rOStm, TRUE );

    return rOStm;
}

// -----------------------------------------------------------------------

inline void Wallpaper::ImplMakeUnique( BOOL bReleaseCache )
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

#ifdef WIN
    static ImplWallpaper _near aStaticImplWallpaper;
#else
    static ImplWallpaper aStaticImplWallpaper;
#endif

    aStaticImplWallpaper.mnRefCount = 0;
    mpImplWallpaper = &aStaticImplWallpaper;
}

// -----------------------------------------------------------------------

Wallpaper::Wallpaper( const Wallpaper& rWallpaper )
{
    DBG_CTOR( Wallpaper, NULL );
    DBG_CHKOBJ( &rWallpaper, Wallpaper, NULL );
    DBG_ASSERT( rWallpaper.mpImplWallpaper->mnRefCount < 0xFFFE, "Wallpaper: RefCount overflow" );

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

    if( WALLPAPER_NULL == mpImplWallpaper->meStyle )
        mpImplWallpaper->meStyle = WALLPAPER_TILE;
}

// -----------------------------------------------------------------------

void Wallpaper::SetStyle( WallpaperStyle eStyle )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique( FALSE );
    mpImplWallpaper->meStyle = eStyle;
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

    if( WALLPAPER_NULL == mpImplWallpaper->meStyle )
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

void Wallpaper::SetGradient( const Gradient& rGradient )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique();

    if ( mpImplWallpaper->mpGradient )
        *(mpImplWallpaper->mpGradient) = rGradient;
    else
        mpImplWallpaper->mpGradient = new Gradient( rGradient );

    if( WALLPAPER_NULL == mpImplWallpaper->meStyle )
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

    if ( mpImplWallpaper->mpGradient )
        return *(mpImplWallpaper->mpGradient);
    else
    {
        Gradient aGradient;
        return aGradient;
    }
}

// -----------------------------------------------------------------------

void Wallpaper::SetRect( const Rectangle& rRect )
{
    DBG_CHKTHIS( Wallpaper, NULL );

    ImplMakeUnique( FALSE );

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
        ImplMakeUnique( FALSE );
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

BOOL Wallpaper::IsFixed() const
{
    if ( mpImplWallpaper->meStyle == WALLPAPER_NULL )
        return FALSE;
    else
        return (!mpImplWallpaper->mpBitmap && !mpImplWallpaper->mpGradient);
}

// -----------------------------------------------------------------------

BOOL Wallpaper::IsScrollable() const
{
    if ( mpImplWallpaper->meStyle == WALLPAPER_NULL )
        return FALSE;
    else if ( !mpImplWallpaper->mpBitmap && !mpImplWallpaper->mpGradient )
        return TRUE;
    else if ( mpImplWallpaper->mpBitmap )
        return (mpImplWallpaper->meStyle == WALLPAPER_TILE);
    else
        return FALSE;
}

// -----------------------------------------------------------------------

Wallpaper& Wallpaper::operator=( const Wallpaper& rWallpaper )
{
    DBG_CHKTHIS( Wallpaper, NULL );
    DBG_CHKOBJ( &rWallpaper, Wallpaper, NULL );
    DBG_ASSERT( rWallpaper.mpImplWallpaper->mnRefCount < 0xFFFE, "Wallpaper: RefCount overflow" );

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

BOOL Wallpaper::operator==( const Wallpaper& rWallpaper ) const
{
    DBG_CHKTHIS( Wallpaper, NULL );
    DBG_CHKOBJ( &rWallpaper, Wallpaper, NULL );

    if ( mpImplWallpaper == rWallpaper.mpImplWallpaper )
        return TRUE;

    if ( ( mpImplWallpaper->meStyle != rWallpaper.mpImplWallpaper->meStyle ) ||
         ( mpImplWallpaper->maColor != rWallpaper.mpImplWallpaper->maColor ) )
        return FALSE;

    if ( mpImplWallpaper->mpRect != rWallpaper.mpImplWallpaper->mpRect
         && ( !mpImplWallpaper->mpRect
              || !rWallpaper.mpImplWallpaper->mpRect
              || *(mpImplWallpaper->mpRect) != *(rWallpaper.mpImplWallpaper->mpRect) ) )
        return FALSE;

    if ( mpImplWallpaper->mpBitmap != rWallpaper.mpImplWallpaper->mpBitmap
         && ( !mpImplWallpaper->mpBitmap
              || !rWallpaper.mpImplWallpaper->mpBitmap
              || *(mpImplWallpaper->mpBitmap) != *(rWallpaper.mpImplWallpaper->mpBitmap) ) )
        return FALSE;

    if ( mpImplWallpaper->mpGradient != rWallpaper.mpImplWallpaper->mpGradient
         && ( !mpImplWallpaper->mpGradient
              || !rWallpaper.mpImplWallpaper->mpGradient
              || *(mpImplWallpaper->mpGradient) != *(rWallpaper.mpImplWallpaper->mpGradient) ) )
        return FALSE;

    return TRUE;
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
