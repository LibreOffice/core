/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_cairo.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:17:37 $
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

    Surface::Surface( const void* pSysData, int x, int y, int width, int height )
        : mnRefCount( 1 ),
          mpSysData( pSysData ),
          mbFreePixmap( false )
    {
        mpSurface = (cairo_surface_t*) cairoHelperGetSurface( pSysData, x, y, width, height );
        mpDisplay = (Display*) cairoHelperGetDisplay( pSysData );
        mhDrawable = cairoHelperGetWindow( pSysData );
    }

    Surface::Surface( const void* pSysData, void *pBmpData, int width, int height )
        : mnRefCount( 1 ),
          mpSysData( pSysData ),
          mbFreePixmap( false )
    {
        mpSurface = (cairo_surface_t*) cairoHelperGetSurface( pSysData, pBmpData, width, height );
        mpDisplay = (Display*) cairoHelperGetDisplay( pSysData );
        mhDrawable = cairoHelperGetWindow( pSysData );
    }


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

    void
    Surface::Resize( int width, int height )
    {
        cairo_xlib_surface_set_size( mpSurface, width, height );
    }

    int
    Surface::getDepth()
    {
        if( mpRenderFormat )
            return ( ( XRenderPictFormat * ) mpRenderFormat )->depth;

        return -1;
    }
}
