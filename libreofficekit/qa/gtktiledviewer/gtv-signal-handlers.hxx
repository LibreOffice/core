/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_SIGNAL_HANDLERS_H
#define GTV_SIGNAL_HANDLERS_H

#include <gtk/gtk.h>

void btn_clicked(GtkWidget* pWidget, gpointer);

void doCopy(GtkWidget* pButton, gpointer /*pItem*/);

void doPaste(GtkWidget* pButton, gpointer /*pItem*/);

void createView(GtkWidget* pButton, gpointer /*pItem*/);

void unoCommandDebugger(GtkWidget* pButton, gpointer /* pItem */);

void toggleEditing(GtkWidget* pButton, gpointer /*pItem*/);

void changePartMode( GtkWidget* pSelector, gpointer /* pItem */ );

void changePart( GtkWidget* pSelector, gpointer /*pItem*/ );

void changeZoom( GtkWidget* pButton, gpointer /* pItem */ );

void toggleFindbar(GtkWidget* pButton, gpointer /*pItem*/);

void documentRedline(GtkWidget* pButton, gpointer /*pItem*/);

void documentRepair(GtkWidget* pButton, gpointer /*pItem*/);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
