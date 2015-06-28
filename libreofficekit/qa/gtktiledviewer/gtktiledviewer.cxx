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
#include <string>
#include <map>

#include <boost/property_tree/json_parser.hpp>
#include <gdk/gdkkeysyms.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#ifndef g_info
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

static int help()
{
    fprintf( stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install's-program-directory> <path-to-document>\n" );
    return 1;
}

static GtkWidget* pDocView;
static GtkToolItem* pEnableEditing;
static GtkToolItem* pBold;
static GtkToolItem* pItalic;
static GtkToolItem* pUnderline;
static GtkToolItem* pStrikethrough;
static GtkWidget* pScrolledWindow;
std::map<GtkToolItem*, std::string> g_aToolItemCommandNames;
std::map<std::string, GtkToolItem*> g_aCommandNameToolItems;
bool g_bToolItemBroadcast = true;
static GtkWidget* pVBox;
static GtkComboBoxText* pPartSelector;
/// Should the part selector avoid calling lok::Document::setPart()?
static bool g_bPartSelectorBroadcast = true;
GtkWidget* pFindbar;
GtkWidget* pFindbarEntry;
GtkWidget* pFindbarLabel;

static void lcl_registerToolItem(GtkToolItem* pItem, const std::string& rName)
{
    g_aToolItemCommandNames[pItem] = rName;
    g_aCommandNameToolItems[rName] = pItem;
}

const float fZooms[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 5.0 };

static void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    const char *sName = gtk_tool_button_get_icon_name( GTK_TOOL_BUTTON(pButton) );

    float fZoom = 0;
    float fCurrentZoom = 0;

    if ( pDocView )
    {
        fCurrentZoom = lok_doc_view_get_zoom( LOK_DOC_VIEW(pDocView) );
    }

    if ( strcmp(sName, "zoom-in-symbolic") == 0)
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
    else if ( strcmp(sName, "zoom-original-symbolic") == 0)
    {
        fZoom = 1;
    }
    else if ( strcmp(sName, "zoom-out-symbolic") == 0)
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
            lok_doc_view_set_zoom( LOK_DOC_VIEW(pDocView), fZoom );
        }
    }
}

/// User clicked on the button -> inform LOKDocView.
static void toggleEditing(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    LOKDocView* pLOKDocView = LOK_DOC_VIEW(pDocView);
    bool bActive = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pEnableEditing));
    if (bool(lok_doc_view_get_edit(pLOKDocView)) != bActive)
        lok_doc_view_set_edit(pLOKDocView, bActive);
}

/// Toggle the visibility of the findbar.
static void toggleFindbar(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    if (gtk_widget_get_visible(pFindbar))
    {
        gtk_widget_hide(pFindbar);
    }
    else
    {
        gtk_widget_show_all(pFindbar);
        gtk_widget_grab_focus(pFindbarEntry);
    }
}

/// Our GtkClipboardGetFunc implementation for HTML.
static void htmlGetFunc(GtkClipboard* /*pClipboard*/, GtkSelectionData* pSelectionData, guint /*info*/, gpointer pUserData)
{
    GdkAtom aAtom(gdk_atom_intern("text/html", false));
    const gchar* pSelection = static_cast<const gchar*>(pUserData);
    gtk_selection_data_set(pSelectionData, aAtom, 8, reinterpret_cast<const guchar *>(pSelection), strlen(pSelection));
}

/// Our GtkClipboardClearFunc implementation for HTML.
static void htmlClearFunc(GtkClipboard* /*pClipboard*/, gpointer pData)
{
    g_free(pData);
}

/// Same as gtk_clipboard_set_text(), but sets HTML.
static void clipboardSetHtml(GtkClipboard* pClipboard, const char* pSelection)
{
    GtkTargetList* pList = gtk_target_list_new(0, 0);
    GdkAtom aAtom(gdk_atom_intern("text/html", false));
    gtk_target_list_add(pList, aAtom, 0, 0);
    gint nTargets = 0;
    GtkTargetEntry* pTargets = gtk_target_table_new_from_list(pList, &nTargets);

    gtk_clipboard_set_with_data(pClipboard, pTargets, nTargets, htmlGetFunc, htmlClearFunc, g_strdup(pSelection));

    gtk_target_table_free(pTargets, nTargets);
    gtk_target_list_unref(pList);
}

/// Handler for the copy button: write clipboard.
static void doCopy(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    LOKDocView* pLOKDocView = LOK_DOC_VIEW(pDocView);
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(pLOKDocView);
    char* pUsedFormat = 0;
    char* pSelection = pDocument->pClass->getTextSelection(pDocument, "text/html", &pUsedFormat);

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(pDocView), GDK_SELECTION_CLIPBOARD);
    std::string aUsedFormat(pUsedFormat);
    if (aUsedFormat == "text/plain;charset=utf-8")
        gtk_clipboard_set_text(pClipboard, pSelection, -1);
    else
        clipboardSetHtml(pClipboard, pSelection);

    free(pSelection);
    free(pUsedFormat);
}

/// Get the visible area of the scrolled window
static void getVisibleAreaTwips(GdkRectangle* pArea)
{
#if GTK_CHECK_VERSION(2,14,0) // we need gtk_adjustment_get_page_size()
    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(pScrolledWindow));
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(pScrolledWindow));

    pArea->x      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_value(pHAdjustment));
    pArea->y      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_value(pVAdjustment));
    pArea->width  = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_page_size(pHAdjustment));
    pArea->height = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_page_size(pVAdjustment));
#endif
}


/// Handles the key-press-event of the window.
static gboolean signalKey(GtkWidget* /*pWidget*/, GdkEvent* pEvent, gpointer/* pData*/)
{
    LOKDocView* pLOKDocView = LOK_DOC_VIEW(pDocView);
    if (!gtk_widget_get_visible(pFindbar) && bool(lok_doc_view_get_edit(pLOKDocView)))
        {
            lok_doc_view_post_key(pLOKDocView, pEvent);
            return TRUE;
        }
    return FALSE;
}

/// Searches for the next or previous text of pFindbarEntry.
static void doSearch(bool bBackwards)
{
    GtkEntry* pEntry = GTK_ENTRY(pFindbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);
    boost::property_tree::ptree aTree;
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchString/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchString/value", '/'), pText);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.Backward/type", '/'), "boolean");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.Backward/value", '/'), bBackwards);

    LOKDocView* pLOKDocView = LOK_DOC_VIEW(pDocView);
    GdkRectangle aArea;
    getVisibleAreaTwips(&aArea);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointX/type", '/'), "long");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointX/value", '/'), aArea.x);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointY/type", '/'), "long");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointY/value", '/'), aArea.y);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    lok_doc_view_post_command(pLOKDocView, ".uno:ExecuteSearch", aStream.str().c_str());
}

/// Click handler for the search next button.
static void signalSearchNext(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    doSearch(/*bBackwards=*/false);
}

/// Click handler for the search previous button.
static void signalSearchPrev(GtkWidget* /*pButton*/, gpointer /*pItem*/)
{
    doSearch(/*bBackwards=*/true);
}

/// Handles the key-press-event of the search entry widget.
static gboolean signalFindbar(GtkWidget* /*pWidget*/, GdkEventKey* pEvent, gpointer /*pData*/)
{
    gtk_label_set_text(GTK_LABEL(pFindbarLabel), "");
    switch(pEvent->keyval)
    {
        case GDK_KEY_Return:
        {
            // Search forward.
            doSearch(/*bBackwards=*/false);
            return TRUE;
        }
        case GDK_KEY_Escape:
        {
            // Hide the findbar.
            gtk_widget_hide(pFindbar);
            return TRUE;
        }
    }
    return FALSE;
}

/// LOKDocView changed edit state -> inform the tool button.
static void signalEdit(LOKDocView* pLOKDocView, gboolean bWasEdit, gpointer /*pData*/)
{
    gboolean bEdit = lok_doc_view_get_edit(pLOKDocView);
    g_info("signalEdit: %d -> %d", bWasEdit, lok_doc_view_get_edit(pLOKDocView));
    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pEnableEditing)) != bEdit)
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pEnableEditing), bEdit);
}

/// LOKDocView changed command state -> inform the tool button.
static void signalCommand(LOKDocView* /*pLOKDocView*/, char* pPayload, gpointer /*pData*/)
{
    std::string aPayload(pPayload);
    size_t nPosition = aPayload.find("=");
    if (nPosition != std::string::npos)
    {
        std::string aKey = aPayload.substr(0, nPosition);
        std::string aValue = aPayload.substr(nPosition + 1);
        g_info("signalCommand: '%s' is '%s'", aKey.c_str(), aValue.c_str());

        if (g_aCommandNameToolItems.find(aKey) != g_aCommandNameToolItems.end())
        {
            GtkToolItem* pItem = g_aCommandNameToolItems[aKey];
            gboolean bEdit = aValue == "true";
            if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pItem)) != bEdit)
            {
                // Avoid invoking lok_doc_view_post_command().
                g_bToolItemBroadcast = false;
                gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pItem), bEdit);
                g_bToolItemBroadcast = true;
            }
        }
    }
}

/// LOKDocView found no search matches -> set the search label accordingly.
static void signalSearch(LOKDocView* /*pLOKDocView*/, char* /*pPayload*/, gpointer /*pData*/)
{
    gtk_label_set_text(GTK_LABEL(pFindbarLabel), "Search key not found");
}

static void signalPart(LOKDocView* /*pLOKDocView*/, int nPart, gpointer /*pData*/)
{
    g_bPartSelectorBroadcast = false;
    gtk_combo_box_set_active(GTK_COMBO_BOX(pPartSelector), nPart);
    g_bPartSelectorBroadcast = true;
}

/// User clicked on a command button -> inform LOKDocView.
static void toggleToolItem(GtkWidget* pWidget, gpointer /*pData*/)
{
    if (g_bToolItemBroadcast)
    {
        LOKDocView* pLOKDocView = LOK_DOC_VIEW(pDocView);
        GtkToolItem* pItem = GTK_TOOL_ITEM(pWidget);
        const std::string& rString = g_aToolItemCommandNames[pItem];
        g_info("toggleToolItem: lok_doc_view_post_command('%s')", rString.c_str());
        lok_doc_view_post_command(pLOKDocView, rString.c_str(), 0);
    }
}

static void populatePartSelector()
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

    int nParts = lok_doc_view_get_parts( LOK_DOC_VIEW(pDocView) );
    for ( int i = 0; i < nParts; i++ )
    {
        char* pName = lok_doc_view_get_part_name( LOK_DOC_VIEW(pDocView), i );
        assert( pName );
        snprintf( sText, nMaxLength, "%i (%s)", i+1, pName );
        free( pName );

        gtk_combo_box_text_append_text( pPartSelector, sText );
    }
    gtk_combo_box_set_active( GTK_COMBO_BOX(pPartSelector),
                              lok_doc_view_get_part( LOK_DOC_VIEW(pDocView) ) );
}

static void changePart( GtkWidget* pSelector, gpointer /* pItem */ )
{
    int nPart = gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) );

    if (g_bPartSelectorBroadcast && pDocView)
    {
        lok_doc_view_set_part( LOK_DOC_VIEW(pDocView), nPart );
    }
}

static void populatePartModeSelector( GtkComboBoxText* pSelector )
{
    gtk_combo_box_text_append_text( pSelector, "Standard" );
    gtk_combo_box_text_append_text( pSelector, "Notes" );
    gtk_combo_box_set_active( GTK_COMBO_BOX(pSelector), 0 );
}

static void changePartMode( GtkWidget* pSelector, gpointer /* pItem */ )
{
    // Just convert directly back to the LibreOfficeKitPartMode enum.
    // I.e. the ordering above should match the enum member ordering.
    LibreOfficeKitPartMode ePartMode =
        LibreOfficeKitPartMode( gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) ) );

    if ( pDocView )
    {
        lok_doc_view_set_partmode( LOK_DOC_VIEW(pDocView), ePartMode );
    }
}

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

    gtk_init( &argc, &argv );

    GtkWidget *pWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(pWindow), "LibreOfficeKit GTK Tiled Viewer" );
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 1024, 768);
    g_signal_connect( pWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL );

    pVBox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_add( GTK_CONTAINER(pWindow), pVBox );

    // Toolbar
    GtkWidget* pToolbar = gtk_toolbar_new();
    gtk_toolbar_set_style( GTK_TOOLBAR(pToolbar), GTK_TOOLBAR_ICONS );

    GtkToolItem* pZoomIn = gtk_tool_button_new( NULL, NULL );
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoomIn), "zoom-in-symbolic");
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoomIn, 0);
    g_signal_connect( G_OBJECT(pZoomIn), "clicked", G_CALLBACK(changeZoom), NULL );

    GtkToolItem* pZoom1 = gtk_tool_button_new( NULL, NULL );
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoom1), "zoom-original-symbolic");
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoom1, -1);
    g_signal_connect( G_OBJECT(pZoom1), "clicked", G_CALLBACK(changeZoom), NULL );

    GtkToolItem* pZoomOut = gtk_tool_button_new( NULL, NULL );
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoomOut), "zoom-out-symbolic");
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pZoomOut, -1);
    g_signal_connect( G_OBJECT(pZoomOut), "clicked", G_CALLBACK(changeZoom), NULL );

    GtkToolItem* pSeparator1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pSeparator1, -1);

    GtkToolItem* pPartSelectorToolItem = gtk_tool_item_new();
    GtkWidget* pComboBox = gtk_combo_box_text_new();
    gtk_container_add( GTK_CONTAINER(pPartSelectorToolItem), pComboBox );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pPartSelectorToolItem, -1 );

    pPartSelector = GTK_COMBO_BOX_TEXT(pComboBox);

    GtkToolItem* pSeparator2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pSeparator2, -1);

    GtkToolItem* pPartModeSelectorToolItem = gtk_tool_item_new();
    GtkWidget* pPartModeComboBox = gtk_combo_box_text_new();
    gtk_container_add( GTK_CONTAINER(pPartModeSelectorToolItem), pPartModeComboBox );
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), pPartModeSelectorToolItem, -1 );

    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);

    // Cut, copy & paste.
    GtkToolItem* pCopyButton = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(pCopyButton), "edit-copy-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pCopyButton, -1);
    g_signal_connect(G_OBJECT(pCopyButton), "clicked", G_CALLBACK(doCopy), NULL);
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);

    pEnableEditing = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pEnableEditing), "insert-text-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pEnableEditing, -1);
    g_signal_connect(G_OBJECT(pEnableEditing), "toggled", G_CALLBACK(toggleEditing), NULL);

    GtkToolItem* pFindButton = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindButton), "edit-find-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pFindButton, -1);
    g_signal_connect(G_OBJECT(pFindButton), "clicked", G_CALLBACK(toggleFindbar), NULL);

    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);

    pBold = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pBold), "format-text-bold-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pBold, -1);
    g_signal_connect(G_OBJECT(pBold), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(pBold, ".uno:Bold");

    pItalic = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pItalic), "format-text-italic-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pItalic, -1);
    g_signal_connect(G_OBJECT(pItalic), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(pItalic, ".uno:Italic");

    pUnderline = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pUnderline), "format-text-underline-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pUnderline, -1);
    g_signal_connect(G_OBJECT(pUnderline), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(pUnderline, ".uno:Underline");

    pStrikethrough = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pStrikethrough), "format-text-strikethrough-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pStrikethrough, -1);
    g_signal_connect(G_OBJECT(pStrikethrough), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(pStrikethrough, ".uno:Strikeout");

    gtk_box_pack_start( GTK_BOX(pVBox), pToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Findbar
    pFindbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(pFindbar), GTK_TOOLBAR_ICONS);

    GtkToolItem* pFindbarClose = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarClose), "window-close-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pFindbar), pFindbarClose, -1);
    g_signal_connect(G_OBJECT(pFindbarClose), "clicked", G_CALLBACK(toggleFindbar), NULL);

    GtkToolItem* pEntryContainer = gtk_tool_item_new();
    pFindbarEntry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(pEntryContainer), pFindbarEntry);
    g_signal_connect(pFindbarEntry, "key-press-event", G_CALLBACK(signalFindbar), 0);
    gtk_toolbar_insert(GTK_TOOLBAR(pFindbar), pEntryContainer, -1);

    GtkToolItem* pFindbarNext = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarNext), "go-down-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pFindbar), pFindbarNext, -1);
    g_signal_connect(G_OBJECT(pFindbarNext), "clicked", G_CALLBACK(signalSearchNext), NULL);

    GtkToolItem* pFindbarPrev = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarPrev), "go-up-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(pFindbar), pFindbarPrev, -1);
    g_signal_connect(G_OBJECT(pFindbarPrev), "clicked", G_CALLBACK(signalSearchPrev), NULL);

    GtkToolItem* pFindbarLabelContainer = gtk_tool_item_new();
    pFindbarLabel = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(pFindbarLabelContainer), pFindbarLabel);
    gtk_toolbar_insert(GTK_TOOLBAR(pFindbar), pFindbarLabelContainer, -1);

    gtk_box_pack_end(GTK_BOX(pVBox), pFindbar, FALSE, FALSE, 0);

    // Docview
    pDocView = lok_doc_view_new (argv[1], NULL, NULL);
    if (pDocView == NULL)
        g_error ("Error while creating LOKDocView widget");
    g_signal_connect(pDocView, "edit-changed", G_CALLBACK(signalEdit), NULL);
    g_signal_connect(pDocView, "command-changed", G_CALLBACK(signalCommand), NULL);
    g_signal_connect(pDocView, "search-not-found", G_CALLBACK(signalSearch), NULL);
    g_signal_connect(pDocView, "part-changed", G_CALLBACK(signalPart), NULL);

    // Input handling.
    g_signal_connect(pWindow, "key-press-event", G_CALLBACK(signalKey), pDocView);
    g_signal_connect(pWindow, "key-release-event", G_CALLBACK(signalKey), pDocView);

    // Scrolled window for DocView
    pScrolledWindow = gtk_scrolled_window_new(0, 0);
    gtk_widget_set_hexpand (pScrolledWindow, TRUE);
    gtk_widget_set_vexpand (pScrolledWindow, TRUE);
    gtk_container_add(GTK_CONTAINER(pVBox), pScrolledWindow);

    gtk_container_add(GTK_CONTAINER(pScrolledWindow), pDocView);

    gtk_widget_show_all( pWindow );
    // Hide the findbar by default.
    gtk_widget_hide(pFindbar);

    int bOpened = lok_doc_view_open_document( LOK_DOC_VIEW(pDocView), argv[2] );
    if (!bOpened)
        g_error("main: lok_doc_view_open_document() failed");
    assert(lok_doc_view_get_document(LOK_DOC_VIEW(pDocView)));

    populatePartSelector();
    populatePartModeSelector( GTK_COMBO_BOX_TEXT(pPartModeComboBox) );
    // Connect these signals after populating the selectors, to avoid re-rendering on setting the default part/partmode.
    g_signal_connect(G_OBJECT(pPartModeComboBox), "changed", G_CALLBACK(changePartMode), 0);
    g_signal_connect(G_OBJECT(pPartSelector), "changed", G_CALLBACK(changePart), 0);

    gtk_main();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
