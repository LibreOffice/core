/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include "gtv-application-window.hxx"
#include "gtv-helpers.hxx"
#include "gtv-calc-header-bar.hxx"
#include "gtv-comments-sidebar.hxx"
#include "gtv-lokdocview-signal-handlers.hxx"
#include "gtv-lok-dialog.hxx"

#include <boost/property_tree/json_parser.hpp>

static gboolean deleteLokDialog(GtkWidget* pWidget, GdkEvent* /*event*/, gpointer userdata)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(userdata);
    g_info("deleteLokDialog");
    gtv_application_window_unregister_child_window(window, GTK_WINDOW(pWidget));

    return FALSE;
}

static gboolean destroyLokDialog(GtkWidget* pWidget, gpointer userdata)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(userdata);
    g_info("destroyLokDialog");
    gtv_application_window_unregister_child_window(window, GTK_WINDOW(pWidget));

    return FALSE;
}

void LOKDocViewSigHandlers::editChanged(LOKDocView* pDocView, gboolean bWasEdit, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    bool bEdit = lok_doc_view_get_edit(LOK_DOC_VIEW(window->lokdocview));
    g_info("signalEdit: %d -> %d", bWasEdit, bEdit);

    // Let the main toolbar know, so that it can enable disable the button
    GtvMainToolbar* pMainToolbar = gtv_application_window_get_main_toolbar(GTV_APPLICATION_WINDOW(window));
    gtv_main_toolbar_set_edit(pMainToolbar, bEdit);
}

void LOKDocViewSigHandlers::commandChanged(LOKDocView* pDocView, char* pPayload, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    std::string aPayload(pPayload);
    size_t nPosition = aPayload.find('=');
    if (nPosition == std::string::npos)
        return;

    const std::string aKey = aPayload.substr(0, nPosition);
    const std::string aValue = aPayload.substr(nPosition + 1);
    GtkToolItem* pItem = gtv_application_window_find_tool_by_unocommand(window, aKey);
    if (pItem != nullptr)
    {
        if (aValue == "true" || aValue == "false") {
            bool bEdit = aValue == "true";
            if (bool(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pItem))) != bEdit)
            {
                // Avoid invoking lok_doc_view_post_command().
                // FIXME: maybe block/unblock the signal (see
                // g_signal_handlers_block_by_func) ?
                gtv_application_window_set_toolbar_broadcast(window, false);
                gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pItem), bEdit);
                gtv_application_window_set_toolbar_broadcast(window, true);

            }
        } else if (aValue == "enabled" || aValue == "disabled") {
            bool bSensitive = aValue == "enabled";
            gtk_widget_set_sensitive(GTK_WIDGET(pItem), bSensitive);

            // Remember state, so in case edit is disable and enabled
            // later, the correct sensitivity can be restored.
            GtvMainToolbar* pMainToolbar = gtv_application_window_get_main_toolbar(window);
            gtv_main_toolbar_set_sensitive_internal(pMainToolbar, pItem, bSensitive);
        }
    }
    else if (aKey == ".uno:TrackedChangeIndex")
    {
        std::string aText("Current redline: ");
        if (aValue.empty())
            aText += "none";
        else
            aText += aValue;
        gtk_label_set_text(GTK_LABEL(window->redlinelabel), aText.c_str());
    }
}

void LOKDocViewSigHandlers::commandResult(LOKDocView*, char* pPayload, gpointer)
{
    fprintf(stderr, "Command finished: %s\n", pPayload);
}

void LOKDocViewSigHandlers::searchNotFound(LOKDocView* pDocView, char* , gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    gtk_label_set_text(GTK_LABEL(window->findbarlabel), "Search key not found");
}

void LOKDocViewSigHandlers::searchResultCount(LOKDocView* pDocView, char* pPayload, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    std::stringstream ss;
    ss << pPayload << " match(es)";
    gtk_label_set_text(GTK_LABEL(window->findbarlabel), ss.str().c_str());
}

void LOKDocViewSigHandlers::partChanged(LOKDocView* /*pDocView*/, int, gpointer)
{
//    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    //rWindow.m_bPartSelectorBroadcast = false;
//    gtk_combo_box_set_active(GTK_COMBO_BOX(rWindow.m_pPartSelector), nPart);
    //  rWindow.m_bPartSelectorBroadcast = true;
}

void LOKDocViewSigHandlers::hyperlinkClicked(LOKDocView* pDocView, char* pPayload, gpointer)
{
    GError* pError = nullptr;
#if GTK_CHECK_VERSION(3,22,0)
    gtk_show_uri_on_window(
        GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(pDocView))),
        pPayload, GDK_CURRENT_TIME, &pError);
#else
    (void) pDocView;
    gtk_show_uri(nullptr, pPayload, GDK_CURRENT_TIME, &pError);
#endif
    if (pError != nullptr)
    {
        g_warning("Unable to show URI %s : %s", pPayload, pError->message);
        g_error_free(pError);
    }
}

void LOKDocViewSigHandlers::cursorChanged(LOKDocView* pDocView, gint nX, gint nY,
                              gint /*nWidth*/, gint /*nHeight*/, gpointer /*pData*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(window->scrolledwindow));
    GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(window->scrolledwindow));
    GdkRectangle visArea;
    gdouble upper;
    gint x = -1, y = -1;

    gtv_application_window_get_visible_area(window, &visArea);

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
        upper = lok_doc_view_pixel_to_twip(pDocView, gtk_adjustment_get_upper(vadj));
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
        upper = lok_doc_view_pixel_to_twip(pDocView, gtk_adjustment_get_upper(hadj));
        if (x > upper)
            x = upper;
    }

    if (y!=-1)
        gtk_adjustment_set_value(vadj, lok_doc_view_twip_to_pixel(pDocView, y));
    if (x!=-1)
        gtk_adjustment_set_value(hadj, lok_doc_view_twip_to_pixel(pDocView, x));
}

void LOKDocViewSigHandlers::addressChanged(LOKDocView* pDocView, char* pPayload, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    GtvMainToolbar* toolbar = gtv_application_window_get_main_toolbar(window);
    GtkEntry* pAddressbar = GTK_ENTRY(toolbar->m_pAddressbar);
    gtk_entry_set_text(pAddressbar, pPayload);
}

void LOKDocViewSigHandlers::formulaChanged(LOKDocView* pDocView, char* pPayload, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    GtvMainToolbar* toolbar = gtv_application_window_get_main_toolbar(window);
    GtkEntry* pFormulabar = GTK_ENTRY(toolbar->m_pFormulabar);
    gtk_entry_set_text(pFormulabar, pPayload);
}

void LOKDocViewSigHandlers::passwordRequired(LOKDocView* pDocView, char* pUrl, gboolean bModify, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    GtkWidget* pPasswordDialog = gtk_dialog_new_with_buttons ("Password required",
                                                              GTK_WINDOW (window),
                                                              GTK_DIALOG_MODAL,
                                                              "OK",
                                                              GTK_RESPONSE_OK,
                                                              nullptr);
    g_object_set(G_OBJECT(pPasswordDialog), "resizable", FALSE, nullptr);
    GtkWidget* pDialogMessageArea = gtk_dialog_get_content_area (GTK_DIALOG (pPasswordDialog));
    GtkWidget* pPasswordEntry = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY(pPasswordEntry), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY(pPasswordEntry), '*');
    gtk_box_pack_end(GTK_BOX(pDialogMessageArea), pPasswordEntry, true, true, 2);
    if (bModify)
    {
        GtkWidget* pSecondaryLabel = gtk_label_new ("Document requires password to edit");
        gtk_box_pack_end(GTK_BOX(pDialogMessageArea), pSecondaryLabel, true, true, 2);
        gtk_dialog_add_button (GTK_DIALOG (pPasswordDialog), "Open as read-only", GTK_RESPONSE_ACCEPT);
    }
    gtk_widget_show_all(pPasswordDialog);

    gint res = gtk_dialog_run (GTK_DIALOG(pPasswordDialog));
    switch (res)
    {
    case GTK_RESPONSE_OK:
        lok_doc_view_set_document_password (LOK_DOC_VIEW(window->lokdocview), pUrl, gtk_entry_get_text(GTK_ENTRY(pPasswordEntry)));
        break;
    case GTK_RESPONSE_ACCEPT:
        // User accepts to open this document as read-only
    case GTK_RESPONSE_DELETE_EVENT:
        lok_doc_view_set_document_password (LOK_DOC_VIEW(window->lokdocview), pUrl, nullptr);
        break;
    }

    gtk_widget_destroy(pPasswordDialog);
}

void LOKDocViewSigHandlers::comment(LOKDocView* pDocView, gchar* pComment, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));

    std::stringstream aStream(pComment);
    boost::property_tree::ptree aRoot;
    boost::property_tree::read_json(aStream, aRoot);
    boost::property_tree::ptree aComment = aRoot.get_child("comment");
    GtvCommentsSidebar* sidebar = GTV_COMMENTS_SIDEBAR(window->commentssidebar);
    GtkWidget* pCommentsGrid = sidebar->commentsgrid;
    GtvGtkWrapper<GList> pChildren(gtk_container_get_children(GTK_CONTAINER(pCommentsGrid)),
                                   [](GList* l)
                                   {
                                       g_list_free(l);
                                   });
    GtkWidget* pSelf = nullptr;
    GtkWidget* pParent = nullptr;
    for (GList* l = pChildren.get(); l != nullptr; l = l->next)
    {
        gchar *id = static_cast<gchar*>(g_object_get_data(G_OBJECT(l->data), "id"));

        if (g_strcmp0(id, aComment.get<std::string>("id").c_str()) == 0)
            pSelf = GTK_WIDGET(l->data);

        // There is no 'parent' in Remove callbacks
        if (g_strcmp0(id, aComment.get("parent", std::string("0")).c_str()) == 0)
            pParent = GTK_WIDGET(l->data);
    }

    if (aComment.get<std::string>("action") == "Remove")
    {
        if (pSelf)
            gtk_widget_destroy(pSelf);
        else
            g_warning("Can't find the comment to remove in the list !!");
    }
    else if (aComment.get<std::string>("action") == "Add" || aComment.get<std::string>("action") == "Modify")
    {
        GtkWidget* pCommentBox = GtvHelpers::createCommentBox(aComment);
        if (pSelf != nullptr || pParent != nullptr)
        {
            gtk_grid_insert_next_to(GTK_GRID(pCommentsGrid), pSelf != nullptr ? pSelf : pParent, GTK_POS_BOTTOM);
            gtk_grid_attach_next_to(GTK_GRID(pCommentsGrid), pCommentBox, pSelf != nullptr ? pSelf : pParent, GTK_POS_BOTTOM, 1, 1);
        }
        else
            gtk_container_add(GTK_CONTAINER(pCommentsGrid), pCommentBox);

        gtk_widget_show_all(pCommentBox);

        // We added the widget already below the existing one, so destroy the
        // already existing one now
        if (pSelf)
            gtk_widget_destroy(pSelf);
    }
}

void LOKDocViewSigHandlers::window(LOKDocView* pDocView, gchar* pPayload, gpointer pData)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(pData);

    std::stringstream aStream(pPayload);
    boost::property_tree::ptree aRoot;
    boost::property_tree::read_json(aStream, aRoot);
    const unsigned nWinId = aRoot.get<unsigned>("id");
    const std::string aAction = aRoot.get<std::string>("action");

    if (aAction == "created")
    {
        const std::string aType = aRoot.get<std::string>("type");
        const std::string aSize = aRoot.get<std::string>("size");
        const std::string aTitle = aRoot.get<std::string>("title", "");
        std::vector<int> aSizePoints = GtvHelpers::split<int>(aSize, ", ", 2);

        if (aType == "dialog")
        {
            GtkWidget* pDialog = gtv_lok_dialog_new(pDocView, nWinId, aSizePoints[0], aSizePoints[1]);
            g_info("created  dialog, for dialogid: %d with size: %s", nWinId, aSize.c_str());

            gtv_application_window_register_child_window(window, GTK_WINDOW(pDialog));
            g_signal_connect(pDialog, "destroy", G_CALLBACK(destroyLokDialog), window);
            g_signal_connect(pDialog, "delete-event", G_CALLBACK(deleteLokDialog), window);

            if (!aTitle.empty())
                gtk_window_set_title(GTK_WINDOW(pDialog), aTitle.c_str());

            gtk_window_set_resizable(GTK_WINDOW(pDialog), false);
            gtk_widget_show_all(GTK_WIDGET(pDialog));
            gtk_window_present(GTK_WINDOW(pDialog));
        }
        else if (aType == "child")
        {
            const unsigned nParentId = std::atoi(aRoot.get<std::string>("parentId").c_str());
            GtkWindow* pDialog = gtv_application_window_get_child_window_by_id(window, nParentId);
            const std::string aPos = aRoot.get<std::string>("position");
            std::vector<int> aPosPoints = GtvHelpers::split<int>(aPos, ", ", 2);
            gtv_lok_dialog_child_create(GTV_LOK_DIALOG(pDialog), nWinId, aPosPoints[0], aPosPoints[1], aSizePoints[0], aSizePoints[1]);
        }
    }
    else
    {
        // check if it's a child window
        GtkWidget* pParent = gtv_application_window_get_parent(window, nWinId);
        if (pParent) // it's a floating window in the dialog
        {
            if (aAction == "invalidate")
                gtv_lok_dialog_child_invalidate(GTV_LOK_DIALOG(pParent));
            else if (aAction == "close")
                gtv_lok_dialog_child_close(GTV_LOK_DIALOG(pParent));
        }
        else if (GtkWindow* pDialog = gtv_application_window_get_child_window_by_id(window, nWinId))
        { // it's the dialog window itself
            if (aAction == "close")
                gtk_widget_destroy(GTK_WIDGET(pDialog));
            else if (aAction == "size_changed")
            {
                const std::string aSize = aRoot.get<std::string>("size");
                std::vector<int> aSizePoints = GtvHelpers::split<int>(aSize, ", ", 2);
                if (aSizePoints.size() != 2)
                {
                    g_error("Malformed size_changed callback");
                    return;
                }

                g_object_set(G_OBJECT(pDialog),
                             "width", aSizePoints[0],
                             "height", aSizePoints[1],
                             nullptr);

                GdkRectangle aGdkRectangle = {0, 0, 0, 0};
                gtv_lok_dialog_invalidate(GTV_LOK_DIALOG(pDialog), aGdkRectangle);
            }
            else if (aAction == "invalidate")
            {
                GdkRectangle aGdkRectangle = {0, 0, 0, 0};
                try
                {
                    const std::string aRectangle = aRoot.get<std::string>("rectangle");
                    std::vector<int> aRectPoints = GtvHelpers::split<int>(aRectangle, ", ", 4);
                    if (aRectPoints.size() == 4)
                        aGdkRectangle = {aRectPoints[0], aRectPoints[1], aRectPoints[2], aRectPoints[3]};
                }
                catch(const std::exception&)
                {}

                gtv_lok_dialog_invalidate(GTV_LOK_DIALOG(pDialog), aGdkRectangle);
            }
            else if (aAction == "title_changed")
            {
                const std::string aTitle = aRoot.get<std::string>("title", "");
                gtk_window_set_title(pDialog, aTitle.c_str());
            }
        }
    }
}

gboolean LOKDocViewSigHandlers::configureEvent(GtkWidget* pWidget, GdkEventConfigure* /*pEvent*/, gpointer /*pData*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pWidget)));

    gboolean isInit = false;
    g_object_get(G_OBJECT(window->lokdocview), "is-initialized", &isInit, nullptr);
    if (!isInit)
    {
        g_info("Ignoring configure event; document not yet ready");
        return false;
    }

    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    if (pDocument && pDocument->pClass->getDocumentType(pDocument) == LOK_DOCTYPE_SPREADSHEET)
    {
        GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(window->scrolledwindow));
        int rowSizePixel = GTV_CALC_HEADER_BAR(window->rowbar)->m_nSizePixel = gtk_adjustment_get_page_size(pVAdjustment);
        int rowPosPixel = GTV_CALC_HEADER_BAR(window->rowbar)->m_nPositionPixel = gtk_adjustment_get_value(pVAdjustment);
        GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(window->scrolledwindow));
        int colSizePixel = GTV_CALC_HEADER_BAR(window->columnbar)->m_nSizePixel = gtk_adjustment_get_page_size(pHAdjustment);
        int colPosPixel = GTV_CALC_HEADER_BAR(window->columnbar)->m_nPositionPixel = gtk_adjustment_get_value(pHAdjustment);

        std::stringstream aCommand;
        aCommand << ".uno:ViewRowColumnHeaders";
        aCommand << "?x=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(window->lokdocview), colPosPixel));
        aCommand << "&width=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(window->lokdocview), colSizePixel));
        aCommand << "&y=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(window->lokdocview), rowPosPixel));
        aCommand << "&height=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(window->lokdocview), rowSizePixel));
        std::stringstream ss;
        ss << "lok::Document::getCommandValues(" << aCommand.str() << ")";
        g_info("%s", ss.str().c_str());
        char* pValues = pDocument->pClass->getCommandValues(pDocument, aCommand.str().c_str());
        g_info("lok::Document::getCommandValues() returned '%s'", pValues);
        std::stringstream aStream(pValues);
        free(pValues);
        assert(!aStream.str().empty());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        gtv_calc_header_bar_configure(GTV_CALC_HEADER_BAR(window->rowbar), &aTree.get_child("rows"));
        gtv_calc_header_bar_configure(GTV_CALC_HEADER_BAR(window->columnbar), &aTree.get_child("columns"));
        gtv_calc_header_bar_configure(GTV_CALC_HEADER_BAR(window->cornerarea), nullptr);
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
