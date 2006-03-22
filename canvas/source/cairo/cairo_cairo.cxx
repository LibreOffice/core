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
