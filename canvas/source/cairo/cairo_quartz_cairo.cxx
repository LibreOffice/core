/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_quartz_cairo.cxx,v $
 *
 * $Revision: 1.3 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"


#ifdef QUARTZ
/************************************************************************
 * Mac OS X/Quartz surface backend for OpenOffice.org Cairo Canvas      *
 ************************************************************************/

#include <osl/diagnose.h>

#include "cairo_cairo.hxx"

#if defined CAIRO_HAS_QUARTZ_SURFACE

// premac.h/postmac.h needed because cairo-quartz.h includes Carbon/Carbon.h
#include "premac.h"
#include <cairo-quartz.h>
#include "postmac.h"

namespace cairo
{
  /**
   * Surface::Surface:     Create generic Canvas surface using given Cairo Surface
   *
   * @param pSurface Cairo Surface
   *
   * This constructor only stores data, it does no processing.
   * It is used with e.g. cairo_image_surface_create_for_data()
   * and Surface::getSimilar()
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( cairo_surface_t* pSurface )
            : mnRefCount( 1 ),
              mpSurface( pSurface )
    {
      // Necessary, context is lost otherwise
      CGContextRetain( cairo_quartz_surface_get_cg_context(pSurface) ); //  == NULL for non-native surfaces
    }

  /**
   * Surface::Surface:   Create Canvas surface from Window reference.
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
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
    Surface::Surface( const SystemEnvData* pSysData, int x, int y, int width, int height )
        : mnRefCount( 1 ),
          mpSurface( NULL )
    {
         OSL_TRACE("Canvas::cairo::Surface(const void*, x:%d, y:%d, w:%d, h:%d): New Surface for window", x, y, width, height);

         // on Mac OS X / Quartz we are not drawing directly to the screen, but via regular CGContextRef.
         // The actual drawing to NSView (i.e. screen) is done in Surface::flush()

     // HACK: currently initial size for windowsurface is 0x0, which is not possible for us.
     if (width == 0 || height == 0) {
       NSView* mpView = (NSView *) pSysData->pView;
       width = [mpView bounds].size.width;
       height = [mpView bounds].size.height;
       OSL_TRACE("Canvas::cairo::Surface(): BUG!! size is ZERO! fixing to %d x %d...", width, height);
     }

         // create a generic surface. Although View/Window is ARGB32, we however do all drawing => RGB24
     mpSurface = cairo_quartz_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        cairo_surface_set_device_offset( mpSurface, x, y );
    }

  /**
   * Surface::Surface:   Create platfrom native Canvas surface from BitmapSystemData
   * @param pSysData (not used)
   * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
   * @param width width of the new surface
   * @param height height of the new surface
   *
   * Create a surface based on image data on pBmpData
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( const SystemEnvData* /* pSysData */, const BitmapSystemData* pBmpData, int width, int height )
        : mnRefCount( 1 ),
          mpSurface( NULL )
    {
        OSL_TRACE("Canvas::cairo::Surface(pBmpData: %p, w:%d, h:%d): New native image surface.", pBmpData, width, height);

        OSL_ASSERT (pBmpData->mnWidth == width && pBmpData->mnHeight == height);
        CGContextRef rContext = (CGContextRef) pBmpData->rImageContext;
    // ensure kCGBitmapByteOrder32Host flag, otherwise Cairo breaks
    OSL_ASSERT ((CGBitmapContextGetBitmapInfo(rContext) & kCGBitmapByteOrderMask) == kCGBitmapByteOrder32Host);

    // create image surface based on CGContext
        mpSurface = cairo_quartz_surface_create_for_cg_context( rContext, width, height);

    // Necessary, context is lost otherwise
    CGContextRetain(rContext);
    }


  /**
   * Surface::getDepth:  Get the color depth of the Canvas surface.
   *
   * @return color depth
   **/
    int
    Surface::getDepth()
    {
        if (mpSurface) {
            switch (cairo_surface_get_content (mpSurface)) {
                case CAIRO_CONTENT_ALPHA:       return 8;  break;
                case CAIRO_CONTENT_COLOR:       return 24; break;
                case CAIRO_CONTENT_COLOR_ALPHA: return 32; break;
            }
        }
        OSL_TRACE("Canvas::cairo::%s(): ERROR - depth unspecified!",__func__);

        return -1;
    }

  /**
   * Surface::fillSystemGraphicsData:   Fill SystemGraphicsData with native surface data
   * @param aSystemGraphicsData Platform native system graphics data (struct SystemGraphicsData in vcl/inc/sysdata.hxx)
   *
   */
    void Surface::fillSystemGraphicsData( SystemGraphicsData& aSystemGraphicsData)
    {
        aSystemGraphicsData.rCGContext = cairo_quartz_surface_get_cg_context( mpSurface );
    }

  /** Surface::flush  Flush the platform native window
   *
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/vcl/sysdata.hxx)
   *
   **/
    void Surface::flush(const SystemEnvData* pSysData)
    {

        OSL_TRACE("Canvas::cairo::%s(): flush to NSView",__func__);

        CGContextRef mrContext = cairo_quartz_surface_get_cg_context(mpSurface);

        // write to the NSView in pSysData
        NSView* mpView = (NSView *) pSysData->pView;

    [mpView lockFocus];

    /**
     * This code is using same screen update code as in VCL (esp. AquaSalGraphics::UpdateWindow() )
         */
    CGContextRef rViewContext = reinterpret_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);
    CGImageRef xImage = CGBitmapContextCreateImage (mrContext);
        CGContextDrawImage(rViewContext, CGRectMake( 0, 0, CGImageGetWidth(xImage), CGImageGetHeight(xImage)), xImage);
        CGImageRelease( xImage );
    CGContextFlush( rViewContext );

    [mpView unlockFocus];
    }

}  // namespace cairo

#endif   // CAIRO_HAS_QUARTZ_SURFACE

#endif   // QUARTZ
