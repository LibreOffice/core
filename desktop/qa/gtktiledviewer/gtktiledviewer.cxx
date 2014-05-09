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


bool drawCallback(GtkWidget* pCanvas, void* /* cairo_t* cr */, gpointer pData)
{
    fprintf(stderr, "attempting to draw tile");

    Document* pDocument = static_cast< Document* >( pData );

    // This is UNX only for now, we need to get the appropriate equivalents
    // for windows/mac (see SystemGraphicsData for what we need)
    SystemGraphicsData aSystemGraphicsData;
    aSystemGraphicsData.pDisplay = GDK_WINDOW_XDISPLAY( pCanvas->window );
    aSystemGraphicsData.hDrawable = GDK_WINDOW_XWINDOW( pCanvas->window );
    aSystemGraphicsData.pVisual = GDK_VISUAL_XVISUAL( gtk_widget_get_visual( pCanvas ) );
    aSystemGraphicsData.nScreen = GDK_SCREEN_XNUMBER( gtk_widget_get_screen( pCanvas ) );
    aSystemGraphicsData.nDepth = gdk_visual_get_depth( gtk_widget_get_visual( pCanvas ) );
    aSystemGraphicsData.aColormap = GDK_COLORMAP_XCOLORMAP(
                                        gdk_screen_get_default_colormap(
                                            gtk_widget_get_screen( pCanvas ) ) );
    aSystemGraphicsData.pXRenderFormat = XRenderFindVisualFormat(
                                        GDK_WINDOW_XDISPLAY( pCanvas->window ),
                                        GDK_VISUAL_XVISUAL( gtk_widget_get_visual( pCanvas ) ) );

    // Hardcoded tile just to see whether or not we get any sort of output.
    pDocument->paintTile( &aSystemGraphicsData, 256, 256, 0, 0, 5000, 5000 );
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

    GtkWidget* pCanvas = gtk_drawing_area_new();
    gtk_widget_set_size_request( pCanvas, 1000, 1000 );
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pScroller), pCanvas );

    g_signal_connect( G_OBJECT(pCanvas), "expose_event", G_CALLBACK(drawCallback), pDocument);

    gtk_widget_show( pCanvas );
    gtk_widget_show( pScroller );
    gtk_widget_show( pWindow );


    gtk_main();

    return 0;
}