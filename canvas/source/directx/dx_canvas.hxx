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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVAS_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVAS_HXX

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/integerbitmapbase.hxx>
#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/graphicdevicebase.hxx>
#include <canvas/base/canvasbase.hxx>
#include <canvas/base/bitmapcanvasbase.hxx>

#include "dx_bitmapprovider.hxx"
#include "dx_canvashelper.hxx"
#include "dx_bitmapcanvashelper.hxx"
#include "dx_impltools.hxx"
#include "dx_devicehelper.hxx"


namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvas,
                                             css::rendering::XGraphicDevice,
                                             css::lang::XMultiServiceFactory,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet,
                                             css::lang::XServiceName >    GraphicDeviceBase1_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase1_Base >,
                                           DeviceHelper,
                                           ::osl::MutexGuard,
                                           ::cppu::OWeakObject >    CanvasBase1_Base;
    typedef ::canvas::CanvasBase< CanvasBase1_Base,
                                  CanvasHelper,
                                  ::osl::MutexGuard,
                                  ::cppu::OWeakObject >     CanvasBaseT;

    /** Product of this component's factory.

        The Canvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class Canvas : public CanvasBaseT
    {
    public:
        Canvas( const css::uno::Sequence<
                      css::uno::Any >&               aArguments,
                const css::uno::Reference<
                      css::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname   Base doing refcounting    Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( Canvas, GraphicDeviceBase1_Base, ::cppu::WeakComponentImplHelperBase )

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) override;

    private:
        css::uno::Sequence< css::uno::Any >                maArguments;
        css::uno::Reference< css::uno::XComponentContext > mxComponentContext;
    };

    typedef ::cppu::WeakComponentImplHelper< css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::rendering::XGraphicDevice,
                                             css::lang::XMultiServiceFactory,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet,
                                             css::lang::XServiceName >    GraphicDeviceBase2_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase2_Base >,
                                           DeviceHelper,
                                           ::osl::MutexGuard,
                                           ::cppu::OWeakObject >    CanvasBase2_Base;
    typedef ::canvas::IntegerBitmapBase<
        canvas::BitmapCanvasBase2<
            CanvasBase2_Base,
            BitmapCanvasHelper,
            ::osl::MutexGuard,
            ::cppu::OWeakObject> > BitmapCanvasBaseT;

    /** Product of this component's factory.

        The Canvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class BitmapCanvas : public BitmapCanvasBaseT, public BitmapProvider
    {
    public:
        BitmapCanvas( const css::uno::Sequence< css::uno::Any >&                aArguments,
                      const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname   Base doing refcounting    Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( BitmapCanvas, GraphicDeviceBase2_Base, ::cppu::WeakComponentImplHelperBase )

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) override;

        // BitmapProvider
        virtual IBitmapSharedPtr getBitmap() const override;

     private:
        css::uno::Sequence< css::uno::Any >                maArguments;
        css::uno::Reference< css::uno::XComponentContext > mxComponentContext;
        IBitmapSharedPtr                                                             mpTarget;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
