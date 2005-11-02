/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygoncutter.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:53:46 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

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
