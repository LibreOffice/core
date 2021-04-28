/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef IOS

#include <headless/svpbmp.hxx>
#include <headless/svpinst.hxx>
#include <headless/svpvd.hxx>
#include <headless/svpgdi.hxx>

#include <basegfx/vector/b2ivector.hxx>
#include <comphelper/lok.hxx>

#include <cairo.h>

using namespace basegfx;

SvpSalVirtualDevice::SvpSalVirtualDevice(cairo_surface_t* pRefSurface, cairo_surface_t* pPreExistingTarget)
    : m_pRefSurface(pRefSurface)
    , m_pSurface(pPreExistingTarget)
    , m_bOwnsSurface(!pPreExistingTarget)
{
    cairo_surface_reference(m_pRefSurface);
}

SvpSalVirtualDevice::~SvpSalVirtualDevice()
{
    if (m_bOwnsSurface)
        cairo_surface_destroy(m_pSurface);
    cairo_surface_destroy(m_pRefSurface);
}

SvpSalGraphics* SvpSalVirtualDevice::AddGraphics(SvpSalGraphics* pGraphics)
{
    pGraphics->setSurface(m_pSurface, m_aFrameSize);
    m_aGraphics.push_back(pGraphics);
    return pGraphics;
}

SalGraphics* SvpSalVirtualDevice::AcquireGraphics()
{
    return AddGraphics(new SvpSalGraphics());
}

void SvpSalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
    m_aGraphics.erase(std::remove(m_aGraphics.begin(), m_aGraphics.end(), dynamic_cast<SvpSalGraphics*>(pGraphics)), m_aGraphics.end());
    delete pGraphics;
}

bool SvpSalVirtualDevice::SetSize( tools::Long nNewDX, tools::Long nNewDY )
{
    return SetSizeUsingBuffer(nNewDX, nNewDY, nullptr);
}

void SvpSalVirtualDevice::CreateSurface(tools::Long nNewDX, tools::Long nNewDY, sal_uInt8 *const pBuffer)
{
    if (m_pSurface)
    {
        cairo_surface_destroy(m_pSurface);
    }

    if (pBuffer)
    {
        double fXScale, fYScale;
        if (comphelper::LibreOfficeKit::isActive())
        {
            // Force scaling of the painting
            fXScale = fYScale = comphelper::LibreOfficeKit::getDPIScale();
        }
        else
        {
            dl_cairo_surface_get_device_scale(m_pRefSurface, &fXScale, &fYScale);
            nNewDX *= fXScale;
            nNewDY *= fYScale;
        }

        m_pSurface = cairo_image_surface_create_for_data(pBuffer, CAIRO_FORMAT_ARGB32,
                            nNewDX, nNewDY, cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, nNewDX));

        dl_cairo_surface_set_device_scale(m_pSurface, fXScale, fYScale);
    }
    else
    {
        m_pSurface = cairo_surface_create_similar(m_pRefSurface, CAIRO_CONTENT_COLOR_ALPHA, nNewDX, nNewDY);
    }
}

bool SvpSalVirtualDevice::SetSizeUsingBuffer( tools::Long nNewDX, tools::Long nNewDY,
        sal_uInt8 *const pBuffer)
{
    if (nNewDX == 0)
        nNewDX = 1;
    if (nNewDY == 0)
        nNewDY = 1;

    if (!m_pSurface || m_aFrameSize.getX() != nNewDX ||
                       m_aFrameSize.getY() != nNewDY)
    {
        m_aFrameSize = basegfx::B2IVector(nNewDX, nNewDY);

        if (m_bOwnsSurface)
            CreateSurface(nNewDX, nNewDY, pBuffer);

        assert(m_pSurface);

        // update device in existing graphics
        for (auto const& graphic : m_aGraphics)
            graphic->setSurface(m_pSurface, m_aFrameSize);
    }
    return true;
}

tools::Long SvpSalVirtualDevice::GetWidth() const
{
    return m_pSurface ? m_aFrameSize.getX() : 0;
}

tools::Long SvpSalVirtualDevice::GetHeight() const
{
    return m_pSurface ? m_aFrameSize.getY() : 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
