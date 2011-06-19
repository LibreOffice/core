/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _BGFX_TOOLS_RECTCLIPTOOLS_HXX
#define _BGFX_TOOLS_RECTCLIPTOOLS_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        namespace RectClipFlags
        {
            static const sal_uInt32 LEFT   = (sal_Int32)0x01;
            static const sal_uInt32 RIGHT  = (sal_Int32)0x02;
            static const sal_uInt32 TOP    = (sal_Int32)0x04;
            static const sal_uInt32 BOTTOM = (sal_Int32)0x08;
        }

        /** Calc clip mask for Cohen-Sutherland rectangle clip

            This function returns a clip mask used for the
            Cohen-Sutherland rectangle clip method, where one or more
            of the lower four bits are set, if the given point is
            outside one or more of the four half planes defining the
            rectangle (see RectClipFlags for possible values)
         */
        template< class Point, class Rect > inline
           sal_uInt32 getCohenSutherlandClipFlags( const Point& rP,
                                                   const Rect&  rR )
        {
            // maxY | minY | maxX | minX
            sal_uInt32 clip  = (rP.getX() < rR.getMinX()) << 0;
                       clip |= (rP.getX() > rR.getMaxX()) << 1;
                       clip |= (rP.getY() < rR.getMinY()) << 2;
                       clip |= (rP.getY() > rR.getMaxY()) << 3;
            return clip;
        }

        /** Determine number of clip planes hit by given clip mask

            This method returns the number of one bits in the four
            least significant bits of the argument, which amounts to
            the number of clip planes hit within the
            getCohenSutherlandClipFlags() method.
         */
        inline sal_uInt32 getNumberOfClipPlanes( sal_uInt32 nFlags )
        {
            // classic bit count algo, see e.g. Reingold, Nievergelt,
            // Deo: Combinatorial Algorithms, Theory and Practice,
            // Prentice-Hall 1977
            nFlags = (nFlags & 0x05) + ((nFlags >> 1) & 0x05);
            nFlags = (nFlags & 0x03) + (nFlags >> 2); // no need for &
                                                      // 0x03, can't
                                                      // overflow
            return nFlags;
        }
    }
}

#endif // _BGFX_TOOLS_RECTCLIPTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
