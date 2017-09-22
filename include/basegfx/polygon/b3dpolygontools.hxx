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

#ifndef INCLUDED_BASEGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#define INCLUDED_BASEGFX_POLYGON_B3DPOLYGONTOOLS_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    // predefinitions
    class B3DPolygon;
    class B3DRange;

    namespace utils
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

        // get length of polygon
        BASEGFX_DLLPUBLIC double getLength(const B3DPolygon& rCandidate);

        /** Apply given LineDashing to given polygon

            For a description see applyLineDashing in b2dpolygontoos.hxx
        */
        BASEGFX_DLLPUBLIC void applyLineDashing(
            const B3DPolygon& rCandidate,
            const ::std::vector<double>& rDotDashArray,
            B3DPolyPolygon* pLineTarget,
            B3DPolyPolygon* pGapTarget,
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
        BASEGFX_DLLPUBLIC B3DPolygon applyDefaultTextureCoordinatesParallel( const B3DPolygon& rCandidate, const B3DRange& rRange, bool bChangeX, bool bChangeY = true);

        /** Create/replace texture coordinates for given 3d geometry with spherical one
            rCenter: the centre of the used 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        BASEGFX_DLLPUBLIC B3DPolygon applyDefaultTextureCoordinatesSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX, bool bChangeY = true);

        // isInside tests for B3DPoint. On border is not inside as long as not true is given in bWithBorder flag.
        BASEGFX_DLLPUBLIC bool isInside(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithBorder);

        // calculates if given point is on given line, taking care of the numerical epsilon
        BASEGFX_DLLPUBLIC bool isPointOnLine(const B3DPoint& rStart, const B3DPoint& rEnd, const B3DPoint& rCandidate, bool bWithPoints);

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

    } // end of namespace utils
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_POLYGON_B3DPOLYGONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
