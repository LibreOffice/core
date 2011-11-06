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
#include <basegfx/point/b2dpoint.hxx>
#include "veewipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon VeeWipe::operator () ( double t )
{
    ::basegfx::B2DPolygon poly;
    poly.append( ::basegfx::B2DPoint( 0.0, -1.0 ) );
    const double d = ::basegfx::pruneScaleValue( 2.0 * t );
    poly.append( ::basegfx::B2DPoint( 0.0, d - 1.0 ) );
    poly.append( ::basegfx::B2DPoint( 0.5, d ) );
    poly.append( ::basegfx::B2DPoint( 1.0, d - 1.0 ) );
    poly.append( ::basegfx::B2DPoint( 1.0, -1.0 ) );
    poly.setClosed(true);
    return ::basegfx::B2DPolyPolygon( poly );
}

}
}
