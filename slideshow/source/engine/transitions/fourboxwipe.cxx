/*************************************************************************
 *
 *  $RCSfile: fourboxwipe.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:06:06 $
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
