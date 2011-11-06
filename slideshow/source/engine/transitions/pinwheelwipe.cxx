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
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "clockwipe.hxx"
#include "pinwheelwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon PinWheelWipe::operator () ( double t )
{
    ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock(
                                    t / m_blades,
                                    2.0 /* max edge when rotating */ ) );
    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 i = m_blades; i--; )
    {
        ::basegfx::B2DPolygon p(poly);
        p.transform(basegfx::tools::createRotateB2DHomMatrix((i * 2.0 * M_PI) / m_blades));
        res.append( p );
    }
    res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));
    return res;
}

}
}
