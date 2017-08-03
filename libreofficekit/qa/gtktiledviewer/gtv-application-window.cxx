/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <memory>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <gtv-application-window.hxx>
#include <gtv-main-toolbar.hxx>
#include <gtv-helpers.hxx>
#include <gtv-signal-handlers.hxx>
#include <gtv-lokdocview-signal-handlers.hxx>
#include <gtv-calc-header-bar.hxx>
#include <gtv-comments-sidebar.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

struct GtvApplicationWindowPrivate
{
    GtkWidget* container;
    GtkWidget* gridcontainer;
    GtkWidget* toolbarcontainer;
    GtkWidget* scrolledwindowcontainer;
    GtkWidget* lokDialog;

    gboolean toolbarBroadcast;
    gboolean partSelectorBroadcast;

    GList* m_pChildWindows;

    // Rendering args; options with which lokdocview was rendered in this window
    GtvRenderingArgs* m_pRenderingArgs;
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvApplicationWindow, gtv_application_window, GTK_TYPE_APPLICATION_WINDOW);

static GtvApplicationWindowPrivate*
getPrivate(GtvApplicationWindow* win)
{
    return static_cast<GtvApplicationWindowPrivate*>(gtv_application_window_get_instance_private(win));
}

static void
gtv_application_window_init(GtvApplicationWindow* win)
{
    const std::string uiFilePath = GtvHelpers::getDirPath(__FILE__) + std::string(UI_FILE_NAME);
    GtvGtkWrapper<GtkBuilder> builder(gtk_builder_new_from_file(uiFilePath.c_str()),
                                      [](GtkBuilder* pBuilder) {
                                          g_object_unref(pBuilder);
                                      });
    GtvApplicationWindowPrivate* priv = getPrivate(win);

    // This is the parent GtkBox holding everything
    priv->container = GTK_WIDGET(gtk_builder_get_object(builder.get(), "container"));
    // Toolbar container
    priv->toolbarcontainer = gtv_main_toolbar_new();

    // Attach to the toolbar to main window
    gtk_box_pack_start(GTK_BOX(priv->container), priv->toolbarcontainer, false, false, false);
    gtk_box_reorder_child(GTK_BOX(priv->container), priv->toolbarcontainer, 0);

    priv->gridcontainer = GTK_WIDGET(gtk_builder_get_object(builder.get(), "maingrid"));
    // scrolled window containing the main drawing area
    win->scrolledwindow = GTK_WIDGET(gtk_builder_get_object(builder.get(), "scrolledwindow"));
    // scrolledwindow container
    priv->scrolledwindowcontainer = GTK_WIDGET(gtk_builder_get_object(builder.get(), "scrolledwindowcontainer"));

    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(win->scrolledwindow));
    g_signal_connect(pHAdjustment, "value-changed", G_CALLBACK(docAdjustmentChanged), win);
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(win->scrolledwindow));
    g_signal_connect(pVAdjustment, "value-changed", G_CALLBACK(docAdjustmentChanged), win);

    // calc header row bar
    win->cornerarea = gtv_calc_header_bar_new();
    gtv_calc_header_bar_set_type_and_width(GTV_CALC_HEADER_BAR(win->cornerarea), CalcHeaderType::CORNER);
    win->rowbar = gtv_calc_header_bar_new();
    gtv_calc_header_bar_set_type_and_width(GTV_CALC_HEADER_BAR(win->rowbar), CalcHeaderType::ROW);
    win->columnbar = gtv_calc_header_bar_new();
    gtv_calc_header_bar_set_type_and_width(GTV_CALC_HEADER_BAR(win->columnbar), CalcHeaderType::COLUMN);

    // attach row/column/corner to the container
    gtk_grid_attach(GTK_GRID(priv->gridcontainer), win->cornerarea, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(priv->gridcontainer), win->rowbar, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(priv->gridcontainer), win->columnbar, 1, 0, 1, 1);

    // statusbar
    win->statusbar = GTK_WIDGET(gtk_builder_get_object(builder.get(), "statusbar"));
    win->redlinelabel = GTK_WIDGET(gtk_builder_get_object(builder.get(), "redlinelabel"));
    win->zoomlabel = GTK_WIDGET(gtk_builder_get_object(builder.get(), "zoomlabel"));

    win->findtoolbar = GTK_WIDGET(gtk_builder_get_object(builder.get(), "findtoolbar"));
    win->findbarlabel = GTK_WIDGET(gtk_builder_get_object(builder.get(), "findbar_label"));
    win->findbarEntry = GTK_WIDGET(gtk_builder_get_object(builder.get(), "findbar_entry"));
    win->findAll = GTK_WIDGET(gtk_builder_get_object(builder.get(), "findbar_findall"));
    priv->toolbarBroadcast = true;
    priv->partSelectorBroadcast = true;

    gtk_container_add(GTK_CONTAINER(win), priv->container);

    priv->m_pChildWindows = nullptr;
    priv->m_pRenderingArgs = new GtvRenderingArgs();
}

static void
gtv_application_window_dispose(GObject* object)
{
    GtvApplicationWindowPrivate* priv = getPrivate(GTV_APPLICATION_WINDOW(object));

    delete priv->m_pRenderingArgs;
    priv->m_pRenderingArgs = nullptr;

    G_OBJECT_CLASS (gtv_application_window_parent_class)->dispose (object);
}

static void
gtv_application_window_class_init(GtvApplicationWindowClass* klass)
{
    G_OBJECT_CLASS(klass)->dispose = gtv_application_window_dispose;
}

/// Helper function to do some tasks after widget is fully loaded (including
/// document load)
static void initWindow(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);

    GList *focusChain = nullptr;
    focusChain = g_list_append( focusChain, window->lokdocview );
    gtk_container_set_focus_chain ( GTK_CONTAINER (priv->container), focusChain );

    // TODO: Implement progressbar in statusbar
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    if (pDocument)
    {
        LibreOfficeKitDocumentType eDocType = static_cast<LibreOfficeKitDocumentType>(pDocument->pClass->getDocumentType(pDocument));
        if (eDocType == LOK_DOCTYPE_SPREADSHEET)
        {
            // Align to top left corner, so the tiles are in sync with the
            // row/column bar, even when zooming out enough that not all space is
            // used.
            gtk_widget_set_halign(GTK_WIDGET(window->lokdocview), GTK_ALIGN_START);
            gtk_widget_set_valign(GTK_WIDGET(window->lokdocview), GTK_ALIGN_START);
        }

        // By default make the document editable in a new window
        lok_doc_view_set_edit(LOK_DOC_VIEW(window->lokdocview), true);
        // Let toolbar adjust its button accordingly
        gtv_main_toolbar_doc_loaded(GTV_MAIN_TOOLBAR(priv->toolbarcontainer), eDocType, true /* Edit button state */);
    }

    // Fill our comments sidebar
    gboolean bTiledAnnotations;
    g_object_get(G_OBJECT(window->lokdocview), "tiled-annotations", &bTiledAnnotations, nullptr);
    if (!bTiledAnnotations && pDocument)
    {
        window->commentssidebar = gtv_comments_sidebar_new();
        gtk_container_add(GTK_CONTAINER(priv->scrolledwindowcontainer), window->commentssidebar);
        // fill the comments sidebar
        gtv_comments_sidebar_view_annotations(GTV_COMMENTS_SIDEBAR(window->commentssidebar));
    }
}

static void
gtv_application_open_document_callback(GObject* source_object, GAsyncResult* res, gpointer /*userdata*/)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (source_object);
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    GError* error = nullptr;
    if (!lok_doc_view_open_document_finish(pDocView, res, &error))
    {
        GtkWidget* pDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
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

    initWindow(window);
}

/// Get the visible area of the scrolled window
void gtv_application_window_get_visible_area(GtvApplicationWindow* pWindow, GdkRectangle* pArea)
{
    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(pWindow->scrolledwindow));
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(pWindow->scrolledwindow));

    pArea->x      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pWindow->lokdocview),
                                               gtk_adjustment_get_value(pHAdjustment));
    pArea->y      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pWindow->lokdocview),
                                               gtk_adjustment_get_value(pVAdjustment));
    pArea->width  = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pWindow->lokdocview),
                                               gtk_adjustment_get_page_size(pHAdjustment));
    pArea->height = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pWindow->lokdocview),
                                               gtk_adjustment_get_page_size(pVAdjustment));
}

void gtv_application_window_toggle_findbar(GtvApplicationWindow* window)
{
    if (gtk_widget_get_visible(window->findtoolbar))
    {
        gtk_widget_hide(window->findtoolbar);
    }
    else
    {
        gtk_widget_show_all(window->findtoolbar);
        gtk_widget_grab_focus(window->findtoolbar);
    }
}

GtkToolItem* gtv_application_window_find_tool_by_unocommand(GtvApplicationWindow* window, const std::string& unoCmd)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    GtkToolItem* result = nullptr;

    // Find in the first toolbar
    GtkContainer* pToolbar1 = gtv_main_toolbar_get_first_toolbar(GTV_MAIN_TOOLBAR(priv->toolbarcontainer));
    GtvGtkWrapper<GList> pList(gtk_container_get_children(pToolbar1),
                               [](GList* l)
                               {
                                   g_list_free(l);
                               });
    for (GList* l = pList.get(); l != nullptr; l = l->next)
    {
        if (GTK_IS_TOOL_BUTTON(l->data))
        {
            GtkToolButton* pButton = GTK_TOOL_BUTTON(l->data);
            const gchar* pLabel = gtk_tool_button_get_label(pButton);
            if (g_strcmp0(unoCmd.c_str(), pLabel) == 0)
            {
                result = GTK_TOOL_ITEM(pButton);
            }
        }
    }

    // Look in second toolbar if not found
    GtkContainer* pToolbar2 = gtv_main_toolbar_get_second_toolbar(GTV_MAIN_TOOLBAR(priv->toolbarcontainer));
    pList.reset(gtk_container_get_children(pToolbar2));
    for (GList* l = pList.get(); result == nullptr && l != nullptr; l = l->next)
    {
        if (GTK_IS_TOOL_BUTTON(l->data))
        {
            GtkToolButton* pButton = GTK_TOOL_BUTTON(l->data);
            const gchar* pLabel = gtk_tool_button_get_label(pButton);
            if (g_strcmp0(unoCmd.c_str(), pLabel) == 0)
            {
                result = GTK_TOOL_ITEM(pButton);
            }
        }
    }

    return result;
}

static const std::string
createRenderingArgsJSON(const GtvRenderingArgs* pRenderingArgs)
{
    boost::property_tree::ptree aTree;
    if (pRenderingArgs->m_bHidePageShadow)
    {
        aTree.put(boost::property_tree::ptree::path_type(".uno:ShowBorderShadow/type", '/'), "boolean");
        aTree.put(boost::property_tree::ptree::path_type(".uno:ShowBorderShadow/value", '/'), false);
    }
    if (pRenderingArgs->m_bHideWhiteSpace)
    {
        aTree.put(boost::property_tree::ptree::path_type(".uno:HideWhitespace/type", '/'), "boolean");
        aTree.put(boost::property_tree::ptree::path_type(".uno:HideWhitespace/value", '/'), true);
    }
    aTree.put(boost::property_tree::ptree::path_type(".uno:Author/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type(".uno:Author/value", '/'), GtvHelpers::getNextAuthor());
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    return aStream.str();
}

static void setupDocView(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    g_object_set(G_OBJECT(window->lokdocview),
                 "doc-password", TRUE,
                 "doc-password-to-modify", TRUE,
                 "tiled-annotations", priv->m_pRenderingArgs->m_bEnableTiledAnnotations,
                 nullptr);

#if GLIB_CHECK_VERSION(2,40,0)
    g_assert_nonnull(window->lokdocview);
#endif
    g_signal_connect(window->lokdocview, "edit-changed", G_CALLBACK(LOKDocViewSigHandlers::editChanged), nullptr);
    g_signal_connect(window->lokdocview, "command-changed", G_CALLBACK(LOKDocViewSigHandlers::commandChanged), nullptr);
    g_signal_connect(window->lokdocview, "command-result", G_CALLBACK(LOKDocViewSigHandlers::commandResult), nullptr);
    g_signal_connect(window->lokdocview, "search-not-found", G_CALLBACK(LOKDocViewSigHandlers::searchNotFound), nullptr);
    g_signal_connect(window->lokdocview, "search-result-count", G_CALLBACK(LOKDocViewSigHandlers::searchResultCount), nullptr);
    g_signal_connect(window->lokdocview, "part-changed", G_CALLBACK(LOKDocViewSigHandlers::partChanged), nullptr);
    g_signal_connect(window->lokdocview, "hyperlink-clicked", G_CALLBACK(LOKDocViewSigHandlers::hyperlinkClicked), nullptr);
    g_signal_connect(window->lokdocview, "cursor-changed", G_CALLBACK(LOKDocViewSigHandlers::cursorChanged), nullptr);
    g_signal_connect(window->lokdocview, "address-changed", G_CALLBACK(LOKDocViewSigHandlers::addressChanged), nullptr);
    g_signal_connect(window->lokdocview, "formula-changed", G_CALLBACK(LOKDocViewSigHandlers::formulaChanged), nullptr);
    g_signal_connect(window->lokdocview, "password-required", G_CALLBACK(LOKDocViewSigHandlers::passwordRequired), nullptr);
    g_signal_connect(window->lokdocview, "comment", G_CALLBACK(LOKDocViewSigHandlers::comment), nullptr);
    g_signal_connect(window->lokdocview, "dialog-invalidate", G_CALLBACK(LOKDocViewSigHandlers::dialogInvalidate), nullptr);
    g_signal_connect(window->lokdocview, "dialog-child", G_CALLBACK(LOKDocViewSigHandlers::dialogChild), nullptr);

    g_signal_connect(window->lokdocview, "configure-event", G_CALLBACK(LOKDocViewSigHandlers::configureEvent), nullptr);
}

void
gtv_application_window_create_view_from_window(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    GApplication* app = g_application_get_default();

    GtvApplicationWindow* newWindow = GTV_APPLICATION_WINDOW(gtv_application_window_new(GTK_APPLICATION(app)));
    const std::string aArguments = createRenderingArgsJSON(priv->m_pRenderingArgs);
    newWindow->lokdocview = lok_doc_view_new_from_widget(LOK_DOC_VIEW(window->lokdocview), aArguments.c_str());
    setupDocView(newWindow);

    gtk_container_add(GTK_CONTAINER(newWindow->scrolledwindow), newWindow->lokdocview);
    gtk_widget_show_all(newWindow->scrolledwindow);
    gtk_window_present(GTK_WINDOW(newWindow));

    initWindow(newWindow);
}

void
gtv_application_window_load_document(GtvApplicationWindow* window,
                                     const GtvRenderingArgs* aArgs,
                                     const std::string& aDocPath)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    // keep a copy of it; we need to use these for creating new views later
    *(priv->m_pRenderingArgs) = *aArgs;

    // setup lokdocview
    window->lokdocview = lok_doc_view_new_from_user_profile(priv->m_pRenderingArgs->m_aLoPath.c_str(),
                                                            priv->m_pRenderingArgs->m_aUserProfile.empty() ? nullptr : priv->m_pRenderingArgs->m_aUserProfile.c_str(),
                                                          nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(window->scrolledwindow), window->lokdocview);

    setupDocView(window);

    // Create argument JSON
    const std::string aArguments = createRenderingArgsJSON(priv->m_pRenderingArgs);
    lok_doc_view_open_document(LOK_DOC_VIEW(window->lokdocview), aDocPath.c_str(),
                               aArguments.c_str(), nullptr,
                               gtv_application_open_document_callback, window->lokdocview);

    gtk_widget_show_all(GTK_WIDGET(window->scrolledwindow));
}

GtvMainToolbar*
gtv_application_window_get_main_toolbar(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    return GTV_MAIN_TOOLBAR(priv->toolbarcontainer);
}

void
gtv_application_window_set_toolbar_broadcast(GtvApplicationWindow* window, bool broadcast)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    priv->toolbarBroadcast = broadcast;
}

gboolean
gtv_application_window_get_toolbar_broadcast(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    return priv->toolbarBroadcast;
}

void
gtv_application_window_set_part_broadcast(GtvApplicationWindow* window, bool broadcast)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    priv->partSelectorBroadcast = broadcast;
}

gboolean
gtv_application_window_get_part_broadcast(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    return priv->partSelectorBroadcast;
}

void
gtv_application_window_register_child_window(GtvApplicationWindow* window, GtkWindow* pChildWin)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    if (pChildWin)
        priv->m_pChildWindows = g_list_append(priv->m_pChildWindows, pChildWin);
}

void
gtv_application_window_unregister_child_window(GtvApplicationWindow* window, GtkWindow* pChildWin)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    if (pChildWin)
        priv->m_pChildWindows = g_list_remove(priv->m_pChildWindows, pChildWin);
}

GtkWindow*
gtv_application_window_get_child_window_by_id(GtvApplicationWindow* window, const gchar* pWinId)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    GList* pIt = nullptr;
    GtkWindow* ret = nullptr;
    // For now, only dialogs are registered as child window
    for (pIt = priv->m_pChildWindows; pIt != nullptr; pIt = pIt->next)
    {
        gchar* dialogId = nullptr;
        g_object_get(G_OBJECT(pIt->data), "dialogid", &dialogId, nullptr);

        // prepend .uno:
        gchar* completeWinId = nullptr;
        if (pWinId != nullptr)
        {
            completeWinId = g_strconcat(".uno:", pWinId, nullptr);
        }

        if (dialogId != nullptr && g_str_equal(dialogId, completeWinId))
        {
            ret = GTK_WINDOW(pIt->data);
            break;
        }
    }
    return ret;
}

// temporary function to invalidate all opened dialogs
// because currently the dialog id returned in dialog invalidation payload
// doesn't match our hard-coded list of dialog ids (uno commands) for some dialogs
GList*
gtv_application_window_get_all_child_windows(GtvApplicationWindow* window)
{
    GtvApplicationWindowPrivate* priv = getPrivate(window);
    return priv->m_pChildWindows;
}

GtvApplicationWindow*
gtv_application_window_new(GtkApplication* app)
{
    g_return_val_if_fail(GTK_IS_APPLICATION(app), nullptr);

    return GTV_APPLICATION_WINDOW(g_object_new(GTV_TYPE_APPLICATION_WINDOW,
                                               "application", app,
                                               "width-request", 1024,
                                               "height-request", 768,
                                               "title", "LibreOffice GtkTiledViewer",
                                               "window-position", GTK_WIN_POS_CENTER,
                                               "show-menubar", false,
                                               nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
