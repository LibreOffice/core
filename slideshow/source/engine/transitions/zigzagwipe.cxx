/*************************************************************************
 *
 *  $RCSfile: zigzagwipe.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:11:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
