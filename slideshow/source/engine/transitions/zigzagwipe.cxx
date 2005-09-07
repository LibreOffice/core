/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zigzagwipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:02:28 $
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

#include "zigzagwipe.hxx"
#include "canvas/debug.hxx"
#include "transitiontools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/point/b2dpoint.hxx"
// #include "basegfx/numeric/ftools.hxx"


namespace presentation {
namespace internal {

ZigZagWipe::ZigZagWipe( sal_Int32 nZigs ) : m_zigEdge( 1.0 / nZigs )
{
    const double d = m_zigEdge;
    const double d2 = (d / 2.0);
    m_stdZigZag.append( ::basegfx::B2DPoint( -1.0 - d, -d ) );
    m_stdZigZag.append( ::basegfx::B2DPoint( -1.0 - d, 1.0 + d ) );
    m_stdZigZag.append( ::basegfx::B2DPoint( -d, 1.0 + d ) );
    for ( sal_Int32 pos = (nZigs + 2); pos--; ) {
        m_stdZigZag.append( ::basegfx::B2DPoint( 0.0, ((pos - 1) * d) + d2 ) );
        m_stdZigZag.append( ::basegfx::B2DPoint( -d, (pos - 1) * d ) );
    }
    m_stdZigZag.setClosed(true);
}

::basegfx::B2DPolyPolygon ZigZagWipe::operator () ( double t )
{
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.translate( (1.0 + m_zigEdge) * t, 0.0 );
    ::basegfx::B2DPolyPolygon res(m_stdZigZag);
    res.transform( aTransform );
    return res;
}

::basegfx::B2DPolyPolygon BarnZigZagWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolygon poly( m_stdZigZag );
    poly.flip();
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.translate( (1.0 + m_zigEdge) * (1.0 - t) / 2.0, 0.0 );
    poly.transform( aTransform );
    res.append( poly );
    aTransform.scale( -1.0, 1.0 );
    aTransform.translate( 1.0, m_zigEdge / 2.0 );
    poly = m_stdZigZag;
    poly.transform( aTransform );
    res.append( poly );
    return res;
}

}
}
