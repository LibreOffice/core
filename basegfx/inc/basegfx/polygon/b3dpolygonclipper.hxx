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

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX
#define _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX

#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B3DRange;
    class B2DRange;

    namespace tools
    {
        /** define for deciding one of X,Y,Z directions
        */
        enum B3DOrientation
        {
            B3DORIENTATION_X,       // X-Axis
            B3DORIENTATION_Y,       // Y-Axis
            B3DORIENTATION_Z        // Z-Axis
        };

        // Clip given 3D polygon against a plane orthogonal to X,Y or Z axis. The plane is defined using the
        // enum ePlaneOrthogonal which names the vector orthogonal to the plane, the fPlaneOffset gives the distance
        // of the plane from the center (0.0).
        // The value bClipPositive defines on which side the return value will be (true -> on positive side of plane).
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B3DPolyPolygon clipPolyPolygonOnOrthogonalPlane(const B3DPolyPolygon& rCandidate, B3DOrientation ePlaneOrthogonal, bool bClipPositive, double fPlaneOffset, bool bStroke);

        // version for Polygons
        B3DPolyPolygon clipPolygonOnOrthogonalPlane(const B3DPolygon& rCandidate, B3DOrientation ePlaneOrthogonal, bool bClipPositive, double fPlaneOffset, bool bStroke);

        // Clip the given PolyPolygon against the given range. bInside defines if the result will contain the
        // parts which are contained in the range or vice versa.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B3DPolyPolygon clipPolyPolygonOnRange(const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bInside, bool bStroke);

        // version for Polygons
        B3DPolyPolygon clipPolygonOnRange(const B3DPolygon& rCandidate, const B3DRange& rRange, bool bInside, bool bStroke);

        // versions for B2DRange, clips only against X,Y
        B3DPolyPolygon clipPolyPolygonOnRange(const B3DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);
        B3DPolyPolygon clipPolygonOnRange(const B3DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);

        // Clip the given PolyPolygon against given plane in 3D. The plane is defined by a plane normal and a point on the plane.
        // The value bClipPositive defines on which side the return value will be (true -> on positive side of plane).
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B3DPolyPolygon clipPolyPolygonOnPlane(const B3DPolyPolygon& rCandidate, const B3DPoint& rPointOnPlane, const B3DVector& rPlaneNormal, bool bClipPositive, bool bStroke);

        // version for Polygons
        B3DPolyPolygon clipPolygonOnPlane(const B3DPolygon& rCandidate, const B3DPoint& rPointOnPlane, const B3DVector& rPlaneNormal, bool bClipPositive, bool bStroke);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
