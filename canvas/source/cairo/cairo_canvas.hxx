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

#ifndef _CAIROCANVAS_CANVAS_HXX_
#define _CAIROCANVAS_CANVAS_HXX_

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <cppuhelper/compbase7.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/disambiguationhelper.hxx>
#include <canvas/base/bufferedgraphicdevicebase.hxx>

#include <basegfx/vector/b2isize.hxx>

#include "cairo_devicehelper.hxx"
#include "cairo_repainttarget.hxx"
#include "cairo_surfaceprovider.hxx"
#include "cairo_spritecanvashelper.hxx"

#define CANVAS_SERVICE_NAME        "com.sun.star.rendering.Canvas.Cairo"
#define CANVAS_IMPLEMENTATION_NAME "com.sun.star.comp.rendering.Canvas.Cairo"

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper7< ::com::sun::star::rendering::XBitmapCanvas,
                                                ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::rendering::XGraphicDevice,
                                                ::com::sun::star::lang::XMultiServiceFactory,
                                                ::com::sun::star::util::XUpdatable,
                                                ::com::sun::star::beans::XPropertySet,
                                                ::com::sun::star::lang::XServiceName >  GraphicDeviceBase_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::DisambiguationHelper< GraphicDeviceBase_Base >,
                                                 DeviceHelper,
                                                 ::osl::MutexGuard,
                                                 ::cppu::OWeakObject > CanvasBase_Base;

    /** Mixin SurfaceProvider

        Have to mixin the SurfaceProvider before deriving from
        ::canvas::CanvasBase, as this template should already
        implement some of those interface methods.

        The reason why this appears kinda convoluted is the fact that
        we cannot specify non-IDL types as WeakComponentImplHelperN
        template args, and furthermore, don't want to derive
        ::canvas::CanvasBase directly from
        SurfaceProvider (because derivees of
        ::canvas::CanvasBase have to explicitly forward the
        XInterface methods (e.g. via DECLARE_UNO3_AGG_DEFAULTS)
        anyway).
     */
    class CanvasBaseSurfaceProvider_Base : public CanvasBase_Base,
                                           public SurfaceProvider
    {
    };

    typedef ::canvas::IntegerBitmapBase< CanvasBaseSurfaceProvider_Base,
                                         CanvasHelper,
                                         ::osl::MutexGuard,
                                         ::cppu::OWeakObject >          CanvasBaseT;

    /** Product of this component's factory.

        The Canvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class Canvas : public CanvasBaseT,
                   public RepaintTarget
    {
    public:
        Canvas( const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::uno::Any >&               aArguments,
                const ::com::sun::star::uno::Reference<
                  ::com::sun::star::uno::XComponentContext >& rxContext );

        void initialize();

        /// For resource tracking
        ~Canvas();

        /// Dispose all internal references
        virtual void disposeThis();

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                          Classname     Base doing refcounting        Base implementing the XComponent interface
        //                            |                 |                            |
        //                            V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( Canvas, GraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase );

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

        // RepaintTarget
        virtual bool repaint( const ::cairo::SurfaceSharedPtr& pSurface,
                  const ::com::sun::star::rendering::ViewState& viewState,
                  const ::com::sun::star::rendering::RenderState&   renderState );

        // SurfaceProvider
        virtual SurfaceSharedPtr getSurface();
        virtual SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, Content aContent = CAIRO_CONTENT_COLOR_ALPHA );
        virtual SurfaceSharedPtr createSurface( ::Bitmap& rBitmap );
        virtual SurfaceSharedPtr changeSurface( bool bHasAlpha, bool bCopyContent );
        virtual OutputDevice* getOutputDevice();

     private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >                maArguments;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< Canvas > CanvasRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
