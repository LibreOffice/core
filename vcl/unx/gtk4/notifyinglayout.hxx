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
#include <tools/link.hxx>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(NotifyingLayout, notifying_layout, NOTIFYING, LAYOUT, GtkLayoutManager)

/*
   Replace the existing GtkLayoutManager of pWidget with pLayout instead which will
   forward all requests to the original GtkLayoutManager but additionally call
   rLink when a size is allocated to the pWidget.

   This provides a workaround for the removal of the size-allocate signal in gtk4
*/
void notifying_layout_start_watch(NotifyingLayout* pLayout, GtkWidget* pWidget,
                                  const Link<void*, void>& rLink);

/*
   Undo a previous notifying_layout_start_watch.
*/
void notifying_layout_stop_watch(NotifyingLayout* pLayout);

G_END_DECLS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
