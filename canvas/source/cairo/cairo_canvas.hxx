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

#pragma once

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <base/basemutexhelper.hxx>
#include <base/bitmapcanvasbase.hxx>
#include <base/graphicdevicebase.hxx>
#include <base/integerbitmapbase.hxx>

#include "cairo_canvashelper.hxx"
#include "cairo_devicehelper.hxx"
#include "cairo_repainttarget.hxx"
#include "cairo_surfaceprovider.hxx"

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::rendering::XGraphicDevice,
                                             css::lang::XMultiServiceFactory,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet,
                                             css::lang::XServiceName,
                                             css::lang::XServiceInfo >  GraphicDeviceBase_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >,
                                                 DeviceHelper,
                                                 ::osl::MutexGuard,
                                                 ::cppu::OWeakObject > CanvasBase_Base;

    /** Mixin SurfaceProvider

        Have to mixin the SurfaceProvider before deriving from
        ::canvas::CanvasBase, as this template should already
        implement some of those interface methods.

        The reason why this appears kinda convoluted is the fact that
        we cannot specify non-IDL types as WeakComponentImplHelper
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

    typedef ::canvas::IntegerBitmapBase<
        canvas::BitmapCanvasBase2<
            CanvasBaseSurfaceProvider_Base,
            CanvasHelper,
            ::osl::MutexGuard,
            ::cppu::OWeakObject> > CanvasBaseT;

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
        Canvas( const css::uno::Sequence< css::uno::Any >&               aArguments,
                const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        void initialize();

        /// For resource tracking
        virtual ~Canvas() override;

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                          Classname     Base doing refcounting        Base implementing the XComponent interface
        //                            |                 |                            |
        //                            V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( Canvas, GraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) override;

        //  XServiceInfo
        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // RepaintTarget
        virtual bool repaint( const ::cairo::SurfaceSharedPtr& pSurface,
                  const css::rendering::ViewState& viewState,
                  const css::rendering::RenderState&   renderState ) override;

        // SurfaceProvider
        virtual ::cairo::SurfaceSharedPtr getSurface() override;
        virtual ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, int aContent ) override;
        virtual ::cairo::SurfaceSharedPtr createSurface( ::Bitmap& rBitmap ) override;
        virtual ::cairo::SurfaceSharedPtr changeSurface() override;
        virtual OutputDevice* getOutputDevice() override;

     private:
        css::uno::Sequence< css::uno::Any >                maArguments;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
