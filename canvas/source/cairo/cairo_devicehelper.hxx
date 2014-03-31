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

#ifndef INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_DEVICEHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_DEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <boost/utility.hpp>

#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>

#include "cairo_cairo.hxx"
#include "cairo_surfaceprovider.hxx"

/* Definition of DeviceHelper class */

namespace cairocanvas
{
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
        ::cairo::SurfaceSharedPtr mpSurface;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
