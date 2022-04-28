/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtSvpVirtualDevice.hxx>
#include <QtSvpGraphics.hxx>

QtSvpVirtualDevice::QtSvpVirtualDevice(QtSvpGraphics& rRefGraphics,
                                       cairo_surface_t* pPreExistingTarget)
    : SvpSalVirtualDevice(rRefGraphics.getSurface(), pPreExistingTarget)
    , m_nScalePercentage(rRefGraphics.GetDPIScalePercentage())
{
    // can't store rRefGraphics, as it's released
    SAL_DEBUG(__func__ << " " << rRefGraphics.frame() << " " << m_nScalePercentage);
}

SalGraphics* QtSvpVirtualDevice::AcquireGraphics()
{
    return AddGraphics(new QtSvpGraphics(nullptr, m_nScalePercentage));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
