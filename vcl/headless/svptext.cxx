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

#include <sal/types.h>
#include <unotools/configmgr.hxx>
#include <vcl/fontcharmap.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <headless/svpgdi.hxx>
#include <config_cairo_canvas.h>
#include <impfontmetricdata.hxx>
#include <sallayout.hxx>

void SvpSalGraphics::SetFont(LogicalFontInstance* pIFSD, int nFallbackLevel)
{
    m_aTextRenderImpl.SetFont(pIFSD, nFallbackLevel);
}

void SvpSalGraphics::GetFontMetric( ImplFontMetricDataRef& xFontMetric, int nFallbackLevel )
{
    m_aTextRenderImpl.GetFontMetric(xFontMetric, nFallbackLevel);
}

const FontCharMapRef SvpSalGraphics::GetFontCharMap() const
{
    return m_aTextRenderImpl.GetFontCharMap();
}

bool SvpSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    return m_aTextRenderImpl.GetFontCapabilities(rFontCapabilities);
}

void SvpSalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    m_aTextRenderImpl.GetDevFontList(pFontCollection);
}

void SvpSalGraphics::ClearDevFontCache()
{
    m_aTextRenderImpl.ClearDevFontCache();
}

bool SvpSalGraphics::AddTempDevFont( PhysicalFontCollection* pFontCollection,
    const OUString& rFileURL, const OUString& rFontName)
{
    return m_aTextRenderImpl.AddTempDevFont(pFontCollection, rFileURL, rFontName);
}

bool SvpSalGraphics::CreateFontSubset(
    const OUString& rToFile,
    const PhysicalFontFace* pFont,
    const sal_GlyphId* pGlyphIds,
    const sal_uInt8* pEncoding,
    sal_Int32* pWidths,
    int nGlyphCount,
    FontSubsetInfo& rInfo)
{
    return m_aTextRenderImpl.CreateFontSubset(rToFile, pFont, pGlyphIds, pEncoding, pWidths, nGlyphCount, rInfo);
}

const void* SvpSalGraphics::GetEmbedFontData(const PhysicalFontFace* pFont, long* pDataLen)
{
    return m_aTextRenderImpl.GetEmbedFontData(pFont, pDataLen);
}

void SvpSalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    m_aTextRenderImpl.FreeEmbedFontData(pData, nLen);
}

void SvpSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   std::vector< sal_Int32 >& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    m_aTextRenderImpl.GetGlyphWidths(pFont, bVertical, rWidths, rUnicodeEnc);
}

std::unique_ptr<GenericSalLayout> SvpSalGraphics::GetTextLayout(int nFallbackLevel)
{
    if (utl::ConfigManager::IsFuzzing())
        return nullptr;
    return m_aTextRenderImpl.GetTextLayout(nFallbackLevel);
}

void SvpSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    m_aTextRenderImpl.DrawTextLayout(rLayout, *this);
}

void SvpSalGraphics::SetTextColor( Color nColor )
{
    m_aTextRenderImpl.SetTextColor(nColor);
}

#if ENABLE_CAIRO_CANVAS

SystemFontData SvpSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    return m_aTextRenderImpl.GetSysFontData(nFallbacklevel);
}

#endif // ENABLE_CAIRO_CANVAS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
