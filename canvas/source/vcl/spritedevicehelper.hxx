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

#include <com/sun/star/awt/Rectangle.hpp>

#include "backbuffer.hxx"
#include "devicehelper.hxx"


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

        bool  showBuffer( bool bWindowVisible, bool bUpdateAll );
        bool  switchBuffer( bool bWindowVisible, bool bUpdateAll );

        css::uno::Any isAccelerated() const;
        css::uno::Any getDeviceHandle() const;
        css::uno::Any getSurfaceHandle() const;

        void dumpScreenContent() const;
        const BackBufferSharedPtr& getBackBuffer() const { return mpBackBuffer; }

        void notifySizeUpdate( const css::awt::Rectangle& rBounds );

    private:
        /// This buffer holds the background content for all associated canvases
        BackBufferSharedPtr     mpBackBuffer;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
