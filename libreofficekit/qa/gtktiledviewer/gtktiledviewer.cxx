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

#define LOK_USE_UNSTABLE_API
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

/// Represents all the state that is specific to one GtkWindow of this app.
class TiledWindow
{
public:
    GtkWidget* m_pDocView;
    GtkWidget* m_pStatusBar;
    GtkToolItem* m_pEnableEditing;
    GtkToolItem* m_pBold;
    GtkToolItem* m_pItalic;
    GtkToolItem* m_pUnderline;
    GtkToolItem* m_pStrikethrough;
    GtkWidget* m_pScrolledWindow;
    std::map<GtkToolItem*, std::string> m_aToolItemCommandNames;
    std::map<std::string, GtkToolItem*> m_aCommandNameToolItems;
    bool m_bToolItemBroadcast;
    GtkWidget* m_pVBox;
    GtkComboBoxText* m_pPartSelector;
    GtkWidget* m_pPartModeComboBox;
    /// Should the part selector avoid calling lok::Document::setPart()?
    bool m_bPartSelectorBroadcast;
    GtkWidget* m_pFindbar;
    GtkWidget* m_pFindbarEntry;
    GtkWidget* m_pFindbarLabel;
    bool m_bFindAll;

    TiledWindow()
        : m_pDocView(0),
        m_pStatusBar(0),
        m_pEnableEditing(0),
        m_pBold(0),
        m_pItalic(0),
        m_pUnderline(0),
        m_pStrikethrough(0),
        m_pScrolledWindow(0),
        m_bToolItemBroadcast(true),
        m_pVBox(0),
        m_pPartSelector(0),
        m_pPartModeComboBox(0),
        m_bPartSelectorBroadcast(true),
        m_pFindbar(0),
        m_pFindbarEntry(0),
        m_pFindbarLabel(0),
        m_bFindAll(false)
    {
    }
};

static std::map<GtkWidget*, TiledWindow> g_aWindows;

static void setupDocView(GtkWidget* pDocView);
static GtkWidget* createWindow(TiledWindow& rWindow);
static void openDocumentCallback (GObject* source_object, GAsyncResult* res, gpointer userdata);

static TiledWindow& lcl_getTiledWindow(GtkWidget* pWidget)
{
    GtkWidget* pToplevel = gtk_widget_get_toplevel(pWidget);
    assert(g_aWindows.find(pToplevel) != g_aWindows.end());
    return g_aWindows[pToplevel];
}

static void lcl_registerToolItem(TiledWindow& rWindow, GtkToolItem* pItem, const std::string& rName)
{
    rWindow.m_aToolItemCommandNames[pItem] = rName;
    rWindow.m_aCommandNameToolItems[rName] = pItem;
}

const float fZooms[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 5.0 };


/// Get the visible area of the scrolled window
static void getVisibleAreaTwips(GtkWidget* pDocView, GdkRectangle* pArea)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pDocView);

    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));

    pArea->x      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_value(pHAdjustment));
    pArea->y      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_value(pVAdjustment));
    pArea->width  = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_page_size(pHAdjustment));
    pArea->height = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_page_size(pVAdjustment));
}

static void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkWidget* pDocView = rWindow.m_pDocView;

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
static void toggleEditing(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);

    LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
    bool bActive = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rWindow.m_pEnableEditing));
    if (bool(lok_doc_view_get_edit(pLOKDocView)) != bActive)
        lok_doc_view_set_edit(pLOKDocView, bActive);
}

/// Toggles if search should find all results or only the first one.
static void toggleFindAll(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    rWindow.m_bFindAll = !rWindow.m_bFindAll;
}

/// Toggle the visibility of the findbar.
static void toggleFindbar(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    if (gtk_widget_get_visible(rWindow.m_pFindbar))
    {
        gtk_widget_hide(rWindow.m_pFindbar);
    }
    else
    {
        gtk_widget_show_all(rWindow.m_pFindbar);
        gtk_widget_grab_focus(rWindow.m_pFindbarEntry);
    }
}

/// Common initialization, regardless if it's just a new view or a full init.
static TiledWindow& setupWidgetAndCreateWindow(GtkWidget* pDocView)
{
    setupDocView(pDocView);
    TiledWindow aWindow;
    aWindow.m_pDocView = pDocView;
    GtkWidget* pWindow = createWindow(aWindow);
    return lcl_getTiledWindow(pWindow);
}

/// Creates a new view, i.e. no LOK init or document load.
static void createView(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkWidget* pDocView = lok_doc_view_new_from_widget(LOK_DOC_VIEW(rWindow.m_pDocView));

    TiledWindow& rNewWindow = setupWidgetAndCreateWindow(pDocView);
    // Hide status bar that contains the unused progress bar.
    gtk_widget_hide(rNewWindow.m_pStatusBar);
}

/// Creates a new model, i.e. LOK init and document load, one view implicitly.
static void createModelAndView(const char* pLOPath, const char* pDocPath)
{
    GtkWidget* pDocView = lok_doc_view_new(pLOPath, 0, 0);

    setupWidgetAndCreateWindow(pDocView);

    lok_doc_view_open_document(LOK_DOC_VIEW(pDocView), pDocPath, 0, openDocumentCallback, pDocView);
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
static void doCopy(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(pLOKDocView);
    char* pUsedFormat = 0;
    char* pSelection = pDocument->pClass->getTextSelection(pDocument, "text/html", &pUsedFormat);

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(rWindow.m_pDocView), GDK_SELECTION_CLIPBOARD);
    std::string aUsedFormat(pUsedFormat);
    if (aUsedFormat == "text/plain;charset=utf-8")
        gtk_clipboard_set_text(pClipboard, pSelection, -1);
    else
        clipboardSetHtml(pClipboard, pSelection);

    free(pSelection);
    free(pUsedFormat);
}


/// Searches for the next or previous text of TiledWindow::m_pFindbarEntry.
static void doSearch(GtkWidget* pButton, bool bBackwards)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkEntry* pEntry = GTK_ENTRY(rWindow.m_pFindbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);
    boost::property_tree::ptree aTree;
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchString/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchString/value", '/'), pText);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.Backward/type", '/'), "boolean");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.Backward/value", '/'), bBackwards);
    if (rWindow.m_bFindAll)
    {
        aTree.put(boost::property_tree::ptree::path_type("SearchItem.Command/type", '/'), "unsigned short");
        // SvxSearchCmd::FIND_ALL
        aTree.put(boost::property_tree::ptree::path_type("SearchItem.Command/value", '/'), "1");
    }

    LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
    GdkRectangle aArea;
    getVisibleAreaTwips(rWindow.m_pDocView, &aArea);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointX/type", '/'), "long");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointX/value", '/'), aArea.x);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointY/type", '/'), "long");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointY/value", '/'), aArea.y);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    lok_doc_view_post_command(pLOKDocView, ".uno:ExecuteSearch", aStream.str().c_str());
}

/// Click handler for the search next button.
static void signalSearchNext(GtkWidget* pButton, gpointer /*pItem*/)
{
    doSearch(pButton, /*bBackwards=*/false);
}

/// Click handler for the search previous button.
static void signalSearchPrev(GtkWidget* pButton, gpointer /*pItem*/)
{
    doSearch(pButton, /*bBackwards=*/true);
}

/// Handles the key-press-event of the search entry widget.
static gboolean signalFindbar(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pWidget);
    gtk_label_set_text(GTK_LABEL(rWindow.m_pFindbarLabel), "");
    switch(pEvent->keyval)
    {
        case GDK_KEY_Return:
        {
            // Search forward.
            doSearch(pWidget, /*bBackwards=*/false);
            return TRUE;
        }
        case GDK_KEY_Escape:
        {
            // Hide the findbar.
            gtk_widget_hide(rWindow.m_pFindbar);
            return TRUE;
        }
    }
    return FALSE;
}

/// LOKDocView changed edit state -> inform the tool button.
static void signalEdit(LOKDocView* pLOKDocView, gboolean bWasEdit, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));

    gboolean bEdit = lok_doc_view_get_edit(pLOKDocView);
    g_info("signalEdit: %d -> %d", bWasEdit, lok_doc_view_get_edit(pLOKDocView));
    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rWindow.m_pEnableEditing)) != bEdit)
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rWindow.m_pEnableEditing), bEdit);
}

/// LOKDocView changed command state -> inform the tool button.
static void signalCommand(LOKDocView* pLOKDocView, char* pPayload, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));

    std::string aPayload(pPayload);
    size_t nPosition = aPayload.find("=");
    if (nPosition != std::string::npos)
    {
        std::string aKey = aPayload.substr(0, nPosition);
        std::string aValue = aPayload.substr(nPosition + 1);

        if (rWindow.m_aCommandNameToolItems.find(aKey) != rWindow.m_aCommandNameToolItems.end())
        {
            GtkToolItem* pItem = rWindow.m_aCommandNameToolItems[aKey];
            gboolean bEdit = aValue == "true";
            if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pItem)) != bEdit)
            {
                // Avoid invoking lok_doc_view_post_command().
                rWindow.m_bToolItemBroadcast = false;
                gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pItem), bEdit);
                rWindow.m_bToolItemBroadcast = true;
            }
        }
    }
}

static void loadChanged(LOKDocView* /*pLOKDocView*/, gdouble fValue, gpointer pData)
{
    GtkWidget* pProgressBar = GTK_WIDGET (pData);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pProgressBar), fValue);
}

/// LOKDocView found no search matches -> set the search label accordingly.
static void signalSearch(LOKDocView* pLOKDocView, char* /*pPayload*/, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    gtk_label_set_text(GTK_LABEL(rWindow.m_pFindbarLabel), "Search key not found");
}

/// LOKDocView found some search matches -> set the search label accordingly.
static void signalSearchResultCount(LOKDocView* pLOKDocView, char* pPayload, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    std::stringstream ss;
    ss << pPayload << " match(es)";
    gtk_label_set_text(GTK_LABEL(rWindow.m_pFindbarLabel), ss.str().c_str());
}

static void signalPart(LOKDocView* pLOKDocView, int nPart, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    rWindow.m_bPartSelectorBroadcast = false;
    gtk_combo_box_set_active(GTK_COMBO_BOX(rWindow.m_pPartSelector), nPart);
    rWindow.m_bPartSelectorBroadcast = true;
}

/// User clicked on a command button -> inform LOKDocView.
static void signalHyperlink(LOKDocView* /*pLOKDocView*/, char* pPayload, gpointer /*pData*/)
{
    GError* pError = NULL;
    gtk_show_uri(NULL, pPayload, GDK_CURRENT_TIME, &pError);
    if (pError != NULL)
    {
        g_warning("Unable to show URI %s : %s", pPayload, pError->message);
        g_error_free(pError);
    }
}

/// Cursor position changed
static void cursorChanged(LOKDocView* pDocView, gint nX, gint nY,
                          gint /*nWidth*/, gint /*nHeight*/, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pDocView));

    GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    GdkRectangle visArea;
    gdouble upper;
    gint x = -1, y = -1;

    getVisibleAreaTwips(GTK_WIDGET(pDocView), &visArea);

    // check vertically
    if (nY < visArea.y)
    {
        y = nY - visArea.height/2;
        if (y < 0)
            y = gtk_adjustment_get_lower(vadj);
    }
    else if (nY > visArea.y + visArea.height)
    {
        y = nY - visArea.height/2;
        upper = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), gtk_adjustment_get_upper(vadj));
        if (y > upper)
            y = upper;

    }

    if (nX < visArea.x)
    {
        x = nX - visArea.width/2;
        if (x < 0)
            x = gtk_adjustment_get_lower(hadj);
    }
    else if (nX > visArea.x + visArea.width)
    {
        x = nX - visArea.width/2;
        upper = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), gtk_adjustment_get_upper(hadj));
        if (x > upper)
            x = upper;
    }

    if (y!=-1)
        gtk_adjustment_set_value(vadj, lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(pDocView), y));
    if (x!=-1)
        gtk_adjustment_set_value(hadj, lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(pDocView), x));
}

static void toggleToolItem(GtkWidget* pWidget, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pWidget);

    if (rWindow.m_bToolItemBroadcast)
    {
        LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
        GtkToolItem* pItem = GTK_TOOL_ITEM(pWidget);
        const std::string& rString = rWindow.m_aToolItemCommandNames[pItem];
        g_info("toggleToolItem: lok_doc_view_post_command('%s')", rString.c_str());
        lok_doc_view_post_command(pLOKDocView, rString.c_str(), 0);
    }
}

static void populatePartSelector(LOKDocView* pLOKDocView)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    gtk_list_store_clear( GTK_LIST_STORE(
                              gtk_combo_box_get_model(
                                  GTK_COMBO_BOX(rWindow.m_pPartSelector) )) );

    if (!pLOKDocView)
    {
        return;
    }

    const int nMaxLength = 50;
    char sText[nMaxLength];

    int nParts = lok_doc_view_get_parts(pLOKDocView);
    for ( int i = 0; i < nParts; i++ )
    {
        char* pName = lok_doc_view_get_part_name(pLOKDocView, i);
        assert( pName );
        snprintf( sText, nMaxLength, "%i (%s)", i+1, pName );
        free( pName );

        gtk_combo_box_text_append_text( rWindow.m_pPartSelector, sText );
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(rWindow.m_pPartSelector), lok_doc_view_get_part(pLOKDocView));
}

static void signalSize(LOKDocView* pLOKDocView, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    rWindow.m_bPartSelectorBroadcast = false;
    populatePartSelector(pLOKDocView);
    rWindow.m_bPartSelectorBroadcast = true;
}

static void changePart( GtkWidget* pSelector, gpointer /* pItem */ )
{
    int nPart = gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) );
    TiledWindow& rWindow = lcl_getTiledWindow(pSelector);

    if (rWindow.m_bPartSelectorBroadcast && rWindow.m_pDocView)
        lok_doc_view_set_part( LOK_DOC_VIEW(rWindow.m_pDocView), nPart );
    lok_doc_view_reset_view(LOK_DOC_VIEW(rWindow.m_pDocView));
}

static void removeChildrenFromStatusbar(GtkWidget* children, gpointer pData)
{
    GtkWidget* pStatusBar = static_cast<GtkWidget*>(pData);

    gtk_container_remove(GTK_CONTAINER(pStatusBar), children);
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
    TiledWindow& rWindow = lcl_getTiledWindow(pSelector);

    if ( rWindow.m_pDocView )
    {
        lok_doc_view_set_partmode( LOK_DOC_VIEW(rWindow.m_pDocView), ePartMode );
    }
}

static void openDocumentCallback (GObject* source_object, GAsyncResult* res, gpointer /*userdata*/)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (source_object);
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pDocView));
    GError* error = NULL;
    GList *focusChain = NULL;

    if (!lok_doc_view_open_document_finish(pDocView, res, &error))
    {
        GtkDialogFlags eFlags = GTK_DIALOG_DESTROY_WITH_PARENT;
        GtkWidget* pDialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView))),
                                                    eFlags,
                                                    GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE,
                                                    "Error occurred while opening the document: '%s'",
                                                    error->message);
        gtk_dialog_run(GTK_DIALOG(pDialog));
        gtk_widget_destroy(pDialog);

        g_error_free(error);
        gtk_widget_destroy(GTK_WIDGET(pDocView));
        gtk_main_quit();
        return;
    }

    populatePartSelector(pDocView);
    populatePartModeSelector( GTK_COMBO_BOX_TEXT(rWindow.m_pPartModeComboBox) );
    // Connect these signals after populating the selectors, to avoid re-rendering on setting the default part/partmode.
    g_signal_connect(G_OBJECT(rWindow.m_pPartModeComboBox), "changed", G_CALLBACK(changePartMode), 0);
    g_signal_connect(G_OBJECT(rWindow.m_pPartSelector), "changed", G_CALLBACK(changePart), 0);

    focusChain = g_list_append( focusChain, pDocView );
    gtk_container_set_focus_chain ( GTK_CONTAINER (rWindow.m_pVBox), focusChain );

    gtk_widget_hide(rWindow.m_pStatusBar);
}

/// Creates the GtkWindow that has main widget as children and registers it in the window map.
static GtkWidget* createWindow(TiledWindow& rWindow)
{
    GtkWidget *pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "LibreOfficeKit GTK Tiled Viewer");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 1024, 768);
    g_signal_connect(pWindow, "destroy", G_CALLBACK(gtk_main_quit), 0);

    rWindow.m_pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pWindow), rWindow.m_pVBox);

    // Toolbar
    GtkWidget* pToolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(pToolbar), GTK_TOOLBAR_ICONS);

    GtkToolItem* pZoomIn = gtk_tool_button_new(NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoomIn), "zoom-in-symbolic");
    gtk_tool_item_set_tooltip_text(pZoomIn, "Zoom In");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pZoomIn, 0);
    g_signal_connect(G_OBJECT(pZoomIn), "clicked", G_CALLBACK(changeZoom), NULL);

    GtkToolItem* pZoom1 = gtk_tool_button_new(NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(pZoom1), "zoom-original-symbolic");
    gtk_tool_item_set_tooltip_text(pZoom1, "Normal Size");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pZoom1, -1);
    g_signal_connect(G_OBJECT(pZoom1), "clicked", G_CALLBACK(changeZoom), NULL);

    GtkToolItem* pZoomOut = gtk_tool_button_new(NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoomOut), "zoom-out-symbolic");
    gtk_tool_item_set_tooltip_text(pZoomOut, "Zoom Out");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pZoomOut, -1);
    g_signal_connect(G_OBJECT(pZoomOut), "clicked", G_CALLBACK(changeZoom), NULL);

    GtkToolItem* pSeparator1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pSeparator1, -1);

    GtkToolItem* pPartSelectorToolItem = gtk_tool_item_new();
    GtkWidget* pComboBox = gtk_combo_box_text_new();
    gtk_container_add(GTK_CONTAINER(pPartSelectorToolItem), pComboBox);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pPartSelectorToolItem, -1);

    rWindow.m_pPartSelector = GTK_COMBO_BOX_TEXT(pComboBox);

    GtkToolItem* pSeparator2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pSeparator2, -1);

    GtkToolItem* pPartModeSelectorToolItem = gtk_tool_item_new();
    rWindow.m_pPartModeComboBox = gtk_combo_box_text_new();
    gtk_container_add(GTK_CONTAINER(pPartModeSelectorToolItem), rWindow.m_pPartModeComboBox);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pPartModeSelectorToolItem, -1);

    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);

    // Cut, copy & paste.
    GtkToolItem* pCopyButton = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(pCopyButton), "edit-copy-symbolic");
    gtk_tool_item_set_tooltip_text(pCopyButton, "Copy");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pCopyButton, -1);
    g_signal_connect(G_OBJECT(pCopyButton), "clicked", G_CALLBACK(doCopy), NULL);
    gtk_toolbar_insert( GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);

    GtkToolItem* pEnableEditing = gtk_toggle_tool_button_new();
    rWindow.m_pEnableEditing = pEnableEditing;
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pEnableEditing), "insert-text-symbolic");
    gtk_tool_item_set_tooltip_text(pEnableEditing, "Edit");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pEnableEditing, -1);
    g_signal_connect(G_OBJECT(pEnableEditing), "toggled", G_CALLBACK(toggleEditing), NULL);

    GtkToolItem* pFindButton = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindButton), "edit-find-symbolic");
    gtk_tool_item_set_tooltip_text(pFindButton, "Find");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pFindButton, -1);
    g_signal_connect(G_OBJECT(pFindButton), "clicked", G_CALLBACK(toggleFindbar), NULL);

    GtkToolItem* pNewViewButton = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pNewViewButton), "view-continuous-symbolic");
    gtk_tool_item_set_tooltip_text(pNewViewButton, "New View");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), pNewViewButton, -1);
    g_signal_connect(G_OBJECT(pNewViewButton), "clicked", G_CALLBACK(createView), NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), gtk_separator_tool_item_new(), -1);

    rWindow.m_pBold = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pBold), "format-text-bold-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pBold, "Bold");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), rWindow.m_pBold, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pBold), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(rWindow, rWindow.m_pBold, ".uno:Bold");

    rWindow.m_pItalic = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (rWindow.m_pItalic), "format-text-italic-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pItalic, "Italic");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), rWindow.m_pItalic, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pItalic), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(rWindow, rWindow.m_pItalic, ".uno:Italic");

    rWindow.m_pUnderline = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (rWindow.m_pUnderline), "format-text-underline-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pUnderline, "Underline");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), rWindow.m_pUnderline, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pUnderline), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(rWindow, rWindow.m_pUnderline, ".uno:Underline");

    rWindow.m_pStrikethrough = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pStrikethrough), "format-text-strikethrough-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pStrikethrough, "Strikethrough");
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), rWindow.m_pStrikethrough, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pStrikethrough), "toggled", G_CALLBACK(toggleToolItem), NULL);
    lcl_registerToolItem(rWindow, rWindow.m_pStrikethrough, ".uno:Strikeout");

    gtk_box_pack_start(GTK_BOX(rWindow.m_pVBox), pToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Findbar
    rWindow.m_pFindbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(rWindow.m_pFindbar), GTK_TOOLBAR_ICONS);

    GtkToolItem* pFindbarClose = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarClose), "window-close-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarClose, -1);
    g_signal_connect(G_OBJECT(pFindbarClose), "clicked", G_CALLBACK(toggleFindbar), NULL);

    GtkToolItem* pEntryContainer = gtk_tool_item_new();
    rWindow.m_pFindbarEntry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(pEntryContainer), rWindow.m_pFindbarEntry);
    g_signal_connect(rWindow.m_pFindbarEntry, "key-press-event", G_CALLBACK(signalFindbar), 0);
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pEntryContainer, -1);

    GtkToolItem* pFindbarNext = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarNext), "go-down-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarNext, -1);
    g_signal_connect(G_OBJECT(pFindbarNext), "clicked", G_CALLBACK(signalSearchNext), NULL);

    GtkToolItem* pFindbarPrev = gtk_tool_button_new( NULL, NULL);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarPrev), "go-up-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarPrev, -1);
    g_signal_connect(G_OBJECT(pFindbarPrev), "clicked", G_CALLBACK(signalSearchPrev), NULL);

    GtkToolItem* pFindAll = gtk_toggle_tool_button_new();
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(pFindAll), "Highlight All");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindAll, -1);
    g_signal_connect(G_OBJECT(pFindAll), "toggled", G_CALLBACK(toggleFindAll), NULL);

    GtkToolItem* pFindbarLabelContainer = gtk_tool_item_new();
    rWindow.m_pFindbarLabel = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(pFindbarLabelContainer), rWindow.m_pFindbarLabel);
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarLabelContainer, -1);

    gtk_box_pack_end(GTK_BOX(rWindow.m_pVBox), rWindow.m_pFindbar, FALSE, FALSE, 0);

    // Scrolled window for DocView
    rWindow.m_pScrolledWindow = gtk_scrolled_window_new(0, 0);
    gtk_widget_set_hexpand(rWindow.m_pScrolledWindow, TRUE);
    gtk_widget_set_vexpand(rWindow.m_pScrolledWindow, TRUE);
    gtk_container_add(GTK_CONTAINER(rWindow.m_pVBox), rWindow.m_pScrolledWindow);

    gtk_container_add(GTK_CONTAINER(rWindow.m_pScrolledWindow), rWindow.m_pDocView);

    GtkWidget* pProgressBar = gtk_progress_bar_new ();
    g_signal_connect(rWindow.m_pDocView, "load-changed", G_CALLBACK(loadChanged), pProgressBar);

    GtkWidget* pStatusBar = gtk_statusbar_new();
    rWindow.m_pStatusBar = pStatusBar;
    gtk_container_forall(GTK_CONTAINER(pStatusBar), removeChildrenFromStatusbar, pStatusBar);
    gtk_container_add (GTK_CONTAINER(rWindow.m_pVBox), pStatusBar);
    gtk_container_add (GTK_CONTAINER(pStatusBar), pProgressBar);
    gtk_widget_set_hexpand(pProgressBar, true);

    gtk_widget_show_all(pWindow);
    // Hide the findbar by default.
    gtk_widget_hide(rWindow.m_pFindbar);

    g_aWindows[pWindow] = rWindow;
    return pWindow;
}

/// Common setup for DocView (regardless if it's just a new view or a document to be loaded).
static void setupDocView(GtkWidget* pDocView)
{
#if GLIB_CHECK_VERSION(2,40,0)
    g_assert_nonnull(pDocView);
#endif
    g_signal_connect(pDocView, "edit-changed", G_CALLBACK(signalEdit), NULL);
    g_signal_connect(pDocView, "command-changed", G_CALLBACK(signalCommand), NULL);
    g_signal_connect(pDocView, "search-not-found", G_CALLBACK(signalSearch), NULL);
    g_signal_connect(pDocView, "search-result-count", G_CALLBACK(signalSearchResultCount), NULL);
    g_signal_connect(pDocView, "part-changed", G_CALLBACK(signalPart), NULL);
    g_signal_connect(pDocView, "size-changed", G_CALLBACK(signalSize), NULL);
    g_signal_connect(pDocView, "hyperlink-clicked", G_CALLBACK(signalHyperlink), NULL);
    g_signal_connect(pDocView, "cursor-changed", G_CALLBACK(cursorChanged), NULL);
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

    createModelAndView(argv[1], argv[2]);

    gtk_main();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
