/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CAIROCANVAS_CANVASBITMAP_HXX
#define _CAIROCANVAS_CANVASBITMAP_HXX

#include <cppuhelper/compbase4.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <comphelper/uno3.hxx>

#include <basegfx/vector/b2isize.hxx>

#include <boost/shared_ptr.hpp>

#include <canvas/base/integerbitmapbase.hxx>

#include "cairo_cairo.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_repainttarget.hxx"
#include "cairo_spritecanvas.hxx"


/* Definition of CanvasBitmap class */

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::rendering::XBitmapCanvas,
                                              ::com::sun::star::rendering::XIntegerBitmap,
                                              ::com::sun::star::lang::XServiceInfo,
                                              ::com::sun::star::beans::XFastPropertySet >   CanvasBitmapBase_Base;
    class CanvasBitmapSpriteSurface_Base :
        public ::canvas::DisambiguationHelper<CanvasBitmapBase_Base>,
        public SurfaceProvider
    {
    };

    typedef ::canvas::IntegerBitmapBase<
          CanvasBitmapSpriteSurface_Base,
          CanvasHelper,
          ::osl::MutexGuard,
          ::cppu::OWeakObject >                         CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base,
                         public RepaintTarget
    {
    public:
        /** Create a canvas bitmap for the given surface

            @param rSize
            Size of the bitmap

            @param rDevice
            Reference device, with which bitmap should be compatible
        */
        CanvasBitmap( const ::basegfx::B2ISize& rSize,
                      const SurfaceProviderRef& rDevice,
                      ::com::sun::star::rendering::XGraphicDevice* pDevice,
                      bool                      bHasAlpha );

        /// Dispose all internal references
        virtual void disposeThis();

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( CanvasBitmap, CanvasBitmapBase_Base, ::cppu::WeakComponentImplHelperBase );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // SurfaceProvider
        virtual SurfaceSharedPtr getSurface();
        virtual SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, Content aContent = CAIRO_CONTENT_COLOR_ALPHA );
        virtual SurfaceSharedPtr createSurface( ::Bitmap& rBitmap );
        virtual SurfaceSharedPtr changeSurface( bool bHasAlpha, bool bCopyContent );
        virtual OutputDevice* getOutputDevice();

        // RepaintTarget
        virtual bool repaint( const SurfaceSharedPtr&                         pSurface,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState );

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
        SurfaceProviderRef        mpSurfaceProvider;
        ::cairo::SurfaceSharedPtr mpBufferSurface;
        ::cairo::CairoSharedPtr   mpBufferCairo;

        const ::basegfx::B2ISize  maSize;
        const bool                mbHasAlpha;
    };
}

#endif /* _CAIROCANVAS_CANVASBITMAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
