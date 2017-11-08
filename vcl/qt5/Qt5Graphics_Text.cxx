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

#include "Qt5Graphics.hxx"
#include "Qt5FontFace.hxx"
#include <qt5/Qt5Font.hxx>

#include <vcl/fontcharmap.hxx>

#include <CommonSalLayout.hxx>
#include <PhysicalFontCollection.hxx>

#include <QtGui/QFontDatabase>
#include <QtGui/QRawFont>
#include <QtCore/QStringList>

void Qt5Graphics::SetTextColor(SalColor nSalColor) { m_aTextColor = nSalColor; }

void Qt5Graphics::SetFont(const FontSelectPattern* pReqFont, int nFallbackLevel)
{
    // release the text styles
    for (int i = nFallbackLevel; i < MAX_FALLBACK; ++i)
    {
        if (!m_pTextStyle[i])
            break;
        delete m_pTextStyle[i];
        m_pTextStyle[i] = nullptr;
    }

    if (!pReqFont)
        // handle release-font-resources request
        m_pFontData[nFallbackLevel] = nullptr;
    else
    {
        m_pFontData[nFallbackLevel] = static_cast<const Qt5FontFace*>(pReqFont->mpFontData);
        m_pTextStyle[nFallbackLevel] = new Qt5Font(*pReqFont);
    }
}

void Qt5Graphics::GetFontMetric(ImplFontMetricDataRef& rFMD, int nFallbackLevel)
{
    QRawFont aRawFont(QRawFont::fromFont(*m_pTextStyle[nFallbackLevel]));

    QByteArray aHheaTable = aRawFont.fontTable("hhea");
    std::vector<uint8_t> rHhea(aHheaTable.data(), aHheaTable.data() + aHheaTable.size());

    QByteArray aOs2Table = aRawFont.fontTable("OS/2");
    std::vector<uint8_t> rOS2(aHheaTable.data(), aHheaTable.data() + aHheaTable.size());

    rFMD->ImplCalcLineSpacing(rHhea, rOS2, aRawFont.unitsPerEm());

    rFMD->SetWidth(aRawFont.averageCharWidth());

    const QChar nKashidaCh[2] = { 0x06, 0x40 };
    quint32 nKashidaGid = 0;
    QPointF aPoint;
    int nNumGlyphs;
    if (aRawFont.glyphIndexesForChars(nKashidaCh, 1, &nKashidaGid, &nNumGlyphs)
        && aRawFont.advancesForGlyphIndexes(&nKashidaGid, &aPoint, 1))
        rFMD->SetMinKashida(lrint(aPoint.rx()));
}

const FontCharMapRef Qt5Graphics::GetFontCharMap() const
{
    if (!m_pFontData[0])
        return FontCharMapRef(new FontCharMap());
    return m_pFontData[0]->GetFontCharMap();
}

bool Qt5Graphics::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    if (!m_pFontData[0])
        return false;
    return m_pFontData[0]->GetFontCapabilities(rFontCapabilities);
}

void Qt5Graphics::GetDevFontList(PhysicalFontCollection* pPFC)
{
    m_pFontCollection = pPFC;
    if (pPFC->Count())
        return;

    QFontDatabase aFDB;
    for (auto& family : aFDB.families())
        for (auto& style : aFDB.styles(family))
        {
            // Just get any size - we don't care
            QList<int> sizes = aFDB.smoothSizes(family, style);
            pPFC->Add(Qt5FontFace::fromQFont(aFDB.font(family, style, *sizes.begin())));
        }
}

void Qt5Graphics::ClearDevFontCache() {}

bool Qt5Graphics::AddTempDevFont(PhysicalFontCollection*, const OUString& rFileURL,
                                 const OUString& rFontName)
{
    return false;
}

bool Qt5Graphics::CreateFontSubset(const OUString& rToFile, const PhysicalFontFace* pFont,
                                   const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                   sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo)
{
    return false;
}

const void* Qt5Graphics::GetEmbedFontData(const PhysicalFontFace*, long* pDataLen)
{
    return nullptr;
}

void Qt5Graphics::FreeEmbedFontData(const void* pData, long nDataLen) {}

void Qt5Graphics::GetGlyphWidths(const PhysicalFontFace* pPFF, bool bVertical,
                                 std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc)
{
}

bool Qt5Graphics::GetGlyphBoundRect(const GlyphItem&, tools::Rectangle&) { return false; }

bool Qt5Graphics::GetGlyphOutline(const GlyphItem&, basegfx::B2DPolyPolygon&) { return false; }

SalLayout* Qt5Graphics::GetTextLayout(ImplLayoutArgs&, int nFallbackLevel)
{
    if (m_pTextStyle[nFallbackLevel])
        return new CommonSalLayout(*m_pTextStyle[nFallbackLevel]);
    return nullptr;
}

void Qt5Graphics::DrawTextLayout(const CommonSalLayout&) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
