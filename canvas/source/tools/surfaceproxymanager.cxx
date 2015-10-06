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

#include <canvas/rendering/isurfaceproxy.hxx>
#include <canvas/rendering/isurfaceproxymanager.hxx>

#include "surfaceproxy.hxx"

namespace canvas
{
    class SurfaceProxyManager : public ISurfaceProxyManager
    {
    public:

        explicit SurfaceProxyManager( const IRenderModuleSharedPtr& rRenderModule ) :
            mpPageManager( new PageManager(rRenderModule) )
        {
        }

        /** the whole idea is build around the concept that you create
            some arbitrary buffer which contains the image data and
            tell the texture manager about it.  from there on you can
            draw this image using any kind of graphics api you want.
            in the technical sense we allocate some space in local
            videomemory or AGP memory which will be filled on demand,
            which means if there exists any rendering operation that
            needs to read from this memory location.  this method
            creates a logical hardware surface object which uses the
            given color buffer as the image source.  internally this
            texture may be distributed to several real hardware
            surfaces.
        */
        virtual std::shared_ptr<ISurfaceProxy> createSurfaceProxy( const IColorBufferSharedPtr& pBuffer ) const SAL_OVERRIDE
        {
            // not much to do for now, simply allocate a new surface
            // proxy from our internal pool and initialize this thing
            // properly. we *don't* create a hardware surface for now.
            return SurfaceProxySharedPtr(new SurfaceProxy(pBuffer,mpPageManager));
        }

    private:
        PageManagerSharedPtr mpPageManager;
    };

    ISurfaceProxyManagerSharedPtr createSurfaceProxyManager( const IRenderModuleSharedPtr& rRenderModule )
    {
        return ISurfaceProxyManagerSharedPtr(
            new SurfaceProxyManager(
                rRenderModule));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
