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

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#define _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B3DPolygon;
    class B3DRange;

    namespace tools
    {
        // B3DPolygon tools

        /** Check if given polygon is closed. This is kind of a
            'classic' method to support old polygon definitions.
            Those old polygon definitions define the closed state
            of the polygon using identical start and endpoints. This
            method corrects this (removes double start/end points)
            and sets the Closed()-state of the polygon correctly.
        */
        void checkClosed(B3DPolygon& rCandidate);

        // Get successor and predecessor indices. Returning the same index means there
        // is none. Same for successor.
        sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const B3DPolygon& rCandidate);
        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B3DPolygon& rCandidate);

        // Get orientation of Polygon
        B2VectorOrientation getOrientation(const B3DPolygon& rCandidate);

        // get size of polygon. Control vectors are included in that ranges.
        B3DRange getRange(const B3DPolygon& rCandidate);

        // get normal vector of polygon
        B3DVector getNormal(const B3DPolygon& rCandidate);

        // get normal vector of positive oriented polygon
        B3DVector getPositiveOrientedNormal(const B3DPolygon& rCandidate);

        // get signed area of polygon
        double getSignedArea(const B3DPolygon& rCandidate);

        // get area of polygon
        double getArea(const B3DPolygon& rCandidate);

        // get signed area of polygon
        double getSignedArea(const B3DPolygon& rCandidate);

        // get area of polygon
        double getArea(const ::basegfx::B3DPolygon& rCandidate);

        // get length of polygon edge from point nIndex to nIndex + 1
        double getEdgeLength(const B3DPolygon& rCandidate, sal_uInt32 nIndex);

        // get length of polygon
        double getLength(const B3DPolygon& rCandidate);

        // get position on polygon for absolute given distance. If
        // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
        // using getLength(...)
        B3DPoint getPositionAbsolute(const B3DPolygon& rCandidate, double fDistance, double fLength = 0.0);

        // get position on polygon for relative given distance in range [0.0 .. 1.0]. If
        // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
        // using getLength(...)
        B3DPoint getPositionRelative(const B3DPolygon& rCandidate, double fDistance, double fLength = 0.0);

        /** Apply given LineDashing to given polygon

            For a description see applyLineDashing in b2dpolygontoos.hxx
        */
        void applyLineDashing(
            const B3DPolygon& rCandidate,
            const ::std::vector<double>& rDotDashArray,
            B3DPolyPolygon* pLineTarget,
            B3DPolyPolygon* pGapTarget = 0,
            double fFullDashDotLen = 0.0);

        /** Create/replace normals for given 3d geometry with default normals from given center to outside.
            rCandidate: the 3d geometry to change
            rCenter:    the center of the 3d geometry
         */
        B3DPolygon applyDefaultNormalsSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter);

        /** invert normals for given 3d geometry.
         */
        B3DPolygon invertNormals( const B3DPolygon& rCandidate);

        /** Create/replace texture coordinates for given 3d geometry with parallel projected one
            rRange: the full range of the 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        B3DPolygon applyDefaultTextureCoordinatesParallel( const B3DPolygon& rCandidate, const B3DRange& rRange, bool bChangeX = true, bool bChangeY = true);

        /** Create/replace texture coordinates for given 3d geometry with spherical one
            rCenter: the centre of the used 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        B3DPolygon applyDefaultTextureCoordinatesSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX = true, bool bChangeY = true);

        // test if point is inside epsilon-range around an edge defined
        // by the two given points. Can be used for HitTesting. The epsilon-range
        // is defined to be the cylinder centered to the given edge, using radius
        // fDistance, and the sphere around both points with radius fDistance.
        bool isInEpsilonRange(const B3DPoint& rEdgeStart, const B3DPoint& rEdgeEnd, const B3DPoint& rTestPosition, double fDistance);

        // test if point is inside epsilon-range around the given Polygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the cylinder centered to
        // the given edge, using radius fDistance, and the sphere around both points with radius fDistance.
        bool isInEpsilonRange(const B3DPolygon& rCandidate, const B3DPoint& rTestPosition, double fDistance);

        // isInside tests for B3DPoint and other B3DPolygon. On border is not inside as long as
        // not true is given in bWithBorder flag.
        bool isInside(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithBorder = false);
        bool isInside(const B3DPolygon& rCandidate, const B3DPolygon& rPolygon, bool bWithBorder = false);

        // calculates if given point is on given line, taking care of the numerical epsilon
        bool isPointOnLine(const B3DPoint& rStart, const B3DPoint& rEnd, const B3DPoint& rCandidate, bool bWithPoints = false);

        // calculates if given point is on given polygon, taking care of the numerical epsilon. Uses
        // isPointOnLine internally
        bool isPointOnPolygon(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithPoints = true);

        // helper to get a fCut position between a plane (given with normal and a point)
        // and a line given by start and end point
        bool getCutBetweenLineAndPlane(const B3DVector& rPlaneNormal, const B3DPoint& rPlanePoint, const B3DPoint& rEdgeStart, const B3DPoint& rEdgeEnd, double& fCut);

        // helper to get a fCut position between a 3d Polygon
        // and a line given by start and end point
        bool getCutBetweenLineAndPolygon(const B3DPolygon& rCandidate, const B3DPoint& rEdgeStart, const B3DPoint& rEdgeEnd, double& fCut);

        //////////////////////////////////////////////////////////////////////
        // comparators with tolerance for 3D Polygons
        bool equal(const B3DPolygon& rCandidateA, const B3DPolygon& rCandidateB, const double& rfSmallValue);
        bool equal(const B3DPolygon& rCandidateA, const B3DPolygon& rCandidateB);

        /** snap some polygon coordinates to discrete coordinates

            This method allows to snap some polygon points to discrete (integer) values
            which equals e.g. a snap to discrete coordinates. It will snap points of
            horizontal and vertical edges

            @param rCandidate
            The source polygon

            @return
            The modified version of the source polygon
        */
        B3DPolygon snapPointsOfHorizontalOrVerticalEdges(const B3DPolygon& rCandidate);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
