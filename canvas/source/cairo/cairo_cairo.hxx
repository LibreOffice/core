#ifndef _CAIROCANVAS_CAIRO_HXX
#define _CAIROCANVAS_CAIRO_HXX

namespace cairo {

#include <cairo.h>

    typedef cairo_t Cairo;
    typedef cairo_matrix_t Matrix;
    typedef cairo_format_t Format;
    typedef cairo_content_t Content;
    typedef cairo_pattern_t Pattern;

    class Surface {
        const void* mpSysData;
        void* mpDisplay;
        long mhDrawable;
        void *mpRenderFormat;
        int mnRefCount;
        bool mbFreePixmap;
    public:
        cairo_surface_t* mpSurface;

        Surface( const void* pSysData, void* pDisplay, long hDrawable, void* pRenderFormat, cairo_surface_t* pSurface )
            : mpSysData( pSysData ),
              mpDisplay( pDisplay ),
              mhDrawable( hDrawable ),
              mpRenderFormat( pRenderFormat ),
              mpSurface( pSurface ),
              mbFreePixmap( true ),
              mnRefCount( 1 )
        {
        }

        Surface( cairo_surface_t* pSurface )
            : mpSurface( pSurface ),
              mpDisplay( NULL ),
              mhDrawable( 0 ),
              mpSysData( NULL ),
              mpRenderFormat( NULL ),
              mbFreePixmap( false ),
              mnRefCount( 1 )
        {
        }

        Surface( const void* pSysData, int x, int y, int width, int height );
        Surface( const void* pSysData, void *pBmpData, int width, int height );


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

        Cairo* getCairo()
        {
            return cairo_create( mpSurface );
        }

        Surface* getSimilar( Content aContent, int width, int height );

        long getPixmap()
        {
            return mhDrawable;
        }

        void* getRenderFormat()
        {
            return mpRenderFormat;
        }

        // use only for surfaces made on X Drawables
        void Resize( int width, int height );

        int getDepth();
    };
}

#endif
