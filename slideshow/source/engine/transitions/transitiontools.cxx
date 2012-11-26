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

#include "transitiontools.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


namespace slideshow {
namespace internal {

// TODO(Q2): Move this to basegfx
::basegfx::B2DPolygon createUnitRect()
{
    return ::basegfx::tools::createUnitPolygon();
}

::basegfx::B2DPolyPolygon flipOnYAxis(
    ::basegfx::B2DPolyPolygon const & polypoly )
{
    ::basegfx::B2DPolyPolygon res(polypoly);
    res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(-1.0, 1.0, 1.0, 0.0));
    res.flip();
    return res;
}

::basegfx::B2DPolyPolygon flipOnXAxis(
    ::basegfx::B2DPolyPolygon const & polypoly )
{
    ::basegfx::B2DPolyPolygon res(polypoly);
    res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(1.0, -1.0, 0.0, 1.0));
    res.flip();
    return res;
}

}
}
