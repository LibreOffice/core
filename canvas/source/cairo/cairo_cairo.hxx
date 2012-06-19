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

#ifndef _CAIROCANVAS_CAIRO_HXX
#define _CAIROCANVAS_CAIRO_HXX

#include <sal/config.h>
#include <boost/shared_ptr.hpp>

struct SystemEnvData;
struct BitmapSystemData;
struct SystemGraphicsData;
class  VirtualDevice;
class  OutputDevice;
class  Window;
class  Size;

#include <cairo.h>  //cannot be inside a namespace, otherwise Quartz fails to compile.

namespace cairo {
    typedef cairo_t Cairo;
    typedef cairo_matrix_t Matrix;
    typedef cairo_format_t Format;
    typedef cairo_content_t Content;
    typedef cairo_pattern_t Pattern;

    typedef boost::shared_ptr<cairo_surface_t> CairoSurfaceSharedPtr;
    typedef boost::shared_ptr<Cairo>           CairoSharedPtr;

    const SystemEnvData* GetSysData(const Window *pOutputWindow);

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
        virtual boost::shared_ptr<Surface> getSimilar( Content aContent, int width, int height ) const = 0;

        /// factory for VirDev on this surface
        virtual boost::shared_ptr<VirtualDevice> createVirtualDevice() const = 0;

        /// Resize the surface (possibly destroying content)
        virtual void Resize( int width, int height ) = 0;

        /// Flush all pending output to surface
        virtual void flush() const = 0;
    };

    typedef boost::shared_ptr<Surface> SurfaceSharedPtr;

    /// Create Surface from given cairo surface
    SurfaceSharedPtr createSurface( const CairoSurfaceSharedPtr& rSurface );
    /// Create surface with given dimensions
    SurfaceSharedPtr createSurface( const OutputDevice& rRefDevice,
                                    int x, int y, int width, int height );
    /// Create Surface for given bitmap data
    SurfaceSharedPtr createBitmapSurface( const OutputDevice&     rRefDevice,
                                          const BitmapSystemData& rData,
                                          const Size&             rSize );

    /// Check whether cairo will work on given window
    bool IsCairoWorking( OutputDevice* );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
