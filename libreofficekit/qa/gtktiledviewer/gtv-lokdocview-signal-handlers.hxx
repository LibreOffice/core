/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_LOKDOCVIEW_SIGNAL_HANDLERS_H
#define GTV_LOKDOCVIEW_SIGNAL_HANDLERS_H

#include <gtk/gtk.h>

void lokdocview_signalEdit(LOKDocView* pDocView, gboolean bWasEdit, gpointer);
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
