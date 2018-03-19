/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <pwd.h>

#include <cstring>

#include "gtv-helpers.hxx"

void GtvHelpers::userPromptDialog(GtkWindow* pWindow, const std::string& aTitle, std::map<std::string, std::string>& aEntries)
{
    GtkWidget* pDialog = gtk_dialog_new_with_buttons (aTitle.c_str(),
                                                      pWindow,
                                                      GTK_DIALOG_MODAL,
                                                      "Ok",
                                                      GTK_RESPONSE_OK,
                                                      nullptr);

    GtkWidget* pDialogMessageArea = gtk_dialog_get_content_area (GTK_DIALOG (pDialog));
    GtkWidget* pEntryArea = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pDialogMessageArea), pEntryArea);
    for (const auto& entry : aEntries)
    {
        GtkWidget* pEntry = gtk_entry_new();
#if GTK_CHECK_VERSION(3,2,0)
        gtk_entry_set_placeholder_text(GTK_ENTRY(pEntry), entry.first.c_str());
#endif
        gtk_container_add(GTK_CONTAINER(pEntryArea), pEntry);
    }

    gtk_widget_show_all(pDialog);

    gint res = gtk_dialog_run(GTK_DIALOG(pDialog));
    switch(res)
    {
    case GTK_RESPONSE_OK:
        GtvGtkWrapper<GList> pList(gtk_container_get_children(GTK_CONTAINER(pEntryArea)),
                                   [](GList* l)
                                   {
                                       g_list_free(l);
                                   });

        for (GList* l = pList.get(); l != nullptr; l = l->next)
        {
            const gchar* pKey = gtk_entry_get_placeholder_text(GTK_ENTRY(l->data));
            aEntries[std::string(pKey)] = std::string(gtk_entry_get_text(GTK_ENTRY(l->data)));
        }
        break;
    }

    gtk_widget_destroy(pDialog);
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

void GtvHelpers::clipboardSetHtml(GtkClipboard* pClipboard, const char* pSelection)
{
    GtvGtkWrapper<GtkTargetList> pList(gtk_target_list_new(nullptr, 0),
                                       [](GtkTargetList* pTargetList)
                                       {
                                           gtk_target_list_unref(pTargetList);
                                       });
    GdkAtom aAtom(gdk_atom_intern("text/html", false));
    gtk_target_list_add(pList.get(), aAtom, 0, 0);
    gint nTargets = 0;
    GtkTargetEntry* pTargets = gtk_target_table_new_from_list(pList.get(), &nTargets);

    gtk_clipboard_set_with_data(pClipboard, pTargets, nTargets, htmlGetFunc, htmlClearFunc, g_strdup(pSelection));

    gtk_target_table_free(pTargets, nTargets);
}

std::string GtvHelpers::getNextAuthor()
{
    static int nCounter = 0;
    struct passwd* pPasswd = getpwuid(getuid());
    return std::string(pPasswd->pw_gecos) + " #" + std::to_string(++nCounter);
}

GtkWidget* GtvHelpers::createCommentBox(const boost::property_tree::ptree& aComment)
{
    GtkWidget* pCommentVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gchar *id = g_strndup(aComment.get<std::string>("id").c_str(), 20);
    g_object_set_data_full(G_OBJECT(pCommentVBox), "id", id, g_free);

    // Set background if its a reply comment
    if (aComment.get("parent", -1) > 0)
    {
        GtkStyleContext* pStyleContext = gtk_widget_get_style_context(pCommentVBox);
        GtkCssProvider* pCssProvider = gtk_css_provider_get_default();
        gtk_style_context_add_class(pStyleContext, "commentbox");
        gtk_style_context_add_provider(pStyleContext, GTK_STYLE_PROVIDER(pCssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_css_provider_load_from_data(pCssProvider, ".commentbox {background-color: lightgreen;}", -1, nullptr);
    }

    GtkWidget* pCommentText = gtk_label_new(aComment.get<std::string>("text").c_str());
    GtkWidget* pCommentAuthor = gtk_label_new(aComment.get<std::string>("author").c_str());
    GtkWidget* pCommentDate = gtk_label_new(aComment.get<std::string>("dateTime").c_str());
    GtkWidget* pControlsHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* pEditButton = gtk_button_new_with_label("Edit");
    GtkWidget* pReplyButton = gtk_button_new_with_label("Reply");
    GtkWidget* pDeleteButton = gtk_button_new_with_label("Delete");
    g_signal_connect(G_OBJECT(pEditButton), "clicked", G_CALLBACK(editButtonClicked), pCommentVBox);
    g_signal_connect(G_OBJECT(pReplyButton), "clicked", G_CALLBACK(replyButtonClicked), pCommentVBox);
    g_signal_connect(G_OBJECT(pDeleteButton), "clicked", G_CALLBACK(deleteCommentButtonClicked), pCommentVBox);

    gtk_container_add(GTK_CONTAINER(pControlsHBox), pEditButton);
    gtk_container_add(GTK_CONTAINER(pControlsHBox), pReplyButton);
    gtk_container_add(GTK_CONTAINER(pControlsHBox), pDeleteButton);
    GtkWidget* pCommentSeparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentText);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentAuthor);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentDate);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pControlsHBox);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentSeparator);

    gtk_label_set_line_wrap(GTK_LABEL(pCommentText), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(pCommentText), 35);

    return pCommentVBox;
}

const std::string GtvHelpers::getDirPath(const std::string& filePath)
{
    int position = filePath.find_last_of('/');
    const std::string dirPath = filePath.substr(0, position + 1);
    return dirPath;
}

const std::vector<int> GtvHelpers::splitIntoIntegers(const std::string& aPayload, const std::string& aDelim, const int nItems)
{
    std::vector<int> aRet;

    if (!aPayload.empty())
    {
        gchar** ppCoordinates = g_strsplit(aPayload.c_str(), aDelim.c_str(), nItems);
        gchar** ppCoordinate  = ppCoordinates;
        while (*ppCoordinate)
        {
            aRet.push_back(atoi(*ppCoordinate));
            ++ppCoordinate;
        }
        g_strfreev(ppCoordinates);
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
