/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <cstring>

#include <gtv-helpers.hxx>

void
userPromptDialog(GtkWindow* pWindow, const std::string& aTitle, std::map<std::string, std::string>& aEntries)
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
        GList* pList = gtk_container_get_children(GTK_CONTAINER(pEntryArea));

        for (GList* l = pList; l != nullptr; l = l->next)
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

void
clipboardSetHtml(GtkClipboard* pClipboard, const char* pSelection)
{
    GtkTargetList* pList = gtk_target_list_new(nullptr, 0);
    GdkAtom aAtom(gdk_atom_intern("text/html", false));
    gtk_target_list_add(pList, aAtom, 0, 0);
    gint nTargets = 0;
    GtkTargetEntry* pTargets = gtk_target_table_new_from_list(pList, &nTargets);

    gtk_clipboard_set_with_data(pClipboard, pTargets, nTargets, htmlGetFunc, htmlClearFunc, g_strdup(pSelection));

    gtk_target_table_free(pTargets, nTargets);
    gtk_target_list_unref(pList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
