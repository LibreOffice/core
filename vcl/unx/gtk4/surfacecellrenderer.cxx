/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/svapp.hxx>
#include "surfacecellrenderer.hxx"
#include <cairo/cairo-gobject.h>

namespace
{
struct _SurfaceCellRendererClass : public GtkCellRendererClass
{
};

enum
{
    PROP_SURFACE = 10000,
};
}

G_DEFINE_TYPE(SurfaceCellRenderer, surface_cell_renderer, GTK_TYPE_CELL_RENDERER)

static void surface_cell_renderer_init(SurfaceCellRenderer* self)
{
    self->surface = nullptr;
    // prevent loplugin:unreffun firing on macro generated function
    (void)surface_cell_renderer_get_instance_private(self);
}

static void surface_cell_renderer_get_property(GObject* object, guint param_id, GValue* value,
                                               GParamSpec* pspec)
{
    SurfaceCellRenderer* cellsurface = SURFACE_CELL_RENDERER(object);

    switch (param_id)
    {
        case PROP_SURFACE:
            g_value_set_boxed(value, cellsurface->surface);
            break;
        default:
            G_OBJECT_CLASS(surface_cell_renderer_parent_class)
                ->get_property(object, param_id, value, pspec);
            break;
    }
}

static void surface_cell_renderer_set_property(GObject* object, guint param_id, const GValue* value,
                                               GParamSpec* pspec)
{
    SurfaceCellRenderer* cellsurface = SURFACE_CELL_RENDERER(object);

    switch (param_id)
    {
        case PROP_SURFACE:
            if (cellsurface->surface)
                cairo_surface_destroy(cellsurface->surface);
            cellsurface->surface = static_cast<cairo_surface_t*>(g_value_get_boxed(value));
            if (cellsurface->surface)
                cairo_surface_reference(cellsurface->surface);
            break;
        default:
            G_OBJECT_CLASS(surface_cell_renderer_parent_class)
                ->set_property(object, param_id, value, pspec);
            break;
    }
}

static bool surface_cell_renderer_get_preferred_size(GtkCellRenderer* cell,
                                                     GtkOrientation orientation, gint* minimum_size,
                                                     gint* natural_size);

static void surface_cell_renderer_snapshot(GtkCellRenderer* cell, GtkSnapshot* snapshot,
                                           GtkWidget* widget, const GdkRectangle* background_area,
                                           const GdkRectangle* cell_area,
                                           GtkCellRendererState flags);

static void surface_cell_renderer_render(GtkCellRenderer* cell, cairo_t* cr, GtkWidget* widget,
                                         const GdkRectangle* background_area,
                                         const GdkRectangle* cell_area, GtkCellRendererState flags);

static void surface_cell_renderer_finalize(GObject* object)
{
    SurfaceCellRenderer* cellsurface = SURFACE_CELL_RENDERER(object);

    if (cellsurface->surface)
        cairo_surface_destroy(cellsurface->surface);

    G_OBJECT_CLASS(surface_cell_renderer_parent_class)->finalize(object);
}

static void surface_cell_renderer_get_preferred_width(GtkCellRenderer* cell, GtkWidget* widget,
                                                      gint* minimum_size, gint* natural_size)
{
    if (!surface_cell_renderer_get_preferred_size(cell, GTK_ORIENTATION_HORIZONTAL, minimum_size,
                                                  natural_size))
    {
        // fallback to parent if we're empty
        GTK_CELL_RENDERER_CLASS(surface_cell_renderer_parent_class)
            ->get_preferred_width(cell, widget, minimum_size, natural_size);
    }
}

static void surface_cell_renderer_get_preferred_height(GtkCellRenderer* cell, GtkWidget* widget,
                                                       gint* minimum_size, gint* natural_size)
{
    if (!surface_cell_renderer_get_preferred_size(cell, GTK_ORIENTATION_VERTICAL, minimum_size,
                                                  natural_size))
    {
        // fallback to parent if we're empty
        GTK_CELL_RENDERER_CLASS(surface_cell_renderer_parent_class)
            ->get_preferred_height(cell, widget, minimum_size, natural_size);
    }
}

static void surface_cell_renderer_get_preferred_height_for_width(GtkCellRenderer* cell,
                                                                 GtkWidget* widget, gint /*width*/,
                                                                 gint* minimum_height,
                                                                 gint* natural_height)
{
    gtk_cell_renderer_get_preferred_height(cell, widget, minimum_height, natural_height);
}

static void surface_cell_renderer_get_preferred_width_for_height(GtkCellRenderer* cell,
                                                                 GtkWidget* widget, gint /*height*/,
                                                                 gint* minimum_width,
                                                                 gint* natural_width)
{
    gtk_cell_renderer_get_preferred_width(cell, widget, minimum_width, natural_width);
}

void surface_cell_renderer_class_init(SurfaceCellRendererClass* klass)
{
    GtkCellRendererClass* cell_class = GTK_CELL_RENDERER_CLASS(klass);
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    /* Hook up functions to set and get our custom cell renderer properties */
    object_class->get_property = surface_cell_renderer_get_property;
    object_class->set_property = surface_cell_renderer_set_property;

    surface_cell_renderer_parent_class = g_type_class_peek_parent(klass);
    object_class->finalize = surface_cell_renderer_finalize;

    cell_class->get_preferred_width = surface_cell_renderer_get_preferred_width;
    cell_class->get_preferred_height = surface_cell_renderer_get_preferred_height;
    cell_class->get_preferred_width_for_height
        = surface_cell_renderer_get_preferred_width_for_height;
    cell_class->get_preferred_height_for_width
        = surface_cell_renderer_get_preferred_height_for_width;

    cell_class->snapshot = surface_cell_renderer_snapshot;

    g_object_class_install_property(
        object_class, PROP_SURFACE,
        g_param_spec_boxed("surface", "Surface", "The cairo surface to render",
                           CAIRO_GOBJECT_TYPE_SURFACE, G_PARAM_READWRITE));
}

GtkCellRenderer* surface_cell_renderer_new()
{
    return GTK_CELL_RENDERER(g_object_new(SURFACE_TYPE_CELL_RENDERER, nullptr));
}

static void get_surface_size(cairo_surface_t* pSurface, int& rWidth, int& rHeight)
{
    double x1, x2, y1, y2;
    cairo_t* cr = cairo_create(pSurface);
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
    cairo_destroy(cr);

    rWidth = x2 - x1;
    rHeight = y2 - y1;
}

bool surface_cell_renderer_get_preferred_size(GtkCellRenderer* cell, GtkOrientation orientation,
                                              gint* minimum_size, gint* natural_size)
{
    SurfaceCellRenderer* cellsurface = SURFACE_CELL_RENDERER(cell);

    int nWidth = 0;
    int nHeight = 0;

    if (cellsurface->surface)
        get_surface_size(cellsurface->surface, nWidth, nHeight);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        if (minimum_size)
            *minimum_size = nWidth;

        if (natural_size)
            *natural_size = nWidth;
    }
    else
    {
        if (minimum_size)
            *minimum_size = nHeight;

        if (natural_size)
            *natural_size = nHeight;
    }

    return true;
}

void surface_cell_renderer_render(GtkCellRenderer* cell, cairo_t* cr, GtkWidget* /*widget*/,
                                  const GdkRectangle* /*background_area*/,
                                  const GdkRectangle* cell_area, GtkCellRendererState /*flags*/)
{
    SurfaceCellRenderer* cellsurface = SURFACE_CELL_RENDERER(cell);
    if (!cellsurface->surface)
        return;

    int nWidth, nHeight;
    get_surface_size(cellsurface->surface, nWidth, nHeight);
    int nXOffset = (cell_area->width - nWidth) / 2;
    int nYOffset = (cell_area->height - nHeight) / 2;

    cairo_set_source_surface(cr, cellsurface->surface, cell_area->x + nXOffset,
                             cell_area->y + nYOffset);
    cairo_paint(cr);
}

static void surface_cell_renderer_snapshot(GtkCellRenderer* cell, GtkSnapshot* snapshot,
                                           GtkWidget* widget, const GdkRectangle* background_area,
                                           const GdkRectangle* cell_area,
                                           GtkCellRendererState flags)
{
    graphene_rect_t rect = GRAPHENE_RECT_INIT(
        static_cast<float>(cell_area->x), static_cast<float>(cell_area->y),
        static_cast<float>(cell_area->width), static_cast<float>(cell_area->height));
    cairo_t* cr = gtk_snapshot_append_cairo(GTK_SNAPSHOT(snapshot), &rect);
    surface_cell_renderer_render(cell, cr, widget, background_area, cell_area, flags);
    cairo_destroy(cr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
