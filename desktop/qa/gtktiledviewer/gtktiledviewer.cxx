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

#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKit.hxx>

static int help()
{
    fprintf( stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install> [path to document]\n" );
    return 1;
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
    (void) pDocument;

    GtkWidget *pWindow;
    GtkWidget *pScroller;

    gtk_init( &argc, &argv );

    pWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(pWindow), "LibreOffice GTK Tiled Viewer" );
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 800, 600);
    g_signal_connect( pWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL );


    pScroller = gtk_scrolled_window_new( 0, 0 );
    gtk_container_add( GTK_CONTAINER(pWindow), pScroller );

    gtk_widget_show( pScroller );
    gtk_widget_show( pWindow );

    //GDK_WINDOW_XWINDOW

    gtk_main();

    return 0;
}