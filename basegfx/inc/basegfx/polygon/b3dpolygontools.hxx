/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dpolygontools.hxx,v $
 * $Revision: 1.8 $
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
        void checkClosed(::basegfx::B3DPolygon& rCandidate);

        // Get successor and predecessor indices. Returning the same index means there
        // is none. Same for successor.
        sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const ::basegfx::B3DPolygon& rCandidate);
        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const ::basegfx::B3DPolygon& rCandidate);

        // get size of polygon. Control vectors are included in that ranges.
        ::basegfx::B3DRange getRange(const ::basegfx::B3DPolygon& rCandidate);

        // get normal vector of polygon
        ::basegfx::B3DVector getNormal(const ::basegfx::B3DPolygon& rCandidate);

        // get signed area of polygon
        double getSignedArea(const B3DPolygon& rCandidate);

        // get area of polygon
        double getArea(const ::basegfx::B3DPolygon& rCandidate);

        // get length of polygon edge from point nIndex to nIndex + 1
        double getEdgeLength(const ::basegfx::B3DPolygon& rCandidate, sal_uInt32 nIndex);

        // get length of polygon
        double getLength(const ::basegfx::B3DPolygon& rCandidate);

        // get position on polygon for absolute given distance. If
        // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
        // using getLength(...)
        ::basegfx::B3DPoint getPositionAbsolute(const ::basegfx::B3DPolygon& rCandidate, double fDistance, double fLength = 0.0);

        // get position on polygon for relative given distance in range [0.0 .. 1.0]. If
        // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
        // using getLength(...)
        ::basegfx::B3DPoint getPositionRelative(const ::basegfx::B3DPolygon& rCandidate, double fDistance, double fLength = 0.0);

        // Apply Line Dashing. This cuts the Polygon into line pieces
        // which are inserted as single polygons into the result.
        // If fFullDashDotLen is not given it will be calculated from the given
        // raDashDotArray.
        ::basegfx::B3DPolyPolygon applyLineDashing(const ::basegfx::B3DPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen = 0.0);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX */
