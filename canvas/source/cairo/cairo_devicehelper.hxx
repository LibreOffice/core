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

#ifndef _CAIROCANVAS_DEVICEHELPER_HXX
#define _CAIROCANVAS_DEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <boost/utility.hpp>

#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>

#include "cairo_cairo.hxx"
#include "cairo_surfaceprovider.hxx"

/* Definition of DeviceHelper class */

struct SystemEnvData;
class Window;

namespace cairocanvas
{
    class Canvas;
    class CanvasHelper;

    class DeviceHelper : private ::boost::noncopyable
    {
    public:
        DeviceHelper();

        /** init helper

            @param rCanvas
            Owning canvas.

            @param rRefDevice
            Reference output device. Needed for resolution
            calculations etc.
         */
        void init( SurfaceProvider& rSurfaceProvider,
                   OutputDevice&    rRefDevice );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        ::com::sun::star::geometry::RealSize2D getPhysicalResolution();
        ::com::sun::star::geometry::RealSize2D getPhysicalSize();
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&                               rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&                                       rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleAlphaBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileAlphaBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        sal_Bool hasFullScreenMode(  );
        sal_Bool enterFullScreenMode( sal_Bool bEnter );

        ::com::sun::star::uno::Any isAccelerated() const;
        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XColorSpace > getColorSpace() const;

        /** called when DumpScreenContent property is enabled on
            XGraphicDevice, and writes out bitmaps of current screen.
         */
        void dumpScreenContent() const;

        OutputDevice* getOutputDevice() const { return mpRefDevice; }
        const void* getSysData() { return mpSysData; }
        ::cairo::SurfaceSharedPtr getSurface();
        ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, ::cairo::Content aContent = CAIRO_CONTENT_COLOR_ALPHA );
        ::cairo::SurfaceSharedPtr createSurface( BitmapSystemData& rData, const Size& rSize );

    protected:
        /** init helper

            @param rCanvas
            Owning canvas.

            @param rRefDevice
            Reference output device. Needed for resolution
            calculations etc.
         */
        void implInit( SurfaceProvider& rSurfaceProvider,
                       OutputDevice&    rRefDevice );
        void setSize( const ::basegfx::B2ISize& rSize );

    private:
        /** Surface provider

            Deliberately not a refcounted reference, because of
            potential circular references for canvas. Provides us with
            our output surface and associated functionality.
         */
        SurfaceProvider*          mpSurfaceProvider;

        OutputDevice*             mpRefDevice;
        const void*               mpSysData;
        ::cairo::SurfaceSharedPtr mpSurface;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
