/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pinwheelwipe.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:43:57 $
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

#include <canvas/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include "clockwipe.hxx"
#include "pinwheelwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon PinWheelWipe::operator () ( double t )
{
    ::basegfx::B2DHomMatrix aTransform;
    ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock(
                                    t / m_blades,
                                    2.0 /* max edge when rotating */ ) );
    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 i = m_blades; i--; )
    {
        aTransform.identity();
        aTransform.rotate( (i * 2.0 * M_PI) / m_blades );
        ::basegfx::B2DPolygon p(poly);
        p.transform( aTransform );
        res.append( p );
    }
    aTransform.identity();
    aTransform.scale( 0.5, 0.5 );
    aTransform.translate( 0.5, 0.5 );
    res.transform( aTransform );
    return res;
}

}
}
