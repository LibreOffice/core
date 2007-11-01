/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_linepolypolygon.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:56:02 $
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

#include <basegfx/tools/canvastools.hxx>
#include "dx_linepolypolygon.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    LinePolyPolygon::LinePolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly ) :
        ::canvas::LinePolyPolygonBase( rPolyPoly ),
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        mpPath()
    {
    }

    GraphicsPathSharedPtr LinePolyPolygon::getGraphicsPath() const
    {
        // generate GraphicsPath only on demand (gets deleted as soon
        // as any of the modifying methods above touches the
        // B2DPolyPolygon).
        if( !mpPath )
            mpPath = tools::graphicsPathFromB2DPolyPolygon( getPolyPolygon() );

        return mpPath;
    }

    void LinePolyPolygon::modifying() const
    {
        mpPath.reset();
    }
}
