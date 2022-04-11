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
#include <cairo.h>

G_BEGIN_DECLS

struct _SurfaceCellRenderer
{
    GtkCellRenderer parent;
    cairo_surface_t* surface;
};

/*
   Provide a mechanism to support rendering a cairo surface in a GtkComboBox
*/

G_DECLARE_FINAL_TYPE(SurfaceCellRenderer, surface_cell_renderer, SURFACE, CELL_RENDERER,
                     GtkCellRenderer)

#define SURFACE_TYPE_CELL_RENDERER (surface_cell_renderer_get_type())

#define SURFACE_CELL_RENDERER(obj)                                                                 \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), SURFACE_TYPE_CELL_RENDERER, SurfaceCellRenderer))

#define SURFACE_IS_CELL_RENDERER(obj)                                                              \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), SURFACE_TYPE_CELL_RENDERER))

GtkCellRenderer* surface_cell_renderer_new();

G_END_DECLS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
