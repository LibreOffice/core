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
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "barndoorwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon BarnDoorWipe::operator () ( double t )
{
    if (m_doubled)
        t /= 2.0;

    basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(-0.5, -0.5));
    aTransform.scale( ::basegfx::pruneScaleValue(t), 1.0 );
    aTransform.translate( 0.5, 0.5 );
    ::basegfx::B2DPolygon poly( m_unitRect );
    poly.transform( aTransform );
    ::basegfx::B2DPolyPolygon res(poly);

    if (m_doubled) {
        aTransform = basegfx::tools::createTranslateB2DHomMatrix(-0.5, -0.5);
        aTransform.rotate( M_PI_2 );
        aTransform.translate( 0.5, 0.5 );
        poly.transform( aTransform );
        res.append(poly);
    }

    return res;
}

}
}
