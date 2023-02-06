/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gtkcairo.hxx"

#include <utility>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>

#include <unx/gtk/gtkgdi.hxx>

namespace
{
    Size get_surface_size(cairo_surface_t *surface)
    {
        cairo_t *cr = cairo_create(surface);
        double x1, x2, y1, y2;
        cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
        cairo_destroy(cr);
        return Size(x2 - x1, y2 - y1);
    }
}

namespace cairo
{
    /**
     * Surface::Surface:     Create generic Canvas surface using given Cairo Surface
     *
     * @param pSurface Cairo Surface
     *
     * This constructor only stores data, it does no processing.
     * It is used with e.g. cairo_image_surface_create_for_data()
     *
     * Set the mpSurface as pSurface
     **/
    Gtk3Surface::Gtk3Surface(CairoSurfaceSharedPtr pSurface)
        : mpGraphics(nullptr)
        , cr(nullptr)
        , mpSurface(std::move(pSurface))
    {}

    /**
     * Surface::Surface:   Create Canvas surface from Window reference.
     * @param x horizontal location of the new surface
     * @param y vertical location of the new surface
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * Set the mpSurface to the new surface or NULL
     **/
    Gtk3Surface::Gtk3Surface(const GtkSalGraphics* pGraphics, int x, int y, int width, int height)
        : mpGraphics(pGraphics)
        , cr(pGraphics->getCairoContext())
    {
        cairo_surface_t* surface = cairo_get_target(cr);
        mpSurface.reset(
            cairo_surface_create_for_rectangle(surface, x, y, width, height),
            &cairo_surface_destroy);
    }

    Gtk3Surface::~Gtk3Surface()
    {
        if (cr)
            cairo_destroy(cr);
    }

    /**
     * Surface::getCairo:  Create Cairo (drawing object) for the Canvas surface
     *
     * @return new Cairo or NULL
     **/
    CairoSharedPtr Gtk3Surface::getCairo() const
    {
        return CairoSharedPtr( cairo_create(mpSurface.get()),
                               &cairo_destroy );
    }

    /**
     * Surface::getSimilar:  Create new similar Canvas surface
     * @param cairo_content_type format of the new surface (cairo_content_t from cairo/src/cairo.h)
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * Creates a new Canvas surface. This normally creates platform native surface, even though
     * generic function is used.
     *
     * Cairo surface from cairo_content_type (cairo_content_t)
     *
     * @return new surface or NULL
     **/
    SurfaceSharedPtr Gtk3Surface::getSimilar(int cairo_content_type, int width, int height ) const
    {
        return std::make_shared<Gtk3Surface>(
                            CairoSurfaceSharedPtr(
                                cairo_surface_create_similar( mpSurface.get(),
                                    static_cast<cairo_content_t>(cairo_content_type), width, height ),
                                &cairo_surface_destroy ));
    }

    void Gtk3Surface::flush() const
    {
        cairo_surface_flush(mpSurface.get());
        //Wonder if there is any benefit in using cairo_fill/stroke extents on
        //every canvas call and only redrawing the union of those in a
        //poor-mans-damage tracking
        if (mpGraphics)
            mpGraphics->WidgetQueueDraw();
    }

    VclPtr<VirtualDevice> Gtk3Surface::createVirtualDevice() const
    {
        SystemGraphicsData aSystemGraphicsData;

        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.pSurface = mpSurface.get();

        return VclPtr<VirtualDevice>::Create(aSystemGraphicsData,
                              get_surface_size(mpSurface.get()),
                              DeviceFormat::WITHOUT_ALPHA);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
