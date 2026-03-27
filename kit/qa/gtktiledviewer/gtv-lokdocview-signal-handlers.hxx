/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_LOKDOCVIEW_SIGNAL_HANDLERS_H
#define GTV_LOKDOCVIEW_SIGNAL_HANDLERS_H

#include <gtk/gtk.h>
#include <COKit/COKitGtk.h>

namespace KitDocumentViewSigHandlers {
    void editChanged(KitDocumentView* pDocView, gboolean bWasEdit, gpointer);
    void commandChanged(KitDocumentView* pDocView, char* pPayload, gpointer);
    void commandResult(KitDocumentView*, char*, gpointer);
    void searchNotFound(KitDocumentView*, char*, gpointer);
    void searchResultCount(KitDocumentView*, char*, gpointer);
    void partChanged(KitDocumentView*, int, gpointer);
    void hyperlinkClicked(KitDocumentView*, char*, gpointer);
    void cursorChanged(KitDocumentView* pDocView, gint nX, gint nY, gint nWidth, gint nHeight, gpointer);
    void addressChanged(KitDocumentView* pDocView, char* pPayload, gpointer);
    void formulaChanged(KitDocumentView* pDocView, char* pPayload, gpointer);
    void passwordRequired(KitDocumentView* pDocView, char* pUrl, gboolean bModify, gpointer);
    void comment(KitDocumentView* pDocView, gchar* pComment, gpointer);
    void window(KitDocumentView* pDocView, gchar* pPayload, gpointer);
    void contentControl(KitDocumentView* pDocView, gchar* pComment, gpointer);

    gboolean configureEvent(GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer pData);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
