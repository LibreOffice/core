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


#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "transitiontools.hxx"
#include "figurewipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon FigureWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res(m_figure);
    res.transform(basegfx::utils::createScaleTranslateB2DHomMatrix(t, t, 0.5, 0.5));
    return res;
}

FigureWipe * FigureWipe::createTriangleWipe()
{
    const double s60 = sin( basegfx::deg2rad(60.0) );
    const double s30 = sin( basegfx::deg2rad(30.0) );
    ::basegfx::B2DPolygon figure;
    figure.append( ::basegfx::B2DPoint( 0.5 + s30, 0.5 ) );
    figure.append( ::basegfx::B2DPoint( 0.0, -0.5 - s60 ) );
    figure.append( ::basegfx::B2DPoint( -0.5 - s30, 0.5 ) );
    figure.setClosed(true);
    return new FigureWipe(figure);
}

FigureWipe * FigureWipe::createArrowHeadWipe()
{
    const double s60 = sin( basegfx::deg2rad(60.0) );
    const double s30 = sin( basegfx::deg2rad(30.0) );
    const double off = s30;
    ::basegfx::B2DPolygon figure;
    figure.append( ::basegfx::B2DPoint( 0.5 + s30 + off, 0.5 + off ) );
    figure.append( ::basegfx::B2DPoint( 0.0, -0.5 - s60 ) );
    figure.append( ::basegfx::B2DPoint( -0.5 - s30 - off, 0.5 + off ) );
    figure.append( ::basegfx::B2DPoint( 0.0, 0.5 ) );
    figure.setClosed(true);
    return new FigureWipe(figure);
}

FigureWipe * FigureWipe::createPentagonWipe()
{
    const double s = sin( basegfx::deg2rad(18.0) );
    const double c = cos( basegfx::deg2rad(18.0) );
    ::basegfx::B2DPolygon figure;
    figure.append( ::basegfx::B2DPoint( 0.5, 0.5 ) );
    figure.append( ::basegfx::B2DPoint( 0.5 + s, 0.5 - c ) );
    figure.append( ::basegfx::B2DPoint( 0.0, 0.5 - c - sin(basegfx::deg2rad(36.0)) ) );
    figure.append( ::basegfx::B2DPoint( -0.5 - s, 0.5 - c ) );
    figure.append( ::basegfx::B2DPoint( -0.5, 0.5 ) );
    figure.setClosed(true);
    return new FigureWipe(figure);
}

FigureWipe * FigureWipe::createHexagonWipe()
{
    const double s = sin( basegfx::deg2rad(30.0) );
    const double c = cos( basegfx::deg2rad(30.0) );
    ::basegfx::B2DPolygon figure;
    figure.append( ::basegfx::B2DPoint( 0.5, c ) );
    figure.append( ::basegfx::B2DPoint( 0.5 + s, 0.0 ) );
    figure.append( ::basegfx::B2DPoint( 0.5, -c ) );
    figure.append( ::basegfx::B2DPoint( -0.5, -c ) );
    figure.append( ::basegfx::B2DPoint( -0.5 - s, 0.0 ) );
    figure.append( ::basegfx::B2DPoint( -0.5, c ) );
    figure.setClosed(true);
    return new FigureWipe(figure);
}

FigureWipe * FigureWipe::createStarWipe( sal_Int32 nPoints )
{
    const double v = (M_PI / nPoints);
    const ::basegfx::B2DPoint p_( 0.0, -M_SQRT2 );
    ::basegfx::B2DPolygon figure;
    for ( sal_Int32 pos = 0; pos < nPoints; ++pos ) {
        const double w = (pos * 2.0 * M_PI / nPoints);
        ::basegfx::B2DHomMatrix aTransform;
        ::basegfx::B2DPoint p(p_);
        aTransform.rotate( -w );
        p *= aTransform;
        figure.append(p);
        p = p_;
        aTransform.identity();
        aTransform.scale( 0.5, 0.5 );
        aTransform.rotate( -w - v );
        p *= aTransform;
        figure.append(p);
    }
    figure.setClosed(true);
    return new FigureWipe(figure);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
