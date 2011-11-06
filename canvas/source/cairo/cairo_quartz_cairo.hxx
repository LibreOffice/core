/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
