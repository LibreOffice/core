/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: figurewipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:53:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 * ************************************************************************/

#include "canvas/debug.hxx"
#include "transitiontools.hxx"
#include "figurewipe.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/point/b2dpoint.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon FigureWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res(m_figure);
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( t, t );
    aTransform.translate( 0.5, 0.5 );
    res.transform( aTransform );
    return res;
}

FigureWipe * FigureWipe::createTriangleWipe()
{
    const double s60 = sin( 60.0 * F_PI180 );
    const double s30 = sin( 30.0 * F_PI180 );
    ::basegfx::B2DPolygon figure;
    figure.append( ::basegfx::B2DPoint( 0.5 + s30, 0.5 ) );
    figure.append( ::basegfx::B2DPoint( 0.0, -0.5 - s60 ) );
    figure.append( ::basegfx::B2DPoint( -0.5 - s30, 0.5 ) );
    figure.setClosed(true);
    return new FigureWipe(figure);
}

FigureWipe * FigureWipe::createArrowHeadWipe()
{
    const double s60 = sin( 60.0 * F_PI180 );
    const double s30 = sin( 30.0 * F_PI180 );
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
    const double s = sin( 18.0 * F_PI180 );
    const double c = cos( 18.0 * F_PI180 );
    ::basegfx::B2DPolygon figure;
    figure.append( ::basegfx::B2DPoint( 0.5, 0.5 ) );
    figure.append( ::basegfx::B2DPoint( 0.5 + s, 0.5 - c ) );
    figure.append( ::basegfx::B2DPoint( 0.0, 0.5 - c - sin(36.0 * F_PI180) ) );
    figure.append( ::basegfx::B2DPoint( -0.5 - s, 0.5 - c ) );
    figure.append( ::basegfx::B2DPoint( -0.5, 0.5 ) );
    figure.setClosed(true);
    return new FigureWipe(figure);
}

FigureWipe * FigureWipe::createHexagonWipe()
{
    const double s = sin( 30.0 * F_PI180 );
    const double c = cos( 30.0 * F_PI180 );
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
    const double v = (F_PI / nPoints);
    const ::basegfx::B2DPoint p_( 0.0, -M_SQRT2 );
    ::basegfx::B2DPolygon figure;
    for ( sal_Int32 pos = 0; pos < nPoints; ++pos ) {
        const double w = (pos * F_2PI / nPoints);
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
