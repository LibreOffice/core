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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SPRITEDEVICEHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SPRITEDEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include "dx_rendermodule.hxx"
#include "dx_surfacebitmap.hxx"
#include "dx_devicehelper.hxx"

#include <canvas/rendering/isurfaceproxymanager.hxx>


namespace dxcanvas
{
    class SpriteCanvas;

    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( vcl::Window&                             rWindow,
                   SpriteCanvas&                            rSpriteCanvas,
                   const css::awt::Rectangle&               rRect,
                   bool                                     bFullscreen );

        /// Dispose all internal references
        void disposing();

        // partial override XWindowGraphicDevice
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

        void        destroyBuffers(  );
        sal_Bool  showBuffer( bool bIsVisible, sal_Bool bUpdateAll );
        sal_Bool  switchBuffer( bool bIsVisible, sal_Bool bUpdateAll );

        const IDXRenderModuleSharedPtr& getRenderModule() const { return mpRenderModule; }
        const DXSurfaceBitmapSharedPtr& getBackBuffer() const { return mpBackBuffer; }
        const ::canvas::ISurfaceProxyManagerSharedPtr &getSurfaceProxy() const { return mpSurfaceProxyManager; }

        css::uno::Any isAccelerated() const;

        void notifySizeUpdate( const css::awt::Rectangle& rBounds );

        /** called when DumpScreenContent property is enabled on
            XGraphicDevice, and writes out bitmaps of current screen.
         */
        void dumpScreenContent() const;

    private:
        void resizeBackBuffer( const ::basegfx::B2ISize& rNewSize );
        HWND getHwnd() const;

        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*                           mpSpriteCanvas;

        DXSurfaceBitmapSharedPtr                mpBackBuffer;

        /// Instance passing out HW textures
        ::canvas::ISurfaceProxyManagerSharedPtr mpSurfaceProxyManager;

        /// Our encapsulation interface to DirectX
        IDXRenderModuleSharedPtr                mpRenderModule;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SPRITEDEVICEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
