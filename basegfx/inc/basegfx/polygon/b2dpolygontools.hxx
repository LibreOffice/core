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

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#define _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B2DPolygon;
    class B2DRange;

    namespace tools
    {
        // B2DPolygon tools

        // open/close with point add/remove and control point corrections
        void openWithGeometryChange(B2DPolygon& rCandidate);
        void closeWithGeometryChange(B2DPolygon& rCandidate);

        /** Check if given polygon is closed.

            This is kind of a 'classic' method to support old polygon
            definitions.  Those old polygon definitions define the
            closed state of the polygon using identical start and
            endpoints. This method corrects this (removes double
            start/end points) and sets the Closed()-state of the
            polygon correctly.
        */
        void checkClosed(B2DPolygon& rCandidate);

        // Get successor and predecessor indices. Returning the same index means there
        // is none. Same for successor.
        sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate);
        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate);

        // Get orientation of Polygon
        B2VectorOrientation getOrientation(const B2DPolygon& rCandidate);

        // isInside tests for B2dPoint and other B2dPolygon. On border is not inside as long as
        // not true is given in bWithBorder flag.
        bool isInside(const B2DPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder = false);
        bool isInside(const B2DPolygon& rCandidate, const B2DPolygon& rPolygon, bool bWithBorder = false);

        /** Get the range of a polygon including bezier control points

            For detailed discussion, see B2DPolygon::getB2DRange()

            @param rCandidate
            The B2DPolygon eventually containing bezier segments

            @return
            The outer range of the bezier curve containing bezier control points
        */
        B2DRange getRangeWithControlPoints(const B2DPolygon& rCandidate);

        /** Get the range of a polygon

            This method creates the outer range of the subdivided bezier curve.
            For detailed discussion see B2DPolygon::getB2DRange()

            @param rCandidate
            The B2DPolygon eventually containing bezier segments

            @return
            The outer range of the bezier curve
        */
        B2DRange getRange(const B2DPolygon& rCandidate);

        // get signed area of polygon
        double getSignedArea(const B2DPolygon& rCandidate);

        // get area of polygon
        double getArea(const B2DPolygon& rCandidate);

        /** get length of polygon edge from point nIndex to nIndex + 1 */
        double getEdgeLength(const B2DPolygon& rCandidate, sal_uInt32 nIndex);

        /** get length of polygon */
        double getLength(const B2DPolygon& rCandidate);

        // get position on polygon for absolute given distance. If
        // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
        // using getLength(...)
        B2DPoint getPositionAbsolute(const B2DPolygon& rCandidate, double fDistance, double fLength = 0.0);

        // get position on polygon for relative given distance in range [0.0 .. 1.0]. If
        // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
        // using getLength(...)
        B2DPoint getPositionRelative(const B2DPolygon& rCandidate, double fDistance, double fLength = 0.0);

        // get a snippet from given polygon for absolute distances. The polygon is assumed
        // to be opened (not closed). fFrom and fTo need to be in range [0.0 .. fLength], where
        // fTo >= fFrom. If length is given, it is assumed the correct polygon length,
        // if 0.0 it is calculated using getLength(...)
        B2DPolygon getSnippetAbsolute(const B2DPolygon& rCandidate, double fFrom, double fTo, double fLength = 0.0);

        // get a snippet from given polygon for relative distances. The polygon is assumed
        // to be opened (not closed). fFrom and fTo need to be in range [0.0 .. 1.0], where
        // fTo >= fFrom. If length is given, it is assumed the correct polygon length,
        // if 0.0 it is calculated using getLength(...)
        B2DPolygon getSnippetRelative(const B2DPolygon& rCandidate, double fFrom = 0.0, double fTo = 1.0, double fLength = 0.0);

        // Continuity check for point with given index
        B2VectorContinuity getContinuityInPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndex);

        // Subdivide all contained curves. Use distanceBound value if given.
        B2DPolygon adaptiveSubdivideByDistance(const B2DPolygon& rCandidate, double fDistanceBound = 0.0);

        // Subdivide all contained curves. Use angleBound value if given.
        B2DPolygon adaptiveSubdivideByAngle(const B2DPolygon& rCandidate, double fAngleBound = 0.0);

        // #i37443# Subdivide all contained curves.
        B2DPolygon adaptiveSubdivideByCount(const B2DPolygon& rCandidate, sal_uInt32 nCount = 0L);

        // Definitions for the cut flags used from the findCut methods
        typedef sal_uInt16 CutFlagValue;

        #define CUTFLAG_NONE            (0x0000)
        #define CUTFLAG_LINE            (0x0001)
        #define CUTFLAG_START1          (0x0002)
        #define CUTFLAG_START2          (0x0004)
        #define CUTFLAG_END1            (0x0008)
        #define CUTFLAG_END2            (0x0010)
        #define CUTFLAG_ALL         (CUTFLAG_LINE|CUTFLAG_START1|CUTFLAG_START2|CUTFLAG_END1|CUTFLAG_END2)
        #define CUTFLAG_DEFAULT     (CUTFLAG_LINE|CUTFLAG_START2|CUTFLAG_END2)

        // Calculate cut between the points given by the two indices. pCut1
        // and pCut2 will contain the cut coordinate on each edge in ]0.0, 1.0]
        // (if given) and the return value will contain a cut description.
        CutFlagValue findCut(
            const B2DPolygon& rCandidate,
            sal_uInt32 nIndex1, sal_uInt32 nIndex2,
            CutFlagValue aCutFlags = CUTFLAG_DEFAULT,
            double* pCut1 = 0L, double* pCut2 = 0L);

        // This version is working with two indexed edges from different
        // polygons.
        CutFlagValue findCut(
            const B2DPolygon& rCandidate1, sal_uInt32 nIndex1,
            const B2DPolygon& rCandidate2, sal_uInt32 nIndex2,
            CutFlagValue aCutFlags = CUTFLAG_DEFAULT,
            double* pCut1 = 0L, double* pCut2 = 0L);

        // This version works with two points and vectors to define the
        // edges for the cut test.
        CutFlagValue findCut(
            const B2DPoint& rEdge1Start, const B2DVector& rEdge1Delta,
            const B2DPoint& rEdge2Start, const B2DVector& rEdge2Delta,
            CutFlagValue aCutFlags = CUTFLAG_DEFAULT,
            double* pCut1 = 0L, double* pCut2 = 0L);

        // test if point is on the given edge in range ]0.0..1.0[ without
        // the start/end points. If so, return true and put the parameter
        // value in pCut (if provided)
        bool isPointOnEdge(
            const B2DPoint& rPoint,
            const B2DPoint& rEdgeStart,
            const B2DVector& rEdgeDelta,
            double* pCut = 0L);

        /** Apply given LineDashing to given polygon

            This method is used to cut down line polygons to the needed
            pieces when a dashing needs to be applied.
            It is now capable of keeping contained bezier segments.
            It is also capable of delivering line and non-line portions
            depending on what target polygons You provide. This is useful
            e.g. for dashed lines with two colors.
            If the last and the first snippet in one of the results have
            a common start/end ppoint, they will be merged to achieve as
            view as needed result line snippets. This is also relevant for
            further processing the results.

            @param rCandidate
            The polygon based on which the snippets will be created.

            @param rDotDashArray
            The line pattern given as array of length values

            @param pLineTarget
            The target for line snippets, e.g. the first entry will be
            a line segment with length rDotDashArray[0]. The given
            polygon will be emptied as preparation.

            @param pGapTarget
            The target for gap snippets, e.g. the first entry will be
            a line segment with length rDotDashArray[1]. The given
            polygon will be emptied as preparation.

            @param fFullDashDotLen
            The sumed-up length of the rDotDashArray. If zero, it will
            be calculated internally.
        */
        void applyLineDashing(
            const B2DPolygon& rCandidate,
            const ::std::vector<double>& rDotDashArray,
            B2DPolyPolygon* pLineTarget,
            B2DPolyPolygon* pGapTarget = 0,
            double fFullDashDotLen = 0.0);

        // test if point is inside epsilon-range around an edge defined
        // by the two given points. Can be used for HitTesting. The epsilon-range
        // is defined to be the rectangle centered to the given edge, using height
        // 2 x fDistance, and the circle around both points with radius fDistance.
        bool isInEpsilonRange(const B2DPoint& rEdgeStart, const B2DPoint& rEdgeEnd, const B2DPoint& rTestPosition, double fDistance);

        // test if point is inside epsilon-range around the given Polygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the rectangle centered
        // to the given edge, using height 2 x fDistance, and the circle around both points
        // with radius fDistance.
        bool isInEpsilonRange(const B2DPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance);

        /** Create a polygon from a rectangle.

            @param rRect
            The rectangle which describes the polygon size

            @param fRadius
            Radius of the edge rounding, relative to the rectangle size. 0.0 means no
            rounding, 1.0 will lead to an ellipse
         */
        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect, double fRadius );

        /** Create a polygon from a rectangle.

            @param rRect
            The rectangle which describes the polygon size

            @param fRadiusX
            @param fRadiusY
            Radius of the edge rounding, relative to the rectangle size. 0.0 means no
            rounding, 1.0 will lead to an ellipse
         */
        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect, double fRadiusX, double fRadiusY );

        /** Create a polygon from a rectangle.
         */
        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect );

        /** Create the unit polygon
         */
        B2DPolygon createUnitPolygon();

        /** Create a circle polygon with given radius.

            This method creates a circle approximation consisting of
            four cubic bezier segments, which approximate the given
            circle with an error of less than 0.5 percent.

            @param rCenter
            Center point of the circle

            @param fRadius
            Radius of the circle
         */
        B2DPolygon createPolygonFromCircle( const B2DPoint& rCenter, double fRadius );

        /** create a polygon which describes the unit circle and close it

            @param nStartQuadrant
            To be able to rebuild the old behaviour where the circles started at bottom,
            this parameter is used. Default is 0 which is the first quadrant and the
            polygon's start point will be the rightmost one. When using e.g. 1, the
            first created quadrant will start at the YMax-position (with Y down on screens,
            this is the lowest one). This is needed since when lines are dashed, toe old
            geometry started at bottom point, else it would look different.
         */
        B2DPolygon createPolygonFromUnitCircle(sal_uInt32 nStartQuadrant = 0);

        /** Create an ellipse polygon with given radii.

            This method creates an ellipse approximation consisting of
            four cubic bezier segments, which approximate the given
            ellipse with an error of less than 0.5 percent.

            @param rCenter
            Center point of the circle

            @param fRadiusX
            Radius of the ellipse in X direction

            @param fRadiusY
            Radius of the ellipse in Y direction
         */
        B2DPolygon createPolygonFromEllipse( const B2DPoint& rCenter, double fRadiusX, double fRadiusY );

        /** Create an unit ellipse polygon with the given angles, from start to end
         */
        B2DPolygon createPolygonFromEllipseSegment( const B2DPoint& rCenter, double fRadiusX, double fRadiusY, double   fStart, double fEnd );

        B2DPolygon createPolygonFromUnitEllipseSegment( double fStart, double fEnd );

        /** Predicate whether a given polygon is a rectangle.

            @param rPoly
            Polygon to check

            @return true, if the polygon describes a rectangle
            (polygon is closed, and the points are either cw or ccw
            enumerations of a rectangle's vertices). Note that
            intermediate points and duplicate points are ignored.
         */
        bool isRectangle( const B2DPolygon& rPoly );

        // create 3d polygon from given 2d polygon. The given fZCoordinate is used to expand the
        // third coordinate.
        B3DPolygon createB3DPolygonFromB2DPolygon(const B2DPolygon& rCandidate, double fZCoordinate = 0.0);

        // create 2d PolyPolygon from given 3d PolyPolygon. All coordinates are transformed using the given
        // matrix and the resulting x,y is used to form the new polygon.
        B2DPolygon createB2DPolygonFromB3DPolygon(const B3DPolygon& rCandidate, const B3DHomMatrix& rMat);

        // create simplified version of the original polygon by
        // replacing segments with spikes/loops and self intersections
        // by several trivial sub-segments
        B2DPolygon createSimplifiedPolygon(const B2DPolygon&);

        // calculate the distance to the given endless ray and return. The relative position on the edge is returned in Cut.
        // That position may be less than 0.0 or more than 1.0
        double getDistancePointToEndlessRay(const B2DPoint& rPointA, const B2DPoint& rPointB, const B2DPoint& rTestPoint, double& rCut);

        // calculate the smallest distance to given edge and return. The relative position on the edge is returned in Cut.
        // That position is in the range [0.0 .. 1.0] and the returned distance is adapted accordingly to the start or end
        // point of the edge
        double getSmallestDistancePointToEdge(const B2DPoint& rPointA, const B2DPoint& rPointB, const B2DPoint& rTestPoint, double& rCut);

        // for each contained edge calculate the smallest distance. Return the index to the smallest
        // edge in rEdgeIndex. The relative position on the edge is returned in rCut.
        // If nothing was found (e.g. empty input plygon), DBL_MAX is returned.
        double getSmallestDistancePointToPolygon(const B2DPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rEdgeIndex, double& rCut);

        // distort single point. rOriginal describes the original range, where the given points describe the distorted corresponding points.
        B2DPoint distort(const B2DPoint& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight);

        // distort polygon. rOriginal describes the original range, where the given points describe the distorted corresponding points.
        B2DPolygon distort(const B2DPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight);

        // rotate polygon around given point with given angle.
        B2DPolygon rotateAroundPoint(const B2DPolygon& rCandidate, const B2DPoint& rCenter, double fAngle);

        // expand all segments (which are not yet) to curve segments. This is done with setting the control
        // vectors on the 1/3 resp. 2/3 distances on each segment.
        B2DPolygon expandToCurve(const B2DPolygon& rCandidate);

        // expand given segment to curve segment. This is done with setting the control
        // vectors on the 1/3 resp. 2/3 distances. The return value describes if a change took place.
        bool expandToCurveInPoint(B2DPolygon& rCandidate, sal_uInt32 nIndex);

        // set continuity for the whole curve. If not a curve, nothing will change. Non-curve points are not changed, too.
        B2DPolygon setContinuity(const B2DPolygon& rCandidate, B2VectorContinuity eContinuity);

        // set continuity for given index. If not a curve, nothing will change. Non-curve points are not changed, too.
        // The return value describes if a change took place.
        bool setContinuityInPoint(B2DPolygon& rCandidate, sal_uInt32 nIndex, B2VectorContinuity eContinuity);

        // test if polygon contains neutral points. A neutral point is one whos orientation is neutral
        // e.g. positioned on the edge of it's predecessor and successor
        bool hasNeutralPoints(const B2DPolygon& rCandidate);

        // remove neutral points. A neutral point is one whos orientation is neutral
        // e.g. positioned on the edge of it's predecessor and successor
        B2DPolygon removeNeutralPoints(const B2DPolygon& rCandidate);

        // tests if polygon is convex
        bool isConvex(const B2DPolygon& rCandidate);

        // calculates the orientation at edge nIndex
        B2VectorOrientation getOrientationForIndex(const B2DPolygon& rCandidate, sal_uInt32 nIndex);

        // calculates if given point is on given line, taking care of the numerical epsilon
        bool isPointOnLine(const B2DPoint& rStart, const B2DPoint& rEnd, const B2DPoint& rCandidate, bool bWithPoints = false);

        // calculates if given point is on given polygon, taking care of the numerical epsilon. Uses
        // isPointOnLine internally
        bool isPointOnPolygon(const B2DPolygon& rCandidate, const B2DPoint& rPoint, bool bWithPoints = true);

        // test if candidate is inside triangle
        bool isPointInTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC, const B2DPoint& rCandidate, bool bWithBorder = false);

        // test if candidateA and candidateB are on the same side of the given line
        bool arePointsOnSameSideOfLine(const B2DPoint& rStart, const B2DPoint& rEnd, const B2DPoint& rCandidateA, const B2DPoint& rCandidateB, bool bWithLine = false);

        // add triangles for given rCandidate to rTarget. For each triangle, 3 points will be added to rCandidate.
        // All triangles will go from the start point of rCandidate to two consecutive points, building (rCandidate.count() - 2)
        // triangles.
        void addTriangleFan(const B2DPolygon& rCandidate, B2DPolygon& rTarget);

        // grow for polygon. Move all geometry in each point in the direction of the normal in that point
        // with the given amount. Value may be negative.
        B2DPolygon growInNormalDirection(const B2DPolygon& rCandidate, double fValue);

        // force all sub-polygons to a point count of nSegments
        B2DPolygon reSegmentPolygon(const B2DPolygon& rCandidate, sal_uInt32 nSegments);

        // create polygon state at t from 0.0 to 1.0 between the two polygons. Both polygons must have the same
        // organisation, e.g. same amount of points
        B2DPolygon interpolate(const B2DPolygon& rOld1, const B2DPolygon& rOld2, double t);

        bool isPolyPolygonEqualRectangle( const B2DPolyPolygon& rPolyPoly, const B2DRange& rRect );

        // #i76891# Try to remove existing curve segments if they are simply edges
        B2DPolygon simplifyCurveSegments(const B2DPolygon& rCandidate);

        // makes the given indexed point the new polygon start point. To do that, the points in the
        // polygon will be rotated. This is only valid for closed polygons, for non-closed ones
        // an assertion will be triggered
        B2DPolygon makeStartPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndexOfNewStatPoint);

        /** create edges of given length along given B2DPolygon

            @param rCandidate
            The polygon to move along. Points at the given polygon are created, starting
            at position fStart and stopping at less or equal to fEnd. The closed state is
            preserved.
            The polygon is subdivided if curve segments are included. That subdivision is the base
            for the newly created points.
            If the source is closed, the indirectly existing last edge may NOT have the
            given length.
            If the source is open, all edges will have the given length. You may use the last
            point of the original when You want to add the last edge Yourself.

            @param fLength
            The length of the created edges. If less or equal zero, an empty polygon is returned.

            @param fStart
            The start distance for the first to be generated point. Use 0.0 to get the
            original start point. Negative values are truncated to 0.0.

            @param fEnd
            The maximum distance for the last point. No more points behind this distance will be created.
            Use 0.0 to proccess the whole polygon. Negative values are truncated to 0.0. It also
            needs to be more or equal to fStart, else it is truncated to fStart.

            @return
            The newly created polygon
         */
        B2DPolygon createEdgesOfGivenLength(const B2DPolygon& rCandidate, double fLength, double fStart = 0.0, double fEnd = 0.0);

        /** Create Waveline along given polygon
            The implementation is based on createEdgesOfGivenLength and creates a curve
            segment with the given dimensions for each created line segment. The polygon
            is treated as if opened (closed state will be ignored) and only for whole
            edges a curve segment will be created (no rest handling)

            @param rCandidate
            The polygon along which the waveline will be created

            @param fWaveWidth
            The length of a single waveline curve segment

            @param fgWaveHeight
            The height of the waveline (amplitude)
        */
        B2DPolygon createWaveline(const B2DPolygon& rCandidate, double fWaveWidth, double fWaveHeight);

        /** split each edge of a polygon in exactly nSubEdges equidistant edges

            @param rCandidate
            The source polygon. If too small (no edges), nSubEdges too small (<2)
            or neither bHandleCurvedEdgesnor bHandleStraightEdges it will just be returned.
            Else for each edge nSubEdges will be created. Closed state is preserved.

            @param nSubEdges
            How many edges shall be created as replacement for each single edge

            @param bHandleCurvedEdges
            Process curved edges or not. If to handle the curved edges will be splitted
            into nSubEdges part curved edges of equidistant bezier distances. If not,
            curved edges will just be copied.

            @param bHandleStraightEdges
            Process straight edges or not. If to handle the straight edges will be splitted
            into nSubEdges part curved edges of equidistant length. If not,
            straight edges will just be copied.
        */
        B2DPolygon reSegmentPolygonEdges(const B2DPolygon& rCandidate, sal_uInt32 nSubEdges, bool bHandleCurvedEdges, bool bHandleStraightEdges);

        //////////////////////////////////////////////////////////////////////
        // comparators with tolerance for 2D Polygons
        bool equal(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, const double& rfSmallValue);
        bool equal(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB);

        /** snap some polygon coordinates to discrete coordinates

            This method allows to snap some polygon points to discrete (integer) values
            which equals e.g. a snap to discrete coordinates. It will snap points of
            horizontal and vertical edges

            @param rCandidate
            The source polygon

            @return
            The modified version of the source polygon
        */
        B2DPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolygon& rCandidate);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
