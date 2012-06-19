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

#ifndef _CAIROCANVAS_QUARTZ_CAIRO_HXX
#define _CAIROCANVAS_QUARTZ_CAIRO_HXX

#include "cairo_cairo.hxx"

#include "premac.h"
#include <TargetConditionals.h>
#if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
# include <Cocoa/Cocoa.h>
#else
# include <UIKit/UIKit.h>
# define NSView UIView
#endif
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
