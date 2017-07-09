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
#include <gtv-signal-handlers.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

void lokdocview_signalEdit(LOKDocView* pDocView, gboolean bWasEdit, gpointer)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    gboolean bEdit = lok_doc_view_get_edit(pDocView);
    g_info("signalEdit: %d -> %d", bWasEdit, bEdit);

    // Let the main toolbar know, so that it can enable disable the button
    GtvMainToolbar* pMainToolbar = gtv_application_window_get_main_toolbar(GTV_APPLICATION_WINDOW(window));
    gtv_main_toolbar_set_edit(pMainToolbar, bEdit);
}

void lokdocview_signalCommand(LOKDocView* pDocView, char* pPayload, gpointer)
{
    GApplication* app = g_application_get_default();
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(app)));

    std::string aPayload(pPayload);
    size_t nPosition = aPayload.find('=');
    if (nPosition != std::string::npos)
    {
        const std::string aKey = aPayload.substr(0, nPosition);
        const std::string aValue = aPayload.substr(nPosition + 1);
        GtkToolItem* pItem = gtv_application_window_find_tool_by_unocommand(window, aKey);
        if (pItem != nullptr)
        {
            if (aValue == "true" || aValue == "false") {
                gboolean bEdit = aValue == "true";
                if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pItem)) != bEdit)
                {
                    // Avoid invoking lok_doc_view_post_command().
                    // FIXME: maybe block/unblock the signal (see
                    // g_signal_handlers_block_by_func) ?
                    gtv_application_window_set_toolbar_broadcast(window, false);
                    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pItem), bEdit);
                    gtv_application_window_set_toolbar_broadcast(window, true);

                }
            } else if (aValue == "enabled" || aValue == "disabled") {
                gboolean bSensitive = aValue == "enabled";
                gtk_widget_set_sensitive(GTK_WIDGET(pItem), bSensitive);

                // Remember state, so in case edit is disable and enabled
                // later, the correct sensitivity can be restored.
                GtvMainToolbar* pMainToolbar = gtv_application_window_get_main_toolbar(window);
                gtv_main_toolbar_set_sensitive_internal(pMainToolbar, pItem, bSensitive);
            }
        }
        else if (aKey == ".uno:TrackedChangeIndex")
        {
            std::string aText = std::string("Current redline: ");
            if (aValue.empty())
                aText += "none";
            else
                aText += aValue;
            gtv_application_window_set_redline_label(window, aText);
        }
    }
}

void lokdocview_signalCommandResult(LOKDocView*, char*, gpointer);
void lokdocview_signalSearch(LOKDocView*, char*, gpointer);
void lokdocview_signalSearchResultCount(LOKDocView*, char*, gpointer);
void lokdocview_signalPart(LOKDocView*, int, gpointer);
void lokdocview_signalHyperlink(LOKDocView*, char*, gpointer);
void lokdocview_cursorChanged(LOKDocView* pDocView, gint nX, gint nY, gint nWidth, gint nHeight, gpointer);
void lokdocview_addressChanged(LOKDocView* pDocView, char* pPayload, gpointer);
void lokdocview_formulaChanged(LOKDocView* pDocView, char* pPayload, gpointer);
void lokdocview_passwordRequired(LOKDocView* pDocView, char* pUrl, gboolean bModify, gpointer);
void lokdocview_commentCallback(LOKDocView* pDocView, gchar* pComment, gpointer);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
