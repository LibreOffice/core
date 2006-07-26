/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fanwipe.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:37:12 $
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
#include "transitiontools.hxx"
#include "clockwipe.hxx"
#include "fanwipe.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon FanWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    ::basegfx::B2DPolygon poly(
        ClockWipe::calcCenteredClock(
            t / ((m_center && m_single) ? 2.0 : 4.0) ) );

    res.append( poly );
    // flip on y-axis:
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( -1.0, 1.0 );
    poly.transform( aTransform );
    poly.flip();
    res.append( poly );
    aTransform.identity();

    if (m_center) {
        aTransform.scale( 0.5, 0.5 );
        aTransform.translate( 0.5, 0.5 );
        res.transform( aTransform );

        if (! m_single)
            res.append( flipOnXAxis(res) );
    }
    else {
        OSL_ASSERT( ! m_fanIn );
        aTransform.scale( 0.5, 1.0 );
        aTransform.translate( 0.5, 1.0 );
        res.transform( aTransform );
    }
    return res;
}

}
}

