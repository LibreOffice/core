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

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#define _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B2DRange;

    namespace tools
    {
        // This method clips the given PolyPolygon against a horizontal or vertical axis (parallell to X or Y axis). The axis is
        // defined by bParallelToXAxis (true -> it's parallel to the X-Axis of the coordinate system, else to the Y-Axis) and the
        // fValueOnOtherAxis (gives the translation to the coordinate system axis). For example, when You want to define
        // a clip axis parallel to X.Axis and 100 above it, use bParallelToXAxis = true and fValueOnOtherAxis = 100.
        // The value bAboveAxis defines on which side the return value will be (true -> above X, right of Y).
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B2DPolyPolygon clipPolyPolygonOnParallelAxis(const B2DPolyPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke);
        B2DPolyPolygon clipPolygonOnParallelAxis(const B2DPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke);

        // Clip the given PolyPolygon against the given range. bInside defines if the result will contain the
        // parts which are contained in the range or vice versa.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B2DPolyPolygon clipPolyPolygonOnRange(const B2DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);
        B2DPolyPolygon clipPolygonOnRange(const B2DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);

        // Clip given PolyPolygon against the endless edge (ray) defined by the given two points. bAbove defines on which side
        // of the edge the result will be together with the definition of the edge. If the edge is seen as a vector
        // from A to B and bAbove is true, the result will contain the geometry left of the vector.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B2DPolyPolygon clipPolyPolygonOnEdge(const B2DPolyPolygon& rCandidate, const B2DPoint& rPointA, const B2DPoint& rPointB, bool bAbove, bool bStroke);
        B2DPolyPolygon clipPolygonOnEdge(const B2DPolygon& rCandidate, const B2DPoint& rPointA, const B2DPoint& rPointB, bool bAbove, bool bStroke);

        // Clip given PolyPolygon against given clipping polygon.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        // With stroke polygons, You get all line snippets inside rCip.
        // With filled polygons, You get all PolyPolygon parts which were inside rClip.
        // The switch bInside decides if the parts inside the clip polygon or outside shall be created.
        // The clip polygon is always assumed closed, even when it's isClosed() is false.
        B2DPolyPolygon clipPolyPolygonOnPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke);
        B2DPolyPolygon clipPolygonOnPolyPolygon(const B2DPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke);

        // clip the given polygon against the given range. the resulting polygon will always contain
        // the inside parts which will always be interpreted as areas. the incoming polygon is expected
        // to be a simple triangle list. the result is also a simple triangle list.
        B2DPolygon clipTriangleListOnRange( const B2DPolygon& rCandidate, const B2DRange& rRange );

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
