/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_textlayout_drawhelper.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
