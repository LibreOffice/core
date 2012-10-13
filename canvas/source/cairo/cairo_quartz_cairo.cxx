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

#if defined QUARTZ || defined IOS
/************************************************************************
 * Mac OS X/Quartz and iOS surface backend for OpenOffice.org Cairo Canvas      *
 ************************************************************************/

#include <osl/diagnose.h>
#include <vcl/sysdata.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/virdev.hxx>

#include "cairo_cairo.hxx"

#if defined CAIRO_HAS_QUARTZ_SURFACE

#include "cairo_quartz_cairo.hxx"

namespace cairo
{
    bool IsCairoWorking( OutputDevice* )
    {
        // trivially true for Mac
        return true;
    }

    /**
     * QuartzSurface::Surface:     Create generic Canvas surface using given Cairo Surface
     *
     * @param pSurface Cairo Surface
     *
     * This constructor only stores data, it does no processing.
     * It is used with e.g. cairo_image_surface_create_for_data()
     * and QuartzSurface::getSimilar()
     *
     * Set the mpSurface to the new surface or NULL
     **/
    QuartzSurface::QuartzSurface( const CairoSurfaceSharedPtr& pSurface ) :
            mpView(NULL),
            mpSurface( pSurface )
    {
        // Necessary, context is lost otherwise
        CGContextRetain( getCGContext() ); //  == NULL for non-native surfaces
    }

    /**
     * QuartzSurface::Surface:   Create Canvas surface from Window reference.
     * @param NSView
     * @param x horizontal location of the new surface
     * @param y vertical location of the new surface
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * pSysData contains the platform native Window reference.
     * pSysData is used to create a surface on the Window
     *
     * Set the mpSurface to the new surface or NULL
     **/
    QuartzSurface::QuartzSurface( NSView* pView, int x, int y, int width, int height ) :
            mpView(pView),
            mpSurface()
    {
        OSL_TRACE("Canvas::cairo::Surface(NSView*, x:%d, y:%d, w:%d, h:%d): New Surface for window", x, y, width, height);

        // on Mac OS X / Quartz we are not drawing directly to the screen, but via regular CGContextRef.
        // The actual drawing to NSView (i.e. screen) is done in QuartzSurface::flush()

        // HACK: currently initial size for windowsurface is 0x0, which is not possible for us.
        if (width == 0 || height == 0) {
            width = [mpView bounds].size.width;
            height = [mpView bounds].size.height;
            OSL_TRACE("Canvas::cairo::Surface(): BUG!! size is ZERO! fixing to %d x %d...", width, height);
        }

        // create a generic surface, NSView/Window is ARGB32.
        mpSurface.reset(
            cairo_quartz_surface_create(CAIRO_FORMAT_ARGB32, width, height),
            &cairo_surface_destroy);

        cairo_surface_set_device_offset( mpSurface.get(), x, y );
    }

    /**
     * QuartzSurface::Surface:   Create Canvas surface from CGContextRef.
     * @param CGContext Native graphics context
     * @param x horizontal location of the new surface
     * @param y vertical location of the new surface
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * Set the mpSurface to the new surface or NULL
     **/
    QuartzSurface::QuartzSurface( CGContextRef rContext, int x, int y, int width, int height ) :
            mpView(NULL),
            mpSurface()
    {
        OSL_TRACE("Canvas::cairo::Surface(CGContext:%p, x:%d, y:%d, w:%d, h:%d): New Surface.", rContext, x, y, width, height);
        // create surface based on CGContext

        // ensure kCGBitmapByteOrder32Host flag, otherwise Cairo breaks (we are practically always using CGBitmapContext)
        OSL_ASSERT ((CGBitmapContextGetBitsPerPixel(rContext) != 32) ||
                    (CGBitmapContextGetBitmapInfo(rContext) & kCGBitmapByteOrderMask) == kCGBitmapByteOrder32Host);

        mpSurface.reset(cairo_quartz_surface_create_for_cg_context(rContext, width, height),
                        &cairo_surface_destroy);

        cairo_surface_set_device_offset( mpSurface.get(), x, y );

        // Necessary, context is lost otherwise
        CGContextRetain(rContext);
    }


    /**
     * QuartzSurface::getCairo:  Create Cairo (drawing object) for the Canvas surface
     *
     * @return new Cairo or NULL
     **/
    CairoSharedPtr QuartzSurface::getCairo() const
    {
        if (mpSurface.get())
        {
            return CairoSharedPtr( cairo_create(mpSurface.get()),
                                   &cairo_destroy );
        }
        else
        {
            return CairoSharedPtr();
        }
    }

    /**
     * QuartzSurface::getSimilar:  Create new similar Canvas surface
     * @param aContent format of the new surface (cairo_content_t from cairo/src/cairo.h)
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * Creates a new Canvas surface. This normally creates platform native surface, even though
     * generic function is used.
     *
     * Cairo surface from aContent (cairo_content_t)
     *
     * @return new surface or NULL
     **/
    SurfaceSharedPtr QuartzSurface::getSimilar( Content aContent, int width, int height ) const
    {
        return SurfaceSharedPtr(
            new QuartzSurface(
                CairoSurfaceSharedPtr(
                    cairo_surface_create_similar( mpSurface.get(), aContent, width, height ),
                    &cairo_surface_destroy )));
    }

    /**
     * QuartzSurface::Resize:  Resizes the Canvas surface.
     * @param width new width of the surface
     * @param height new height of the surface
     *
     * Only used on X11.
     *
     * @return The new surface or NULL
     **/
    void QuartzSurface::Resize( int /* width */, int /* height */ )
    {
        OSL_FAIL("not supposed to be called!");
    }


    /**
     * QuartzSurface::flush:  Draw the data to screen
     **/
    void QuartzSurface::flush() const
    {
        // can only flush surfaces with NSView
        if( !mpView )
            return;

        OSL_TRACE("Canvas::cairo::QuartzSurface::flush(): flush to NSView");

        CGContextRef mrContext = getCGContext();

        if (!mrContext)
            return;
#ifndef IOS
        [mpView lockFocus];
#endif

#ifndef IOS
        /**
         * This code is using same screen update code as in VCL (esp. AquaSalGraphics::UpdateWindow() )
         */
        CGContextRef rViewContext = reinterpret_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);
#else
        // Just guessing for now...
        CGContextRef rViewContext = UIGraphicsGetCurrentContext();
#endif
        CGImageRef xImage = CGBitmapContextCreateImage(mrContext);
        CGContextDrawImage(rViewContext,
                           CGRectMake( 0, 0,
                                       CGImageGetWidth(xImage),
                                       CGImageGetHeight(xImage)),
                           xImage);
        CGImageRelease( xImage );
        CGContextFlush( rViewContext );
#ifndef IOS
        [mpView unlockFocus];
#endif
    }

    /**
     * QuartzSurface::getDepth:  Get the color depth of the Canvas surface.
     *
     * @return color depth
     **/
    int QuartzSurface::getDepth() const
    {
        if (mpSurface.get())
        {
            switch (cairo_surface_get_content (mpSurface.get()))
            {
                case CAIRO_CONTENT_ALPHA:       return 8;  break;
                case CAIRO_CONTENT_COLOR:       return 24; break;
                case CAIRO_CONTENT_COLOR_ALPHA: return 32; break;
            }
        }
        OSL_TRACE("Canvas::cairo::QuartzSurface::getDepth(): ERROR - depth unspecified!");

        return -1;
    }

    /**
     * QuartzSurface::getCGContext: Get the native CGContextRef of the Canvas's cairo surface
     *
     * @return graphics context
     **/
    CGContextRef QuartzSurface::getCGContext() const
    {
        if (mpSurface.get())
            return cairo_quartz_surface_get_cg_context(mpSurface.get());
        else
            return NULL;
    }

    /**
     * cairo::createVirtualDevice:  Create a VCL virtual device for the CGContext in the cairo Surface
     *
     * @return The new virtual device
     **/
    boost::shared_ptr<VirtualDevice> QuartzSurface::createVirtualDevice() const
    {
        SystemGraphicsData aSystemGraphicsData;
        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.rCGContext = getCGContext();
        return boost::shared_ptr<VirtualDevice>(
            new VirtualDevice( &aSystemGraphicsData, getDepth() ));
    }

    /**
     * cairo::createSurface:     Create generic Canvas surface using given Cairo Surface
     *
     * @param rSurface Cairo Surface
     *
     * @return new Surface
     */
    SurfaceSharedPtr createSurface( const CairoSurfaceSharedPtr& rSurface )
    {
        return SurfaceSharedPtr(new QuartzSurface(rSurface));
    }

    /**
     * cairo::createSurface:     Create Canvas surface using given VCL Window or Virtualdevice
     *
     * @param rSurface Cairo Surface
     *
     *  For VCL Window, use platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
     *  For VCL Virtualdevice, use platform native system graphics data (struct SystemGraphicsData in vcl/inc/sysdata.hxx)
     *
     * @return new Surface
     */
    SurfaceSharedPtr createSurface( const OutputDevice& rRefDevice,
                                    int x, int y, int width, int height )
    {
        SurfaceSharedPtr surf;

        if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
        {
            const Window &rWindow = (const Window &) rRefDevice;
            const SystemEnvData* pSysData = GetSysData(&rWindow);
            if (pSysData)
                surf = SurfaceSharedPtr(new QuartzSurface(pSysData->pView, x, y, width, height));
        }
        else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
        {
            SystemGraphicsData aSysData = ((const VirtualDevice&) rRefDevice).GetSystemGfxData();

            if (aSysData.rCGContext)
                surf =  SurfaceSharedPtr(new QuartzSurface(aSysData.rCGContext, x, y, width, height));
        }
        return surf;
    }

    /**
     * cairo::createBitmapSurface:   Create platfrom native Canvas surface from BitmapSystemData
     * @param OutputDevice (not used)
     * @param rData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
     * @param rSize width and height of the new surface
     *
     * Create a surface based on image data on rData
     *
     * @return new surface or empty surface
     **/
    SurfaceSharedPtr createBitmapSurface( const OutputDevice&     /* rRefDevice */,
                                          const BitmapSystemData& rData,
                                          const Size&             rSize )
    {
        OSL_TRACE( "requested size: %d x %d available size: %d x %d",
                   rSize.Width(), rSize.Height(), rData.mnWidth, rData.mnHeight );

        if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
        {
            CGContextRef rContext = (CGContextRef)rData.rImageContext;
            OSL_TRACE("Canvas::cairo::createBitmapSurface(): New native image surface, context = %p.", rData.rImageContext);

            return SurfaceSharedPtr(new QuartzSurface(rContext, 0, 0, rData.mnWidth, rData.mnHeight));
        }
        return SurfaceSharedPtr();
    }

}  // namespace cairo

#endif   // CAIRO_HAS_QUARTZ_SURFACE

#endif   // QUARTZ

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
