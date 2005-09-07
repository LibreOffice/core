/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygontools.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:30:02 $
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

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#define _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#include <vector>

namespace rtl
{
    class OUString;
}

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B2DPolyPolygon;
    class B2DRange;

    namespace tools
    {
        // B2DPolyPolygon tools

        // Check and evtl. correct orientations of all contained Polygons so that
        // the orientations of contained polygons will variate to express areas and
        // holes
//BFS08     void correctOrientations(::basegfx::B2DPolyPolygon& rCandidate);
        ::basegfx::B2DPolyPolygon correctOrientations(const ::basegfx::B2DPolyPolygon& rCandidate);

        // Remove all intersections, the self intersections and the in-between
        // polygon intersections. After this operation there are no more intersections
        // in the given PolyPolygon. Only closed polygons are handled. The non-closed
        // polygons or the ones with less than 3 points are preserved, but not
        // computed.
        // bForceOrientation: If true, the orientations of all contained polygons
        // is changed to ORIENTATION_POSITIVE before computing.
//BFS08     // bInvertRemove: if true, created polygons which are inside others and
//BFS08     // have the same orientation are removed (cleanup).
//BFS08     void removeIntersections(::basegfx::B2DPolyPolygon& rCandidate,
//BFS08         bool bForceOrientation = true, bool bInvertRemove = false);
//BFS08     void removeIntersections(::basegfx::B2DPolyPolygon& rCandidate, bool bForceOrientation = true);
//BFS08     void removeIntersections(::basegfx::B2DPolyPolygon& rCandidate);
        ::basegfx::B2DPolyPolygon removeIntersections(const ::basegfx::B2DPolyPolygon& rCandidate);
        ::basegfx::B2DPolyPolygon removeAllIntersections(const ::basegfx::B2DPolyPolygon& rCandidate);
        ::basegfx::B2DPolyPolygon removeNeutralPolygons(const ::basegfx::B2DPolyPolygon& rCandidate, bool bUseOr);

        // Subdivide all contained curves. Use distanceBound value if given.
        ::basegfx::B2DPolyPolygon adaptiveSubdivideByDistance(const ::basegfx::B2DPolyPolygon& rCandidate, double fDistanceBound = 0.0);

        // Subdivide all contained curves. Use distanceBound value if given. Else, a convenient one
        // is created.
        ::basegfx::B2DPolyPolygon adaptiveSubdivideByAngle(const ::basegfx::B2DPolyPolygon& rCandidate, double fAngleBound = 0.0);

        // Subdivide all contained curves. Use nCount divisions if given. Else, a convenient one
        // is created.
        ::basegfx::B2DPolyPolygon adaptiveSubdivideByCount(const ::basegfx::B2DPolyPolygon& rCandidate, sal_uInt32 nCount = 0L);

        // get size of PolyPolygon. Control vectors are included in that ranges.
        ::basegfx::B2DRange getRange(const ::basegfx::B2DPolyPolygon& rCandidate);

        // Apply Line Dashing. This cuts every contained PolyPolygon into line pieces
        // which are inserted as single polygons into the result.
        // If fFullDashDotLen is not given it will be calculated from the given
        // raDashDotArray.
        ::basegfx::B2DPolyPolygon applyLineDashing(const ::basegfx::B2DPolyPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen = 0.0);

        // Merge contained Polygons to longer ones if the end point of one Polygon equals
        // the start point of the next one. Only direct successors are checked.
        // This method is mainly for joining line snippets which reach over an original
        // polygon edge after using applyLineDashing. This is necessary if the dashed line
        // parts shall be used for creating thick line geometry to be able to do correct
        // line joints.
        ::basegfx::B2DPolyPolygon mergeDashedLines(const ::basegfx::B2DPolyPolygon& rCandidate);

        // test if point is inside epsilon-range around the given PolyPolygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the tube around the PolyPolygon
        // with distance fDistance and rounded edges (start and end point).
        bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance);

        /** Read poly-polygon from SVG.

            This function imports a poly-polygon from an SVG-D
            statement. Currently, elliptical arc elements are not yet
            supported (and ignored during parsing).

            @param o_rPolyPoly
            The output poly-polygon

            @param rSvgDStatement
            A valid SVG-D statement

            @return true, if the string was successfully parsed
         */
        bool importFromSvgD( ::basegfx::B2DPolyPolygon& o_rPolyPoly,
                             const ::rtl::OUString&     rSvgDStatement );

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX */
