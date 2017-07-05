/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <gtv-application-window.hxx>
#include <gtv-helpers.hxx>

#include <map>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

void btn_clicked(GtkWidget* pWidget, gpointer)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));

    GtkToolButton* pItem = GTK_TOOL_BUTTON(pWidget);
    const gchar* label = gtk_tool_button_get_label(pItem);
    if (g_str_has_prefix(label, ".uno:"))
    {
        std::string aArguments;
        if (g_strcmp0(label, ".uno:InsertAnnotation") == 0)
        {
            std::map<std::string, std::string> aEntries;
            aEntries["Text"] = "";
            userPromptDialog(window, "Insert Comment", aEntries);

            boost::property_tree::ptree aTree;
            aTree.put(boost::property_tree::ptree::path_type(g_strconcat("Text", "/", "type", nullptr), '/'), "string");
            aTree.put(boost::property_tree::ptree::path_type(g_strconcat("Text", "/", "value", nullptr), '/'), aEntries["Text"]);

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            aArguments = aStream.str();
        }

        bool bNotify = g_strcmp0(label, ".uno:Save") == 0;
        LOKDocView* lokdocview = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));
        if (lokdocview)
            lok_doc_view_post_command(lokdocview, label, aArguments.c_str(), bNotify);
    }
}

void doCopy(GtkWidget* pButton, gpointer /*pItem*/)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    LOKDocView* pLOKDocView = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));
    char* pUsedFormat = nullptr;
    // TODO: Should check `text-selection` signal before trying to copy
    char* pSelection = lok_doc_view_copy_selection(pLOKDocView, "text/html", &pUsedFormat);
    if (!pSelection)
        return;

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(pButton), GDK_SELECTION_CLIPBOARD);
    std::string aUsedFormat(pUsedFormat);
    if (aUsedFormat == "text/plain;charset=utf-8")
        gtk_clipboard_set_text(pClipboard, pSelection, -1);
    else
        clipboardSetHtml(pClipboard, pSelection);

    free(pSelection);
    free(pUsedFormat);
}

void doPaste(GtkWidget* pButton, gpointer /*pItem*/)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    LOKDocView* pLOKDocView = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(pButton), GDK_SELECTION_CLIPBOARD);
    GdkAtom* pTargets;
    gint nTargets;
    std::map<std::string, GdkAtom> aTargets;
    if (gtk_clipboard_wait_for_targets(pClipboard, &pTargets, &nTargets))
    {
        for (gint i = 0; i < nTargets; ++i)
        {
            gchar* pName = gdk_atom_name(pTargets[i]);
            aTargets[pName] = pTargets[i];
            g_free(pName);
        }
        g_free(pTargets);
    }

    boost::optional<GdkAtom> oTarget;
    std::string aTargetName;

    std::vector<std::string> aPreferredNames =
    {
        std::string("image/png"),
        std::string("text/html")
    };
    for (const std::string& rName : aPreferredNames)
    {
        std::map<std::string, GdkAtom>::iterator it = aTargets.find(rName);
        if (it != aTargets.end())
        {
            aTargetName = it->first;
            oTarget = it->second;
            break;
        }
    }

    if (oTarget)
    {
        GtkSelectionData* pSelectionData = gtk_clipboard_wait_for_contents(pClipboard, *oTarget);
        if (!pSelectionData)
        {
            return;
        }
        gint nLength;
        const guchar* pData = gtk_selection_data_get_data_with_length(pSelectionData, &nLength);
        bool bSuccess = lok_doc_view_paste(pLOKDocView, aTargetName.c_str(), reinterpret_cast<const char*>(pData), nLength);
        gtk_selection_data_free(pSelectionData);
        if (bSuccess)
            return;
    }

    gchar* pText = gtk_clipboard_wait_for_text(pClipboard);
    if (pText)
        lok_doc_view_paste(pLOKDocView, "text/plain;charset=utf-8", pText, strlen(pText));
}

void createView(GtkWidget*, gpointer /*pItem*/)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));

    gtv_application_window_create_view_from_window(GTV_APPLICATION_WINDOW(window));
}

static void removeUnoParam(GtkWidget* pWidget, gpointer userdata)
{
    GtkWidget* pParamAreaBox = GTK_WIDGET(userdata);
    GtkWidget* pParamContainer = gtk_widget_get_parent(pWidget);

    gtk_container_remove(GTK_CONTAINER(pParamAreaBox), pParamContainer);
}

static void addMoreUnoParam(GtkWidget* /*pWidget*/, gpointer userdata)
{
    GtkWidget* pUnoParamAreaBox = GTK_WIDGET(userdata);

    GtkWidget* pParamContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pUnoParamAreaBox), pParamContainer, TRUE, TRUE, 2);

    GtkWidget* pTypeEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pTypeEntry, TRUE, TRUE, 2);
#if GTK_CHECK_VERSION(3,2,0)
    gtk_entry_set_placeholder_text(GTK_ENTRY(pTypeEntry), "Param type (Eg. boolean, string etc.)");
#endif

    GtkWidget* pNameEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pNameEntry, TRUE, TRUE, 2);
#if GTK_CHECK_VERSION(3,2,0)
    gtk_entry_set_placeholder_text(GTK_ENTRY(pNameEntry), "Param name");
#endif

    GtkWidget* pValueEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pValueEntry, TRUE, TRUE, 2);
#if GTK_CHECK_VERSION(3,2,0)
    gtk_entry_set_placeholder_text(GTK_ENTRY(pValueEntry), "Param value");
#endif

    GtkWidget* pRemoveButton = gtk_button_new_from_icon_name("list-remove-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(pRemoveButton, "clicked", G_CALLBACK(removeUnoParam), pUnoParamAreaBox);
    gtk_box_pack_start(GTK_BOX(pParamContainer), pRemoveButton, TRUE, TRUE, 2);

    gtk_widget_show_all(pUnoParamAreaBox);
}

static void iterateUnoParams(GtkWidget* pWidget, gpointer userdata)
{
    boost::property_tree::ptree *pTree = static_cast<boost::property_tree::ptree*>(userdata);

    GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pWidget));
    GList* pIt;
    guint i = 0;
    const gchar* unoParam[3];
    for (pIt = pChildren, i = 0; pIt != nullptr && i < 3; pIt = pIt->next, i++)
    {
        unoParam[i] = gtk_entry_get_text(GTK_ENTRY(pIt->data));
    }

    pTree->put(boost::property_tree::ptree::path_type(g_strconcat(unoParam[1], "/", "type", nullptr), '/'), unoParam[0]);
    pTree->put(boost::property_tree::ptree::path_type(g_strconcat(unoParam[1], "/", "value", nullptr), '/'), unoParam[2]);
}

void unoCommandDebugger(GtkWidget* pButton, gpointer /* pItem */)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    LOKDocView* pDocView = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));

    GtkWidget* pUnoCmdDialog = gtk_dialog_new_with_buttons ("Execute UNO command",
                                                            GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(pDocView))),
                                                            GTK_DIALOG_MODAL,
                                                            "Execute",
                                                            GTK_RESPONSE_OK,
                                                            nullptr);
    g_object_set(G_OBJECT(pUnoCmdDialog), "resizable", FALSE, nullptr);
    GtkWidget* pDialogMessageArea = gtk_dialog_get_content_area (GTK_DIALOG (pUnoCmdDialog));
    GtkWidget* pUnoCmdAreaBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pUnoCmdAreaBox, TRUE, TRUE, 2);

    GtkWidget* pUnoCmdLabel = gtk_label_new("Enter UNO command");
    gtk_box_pack_start(GTK_BOX(pUnoCmdAreaBox), pUnoCmdLabel, TRUE, TRUE, 2);

    GtkWidget* pUnoCmdEntry = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(pUnoCmdAreaBox), pUnoCmdEntry, TRUE, TRUE, 2);
#if GTK_CHECK_VERSION(3,2,0)
    gtk_entry_set_placeholder_text(GTK_ENTRY(pUnoCmdEntry), "UNO command (Eg. Bold, Italic etc.)");
#endif
    GtkWidget* pUnoParamAreaBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pUnoParamAreaBox, TRUE, TRUE, 2);

    GtkWidget* pAddMoreButton = gtk_button_new_with_label("Add UNO parameter");
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pAddMoreButton, TRUE, TRUE, 2);
    g_signal_connect(G_OBJECT(pAddMoreButton), "clicked", G_CALLBACK(addMoreUnoParam), pUnoParamAreaBox);

    gtk_widget_show_all(pUnoCmdDialog);

    gint res = gtk_dialog_run (GTK_DIALOG(pUnoCmdDialog));
    switch (res)
    {
    case GTK_RESPONSE_OK:
    {
        const gchar* sUnoCmd = g_strconcat(".uno:", gtk_entry_get_text(GTK_ENTRY(pUnoCmdEntry)), nullptr);

        boost::property_tree::ptree aTree;
        gtk_container_foreach(GTK_CONTAINER(pUnoParamAreaBox), iterateUnoParams, &aTree);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        std::string aArguments = aStream.str();

        g_info("Generated UNO command: %s %s", sUnoCmd, aArguments.c_str());

        lok_doc_view_post_command(pDocView, sUnoCmd, (aArguments.empty() ? nullptr : aArguments.c_str()), false);
    }
        break;
    }

    gtk_widget_destroy(pUnoCmdDialog);
}

void toggleEditing(GtkWidget* pButton, gpointer /*pItem*/)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    LOKDocView* pDocView = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));

    bool bActive = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pButton));
    if (bool(lok_doc_view_get_edit(pDocView)) != bActive)
        lok_doc_view_set_edit(pDocView, bActive);
}

static void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    LOKDocView* pDocView = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));

    const char *sName = gtk_tool_button_get_icon_name( GTK_TOOL_BUTTON(pButton) );

    float fZoom = 0;
    float fCurrentZoom = 0;

    if ( pDocView )
    {
        fCurrentZoom = lok_doc_view_get_zoom( LOK_DOC_VIEW(pDocView) );
    }

    if ( strcmp(sName, "zoom-in-symbolic") == 0)
    {
        for ( unsigned int i = 0; i < SAL_N_ELEMENTS( fZooms ); i++ )
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
        for ( unsigned int i = 0; i < SAL_N_ELEMENTS( fZooms ); i++ )
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
            GdkRectangle aVisibleArea;
            getVisibleAreaTwips(pDocView, &aVisibleArea);
            lok_doc_view_set_visible_area(LOK_DOC_VIEW(pDocView), &aVisibleArea);
        }
    }
    std::string aZoom = std::string("Zoom: ") + std::to_string(int(fZoom * 100)) + std::string("%");
    gtk_label_set_text(GTK_LABEL(rWindow.m_pZoomLabel), aZoom.c_str());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
