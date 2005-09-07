/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fanwipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:52:32 $
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
    if (m_center) {
        ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock(
                                        t / (m_single ? 2.0 : 4.0) ) );
        res.append( poly );
        // flip on y-axis:
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( -1.0, 1.0 );
        poly.transform( aTransform );
        poly.flip();
        res.append( poly );

        aTransform.identity();
        aTransform.scale( 0.5, 0.5 );
        aTransform.translate( 0.5, 0.5 );
        res.transform( aTransform );

        if (! m_single) {
            res.append( flipOnXAxis(res) );
            if (m_fanIn) {
                // xxx todo
            }
        }
    }
    else {
        OSL_ASSERT( !m_single && !m_fanIn );
        ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock( t / 4.0 ) );
        res.append( poly );
        // flip on y-axis:
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( -1.0, 1.0 );
        poly.transform( aTransform );
        poly.flip();
        res.append( poly );

        aTransform.identity();
        aTransform.scale( 0.5, 1.0 );
        aTransform.translate( 0.5, 1.0 );
        res.transform( aTransform );
    }
    return res;
}

}
}

// xxx todo: tests
//     switch (n % 4) {
//     case 0:
//         nType = DOUBLEFANWIPE;
//         nSubType = FANOUTVERTICAL;
//         break;
//     case 1:
//         nType = DOUBLEFANWIPE;
//         nSubType = FANOUTHOR
//         break;
//     case 2:
//         nType = DOUBLEFANWIPE;
//         nSubType = FANINVERTICAL;
//         break;
//     case 3:
//         nType = DOUBLEFANWIPE;
//         nSubType = FANINHORIZONTAL;
//         break;
