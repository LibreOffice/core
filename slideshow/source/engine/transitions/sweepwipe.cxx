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
#include "sweepwipe.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon SweepWipe::operator () ( double t )
{
    t /= 2.0;
    if (! m_center)
        t /= 2.0;
    if (!m_single && !m_oppositeVertical)
        t /= 2.0;

    ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock( 0.25 + t ) );
    ::basegfx::B2DHomMatrix aTransform;

    if (m_center)
    {
        aTransform = basegfx::tools::createTranslateB2DHomMatrix(0.5, 0.0);
        poly.transform( aTransform );
    }
    ::basegfx::B2DPolyPolygon res(poly);

    if (! m_single)
    {
        if (m_oppositeVertical)
        {
            aTransform = basegfx::tools::createScaleB2DHomMatrix(1.0, -1.0);
            aTransform.translate( 0.0, 1.0 );
            poly.transform( aTransform );
            poly.flip();
        }
        else
        {
            aTransform = basegfx::tools::createTranslateB2DHomMatrix(-0.5, -0.5);
            aTransform.rotate( M_PI );
            aTransform.translate( 0.5, 0.5 );
            poly.transform( aTransform );
        }
        res.append(poly);
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}
