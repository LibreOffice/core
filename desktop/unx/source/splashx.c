/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "splashx.h"

#if defined(ENABLE_QUICKSTART_LIBPNG) && !defined(LIBO_HEADLESS)

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#ifdef USE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <osl/endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <png.h>

#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.h>
#include <rtl/ustrbuf.h>

typedef struct {
    unsigned char b, g, r;
} color_t;

struct splash
{
    Display* display;
    int screen;
    int depth;
    int display_width;
    int display_height;
    int display_x_pos;
    int display_y_pos;
    Visual* visual;

    int width;
    int height;

    Colormap color_map;
    Window win;
    GC gc;
    //true when intro-highres loaded successfully
    sal_Bool bHasHiDpiImage;

// Progress bar values
// taken from desktop/source/splash/splash.cxx
    int tlx;
    int tly;
    int barwidth;
    int barheight;
    int barspace;
    color_t barcol;
    color_t framecol;

    XColor barcolor;
    XColor framecolor;

    unsigned char** bitmap_rows;
    png_structp png_ptr;
    png_infop info_ptr;

};

#define WINDOW_WIDTH  440
#define WINDOW_HEIGHT 299

#define PROGRESS_XOFFSET 12
#define PROGRESS_YOFFSET 18
#define PROGRESS_BARSPACE 2

/* libpng-1.2.41 */
#ifndef PNG_TRANSFORM_GRAY_TO_RGB
#  define PNG_TRANSFORM_GRAY_TO_RGB   0x2000
#endif

static int splash_load_bmp( struct splash* splash, const char *filename )
{
    FILE *file;

    if ( !(file = fopen( filename, "r" ) ) )
        return 0;

    splash->png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    splash->info_ptr = png_create_info_struct(splash->png_ptr);
    png_init_io( splash->png_ptr, file );

    if( setjmp( png_jmpbuf( splash->png_ptr ) ) )
    {
        png_destroy_read_struct( &(splash->png_ptr), &(splash->info_ptr), NULL );
        fclose( file );
        return 0;
    }

    png_read_png( splash->png_ptr, splash->info_ptr,
                  PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA |
                  PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_BGR, NULL);

    splash->bitmap_rows = png_get_rows( splash->png_ptr, splash->info_ptr );
    splash->width = png_get_image_width( splash->png_ptr, splash->info_ptr );
    splash->height = png_get_image_height( splash->png_ptr, splash->info_ptr );

    fclose( file );
    return 1;
}

static void setup_color( int const val[3], color_t *col )
{
    if ( val[0] < 0 || val[1] < 0 || val[2] < 0 )
        return;

#define CONVERT_COLOR( from,to ) if ( from < 0 ) to = 0; else if ( from > 255 ) to = 255; else to = from;
    CONVERT_COLOR( val[0], col->r );
    CONVERT_COLOR( val[1], col->g );
    CONVERT_COLOR( val[2], col->b );
#undef CONVERT_COLOR
}

/* Fill 'array' with values of the key 'name'.
   Its value is a comma delimited list of integers */
static void get_bootstrap_value( int *array, int size, rtlBootstrapHandle handle, const char *name )
{
    rtl_uString *pKey = NULL, *pValue = NULL;

    /* get the value from the ini file */
    rtl_uString_newFromAscii( &pKey, name );
    rtl_bootstrap_get_from_handle( handle, pKey, &pValue, NULL );

    /* the value is several numbers delimited by ',' - parse it */
    if ( rtl_uString_getLength( pValue ) > 0 )
    {
        rtl_uString *pToken = NULL;
        int i = 0;
        sal_Int32 nIndex = 0;
        for ( ; ( nIndex >= 0 ) && ( i < size ); ++i )
        {
            nIndex = rtl_uString_getToken( &pToken, pValue, 0, ',', nIndex );
            array[i] = rtl_ustr_toInt32( rtl_uString_getStr( pToken ), 10 );
        }

        rtl_uString_release( pToken );
    }

    /* cleanup */
    rtl_uString_release( pKey );
    rtl_uString_release( pValue );
}

// setup
static void splash_setup( struct splash* splash, int const barc[3], int const framec[3], int posx, int posy, int w, int h )
{
    if ( splash->width <= 500 )
    {
        splash->barwidth  = splash->width - ( 2 * PROGRESS_XOFFSET );
        splash->barheight = 6;
        splash->tlx = PROGRESS_XOFFSET;
        splash->tly = splash->height - PROGRESS_YOFFSET;

        splash->barcol.r = 0;
        splash->barcol.g = 0;
        splash->barcol.b = 128;
    }

    if ( posx >= 0 )
        splash->tlx = posx;
    if ( posy >= 0 )
        splash->tly = posy;
    if ( w >= 0 )
        splash->barwidth = w;
    if ( h >= 0 )
        splash->barheight = h;

    setup_color( barc, &(splash->barcol) );
    setup_color( framec, &(splash->framecol) );
}

// Universal shift: bits >= 0 - left, otherwise right
#define SHIFT( x, bits ) ( ( (bits) >= 0 )? ( (x) << (bits) ): ( (x) >> -(bits) ) )

// Position of the highest bit (more or less integer log2)
static inline int HIGHEST_BIT( unsigned long x )
{
    int i = 0;
    for ( ; x; ++i )
        x >>= 1;

    return i;
}

// Number of bits set to 1
static inline int BITS( unsigned long x )
{
    int i = 0;
    for ( ; x; x >>= 1 )
        if ( x & 1UL )
            ++i;

    return i;
}

// Set 'bitmap' as the background of our 'win' window
static void create_pixmap(struct splash* splash)
{
    Pixmap pixmap;
    GC pixmap_gc;
    XGCValues values;

    if ( !splash->bitmap_rows )
    {
        return;
    }
    pixmap = XCreatePixmap( splash->display, splash->win, splash->width, splash->height, splash->depth );

    pixmap_gc = XCreateGC( splash->display, pixmap, 0/*value_mask*/, &values );

    if ( splash->visual->class == TrueColor )
    {
        const unsigned long red_mask   = splash->visual->red_mask;
        const unsigned long green_mask = splash->visual->green_mask;
        const unsigned long blue_mask  = splash->visual->blue_mask;

        const unsigned long red_delta_mask   = ( 1UL << ( 8 - BITS( red_mask ) ) ) - 1;
        const unsigned long green_delta_mask = ( 1UL << ( 8 - BITS( green_mask ) ) ) - 1;
        const unsigned long blue_delta_mask  = ( 1UL << ( 8 - BITS( blue_mask ) ) ) - 1;

        const int red_shift   = HIGHEST_BIT( red_mask ) - 8;
        const int green_shift = HIGHEST_BIT( green_mask ) - 8;
        const int blue_shift  = HIGHEST_BIT( blue_mask ) - 8;

        XImage* image = XCreateImage( splash->display, splash->visual, splash->depth, ZPixmap,
                                      0, NULL, splash->width, splash->height, 32, 0 );

        const int bytes_per_line = image->bytes_per_line;
        const int bpp = image->bits_per_pixel;
        const int byte_order = image->byte_order;
#if defined OSL_LITENDIAN
        const int machine_byte_order = LSBFirst;
#else /* OSL_BIGENDIAN */
        const int machine_byte_order = MSBFirst;
#endif

        char *data = malloc( splash->height * bytes_per_line );
        char *out = data;
        image->data = data;

        // The following dithers & converts the color_t color to one
        // acceptable for the visual
#define COPY_IN_OUT( pix_size, code ) \
        { \
            int x, y; \
            for ( y = 0; y < splash->height; ++y ) \
            { \
                unsigned long red_delta = 0, green_delta = 0, blue_delta = 0; \
                color_t *in = (color_t *)(splash->bitmap_rows[y]);      \
                out = data + y * bytes_per_line; \
                for ( x = 0; x < splash->width; ++x, ++in  ) \
                { \
                    unsigned long red   = in->r + red_delta; \
                    unsigned long green = in->g + green_delta; \
                    unsigned long blue  = in->b + blue_delta; \
                    unsigned long pixel = 0; \
                    uint32_t tmp = 0; \
                    (void) tmp; \
                    red_delta = red & red_delta_mask; \
                    green_delta = green & green_delta_mask; \
                    blue_delta = blue & blue_delta_mask; \
                    if ( red > 255 ) \
                        red = 255; \
                    if ( green > 255 ) \
                        green = 255; \
                    if ( blue > 255 ) \
                        blue = 255; \
                    pixel = \
                        ( SHIFT( red, red_shift ) & red_mask ) | \
                        ( SHIFT( green, green_shift ) & green_mask ) | \
                        ( SHIFT( blue, blue_shift ) & blue_mask ); \
                    code \
                } \
            } \
        }

        if ( bpp == 32 )
        {
            if ( machine_byte_order == byte_order )
                COPY_IN_OUT( 4, *( (uint32_t *)out ) = (uint32_t)pixel; out += 4; )
            else
                COPY_IN_OUT( 4, tmp = pixel;
                             *( (uint8_t *)out     ) = *( (uint8_t *)(&tmp) + 3 );
                             *( (uint8_t *)out + 1 ) = *( (uint8_t *)(&tmp) + 2 );
                             *( (uint8_t *)out + 2 ) = *( (uint8_t *)(&tmp) + 1 );
                             *( (uint8_t *)out + 3 ) = *( (uint8_t *)(&tmp)     );
                             out += 4; )
        }
        else if ( bpp == 24 )
        {
            if (machine_byte_order == byte_order)
            {
#if defined OSL_LITENDIAN
                COPY_IN_OUT( 3, memcpy(out, &pixel, sizeof (color_t)); out += 3; )
#else /* OSL_BIGENDIAN */
                COPY_IN_OUT( 3, tmp = pixel;
                             *( (uint8_t *)out     ) = *( (uint8_t *)(&tmp) + 1 );
                             *( (uint8_t *)out + 1 ) = *( (uint8_t *)(&tmp) + 2 );
                             *( (uint8_t *)out + 2 ) = *( (uint8_t *)(&tmp) + 3 );
                             out += 3; )
#endif
            }
            else
                COPY_IN_OUT( 3, tmp = pixel;
                             *( (uint8_t *)out     ) = *( (uint8_t *)(&tmp) + 3 );
                             *( (uint8_t *)out + 1 ) = *( (uint8_t *)(&tmp) + 2 );
                             *( (uint8_t *)out + 2 ) = *( (uint8_t *)(&tmp) + 1 );
                             out += 3; )
        }
        else if ( bpp == 16 )
        {
            if ( machine_byte_order == byte_order )
                COPY_IN_OUT( 2, *( (uint16_t *)out ) = (uint16_t)pixel; out += 2; )
            else
                COPY_IN_OUT( 2, tmp = pixel;
                             *( (uint8_t *)out     ) = *( (uint8_t *)(&tmp) + 1 );
                             *( (uint8_t *)out + 1 ) = *( (uint8_t *)(&tmp)     );
                             out += 2; );
        }
        else if ( bpp == 8 )
        {
            COPY_IN_OUT( 1, *( (uint8_t *)out ) = (uint8_t)pixel; ++out; )
        }
        else
        {
            fprintf( stderr, "Unsupported depth: %d bits per pixel.\n", bpp );
            XFreeGC( splash->display, pixmap_gc );
            XFreePixmap( splash->display, pixmap );
            XDestroyImage( image );
            return;
        }

#undef COPY_IN_OUT

        XPutImage( splash->display, pixmap, pixmap_gc, image, 0, 0, 0, 0, splash->width, splash->height );
        XDestroyImage( image );
    }

    XSetWindowBackgroundPixmap( splash->display, splash->win, pixmap );

    XFreeGC( splash->display, pixmap_gc );
    XFreePixmap( splash->display, pixmap );
}

// The old method of hiding the window decorations
static void suppress_decorations_motif(struct splash* splash)
{
    struct
    {
        unsigned long flags, functions, decorations;
        long input_mode;
    } mwmhints;

    Atom a = XInternAtom( splash->display, "_MOTIF_WM_HINTS", False );

    mwmhints.flags = 15; // functions, decorations, input_mode, status
    mwmhints.functions = 2; // ?
    mwmhints.decorations = 0;
    mwmhints.input_mode = 0;

    XChangeProperty( splash->display, splash->win, a, a, 32,
                     PropModeReplace, (unsigned char*)&mwmhints, 5 );
}

// This is a splash, set it as such.
// If it fails, just hide the decorations...
static void suppress_decorations(struct splash* splash)
{
    Atom atom_type = XInternAtom( splash->display, "_NET_WM_WINDOW_TYPE", True );
    Atom atom_splash = XInternAtom( splash->display, "_NET_WM_WINDOW_TYPE_SPLASH", True );

    if ( atom_type != None && atom_splash != None )
        XChangeProperty( splash->display, splash->win, atom_type, XA_ATOM, 32,
                         PropModeReplace, (unsigned char*)&atom_splash, 1 );
    //else
    suppress_decorations_motif(splash); // FIXME: Unconditional until Metacity/compiz's SPLASH handling is fixed
}

/**
 * Connects to the display and initializes splash with the screen details
 *
 * @return Success: 1; Failure: 0
 */
static int splash_init_display( struct splash* splash, int argc, char** argv )
{
    char *display_name = NULL;
    int i;
#ifdef USE_XINERAMA
    int n_xinerama_screens = 1;
    XineramaScreenInfo* p_screens = NULL;
#endif

    for ( i = 0; i < argc; i++ )
    {
        if ( !strcmp( argv[i], "-display" )  || !strcmp( argv[i], "--display" ) )
        {
            display_name = ( i + 1 < argc )? argv[i+1]: NULL;
        }
    }

    if ( !display_name )
    {
        display_name = getenv( "DISPLAY" );
    }
    // init display
    splash->display = XOpenDisplay( display_name );
    if ( !splash->display )
    {
        fprintf( stderr, "Failed to open display\n" );
        return 0;
    }

    // create the window
    splash->screen = DefaultScreen( splash->display );
    splash->depth = DefaultDepth( splash->display, splash->screen );
    splash->color_map = DefaultColormap( splash->display, splash->screen );
    splash->visual = DefaultVisual( splash->display, splash->screen );

    splash->display_width = DisplayWidth( splash->display, splash->screen );
    splash->display_height = DisplayHeight( splash->display, splash->screen );
    splash->display_x_pos = 0;
    splash->display_y_pos = 0;

#ifdef USE_XINERAMA
    p_screens = XineramaQueryScreens( splash->display, &n_xinerama_screens );
    if( p_screens )
    {
        for( i=0; i < n_xinerama_screens; i++ )
        {
            if ( p_screens[i].screen_number == splash->screen )
            {
                splash->display_width = p_screens[i].width;
                splash->display_height = p_screens[i].height;
                splash->display_x_pos = p_screens[i].x_org;
                splash->display_y_pos = p_screens[i].y_org;
                break;
            }
        }
        XFree( p_screens );
    }
#endif
    return 1;
}

/**
 * Create the window for the splash screen
 *
 * @return Success: 1; Failure: 0
 */
static int splash_create_window(struct splash* splash)
{
    Window root_win;
    XGCValues values;
    const char* name = "LibreOffice";
    const char* icon = "icon"; // FIXME
    XSizeHints size_hints;

    root_win = RootWindow( splash->display, splash->screen );

    splash->win = XCreateSimpleWindow( splash->display, root_win,
            (splash->display_x_pos + (splash->display_width - splash->width)/2),
            (splash->display_y_pos + (splash->display_height - splash->height)/2),
            splash->width, splash->height, 0,
            BlackPixel( splash->display, splash->screen ), BlackPixel( splash->display, splash->screen ) );

    XSetWindowColormap( splash->display, splash->win, splash->color_map );

    // setup colors
#define FILL_COLOR( xcol,col ) xcol.red = 256*col.r; xcol.green = 256*col.g; xcol.blue = 256*col.b;
    FILL_COLOR( splash->barcolor, splash->barcol );
    FILL_COLOR( splash->framecolor, splash->framecol );
#undef FILL_COLOR

    XAllocColor( splash->display, splash->color_map, &(splash->barcolor) );
    XAllocColor( splash->display, splash->color_map, &(splash->framecolor) );

    // not resizable, no decorations, etc.
    splash->gc = XCreateGC( splash->display, splash->win, 0/*value_mask*/, &values );

    size_hints.flags = PPosition | PSize | PMinSize | PMaxSize;
    size_hints.x = splash->display_x_pos;
    size_hints.y = splash->display_y_pos;
    size_hints.width = splash->width;
    size_hints.height = splash->height;
    size_hints.min_width = splash->width;
    size_hints.max_width = splash->width;
    size_hints.min_height = splash->height;
    size_hints.max_height = splash->height;

    XSetStandardProperties( splash->display, splash->win, name, icon, None,
            NULL, 0, &size_hints );

    // the actual work
    suppress_decorations(splash);
    create_pixmap(splash);

    // show it
    XSelectInput( splash->display, splash->win, 0 );
    XMapWindow( splash->display, splash->win );

    return 1;
}

// Re-draw & process the events
// Just throwing them away - we do not need anything more...
static void process_events(struct splash* splash)
{
    XEvent xev;
    int num_events;

    XFlush( splash->display );
    num_events = XPending( splash->display );
    while ( num_events > 0 )
    {
        num_events--;
        XNextEvent( splash->display, &xev );
    }
}


static rtl_String* ustr_to_str( rtl_uString* pStr )
{
    rtl_String *pOut = NULL;

    rtl_uString2String( &pOut, rtl_uString_getStr( pStr ),
                        rtl_uString_getLength( pStr ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );

    return pOut;
}

static sal_Bool isHiDPI(struct splash* splash)
{
    const char* pValStr;
    double nDPI;

    /*
     * GNOME currently enables hi-dpi support when the screen resolution is at least 192 dpi
     * and the screen height (in device pixels) is at least 1200.
     */

    if (splash->display_height < 1200)
        return sal_False;

    pValStr = XGetDefault(splash->display, "Xft", "dpi");
    /* if it's too old to have this, assume it's not hidpi */
    if (!pValStr)
        return sal_False;

    nDPI = strtod(pValStr, NULL);
    if (nDPI < 192)
        return sal_False;

    return sal_True;
}

#define IMG_SUFFIX           ".png"

static void splash_load_image( struct splash* splash, rtl_uString* pUAppPath )
{
    /* FIXME-BCP47: if we wanted to support language tags here that would get
     * complicated, this is C-source not C++ so LanguageTag can't be used. For
     * now the splash screen will have to get along with language-territory. */

    char *pBuffer, *pSuffix, *pLocale;
    int nLocSize;
    rtl_Locale *pLoc = NULL;
    rtl_String *pLang, *pCountry, *pAppPath;

    osl_getProcessLocale (&pLoc);
    pLang = ustr_to_str (pLoc->Language);
    pCountry = ustr_to_str (pLoc->Country);

    nLocSize = strlen (pLang->buffer) + strlen (pCountry->buffer) + 3;
    pLocale = malloc (nLocSize);
    pLocale[0] = '-';
    strcpy (pLocale + 1, pLang->buffer);
    strcat (pLocale, "_");
    strcat (pLocale, pCountry->buffer);

    rtl_string_release( pCountry );
    rtl_string_release( pLang );

    pAppPath = ustr_to_str (pUAppPath);
    pBuffer = malloc (pAppPath->length + nLocSize + 256);
    strcpy (pBuffer, pAppPath->buffer);
    pSuffix = pBuffer + pAppPath->length;
    rtl_string_release( pAppPath );

    strcpy (pSuffix, "intro");
    strcat (pSuffix, pLocale);
    strcat (pSuffix, IMG_SUFFIX);
    if ( splash_load_bmp( splash, pBuffer ) )
        goto cleanup; /* success */

    /* load high resolution splash image */
    splash->bHasHiDpiImage = sal_False;
    if (isHiDPI(splash))
    {
        strcpy (pSuffix, "intro-highres" IMG_SUFFIX);
        if ( splash_load_bmp( splash, pBuffer ) )
        {
            splash->bHasHiDpiImage = sal_True;
            goto cleanup; /* success */
        }
    }
    /* load standard resolution splash image */
    strcpy (pSuffix, "intro" IMG_SUFFIX);
    if ( splash_load_bmp( splash, pBuffer ) )
        goto cleanup;   /* success */

    fprintf (stderr, "Failed to find intro image\n");

 cleanup:
    free (pLocale);
    free (pBuffer);
}

/* Load the colors and size of the splash. */
static void splash_load_defaults( struct splash* splash, rtl_uString* pAppPath, sal_Bool* bNoDefaults )
{
    rtl_uString *pSettings = NULL, *pTmp = NULL;
    rtlBootstrapHandle handle;
    int logo[1] =  { -1 },
        bar[3] =   { -1, -1, -1 },
        frame[3] = { -1, -1, -1 },
        pos[2] =   { -1, -1 },
        size[2] =  { -1, -1 };

    /* construct the sofficerc file location */
    rtl_uString_newFromAscii( &pSettings, "file://" );
    rtl_uString_newConcat( &pSettings, pSettings, pAppPath );
    rtl_uString_newConcat( &pSettings, pSettings, pTmp );
    rtl_uString_newFromAscii( &pTmp, SAL_CONFIGFILE( "soffice" ) );
    rtl_uString_newConcat( &pSettings, pSettings, pTmp );

    /* use it as the bootstrap file */
    handle = rtl_bootstrap_args_open( pSettings );

    /* get the values */
    get_bootstrap_value( logo,  1, handle, "Logo" );
    get_bootstrap_value( bar,   3, handle, "ProgressBarColor" );
    get_bootstrap_value( frame, 3, handle, "ProgressFrameColor" );
    if (isHiDPI(splash) && splash->bHasHiDpiImage)
    {
       get_bootstrap_value( pos,   2, handle, "ProgressPositionHigh" );
       get_bootstrap_value( size,  2, handle, "ProgressSizeHigh" );
    }
    else
    {
       get_bootstrap_value( pos,   2, handle, "ProgressPosition" );
       get_bootstrap_value( size,  2, handle, "ProgressSize" );
    }

    if ( logo[0] == 0 )
    {
        *bNoDefaults = sal_True;
    }

    splash_setup( splash, bar, frame, pos[0], pos[1], size[0], size[1] );

    /* cleanup */
    rtl_bootstrap_args_close( handle );
    rtl_uString_release( pSettings );
    rtl_uString_release( pTmp );
}


// Draw the progress
void splash_draw_progress( struct splash* splash, int progress )
{
    int length = 0;

    if (!splash)
    {
        return;
    }
    // sanity
    if ( progress < 0 )
    {
        progress = 0;
    }
    if ( progress > 100 )
    {
        progress = 100;
    }
    // draw progress...
    length = ( progress * splash->barwidth / 100 ) - ( 2 * splash->barspace );
    if ( length < 0 )
    {
        length = 0;
    }
    // border
    XSetForeground( splash->display, splash->gc, splash->framecolor.pixel );
    XDrawRectangle( splash->display, splash->win, splash->gc, splash->tlx, splash->tly,
            splash->barwidth, splash->barheight );

    // progress bar
    XSetForeground( splash->display, splash->gc, splash->barcolor.pixel );
    XFillRectangle( splash->display, splash->win, splash->gc,
            splash->tlx + splash->barspace, splash->tly + splash->barspace,
            length + 1, splash->barheight - 2 * splash->barspace + 1 );

    // pending events
    process_events(splash);
}

void splash_destroy(struct splash* splash)
{
    if(splash)
    {
        if(splash->display)
        {
            if(splash->gc)
            {
                XFreeGC(splash->display, splash->gc);
                splash->gc = NULL;
            }

            XCloseDisplay( splash->display );
            splash->display = NULL;
            png_destroy_read_struct( &(splash->png_ptr), &(splash->info_ptr), NULL );
        }
        free(splash);
    }
}

struct splash* splash_create(rtl_uString* pAppPath, int argc, char** argv)
{
    struct splash* splash;
    sal_Bool bNoDefaults = sal_False;

    splash = calloc(1, sizeof(struct splash));
    if (splash && !splash_init_display(splash, argc, argv))
    {
        splash_destroy(splash);
        splash = NULL;
    }

    if (!splash)
        return NULL;

    splash->width = WINDOW_WIDTH;
    splash->height = WINDOW_HEIGHT;

    splash->tlx = 212;
    splash->tly = 216;
    splash->barwidth = 263;
    splash->barheight = 8;
    splash->barspace = PROGRESS_BARSPACE;
    splash->barcol.b = 18;
    splash->barcol.g = 202;
    splash->barcol.r = 157;
    splash->framecol.b = 0xD3;
    splash->framecol.g = 0xD3;
    splash->framecol.r = 0xD3;

    splash_load_image( splash, pAppPath );
    splash_load_defaults( splash, pAppPath, &bNoDefaults );

    if (!bNoDefaults && splash_create_window(splash))
    {
        splash_draw_progress( splash, 0 );
        return splash;
    }

    splash_destroy(splash);
    return NULL;
}

#else /* not ENABLE_QUICKSTART_LIBPNG */

struct splash
{
};

/* Stubs that will never be called in this case */
void splash_draw_progress( struct splash* splash, int progress )
{
    (void)splash; (void)progress;
}

void splash_destroy(struct splash* splash)
{
    (void)splash;
}

struct splash* splash_create(rtl_uString* pAppPath, int argc, char** argv)
{
    (void)pAppPath; (void)argc; (void)argv;
    return NULL;
}


#endif // ENABLE_QUICKSTART_LIBPNG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
