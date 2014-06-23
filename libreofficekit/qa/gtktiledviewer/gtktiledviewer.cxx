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
#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>

static int help()
{
    fprintf( stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install> [path to document]\n" );
    return 1;
}

static GtkWidget* pDocView;

const float fZooms[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 5.0 };

void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    const char *sName = gtk_tool_button_get_stock_id( GTK_TOOL_BUTTON(pButton) );

    float fZoom = 0;
    const float fCurrentZoom = lok_docview_get_zoom( LOK_DOCVIEW(pDocView) );
    if ( strcmp(sName, "gtk-zoom-in") == 0)
    {
        for ( unsigned int i = 0; i < sizeof( fZooms ) / sizeof( fZooms[0] ); i++ )
        {
            if ( fCurrentZoom < fZooms[i] )
            {
                fZoom = fZooms[i];
                break;
            }
        }
    }
    else if ( strcmp(sName, "gtk-zoom-100") == 0)
    {
        fZoom = 1;
    }
    else if ( strcmp(sName, "gtk-zoom-fit") == 0)
    {
        // TODO -- will need access to lokdocview internals?
    }
    else if ( strcmp(sName, "gtk-zoom-out") == 0)
    {
        for ( unsigned int i = 0; i < sizeof( fZooms ) / sizeof( fZooms[0] ); i++ )
        {
            if ( fCurrentZoom > fZooms[i] )
            {
                fZoom = fZooms[i];
            }
        }
    }

    if ( fZoom != 0 )
    {
        lok_docview_set_zoom( LOK_DOCVIEW(pDocView), fZoom );
    }
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

    LibreOfficeKit* pOffice = lok_init( argv[1] );

    gtk_init( &argc, &argv );

    GtkWidget *pWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(pWindow), "LibreOffice GTK Tiled Viewer" );
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 800, 600);
    g_signal_connect( pWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL );

    GtkWidget* pVBox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER(pWindow), pVBox );

    // Toolbar
    GtkWidget* pToolbar = gtk_toolbar_new();
    gtk_toolbar_set_style( GTK_TOOLBAR(pToolbar), GTK_TOOLBAR_ICONS );

    GtkToolItem* pZoomIn = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_IN );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoomIn, 0);
    g_signal_connect( G_OBJECT(pZoomIn), "clicked", G_CALLBACK(changeZoom), NULL );

    GtkToolItem* pZoom1 = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_100 );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoom1, -1);
    g_signal_connect( G_OBJECT(pZoom1), "clicked", G_CALLBACK(changeZoom), NULL );

    GtkToolItem* pZoomFit = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_FIT );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoomFit, -1);
    g_signal_connect( G_OBJECT(pZoomFit), "clicked", G_CALLBACK(changeZoom), NULL );

    GtkToolItem* pZoomOut = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_OUT );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoomOut, -1);
    g_signal_connect( G_OBJECT(pZoomOut), "clicked", G_CALLBACK(changeZoom), NULL );

    gtk_box_pack_start( GTK_BOX(pVBox), pToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Docview
    pDocView = lok_docview_new( pOffice );
    gtk_container_add( GTK_CONTAINER(pVBox), pDocView );

    gtk_widget_show_all( pWindow );

    lok_docview_open_document( LOK_DOCVIEW(pDocView), argv[2] );

    gtk_main();

    return 0;
}