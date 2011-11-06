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



#ifndef _SVX_UNOPOLYHELPER_HXX
#define _SVX_UNOPOLYHELPER_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <svx/svxdllapi.h>

namespace com { namespace sun { namespace star { namespace drawing {
    struct PolyPolygonBezierCoords;
} } } }

namespace basegfx {
    class B2DPolyPolygon;
}

/** convert a drawing::PolyPolygonBezierCoords to a B2DPolyPolygon
*/
basegfx::B2DPolyPolygon SvxConvertPolyPolygonBezierToB2DPolyPolygon( const com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon)
    throw( com::sun::star::lang::IllegalArgumentException );

/** convert a B2DPolyPolygon to a drawing::PolyPolygonBezierCoords
*/
SVX_DLLPUBLIC void SvxConvertB2DPolyPolygonToPolyPolygonBezier( const basegfx::B2DPolyPolygon& rPolyPoly, com::sun::star::drawing::PolyPolygonBezierCoords& rRetval );


#endif


