/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transitiontools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:00:49 $
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

#include <transitiontools.hxx>

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#include "basegfx/matrix/b2dhommatrix.hxx"


namespace presentation {
namespace internal {

// TODO(Q2): Move this to basegfx
::basegfx::B2DPolygon createUnitRect()
{
    return ::basegfx::tools::createPolygonFromRect(
        ::basegfx::B2DRectangle(0.0,0.0,
                                1.0,1.0 ) );
}

::basegfx::B2DPolyPolygon flipOnYAxis(
    ::basegfx::B2DPolyPolygon const & polypoly )
{
    ::basegfx::B2DPolyPolygon res(polypoly);
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( -1.0, 1.0 );
    aTransform.translate( 1.0, 0.0 );
    res.transform( aTransform );
    res.flip();
    return res;
}

::basegfx::B2DPolyPolygon flipOnXAxis(
    ::basegfx::B2DPolyPolygon const & polypoly )
{
    ::basegfx::B2DPolyPolygon res(polypoly);
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( 1.0, -1.0 );
    aTransform.translate( 0.0, 1.0 );
    res.transform( aTransform );
    res.flip();
    return res;
}

}
}
