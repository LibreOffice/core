/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_textlayout_drawhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:59:39 $
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

#ifndef _TEXTLAYOUT_DRAWHELPER_HXX
#define _TEXTLAYOUT_DRAWHELPER_HXX

#include <boost/shared_ptr.hpp>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2isize.hxx>

class ::com::sun::star::rendering::XCanvasFont;

namespace dxcanvas
{
    class DXBitmap;
    class TextLayoutDrawHelper
    {
    public:
        TextLayoutDrawHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& xGraphicDevice);
        ~TextLayoutDrawHelper();

        // draw text
        void drawText( const ::boost::shared_ptr< DXBitmap > &rBitmap,
                        const ::com::sun::star::rendering::ViewState&       rViewState,
                       const ::com::sun::star::rendering::RenderState&      rRenderState,
                       const ::basegfx::B2ISize&                            rOutputOffset,
                       const ::com::sun::star::rendering::StringContext&    rText,
                       const ::com::sun::star::uno::Sequence< double >&     rLogicalAdvancements,
                       const ::com::sun::star::uno::Reference<
                               ::com::sun::star::rendering::XCanvasFont >&  rCanvasFont,
                       const ::com::sun::star::geometry::Matrix2D&          rFontMatrix );

        ::com::sun::star::geometry::RealRectangle2D queryTextBounds(
            const ::com::sun::star::rendering::StringContext&   rText,
            const ::com::sun::star::uno::Sequence< double >&    rLogicalAdvancements,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont >&     rCanvasFont,
              const ::com::sun::star::geometry::Matrix2D&       rFontMatrix );

#ifdef DBG_UTIL
        void test();
#endif

    protected:
        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XGraphicDevice > mxGraphicDevice;
    };
}

#endif /* _TEXTLAYOUT_DRAWHELPER_HXX */
// eof
