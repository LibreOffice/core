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

#ifndef _DXCANVAS_SPRITEDEVICEHELPER_HXX
#define _DXCANVAS_SPRITEDEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include "dx_rendermodule.hxx"
#include "dx_surfacebitmap.hxx"
#include "dx_devicehelper.hxx"

#include <canvas/rendering/isurfaceproxymanager.hxx>

#include <boost/utility.hpp>


namespace dxcanvas
{
    class SpriteCanvas;
    class SpriteCanvasHelper;

    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( Window&                                  rWindow,
                   SpriteCanvas&                            rSpriteCanvas,
                   const ::com::sun::star::awt::Rectangle&  rRect,
                   bool                                     bFullscreen );

        /// Dispose all internal references
        void disposing();

        // partial override XWindowGraphicDevice
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

        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        ::sal_Bool  showBuffer( bool bIsVisible, ::sal_Bool bUpdateAll );
        ::sal_Bool  switchBuffer( bool bIsVisible, ::sal_Bool bUpdateAll );

        const IDXRenderModuleSharedPtr& getRenderModule() const { return mpRenderModule; }
        const DXSurfaceBitmapSharedPtr& getBackBuffer() const { return mpBackBuffer; }
        const ::canvas::ISurfaceProxyManagerSharedPtr &getSurfaceProxy() const { return mpSurfaceProxyManager; }

        ::com::sun::star::uno::Any isAccelerated() const;

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );

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

#endif /* _DXCANVAS_SPRITEDEVICEHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
