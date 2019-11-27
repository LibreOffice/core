/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <vcl/cairo.hxx>

class Qt5SvpGraphics;
class OutputDevice;

namespace cairo
{
class Qt5SvpSurface final : public Surface
{
    const Qt5SvpGraphics* m_pGraphics;
    cairo_t* const m_pCairoContext;
    CairoSurfaceSharedPtr m_pSurface;

public:
    /// takes over ownership of passed cairo_surface
    explicit Qt5SvpSurface(const CairoSurfaceSharedPtr& pSurface);
    /// create surface on subarea of given drawable
    explicit Qt5SvpSurface(const Qt5SvpGraphics* pGraphics, int x, int y, int width, int height);
    ~Qt5SvpSurface() override;

    // Surface interface
    CairoSharedPtr getCairo() const override;
    CairoSurfaceSharedPtr getCairoSurface() const override { return m_pSurface; }
    SurfaceSharedPtr getSimilar(int nContentType, int width, int height) const override;

    VclPtr<VirtualDevice> createVirtualDevice() const override;
    void flush() const override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
