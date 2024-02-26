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

#include <sal/config.h>
#include <osl/endian.h>
#include <vcl/vclptr.hxx>
#include <config_cairo_rgba.h>
#include <memory>

// Using formats that match cairo's formats.
// SVP_24BIT_FORMAT is used to store 24-bit images in 3-byte pixels to conserve memory.

/*
 For internal cairo we have the option --enable-cairo-rgba which is potentially
 useful for Android or Online to switch the rgb components. For Android cairo then
 matches the OpenGL GL_RGBA format so we can use it there where we don't have
 GL_BGRA support. Similarly for Online we can then use cairo's pixel data
 without needing to swizzle it for use as a canvas ImageData.
*/
#if ENABLE_CAIRO_RGBA
#define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcRgb)
#define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcRgba)
#define SVP_CAIRO_BLUE 1
#define SVP_CAIRO_GREEN 2
#define SVP_CAIRO_RED 0
#define SVP_CAIRO_ALPHA 3
#elif defined OSL_BIGENDIAN
#define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcRgb)
#define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcArgb)
#define SVP_CAIRO_BLUE 3
#define SVP_CAIRO_GREEN 2
#define SVP_CAIRO_RED 1
#define SVP_CAIRO_ALPHA 0
#else
#define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcBgr)
#define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcBgra)
#define SVP_CAIRO_BLUE 0
#define SVP_CAIRO_GREEN 1
#define SVP_CAIRO_RED 2
#define SVP_CAIRO_ALPHA 3
#endif

typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;

class VirtualDevice;

namespace cairo {

    typedef std::shared_ptr<cairo_surface_t> CairoSurfaceSharedPtr;
    typedef std::shared_ptr<cairo_t>         CairoSharedPtr;
    struct Surface;
    typedef std::shared_ptr<Surface>         SurfaceSharedPtr;

    /** Cairo surface interface

        For each cairo-supported platform, there's an implementation of
        this interface
     */
    struct Surface
    {
    public:
        virtual ~Surface() {}

        // Query methods
        virtual CairoSharedPtr getCairo() const = 0;
        virtual CairoSurfaceSharedPtr getCairoSurface() const = 0;
        virtual SurfaceSharedPtr getSimilar(int cairo_content_type, int width, int height) const = 0;

        /// factory for VirDev on this surface
        virtual VclPtr<VirtualDevice> createVirtualDevice() const = 0;

        /// Resize the surface (possibly destroying content), only possible for X11 typically
        /// so on failure caller must create a new surface instead
        virtual bool Resize( int /*width*/, int /*height*/ ) { return false; }

        /// Flush all pending output to surface
        virtual void flush() const = 0;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
