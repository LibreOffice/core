/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sweepwipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:59:57 $
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
#include "sweepwipe.hxx"
#include "transitiontools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/numeric/ftools.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon SweepWipe::operator () ( double t )
{
    t /= 2.0;
    if (! m_center)
        t /= 2.0;
    if (!m_single && !m_oppositeVertical)
        t /= 2.0;

    ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock( 0.25 + t ) );
    ::basegfx::B2DHomMatrix aTransform;
    if (m_center) {
        aTransform.translate( 0.5, 0.0 );
        poly.transform( aTransform );
    }
    ::basegfx::B2DPolyPolygon res(poly);

    if (! m_single) {
        aTransform.identity();
        if (m_oppositeVertical) {
            aTransform.scale( 1.0, -1.0 );
            aTransform.translate( 0.0, 1.0 );
            poly.transform( aTransform );
            poly.flip();
        }
        else {
            aTransform.translate( -0.5, -0.5 );
            aTransform.rotate( F_PI );
            aTransform.translate( 0.5, 0.5 );
            poly.transform( aTransform );
        }
        res.append(poly);
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}
