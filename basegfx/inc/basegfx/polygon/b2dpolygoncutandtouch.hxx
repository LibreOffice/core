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

#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#define _BGFX_POLYGON_CUTANDTOUCH_HXX

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // look for self-intersections and self-touches (points on an edge) in given polygon and add
        // extra points there. Result will have no touches or intersections on an edge, only on points
        B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate);

        // look for polypolygon-intersections and polypolygon-touches (point of poly A on an edge of poly B) in given PolyPolygon and add
        // extra points there. Result will have no touches or intersections between contained polygons on an edge, only on points. For
        // convenience, the correction for self-intersections for each member polygon will be used, too.
        // Changed: Self intersections are searched by default, but may be switched off by 2nd parameter.
        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate, bool bSelfIntersections = true);

        // look for intersections of rCandidate with all polygons from rMask and add extra points there. Do
        // not change or add points to rMask.
        B2DPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolygon& rCandidate);

        // look for intersections of rCandidate with all polygons from rMask and add extra points there. Do
        // not change or add points to rMask.
        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolyPolygon& rCandidate);

        // look for intersections of rCandidate with the edge from rStart to rEnd and add extra points there.
        // Points are only added in the range of the edge, not on the endless vector.
        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd);
        B2DPolyPolygon addPointsAtCuts(const B2DPolyPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd);

        // look for intersections of rCandidate with the mask Polygon and add extra points there.
        // The mask polygon is assumed to be closed, even when it's not explicitely.
        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPolyPolygon& rMask);
        B2DPolyPolygon addPointsAtCuts(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rMask);

        // look for self-intersections in given polygon and add extra points there. Result will have no
        // intersections on an edge
        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate);

        // add points at all self-intersections of single polygons (depends on bSelfIntersections)
        // and at polygon-polygon intersections
        B2DPolyPolygon addPointsAtCuts(const B2DPolyPolygon& rCandidate, bool bSelfIntersections = true);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_CUTANDTOUCH_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
