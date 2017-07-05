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

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

/// Set sensitivity based on rWindow.m_aToolItemSensitivities, taking edit
/// state into account.
static void setSensitiveIfInEdit(GtkToolItem* pItem, gboolean bEdit, TiledWindow& rWindow)
{
    gtk_widget_set_sensitive(GTK_WIDGET(pItem), bEdit && rWindow.m_aToolItemSensitivities[pItem]);
}

void lokdocview_signalEdit(LOKDocView* pDocView, gboolean bWasEdit, gpointer)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));
    gboolean bEdit = lok_doc_view_get_edit(pDocView);
    g_info("signalEdit: %d -> %d", bWasEdit, bEdit);
}

void lokdocview_signalCommand(LOKDocView* pDocView, char* pPayload, gpointer);
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
