/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _VCLCANVAS_CANVASBITMAP_HXX
#define _VCLCANVAS_CANVASBITMAP_HXX

#include <cppuhelper/compbase4.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>

#include <canvas/vclwrapper.hxx>

#include <canvas/base/integerbitmapbase.hxx>
#include <canvasbitmaphelper.hxx>

#include "impltools.hxx"
#include "repainttarget.hxx"
#include "spritecanvas.hxx"


/* Definition of CanvasBitmap class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::rendering::XBitmapCanvas,
                                               ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::lang::XServiceInfo,
                                             ::com::sun::star::beans::XFastPropertySet >    CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase< ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
                                         CanvasBitmapHelper,
                                         tools::LocalGuard,
                                         ::cppu::OWeakObject >                          CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base,
                         public RepaintTarget
    {
    public:
        /** Must be called with locked Solar mutex

            @param rSize
            Size in pixel of the bitmap to generate

            @param bAlphaBitmap
            When true, bitmap will have an alpha channel

            @param rDevice
            Reference device, with which bitmap should be compatible
         */
        CanvasBitmap( const ::Size&                                rSize,
                      bool                                         bAlphaBitmap,
                      ::com::sun::star::rendering::XGraphicDevice& rDevice,
                      const OutDevProviderSharedPtr&               rOutDevProvider );

        /// Must be called with locked Solar mutex
        CanvasBitmap( const BitmapEx&                              rBitmap,
                      ::com::sun::star::rendering::XGraphicDevice& rDevice,
                      const OutDevProviderSharedPtr&               rOutDevProvider );

        // overridden because of mpDevice
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // RepaintTarget interface
        virtual bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState,
                              const ::Point&                                  rPt,
                              const ::Size&                                   rSz,
                              const GraphicAttr&                              rAttr ) const;

        /// Not threadsafe! Returned object is shared!
        BitmapEx getBitmap() const;

        // XFastPropertySet
        // used to retrieve BitmapEx pointer or X Pixmap handles for this bitmap
        // handle values have these meanings:
        // 0 ... get pointer to BitmapEx
        // 1 ... get X pixmap handle to rgb content
        // 2 ... get X pitmap handle to alpha mask
        // returned any contains either BitmapEx pointer or array of three Any value
        //     1st a bool value: true - free the pixmap after used by XFreePixmap, false do nothing, the pixmap is used internally in the canvas
        //     2nd the pixmap handle
        //     3rd the pixmap depth
        virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle)  throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFastPropertyValue(sal_Int32, const ::com::sun::star::uno::Any&)  throw (::com::sun::star::uno::RuntimeException) {}

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        ::com::sun::star::uno::Reference<com::sun::star::rendering::XGraphicDevice> mxDevice;
    };
}

#endif /* _VCLCANVAS_CANVASBITMAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
