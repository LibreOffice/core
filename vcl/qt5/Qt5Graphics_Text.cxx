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

#include <Qt5Graphics.hxx>
#include <Qt5FontFace.hxx>
#include <Qt5Font.hxx>
#include <Qt5Painter.hxx>

#include <vcl/fontcharmap.hxx>
#include <unx/geninst.h>
#include <unx/fontmanager.hxx>
#include <unx/glyphcache.hxx>
#include <unx/genpspgraphics.h>

#include <sallayout.hxx>
#include <PhysicalFontCollection.hxx>

#include <QtGui/QGlyphRun>
#include <QtGui/QFontDatabase>
#include <QtGui/QRawFont>
#include <QtCore/QStringList>

void Qt5Graphics::SetTextColor(Color nColor) { m_aTextColor = nColor; }

void Qt5Graphics::SetFont(LogicalFontInstance* pReqFont, int nFallbackLevel)
{
    // release the text styles
    for (int i = nFallbackLevel; i < MAX_FALLBACK; ++i)
    {
        if (!m_pTextStyle[i])
            break;
        m_pTextStyle[i].clear();
    }

    if (!pReqFont)
        return;

    m_pTextStyle[nFallbackLevel] = static_cast<Qt5Font*>(pReqFont);
}

void Qt5Graphics::GetFontMetric(ImplFontMetricDataRef& rFMD, int nFallbackLevel)
{
    QRawFont aRawFont(QRawFont::fromFont(*m_pTextStyle[nFallbackLevel]));
    Qt5FontFace::fillAttributesFromQFont(*m_pTextStyle[nFallbackLevel], *rFMD);

    QByteArray aHheaTable = aRawFont.fontTable("hhea");
    std::vector<uint8_t> rHhea(aHheaTable.data(), aHheaTable.data() + aHheaTable.size());

    QByteArray aOs2Table = aRawFont.fontTable("OS/2");
    std::vector<uint8_t> rOS2(aOs2Table.data(), aOs2Table.data() + aOs2Table.size());

    rFMD->ImplCalcLineSpacing(rHhea, rOS2, aRawFont.unitsPerEm());

    rFMD->SetSlant(0);
    rFMD->SetWidth(aRawFont.averageCharWidth());

    rFMD->SetMinKashida(m_pTextStyle[nFallbackLevel]->GetKashidaWidth());
}

const FontCharMapRef Qt5Graphics::GetFontCharMap() const
{
    if (!m_pTextStyle[0])
        return FontCharMapRef(new FontCharMap());
    return static_cast<const Qt5FontFace*>(m_pTextStyle[0]->GetFontFace())->GetFontCharMap();
}

bool Qt5Graphics::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    if (!m_pTextStyle[0])
        return false;
    return static_cast<const Qt5FontFace*>(m_pTextStyle[0]->GetFontFace())
        ->GetFontCapabilities(rFontCapabilities);
}

void Qt5Graphics::GetDevFontList(PhysicalFontCollection* pPFC)
{
    static const bool bUseFontconfig = (nullptr == getenv("SAL_VCL_QT5_NO_FONTCONFIG"));

    m_pFontCollection = pPFC;
    if (pPFC->Count())
        return;

    QFontDatabase aFDB;
    QStringList aFontFamilyList;
    if (bUseFontconfig)
        aFontFamilyList = aFDB.families();
    GlyphCache& rGC = GlyphCache::GetInstance();
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    ::std::vector<psp::fontID> aList;
    psp::FastPrintFontInfo aInfo;

    rMgr.getFontList(aList);
    for (auto const& elem : aList)
    {
        if (!rMgr.getFontFastInfo(elem, aInfo))
            continue;

        // normalize face number to the GlyphCache
        int nFaceNum = rMgr.getFontFaceNumber(aInfo.m_nID);

        // inform GlyphCache about this font provided by the PsPrint subsystem
        FontAttributes aDFA = GenPspGraphics::Info2FontAttributes(aInfo);
        aDFA.IncreaseQualityBy(4096);
        const OString& rFileName = rMgr.getFontFileSysPath(aInfo.m_nID);
        rGC.AddFontFile(rFileName, nFaceNum, aInfo.m_nID, aDFA);

        // register font files unknown to Qt
        if (bUseFontconfig)
        {
            QString aFilename = toQString(
                OStringToOUString(rMgr.getFontFileSysPath(aInfo.m_nID), RTL_TEXTENCODING_UTF8));
            QRawFont aRawFont(aFilename, 0.0);
            QString aFamilyName = aRawFont.familyName();
            if (!aFontFamilyList.contains(aFamilyName)
                || !aFDB.styles(aFamilyName).contains(aRawFont.styleName()))
                QFontDatabase::addApplicationFont(aFilename);
        }
    }

    if (bUseFontconfig)
        SalGenericInstance::RegisterFontSubstitutors(pPFC);

    for (auto& family : aFDB.families())
        for (auto& style : aFDB.styles(family))
        {
            // Just get any size - we don't care
            QList<int> sizes = aFDB.smoothSizes(family, style);
            pPFC->Add(Qt5FontFace::fromQFont(aFDB.font(family, style, *sizes.begin())));
        }
}

void Qt5Graphics::ClearDevFontCache() {}

bool Qt5Graphics::AddTempDevFont(PhysicalFontCollection*, const OUString& /*rFileURL*/,
                                 const OUString& /*rFontName*/)
{
    return false;
}

bool Qt5Graphics::CreateFontSubset(const OUString& /*rToFile*/, const PhysicalFontFace* /*pFont*/,
                                   const sal_GlyphId* /*pGlyphIds*/, const sal_uInt8* /*pEncoding*/,
                                   sal_Int32* /*pWidths*/, int /*nGlyphs*/,
                                   FontSubsetInfo& /*rInfo*/)
{
    return false;
}

const void* Qt5Graphics::GetEmbedFontData(const PhysicalFontFace*, long* /*pDataLen*/)
{
    return nullptr;
}

void Qt5Graphics::FreeEmbedFontData(const void* /*pData*/, long /*nDataLen*/) {}

void Qt5Graphics::GetGlyphWidths(const PhysicalFontFace* /*pPFF*/, bool /*bVertical*/,
                                 std::vector<sal_Int32>& /*rWidths*/, Ucs2UIntMap& /*rUnicodeEnc*/)
{
}

class Qt5CommonSalLayout : public GenericSalLayout
{
public:
    Qt5CommonSalLayout(LogicalFontInstance& rLFI)
        : GenericSalLayout(rLFI)
    {
    }

    void SetOrientation(int nOrientation) { mnOrientation = nOrientation; }
};

std::unique_ptr<GenericSalLayout> Qt5Graphics::GetTextLayout(int nFallbackLevel)
{
    assert(m_pTextStyle[nFallbackLevel]);
    if (!m_pTextStyle[nFallbackLevel])
        return nullptr;
    return std::make_unique<Qt5CommonSalLayout>(*m_pTextStyle[nFallbackLevel]);
}

void Qt5Graphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    const Qt5Font* pFont = static_cast<const Qt5Font*>(&rLayout.GetFont());
    assert(pFont);
    QRawFont aRawFont(QRawFont::fromFont(*pFont));

    QVector<quint32> glyphIndexes;
    QVector<QPointF> positions;

    // prevent glyph rotation inside the SalLayout
    // probably better to add a parameter to GetNextGlyphs?
    Qt5CommonSalLayout* pQt5Layout
        = static_cast<Qt5CommonSalLayout*>(const_cast<GenericSalLayout*>(&rLayout));
    int nOrientation = rLayout.GetOrientation();
    if (nOrientation)
        pQt5Layout->SetOrientation(0);

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        glyphIndexes.push_back(pGlyph->m_aGlyphId);
        positions.push_back(QPointF(aPos.X(), aPos.Y()));
    }

    if (nOrientation)
        pQt5Layout->SetOrientation(nOrientation);

    QGlyphRun aGlyphRun;
    aGlyphRun.setPositions(positions);
    aGlyphRun.setGlyphIndexes(glyphIndexes);
    aGlyphRun.setRawFont(aRawFont);

    Qt5Painter aPainter(*this);
    QColor aColor = toQColor(m_aTextColor);
    aPainter.setPen(aColor);

    if (nOrientation)
    {
        // make text position the center of the rotation
        // then rotate and move back
        QRect window = aPainter.window();
        window.moveTo(-positions[0].x(), -positions[0].y());
        aPainter.setWindow(window);

        QTransform p;
        p.rotate(-static_cast<qreal>(nOrientation) / 10.0);
        p.translate(-positions[0].x(), -positions[0].y());
        aPainter.setTransform(p);
    }

    aPainter.drawGlyphRun(QPointF(), aGlyphRun);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
