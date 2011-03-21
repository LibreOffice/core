/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *               Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jan Holesovsky <kendy@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifdef ENABLE_QUICKSTART_LIBPNG

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define USE_LIBPNG

#include "osl/endian.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_LIBPNG
#  include <png.h>
#endif

#include "splashx.h"

typedef struct {
    unsigned char b, g, r;
} color_t;

#define WINDOW_WIDTH  440
#define WINDOW_HEIGHT 299

#define PROGRESS_XOFFSET 12
#define PROGRESS_YOFFSET 18
#define PROGRESS_BARSPACE 2

static Display *display = NULL;
static int screen;
static int depth;
static Visual *visual = NULL;

static int width = WINDOW_WIDTH;
static int height = WINDOW_HEIGHT;

static Colormap color_map;
static Window win;
static GC gc;

// Progress bar values
// taken from desktop/source/splash/splash.cxx
static int tlx = 212;
static int tly = 216;
static int barwidth = 263;
static int barheight = 8;
static int barspace = PROGRESS_BARSPACE;
static color_t barcol = { 18, 202, 157 };
static color_t framecol = { 0xD3, 0xD3, 0xD3 };

static XColor barcolor;
static XColor framecolor;

static unsigned char **bitmap_rows = NULL;

#define BMP_HEADER_LEN 14
#define WIN_INFO_LEN 40

#define UINT8( x )      ( (unsigned int)( ( (uint8_t *)( x ) )[0] ) )

#define UINT16( x ) (   ( (unsigned int)( ( (uint8_t *)( x ) )[0] ) ) + \
                      ( ( (unsigned int)( ( (uint8_t *)( x ) )[1] ) ) << 8 ) )

#define UINT32( x ) (   ( (unsigned int)( ( (uint8_t *)( x ) )[0] ) ) + \
                      ( ( (unsigned int)( ( (uint8_t *)( x ) )[1] ) ) << 8  ) + \
                      ( ( (unsigned int)( ( (uint8_t *)( x ) )[2] ) ) << 16 ) + \
                      ( ( (unsigned int)( ( (uint8_t *)( x ) )[3] ) ) << 24 ) )

#define MAX( x, y ) ( ( (x) > (y) )? (x): (y) )

#define LOAD_FAILURE( msg ) \
    { \
        fprintf( stderr, "%s: " msg, filename ); \
        close( fd ); \
        return 0; \
    }

#ifdef USE_LIBPNG

/* libpng-1.2.41 */
#ifndef PNG_TRANSFORM_GRAY_TO_RGB
#  define PNG_TRANSFORM_GRAY_TO_RGB   0x2000
#endif

int splash_load_bmp( const char *filename )
{
    FILE *file;
    png_structp png_ptr;
    png_infop info_ptr;

    if ( !(file = fopen( filename, "r" ) ) )
        return 0;

    png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io( png_ptr, file );

    if( setjmp( png_jmpbuf( png_ptr ) ) )
    {
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
        fclose( file );
        return 0;
    }

    png_read_png( png_ptr, info_ptr,
                  PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA |
                  PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_BGR, NULL);

    bitmap_rows = png_get_rows( png_ptr, info_ptr );
    width = png_get_image_width( png_ptr, info_ptr );
    height = png_get_image_height( png_ptr, info_ptr );

#if 0
    {
      int i,j;
      for (j = 0; j < height; j++) {
        for (i = 0; i < width*3; i++) {
          fprintf (stderr, "%.2x", bitmap_rows[j][i]);
        }
        fprintf (stderr, "\n");
      }
    }
#endif

    return 1;
}
#else

/* Load the specified Windows 24bit BMP to 'bitmap'
 * Return: 1 - success, 0 - failure */
int splash_load_bmp( const char *filename )
{
    int fd = open( filename, O_RDONLY );
    if ( fd < 0 )
        return 0;

    char file_header[ BMP_HEADER_LEN ];

    if ( read( fd, file_header, BMP_HEADER_LEN ) != BMP_HEADER_LEN || file_header[0] != 'B' || file_header[1] != 'M' )
        LOAD_FAILURE( "Not a bitmap.\n" );

/*    int file_size = UINT32( file_header + 2 ); */

    char info_header[ WIN_INFO_LEN ];
    if ( read( fd, info_header, 4 ) != 4 )
        LOAD_FAILURE( "Unable to read the header.\n" );

    int header_size = UINT32( info_header );
    if ( header_size != WIN_INFO_LEN )
        LOAD_FAILURE( "Not a Windows bitmap.\n" );

    if ( read( fd, info_header + 4, WIN_INFO_LEN - 4 ) != WIN_INFO_LEN - 4 )
        LOAD_FAILURE( "The header ended too early.\n" );

    width = UINT32( info_header + 4 );
    height = UINT32( info_header + 8 );

    int bits = UINT16( info_header + 14 );
    int compression = UINT16( info_header + 16 );

    if ( bits != 24 )
        LOAD_FAILURE( "Just 24 bpp bitmaps are supported.\n" );

    if ( compression != 0 )
        LOAD_FAILURE( "Just uncompressed bitmaps are supported.\n" );

    size_t bitmap_size = width * height * 3;
    unsigned char *bitmap = malloc( bitmap_size );
    if ( bitmap == NULL )
        LOAD_FAILURE( "Cannot allocate memory for the data.\n" );

    if ( read( fd, bitmap, bitmap_size ) != bitmap_size )
        LOAD_FAILURE( "Cannot read the bitmap data.\n" );

    bitmap_rows = malloc (sizeof (unsigned char*) * height);
    int i;
    for (i = 0; i < height; i++)
        bitmap_rows[i] = bitmap + (width * height * 3) - width * 3 * (i + 1);

    close( fd );
    return 1;
}
#endif

static void setup_color( int val[3], color_t *col )
{
    if ( val[0] < 0 || val[1] < 0 || val[2] < 0 )
        return;

#define CONVERT_COLOR( from,to ) if ( from < 0 ) to = 0; else if ( from > 255 ) to = 255; else to = from;
    CONVERT_COLOR( val[0], col->r );
    CONVERT_COLOR( val[1], col->g );
    CONVERT_COLOR( val[2], col->b );
#undef CONVERT_COLOR
}

// setup
void splash_setup( int barc[3], int framec[3], int posx, int posy, int w, int h )
{
    if ( width <= 500 )
    {
        barwidth  = width - ( 2 * PROGRESS_XOFFSET );
        barheight = 6;
        tlx = PROGRESS_XOFFSET;
        tly = height - PROGRESS_YOFFSET;

        barcol.r = 0;
        barcol.g = 0;
        barcol.b = 128;
    }

    if ( posx >= 0 )
        tlx = posx;
    if ( posy >= 0 )
        tly = posy;
    if ( w >= 0 )
        barwidth = w;
    if ( h >= 0 )
        barheight = h;

    setup_color( barc, &barcol );
    setup_color( framec, &framecol );
}

// Universal shift: bits >= 0 - left, otherwise right
#define SHIFT( x, bits ) ( ( (bits) >= 0 )? ( (x) << (bits) ): ( (x) >> -(bits) ) )

// Position of the highest bit (more or less integer log2)
inline int HIGHEST_BIT( unsigned long x )
{
    int i = 0;
    for ( ; x; ++i )
        x >>= 1;

    return i;
}

// Number of bits set to 1
inline int BITS( unsigned long x )
{
    int i = 0;
    for ( ; x; x >>= 1 )
        if ( x & 1UL )
            ++i;

    return i;
}

// Set 'bitmap' as the background of our 'win' window
static void create_pixmap()
{
    if ( !bitmap_rows )
        return;

    Pixmap pixmap = XCreatePixmap( display, win, width, height, depth );

    unsigned long value_mask = 0;
    XGCValues values;
    GC pixmap_gc = XCreateGC( display, pixmap, value_mask, &values );

    if ( visual->class == TrueColor )
    {
        unsigned long red_mask   = visual->red_mask;
        unsigned long green_mask = visual->green_mask;
        unsigned long blue_mask  = visual->blue_mask;

        unsigned long red_delta_mask   = ( 1UL << ( 8 - BITS( red_mask ) ) ) - 1;
        unsigned long green_delta_mask = ( 1UL << ( 8 - BITS( green_mask ) ) ) - 1;
        unsigned long blue_delta_mask  = ( 1UL << ( 8 - BITS( blue_mask ) ) ) - 1;

        int red_shift   = HIGHEST_BIT( red_mask ) - 8;
        int green_shift = HIGHEST_BIT( green_mask ) - 8;
        int blue_shift  = HIGHEST_BIT( blue_mask ) - 8;

        XImage *image = XCreateImage( display, visual, depth, ZPixmap,
                0, NULL, width, height, 32, 0 );

        int bytes_per_line = image->bytes_per_line;
        int bpp = image->bits_per_pixel;
        int byte_order = image->byte_order;
#if defined( _LITTLE_ENDIAN )
        int machine_byte_order = LSBFirst;
#elif defined( _BIG_ENDIAN )
        int machine_byte_order = MSBFirst;
#else
        {
            fprintf( stderr, "Unsupported machine endianity.\n" );
            XFreeGC( display, pixmap_gc );
            XFreePixmap( display, pixmap );
            XDestroyImage( image );
            return;
        }
#endif

        char *data = malloc( height * bytes_per_line );
        image->data = data;

        // The following dithers & converts the color_t color to one
        // acceptable for the visual
#define COPY_IN_OUT( pix_size, code ) \
        { \
            int x, y; \
            for ( y = 0; y < height; ++y ) \
            { \
                unsigned long red_delta = 0, green_delta = 0, blue_delta = 0; \
                color_t *in = (color_t *)bitmap_rows[y]; \
                for ( x = 0; x < width; ++x, ++in  ) \
                { \
                    unsigned long red   = in->r + red_delta; \
                    unsigned long green = in->g + green_delta; \
                    unsigned long blue  = in->b + blue_delta; \
                    red_delta = red & red_delta_mask; \
                    green_delta = green & green_delta_mask; \
                    blue_delta = blue & blue_delta_mask; \
                    if ( red > 255 ) \
                        red = 255; \
                    if ( green > 255 ) \
                        green = 255; \
                    if ( blue > 255 ) \
                        blue = 255; \
                    unsigned long pixel = \
                        ( SHIFT( red, red_shift ) & red_mask ) | \
                        ( SHIFT( green, green_shift ) & green_mask ) | \
                        ( SHIFT( blue, blue_shift ) & blue_mask ); \
                    code \
                } \
            } \
        }

        char *out = data;

        if ( bpp == 32 )
        {
            if ( machine_byte_order == byte_order )
                COPY_IN_OUT( 4, *( (uint32_t *)out ) = (uint32_t)pixel; out += 4; )
            else
                COPY_IN_OUT( 4, uint32_t tmp = pixel;
                             *( (uint8_t *)out     ) = *( (uint8_t *)(&tmp) + 3 );
                             *( (uint8_t *)out + 1 ) = *( (uint8_t *)(&tmp) + 2 );
                             *( (uint8_t *)out + 2 ) = *( (uint8_t *)(&tmp) + 1 );
                             *( (uint8_t *)out + 3 ) = *( (uint8_t *)(&tmp)     );
                             out += 4; )
        }
        else if ( bpp == 24 )
        {
            if ( machine_byte_order == byte_order && byte_order == LSBFirst )
                COPY_IN_OUT( 3, *( (color_t *)out ) = *( (color_t *)( &pixel ) ); out += 3; )
            if ( machine_byte_order == byte_order && byte_order == MSBFirst )
                COPY_IN_OUT( 3, uint32_t tmp = pixel;
                             *( (uint8_t *)out     ) = *( (uint8_t *)(&tmp) + 1 );
                             *( (uint8_t *)out + 1 ) = *( (uint8_t *)(&tmp) + 2 );
                             *( (uint8_t *)out + 2 ) = *( (uint8_t *)(&tmp) + 3 );
                             out += 3; )
            else
                COPY_IN_OUT( 3, uint32_t tmp = pixel;
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
                COPY_IN_OUT( 2, uint16_t tmp = pixel;
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
            XFreeGC( display, pixmap_gc );
            XFreePixmap( display, pixmap );
            XDestroyImage( image );
            return;
        }

#undef COPY_IN_OUT

        XPutImage( display, pixmap, pixmap_gc, image, 0, 0, 0, 0, width, height );
        XDestroyImage( image );
    }
    else //if ( depth == 1 || visual->class == DirectColor )
    {
        // FIXME Something like the following, but faster ;-) - XDrawPoint is not
        // a good idea...
        int x, y;
        for ( y = 0; y < height; ++y )
        {
            color_t *color = (color_t *)&bitmap_rows[y];

            int delta = 0;
            for ( x = 0; x < width; ++x, ++color )
            {
                int rnd = (int)( ( (long)( random() - RAND_MAX/2 ) * 32000 )/RAND_MAX );
                int luminance = delta + rnd + 299 * (int)color->r + 587 * (int)color->g + 114 * (int)color->b;

                if ( luminance < 128000 )
                {
                    XSetForeground( display, pixmap_gc, BlackPixel( display, screen ) );
                    delta = luminance;
                }
                else
                {
                    XSetForeground( display, pixmap_gc, WhitePixel( display, screen ) );
                    delta = luminance - 255000;
                }

                XDrawPoint( display, pixmap, pixmap_gc, x, y );
            }
        }
    }

    XSetWindowBackgroundPixmap( display, win, pixmap );

    XFreeGC( display, pixmap_gc );
    XFreePixmap( display, pixmap );
}

// The old method of hiding the window decorations
static void suppress_decorations_motif()
{
    struct {
        unsigned long flags, functions, decorations;
        long input_mode;
    } mwmhints;

    Atom a = XInternAtom( display, "_MOTIF_WM_HINTS", False );

    mwmhints.flags = 15; // functions, decorations, input_mode, status
    mwmhints.functions = 2; // ?
    mwmhints.decorations = 0;
    mwmhints.input_mode = 0;

    XChangeProperty( display, win, a, a, 32,
            PropModeReplace, (unsigned char*)&mwmhints, 5 );
}

// This is a splash, set it as such.
// If it fails, just hide the decorations...
static void suppress_decorations()
{
    Atom atom_type = XInternAtom( display, "_NET_WM_WINDOW_TYPE", True );
    Atom atom_splash = XInternAtom( display, "_NET_WM_WINDOW_TYPE_SPLASH", True );

    if ( atom_type != None && atom_splash != None )
        XChangeProperty( display, win, atom_type, XA_ATOM, 32,
                PropModeReplace, (unsigned char*)&atom_splash, 1 );
    //else
        suppress_decorations_motif(); // FIXME: Unconditional until Metacity/compiz's SPLASH handling is fixed
}

// Create the window
// Return: 1 - success, 0 - failure
int splash_create_window( int argc, char** argv )
{
    char *display_name = NULL;
    int i;
    for ( i = 0; i < argc; i++ )
    {
        if ( !strcmp( argv[i], "-display" )  || !strcmp( argv[i], "--display" ) )
            display_name = ( i + 1 < argc )? argv[i+1]: NULL;
    }

    if ( !display_name )
        display_name = getenv( "DISPLAY" );

    // init display
    display = XOpenDisplay( display_name );
    if ( !display )
    {
        fprintf( stderr, "Failed to open display\n" );
        return 0;
    }

    // create the window
    screen = DefaultScreen( display );
    depth = DefaultDepth( display, screen );
    color_map = DefaultColormap( display, screen );
    visual = DefaultVisual( display, screen );

    Window root_win = RootWindow( display, screen );
    int display_width = DisplayWidth( display, screen );
    int display_height = DisplayHeight( display, screen );

    win = XCreateSimpleWindow( display, root_win,
            ( display_width - width ) / 2, ( display_height - height ) / 2,
            width, height, 0,
            BlackPixel( display, screen ), BlackPixel( display, screen ) );

    XSetWindowColormap( display, win, color_map );

    // setup colors
#define FILL_COLOR( xcol,col ) xcol.red = 256*col.r; xcol.green = 256*col.g; xcol.blue = 256*col.b;
    FILL_COLOR( barcolor, barcol );
    FILL_COLOR( framecolor, framecol );
#undef FILL_COLOR

    XAllocColor( display, color_map, &barcolor );
    XAllocColor( display, color_map, &framecolor );

    // not resizable, no decorations, etc.
    unsigned long value_mask = 0;
    XGCValues values;
    gc = XCreateGC( display, win, value_mask, &values );

    XSizeHints size_hints;
    size_hints.flags = PPosition | PSize | PMinSize | PMaxSize;
    size_hints.min_width = width;
    size_hints.max_width = width;
    size_hints.min_height = height;
    size_hints.max_height = height;

    char *name = "OpenOffice.org";
    char *icon = "icon"; // FIXME

    XSetStandardProperties( display, win, name, icon, None,
            0, 0, &size_hints );

    // the actual work
    suppress_decorations();
    create_pixmap();

    // show it
    XSelectInput( display, win, 0 );
    XMapWindow( display, win );

    return 1;
}

// Re-draw & process the events
// Just throwing them away - we do not need anything more...
static void process_events()
{
    XEvent xev;
    int num_events;

    XFlush( display );
    num_events = XPending( display );
    while ( num_events > 0 )
    {
        num_events--;
        XNextEvent( display, &xev );
        //process_event(xev);
    }
}

// Draw the progress
void splash_draw_progress( int progress )
{
    // sanity
    if ( progress < 0 )
        progress = 0;
    if ( progress > 100 )
        progress = 100;

    // draw progress...
    int length = ( progress * barwidth / 100 ) - ( 2 * barspace );
    if ( length < 0 )
        length = 0;

    // border
    XSetForeground( display, gc, framecolor.pixel );
    XDrawRectangle( display, win, gc,
            tlx, tly,
            barwidth, barheight );

    // progress bar
    XSetForeground( display, gc, barcolor.pixel );
    XFillRectangle( display, win, gc,
            tlx + barspace, tly + barspace,
            length + 1, barheight - 2*barspace + 1 );

    // pending events
    process_events();
}

// Close the window & cleanup
void splash_close_window()
{
    XCloseDisplay( display );

    // leak it is faster
    bitmap_rows = NULL;
}

#else /* not ENABLE_QUICKSTART_LIBPNG */

/* Stubs that will never be called in this case */

int splash_load_bmp( const char *filename )
{
    (void)filename;
    return 1;
}
void splash_setup( int barc[3], int framec[3], int posx, int posy, int w, int h )
{
    (void)barc; (void)framec; (void)posx; (void)posy; (void)w; (void)h;
}
int splash_create_window( int argc, char** argv )
{
    (void)argc; (void)argv;
    return 1;
}
void splash_close_window()
{
}
void splash_draw_progress( int progress )
{
    (void)progress;
}

#endif // ENABLE_QUICKSTART_LIBPNG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
