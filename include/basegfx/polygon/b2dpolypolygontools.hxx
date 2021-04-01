/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <vector>
#include <basegfx/basegfxdllapi.h>
#include <o3tl/sorted_vector.hxx>

namespace com::sun::star::drawing { struct PolyPolygonBezierCoords; }

namespace basegfx
{
    class B2DPolyPolygon;
    class B2DRange;
}

namespace basegfx::utils
{
        // B2DPolyPolygon tools

        // Check and evtl. correct orientations of all contained Polygons so that
        // the orientations of contained polygons will variate to express areas and
        // holes
        BASEGFX_DLLPUBLIC B2DPolyPolygon correctOrientations(const B2DPolyPolygon& rCandidate);

        // make sure polygon with index 0L is not a hole. This may evtl. change the
        // sequence of polygons, but allows to use polygon with index 0L to
        // get the correct normal for the whole polyPolygon
        BASEGFX_DLLPUBLIC B2DPolyPolygon correctOutmostPolygon(const B2DPolyPolygon& rCandidate);

        // Subdivide all contained curves. Use distanceBound value if given.
        BASEGFX_DLLPUBLIC B2DPolyPolygon adaptiveSubdivideByDistance(const B2DPolyPolygon& rCandidate, double fDistanceBound);

        // Subdivide all contained curves. Use distanceBound value if given. Else, a convenient one
        // is created.
        BASEGFX_DLLPUBLIC B2DPolyPolygon adaptiveSubdivideByAngle(const B2DPolyPolygon& rCandidate, double fAngleBound = 0.0);

        // isInside test for B2dPoint. On border is not inside as long as not true is given
        // in bWithBorder flag. It is assumed that the orientations of the given polygon are correct.
        BASEGFX_DLLPUBLIC bool isInside(const B2DPolyPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder = false);

        /** Get the range of a polyPolygon

            For detailed description look at getRange(const B2DPolygon&).
            This method just expands by the range of every sub-Polygon.

            @param rCandidate
            The B2DPolyPolygon possibly containing bezier segments

            @return
            The outer range of the polygon
        */
        BASEGFX_DLLPUBLIC B2DRange getRange(const B2DPolyPolygon& rCandidate);

        // get signed area of polygon
        BASEGFX_DLLPUBLIC double getSignedArea(const B2DPolyPolygon& rCandidate);

        // get area of polygon
        BASEGFX_DLLPUBLIC double getArea(const B2DPolyPolygon& rCandidate);

        /** Apply given LineDashing to given polyPolygon

            For a description see applyLineDashing in b2dpolygontoos.hxx
        */
        BASEGFX_DLLPUBLIC void applyLineDashing(
            const B2DPolyPolygon& rCandidate,
            const ::std::vector<double>& rDotDashArray,
            B2DPolyPolygon* pLineTarget,
            double fFullDashDotLen = 0.0);

        // test if point is inside epsilon-range around the given PolyPolygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the tube around the PolyPolygon
        // with distance fDistance and rounded edges (start and end point).
        BASEGFX_DLLPUBLIC bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance);

        /** Helper class to transport PointIndices to a PolyPolygon,
            with an operator< for convenient sorting in a std::set usage
         */
        class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC PointIndex
        {
        private:
            sal_uInt32 mnPolygonIndex;
            sal_uInt32 mnPointIndex;

        public:
            PointIndex(sal_uInt32 nPolygonIndex, sal_uInt32 nPointIndex)
            :   mnPolygonIndex(nPolygonIndex),
                mnPointIndex(nPointIndex)
            {}

            sal_uInt32 getPolygonIndex() const { return mnPolygonIndex; }
            sal_uInt32 getPointIndex() const { return mnPointIndex; }
            bool operator<(const PointIndex& rComp) const;
        };

        /** the PointIndexSet itself; it allows to define a 'selection'of
            points in a tools::PolyPolygon by giving the polygon and point index.
            Adding points double makes no sense, hence the std::set
         */
        typedef o3tl::sorted_vector< PointIndex > PointIndexSet;

        /** Read poly-polygon from SVG.

            This function imports a poly-polygon from an SVG-D
            attribute.

            @param o_rPolyPoly
            The output poly-polygon

            @param rSvgDAttribute
            A valid SVG-D attribute string

            @param bHandleRelativeNextPointCompatible
            If set to true, the old error that after a relative 'z' command
            the current point was not reset to the first point of the current
            polygon is kept; this is needed to read odf files.
            If false, pure svg is used; this is needed for svg import.

            @param pHelpPointIndexSet
            If given, all points created in the target PolyPolygon
            which are only helper points are added here using their
            point indices; this are currently points created from
            import of the 'a' and 'A' svg:d statements which create
            bezier curve info as representation and maybe points
            which are no 'real' svg:d points, but helper points. It
            is necessary to identify these e.g. when markers need to
            be created in the svg import

            @return true, if the string was successfully parsed
         */
        BASEGFX_DLLPUBLIC bool importFromSvgD(
            B2DPolyPolygon& o_rPolyPoly,
            const OUString& rSvgDAttribute,
            bool bHandleRelativeNextPointCompatible,
            PointIndexSet* pHelpPointIndexSet);

        // grow for polyPolygon. Move all geometry in each point in the direction of the normal in that point
        // with the given amount. Value may be negative.
        BASEGFX_DLLPUBLIC B2DPolyPolygon growInNormalDirection(const B2DPolyPolygon& rCandidate, double fValue);

        // force all sub-polygons to a point count of nSegments
        BASEGFX_DLLPUBLIC B2DPolyPolygon reSegmentPolyPolygon(const B2DPolyPolygon& rCandidate, sal_uInt32 nSegments);

        // create polygon state at t from 0.0 to 1.0 between the two polygons. Both polygons must have the same
        // organisation, e.g. same amount of polygons
        BASEGFX_DLLPUBLIC B2DPolyPolygon interpolate(const B2DPolyPolygon& rOld1, const B2DPolyPolygon& rOld2, double t);

        // create 3d tools::PolyPolygon from given 2d PolyPolygon. The given fZCoordinate is used to expand the
        // third coordinate.
        BASEGFX_DLLPUBLIC B3DPolyPolygon createB3DPolyPolygonFromB2DPolyPolygon(const B2DPolyPolygon& rCandidate, double fZCoordinate = 0.0);

        // create 2d tools::PolyPolygon from given 3d PolyPolygon. All coordinates are transformed using the given
        // matrix and the resulting x,y is used to form the new polygon.
        BASEGFX_DLLPUBLIC B2DPolyPolygon createB2DPolyPolygonFromB3DPolyPolygon(const B3DPolyPolygon& rCandidate, const B3DHomMatrix& rMat);

        // for each contained edge in each contained polygon calculate the smallest distance. Return the index to the smallest
        // edge in rEdgeIndex and the index to the polygon in rPolygonIndex. The relative position on the edge is returned in rCut.
        // If nothing was found (e.g. empty input plygon), DBL_MAX is returned.
        BASEGFX_DLLPUBLIC double getSmallestDistancePointToPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rPolygonIndex, sal_uInt32& rEdgeIndex, double& rCut);

        // distort PolyPolygon. rOriginal describes the original range, where the given points describe the distorted
        // corresponding points.
        BASEGFX_DLLPUBLIC B2DPolyPolygon distort(const B2DPolyPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight);

        // expand all segments (which are not yet) to curve segments. This is done with setting the control
        // vectors on the 1/3 resp. 2/3 distances on each segment.
        BASEGFX_DLLPUBLIC B2DPolyPolygon expandToCurve(const B2DPolyPolygon& rCandidate);

        /** Predicate whether a given poly-polygon is a rectangle.

            @param rPoly
            tools::PolyPolygon to check

            @return true, if the poly-polygon describes a rectangle
            (contains exactly one polygon, polygon is closed, and the
            points are either cw or ccw enumerations of a rectangle's
            vertices). Note that intermediate points and duplicate
            points are ignored.
         */
        BASEGFX_DLLPUBLIC bool isRectangle( const B2DPolyPolygon& rPoly );

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

            @param bHandleRelativeNextPointCompatible
            If set to true, the old error that after a relative 'z' command
            the current point was not reset to the first point of the current
            polygon is kept; this is needed to read odf files.
            If false, pure svg is used; this is needed for svg import.

            @param bOOXMLMotionPath
            If set to true, export string format that is acceptable for
            for animation motion path for PowerPoint: always space delimited,
            never neglect command char, always end with E, and do not export
            H or V.

            @return the generated SVG-D statement (the XML d attribute
            value alone, without any "<path ...>" or "d="...")
         */
        BASEGFX_DLLPUBLIC OUString exportToSvgD(
            const B2DPolyPolygon& rPolyPoly,
            bool bUseRelativeCoordinates,
            bool bDetectQuadraticBeziers,
            bool bHandleRelativeNextPointCompatible,
            bool bOOXMLMotionPath = false);

        // #i76891# Try to remove existing curve segments if they are simply edges
        BASEGFX_DLLPUBLIC B2DPolyPolygon simplifyCurveSegments(const B2DPolyPolygon& rCandidate);

        /** Creates polypolygon for seven-segment display number

            This function takes an integer number between 0 and 9 and
            convert it into the well-known seven-segment display
            number (like most digital clocks show their numbers). The
            digit will exactly fit the unit rectangle. The polypolygon
            will be a line polygon, i.e. if you need the segment parts
            to have width, use createAreaGeometry() on the result.

            @param cNumber
            Number from '0' to '9' as ASCII char, or '-', 'E' and '.'
            to convert to 7 segment code

            @param bLitSegments
            When true, return a polygon containing the segments that
            are 'lit' for the given number. Return un-lit segments
            otherwise.
         */
        B2DPolyPolygon createSevenSegmentPolyPolygon(char cNumber, bool bLitSegments);

        /** snap some polygon coordinates to discrete coordinates

            This method allows to snap some polygon points to discrete (integer) values
            which equals e.g. a snap to discrete coordinates. It will snap points of
            horizontal and vertical edges

            @param rCandidate
            The source polygon

            @return
            The modified version of the source polygon
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolyPolygon& rCandidate);

        /// converters for css::drawing::PointSequence
        BASEGFX_DLLPUBLIC B2DPolyPolygon UnoPointSequenceSequenceToB2DPolyPolygon(
            const css::drawing::PointSequenceSequence& rPointSequenceSequenceSource);
        BASEGFX_DLLPUBLIC void B2DPolyPolygonToUnoPointSequenceSequence(
            const B2DPolyPolygon& rPolyPolygon,
            css::drawing::PointSequenceSequence& rPointSequenceSequenceRetval);

        /// converters for css::drawing::PolyPolygonBezierCoords (curved polygons)
        BASEGFX_DLLPUBLIC B2DPolyPolygon UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
            const css::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsSource);
        BASEGFX_DLLPUBLIC void B2DPolyPolygonToUnoPolyPolygonBezierCoords(
            const B2DPolyPolygon& rPolyPolygon,
            css::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsRetval);

} // end of namespace basegfx::utils

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
