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

#include <vcl/fontcharmap.hxx>
#include <sal/config.h>
#include <headless/svpgdi.hxx>
#include <font/FontMetricData.hxx>
#include <sallayout.hxx>

void SvpSalGraphics::SetFont(LogicalFontInstance* pIFSD, int nFallbackLevel)
{
    m_aTextRenderImpl.SetFont(pIFSD, nFallbackLevel);
}

void SvpSalGraphics::GetFontMetric( FontMetricDataRef& xFontMetric, int nFallbackLevel )
{
    m_aTextRenderImpl.GetFontMetric(xFontMetric, nFallbackLevel);
}

FontCharMapRef SvpSalGraphics::GetFontCharMap() const
{
    return m_aTextRenderImpl.GetFontCharMap();
}

bool SvpSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    return m_aTextRenderImpl.GetFontCapabilities(rFontCapabilities);
}

void SvpSalGraphics::GetDevFontList( vcl::font::PhysicalFontCollection* pFontCollection )
{
    m_aTextRenderImpl.GetDevFontList(pFontCollection);
}

void SvpSalGraphics::ClearDevFontCache()
{
    m_aTextRenderImpl.ClearDevFontCache();
}

bool SvpSalGraphics::AddTempDevFont( vcl::font::PhysicalFontCollection* pFontCollection,
    const OUString& rFileURL, const OUString& rFontName)
{
    return m_aTextRenderImpl.AddTempDevFont(pFontCollection, rFileURL, rFontName);
}

bool SvpSalGraphics::RemoveTempDevFont(const OUString& rFileURL, const OUString& rFontName)
{
    return m_aTextRenderImpl.RemoveTempDevFont(rFileURL, rFontName);
}

std::unique_ptr<GenericSalLayout> SvpSalGraphics::GetTextLayout(int nFallbackLevel)
{
    return m_aTextRenderImpl.GetTextLayout(nFallbackLevel);
}

void SvpSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    m_aTextRenderImpl.DrawTextLayout(rLayout, *this);
}

void SvpSalGraphics::DrawSalLayout(const GenericSalLayout& rLayout, void* pSurface, const basegfx::BColor& rTextColor, bool bAntiAliased) const
{
    CairoTextRender::ImplDrawTextLayout(
        static_cast<cairo_t*>(pSurface),
        Color(rTextColor),
        rLayout,
        nullptr,
        bAntiAliased);
}

void SvpSalGraphics::SetTextColor( Color nColor )
{
    m_aTextRenderImpl.SetTextColor(nColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
