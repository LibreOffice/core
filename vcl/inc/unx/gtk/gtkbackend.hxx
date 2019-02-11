/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKBACKEND_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKBACKEND_HXX

#include <gtk/gtk.h>
#if defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
bool DLSYM_GDK_IS_X11_DISPLAY(GdkDisplay* pDisplay);
#endif
#if defined(GDK_WINDOWING_WAYLAND)
#include <gdk/gdkwayland.h>
bool DLSYM_GDK_IS_WAYLAND_DISPLAY(GdkDisplay* pDisplay);
#endif

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKBACKEND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
