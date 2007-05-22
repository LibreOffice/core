/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Clipping.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:18:10 $
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

#ifndef _CHART2_CLIPPING_HXX
#define _CHART2_CLIPPING_HXX

#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class Clipping
{
    /** This class uses the Liang-Biarsky parametric line-clipping algorithm as described in:
            Computer Graphics: principles and practice, 2nd ed.,
            James D. Foley et al.,
            Section 3.12.4 on page 117.
    */

public:
    /** @descr  The intersection between an open polygon and a rectangle is
            calculated and the resulting lines are placed into the poly-polygon aResult.
        @param  rPolygon    The polygon is required to be open, ie. it's start and end point
            have different coordinates and that it is continuous, ie. has no holes.
        @param  rRectangle  The clipping area.
        @param  aResult The resulting lines that are the parts of the given polygon lying inside
            the clipping area are stored into aResult whose prior content is deleted first.
     */
    static void clipPolygonAtRectangle(
                                const ::com::sun::star::drawing::PolyPolygonShape3D& rPolygon
                                , const ::basegfx::B2DRectangle& rRectangle
                                , ::com::sun::star::drawing::PolyPolygonShape3D& aResult
                                , bool bSplitPiecesToDifferentPolygons = true );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
