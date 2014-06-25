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


    GtkWidget* pDocView = lok_docview_new( pOffice );
    gtk_container_add( GTK_CONTAINER(pWindow), pDocView );

    lok_docview_open_document( LOK_DOCVIEW(pDocView), argv[2] );

    gtk_widget_show( pDocView );
    gtk_widget_show( pWindow );

    gtk_main();

    return 0;
}