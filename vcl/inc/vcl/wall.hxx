/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wall.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:16:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_WALL_HXX
#define _SV_WALL_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

class Rectangle;
class Gradient;
class BitmapEx;
class ImplWallpaper;

// -------------------
// - Wallpaper-Types -
// -------------------

#define WALLPAPER_NULL                  WallpaperStyle_NULL
#define WALLPAPER_TILE                  WallpaperStyle_TILE
#define WALLPAPER_CENTER                WallpaperStyle_CENTER
#define WALLPAPER_SCALE                 WallpaperStyle_SCALE
#define WALLPAPER_TOPLEFT               WallpaperStyle_TOPLEFT
#define WALLPAPER_TOP                   WallpaperStyle_TOP
#define WALLPAPER_TOPRIGHT              WallpaperStyle_TOPRIGHT
#define WALLPAPER_LEFT                  WallpaperStyle_LEFT
#define WALLPAPER_RIGHT                 WallpaperStyle_RIGHT
#define WALLPAPER_BOTTOMLEFT            WallpaperStyle_BOTTOMLEFT
#define WALLPAPER_BOTTOM                WallpaperStyle_BOTTOM
#define WALLPAPER_BOTTOMRIGHT           WallpaperStyle_BOTTOMRIGHT
#define WALLPAPER_APPLICATIONGRADIENT   WallpaperStyle_APPLICATIONGRADIENT
#define WALLPAPER_FORCE_EQUAL_SIZE      WallpaperStyle_FORCE_EQUAL_SIZE

#ifndef ENUM_WALLPAPERSTYLE_DECLARED
#define ENUM_WALLPAPERSTYLE_DECLARED

enum WallpaperStyle
{
    WALLPAPER_NULL,
    WALLPAPER_TILE,
    WALLPAPER_CENTER,
    WALLPAPER_SCALE,
    WALLPAPER_TOPLEFT,
    WALLPAPER_TOP,
    WALLPAPER_TOPRIGHT,
    WALLPAPER_LEFT,
    WALLPAPER_RIGHT,
    WALLPAPER_BOTTOMLEFT,
    WALLPAPER_BOTTOM,
    WALLPAPER_BOTTOMRIGHT,
    WALLPAPER_APPLICATIONGRADIENT,          // defines a gradient that internally covers the whole application
                                            // and uses a color derived from the face color
    WALLPAPER_FORCE_EQUAL_SIZE = 0x7fffffff
};

#endif

// -------------
// - Wallpaper -
// -------------

class VCL_DLLPUBLIC Wallpaper
{
private:
    ImplWallpaper*  mpImplWallpaper;

    SAL_DLLPRIVATE void           ImplMakeUnique( BOOL bReleaseCache = TRUE );
    SAL_DLLPRIVATE Gradient       ImplGetApplicationGradient() const;

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE ImplWallpaper* ImplGetImpWallpaper() const { return mpImplWallpaper; }
//#endif

public:
                    Wallpaper();
                    Wallpaper( const Wallpaper& rWallpaper );
                    Wallpaper( const Color& rColor );
                    Wallpaper( const BitmapEx& rBmpEx );
                    Wallpaper( const Gradient& rGradient );
                    ~Wallpaper();

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const;

    void            SetStyle( WallpaperStyle eStyle );
    WallpaperStyle  GetStyle() const;

    void            SetBitmap( const BitmapEx& rBitmap );
    void            SetBitmap();
    BitmapEx        GetBitmap() const;
    BOOL            IsBitmap() const;

    void            SetGradient( const Gradient& rGradient );
    void            SetGradient();
    Gradient        GetGradient() const;
    BOOL            IsGradient() const;

    void            SetRect( const Rectangle& rRect );
    void            SetRect();
    Rectangle       GetRect() const;
    BOOL            IsRect() const;

    BOOL            IsFixed() const;
    BOOL            IsScrollable() const;

    Wallpaper&      operator=( const Wallpaper& rWallpaper );
    BOOL            operator==( const Wallpaper& rWallpaper ) const;
    BOOL            operator!=( const Wallpaper& rWallpaper ) const
                        { return !(Wallpaper::operator==( rWallpaper )); }
    BOOL            IsSameInstance( const Wallpaper& rWallpaper ) const
                        { return (mpImplWallpaper == rWallpaper.mpImplWallpaper); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, Wallpaper& rWallpaper );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const Wallpaper& rWallpaper );
};

#endif  // _SV_WALL_HXX
