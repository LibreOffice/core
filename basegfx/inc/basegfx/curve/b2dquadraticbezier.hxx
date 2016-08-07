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



#ifndef _BGFX_CURVE_B2DQUADRATICBEZIER_HXX
#define _BGFX_CURVE_B2DQUADRATICBEZIER_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BASEGFX_DLLPUBLIC B2DQuadraticBezier
    {
        ::basegfx::B2DPoint                         maStartPoint;
        ::basegfx::B2DPoint                         maEndPoint;
        ::basegfx::B2DPoint                         maControlPoint;

    public:
        B2DQuadraticBezier();
        B2DQuadraticBezier(const B2DQuadraticBezier& rBezier);
        B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd);
        B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart,
            const ::basegfx::B2DPoint& rControlPoint, const ::basegfx::B2DPoint& rEnd);
        ~B2DQuadraticBezier();

        // assignment operator
        B2DQuadraticBezier& operator=(const B2DQuadraticBezier& rBezier);

        // compare operators
        bool operator==(const B2DQuadraticBezier& rBezier) const;
        bool operator!=(const B2DQuadraticBezier& rBezier) const;

        // test if control point is placed on the edge
        bool isBezier() const;

        // data interface
        ::basegfx::B2DPoint getStartPoint() const { return maStartPoint; }
        void setStartPoint(const ::basegfx::B2DPoint& rValue) { maStartPoint = rValue; }

        ::basegfx::B2DPoint getEndPoint() const { return maEndPoint; }
        void setEndPoint(const ::basegfx::B2DPoint& rValue) { maEndPoint = rValue; }

        ::basegfx::B2DPoint getControlPoint() const { return maControlPoint; }
        void setControlPoint(const ::basegfx::B2DPoint& rValue) { maControlPoint = rValue; }
    };
} // end of namespace basegfx

#endif /* _BGFX_CURVE_B2DQUADRATICBEZIER_HXX */
