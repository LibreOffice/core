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



#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX
#define _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX

#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/basegfxdllapi.h>

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
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolyPolygonOnOrthogonalPlane(const B3DPolyPolygon& rCandidate, B3DOrientation ePlaneOrthogonal, bool bClipPositive, double fPlaneOffset, bool bStroke);

        // version for Polygons
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolygonOnOrthogonalPlane(const B3DPolygon& rCandidate, B3DOrientation ePlaneOrthogonal, bool bClipPositive, double fPlaneOffset, bool bStroke);

        // Clip the given PolyPolygon against the given range. bInside defines if the result will contain the
        // parts which are contained in the range or vice versa.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolyPolygonOnRange(const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bInside, bool bStroke);

        // version for Polygons
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolygonOnRange(const B3DPolygon& rCandidate, const B3DRange& rRange, bool bInside, bool bStroke);

        // versions for B2DRange, clips only against X,Y
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolyPolygonOnRange(const B3DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolygonOnRange(const B3DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);

        // Clip the given PolyPolygon against given plane in 3D. The plane is defined by a plane normal and a point on the plane.
        // The value bClipPositive defines on which side the return value will be (true -> on positive side of plane).
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolyPolygonOnPlane(const B3DPolyPolygon& rCandidate, const B3DPoint& rPointOnPlane, const B3DVector& rPlaneNormal, bool bClipPositive, bool bStroke);

        // version for Polygons
        BASEGFX_DLLPUBLIC B3DPolyPolygon clipPolygonOnPlane(const B3DPolygon& rCandidate, const B3DPoint& rPointOnPlane, const B3DVector& rPlaneNormal, bool bClipPositive, bool bStroke);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX */
