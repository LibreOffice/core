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

#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include "outdevprovider.hxx"


/* Definition of DeviceHelper class */

namespace vclcanvas
{
    class DeviceHelper
    {
    public:
        DeviceHelper();

        /// make noncopyable
        DeviceHelper(const DeviceHelper&) = delete;
        const DeviceHelper& operator=(const DeviceHelper&) = delete;

        void init( const OutDevProviderSharedPtr& rOutDev );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        css::geometry::RealSize2D getPhysicalResolution();
        css::geometry::RealSize2D getPhysicalSize();
        css::uno::Reference< css::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon(
            const css::uno::Reference< css::rendering::XGraphicDevice >&                               rDevice,
            const css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > >& points );
        css::uno::Reference< css::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon(
            const css::uno::Reference< css::rendering::XGraphicDevice >&                                       rDevice,
            const css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > >& points );
        css::uno::Reference< css::rendering::XBitmap > createCompatibleBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );
        css::uno::Reference< css::rendering::XVolatileBitmap > createVolatileBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );
        css::uno::Reference< css::rendering::XBitmap > createCompatibleAlphaBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );
        css::uno::Reference< css::rendering::XVolatileBitmap > createVolatileAlphaBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );

        css::uno::Any isAccelerated() const;
        css::uno::Any getDeviceHandle() const;
        css::uno::Any getSurfaceHandle() const;
        css::uno::Reference<
            css::rendering::XColorSpace > const & getColorSpace() const;

        const OutDevProviderSharedPtr& getOutDev() const { return mpOutDev; }

        /** called when DumpScreenContent property is enabled on
            XGraphicDevice, and writes out bitmaps of current screen.
         */
        void dumpScreenContent() const;

    private:
        /// For retrieving device info
        OutDevProviderSharedPtr mpOutDev;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
