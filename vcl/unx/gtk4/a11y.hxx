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
#include <com/sun/star/accessibility/XAccessible.hpp>

//TODO: Silence various loplugin:external and loplugin:unreffun in (WIP?) a11y.cxx for now:
struct LoAccessibleClass;
struct OOoFixed;
struct OOoFixedClass;
GType lo_accessible_get_type();
GtkWidget* ooo_fixed_new();

struct LoAccessible
{
    GObject parent_instance;
    GdkDisplay* display;
    GtkAccessible* parent;
    GtkATContext* at_context;
    css::uno::Reference<css::accessibility::XAccessible> uno_accessible;
};

LoAccessible*
lo_accessible_new(GdkDisplay* pDisplay, GtkAccessible* pParent,
                  const css::uno::Reference<css::accessibility::XAccessible>& rAccessible);

#define LO_TYPE_ACCESSIBLE (lo_accessible_get_type())
#define LO_ACCESSIBLE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), LO_TYPE_ACCESSIBLE, LoAccessible))
// #define LO_IS_ACCESSIBLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LO_TYPE_ACCESSIBLE))

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
