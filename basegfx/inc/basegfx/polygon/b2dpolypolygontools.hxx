/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#define _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
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
        B2DPolyPolygon correctOrientations(const B2DPolyPolygon& rCandidate);

        // make sure polygon with index 0L is not a hole. This may evtl. change the
        // sequence of polygons, but allows to use polygon with index 0L to
        // get the correct normal for the whole polyPolygon
        B2DPolyPolygon correctOutmostPolygon(const B2DPolyPolygon& rCandidate);

        // Subdivide all contained curves. Use distanceBound value if given.
        B2DPolyPolygon adaptiveSubdivideByDistance(const B2DPolyPolygon& rCandidate, double fDistanceBound = 0.0);

        // Subdivide all contained curves. Use distanceBound value if given. Else, a convenient one
        // is created.
        B2DPolyPolygon adaptiveSubdivideByAngle(const B2DPolyPolygon& rCandidate, double fAngleBound = 0.0);

        // Subdivide all contained curves. Use nCount divisions if given. Else, a convenient one
        // is created.
        B2DPolyPolygon adaptiveSubdivideByCount(const B2DPolyPolygon& rCandidate, sal_uInt32 nCount = 0L);

        // isInside test for B2dPoint. On border is not inside as long as not true is given
        // in bWithBorder flag. It is assumed that the orientations of the given polygon are correct.
        bool isInside(const B2DPolyPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder = false);

        /** get range of PolyPolygon. Control points are included.

            For detailed description look at getRangeWithControlPoints(const B2DPolygon&).
            This method just expands by the range of every sub-Polygon.

            @param rCandidate
            The B2DPolyPolygon eventually containing bezier segments

            @return
            The outer range including control points
        */
        B2DRange getRangeWithControlPoints(const B2DPolyPolygon& rCandidate);

        /** Get the range of a polyPolygon

            For detailed description look at getRange(const B2DPolygon&).
            This method just expands by the range of every sub-Polygon.

            @param rCandidate
            The B2DPolyPolygon eventually containing bezier segments

            @return
            The outer range of the polygon
        */
        B2DRange getRange(const B2DPolyPolygon& rCandidate);

        // get signed area of polygon
        double getSignedArea(const B2DPolyPolygon& rCandidate);

        // get area of polygon
        double getArea(const B2DPolyPolygon& rCandidate);

        /** Apply given LineDashing to given polyPolygon

            For a description see applyLineDashing in b2dpolygontoos.hxx
        */
        void applyLineDashing(
            const B2DPolyPolygon& rCandidate,
            const ::std::vector<double>& rDotDashArray,
            B2DPolyPolygon* pLineTarget,
            B2DPolyPolygon* pGapTarget = 0,
            double fFullDashDotLen = 0.0);

        // test if point is inside epsilon-range around the given PolyPolygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the tube around the PolyPolygon
        // with distance fDistance and rounded edges (start and end point).
        bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance);

        /** Read poly-polygon from SVG.

            This function imports a poly-polygon from an SVG-D
            attribute. Currently, elliptical arc elements are not yet
            supported (and ignored during parsing).

            @param o_rPolyPoly
            The output poly-polygon

            @param rSvgDAttribute
            A valid SVG-D attribute string

            @return true, if the string was successfully parsed
         */
        bool importFromSvgD( B2DPolyPolygon&        o_rPolyPoly,
                             const ::rtl::OUString& rSvgDAttribute );

        /** Read poly-polygon from SVG.

            This function imports a poly-polygon from an SVG points
            attribute (a plain list of coordinate pairs).

            @param o_rPoly
            The output polygon. Note that svg:points can only define a
            single polygon

            @param rSvgPointsAttribute
            A valid SVG points attribute string

            @return true, if the string was successfully parsed
         */
        bool importFromSvgPoints( B2DPolygon&            o_rPoly,
                                  const ::rtl::OUString& rSvgPointsAttribute );


        // grow for polyPolygon. Move all geometry in each point in the direction of the normal in that point
        // with the given amount. Value may be negative.
        B2DPolyPolygon growInNormalDirection(const B2DPolyPolygon& rCandidate, double fValue);

        // This method will correct a pair of polyPolygons where the goal is to keep same point count
        // to allow direct point association and also to remove self-intersections produced by shrinks.
        // This method will eventually change both polyPolygons to reach that goal because there are cases
        // where it is necessary to add new cut points to the original
        void correctGrowShrinkPolygonPair(B2DPolyPolygon& rOriginal, B2DPolyPolygon& rGrown);

        // force all sub-polygons to a point count of nSegments
        B2DPolyPolygon reSegmentPolyPolygon(const B2DPolyPolygon& rCandidate, sal_uInt32 nSegments);

        // create polygon state at t from 0.0 to 1.0 between the two polygons. Both polygons must have the same
        // organisation, e.g. same amount of polygons
        B2DPolyPolygon interpolate(const B2DPolyPolygon& rOld1, const B2DPolyPolygon& rOld2, double t);

        // create 3d PolyPolygon from given 2d PolyPolygon. The given fZCoordinate is used to expand the
        // third coordinate.
        B3DPolyPolygon createB3DPolyPolygonFromB2DPolyPolygon(const B2DPolyPolygon& rCandidate, double fZCoordinate = 0.0);

        // create 2d PolyPolygon from given 3d PolyPolygon. All coordinates are transformed using the given
        // matrix and the resulting x,y is used to form the new polygon.
        B2DPolyPolygon createB2DPolyPolygonFromB3DPolyPolygon(const B3DPolyPolygon& rCandidate, const B3DHomMatrix& rMat);

        // for each contained edge in each contained polygon calculate the smallest distance. Return the index to the smallest
        // edge in rEdgeIndex and the index to the polygon in rPolygonIndex. The relative position on the edge is returned in rCut.
        // If nothing was found (e.g. empty input plygon), DBL_MAX is returned.
        double getSmallestDistancePointToPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rPolygonIndex, sal_uInt32& rEdgeIndex, double& rCut);

        // distort PolyPolygon. rOriginal describes the original range, where the given points describe the distorted
        // corresponding points.
        B2DPolyPolygon distort(const B2DPolyPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight);

        // rotate PolyPolygon around given point with given angle.
        B2DPolyPolygon rotateAroundPoint(const B2DPolyPolygon& rCandidate, const B2DPoint& rCenter, double fAngle);

        // expand all segments (which are not yet) to curve segments. This is done with setting the control
        // vectors on the 1/3 resp. 2/3 distances on each segment.
        B2DPolyPolygon expandToCurve(const B2DPolyPolygon& rCandidate);

        // set continuity for the whole curve. If not a curve, nothing will change. Non-curve points are not changed, too.
        B2DPolyPolygon setContinuity(const B2DPolyPolygon& rCandidate, B2VectorContinuity eContinuity);

        /** Predicate whether a given poly-polygon is a rectangle.

            @param rPoly
            PolyPolygon to check

            @return true, if the poly-polygon describes a rectangle
            (contains exactly one polygon, polygon is closed, and the
            points are either cw or ccw enumerations of a rectangle's
            vertices). Note that intermediate points and duplicate
            points are ignored.
         */
        bool isRectangle( const B2DPolyPolygon& rPoly );

        /** Export poly-polygon to SVG.

            This function exports a poly-polygon into an SVG-D
            statement. Currently, output of relative point sequences
            is not yet supported (might cause slightly larger output)

            @param rPolyPoly
            The poly-polygon to export

            @param bUseRelativeCoordinates
            When true, all coordinate values are exported as relative
            to the current position. This tends to save some space,
            since fewer digits needs to be written.

            @param bDetectQuadraticBeziers
            When true, the export tries to detect cubic bezier
            segments in the input polygon, which can be represented by
            quadratic bezier segments. Note that the generated string
            causes versions prior to OOo2.0 to crash.

            @return the generated SVG-D statement (the XML d attribute
            value alone, without any "<path ...>" or "d="...")
         */
        ::rtl::OUString exportToSvgD( const B2DPolyPolygon& rPolyPoly,
                                      bool                  bUseRelativeCoordinates=true,
                                      bool                  bDetectQuadraticBeziers=true );

        // #i76891# Try to remove existing curve segments if they are simply edges
        B2DPolyPolygon simplifyCurveSegments(const B2DPolyPolygon& rCandidate);

        /** split each edge of a polyPolygon in exactly nSubEdges equidistant edges

            @param rCandidate
            The source polyPolygon. If too small (no edges), nSubEdges too small (<2)
            or neither bHandleCurvedEdgesnor bHandleStraightEdges it will just be returned.
            Else for each edge nSubEdges will be created. Closed state is preserved.

            @param nSubEdges
            @param bHandleCurvedEdges
            @param bHandleStraightEdges
            Please take a look at reSegmentPolygonEdges description, these are the same.
        */
        B2DPolyPolygon reSegmentPolyPolygonEdges(const B2DPolyPolygon& rCandidate, sal_uInt32 nSubEdges, bool bHandleCurvedEdges, bool bHandleStraightEdges);

        //////////////////////////////////////////////////////////////////////
        // comparators with tolerance for 2D PolyPolygons
        bool equal(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB, const double& rfSmallValue);
        bool equal(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        /** snap some polygon coordinates to discrete coordinates

            This method allows to snap some polygon points to discrete (integer) values
            which equals e.g. a snap to discrete coordinates. It will snap points of
            horizontal and vertical edges

            @param rCandidate
            The source polygon

            @return
            The modified version of the source polygon
        */
        B2DPolyPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolyPolygon& rCandidate);

        /** returns true if the Polygon only contains horizontal or vertical edges
            so that it could be represented by RegionBands
        */
        bool containsOnlyHorizontalAndVerticalEdges(const B2DPolyPolygon& rCandidate);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX */
