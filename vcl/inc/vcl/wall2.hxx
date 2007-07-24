/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wall2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:02:51 $
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

#ifndef _SV_WALL2_HXX
#define _SV_WALL2_HXX

#ifndef _SV_WALL_HXX
#include <vcl/wall.hxx>
#endif

class ImplWallpaper
{
    friend class Wallpaper;

private:
    Color           maColor;
    BitmapEx*       mpBitmap;
    Gradient*       mpGradient;
    Rectangle*      mpRect;
    WallpaperStyle  meStyle;
    ULONG           mnRefCount;
    BitmapEx*       mpCache;

    friend SvStream& operator>>( SvStream& rIStm, ImplWallpaper& rImplWallpaper );
    friend SvStream& operator<<( SvStream& rOStm, const ImplWallpaper& rImplWallpaper );

public:
                    ImplWallpaper();
                    ImplWallpaper( const ImplWallpaper& rImplWallpaper );
                    ~ImplWallpaper();

    void            ImplSetCachedBitmap( BitmapEx& rBmp );
    const BitmapEx* ImplGetCachedBitmap() { return mpCache; }
    void            ImplReleaseCachedBitmap();
};


#endif  // _SV_WALL2_HXX
