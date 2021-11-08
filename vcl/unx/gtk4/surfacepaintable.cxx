/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "surfacepaintable.hxx"

struct _SurfacePaintable
{
    GObject parent_instance;
    int width;
    int height;
    cairo_surface_t* surface;
};

namespace
{
struct _SurfacePaintableClass : public GObjectClass
{
};
}

static void surface_paintable_snapshot(GdkPaintable* paintable, GdkSnapshot* snapshot, double width,
                                       double height)
{
    graphene_rect_t rect
        = GRAPHENE_RECT_INIT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    SurfacePaintable* self = SURFACE_PAINTABLE(paintable);
    cairo_t* cr = gtk_snapshot_append_cairo(GTK_SNAPSHOT(snapshot), &rect);
    cairo_set_source_surface(cr, self->surface, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
}

static GdkPaintableFlags surface_paintable_get_flags(GdkPaintable* /*paintable*/)
{
    return static_cast<GdkPaintableFlags>(GDK_PAINTABLE_STATIC_SIZE
                                          | GDK_PAINTABLE_STATIC_CONTENTS);
}

static int surface_paintable_get_intrinsic_width(GdkPaintable* paintable)
{
    SurfacePaintable* self = SURFACE_PAINTABLE(paintable);
    return self->width;
}

static int surface_paintable_get_intrinsic_height(GdkPaintable* paintable)
{
    SurfacePaintable* self = SURFACE_PAINTABLE(paintable);
    return self->height;
}

static void surface_paintable_init_interface(GdkPaintableInterface* iface)
{
    iface->snapshot = surface_paintable_snapshot;
    iface->get_flags = surface_paintable_get_flags;
    iface->get_intrinsic_width = surface_paintable_get_intrinsic_width;
    iface->get_intrinsic_height = surface_paintable_get_intrinsic_height;
}

G_DEFINE_TYPE_WITH_CODE(SurfacePaintable, surface_paintable, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GDK_TYPE_PAINTABLE,
                                              surface_paintable_init_interface));

static void surface_paintable_init(SurfacePaintable* self)
{
    self->width = 0;
    self->height = 0;
    self->surface = nullptr;

    // prevent loplugin:unreffun firing on macro generated function
    (void)surface_paintable_get_instance_private(self);
}

static void surface_paintable_dispose(GObject* object)
{
    SurfacePaintable* self = SURFACE_PAINTABLE(object);
    cairo_surface_destroy(self->surface);
    G_OBJECT_CLASS(surface_paintable_parent_class)->dispose(object);
}

static void surface_paintable_class_init(SurfacePaintableClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = surface_paintable_dispose;
}

void surface_paintable_set_source(SurfacePaintable* pPaintable, cairo_surface_t* pSource,
                                  int nWidth, int nHeight)
{
    pPaintable->surface = pSource;
    pPaintable->width = nWidth;
    pPaintable->height = nHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
