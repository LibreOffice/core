/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: clockwipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:50:22 $
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
 ************************************************************************/

#include "canvas/debug.hxx"
#include "clockwipe.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/point/b2dpoint.hxx"
#include "basegfx/numeric/ftools.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolygon ClockWipe::calcCenteredClock( double t, double e )
{
    ::basegfx::B2DPolygon poly;
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.rotate( t * F_2PI );
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
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( 0.5, 0.5 );
    aTransform.translate( 0.5, 0.5 );
    ::basegfx::B2DPolygon poly( calcCenteredClock(t) );
    poly.transform( aTransform );
    return ::basegfx::B2DPolyPolygon(poly);
}

}
}
