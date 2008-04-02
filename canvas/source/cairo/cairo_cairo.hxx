#ifndef _CAIROCANVAS_CAIRO_HXX
#define _CAIROCANVAS_CAIRO_HXX

#ifdef QUARTZ
// needed because sysdata.hxx contains native NS* classes
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#include <cairo.h>

#ifdef CAIRO_HAS_WIN32_SURFACE
// needed because sysdata.hxx contains native types
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#undef min
#undef max
#endif

#include <vcl/virdev.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/bitmap.hxx>

#include <stdio.h>

namespace cairo {

// There are some different code paths with Cairo < 1.2.0 (only supported on XLib)
#if (CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 2, 0))
#define USE_CAIRO10_APIS 1
#endif

    typedef cairo_t Cairo;
    typedef cairo_matrix_t Matrix;
    typedef cairo_format_t Format;
    typedef cairo_content_t Content;
    typedef cairo_pattern_t Pattern;

#if defined(CAIRO_HAS_XLIB_SURFACE) && !defined (QUARTZ)
     bool HasXRender( const void* pSysData );
#endif

    class Surface {
        int mnRefCount;
#if defined QUARTZ
        // nothing needed
#elif defined (CAIRO_HAS_XLIB_SURFACE)
    #ifdef USE_CAIRO10_APIS
        const void* mpSysData;
        void* mpDisplay;
        long mhDrawable;
        void *mpRenderFormat;
        bool mbFreePixmap;
    #endif
#elif defined (CAIRO_HAS_WIN32_SURFACE)
        // nothing needed
#endif
    public:
        cairo_surface_t* mpSurface;

#if defined(USE_CAIRO10_APIS) && defined(CAIRO_HAS_XLIB_SURFACE) && !defined(QUARTZ)
        Surface( const void* pSysData, void* pDisplay, long hDrawable, void* pRenderFormat, cairo_surface_t* pSurface );
#endif

        explicit Surface( cairo_surface_t* pSurface );
        Surface( const SystemEnvData* pSysData, int x, int y, int width, int height );
        Surface( const SystemEnvData* pSysData, const BitmapSystemData* pBmpData, int width, int height );

        ~Surface();

        void Ref()
        {
            mnRefCount ++;
        }

        void Unref()
        {
            mnRefCount --;
            if( mnRefCount <= 0 )
                delete this;
        }

        Cairo* getCairo();

        Surface* getSimilar( Content aContent, int width, int height );

        VirtualDevice* createVirtualDevice();
        void fillSystemGraphicsData( SystemGraphicsData& data );

        int getDepth();


#if defined(CAIRO_HAS_XLIB_SURFACE) && !defined (QUARTZ)
    #if defined USE_CAIRO10_APIS
        long getPixmap()
        {
            return mhDrawable;
        }

        void* getRenderFormat()
        {
            return mpRenderFormat;
        }
    #endif

        int getXFormat( Content aContent );

        // use only for surfaces made on X Drawables
        void Resize( int width, int height );
#endif


#ifdef UNX
        // Only used by Xlib and the current Mac OS X Quartz implementation
        void flush(const SystemEnvData* pSysData);
#endif

    };  // class Surface

} // namespace cairo

#endif
