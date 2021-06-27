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
#include <vcl/virdev.hxx>

G_BEGIN_DECLS

struct _CustomCellRenderer
{
    GtkCellRendererText parent;
    VclPtr<VirtualDevice> device;
    gchar* id;
    gpointer instance;
};

/*
   Provide a mechanism to support custom rendering of cells in a GtkTreeView/GtkComboBox
*/

G_DECLARE_FINAL_TYPE(CustomCellRenderer, custom_cell_renderer, CUSTOM, CELL_RENDERER,
                     GtkCellRendererText)

#define CUSTOM_TYPE_CELL_RENDERER (custom_cell_renderer_get_type())

#define CUSTOM_CELL_RENDERER(obj)                                                                  \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CUSTOM_TYPE_CELL_RENDERER, CustomCellRenderer))

#define CUSTOM_IS_CELL_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CUSTOM_TYPE_CELL_RENDERER))

GtkCellRenderer* custom_cell_renderer_new();

G_END_DECLS

void custom_cell_renderer_ensure_device(CustomCellRenderer* cellsurface, gpointer user_data);
Size custom_cell_renderer_get_size(VirtualDevice& rDevice, const OUString& rCellId,
                                   gpointer user_data);
void custom_cell_renderer_render(VirtualDevice& rDevice, const tools::Rectangle& rRect,
                                 bool bSelected, const OUString& rId, gpointer user_data);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
