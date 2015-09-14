/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASFONT_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASFONT_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>


/* Definition of CanvasFont class */

namespace oglcanvas
{
    class SpriteCanvas;

    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvasFont > CanvasFontBaseT;

    class CanvasFont : public ::comphelper::OBaseMutex,
                       public CanvasFontBaseT,
                       private ::boost::noncopyable
    {
    public:
        typedef rtl::Reference<CanvasFont> ImplRef;

        CanvasFont( const css::rendering::FontRequest&                                     fontRequest,
                    const css::uno::Sequence< css::beans::PropertyValue >&    extraFontProperties,
                    const css::geometry::Matrix2D&                                         fontMatrix );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // XCanvasFont
        virtual css::uno::Reference< css::rendering::XTextLayout > SAL_CALL createTextLayout( const css::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::rendering::FontRequest SAL_CALL getFontRequest(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::rendering::FontMetrics SAL_CALL getFontMetrics(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        const css::geometry::Matrix2D& getFontMatrix() const { return maFontMatrix; }

    private:
        css::rendering::FontRequest    maFontRequest;
        css::geometry::Matrix2D        maFontMatrix;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
