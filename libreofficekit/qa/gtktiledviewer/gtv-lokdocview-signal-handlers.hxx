/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtk/gtk.h>

namespace LOKDocViewSigHandlers {
    void editChanged(LOKDocView* pDocView, gboolean bWasEdit, gpointer);
    void commandChanged(LOKDocView* pDocView, char* pPayload, gpointer);
    void commandResult(LOKDocView*, char*, gpointer);
    void searchNotFound(LOKDocView*, char*, gpointer);
    void searchResultCount(LOKDocView*, char*, gpointer);
    void partChanged(LOKDocView*, int, gpointer);
    void hyperlinkClicked(LOKDocView*, char*, gpointer);
    void cursorChanged(LOKDocView* pDocView, gint nX, gint nY, gint nWidth, gint nHeight, gpointer);
    void addressChanged(LOKDocView* pDocView, char* pPayload, gpointer);
    void formulaChanged(LOKDocView* pDocView, char* pPayload, gpointer);
    void passwordRequired(LOKDocView* pDocView, char* pUrl, gboolean bModify, gpointer);
    void comment(LOKDocView* pDocView, gchar* pComment, gpointer);
    void window(LOKDocView* pDocView, gchar* pPayload, gpointer);

    gboolean configureEvent(GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer pData);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
