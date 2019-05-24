/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <config_cairo_canvas.h>

#include <Qt5SvpGraphics.hxx>
#include <Qt5SvpSurface.hxx>

#include <QtWidgets/QWidget>

Qt5SvpGraphics::Qt5SvpGraphics(QWidget* pQWidget)
    : SvpSalGraphics()
    , m_pQWidget(pQWidget)
{
}

Qt5SvpGraphics::~Qt5SvpGraphics() {}

void Qt5SvpGraphics::updateQWidget() const
{
    if (m_pQWidget)
        m_pQWidget->update(m_pQWidget->rect());
}

#if ENABLE_CAIRO_CANVAS

bool Qt5SvpGraphics::SupportsCairo() const { return true; }

cairo::SurfaceSharedPtr
Qt5SvpGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return cairo::SurfaceSharedPtr(new cairo::Qt5SvpSurface(rSurface));
}

cairo::SurfaceSharedPtr Qt5SvpGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int x,
                                                      int y, int width, int height) const
{
    return cairo::SurfaceSharedPtr(new cairo::Qt5SvpSurface(this, x, y, width, height));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
