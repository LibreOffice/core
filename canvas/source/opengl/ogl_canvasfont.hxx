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

#include <comphelper/implementationreference.hxx>

#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <rtl/ref.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>


/* Definition of CanvasFont class */

namespace oglcanvas
{
    class SpriteCanvas;

    typedef ::cppu::WeakComponentImplHelper1< ::com::sun::star::rendering::XCanvasFont > CanvasFontBaseT;

    class CanvasFont : public ::comphelper::OBaseMutex,
                       public CanvasFontBaseT,
                       private ::boost::noncopyable
    {
    public:
        typedef ::comphelper::ImplementationReference<
            CanvasFont,
            ::com::sun::star::rendering::XCanvasFont > ImplRef;

        CanvasFont( const ::com::sun::star::rendering::FontRequest&                                     fontRequest,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    extraFontProperties,
                    const ::com::sun::star::geometry::Matrix2D&                                         fontMatrix );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XCanvasFont
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XTextLayout > SAL_CALL createTextLayout( const ::com::sun::star::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::rendering::FontRequest SAL_CALL getFontRequest(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::rendering::FontMetrics SAL_CALL getFontMetrics(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        const ::com::sun::star::geometry::Matrix2D& getFontMatrix() const;

    private:
        ::com::sun::star::rendering::FontRequest    maFontRequest;
        ::com::sun::star::geometry::Matrix2D        maFontMatrix;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
