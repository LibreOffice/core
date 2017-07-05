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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
