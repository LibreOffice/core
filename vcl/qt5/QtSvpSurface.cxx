/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <headless/cairohelper.hxx>
#include <QtSvpGraphics.hxx>
#include <QtSvpSurface.hxx>

#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>

#include <utility>

namespace cairo
{
QtSvpSurface::QtSvpSurface(CairoSurfaceSharedPtr pSurface)
    : m_pGraphics(nullptr)
    , m_pCairoContext(nullptr)
    , m_pSurface(std::move(pSurface))
{
}

QtSvpSurface::QtSvpSurface(const QtSvpGraphics* pGraphics, int x, int y, int width, int height)
    : m_pGraphics(pGraphics)
    , m_pCairoContext(pGraphics->getCairoContext())
{
    cairo_surface_t* surface = cairo_get_target(m_pCairoContext);
    m_pSurface.reset(cairo_surface_create_for_rectangle(surface, x, y, width, height),
                     &cairo_surface_destroy);
}

QtSvpSurface::~QtSvpSurface()
{
    if (m_pCairoContext)
        cairo_destroy(m_pCairoContext);
}

CairoSharedPtr QtSvpSurface::getCairo() const
{
    return CairoSharedPtr(cairo_create(m_pSurface.get()), &cairo_destroy);
}

SurfaceSharedPtr QtSvpSurface::getSimilar(int cairo_content_type, int width, int height) const
{
    return std::make_shared<QtSvpSurface>(CairoSurfaceSharedPtr(
        cairo_surface_create_similar(
            m_pSurface.get(), static_cast<cairo_content_t>(cairo_content_type), width, height),
        &cairo_surface_destroy));
}

void QtSvpSurface::flush() const
{
    cairo_surface_flush(m_pSurface.get());
    if (m_pGraphics)
        m_pGraphics->updateQWidget();
}

VclPtr<VirtualDevice> QtSvpSurface::createVirtualDevice() const
{
    SystemGraphicsData aSystemGraphicsData;

    aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
    aSystemGraphicsData.pSurface = m_pSurface.get();

    return VclPtr<VirtualDevice>::Create(aSystemGraphicsData,
                                         cairohelper::get_surface_size(m_pSurface.get()),
                                         DeviceFormat::WITHOUT_ALPHA);
}

} // namespace cairo

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
