/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fourboxwipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:53:37 $
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
#include "fourboxwipe.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "basegfx/numeric/ftools.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon FourBoxWipe::operator () ( double t )
{
    ::basegfx::B2DHomMatrix aTransform;
    const double d = ::basegfx::pruneScaleValue( t / 2.0 );
    if (m_cornersOut) {
        aTransform.translate( -0.5, -0.5 );
        aTransform.scale( d, d );
        aTransform.translate( -0.25, -0.25 );
    } else {
        aTransform.scale( d, d );
        aTransform.translate( -0.5, -0.5 );
    }

    // top left:
    ::basegfx::B2DPolygon square( m_unitRect );
    square.transform( aTransform );
    ::basegfx::B2DPolyPolygon res( square );
    // bottom left, flip on x-axis:
    aTransform.scale( -1.0, 1.0 );
    ::basegfx::B2DPolygon square2( m_unitRect );
    square2.transform( aTransform );
    square2.flip(); // flip direction
    res.append( square2 );
    // bottom right, flip on y-axis:
    aTransform.scale( 1.0, -1.0 );
    ::basegfx::B2DPolygon square3( m_unitRect );
    square3.transform( aTransform );
    res.append( square3 );
    // top right, flip on x-axis:
    aTransform.scale( -1.0, 1.0 );
    ::basegfx::B2DPolygon square4( m_unitRect );
    square4.transform( aTransform );
    square4.flip(); // flip direction
    res.append( square4 );

    aTransform.identity();
    aTransform.translate( 0.5, 0.5 );
    res.transform( aTransform );
    return res;
}

}
}
