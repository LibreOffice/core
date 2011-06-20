/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "clockwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolygon ClockWipe::calcCenteredClock( double t, double e )
{
    ::basegfx::B2DPolygon poly;
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.rotate( t * 2.0 * M_PI );
    const double MAX_EDGE = 2.0;
    ::basegfx::B2DPoint p( 0.0, -MAX_EDGE );
    p *= aTransform;
    poly.append( p );
    if (t >= 0.875)
        poly.append( ::basegfx::B2DPoint( -e, -e ) );
    if (t >= 0.625)
        poly.append( ::basegfx::B2DPoint( -e, e ) );
    if (t >= 0.375)
        poly.append( ::basegfx::B2DPoint( e, e ) );
    if (t >= 0.125)
        poly.append( ::basegfx::B2DPoint( e, -e ) );
    poly.append( ::basegfx::B2DPoint( 0.0, -e ) );
    poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
    poly.setClosed(true);
    return poly;
}

::basegfx::B2DPolyPolygon ClockWipe::operator () ( double t )
{
    const basegfx::B2DHomMatrix aTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));
    ::basegfx::B2DPolygon poly( calcCenteredClock(t) );
    poly.transform( aTransform );
    return ::basegfx::B2DPolyPolygon(poly);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
