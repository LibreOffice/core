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

#include <sal/config.h>

#include <QtGraphics.hxx>
#include <QtFontFace.hxx>
#include <QtFont.hxx>
#include <QtPainter.hxx>

#include <fontsubset.hxx>
#include <vcl/fontcharmap.hxx>
#include <unx/geninst.h>
#include <unx/fontmanager.hxx>
#include <unx/glyphcache.hxx>
#include <unx/genpspgraphics.h>

#include <sallayout.hxx>
#include <font/PhysicalFontCollection.hxx>

#include <QtGui/QGlyphRun>
#include <QtGui/QFontDatabase>
#include <QtGui/QRawFont>
#include <QtCore/QStringList>

void QtGraphics::SetTextColor(Color nColor) { m_aTextColor = nColor; }

void QtGraphics::SetFont(LogicalFontInstance* pReqFont, int nFallbackLevel)
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

    m_pTextStyle[nFallbackLevel] = static_cast<QtFont*>(pReqFont);
}

void QtGraphics::GetFontMetric(ImplFontMetricDataRef& rFMD, int nFallbackLevel)
{
    auto& rFI = *m_pTextStyle[nFallbackLevel];
    QRawFont aRawFont(QRawFont::fromFont(rFI));
    QtFontFace::fillAttributesFromQFont(rFI, *rFMD);

    rFMD->SetSlant(0);
    rFMD->ImplCalcLineSpacing(&rFI);
    rFMD->ImplInitBaselines(&rFI);
    rFMD->SetMinKashida(rFI.GetKashidaWidth());
}

FontCharMapRef QtGraphics::GetFontCharMap() const
{
    if (!m_pTextStyle[0])
        return FontCharMapRef(new FontCharMap());
    return m_pTextStyle[0]->GetFontFace()->GetFontCharMap();
}

bool QtGraphics::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    if (!m_pTextStyle[0])
        return false;
    return m_pTextStyle[0]->GetFontFace()->GetFontCapabilities(rFontCapabilities);
}

void QtGraphics::GetDevFontList(vcl::font::PhysicalFontCollection* pPFC)
{
    static const bool bUseFontconfig = (nullptr == getenv("SAL_VCL_QT5_NO_FONTCONFIG"));

    if (pPFC->Count())
        return;

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    for (auto& family : QFontDatabase::families())
        for (auto& style : QFontDatabase::styles(family))
            pPFC->Add(QtFontFace::fromQFontDatabase(family, style));
#else
    QFontDatabase aFDB;
    for (auto& family : aFDB.families())
        for (auto& style : aFDB.styles(family))
            pPFC->Add(QtFontFace::fromQFontDatabase(family, style));
#endif
}

void QtGraphics::ClearDevFontCache() {}

bool QtGraphics::AddTempDevFont(vcl::font::PhysicalFontCollection*, const OUString& /*rFileURL*/,
                                const OUString& /*rFontName*/)
{
    return false;
}

namespace
{
class QtTrueTypeFont : public vcl::AbstractTrueTypeFont
{
    const QRawFont& m_aRawFont;
    mutable QByteArray m_aFontTable[vcl::NUM_TAGS];

public:
    QtTrueTypeFont(const QtFontFace& aFontFace, const QRawFont& aRawFont);

    bool hasTable(sal_uInt32 ord) const override;
    const sal_uInt8* table(sal_uInt32 ord, sal_uInt32& size) const override;
};

QtTrueTypeFont::QtTrueTypeFont(const QtFontFace& aFontFace, const QRawFont& aRawFont)
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

bool QtTrueTypeFont::hasTable(sal_uInt32 ord) const
{
    const char* table_char = vclFontTableAsChar(ord);
    if (!table_char)
        return false;
    if (m_aFontTable[ord].isEmpty())
        m_aFontTable[ord] = m_aRawFont.fontTable(table_char);
    return !m_aFontTable[ord].isEmpty();
}

const sal_uInt8* QtTrueTypeFont::table(sal_uInt32 ord, sal_uInt32& size) const
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

bool QtGraphics::CreateFontSubset(const OUString& rToFile,
                                  const vcl::font::PhysicalFontFace* pFontFace,
                                  const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                  sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo)
{
    OUString aSysPath;
    if (osl_File_E_None != osl_getSystemPathFromFileURL(rToFile.pData, &aSysPath.pData))
        return false;

    // get the raw-bytes from the font to be subset
    const QtFontFace* pQtFontFace = static_cast<const QtFontFace*>(pFontFace);
    const QFont aFont = pQtFontFace->CreateFont();
    const QRawFont aRawFont(QRawFont::fromFont(aFont));
    const OString aToFile(OUStringToOString(aSysPath, osl_getThreadTextEncoding()));

    // handle CFF-subsetting
    QByteArray aCFFtable = aRawFont.fontTable("CFF ");
    if (!aCFFtable.isEmpty())
        return SalGraphics::CreateCFFfontSubset(
            reinterpret_cast<const sal_uInt8*>(aCFFtable.data()), aCFFtable.size(), aToFile,
            pGlyphIds, pEncoding, pGlyphWidths, nGlyphCount, rInfo);

    // fill details about the subsetted font
    rInfo.m_nFontType = FontType::SFNT_TTF;
    rInfo.m_aPSName = toOUString(aRawFont.familyName());
    rInfo.m_nCapHeight = aRawFont.capHeight();
    rInfo.m_nAscent = aRawFont.ascent();
    rInfo.m_nDescent = aRawFont.descent();

    QtTrueTypeFont aTTF(*pQtFontFace, aRawFont);
    int nXmin, nYmin, nXmax, nYmax;
    sal_uInt16 nMacStyleFlags;
    if (GetTTGlobalFontHeadInfo(&aTTF, nXmin, nYmin, nXmax, nYmax, nMacStyleFlags))
        rInfo.m_aFontBBox = tools::Rectangle(Point(nXmin, nYmin), Point(nXmax, nYmax));

    return SalGraphics::CreateTTFfontSubset(aTTF, aToFile, false /* use FontSelectPattern? */,
                                            pGlyphIds, pEncoding, pGlyphWidths, nGlyphCount);
}

const void* QtGraphics::GetEmbedFontData(const vcl::font::PhysicalFontFace*,
                                         tools::Long* /*pDataLen*/)
{
    return nullptr;
}

void QtGraphics::FreeEmbedFontData(const void* /*pData*/, tools::Long /*nDataLen*/) {}

void QtGraphics::GetGlyphWidths(const vcl::font::PhysicalFontFace* pFontFace, bool bVertical,
                                std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc)
{
    const QtFontFace* pQtFontFace = static_cast<const QtFontFace*>(pFontFace);
    const QRawFont aRawFont(QRawFont::fromFont(pQtFontFace->CreateFont()));
    QtTrueTypeFont aTTF(*pQtFontFace, aRawFont);
    SalGraphics::GetGlyphWidths(aTTF, *pFontFace, bVertical, rWidths, rUnicodeEnc);
}

namespace
{
class QtCommonSalLayout : public GenericSalLayout
{
public:
    QtCommonSalLayout(LogicalFontInstance& rLFI)
        : GenericSalLayout(rLFI)
    {
    }

    void SetOrientation(Degree10 nOrientation) { mnOrientation = nOrientation; }
};
}

std::unique_ptr<GenericSalLayout> QtGraphics::GetTextLayout(int nFallbackLevel)
{
    assert(m_pTextStyle[nFallbackLevel]);
    if (!m_pTextStyle[nFallbackLevel])
        return nullptr;
    return std::make_unique<QtCommonSalLayout>(*m_pTextStyle[nFallbackLevel]);
}

static QRawFont GetRawFont(const QFont& rFont, bool bWithoutHintingInTextDirection)
{
    QFont::HintingPreference eHinting = rFont.hintingPreference();
    bool bAllowedHintStyle
        = !bWithoutHintingInTextDirection
          || (eHinting == QFont::PreferNoHinting || eHinting == QFont::PreferVerticalHinting);
    if (bWithoutHintingInTextDirection && !bAllowedHintStyle)
    {
        QFont aFont(rFont);
        aFont.setHintingPreference(QFont::PreferVerticalHinting);
        return QRawFont::fromFont(aFont);
    }
    return QRawFont::fromFont(rFont);
}

void QtGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    const QtFont* pFont = static_cast<const QtFont*>(&rLayout.GetFont());
    assert(pFont);
    QRawFont aRawFont(GetRawFont(*pFont, getTextRenderModeForResolutionIndependentLayoutEnabled()));

    QVector<quint32> glyphIndexes;
    QVector<QPointF> positions;

    // prevent glyph rotation inside the SalLayout
    // probably better to add a parameter to GetNextGlyphs?
    QtCommonSalLayout* pQtLayout
        = static_cast<QtCommonSalLayout*>(const_cast<GenericSalLayout*>(&rLayout));
    Degree10 nOrientation = rLayout.GetOrientation();
    if (nOrientation)
        pQtLayout->SetOrientation(0_deg10);

    DevicePoint aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        glyphIndexes.push_back(pGlyph->glyphId());
        positions.push_back(QPointF(aPos.getX(), aPos.getY()));
    }

    // seems to be common to try to layout an empty string...
    if (positions.empty())
        return;

    if (nOrientation)
        pQtLayout->SetOrientation(nOrientation);

    QGlyphRun aGlyphRun;
    aGlyphRun.setPositions(positions);
    aGlyphRun.setGlyphIndexes(glyphIndexes);
    aGlyphRun.setRawFont(aRawFont);

    QtPainter aPainter(*m_pBackend);
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
        p.rotate(-static_cast<qreal>(nOrientation.get()) / 10.0);
        p.translate(-positions[0].x(), -positions[0].y());
        aPainter.setTransform(p);
    }

    aPainter.drawGlyphRun(QPointF(), aGlyphRun);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
