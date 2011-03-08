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

#ifndef _VCLCANVAS_SPRITEDEVICEHELPER_HXX
#define _VCLCANVAS_SPRITEDEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

#include "backbuffer.hxx"
#include "devicehelper.hxx"

#include <boost/utility.hpp>


/* Definition of DeviceHelper class */

namespace vclcanvas
{
    class SpriteCanvas;
    class SpriteCanvasHelper;

    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( const OutDevProviderSharedPtr& rOutDev );

        /// Dispose all internal references
        void disposing();

        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        ::sal_Bool  showBuffer( bool bWindowVisible, ::sal_Bool bUpdateAll );
        ::sal_Bool  switchBuffer( bool bWindowVisible, ::sal_Bool bUpdateAll );

        ::com::sun::star::uno::Any isAccelerated() const;
        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;

        void dumpScreenContent() const;
        BackBufferSharedPtr getBackBuffer() const { return mpBackBuffer; }

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );

    private:
        /// This buffer holds the background content for all associated canvases
        BackBufferSharedPtr     mpBackBuffer;
    };
}

#endif /* _VCLCANVAS_SPRITEDEVICEHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
