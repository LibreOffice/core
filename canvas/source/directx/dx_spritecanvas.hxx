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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SPRITECANVAS_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SPRITECANVAS_HXX

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/spritesurface.hxx>
#include <canvas/base/disambiguationhelper.hxx>
#include <canvas/base/bufferedgraphicdevicebase.hxx>

#include "dx_bitmapprovider.hxx"
#include "dx_spritecanvashelper.hxx"
#include "dx_surfacebitmap.hxx"
#include "dx_impltools.hxx"
#include "dx_spritedevicehelper.hxx"


namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XSpriteCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::rendering::XGraphicDevice,
                                             css::lang::XMultiServiceFactory,
                                             css::rendering::XBufferController,
                                             css::awt::XWindowListener,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet,
                                             css::lang::XServiceName >    WindowGraphicDeviceBase_Base;
    typedef ::canvas::BufferedGraphicDeviceBase< ::canvas::DisambiguationHelper< WindowGraphicDeviceBase_Base >,
                                                   SpriteDeviceHelper,
                                                   ::osl::MutexGuard,
                                                   ::cppu::OWeakObject >    SpriteCanvasBase_Base;
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
                                               public ::canvas::SpriteSurface
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
    class SpriteCanvas : public SpriteCanvasBaseT, public BitmapProvider
    {
    public:
        SpriteCanvas( const css::uno::Sequence<
                            css::uno::Any >&               aArguments,
                      const css::uno::Reference<
                            css::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( SpriteCanvas, WindowGraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XBufferController (partial)
        virtual sal_Bool SAL_CALL showBuffer( sal_Bool bUpdateAll ) override;
        virtual sal_Bool SAL_CALL switchBuffer( sal_Bool bUpdateAll ) override;

        // XSpriteCanvas (partial)
        virtual sal_Bool SAL_CALL updateScreen( sal_Bool bUpdateAll ) override;

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) override;

        /// Retrieve rendermodule object for this Canvas
        const IDXRenderModuleSharedPtr& getRenderModule() const;

        /// Get backbuffer for this canvas
        const DXSurfaceBitmapSharedPtr& getBackBuffer() const;

        // BitmapProvider
        virtual IBitmapSharedPtr getBitmap() const override;

     private:
        css::uno::Sequence< css::uno::Any >                maArguments;
        css::uno::Reference< css::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< SpriteCanvas > SpriteCanvasRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
