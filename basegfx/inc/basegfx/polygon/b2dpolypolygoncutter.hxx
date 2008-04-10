/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dpolypolygoncutter.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX

//#ifndef _SAL_TYPES_H_
//#include <sal/types.h>
//#endif
//
//#ifndef _BGFX_POINT_B2DPOINT_HXX
//#include <basegfx/point/b2dpoint.hxx>
//#endif
//
//#ifndef _BGFX_RANGE_B2DRANGE_HXX
//#include <basegfx/range/b2drange.hxx>
//#endif

#include <basegfx/polygon/b2dpolypolygon.hxx>

//#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // solve all crossovers in a polyPolygon. This re-layouts all contained polygons so that the
        // result will contain only non-cutting polygons. For that reason, points will be added at
        // crossover and touch points and the single Polygons may be re-combined. The orientations
        // of the contained polygons in not changed but used as hints.
        // If bSelfCrossovers is set, first all self-intersections of the contained polygons will be
        // solved.
        B2DPolyPolygon SolveCrossovers(const B2DPolyPolygon& rCandidate, bool bSelfCrossovers = true);

        // version for single polygons. This is for solving self-intersections. Result will be free of
        // crossovers. When result contains multiple polygons, it may be necessary to rearrange their
        // orientations since holes may have been created (use correctOrientations eventually).
        B2DPolyPolygon SolveCrossovers(const B2DPolygon& rCandidate);

        // Neutral polygons will be stripped. Neutral polygons are ones who's orientation is
        // neutral, so normally they have no volume -> just closed paths. A polygon with the same
        // positive and negative oriented volume is also neutral, so this may not be wanted. It is
        // safe to call with crossover-free polygons, though (that's where it's mostly used).
        B2DPolyPolygon StripNeutralPolygons(const B2DPolyPolygon& rCandidate);

        // Remove not necessary polygons. Works only correct with crossover-free polygons. For each
        // polygon, the depth for the PolyPolygon is calculated. The orientation is used to identify holes.
        // Start value for holes is -1, for polygons it's zero. Ech time a polygon is contained in another one,
        // it's depth is increased when inside a polygon, decreased when inside a hole. The result is a depth
        // which e.g. is -1 for holes outside everything, 1 for a polygon covered by another polygon and zero
        // for e.g. holes in a polygon or polygons outside everythig else.
        // In the 2nd step, all polygons with depth other than zero are removed. If bKeepAboveZero is used,
        // all polygons < 1 are removed. The bKeepAboveZero mode is useful for clipping, e.g. just append
        // one polygon to another and use this mode -> only parts where two polygons overlapped will be kept.
        // In combination with correct orientation of the input orientations and the SolveCrossover calls this
        // can be combined for logical polygon operations or polygon clipping.
        B2DPolyPolygon StripDispensablePolygons(const B2DPolyPolygon& rCandidate, bool bKeepAboveZero = false);
    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif /* _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX */
