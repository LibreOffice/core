/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include "./base/basemutexhelper.hxx"
#include "./base/bitmapcanvasbase.hxx"
#include "./base/graphicdevicebase.hxx"

#include "canvashelper.hxx"
#include "impltools.hxx"
#include "devicehelper.hxx"
#include "repainttarget.hxx"

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::rendering::XGraphicDevice,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet >    GraphicDeviceBase_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >,
                                           DeviceHelper,
                                           vclcanvastools::LocalGuard,
                                           ::cppu::OWeakObject >    CanvasBase_Base;
    typedef canvas::BitmapCanvasBase<
            CanvasBase_Base,
            CanvasHelper,
            vclcanvastools::LocalGuard,
            ::cppu::OWeakObject> CanvasBaseT;

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
        Canvas( sal_Int64 nOutDev );

        /// For resource tracking
        virtual ~Canvas() override;

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        virtual void acquire() noexcept override { GraphicDeviceBase_Base::acquire(); }   \
        virtual void release() noexcept override { GraphicDeviceBase_Base::release(); }   \
        virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override \
            { return GraphicDeviceBase_Base::queryInterface(_rType); }                               \
        virtual void dispose() override \
        {                                                                               \
            ::cppu::WeakComponentImplHelperBase::dispose();                                                      \
        }                                                                               \
        virtual void addEventListener(                                         \
            css::uno::Reference< css::lang::XEventListener > const & xListener ) override \
        {                                                                               \
            ::cppu::WeakComponentImplHelperBase::addEventListener(xListener);                                        \
        }                                                                               \
        virtual void removeEventListener(                                      \
            css::uno::Reference< css::lang::XEventListener > const & xListener ) override \
        {                                                                               \
            ::cppu::WeakComponentImplHelperBase::removeEventListener(xListener);                                 \
        }

        // RepaintTarget
        virtual bool repaint( const GraphicObjectSharedPtr&                 rGrf,
                              const css::rendering::ViewState&              viewState,
                              const css::rendering::RenderState&            renderState,
                              const ::Point&                                rPt,
                              const ::Size&                                 rSz,
                              const GraphicAttr&                            rAttr ) const override;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
