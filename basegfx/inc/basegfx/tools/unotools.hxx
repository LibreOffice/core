/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Thorsten Behrens <tbehrens@novell.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_BASEGFX_UNOTOOLS_HXX
#define INCLUDED_BASEGFX_UNOTOOLS_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBezierPolyPolygon2D.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>


namespace basegfx
{
class B2DPolyPolygon;

namespace unotools
{

    B2DPolyPolygon polyPolygonBezierToB2DPolyPolygon(const ::com::sun::star::drawing::PolyPolygonBezierCoords& rSourcePolyPolygon)
        throw( ::com::sun::star::lang::IllegalArgumentException );

    void b2DPolyPolygonToPolyPolygonBezier( const B2DPolyPolygon& rPolyPoly,
                                            ::com::sun::star::drawing::PolyPolygonBezierCoords& rRetval );
}
}

#endif /* INCLUDED_BASEGFX_UNOTOOLS_HXX */
