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

#ifndef _DXCANVAS_CANVAS_HXX_
#define _DXCANVAS_CANVAS_HXX_

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <cppuhelper/compbase7.hxx>
#include <cppuhelper/compbase6.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/integerbitmapbase.hxx>
#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/graphicdevicebase.hxx>

#include "dx_bitmapprovider.hxx"
#include "dx_canvashelper.hxx"
#include "dx_bitmapcanvashelper.hxx"
#include "dx_impltools.hxx"
#include "dx_devicehelper.hxx"


namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper6< ::com::sun::star::rendering::XCanvas,
                                               ::com::sun::star::rendering::XGraphicDevice,
                                              ::com::sun::star::lang::XMultiServiceFactory,
                                              ::com::sun::star::util::XUpdatable,
                                              ::com::sun::star::beans::XPropertySet,
                                              ::com::sun::star::lang::XServiceName >    GraphicDeviceBase1_Base;
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
        Canvas( const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::uno::Any >&               aArguments,
                const ::com::sun::star::uno::Reference<
                      ::com::sun::star::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname   Base doing refcounting    Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( Canvas, GraphicDeviceBase1_Base, ::cppu::WeakComponentImplHelperBase );

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >                maArguments;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< Canvas > CanvasRef;

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef ::cppu::WeakComponentImplHelper7< ::com::sun::star::rendering::XBitmapCanvas,
                                               ::com::sun::star::rendering::XIntegerBitmap,
                                               ::com::sun::star::rendering::XGraphicDevice,
                                              ::com::sun::star::lang::XMultiServiceFactory,
                                              ::com::sun::star::util::XUpdatable,
                                              ::com::sun::star::beans::XPropertySet,
                                              ::com::sun::star::lang::XServiceName >    GraphicDeviceBase2_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase2_Base >,
                                           DeviceHelper,
                                           ::osl::MutexGuard,
                                           ::cppu::OWeakObject >    CanvasBase2_Base;
    typedef ::canvas::IntegerBitmapBase< CanvasBase2_Base,
                                         BitmapCanvasHelper,
                                         ::osl::MutexGuard,
                                         ::cppu::OWeakObject >      BitmapCanvasBaseT;

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
        BitmapCanvas( const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Any >&               aArguments,
                      const ::com::sun::star::uno::Reference<
                          ::com::sun::star::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname   Base doing refcounting    Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( BitmapCanvas, GraphicDeviceBase2_Base, ::cppu::WeakComponentImplHelperBase );

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

        // BitmapProvider
        virtual IBitmapSharedPtr getBitmap() const;

     private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >                maArguments;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
        IBitmapSharedPtr                                                             mpTarget;
    };

    typedef ::rtl::Reference< BitmapCanvas > BitmapCanvasRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
