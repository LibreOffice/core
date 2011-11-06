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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/curve/b2dquadraticbezier.hxx>
#include <basegfx/numeric/ftools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B2DQuadraticBezier::B2DQuadraticBezier(const B2DQuadraticBezier& rBezier)
    :   maStartPoint(rBezier.maStartPoint),
        maEndPoint(rBezier.maEndPoint),
        maControlPoint(rBezier.maControlPoint)
    {
    }

    B2DQuadraticBezier::B2DQuadraticBezier()
    {
    }

    B2DQuadraticBezier::B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd)
    {
    }

    B2DQuadraticBezier::B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rControl, const ::basegfx::B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd),
        maControlPoint(rControl)
    {
    }

    B2DQuadraticBezier::~B2DQuadraticBezier()
    {
    }

    // assignment operator
    B2DQuadraticBezier& B2DQuadraticBezier::operator=(const B2DQuadraticBezier& rBezier)
    {
        maStartPoint = rBezier.maStartPoint;
        maEndPoint = rBezier.maEndPoint;
        maControlPoint = rBezier.maControlPoint;

        return *this;
    }

    // compare operators
    bool B2DQuadraticBezier::operator==(const B2DQuadraticBezier& rBezier) const
    {
        return (
            maStartPoint == rBezier.maStartPoint
            && maEndPoint == rBezier.maEndPoint
            && maControlPoint == rBezier.maControlPoint
        );
    }

    bool B2DQuadraticBezier::operator!=(const B2DQuadraticBezier& rBezier) const
    {
        return (
            maStartPoint != rBezier.maStartPoint
            || maEndPoint != rBezier.maEndPoint
            || maControlPoint != rBezier.maControlPoint
        );
    }

    // test if control vector is used
    bool B2DQuadraticBezier::isBezier() const
    {
        // if control vector is empty, bezier is not used
        if(maControlPoint == maStartPoint || maControlPoint == maEndPoint)
            return false;

        return true;
    }
} // end of namespace basegfx

// eof
