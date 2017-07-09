/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_MAIN_TOOLBAR_H
#define GTV_MAIN_TOOLBAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTV_MAIN_TOOLBAR_TYPE (gtv_main_toolbar_get_type())
G_DECLARE_FINAL_TYPE(GtvMainToolbar, gtv_main_toolbar, GTV, MAIN_TOOLBAR, GtkBox);

GtkWidget* gtv_main_toolbar_new();

GtkContainer* gtv_main_toolbar_get_first_toolbar(GtvMainToolbar* toolbar);

GtkContainer* gtv_main_toolbar_get_second_toolbar(GtvMainToolbar* toolbar);

void gtv_main_toolbar_set_sensitive_internal(GtvMainToolbar* toolbar, GtkToolItem* pItem, bool isSensitive);

/// Use internal sensitivity map to set actual widget's sensitivness
void gtv_main_toolbar_set_edit(GtvMainToolbar* toolbar, gboolean bEdit);

G_END_DECLS

#endif /* GTV_MAIN_TOOLBAR_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
