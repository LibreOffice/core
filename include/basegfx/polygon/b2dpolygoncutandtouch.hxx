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

#ifndef INCLUDED_BASEGFX_POLYGON_B2DPOLYGONCUTANDTOUCH_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DPOLYGONCUTANDTOUCH_HXX

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    namespace utils
    {
        // look for self-intersections and self-touches (points on an edge) in given polygon and add
        // extra points there. Result will have no touches or intersections on an edge, only on points
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate);

        // look for polypolygon-intersections and polypolygon-touches (point of poly A on an edge of poly B) in given tools::PolyPolygon and add
        // extra points there. Result will have no touches or intersections between contained polygons on an edge, only on points. For
        // convenience, the correction for self-intersections for each member polygon will be used, too.
        // Changed: Self intersections are searched by default, but may be switched off by 2nd parameter.
        BASEGFX_DLLPUBLIC B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate, bool bSelfIntersections = true);

        // look for intersections of rCandidate with the edge from rStart to rEnd and add extra points there.
        // Points are only added in the range of the edge, not on the endless vector.
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd);

        // look for intersections of rCandidate with the mask Polygon and add extra points there.
        // The mask polygon is assumed to be closed, even when it's not explicitly.
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPolyPolygon& rMask);

    } // end of namespace utils
} // end of namespace basegfx


#endif // INCLUDED_BASEGFX_POLYGON_B2DPOLYGONCUTANDTOUCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
