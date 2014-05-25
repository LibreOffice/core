/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <X11/extensions/Xrender.h>

// Only for the SystemGraphicsData struct, and hopefully we can find some better
// replacement for that at some point.
#include <vcl/sysdata.hxx>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.hxx>

using namespace ::lok;

static int help()
{
    fprintf( stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install> [path to document]\n" );
    return 1;
}

static GtkWidget* ourCanvas;
static GdkPixbuf* ourPixBuf = 0;

bool drawCallback(GtkWidget* /* The eventbox */, void* /* cairo_t* cr */, gpointer pData)
{
    fprintf(stderr, "attempting to draw tile");

    Document* pDocument = static_cast< Document* >( pData );

    long nWidth, nHeight;
    pDocument->getDocumentSize( &nWidth, &nHeight );

    // Draw the whole document at once (for now)
    int nRenderWidth = nWidth / 10;
    int nRenderHeight = nHeight / 10;
    int nRowStride;

    if ( ourPixBuf &&
            (gdk_pixbuf_get_width( ourPixBuf ) != nRenderWidth ||
             gdk_pixbuf_get_height( ourPixBuf ) != nRenderHeight ) )
    {
        g_object_unref( G_OBJECT( ourPixBuf ) );
        ourPixBuf = 0;

    }
    if (!ourPixBuf)
    {
        ourPixBuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                    true, 8,
                                    nRenderWidth, nRenderHeight);
    }

    unsigned char* pBuffer = gdk_pixbuf_get_pixels( ourPixBuf );

    pDocument->paintTile( pBuffer,
                          nRenderWidth, nRenderHeight,
                          &nRowStride,
                          0, 0, // origin
                          nWidth, nHeight );
    // TODO: double check that the rowstride really matches what we expected,
    // although presumably we'd already be crashing by now if things were
    // wrong.
    (void) nRowStride;

    for (int i = 3; i < nRowStride*nRenderHeight; i += 4)
    {
        pBuffer[i] = 0xFF;
    }

    gtk_image_set_from_pixbuf( GTK_IMAGE( ourCanvas ), ourPixBuf );

    return true;

}

int main( int argc, char* argv[] )
{
    if( argc < 2 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();

    if ( argv[1][0] != '/' )
    {
        fprintf(stderr, "Absolute path required to libreoffice install\n");
        return 1;
    }

    ::lok::Office *pOffice = ::lok::lok_cpp_init( argv[1] );
    if( !pOffice )
    {
        fprintf( stderr, "Failed to initialize\n" );
        return -1;
    }

    ::lok::Document* pDocument = pOffice->documentLoad( argv[2] );

    gtk_init( &argc, &argv );

    GtkWidget *pWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(pWindow), "LibreOffice GTK Tiled Viewer" );
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 800, 600);
    g_signal_connect( pWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL );


    GtkWidget* pScroller = gtk_scrolled_window_new( 0, 0 );
    gtk_container_add( GTK_CONTAINER(pWindow), pScroller );

    GtkWidget* pEventBox = gtk_event_box_new();
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pScroller), pEventBox );

    GtkWidget* pCanvas = gtk_image_new();
    ourCanvas = pCanvas;
    gtk_container_add( GTK_CONTAINER( pEventBox ), pCanvas );

    g_signal_connect( G_OBJECT(pEventBox), "button-press-event", G_CALLBACK(drawCallback), pDocument);

    gtk_widget_show( pCanvas );
    gtk_widget_show( pEventBox );
    gtk_widget_show( pScroller );
    gtk_widget_show( pWindow );

    drawCallback( pCanvas, 0, pDocument );

    gtk_main();

    return 0;
}