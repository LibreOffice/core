/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/svapp.hxx>
#include "customcellrenderer.hxx"
#if !GTK_CHECK_VERSION(4, 0, 0)
#include <gtk/gtk-a11y.h>
#endif

namespace
{
struct _CustomCellRendererClass : public GtkCellRendererTextClass
{
};

enum
{
    PROP_ID = 10000,
    PROP_INSTANCE_TREE_VIEW = 10001
};
}

G_DEFINE_TYPE(CustomCellRenderer, custom_cell_renderer, GTK_TYPE_CELL_RENDERER_TEXT)

static void custom_cell_renderer_init(CustomCellRenderer* self)
{
    {
        SolarMutexGuard aGuard;
        new (&self->device) VclPtr<VirtualDevice>;
    }

    // prevent loplugin:unreffun firing on macro generated function
    (void)custom_cell_renderer_get_instance_private(self);
}

static void custom_cell_renderer_get_property(GObject* object, guint param_id, GValue* value,
                                              GParamSpec* pspec)
{
    CustomCellRenderer* cellsurface = CUSTOM_CELL_RENDERER(object);

    switch (param_id)
    {
        case PROP_ID:
            g_value_set_string(value, cellsurface->id);
            break;
        case PROP_INSTANCE_TREE_VIEW:
            g_value_set_pointer(value, cellsurface->instance);
            break;
        default:
            G_OBJECT_CLASS(custom_cell_renderer_parent_class)
                ->get_property(object, param_id, value, pspec);
            break;
    }
}

static void custom_cell_renderer_set_property(GObject* object, guint param_id, const GValue* value,
                                              GParamSpec* pspec)
{
    CustomCellRenderer* cellsurface = CUSTOM_CELL_RENDERER(object);

    switch (param_id)
    {
        case PROP_ID:
            g_free(cellsurface->id);
            cellsurface->id = g_value_dup_string(value);
            break;
        case PROP_INSTANCE_TREE_VIEW:
            cellsurface->instance = g_value_get_pointer(value);
            break;
        default:
            G_OBJECT_CLASS(custom_cell_renderer_parent_class)
                ->set_property(object, param_id, value, pspec);
            break;
    }
}

static bool custom_cell_renderer_get_preferred_size(GtkCellRenderer* cell,
                                                    GtkOrientation orientation, gint* minimum_size,
                                                    gint* natural_size);

#if GTK_CHECK_VERSION(4, 0, 0)
static void custom_cell_renderer_snapshot(GtkCellRenderer* cell, GtkSnapshot* snapshot,
                                          GtkWidget* widget, const GdkRectangle* background_area,
                                          const GdkRectangle* cell_area,
                                          GtkCellRendererState flags);
#endif

static void custom_cell_renderer_render(GtkCellRenderer* cell, cairo_t* cr, GtkWidget* widget,
                                        const GdkRectangle* background_area,
                                        const GdkRectangle* cell_area, GtkCellRendererState flags);

static void custom_cell_renderer_finalize(GObject* object)
{
    CustomCellRenderer* cellsurface = CUSTOM_CELL_RENDERER(object);

    g_free(cellsurface->id);

    {
        SolarMutexGuard aGuard;
        cellsurface->device.disposeAndClear();
        cellsurface->device.~VclPtr<VirtualDevice>();
    }

    G_OBJECT_CLASS(custom_cell_renderer_parent_class)->finalize(object);
}

static void custom_cell_renderer_get_preferred_width(GtkCellRenderer* cell, GtkWidget* widget,
                                                     gint* minimum_size, gint* natural_size)
{
    if (!custom_cell_renderer_get_preferred_size(cell, GTK_ORIENTATION_HORIZONTAL, minimum_size,
                                                 natural_size))
    {
        // fallback to parent if we're empty
        GTK_CELL_RENDERER_CLASS(custom_cell_renderer_parent_class)
            ->get_preferred_width(cell, widget, minimum_size, natural_size);
    }
}

static void custom_cell_renderer_get_preferred_height(GtkCellRenderer* cell, GtkWidget* widget,
                                                      gint* minimum_size, gint* natural_size)
{
    if (!custom_cell_renderer_get_preferred_size(cell, GTK_ORIENTATION_VERTICAL, minimum_size,
                                                 natural_size))
    {
        // fallback to parent if we're empty
        GTK_CELL_RENDERER_CLASS(custom_cell_renderer_parent_class)
            ->get_preferred_height(cell, widget, minimum_size, natural_size);
    }
}

static void custom_cell_renderer_get_preferred_height_for_width(GtkCellRenderer* cell,
                                                                GtkWidget* widget, gint /*width*/,
                                                                gint* minimum_height,
                                                                gint* natural_height)
{
    gtk_cell_renderer_get_preferred_height(cell, widget, minimum_height, natural_height);
}

static void custom_cell_renderer_get_preferred_width_for_height(GtkCellRenderer* cell,
                                                                GtkWidget* widget, gint /*height*/,
                                                                gint* minimum_width,
                                                                gint* natural_width)
{
    gtk_cell_renderer_get_preferred_width(cell, widget, minimum_width, natural_width);
}

void custom_cell_renderer_class_init(CustomCellRendererClass* klass)
{
    GtkCellRendererClass* cell_class = GTK_CELL_RENDERER_CLASS(klass);
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    /* Hook up functions to set and get our custom cell renderer properties */
    object_class->get_property = custom_cell_renderer_get_property;
    object_class->set_property = custom_cell_renderer_set_property;

    custom_cell_renderer_parent_class = g_type_class_peek_parent(klass);
    object_class->finalize = custom_cell_renderer_finalize;

    cell_class->get_preferred_width = custom_cell_renderer_get_preferred_width;
    cell_class->get_preferred_height = custom_cell_renderer_get_preferred_height;
    cell_class->get_preferred_width_for_height
        = custom_cell_renderer_get_preferred_width_for_height;
    cell_class->get_preferred_height_for_width
        = custom_cell_renderer_get_preferred_height_for_width;

#if GTK_CHECK_VERSION(4, 0, 0)
    cell_class->snapshot = custom_cell_renderer_snapshot;
#else
    cell_class->render = custom_cell_renderer_render;
#endif

    g_object_class_install_property(
        object_class, PROP_ID,
        g_param_spec_string("id", "ID", "The ID of the custom data", nullptr, G_PARAM_READWRITE));

    g_object_class_install_property(
        object_class, PROP_INSTANCE_TREE_VIEW,
        g_param_spec_pointer("instance", "Instance", "The GtkInstanceTreeView", G_PARAM_READWRITE));

#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_cell_renderer_class_set_accessible_type(cell_class, GTK_TYPE_TEXT_CELL_ACCESSIBLE);
#endif
}

GtkCellRenderer* custom_cell_renderer_new()
{
    return GTK_CELL_RENDERER(g_object_new(CUSTOM_TYPE_CELL_RENDERER, nullptr));
}

bool custom_cell_renderer_get_preferred_size(GtkCellRenderer* cell, GtkOrientation orientation,
                                             gint* minimum_size, gint* natural_size)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_object_get_property(G_OBJECT(cell), "id", &value);

    const char* pStr = g_value_get_string(&value);

    OUString sId(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);

    value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_POINTER);
    g_object_get_property(G_OBJECT(cell), "instance", &value);

    CustomCellRenderer* cellsurface = CUSTOM_CELL_RENDERER(cell);

    Size aSize;

    gpointer pWidget = g_value_get_pointer(&value);
    if (pWidget)
    {
        SolarMutexGuard aGuard;
        custom_cell_renderer_ensure_device(cellsurface, pWidget);
        aSize = custom_cell_renderer_get_size(*cellsurface->device, sId, pWidget);
    }

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        if (minimum_size)
            *minimum_size = aSize.Width();

        if (natural_size)
            *natural_size = aSize.Width();
    }
    else
    {
        if (minimum_size)
            *minimum_size = aSize.Height();

        if (natural_size)
            *natural_size = aSize.Height();
    }

    return true;
}

void custom_cell_renderer_render(GtkCellRenderer* cell, cairo_t* cr, GtkWidget* /*widget*/,
                                 const GdkRectangle* /*background_area*/,
                                 const GdkRectangle* cell_area, GtkCellRendererState flags)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_object_get_property(G_OBJECT(cell), "id", &value);

    const char* pStr = g_value_get_string(&value);
    OUString sId(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);

    value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_POINTER);
    g_object_get_property(G_OBJECT(cell), "instance", &value);

    CustomCellRenderer* cellsurface = CUSTOM_CELL_RENDERER(cell);

    gpointer pWidget = g_value_get_pointer(&value);
    if (!pWidget)
        return;

    SolarMutexGuard aGuard;

    custom_cell_renderer_ensure_device(cellsurface, pWidget);

    Size aSize(cell_area->width, cell_area->height);
    // false to not bother setting the bg on resize as we'll do that
    // ourself via cairo
    cellsurface->device->SetOutputSizePixel(aSize, false);

    cairo_surface_t* pSurface = get_underlying_cairo_surface(*cellsurface->device);

    // fill surface as transparent so it can be blended with the potentially
    // selected background
    cairo_t* tempcr = cairo_create(pSurface);
    cairo_set_source_rgba(tempcr, 0, 0, 0, 0);
    cairo_set_operator(tempcr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(tempcr);
    cairo_destroy(tempcr);
    cairo_surface_flush(pSurface);

    custom_cell_renderer_render(*cellsurface->device, tools::Rectangle(Point(0, 0), aSize),
                                static_cast<bool>(flags & GTK_CELL_RENDERER_SELECTED), sId,
                                pWidget);

    cairo_surface_mark_dirty(pSurface);

    cairo_set_source_surface(cr, pSurface, cell_area->x, cell_area->y);
    cairo_paint(cr);
}

#if GTK_CHECK_VERSION(4, 0, 0)
static void custom_cell_renderer_snapshot(GtkCellRenderer* cell, GtkSnapshot* snapshot,
                                          GtkWidget* widget, const GdkRectangle* background_area,
                                          const GdkRectangle* cell_area, GtkCellRendererState flags)
{
    graphene_rect_t rect = GRAPHENE_RECT_INIT(
        static_cast<float>(cell_area->x), static_cast<float>(cell_area->y),
        static_cast<float>(cell_area->width), static_cast<float>(cell_area->height));
    cairo_t* cr = gtk_snapshot_append_cairo(GTK_SNAPSHOT(snapshot), &rect);
    custom_cell_renderer_render(cell, cr, widget, background_area, cell_area, flags);
    cairo_destroy(cr);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
