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
#include <basegfx/basegfxdllapi.h>

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
        BASEGFX_DLLPUBLIC void checkClosed(B3DPolygon& rCandidate);

        // Get successor and predecessor indices. Returning the same index means there
        // is none. Same for successor.
        BASEGFX_DLLPUBLIC sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B3DPolygon& rCandidate);

        // get size of polygon. Control vectors are included in that ranges.
        BASEGFX_DLLPUBLIC B3DRange getRange(const B3DPolygon& rCandidate);

        // get normal vector of polygon
        BASEGFX_DLLPUBLIC B3DVector getNormal(const B3DPolygon& rCandidate);

        // get area of polygon
        BASEGFX_DLLPUBLIC double getArea(const ::basegfx::B3DPolygon& rCandidate);

        // get length of polygon
        BASEGFX_DLLPUBLIC double getLength(const B3DPolygon& rCandidate);

        /** Apply given LineDashing to given polygon

            For a description see applyLineDashing in b2dpolygontoos.hxx
        */
        BASEGFX_DLLPUBLIC void applyLineDashing(
            const B3DPolygon& rCandidate,
            const ::std::vector<double>& rDotDashArray,
            B3DPolyPolygon* pLineTarget,
            B3DPolyPolygon* pGapTarget = 0,
            double fFullDashDotLen = 0.0);

        /** Create/replace normals for given 3d geometry with default normals from given center to outside.
            rCandidate: the 3d geometry to change
            rCenter:    the center of the 3d geometry
         */
        BASEGFX_DLLPUBLIC B3DPolygon applyDefaultNormalsSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter);

        /** invert normals for given 3d geometry.
         */
        BASEGFX_DLLPUBLIC B3DPolygon invertNormals( const B3DPolygon& rCandidate);

        /** Create/replace texture coordinates for given 3d geometry with parallel projected one
            rRange: the full range of the 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        BASEGFX_DLLPUBLIC B3DPolygon applyDefaultTextureCoordinatesParallel( const B3DPolygon& rCandidate, const B3DRange& rRange, bool bChangeX = true, bool bChangeY = true);

        /** Create/replace texture coordinates for given 3d geometry with spherical one
            rCenter: the centre of the used 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        BASEGFX_DLLPUBLIC B3DPolygon applyDefaultTextureCoordinatesSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX = true, bool bChangeY = true);

        // isInside tests for B3DPoint. On border is not inside as long as not true is given in bWithBorder flag.
        BASEGFX_DLLPUBLIC bool isInside(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithBorder = false);

        // calculates if given point is on given line, taking care of the numerical epsilon
        BASEGFX_DLLPUBLIC bool isPointOnLine(const B3DPoint& rStart, const B3DPoint& rEnd, const B3DPoint& rCandidate, bool bWithPoints = false);

        // calculates if given point is on given polygon, taking care of the numerical epsilon. Uses
        // isPointOnLine internally
        BASEGFX_DLLPUBLIC bool isPointOnPolygon(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithPoints = true);

        // helper to get a fCut position between a plane (given with normal and a point)
        // and a line given by start and end point
        BASEGFX_DLLPUBLIC bool getCutBetweenLineAndPlane(const B3DVector& rPlaneNormal, const B3DPoint& rPlanePoint, const B3DPoint& rEdgeStart, const B3DPoint& rEdgeEnd, double& fCut);

        /** snap some polygon coordinates to discrete coordinates

            This method allows to snap some polygon points to discrete (integer) values
            which equals e.g. a snap to discrete coordinates. It will snap points of
            horizontal and vertical edges

            @param rCandidate
            The source polygon

            @return
            The modified version of the source polygon
        */
        BASEGFX_DLLPUBLIC B3DPolygon snapPointsOfHorizontalOrVerticalEdges(const B3DPolygon& rCandidate);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
