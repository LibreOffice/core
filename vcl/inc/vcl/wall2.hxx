/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wall2.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_WALL2_HXX
#define _SV_WALL2_HXX

#include <vcl/wall.hxx>

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
