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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_CANVAS_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_CANVAS_HXX

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
#include <comphelper/types.hxx>
#include <cppu/unotype.hxx>

#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/bitmapcanvasbase.hxx>
#include <canvas/base/integerbitmapbase.hxx>
#include <canvas/base/graphicdevicebase.hxx>

#include "canvashelper.hxx"
#include "impltools.hxx"
#include "devicehelper.hxx"
#include "repainttarget.hxx"

#define CANVAS_SERVICE_NAME        "com.sun.star.rendering.Canvas.VCL"
#define CANVAS_IMPLEMENTATION_NAME "com.sun.star.comp.rendering.Canvas.VCL"

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::rendering::XGraphicDevice,
                                             css::lang::XMultiServiceFactory,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet,
                                             css::lang::XServiceName >    GraphicDeviceBase_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >,
                                           DeviceHelper,
                                           tools::LocalGuard,
                                           ::cppu::OWeakObject >    CanvasBase_Base;
    typedef ::canvas::IntegerBitmapBase<
        canvas::BitmapCanvasBase2<
            CanvasBase_Base,
            CanvasHelper,
            tools::LocalGuard,
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
        Canvas( const css::uno::Sequence<
                      css::uno::Any >&               aArguments,
                const css::uno::Reference<
                      css::uno::XComponentContext >& rxContext );

        void initialize();

        /// For resource tracking
        virtual ~Canvas() override;

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( Canvas,   GraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) override;

        // RepaintTarget
        virtual bool repaint( const GraphicObjectSharedPtr&                 rGrf,
                              const css::rendering::ViewState&              viewState,
                              const css::rendering::RenderState&            renderState,
                              const ::Point&                                rPt,
                              const ::Size&                                 rSz,
                              const GraphicAttr&                            rAttr ) const override;

    private:
        css::uno::Sequence< css::uno::Any >                maArguments;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
