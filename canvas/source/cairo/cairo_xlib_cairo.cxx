/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_xlib_cairo.cxx,v $
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

// NOTE: (when needed) #if defined QUARTZ has to be before UNX.
//       since Mac OS X can have both X11 and QUARTZ versions
//       of cairo libraries at the same time.
#if defined (UNX) && !defined (QUARTZ)

/************************************************************************
 * XLib/Xrender surface backend for OpenOffice.org Cairo Canvas         *
 ************************************************************************/

#include <prex.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>
#include <postx.h>

#include <osl/diagnose.h>

#include "cairo_cairo.hxx"

#ifdef CAIRO_HAS_XLIB_SURFACE

#ifdef USE_CAIRO10_APIS
#include "cairo_xlib_helper.hxx"
#endif

#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

namespace cairo
{

bool HasXRender( const void* pSysData )
{
    Display *pDisplay = (Display*) ((const SystemEnvData*) pSysData)->pDisplay;
    int nDummy;
    return XQueryExtension( pDisplay, "RENDER", &nDummy, &nDummy, &nDummy );
}


#ifdef USE_CAIRO10_APIS
    Surface::Surface( const void* pSysData, void* pDisplay, long hDrawable, void* pRenderFormat, cairo_surface_t* pSurface )
            : mnRefCount( 1 ),
              mpSysData( pSysData ),
              mpDisplay( pDisplay ),
              mhDrawable( hDrawable ),
              mpRenderFormat( pRenderFormat ),
              mbFreePixmap( true ),
              mpSurface( pSurface )
    {
    }
#endif



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
#ifdef USE_CAIRO10_APIS
              mpSysData( NULL ),
              mpDisplay( NULL ),
              mhDrawable( 0 ),
              mpRenderFormat( NULL ),
              mbFreePixmap( false ),
#endif
              mpSurface( pSurface )
    {
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
#ifdef USE_CAIRO10_APIS
          mpSysData( pSysData ),
          mpDisplay( NULL ),
          mhDrawable( 0 ),
          mpRenderFormat( NULL ),
          mbFreePixmap( false ),
#endif
          mpSurface( NULL )
    {
#ifdef USE_CAIRO10_APIS
        mpSurface = (cairo_surface_t*) cairoHelperGetSurface( pSysData, x, y, width, height );
        mpDisplay = (Display*) cairoHelperGetDisplay( pSysData );
        mhDrawable = cairoHelperGetWindow( pSysData );
#else
        mpSurface = cairo_xlib_surface_create( (Display*) pSysData->pDisplay,
                                               pSysData->aWindow,
                                               (Visual*) pSysData->pVisual,
                                               width + x, height + y );
        cairo_surface_set_device_offset( mpSurface, x, y );
        // XSynchronize( (Display*) pSysData->pDisplay, TRUE );
#endif
    }


  /**
   * Surface::Surface:   Create platfrom native Canvas surface from BitmapSystemData
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
   * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
   * @param width width of the new surface
   * @param height height of the new surface
   *
   * Create a surface based on image data on pBmpData
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( const SystemEnvData* pSysData, const BitmapSystemData* pBmpData, int width, int height )
        : mnRefCount( 1 ),
#ifdef USE_CAIRO10_APIS
          mpSysData( pSysData ),
          mpDisplay( NULL ),
          mhDrawable( 0 ),
          mpRenderFormat( NULL ),
          mbFreePixmap( false ),
#endif
          mpSurface( NULL )
    {
#ifdef USE_CAIRO10_APIS
        mpSurface = (cairo_surface_t*) cairoHelperGetSurface( pSysData, pBmpData, width, height );
        mpDisplay = (Display*) cairoHelperGetDisplay( pSysData );
        mhDrawable = cairoHelperGetWindow( pSysData );
#else
        OSL_ASSERT (pBmpData->mnWidth == width && pBmpData->mnHeight == height);

        mpSurface =  cairo_xlib_surface_create( (Display*) pSysData->pDisplay,
                                                (Drawable) pBmpData->aPixmap,
                                                (Visual*) pSysData->pVisual,
                                                width, height );
#endif
    }

#if defined USE_CAIRO10_APIS
  /**
   * Surface::~Surface:  Destroy the Canvas surface
   *
   * Also free any image data and other references related to the Canvas.
   *
   **/
    Surface::~Surface()
    {
        if( mpSurface )
        {
            cairo_surface_destroy( mpSurface );
            mpSurface = NULL;
        }
        mpSysData = NULL;

// In cairo API < 1.2, explicit freeing is needed
        if( mbFreePixmap && mhDrawable )
            XFreePixmap( (Display*) mpDisplay, mhDrawable );
    }
#endif


  /**
   * Surface::Resize:  Resizes the Canvas surface.
   * @param width new width of the surface
   * @param height new height of the surface
   *
   * Only used on X11.
   *
   * @return The new surface or NULL
   **/
    void
    Surface::Resize( int width, int height )
    {
        cairo_xlib_surface_set_size( mpSurface, width, height );
    }


  /**
   * Surface::getDepth:  Get the color depth of the Canvas surface.
   *
   * @return color depth
   **/
    int
    Surface::getDepth()
    {
#ifdef USE_CAIRO10_APIS
        if( mpRenderFormat )
            return ( ( XRenderPictFormat * ) mpRenderFormat )->depth;
#else
        // TODO: verify that this works correctly
        return cairo_xlib_surface_get_depth( mpSurface );
#endif
        return -1;
    }

#ifdef USE_CAIRO10_APIS
// This function is platform dependent on Cairo < 1.2 API only.
    Surface* Surface::getSimilar( Content aContent, int width, int height )
    {
        Pixmap hPixmap;

        if( mpSysData && mpDisplay && mhDrawable ) {
            XRenderPictFormat *pFormat;

            pFormat = XRenderFindStandardFormat( (Display*) mpDisplay, getXFormat(aContent) );
            hPixmap = XCreatePixmap( (Display*) mpDisplay, cairoHelperGetWindow( mpSysData ),
                                     width > 0 ? width : 1, height > 0 ? height : 1,
                                     pFormat->depth );

            return new Surface( mpSysData, mpDisplay, (long) hPixmap, pFormat,
                                cairo_xlib_surface_create_with_xrender_format( (Display*) mpDisplay, hPixmap,
                                                                               DefaultScreenOfDisplay( (Display *) mpDisplay ),
                                                                               pFormat, width, height ) );
        } else
            return new Surface( mpSysData, mpDisplay, 0, NULL, cairo_surface_create_similar( mpSurface, aContent, width, height ) );
    }
#endif   // USE_CAIRO10_APIS

  /**
   * Surface::fillSystemGraphicsData:   Fill SystemGraphicsData with native surface data
   * @param aSystemGraphicsData Platform native system graphics data (struct SystemGraphicsData in vcl/inc/sysdata.hxx)
   *
   */
    void Surface::fillSystemGraphicsData( SystemGraphicsData& aSystemGraphicsData)
    {
    #ifdef USE_CAIRO10_APIS
        // Backward compatibility for Cairo 1.0
        aSystemGraphicsData.hDrawable = mhDrawable;
        aSystemGraphicsData.pRenderFormat = mpRenderFormat;
    #else
        aSystemGraphicsData.hDrawable = cairo_xlib_surface_get_drawable( mpSurface );
        aSystemGraphicsData.pRenderFormat = XRenderFindStandardFormat( cairo_xlib_surface_get_display (mpSurface),
                                                                       getXFormat(cairo_surface_get_content(mpSurface)) );
    #endif //USE_CAIRO10_APIS
    }


    int Surface::getXFormat(Content aContent)
    {
        switch(aContent)
        {
            case CAIRO_CONTENT_ALPHA: return PictStandardA8;     break;
            case CAIRO_CONTENT_COLOR: return PictStandardRGB24;  break;
            case CAIRO_CONTENT_COLOR_ALPHA:
            default:                  return PictStandardARGB32; break;
        }
    }


  /** Surface::flush  Flush the platform native window
   *
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
   *
   **/
    void Surface::flush(const SystemEnvData* pSysData)
    {
#ifdef USE_CAIRO10_APIS
        cairoHelperFlush( pSysData );
#else
        XSync( (Display*) pSysData->pDisplay, false );
#endif
    }


}  // namespace cairo

#endif   // CAIRO_HAS_XLIB_SURFACE

#endif   // defined (UNX) && !defined (QUARTZ)
