/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygontools.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:29:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        void correctOrientations(::basegfx::B2DPolyPolygon& rCandidate);

        // Remove all intersections, the self intersections and the in-between
        // polygon intersections. After this operation there are no more intersections
        // in the given PolyPolygon. Only closed polygons are handled. The non-closed
        // polygons or the ones with less than 3 points are preserved, but not
        // computed.
        // bForceOrientation: If true, the orientations of all contained polygons
        // is changed to ORIENTATION_POSITIVE before computing.
        // bInvertRemove: if true, created polygons which are inside others and
        // have the same orientation are removed (cleanup).
        void removeIntersections(::basegfx::B2DPolyPolygon& rCandidate,
            bool bForceOrientation = true, bool bInvertRemove = false);

        // Subdivide all contained curves. Use distanceBound value if given.
        ::basegfx::B2DPolyPolygon adaptiveSubdivideByDistance(const ::basegfx::B2DPolyPolygon& rCandidate, double fDistanceBound = 0.0);

        // Subdivide all contained curves. Use distanceBound value if given.
        ::basegfx::B2DPolyPolygon adaptiveSubdivideByAngle(const ::basegfx::B2DPolyPolygon& rCandidate, double fAngleBound = 5.0);

        // get size of PolyPolygon. Control vectors are included in that ranges.
        ::basegfx::B2DRange getRange(const ::basegfx::B2DPolyPolygon& rCandidate);

        // Apply Line Dashing. This cuts every contained PolyPolygon into line pieces
        // which are inserted as single polygons into the result.
        // If fFullDashDotLen is not given it will be calculated from the given
        // raDashDotArray.
        ::basegfx::B2DPolyPolygon applyLineDashing(const ::basegfx::B2DPolyPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen = 0.0);

        // test if point is inside epsilon-range around the given PolyPolygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the tube around the PolyPolygon
        // with distance fDistance and rounded edges (start and end point).
        bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX */
