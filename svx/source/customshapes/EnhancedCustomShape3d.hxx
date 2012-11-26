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

#ifndef _ENHANCEDCUSTOMSHAPE3D_HXX
#define _ENHANCEDCUSTOMSHAPE3D_HXX

#include <vector>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>

class SdrObject;
class SdrObjCustomShape;

class EnhancedCustomShape3d
{
private:
    class Transformation2D
    {
    private:
        basegfx::B2DPoint                           maCenter;
        com::sun::star::drawing::ProjectionMode     meProjectionMode;

        // parallel projection
        double                  mfSkewAngle;
        double                  mfSkew;         // in percent

        // perspective projection
        double                  mfZScreen;
        basegfx::B3DPoint       mfViewPoint;
        double                  mfOriginX;
        double                  mfOriginY;

        const double            mfMap;

    public :
        Transformation2D(const SdrObjCustomShape& rCustomShape, const double fMap);

        basegfx::B3DPolygon ApplySkewSettings(const basegfx::B3DPolygon& rPolygon3D) const;
        basegfx::B2DPoint Transform2D(const basegfx::B3DPoint& rPoint) const;
        bool IsParallel() const;
    };

    friend class Transformation2D;

protected :
    static basegfx::B2DRange CalculateNewSnapRect(
        const SdrObjCustomShape& rCustomShape,
        const basegfx::B2DRange& rSnapRect,
        const basegfx::B2DRange& rBoundRect,
        const double fMap);

public :
    static SdrObject* Create3DObject(
        const SdrObject& rShape2d,
        const SdrObjCustomShape& rCustomShape);
};

#endif

