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

#include <sal/config.h>

#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/unopolypolygon.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/cairo.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/syschild.hxx>

#include <canvas/canvastools.hxx>

#include "cairo_canvasbitmap.hxx"
#include "cairo_devicehelper.hxx"
#include "cairo_spritecanvas.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{

    SpriteDeviceHelper::SpriteDeviceHelper() :
        mpSpriteCanvas( nullptr ),
        mpBufferSurface(),
        maSize(),
        mbFullScreen( false )
    {}

    void SpriteDeviceHelper::init( vcl::Window&                   rOutputWindow,
                                   SpriteCanvas&             rSpriteCanvas,
                                   const ::basegfx::B2ISize& rSize,
                                   bool                      bFullscreen )
    {
        DeviceHelper::init(rSpriteCanvas,
                           rOutputWindow);

        mpSpriteCanvas = &rSpriteCanvas;
        mbFullScreen = bFullscreen;

        setSize( rSize );
    }

    void SpriteDeviceHelper::disposing()
    {
        // release all references
        mpBufferSurface.reset();
        mpSpriteCanvas = nullptr;
    }

    bool SpriteDeviceHelper::showBuffer( bool, bool )
    {
        SAL_WARN("canvas.cairo", "showBuffer Not supposed to be called, handled by SpriteCanvas");
        return false;
    }

    bool SpriteDeviceHelper::switchBuffer( bool, bool )
    {
        SAL_WARN("canvas.cairo", "showBuffer Not supposed to be called, handled by SpriteCanvas");
        return false;
    }

    uno::Any SpriteDeviceHelper::isAccelerated() const
    {
        return css::uno::Any(true);
    }

    uno::Any SpriteDeviceHelper::getDeviceHandle() const
    {
        return DeviceHelper::getDeviceHandle();
    }

    uno::Any SpriteDeviceHelper::getSurfaceHandle() const
    {
        return DeviceHelper::getSurfaceHandle();
    }

    void SpriteDeviceHelper::setSize( const ::basegfx::B2ISize& rSize )
    {
        SAL_INFO(
            "canvas.cairo",
            "device size " << rSize.getX() << " x " << rSize.getY());

        if( !mpSpriteCanvas )
            return; // disposed

        DeviceHelper::setSize(rSize);

        if( mpBufferSurface && maSize != rSize )
            mpBufferSurface.reset();
        if( !mpBufferSurface )
            mpBufferSurface = getWindowSurface()->getSimilar(
                CAIRO_CONTENT_COLOR,
                rSize.getX(), rSize.getY() );

        if( maSize != rSize )
            maSize = rSize;

        mpSpriteCanvas->setSizePixel( maSize );
    }


    void SpriteDeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        setSize( ::basegfx::B2ISize(rBounds.Width, rBounds.Height) );
    }

    SurfaceSharedPtr SpriteDeviceHelper::getWindowSurface()
    {
        return DeviceHelper::getSurface();
    }

    SurfaceSharedPtr SpriteDeviceHelper::createSurface( const ::basegfx::B2ISize& rSize, int aContent )
    {
        if( mpBufferSurface )
            return mpBufferSurface->getSimilar( aContent, rSize.getX(), rSize.getY() );

        return SurfaceSharedPtr();
    }

    SurfaceSharedPtr SpriteDeviceHelper::createSurface( BitmapSystemData& rData, const Size& rSize )
    {
        OutputDevice *pDevice = getOutputDevice();
        if (pDevice)
            return pDevice->CreateBitmapSurface(rData, rSize);
        return SurfaceSharedPtr();
    }

    /** SpriteDeviceHelper::flush  Flush the platform native window
     *
     * Flushes the window by using the internally stored mpSysData.
     *
     **/
    void SpriteDeviceHelper::flush()
    {
        SurfaceSharedPtr pWinSurface=getWindowSurface();
        if( pWinSurface )
            pWinSurface->flush();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
