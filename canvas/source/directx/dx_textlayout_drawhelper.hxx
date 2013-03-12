/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

namespace Gdiplus { class Graphics; }

namespace dxcanvas
{
    struct Bitmap;
    class TextLayoutDrawHelper
    {
    public:
        TextLayoutDrawHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >& xGraphicDevice);
        ~TextLayoutDrawHelper();

        // draw text
        void drawText( const boost::shared_ptr<Gdiplus::Graphics>&          rGraphics,
                       const ::com::sun::star::rendering::ViewState&        rViewState,
                       const ::com::sun::star::rendering::RenderState&      rRenderState,
                       const ::basegfx::B2ISize&                            rOutputOffset,
                       const ::com::sun::star::rendering::StringContext&    rText,
                       const ::com::sun::star::uno::Sequence< double >&     rLogicalAdvancements,
                       const ::com::sun::star::uno::Reference<
                               ::com::sun::star::rendering::XCanvasFont >&  rCanvasFont,
                       const ::com::sun::star::geometry::Matrix2D&          rFontMatrix,
                       bool                                                 bAlphaSurface,
                       sal_Int8                                             nTextDirection);

        ::com::sun::star::geometry::RealRectangle2D queryTextBounds(
            const ::com::sun::star::rendering::StringContext&   rText,
            const ::com::sun::star::uno::Sequence< double >&    rLogicalAdvancements,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont >&     rCanvasFont,
              const ::com::sun::star::geometry::Matrix2D&       rFontMatrix,
              sal_Int8                                          nTextDirection );

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
