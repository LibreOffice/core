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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_SPRITEDEVICEHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_SPRITEDEVICEHELPER_HXX

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
    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( const OutDevProviderSharedPtr& rOutDev );

        /// Dispose all internal references
        void disposing();

        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        sal_Bool  showBuffer( bool bWindowVisible, sal_Bool bUpdateAll );
        sal_Bool  switchBuffer( bool bWindowVisible, sal_Bool bUpdateAll );

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

#endif // INCLUDED_CANVAS_SOURCE_VCL_SPRITEDEVICEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
