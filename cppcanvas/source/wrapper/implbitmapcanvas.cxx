/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implbitmapcanvas.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:43:02 $
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

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XBITMAPCANVAS_HPP__
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#include <implbitmapcanvas.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplBitmapCanvas::ImplBitmapCanvas( const uno::Reference< rendering::XBitmapCanvas >& rCanvas ) :
            ImplCanvas( uno::Reference< rendering::XCanvas >(rCanvas,
                                                             uno::UNO_QUERY) ),
            mxBitmapCanvas( rCanvas ),
            mxBitmap( rCanvas,
                      uno::UNO_QUERY )
        {
            OSL_ENSURE( mxBitmapCanvas.is(), "ImplBitmapCanvas::ImplBitmapCanvas(): Invalid canvas" );
            OSL_ENSURE( mxBitmap.is(), "ImplBitmapCanvas::ImplBitmapCanvas(): Invalid bitmap" );
        }

        ImplBitmapCanvas::~ImplBitmapCanvas()
        {
        }

        ::basegfx::B2ISize ImplBitmapCanvas::getSize() const
        {
            OSL_ENSURE( mxBitmap.is(), "ImplBitmapCanvas::getSize(): Invalid canvas" );
            return ::basegfx::unotools::b2ISizeFromIntegerSize2D( mxBitmap->getSize() );
        }

        CanvasSharedPtr ImplBitmapCanvas::clone() const
        {
            return BitmapCanvasSharedPtr( new ImplBitmapCanvas( *this ) );
        }
    }
}
