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
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <com/sun/star/awt/Key.hpp>
#include <rsc/rsc-vcl-shared-types.hxx>

#ifndef g_info
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

static int help()
{
    fprintf( stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install> <path-to-document>\n" );
    return 1;
}

static GtkWidget* pDocView;
static GtkToolItem* pEnableEditing;
static GtkToolItem* pBold;
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
    }
}

/// User clicked on the button -> inform LOKDocView.
void toggleEditing(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    LOKDocView* pLOKDocView = LOK_DOCVIEW(pDocView);
    bool bActive = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pEnableEditing));
    if (lok_docview_get_edit(pLOKDocView) != bActive)
        lok_docview_set_edit(pLOKDocView, bActive);
}

/// LOKDocView changed edit state -> inform the tool button.
static void signalEdit(LOKDocView* pLOKDocView, gboolean bWasEdit, gpointer /*pData*/)
{
    gboolean bEdit = lok_docview_get_edit(pLOKDocView);
    g_info("signalEdit: %d -> %d", bWasEdit, lok_docview_get_edit(pLOKDocView));
    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pEnableEditing)) != bEdit)
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pEnableEditing), bEdit);
}

/// User clicked on the 'Bold' button -> inform LOKDocView.
void toggleBold(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    LOKDocView* pLOKDocView = LOK_DOCVIEW(pDocView);

    lok_docview_post_command(pLOKDocView, ".uno:Bold");
}

/// Receives a key press or release event.
static void signalKey(GtkWidget* /*pWidget*/, GdkEventKey* pEvent, gpointer /*pData*/)
{
    LOKDocView* pLOKDocView = LOK_DOCVIEW(pDocView);
    int nCharCode = 0;
    int nKeyCode = 0;

    if (!pLOKDocView->m_bEdit)
    {
        g_info("signalKey: not in edit mode, ignore");
        return;
    }

    switch (pEvent->keyval)
    {
    case GDK_BackSpace:
        nKeyCode = com::sun::star::awt::Key::BACKSPACE;
        break;
    case GDK_Return:
        nKeyCode = com::sun::star::awt::Key::RETURN;
        break;
    case GDK_Escape:
        nKeyCode = com::sun::star::awt::Key::ESCAPE;
        break;
    case GDK_Tab:
        nKeyCode = com::sun::star::awt::Key::TAB;
        break;
    case GDK_Down:
        nKeyCode = com::sun::star::awt::Key::DOWN;
        break;
    case GDK_Up:
        nKeyCode = com::sun::star::awt::Key::UP;
        break;
    case GDK_Left:
        nKeyCode = com::sun::star::awt::Key::LEFT;
        break;
    case GDK_Right:
        nKeyCode = com::sun::star::awt::Key::RIGHT;
        break;
    default:
        if (pEvent->keyval >= GDK_F1 && pEvent->keyval <= GDK_F26)
            nKeyCode = com::sun::star::awt::Key::F1 + (pEvent->keyval - GDK_F1);
        else
            nCharCode = gdk_keyval_to_unicode(pEvent->keyval);
    }

    // rsc is not public API, but should be good enough for debugging purposes.
    // If this is needed for real, then probably a new param of type
    // css::awt::KeyModifier is needed in postKeyEvent().
    if (pEvent->state & GDK_SHIFT_MASK)
        nKeyCode |= KEY_SHIFT;

    if (pEvent->type == GDK_KEY_RELEASE)
        pLOKDocView->pDocument->pClass->postKeyEvent(pLOKDocView->pDocument, LOK_KEYEVENT_KEYUP, nCharCode, nKeyCode);
    else
        pLOKDocView->pDocument->pClass->postKeyEvent(pLOKDocView->pDocument, LOK_KEYEVENT_KEYINPUT, nCharCode, nKeyCode);
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
    gtk_window_set_title( GTK_WINDOW(pWindow), "LibreOfficeKit GTK Tiled Viewer" );
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

    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);
    pEnableEditing = gtk_toggle_tool_button_new();
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(pEnableEditing), "Editing");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pEnableEditing, -1);
    g_signal_connect(G_OBJECT(pEnableEditing), "toggled", G_CALLBACK(toggleEditing), NULL);

    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);
    pBold = gtk_toggle_tool_button_new();
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(pBold), "Bold");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pBold, -1);
    g_signal_connect(G_OBJECT(pBold), "toggled", G_CALLBACK(toggleBold), NULL);

    gtk_box_pack_start( GTK_BOX(pVBox), pToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Docview
    pDocView = lok_docview_new( pOffice );
    g_signal_connect(pDocView, "edit-changed", G_CALLBACK(signalEdit), NULL);

    // Input handling.
    g_signal_connect(pWindow, "key-press-event", G_CALLBACK(signalKey), NULL);
    g_signal_connect(pWindow, "key-release-event", G_CALLBACK(signalKey), NULL);

    gtk_container_add( GTK_CONTAINER(pVBox), pDocView );

    gtk_widget_show_all( pWindow );

    pFileName = argv[2];
    int bOpened = lok_docview_open_document( LOK_DOCVIEW(pDocView), argv[2] );
    if (!bOpened)
        g_error("main: lok_docview_open_document() failed with '%s'", pOffice->pClass->getError(pOffice));
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
