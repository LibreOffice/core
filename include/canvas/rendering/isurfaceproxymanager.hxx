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

#ifndef INCLUDED_CANVAS_RENDERING_ISURFACEPROXYMANAGER_HXX
#define INCLUDED_CANVAS_RENDERING_ISURFACEPROXYMANAGER_HXX

#include <canvas/canvastoolsdllapi.h>
#include <memory>

namespace canvas
{
    struct ISurfaceProxy;
    struct IColorBuffer;
    struct IRenderModule;

    /** Manager interface, which handles surface proxy objects.

        Typically, each canvas instantiation has one
        SurfaceProxyManager object, to handle their surfaces. Surfaces
        itself are opaque objects, which encapsulate a framebuffer to
        render upon, plus an optional (possibly accelerated) texture.
     */
    struct ISurfaceProxyManager
    {
        virtual ~ISurfaceProxyManager() {}

        /** Create a surface proxy for a color buffer.

            The whole idea is build around the concept that you create
            some arbitrary buffer which contains the image data and
            tell the texture manager about it.  From there on you can
            draw into this image using any kind of graphics api you
            want.  In the technical sense we allocate some space in
            local videomemory or AGP memory which will be filled on
            demand, which means if there exists any rendering
            operation that needs to read from this memory location.
            This method creates a logical hardware surface object
            which uses the given color buffer as the image source.
            Internally this texture may even be distributed to several
            real hardware surfaces.
        */
        virtual std::shared_ptr< ISurfaceProxy > createSurfaceProxy(
            const std::shared_ptr<IColorBuffer>& pBuffer ) const = 0;
    };

    /** Create a surface proxy for the given render module.
     */
    CANVASTOOLS_DLLPUBLIC std::shared_ptr<ISurfaceProxyManager> createSurfaceProxyManager( const std::shared_ptr<IRenderModule>& rRenderModule );
}

#endif // INCLUDED_CANVAS_RENDERING_ISURFACEPROXYMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
