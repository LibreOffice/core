/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygoncutandtouch.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:53:25 $
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

#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#define _BGFX_POLYGON_CUTANDTOUCH_HXX

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // look for self-intersections and self-touches (points on an edge) in given polygon and add
        // extra points there. Result will have no touches or intersections on an edge, only on points
        B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate);

        // look for polypolygon-intersections and polypolygon-touches (point of poly A on an edge of poly B) in given PolyPolygon and add
        // extra points there. Result will have no touches or intersections between contained polygons on an edge, only on points. For
        // convenience, the correction for self-intersections for each member polygon will be used, too.
        // Changed: Self intersections are searched by default, but may be switched off by 2nd parameter.
        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate, bool bSelfIntersections = true);

        // look for intersections of rCandidate with all polygons from rMask and add extra points there. Do
        // not change or add points to rMask.
        B2DPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolygon& rCandidate);

        // look for intersections of rCandidate with all polygons from rMask and add extra points there. Do
        // not change or add points to rMask.
        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolyPolygon& rCandidate);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_CUTANDTOUCH_HXX */
