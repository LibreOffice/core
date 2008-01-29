/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_cairo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:01:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"
#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>
#include "cairo_cairo.hxx"
#include "cairo_helper.hxx"

namespace cairo
{

#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

    bool HasXRender( const void* pSysData )
    {
            Display *pDisplay = (Display*) cairoHelperGetDisplay( pSysData );
            int nDummy;
            return XQueryExtension( pDisplay, "RENDER", &nDummy, &nDummy, &nDummy );
    }

  /**
   * Surface::Surface:   Create Canvas surface with existing data
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
   * @param pSurface Cairo surface
   *
   * pSysData contains the platform native Window reference
   * This constructor only stores data, it does no processing.
   * It is used by e.g. Surface::getSimilar()
   *
   * Set the mpSurface as pSurface
   **/
    Surface::Surface( const void* pSysData, void* pDisplay, long hDrawable, void* pRenderFormat, cairo_surface_t* pSurface )
        : mpSysData( pSysData ),
          mpDisplay( pDisplay ),
          mhDrawable( hDrawable ),
          mpRenderFormat( pRenderFormat ),
          mnRefCount( 1 ),
          mbFreePixmap( true ),
          mpSurface( pSurface )
    {
    }


  /**
   * Surface::Surface:     Create generic Canvas surface using given Cairo Surface
   *
   * @param pSurface Cairo Surface
   *
   * This constructor only stores data, it does no processing.
   * It is used with e.g. cairo_image_surface_create_for_data()
   * Unlike other constructors, mpSysData is set to NULL
   *
   * Set the mpSurface as pSurface
   **/
    Surface::Surface( cairo_surface_t* pSurface )
        : mpSysData( NULL ),
          mpDisplay( NULL ),
          mhDrawable( 0 ),
          mpRenderFormat( NULL ),
          mnRefCount( 1 ),
          mbFreePixmap( false ),
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
   *
   * pSysData is used to create a surface on the Window
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( const void* pSysData, int x, int y, int width, int height )
        : mpSysData( pSysData ),
          mpDisplay( NULL ),
          mhDrawable( 0 ),
          mpRenderFormat( NULL ),
          mnRefCount( 1 ),
          mbFreePixmap( false ),
          mpSurface( NULL )
    {
        mpSurface = (cairo_surface_t*) cairoHelperGetSurface( pSysData, x, y, width, height );
        mpDisplay = (Display*) cairoHelperGetDisplay( pSysData );
        mhDrawable = cairoHelperGetWindow( pSysData );
    }


  /**
   * Surface::Surface:   Create platfrom native Canvas surface from BitmapSystemData
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
   * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
   * @param width width of the new surface
   * @param height height of the new surface
   *
   * The pBmpData provides the imagedata that the created surface should contain.
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( const void* pSysData, void *pBmpData, int width, int height )
        : mpSysData( pSysData ),
          mpDisplay( NULL ),
          mhDrawable( 0 ),
          mpRenderFormat( NULL ),
          mnRefCount( 1 ),
          mbFreePixmap( false ),
          mpSurface( NULL )
    {
        mpSurface = (cairo_surface_t*) cairoHelperGetSurface( pSysData, pBmpData, width, height );
        mpDisplay = (Display*) cairoHelperGetDisplay( pSysData );
        mhDrawable = cairoHelperGetWindow( pSysData );
    }


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
        if( mbFreePixmap && mhDrawable )
            XFreePixmap( (Display*) mpDisplay, mhDrawable );
    }

  /**
   * Surface::getCairo:  Create Cairo (drawing object) for the Canvas surface
   *
   * @return new Cairo or NULL
   **/
    Cairo*
    Surface::getCairo()
    {
        return cairo_create( mpSurface );
    }

  /**
   * Surface::getSimilar:  Create new similar Canvas surface
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
    Surface* Surface::getSimilar( Content aContent, int width, int height )
    {
        Pixmap hPixmap;

        if( mpSysData && mpDisplay && mhDrawable ) {
            XRenderPictFormat *pFormat;
            int nFormat;

            switch (aContent) {
            case CAIRO_CONTENT_ALPHA:
                nFormat = PictStandardA8;
                break;
            case CAIRO_CONTENT_COLOR:
                nFormat = PictStandardRGB24;
                break;
            case CAIRO_CONTENT_COLOR_ALPHA:
            default:
                nFormat = PictStandardARGB32;
                break;
            }

            pFormat = XRenderFindStandardFormat( (Display*) mpDisplay, nFormat );
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
        if( mpRenderFormat )
            return ( ( XRenderPictFormat * ) mpRenderFormat )->depth;

        return -1;
    }
}
