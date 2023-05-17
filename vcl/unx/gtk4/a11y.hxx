/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtk/gtk.h>

//TODO: Silence various loplugin:external and lopluign:unreffun in (WIP?) a11y.cxx for now:
struct LoAccessible;
struct LoAccessibleClass;
struct OOoFixed;
struct OOoFixedClass;
static inline gpointer lo_accessible_get_instance_private(LoAccessible*);
GType lo_accessible_get_type();
static inline gpointer ooo_fixed_get_instance_private(OOoFixed*);
GtkWidget* ooo_fixed_new();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
