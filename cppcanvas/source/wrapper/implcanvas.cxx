/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implcanvas.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:43:15 $
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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <com/sun/star/rendering/XCanvas.hpp>
#endif

#include <canvas/canvastools.hxx>

#include <cppcanvas/polypolygon.hxx>
#include <implfont.hxx>
#include <implcolor.hxx>

#include <implcanvas.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        ImplCanvas::ImplCanvas( const uno::Reference< rendering::XCanvas >& xCanvas ) :
            maViewState(),
            maClipPolyPolygon(),
            mxCanvas( xCanvas )
        {
            OSL_ENSURE( mxCanvas.is(), "Canvas::Canvas() invalid XCanvas" );

            ::canvas::tools::initViewState( maViewState );
        }

        ImplCanvas::~ImplCanvas()
        {
        }

        void ImplCanvas::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            ::canvas::tools::setViewStateTransform( maViewState, rMatrix );
        }

        ::basegfx::B2DHomMatrix ImplCanvas::getTransformation() const
        {
            ::basegfx::B2DHomMatrix aMatrix;
            return ::canvas::tools::getViewStateTransform( aMatrix,
                                                           maViewState );
        }

        void ImplCanvas::setClip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            // TODO(T3): not thread-safe. B2DPolyPolygon employs copy-on-write
            maClipPolyPolygon = rClipPoly;
            maViewState.Clip.clear();
        }

        ::basegfx::B2DPolyPolygon ImplCanvas::getClip() const
        {
            return maClipPolyPolygon;
        }

        FontSharedPtr ImplCanvas::createFont( const ::rtl::OUString& rFontName, const double& rCellSize ) const
        {
            return FontSharedPtr( new ImplFont( getUNOCanvas(), rFontName, rCellSize ) );
        }

        ColorSharedPtr ImplCanvas::createColor() const
        {
            return ColorSharedPtr( new ImplColor( getUNOCanvas()->getDevice() ) );
        }

        CanvasSharedPtr ImplCanvas::clone() const
        {
            return CanvasSharedPtr( new ImplCanvas( *this ) );
        }

        uno::Reference< rendering::XCanvas > ImplCanvas::getUNOCanvas() const
        {
            OSL_ENSURE( mxCanvas.is(), "ImplCanvas::getUNOCanvas(): Invalid XCanvas" );

            return mxCanvas;
        }

        rendering::ViewState ImplCanvas::getViewState() const
        {
            if( maClipPolyPolygon.count() && !maViewState.Clip.is() )
            {
                if( !mxCanvas.is() )
                    return maViewState;

                maViewState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    maClipPolyPolygon );
            }

            return maViewState;
        }

    }
}
