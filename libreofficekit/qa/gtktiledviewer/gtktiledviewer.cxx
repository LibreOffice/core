/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include "../lokdocview_quad/lokdocview_quad.h"

#include <com/sun/star/awt/Key.hpp>

static int help()
{
    fprintf( stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install> <path-to-document>\n" );
    return 1;
}

static GtkWidget* pDocView;
static GtkWidget* pDocViewQuad;
static GtkWidget* pVBox;
// GtkComboBox requires gtk 2.24 or later
#if ( GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION >= 24 ) || GTK_MAJOR_VERSION > 2
static GtkComboBoxText* pPartSelector;
#endif

static LibreOfficeKit* pOffice;
static char* pFileName;

const float fZooms[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 5.0 };

void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    const char *sName = gtk_tool_button_get_stock_id( GTK_TOOL_BUTTON(pButton) );

    float fZoom = 0;
    float fCurrentZoom = 0;

    if ( pDocView )
    {
        fCurrentZoom = lok_docview_get_zoom( LOK_DOCVIEW(pDocView) );
    }
    else if ( pDocViewQuad )
    {
        fCurrentZoom = lok_docview_quad_get_zoom( LOK_DOCVIEW_QUAD(pDocViewQuad) );
    }

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
        if ( pDocView )
        {
            lok_docview_set_zoom( LOK_DOCVIEW(pDocView), fZoom );
        }
        else if ( pDocViewQuad )
        {
            lok_docview_quad_set_zoom( LOK_DOCVIEW_QUAD(pDocViewQuad), fZoom );
        }
    }
}

void changeQuadView( GtkWidget* /*pButton*/, gpointer /* pItem */ )
{
    if ( pDocView )
    {
        const float fCurrentZoom = lok_docview_get_zoom( LOK_DOCVIEW(pDocView) );
        gtk_widget_destroy( pDocView );
        pDocView = 0;
        pDocViewQuad = lok_docview_quad_new( pOffice );
        gtk_container_add( GTK_CONTAINER(pVBox), pDocViewQuad );
        gtk_widget_show( pDocViewQuad );

        lok_docview_quad_set_zoom( LOK_DOCVIEW_QUAD(pDocViewQuad), fCurrentZoom );
        lok_docview_quad_open_document( LOK_DOCVIEW_QUAD(pDocViewQuad), pFileName );
    }
    else if ( pDocViewQuad )
    {
        const float fCurrentZoom = lok_docview_quad_get_zoom( LOK_DOCVIEW_QUAD(pDocViewQuad) );
        gtk_widget_destroy( pDocViewQuad );
        pDocViewQuad = 0;
        pDocView = lok_docview_new( pOffice );
        gtk_container_add( GTK_CONTAINER(pVBox), pDocView );
        gtk_widget_show( pDocView );

        lok_docview_set_zoom( LOK_DOCVIEW(pDocView), fCurrentZoom );
        lok_docview_open_document( LOK_DOCVIEW(pDocView), pFileName );
    }
}

/// Receives a key press or release event.
static void signalKey(GtkWidget* /*pWidget*/, GdkEventKey* pEvent, gpointer /*pData*/)
{
    LOKDocView* pLOKDocView = LOK_DOCVIEW(pDocView);

    int nCode = 0;
    switch (pEvent->keyval)
    {
    case GDK_BackSpace:
        nCode = com::sun::star::awt::Key::BACKSPACE;
        break;
    case GDK_Return:
        nCode = com::sun::star::awt::Key::RETURN;
        break;
    default:
        if (pEvent->keyval >= GDK_F1 && pEvent->keyval <= GDK_F26)
            nCode = com::sun::star::awt::Key::F1 + (pEvent->keyval - GDK_F1);
        else
            nCode = gdk_keyval_to_unicode(pEvent->keyval);
    }

    if (pEvent->type == GDK_KEY_RELEASE)
        pLOKDocView->pOffice->pClass->postKeyEvent(pLOKDocView->pOffice, LOK_KEYEVENT_KEYUP, nCode);
    else
        pLOKDocView->pOffice->pClass->postKeyEvent(pLOKDocView->pOffice, LOK_KEYEVENT_KEYINPUT, nCode);
}

// GtkComboBox requires gtk 2.24 or later
#if ( GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION >= 24 ) || GTK_MAJOR_VERSION > 2
void populatePartSelector()
{
    gtk_list_store_clear( GTK_LIST_STORE(
                              gtk_combo_box_get_model(
                                  GTK_COMBO_BOX(pPartSelector) )) );

    if ( !pDocView )
    {
        return;
    }

    const int nMaxLength = 50;
    char sText[nMaxLength];

    int nParts = lok_docview_get_parts( LOK_DOCVIEW(pDocView) );
    for ( int i = 0; i < nParts; i++ )
    {
        char* pName = lok_docview_get_part_name( LOK_DOCVIEW(pDocView), i );
        assert( pName );
        snprintf( sText, nMaxLength, "%i (%s)", i+1, pName );
        free( pName );

        gtk_combo_box_text_append_text( pPartSelector, sText );
    }
    gtk_combo_box_set_active( GTK_COMBO_BOX(pPartSelector),
                              lok_docview_get_part( LOK_DOCVIEW(pDocView) ) );
}

void changePart( GtkWidget* pSelector, gpointer /* pItem */ )
{
    int nPart = gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) );

    // We don't really care about the quad view for now -- it's only purpose
    // is to check that the edges of tiles aren't messed up, and no real
    // reason to maintain it to be able to show other document parts etc.
    if ( pDocView )
    {
        lok_docview_set_part( LOK_DOCVIEW(pDocView), nPart );
    }
}

void populatePartModeSelector( GtkComboBoxText* pSelector )
{
    gtk_combo_box_text_append_text( pSelector, "Default" );
    gtk_combo_box_text_append_text( pSelector, "Slide" );
    gtk_combo_box_text_append_text( pSelector, "Notes" );
    gtk_combo_box_text_append_text( pSelector, "Combined (SlideNotes)" );
    gtk_combo_box_text_append_text( pSelector, "Embedded Objects" );
    gtk_combo_box_set_active( GTK_COMBO_BOX(pSelector), 0 );
}

void changePartMode( GtkWidget* pSelector, gpointer /* pItem */ )
{
    // Just convert directly back to the LibreOfficeKitPartMode enum.
    // I.e. the ordering above should match the enum member ordering.
    LibreOfficeKitPartMode ePartMode =
        LibreOfficeKitPartMode( gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) ) );

    if ( pDocView )
    {
        lok_docview_set_partmode( LOK_DOCVIEW(pDocView), ePartMode );
    }

    // The number of items could change e.g. if we change from slide
    // to embeddede obj mode -- hence we should update the part list.
    populatePartSelector();
}
#endif

int main( int argc, char* argv[] )
{
    if( argc < 3 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();

    if ( argv[1][0] != '/' )
    {
        fprintf(stderr, "Absolute path required to libreoffice install\n");
        return 1;
    }

    pOffice = lok_init( argv[1] );
    if ( pOffice == NULL )
        return 1;

    gtk_init( &argc, &argv );

    GtkWidget *pWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(pWindow), "LibreOffice GTK Tiled Viewer" );
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 800, 600);
    g_signal_connect( pWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL );

    pVBox = gtk_vbox_new( FALSE, 0 );
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

    GtkToolItem* pSeparator1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pSeparator1, -1);

// GtkComboBox requires gtk 2.24 or later
#if ( GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION >= 24 ) || GTK_MAJOR_VERSION > 2
    GtkToolItem* pPartSelectorToolItem = gtk_tool_item_new();
    GtkWidget* pComboBox = gtk_combo_box_text_new();
    gtk_container_add( GTK_CONTAINER(pPartSelectorToolItem), pComboBox );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pPartSelectorToolItem, -1 );
    g_signal_connect( G_OBJECT(pComboBox), "changed", G_CALLBACK(changePart), NULL );

    pPartSelector = GTK_COMBO_BOX_TEXT(pComboBox);

    GtkToolItem* pSeparator2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pSeparator2, -1);

    GtkToolItem* pPartModeSelectorToolItem = gtk_tool_item_new();
    GtkWidget* pPartModeComboBox = gtk_combo_box_text_new();
    gtk_container_add( GTK_CONTAINER(pPartModeSelectorToolItem), pPartModeComboBox );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pPartModeSelectorToolItem, -1 );
    g_signal_connect( G_OBJECT(pPartModeComboBox), "changed", G_CALLBACK(changePartMode), NULL );
#endif

    GtkToolItem* pSeparator3 = gtk_separator_tool_item_new();
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pSeparator3, -1);

    GtkToolItem* pEnableQuadView = gtk_toggle_tool_button_new();
    gtk_tool_button_set_label( GTK_TOOL_BUTTON(pEnableQuadView), "Use Quad View" );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pEnableQuadView, -1 );
    g_signal_connect( G_OBJECT(pEnableQuadView), "toggled", G_CALLBACK(changeQuadView), NULL );

    gtk_box_pack_start( GTK_BOX(pVBox), pToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Docview
    pDocView = lok_docview_new( pOffice );
    pDocViewQuad = 0;

    // Input handling.
    g_signal_connect(pWindow, "key-press-event", G_CALLBACK(signalKey), NULL);
    g_signal_connect(pWindow, "key-release-event", G_CALLBACK(signalKey), NULL);

    gtk_container_add( GTK_CONTAINER(pVBox), pDocView );

    gtk_widget_show_all( pWindow );

    pFileName = argv[2];
    int bOpened = lok_docview_open_document( LOK_DOCVIEW(pDocView), argv[2] );
    assert( bOpened ); (void)bOpened;
    assert( LOK_DOCVIEW(pDocView)->pDocument );

    // GtkComboBox requires gtk 2.24 or later
#if ( GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION >= 24 ) || GTK_MAJOR_VERSION > 2
    populatePartSelector();
    populatePartModeSelector( GTK_COMBO_BOX_TEXT(pPartModeComboBox) );
#endif

    gtk_main();

    pOffice->pClass->destroy( pOffice );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
