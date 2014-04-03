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

#ifndef INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITEDEVICEHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITEDEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <boost/utility.hpp>

#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>

#include "cairo_cairo.hxx"
#include "cairo_devicehelper.hxx"

/* Definition of DeviceHelper class */

namespace cairocanvas
{
    class SpriteCanvas;

    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( Window&                   rOutputWindow,
                   SpriteCanvas&             rSpriteCanvas,
                   const ::basegfx::B2ISize& rSize,
                   bool                      bFullscreen );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        sal_Bool  showBuffer( bool, sal_Bool );
        sal_Bool  switchBuffer( bool, sal_Bool bUpdateAll );

        ::com::sun::star::uno::Any isAccelerated() const;
        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );
        void setSize( const ::basegfx::B2ISize& rSize );

        ::cairo::SurfaceSharedPtr getBufferSurface();
        ::cairo::SurfaceSharedPtr getWindowSurface();
        ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, ::cairo::Content aContent = CAIRO_CONTENT_COLOR_ALPHA );
        ::cairo::SurfaceSharedPtr createSurface( BitmapSystemData& rData, const Size& rSize );
        const ::basegfx::B2ISize& getSizePixel();
        void flush();

    private:
        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*             mpSpriteCanvas;

        ::cairo::SurfaceSharedPtr mpBufferSurface;

        ::basegfx::B2ISize        maSize;
        bool                      mbFullScreen;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
