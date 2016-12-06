/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_BASEGFX_TOOLS_UNOTOOLS_HXX
#define INCLUDED_BASEGFX_TOOLS_UNOTOOLS_HXX

#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
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

    BASEGFX_DLLPUBLIC B2DPolyPolygon polyPolygonBezierToB2DPolyPolygon(const css::drawing::PolyPolygonBezierCoords& rSourcePolyPolygon)
        throw( css::lang::IllegalArgumentException );

    BASEGFX_DLLPUBLIC void b2DPolyPolygonToPolyPolygonBezier( const B2DPolyPolygon& rPolyPoly,
                                            css::drawing::PolyPolygonBezierCoords& rRetval );
}
}

#endif // INCLUDED_BASEGFX_TOOLS_UNOTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
