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
    SurfaceSharedPtr createBitmapSurface( const OutputDevice& rRefDevice,
                                          const BitmapSystemData& rData,
                                          const Size&             rSize );

    /// Check whether cairo will work on given window
    bool IsCairoWorking( OutputDevice* );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
