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
 * ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
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
    res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(t, t, 0.5, 0.5));
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
