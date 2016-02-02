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

#ifndef INCLUDED_BASEGFX_POLYGON_B2DPOLYGONCLIPPER_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DPOLYGONCLIPPER_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    // predefinitions
    class B2DRange;

    namespace tools
    {
        // This method clips the given tools::PolyPolygon against a horizontal or vertical axis (parallel to X or Y axis). The axis is
        // defined by bParallelToXAxis (true -> it's parallel to the X-Axis of the coordinate system, else to the Y-Axis) and the
        // fValueOnOtherAxis (gives the translation to the coordinate system axis). For example, when You want to define
        // a clip axis parallel to X.Axis and 100 above it, use bParallelToXAxis = true and fValueOnOtherAxis = 100.
        // The value bAboveAxis defines on which side the return value will be (true -> above X, right of Y).
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        BASEGFX_DLLPUBLIC B2DPolyPolygon clipPolyPolygonOnParallelAxis(const B2DPolyPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke);
        BASEGFX_DLLPUBLIC B2DPolyPolygon clipPolygonOnParallelAxis(const B2DPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke);

        // Clip the given tools::PolyPolygon against the given range. bInside defines if the result will contain the
        // parts which are contained in the range or vice versa.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        BASEGFX_DLLPUBLIC B2DPolyPolygon clipPolyPolygonOnRange(const B2DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);
        BASEGFX_DLLPUBLIC B2DPolyPolygon clipPolygonOnRange(const B2DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);

        // Clip given tools::PolyPolygon against given clipping polygon.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        // With stroke polygons, You get all line snippets inside rCip.
        // With filled polygons, You get all tools::PolyPolygon parts which were inside rClip.
        // The switch bInside decides if the parts inside the clip polygon or outside shall be created.
        // The clip polygon is always assumed closed, even when it's isClosed() is false.
        BASEGFX_DLLPUBLIC B2DPolyPolygon clipPolyPolygonOnPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke);
        BASEGFX_DLLPUBLIC B2DPolyPolygon clipPolygonOnPolyPolygon(const B2DPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke);

        // clip the given polygon against the given range. the resulting polygon will always contain
        // the inside parts which will always be interpreted as areas. the incoming polygon is expected
        // to be a simple triangle list. the result is also a simple triangle list.
        BASEGFX_DLLPUBLIC B2DPolygon clipTriangleListOnRange( const B2DPolygon& rCandidate, const B2DRange& rRange );

    } // end of namespace tools
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_POLYGON_B2DPOLYGONCLIPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
