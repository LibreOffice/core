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

#include <fontsubset.hxx>
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

    rFMD->ImplCalcLineSpacing(m_pTextStyle[nFallbackLevel].get());

    rFMD->SetSlant(0);
    rFMD->SetWidth(aRawFont.averageCharWidth());

    rFMD->SetMinKashida(m_pTextStyle[nFallbackLevel]->GetKashidaWidth());
}

FontCharMapRef Qt5Graphics::GetFontCharMap() const
{
    if (!m_pTextStyle[0])
        return FontCharMapRef(new FontCharMap());
    return m_pTextStyle[0]->GetFontFace()->GetFontCharMap();
}

bool Qt5Graphics::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    if (!m_pTextStyle[0])
        return false;
    return m_pTextStyle[0]->GetFontFace()->GetFontCapabilities(rFontCapabilities);
}

void Qt5Graphics::GetDevFontList(PhysicalFontCollection* pPFC)
{
    static const bool bUseFontconfig = (nullptr == getenv("SAL_VCL_QT5_NO_FONTCONFIG"));

    m_pFontCollection = pPFC;
    if (pPFC->Count())
        return;

    QFontDatabase aFDB;
    FreetypeManager& rFontManager = FreetypeManager::get();
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    ::std::vector<psp::fontID> aList;
    psp::FastPrintFontInfo aInfo;

    rMgr.getFontList(aList);
    for (auto const& elem : aList)
    {
        if (!rMgr.getFontFastInfo(elem, aInfo))
            continue;

        // normalize face number to the FreetypeManager
        int nFaceNum = rMgr.getFontFaceNumber(aInfo.m_nID);
        int nVariantNum = rMgr.getFontFaceVariation(aInfo.m_nID);

        // inform FreetypeManager about this font provided by the PsPrint subsystem
        FontAttributes aDFA = GenPspGraphics::Info2FontAttributes(aInfo);
        aDFA.IncreaseQualityBy(4096);
        const OString& rFileName = rMgr.getFontFileSysPath(aInfo.m_nID);
        rFontManager.AddFontFile(rFileName, nFaceNum, nVariantNum, aInfo.m_nID, aDFA);
    }

    if (bUseFontconfig)
        SalGenericInstance::RegisterFontSubstitutors(pPFC);

    for (auto& family : aFDB.families())
        for (auto& style : aFDB.styles(family))
            pPFC->Add(Qt5FontFace::fromQFontDatabase(family, style));
}

void Qt5Graphics::ClearDevFontCache() {}

bool Qt5Graphics::AddTempDevFont(PhysicalFontCollection*, const OUString& /*rFileURL*/,
                                 const OUString& /*rFontName*/)
{
    return false;
}

namespace
{
class Qt5TrueTypeFont : public vcl::AbstractTrueTypeFont
{
    const QRawFont& m_aRawFont;
    mutable QByteArray m_aFontTable[vcl::NUM_TAGS];

public:
    Qt5TrueTypeFont(const Qt5FontFace& aFontFace, const QRawFont& aRawFont);

    bool hasTable(sal_uInt32 ord) const override;
    const sal_uInt8* table(sal_uInt32 ord, sal_uInt32& size) const override;
};

Qt5TrueTypeFont::Qt5TrueTypeFont(const Qt5FontFace& aFontFace, const QRawFont& aRawFont)
    : vcl::AbstractTrueTypeFont(nullptr, aFontFace.GetFontCharMap())
    , m_aRawFont(aRawFont)
{
    indexGlyphData();
}

const char* vclFontTableAsChar(sal_uInt32 ord)
{
    switch (ord)
    {
        case vcl::O_maxp:
            return "maxp";
        case vcl::O_glyf:
            return "glyf";
        case vcl::O_head:
            return "head";
        case vcl::O_loca:
            return "loca";
        case vcl::O_name:
            return "name";
        case vcl::O_hhea:
            return "hhea";
        case vcl::O_hmtx:
            return "hmtx";
        case vcl::O_cmap:
            return "cmap";
        case vcl::O_vhea:
            return "vhea";
        case vcl::O_vmtx:
            return "vmtx";
        case vcl::O_OS2:
            return "OS/2";
        case vcl::O_post:
            return "post";
        case vcl::O_cvt:
            return "cvt ";
        case vcl::O_prep:
            return "prep";
        case vcl::O_fpgm:
            return "fpgm";
        case vcl::O_gsub:
            return "gsub";
        case vcl::O_CFF:
            return "CFF ";
        default:
            return nullptr;
    }
}

bool Qt5TrueTypeFont::hasTable(sal_uInt32 ord) const
{
    const char* table_char = vclFontTableAsChar(ord);
    if (!table_char)
        return false;
    if (m_aFontTable[ord].isEmpty())
        m_aFontTable[ord] = m_aRawFont.fontTable(table_char);
    return !m_aFontTable[ord].isEmpty();
}

const sal_uInt8* Qt5TrueTypeFont::table(sal_uInt32 ord, sal_uInt32& size) const
{
    const char* table_char = vclFontTableAsChar(ord);
    if (!table_char)
        return nullptr;
    if (m_aFontTable[ord].isEmpty())
        m_aFontTable[ord] = m_aRawFont.fontTable(table_char);
    size = m_aFontTable[ord].size();
    return reinterpret_cast<const sal_uInt8*>(m_aFontTable[ord].data());
}
}

bool Qt5Graphics::CreateFontSubset(const OUString& rToFile, const PhysicalFontFace* pFontFace,
                                   const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                   sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo)
{
    // prepare the requested file name for writing the font-subset file
    OUString aSysPath;
    if (osl_File_E_None != osl_getSystemPathFromFileURL(rToFile.pData, &aSysPath.pData))
        return false;

    // get the raw-bytes from the font to be subset
    const Qt5FontFace* pQt5FontFace = static_cast<const Qt5FontFace*>(pFontFace);
    const QFont aFont = pQt5FontFace->CreateFont();
    const QRawFont aRawFont(QRawFont::fromFont(aFont));
    const QFontInfo aFontInfo(aFont);
    const OString aToFile(OUStringToOString(aSysPath, osl_getThreadTextEncoding()));
    const int nOrigGlyphCount = nGlyphCount;

    QByteArray aCFFtable = aRawFont.fontTable("CFF ");
    if (!aCFFtable.isEmpty())
    {
        FILE* pOutFile = fopen(aToFile.getStr(), "wb");
        rInfo.LoadFont(FontType::CFF_FONT, reinterpret_cast<const sal_uInt8*>(aCFFtable.data()),
                       aCFFtable.size());
        bool bRet = rInfo.CreateFontSubset(FontType::TYPE1_PFB, pOutFile, nullptr, pGlyphIds,
                                           pEncoding, nGlyphCount, pGlyphWidths);
        fclose(pOutFile);
        return bRet;
    }

    // get details about the subsetted font
    rInfo.m_nFontType = FontType::SFNT_TTF;
    rInfo.m_aPSName = toOUString(aRawFont.familyName());
    rInfo.m_nCapHeight = aRawFont.capHeight();
    rInfo.m_nAscent = aRawFont.ascent();
    rInfo.m_nDescent = aRawFont.descent();

    Qt5TrueTypeFont aTTF(*pQt5FontFace, aRawFont);
    int nXmin, nYmin, nXmax, nYmax;
    sal_uInt16 nMacStyleFlags;
    if (GetTTGlobalFontHeadInfo(&aTTF, nXmin, nYmin, nXmax, nYmax, nMacStyleFlags))
        rInfo.m_aFontBBox = tools::Rectangle(Point(nXmin, nYmin), Point(nXmax, nYmax));

    sal_uInt16 aShortIDs[nGlyphCount + 1];
    sal_uInt8 aTempEncs[nGlyphCount + 1];

    int nNotDef = -1;

    for (int i = 0; i < nGlyphCount; ++i)
    {
        aTempEncs[i] = pEncoding[i];

        sal_GlyphId aGlyphId(pGlyphIds[i]);
        aShortIDs[i] = static_cast<sal_uInt16>(aGlyphId);
        if (!aGlyphId && nNotDef < 0)
            nNotDef = i; // first NotDef glyph found
    }

    if (nNotDef != 0)
    {
        // add fake NotDef glyph if needed
        if (nNotDef < 0)
            nNotDef = nGlyphCount++;
        // NotDef glyph must be in pos 0 => swap glyphids
        aShortIDs[nNotDef] = aShortIDs[0];
        aTempEncs[nNotDef] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }

    std::unique_ptr<sal_uInt16[]> pGlyphMetrics
        = GetTTSimpleGlyphMetrics(&aTTF, aShortIDs, nGlyphCount, false);
    if (!pGlyphMetrics)
        return false;

    sal_uInt16 nNotDefAdv = pGlyphMetrics[0];
    pGlyphMetrics[0] = pGlyphMetrics[nNotDef];
    pGlyphMetrics[nNotDef] = nNotDefAdv;

    for (int i = 0; i < nOrigGlyphCount; ++i)
        pGlyphWidths[i] = pGlyphMetrics[i];

    // write subset into destination file
    vcl::SFErrCodes nRC
        = vcl::CreateTTFromTTGlyphs(&aTTF, aToFile.getStr(), aShortIDs, aTempEncs, nGlyphCount);
    return (nRC == vcl::SFErrCodes::Ok);
}

const void* Qt5Graphics::GetEmbedFontData(const PhysicalFontFace*, long* /*pDataLen*/)
{
    return nullptr;
}

void Qt5Graphics::FreeEmbedFontData(const void* /*pData*/, long /*nDataLen*/) {}

void Qt5Graphics::GetGlyphWidths(const PhysicalFontFace* pFontFace, bool bVertical,
                                 std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc)
{
    const Qt5FontFace* pQt5FontFace = static_cast<const Qt5FontFace*>(pFontFace);
    const QRawFont aRawFont(QRawFont::fromFont(pQt5FontFace->CreateFont()));
    Qt5TrueTypeFont aTTF(*pQt5FontFace, aRawFont);
    SalGraphics::GetGlyphWidths(aTTF, *pFontFace, bVertical, rWidths, rUnicodeEnc);
}

namespace
{
class Qt5CommonSalLayout : public GenericSalLayout
{
public:
    Qt5CommonSalLayout(LogicalFontInstance& rLFI)
        : GenericSalLayout(rLFI)
    {
    }

    void SetOrientation(int nOrientation) { mnOrientation = nOrientation; }
};
}

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
        glyphIndexes.push_back(pGlyph->glyphId());
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
