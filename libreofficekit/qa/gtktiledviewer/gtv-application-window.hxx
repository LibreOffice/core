/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_APPLICATION_WINDOW_H
#define GTV_APPLICATION_WINDOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTV_APPLICATION_WINDOW_TYPE (gtv_application_window_get_type())
G_DECLARE_FINAL_TYPE(GtvApplicationWindow, gtv_application_window, GTV, APPLICATION_WINDOW, GtkApplicationWindow);

GtvApplicationWindow* gtv_application_window_new(GtkApplication* application);

G_END_DECLS

#endif /* GTV_APPLICATION_WINDOW_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
