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
#include <basegfx/range/b2ibox.hxx>
#include "headless/svpgdi.hxx"
#include <config_cairo_canvas.h>

sal_uInt16 SvpSalGraphics::SetFont( FontSelectPattern* pIFSD, int nFallbackLevel )
{
    return m_xTextRenderImpl->SetFont(pIFSD, nFallbackLevel);
}

void SvpSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    m_xTextRenderImpl->GetFontMetric(pMetric, nFallbackLevel);
}

const FontCharMapPtr SvpSalGraphics::GetFontCharMap() const
{
    return m_xTextRenderImpl->GetFontCharMap();
}

bool SvpSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    return m_xTextRenderImpl->GetFontCapabilities(rFontCapabilities);
}

void SvpSalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    m_xTextRenderImpl->GetDevFontList(pFontCollection);
}

void SvpSalGraphics::ClearDevFontCache()
{
    m_xTextRenderImpl->ClearDevFontCache();
}

bool SvpSalGraphics::AddTempDevFont( PhysicalFontCollection* pFontCollection,
    const OUString& rFileURL, const OUString& rFontName)
{
    return m_xTextRenderImpl->AddTempDevFont(pFontCollection, rFileURL, rFontName);
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
    return m_xTextRenderImpl->CreateFontSubset(rToFile, pFont, pGlyphIds, pEncoding, pWidths, nGlyphCount, rInfo);
}

const Ucs2SIntMap* SvpSalGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded, std::set<sal_Unicode> const** ppPriority)
{
    return m_xTextRenderImpl->GetFontEncodingVector(pFont, pNonEncoded, ppPriority);
}

const void* SvpSalGraphics::GetEmbedFontData(
    const PhysicalFontFace* pFont,
    const sal_Ucs* pUnicodes,
    sal_Int32* pWidths,
    size_t nLen,
    FontSubsetInfo& rInfo,
    long* pDataLen)
{
    return m_xTextRenderImpl->GetEmbedFontData(pFont, pUnicodes, pWidths, nLen, rInfo, pDataLen);
}

void SvpSalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    m_xTextRenderImpl->FreeEmbedFontData(pData, nLen);
}

void SvpSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    m_xTextRenderImpl->GetGlyphWidths(pFont, bVertical, rWidths, rUnicodeEnc);
}

bool SvpSalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    return m_xTextRenderImpl->GetGlyphBoundRect(aGlyphId, rRect);
}

bool SvpSalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId, basegfx::B2DPolyPolygon& rPolyPoly )
{
    return m_xTextRenderImpl->GetGlyphOutline(aGlyphId, rPolyPoly);
}

SalLayout* SvpSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    return m_xTextRenderImpl->GetTextLayout(rArgs, nFallbackLevel);
}

void SvpSalGraphics::DrawServerFontLayout( const ServerFontLayout& rSalLayout )
{
    m_xTextRenderImpl->DrawServerFontLayout(rSalLayout );
}

void SvpSalGraphics::SetTextColor( SalColor nSalColor )
{
    m_xTextRenderImpl->SetTextColor(nSalColor);
}

#if ENABLE_CAIRO_CANVAS

SystemFontData SvpSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    return m_xTextRenderImpl->GetSysFontData(nFallbacklevel);
}

#endif // ENABLE_CAIRO_CANVAS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
