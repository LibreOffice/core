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

#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <vcl/window.hxx>
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <basegfx/tools/canvastools.hxx>
#include "dx_linepolypolygon.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_canvasbitmap.hxx"
#include "dx_spritedevicehelper.hxx"


#undef WB_LEFT
#undef WB_RIGHT
#include "dx_winstuff.hxx"


#include <vcl/sysdata.hxx>

using namespace ::com::sun::star;

namespace dxcanvas
{
    SpriteDeviceHelper::SpriteDeviceHelper() :
        DeviceHelper(),
        mpSpriteCanvas( NULL ),
        mpSurfaceProxyManager(),
        mpRenderModule(),
        mpBackBuffer()
    {
    }

    void SpriteDeviceHelper::init( Window&               rWindow,
                                   SpriteCanvas&         rSpriteCanvas,
                                   const awt::Rectangle& rRect,
                                   bool                  /*bFullscreen*/ )
    {
        // #i60490# ensure backbuffer has sensible minimal size
        const sal_Int32 w( ::std::max(sal_Int32(1),sal_Int32(rRect.Width)));
        const sal_Int32 h( ::std::max(sal_Int32(1),sal_Int32(rRect.Height)));

        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rWindow),
                uno::UNO_QUERY_THROW) );

        const SystemEnvData *pData = rWindow.GetSystemData();
        const HWND hWnd = reinterpret_cast<HWND>(pData->hWnd);
        if( !IsWindow( hWnd ) )
            throw lang::NoSupportException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                     "Passed window has invalid system window, or canvas out-of-process!")),
                NULL);

        mpSpriteCanvas = &rSpriteCanvas;

        try
        {
            // setup directx rendermodule
            mpRenderModule = createRenderModule( rWindow );
        }
        catch (...) {

            throw lang::NoSupportException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                     "Could not create DirectX device!") ),
                static_cast< ::cppu::OWeakObject* >(&rSpriteCanvas) );
        }

        // create the surfaceproxy manager
        mpSurfaceProxyManager = ::canvas::createSurfaceProxyManager( mpRenderModule );

        // #i60490# ensure backbuffer has sensible minimal size
        mpBackBuffer.reset(new DXSurfaceBitmap(
                               ::basegfx::B2ISize(w,h),
                               mpSurfaceProxyManager,
                               mpRenderModule,
                               false));

        // Assumes: SystemChildWindow() has CS_OWNDC
        DeviceHelper::init(GetDC(mpRenderModule->getHWND()),
                           rSpriteCanvas);
    }

    void SpriteDeviceHelper::disposing()
    {
        // release all references
        mpBackBuffer.reset();
        mpSurfaceProxyManager.reset();
        mpRenderModule.reset();
        mpSpriteCanvas = NULL;

        DeviceHelper::disposing();
    }

    uno::Reference< rendering::XBitmap > SpriteDeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        if( !getDevice() )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        DXSurfaceBitmapSharedPtr pBitmap(
            new DXSurfaceBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D(size),
                mpSurfaceProxyManager,
                mpRenderModule,
                false));

        // create a 24bit RGB system memory surface
        return uno::Reference< rendering::XBitmap >(new CanvasBitmap(pBitmap,getDevice()));
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > SpriteDeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        if( !getDevice() )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        DXSurfaceBitmapSharedPtr pBitmap(
            new DXSurfaceBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D(size),
                mpSurfaceProxyManager,
                mpRenderModule,
                true));

        // create a 32bit ARGB system memory surface
        return uno::Reference< rendering::XBitmap >(new CanvasBitmap(pBitmap,getDevice()));
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    sal_Bool SpriteDeviceHelper::hasFullScreenMode()
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    sal_Bool SpriteDeviceHelper::enterFullScreenMode( sal_Bool /*bEnter*/ )
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    ::sal_Int32 SpriteDeviceHelper::createBuffers( ::sal_Int32 /*nBuffers*/ )
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
        return 1;
    }

    void SpriteDeviceHelper::destroyBuffers()
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
    }

    ::sal_Bool SpriteDeviceHelper::showBuffer( bool, ::sal_Bool )
    {
        OSL_FAIL("Not supposed to be called, handled by SpriteCanvas");
        return sal_False;
    }

    ::sal_Bool SpriteDeviceHelper::switchBuffer( bool, ::sal_Bool )
    {
        OSL_FAIL("Not supposed to be called, handled by SpriteCanvas");
        return sal_False;
    }

    uno::Any SpriteDeviceHelper::isAccelerated() const
    {
        return ::com::sun::star::uno::makeAny(true);
    }

    void SpriteDeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        // #i60490# ensure backbuffer has sensible minimal size
        const sal_Int32 x(rBounds.X);
        const sal_Int32 y(rBounds.Y);
        const sal_Int32 w(::std::max(sal_Int32(1),sal_Int32(rBounds.Width)));
        const sal_Int32 h(::std::max(sal_Int32(1),sal_Int32(rBounds.Height)));

        if( mpRenderModule )
            mpRenderModule->resize(::basegfx::B2IRange(x,y,x+w,y+h));

        resizeBackBuffer(::basegfx::B2ISize(w,h));
    }

    void SpriteDeviceHelper::resizeBackBuffer( const ::basegfx::B2ISize& rNewSize )
    {
        // disposed?
        if(!(mpBackBuffer))
            return;

        mpBackBuffer->resize(rNewSize);
        mpBackBuffer->clear();
    }

    HWND SpriteDeviceHelper::getHwnd() const
    {
        if( mpRenderModule )
            return mpRenderModule->getHWND();
        else
            return 0;
    }

    void SpriteDeviceHelper::dumpScreenContent() const
    {
        if( mpRenderModule )
            mpRenderModule->screenShot();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
