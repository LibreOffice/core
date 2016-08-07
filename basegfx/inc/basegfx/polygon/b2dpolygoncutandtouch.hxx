/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#define _BGFX_POLYGON_CUTANDTOUCH_HXX

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // look for self-intersections and self-touches (points on an edge) in given polygon and add
        // extra points there. Result will have no touches or intersections on an edge, only on points
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate);

        // look for polypolygon-intersections and polypolygon-touches (point of poly A on an edge of poly B) in given PolyPolygon and add
        // extra points there. Result will have no touches or intersections between contained polygons on an edge, only on points. For
        // convenience, the correction for self-intersections for each member polygon will be used, too.
        // Changed: Self intersections are searched by default, but may be switched off by 2nd parameter.
        BASEGFX_DLLPUBLIC B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate, bool bSelfIntersections = true);

        // look for intersections of rCandidate with all polygons from rMask and add extra points there. Do
        // not change or add points to rMask.
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolygon& rCandidate);

        // look for intersections of rCandidate with all polygons from rMask and add extra points there. Do
        // not change or add points to rMask.
        BASEGFX_DLLPUBLIC B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolyPolygon& rCandidate);

        // look for intersections of rCandidate with the edge from rStart to rEnd and add extra points there.
        // Points are only added in the range of the edge, not on the endless vector.
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd);
        BASEGFX_DLLPUBLIC B2DPolyPolygon addPointsAtCuts(const B2DPolyPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd);

        // look for intersections of rCandidate with the mask Polygon and add extra points there.
        // The mask polygon is assumed to be closed, even when it's not explicitely.
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPolyPolygon& rMask);
        BASEGFX_DLLPUBLIC B2DPolyPolygon addPointsAtCuts(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rMask);

        // look for self-intersections in given polygon and add extra points there. Result will have no
        // intersections on an edge
        BASEGFX_DLLPUBLIC B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate);

        // add points at all self-intersections of single polygons (depends on bSelfIntersections)
        // and at polygon-polygon intersections
        BASEGFX_DLLPUBLIC B2DPolyPolygon addPointsAtCuts(const B2DPolyPolygon& rCandidate, bool bSelfIntersections = true);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_CUTANDTOUCH_HXX */
