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

#ifndef INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITECANVAS_HXX
#define INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITECANVAS_HXX

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

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/disambiguationhelper.hxx>
#include <canvas/base/bufferedgraphicdevicebase.hxx>

#include <basegfx/vector/b2isize.hxx>

#include "cairo_spritedevicehelper.hxx"
#include "cairo_repainttarget.hxx"
#include "cairo_surfaceprovider.hxx"
#include "cairo_spritecanvashelper.hxx"

#define SPRITECANVAS_SERVICE_NAME        "com.sun.star.rendering.SpriteCanvas.Cairo"
#define SPRITECANVAS_IMPLEMENTATION_NAME "com.sun.star.comp.rendering.SpriteCanvas.Cairo"

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper< ::com::sun::star::rendering::XSpriteCanvas,
                                                ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::rendering::XGraphicDevice,
                                                ::com::sun::star::lang::XMultiServiceFactory,
                                                ::com::sun::star::rendering::XBufferController,
                                                ::com::sun::star::awt::XWindowListener,
                                                ::com::sun::star::util::XUpdatable,
                                                ::com::sun::star::beans::XPropertySet,
                                                ::com::sun::star::lang::XServiceName >  WindowGraphicDeviceBase_Base;
    typedef ::canvas::BufferedGraphicDeviceBase< ::canvas::DisambiguationHelper< WindowGraphicDeviceBase_Base >,
                                                 SpriteDeviceHelper,
                                                 ::osl::MutexGuard,
                                                 ::cppu::OWeakObject > SpriteCanvasBase_Base;
    /** Mixin SpriteSurface

        Have to mixin the SpriteSurface before deriving from
        ::canvas::SpriteCanvasBase, as this template should already
        implement some of those interface methods.

        The reason why this appears kinda convoluted is the fact that
        we cannot specify non-IDL types as WeakComponentImplHelper
        template args, and furthermore, don't want to derive
        ::canvas::SpriteCanvasBase directly from
        ::canvas::SpriteSurface (because derivees of
        ::canvas::SpriteCanvasBase have to explicitly forward the
        XInterface methods (e.g. via DECLARE_UNO3_AGG_DEFAULTS)
        anyway). Basically, ::canvas::CanvasCustomSpriteBase should
        remain a base class that provides implementation, not to
        enforce any specific interface on its derivees.
     */
    class SpriteCanvasBaseSpriteSurface_Base : public SpriteCanvasBase_Base,
                                               public ::canvas::SpriteSurface,
                                               public SurfaceProvider
    {
    };

    typedef ::canvas::SpriteCanvasBase< SpriteCanvasBaseSpriteSurface_Base,
                                        SpriteCanvasHelper,
                                        ::osl::MutexGuard,
                                        ::cppu::OWeakObject >           SpriteCanvasBaseT;

    /** Product of this component's factory.

        The SpriteCanvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class SpriteCanvas : public SpriteCanvasBaseT,
                         public RepaintTarget
    {
    public:
        SpriteCanvas( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Any >&               aArguments,
                      const ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void disposeThis() SAL_OVERRIDE;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( SpriteCanvas, WindowGraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XBufferController (partial)
        virtual sal_Bool SAL_CALL showBuffer( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL switchBuffer( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

        // XSpriteCanvas (partial)
        virtual sal_Bool SAL_CALL updateScreen( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // SurfaceProvider
        virtual ::cairo::SurfaceSharedPtr getSurface() SAL_OVERRIDE;
        virtual ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, int aContent = CAIRO_CONTENT_COLOR_ALPHA ) SAL_OVERRIDE;
        virtual ::cairo::SurfaceSharedPtr createSurface( ::Bitmap& rBitmap ) SAL_OVERRIDE;
        virtual ::cairo::SurfaceSharedPtr changeSurface( bool bHasAlpha, bool bCopyContent ) SAL_OVERRIDE;
        virtual OutputDevice* getOutputDevice() SAL_OVERRIDE;

        // RepaintTarget
        virtual bool repaint( const ::cairo::SurfaceSharedPtr&                pSurface,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState ) SAL_OVERRIDE;

        ::cairo::SurfaceSharedPtr getWindowSurface();
        ::cairo::SurfaceSharedPtr getBufferSurface();

        const ::basegfx::B2ISize& getSizePixel();
        void setSizePixel( const ::basegfx::B2ISize& rSize );
        void flush();

     private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > maArguments;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< SpriteCanvas > SpriteCanvasRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
