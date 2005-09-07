/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ellipsewipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:51:56 $
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
#include "ellipsewipe.hxx"
#include "transitiontools.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon EllipseWipe::operator () ( double t )
{
    // currently only circle:
    ::basegfx::B2DPolygon poly(
        ::basegfx::tools::createPolygonFromCircle(
            ::basegfx::B2DPoint( 0.5, 0.5 ),
            ::basegfx::pruneScaleValue( t * M_SQRT2 / 2.0 ) ) );
    return ::basegfx::B2DPolyPolygon( poly );
}

}
}
