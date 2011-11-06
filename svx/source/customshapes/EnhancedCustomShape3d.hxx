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

#include <tools/gen.hxx>

class SdrObject;

class EnhancedCustomShape3d
{
    class Transformation2D
    {
        Point                                   aCenter;
        com::sun::star::drawing::ProjectionMode eProjectionMode;

        // parallel projection
        double      fSkewAngle;
        double      fSkew;          // in percent

        // perspective projection
        double      fZScreen;
        basegfx::B3DPoint       fViewPoint;
        double      fOriginX;
        double      fOriginY;

        const double* pMap;

        public :

                        Transformation2D( const SdrObject* pCustomShape, const Rectangle& rBoundRect, const double* pMap );

            basegfx::B3DPolygon ApplySkewSettings( const basegfx::B3DPolygon& rPolygon3D ) const;
            Point       Transform2D( const basegfx::B3DPoint& rPoint ) const;
            sal_Bool    IsParallel() const;
    };

    friend class Transformation2D;

    protected :
        static Rectangle CalculateNewSnapRect( const SdrObject* pCustomShape, const Rectangle& rSnapRect, const Rectangle& rBoundRect, const double* pMap );

    public :
        static SdrObject* Create3DObject( const SdrObject* pShape2d, const SdrObject* pCustomShape );
};

#endif

