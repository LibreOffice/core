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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "transitiontools.hxx"
#include "clockwipe.hxx"
#include "fanwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon FanWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    ::basegfx::B2DPolygon poly(
        ClockWipe::calcCenteredClock(
            t / ((m_center && m_single) ? 2.0 : 4.0) ) );

    res.append( poly );
    // flip on y-axis:
    poly.transform(basegfx::tools::createScaleB2DHomMatrix(-1.0, 1.0));
    poly.flip();
    res.append( poly );

    if (m_center)
    {
        res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

        if (! m_single)
            res.append( flipOnXAxis(res) );
    }
    else
    {
        OSL_ASSERT( ! m_fanIn );
        res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 1.0, 0.5, 1.0));
    }
    return res;
}

}
}

