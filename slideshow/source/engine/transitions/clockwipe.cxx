/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));
    ::basegfx::B2DPolygon poly( calcCenteredClock(t) );
    poly.transform( aTransform );
    return ::basegfx::B2DPolyPolygon(poly);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
