/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: barndoorwipe.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:37:11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include "canvas/debug.hxx"
#include "barndoorwipe.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon BarnDoorWipe::operator () ( double t )
{
    if (m_doubled)
        t /= 2.0;

    ::basegfx::B2DHomMatrix aTransform;
    aTransform.translate( -0.5, -0.5 );
    aTransform.scale( ::basegfx::pruneScaleValue(t), 1.0 );
    aTransform.translate( 0.5, 0.5 );
    ::basegfx::B2DPolygon poly( m_unitRect );
    poly.transform( aTransform );
    ::basegfx::B2DPolyPolygon res(poly);

    if (m_doubled) {
        aTransform.identity();
        aTransform.translate( -0.5, -0.5 );
        aTransform.rotate( F_PI2 );
        aTransform.translate( 0.5, 0.5 );
        poly.transform( aTransform );
        res.append(poly);
    }

    return res;
}

}
}
