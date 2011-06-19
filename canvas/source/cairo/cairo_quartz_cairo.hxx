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

#ifndef _CAIROCANVAS_QUARTZ_CAIRO_HXX
#define _CAIROCANVAS_QUARTZ_CAIRO_HXX

#include "cairo_cairo.hxx"

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include <cairo-quartz.h>
#include "postmac.h"

namespace cairo {

    class QuartzSurface : public Surface
    {
        NSView*               mpView; // if NULL - bg surface
        CairoSurfaceSharedPtr mpSurface;

    public:
        /// takes over ownership of passed cairo_surface
        explicit QuartzSurface( const CairoSurfaceSharedPtr& pSurface );

        /// create surface on subarea of given CGContext
        explicit QuartzSurface( CGContextRef rContext, int x, int y, int width, int height );

        /// create a offscreen surface for given NSView
        QuartzSurface( NSView* pView, int x, int y, int width, int height );

        // Surface interface
        virtual CairoSharedPtr getCairo() const;
        virtual CairoSurfaceSharedPtr getCairoSurface() const { return mpSurface; }
        virtual SurfaceSharedPtr getSimilar( Content aContent, int width, int height ) const;

        virtual boost::shared_ptr<VirtualDevice> createVirtualDevice() const;

        virtual void Resize( int width, int height );

        virtual void flush() const;

        int getDepth() const;

        CGContextRef getCGContext() const;

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
