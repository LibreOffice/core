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

#ifndef INCLUDED_VCL_CAIRO_HXX
#define INCLUDED_VCL_CAIRO_HXX

#include <sal/config.h>
#include <vcl/vclptr.hxx>
#include <memory>

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
