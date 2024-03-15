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

#if GTK_CHECK_VERSION(4, 14, 0)

void lo_accessible_text_init(GtkAccessibleTextInterface* iface);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
