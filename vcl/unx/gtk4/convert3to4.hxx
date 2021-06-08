/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <gtk/gtk.h>

// convert gtk3 .ui to gtk4 and load it
void builder_add_from_gtk3_file(GtkBuilder* pBuilder, const OUString& rUri);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
