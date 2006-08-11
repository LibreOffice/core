/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rectcliptools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-11 17:40:45 $
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

#ifndef _BGFX_TOOLS_RECTCLIPTOOLS_HXX
#define _BGFX_TOOLS_RECTCLIPTOOLS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
        sal_uInt32 getNumberOfClipPlanes( sal_uInt32 nFlags )
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
