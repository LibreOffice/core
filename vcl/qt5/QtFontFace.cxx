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

#include <unotools/fontdefs.hxx>

#include <QtFontFace.hxx>
#include <QtFont.hxx>
#include <QtTools.hxx>

#include <sft.hxx>
#include <impfontcharmap.hxx>
#include <fontinstance.hxx>
#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontCollection.hxx>

#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QFontInfo>
#include <QtGui/QRawFont>

using namespace vcl;

QtFontFace::QtFontFace(const QtFontFace& rSrc)
    : vcl::font::PhysicalFontFace(rSrc)
    , m_aFontId(rSrc.m_aFontId)
    , m_eFontIdType(rSrc.m_eFontIdType)
{
    if (rSrc.m_xCharMap.is())
        m_xCharMap = rSrc.m_xCharMap;
}

FontWeight QtFontFace::toFontWeight(const int nWeight)
{
    if (nWeight <= QFont::Thin)
        return WEIGHT_THIN;
    if (nWeight <= QFont::ExtraLight)
        return WEIGHT_ULTRALIGHT;
    if (nWeight <= QFont::Light)
        return WEIGHT_LIGHT;
    if (nWeight <= QFont::Normal)
        return WEIGHT_NORMAL;
    if (nWeight <= QFont::Medium)
        return WEIGHT_MEDIUM;
    if (nWeight <= QFont::DemiBold)
        return WEIGHT_SEMIBOLD;
    if (nWeight <= QFont::Bold)
        return WEIGHT_BOLD;
    if (nWeight <= QFont::ExtraBold)
        return WEIGHT_ULTRABOLD;
    return WEIGHT_BLACK;
}

FontWidth QtFontFace::toFontWidth(const int nStretch)
{
    if (nStretch == 0) // QFont::AnyStretch since Qt 5.8
        return WIDTH_DONTKNOW;
    if (nStretch <= QFont::UltraCondensed)
        return WIDTH_ULTRA_CONDENSED;
    if (nStretch <= QFont::ExtraCondensed)
        return WIDTH_EXTRA_CONDENSED;
    if (nStretch <= QFont::Condensed)
        return WIDTH_CONDENSED;
    if (nStretch <= QFont::SemiCondensed)
        return WIDTH_SEMI_CONDENSED;
    if (nStretch <= QFont::Unstretched)
        return WIDTH_NORMAL;
    if (nStretch <= QFont::SemiExpanded)
        return WIDTH_SEMI_EXPANDED;
    if (nStretch <= QFont::Expanded)
        return WIDTH_EXPANDED;
    if (nStretch <= QFont::ExtraExpanded)
        return WIDTH_EXTRA_EXPANDED;
    return WIDTH_ULTRA_EXPANDED;
}

FontItalic QtFontFace::toFontItalic(const QFont::Style eStyle)
{
    switch (eStyle)
    {
        case QFont::StyleNormal:
            return ITALIC_NONE;
        case QFont::StyleItalic:
            return ITALIC_NORMAL;
        case QFont::StyleOblique:
            return ITALIC_OBLIQUE;
    }

    return ITALIC_NONE;
}

void QtFontFace::fillAttributesFromQFont(const QFont& rFont, FontAttributes& rFA)
{
    QFontInfo aFontInfo(rFont);

    rFA.SetFamilyName(toOUString(aFontInfo.family()));
    if (IsStarSymbol(toOUString(aFontInfo.family())))
        rFA.SetSymbolFlag(true);
    rFA.SetStyleName(toOUString(aFontInfo.styleName()));
    rFA.SetPitch(aFontInfo.fixedPitch() ? PITCH_FIXED : PITCH_VARIABLE);
    rFA.SetWeight(QtFontFace::toFontWeight(aFontInfo.weight()));
    rFA.SetItalic(QtFontFace::toFontItalic(aFontInfo.style()));
    rFA.SetWidthType(QtFontFace::toFontWidth(rFont.stretch()));
}

QtFontFace* QtFontFace::fromQFont(const QFont& rFont)
{
    FontAttributes aFA;
    fillAttributesFromQFont(rFont, aFA);
    return new QtFontFace(aFA, rFont.toString(), FontIdType::Font);
}

QtFontFace* QtFontFace::fromQFontDatabase(const QString& aFamily, const QString& aStyle)
{
    QFontDatabase aFDB;
    FontAttributes aFA;

    aFA.SetFamilyName(toOUString(aFamily));
    if (IsStarSymbol(aFA.GetFamilyName()))
        aFA.SetSymbolFlag(true);
    aFA.SetStyleName(toOUString(aStyle));
    aFA.SetPitch(aFDB.isFixedPitch(aFamily, aStyle) ? PITCH_FIXED : PITCH_VARIABLE);
    aFA.SetWeight(QtFontFace::toFontWeight(aFDB.weight(aFamily, aStyle)));
    aFA.SetItalic(aFDB.italic(aFamily, aStyle) ? ITALIC_NORMAL : ITALIC_NONE);

    int nPointSize = 0;
    QList<int> aPointList = aFDB.pointSizes(aFamily, aStyle);
    if (!aPointList.empty())
        nPointSize = aPointList[0];

    return new QtFontFace(aFA, aFamily + "," + aStyle + "," + QString::number(nPointSize),
                          FontIdType::FontDB);
}

QtFontFace::QtFontFace(const FontAttributes& rFA, const QString& rFontID,
                       const FontIdType eFontIdType)
    : PhysicalFontFace(rFA)
    , m_aFontId(rFontID)
    , m_eFontIdType(eFontIdType)
    , m_bFontCapabilitiesRead(false)
{
}

sal_IntPtr QtFontFace::GetFontId() const { return reinterpret_cast<sal_IntPtr>(&m_aFontId); }

QFont QtFontFace::CreateFont() const
{
    QFont aFont;
    switch (m_eFontIdType)
    {
        case FontDB:
        {
            QFontDatabase aFDB;
            QStringList aStrList = m_aFontId.split(",");
            if (3 == aStrList.size())
                aFont = aFDB.font(aStrList[0], aStrList[1], aStrList[2].toInt());
            else
                SAL_WARN("vcl.qt", "Invalid QFontDatabase font ID " << m_aFontId);
            break;
        }
        case Font:
            bool bRet = aFont.fromString(m_aFontId);
            SAL_WARN_IF(!bRet, "vcl.qt", "Failed to create QFont from ID: " << m_aFontId);
            Q_UNUSED(bRet);
            break;
    }
    return aFont;
}

rtl::Reference<LogicalFontInstance>
QtFontFace::CreateFontInstance(const vcl::font::FontSelectPattern& rFSD) const
{
    return new QtFont(*this, rFSD);
}

FontCharMapRef QtFontFace::GetFontCharMap() const
{
    if (m_xCharMap.is())
        return m_xCharMap;

    QFont aFont = CreateFont();
    QRawFont aRawFont(QRawFont::fromFont(aFont));
    QByteArray aCMapTable = aRawFont.fontTable("cmap");
    if (aCMapTable.isEmpty())
    {
        m_xCharMap = new FontCharMap();
        return m_xCharMap;
    }

    CmapResult aCmapResult;
    if (ParseCMAP(reinterpret_cast<const unsigned char*>(aCMapTable.data()), aCMapTable.size(),
                  aCmapResult))
        m_xCharMap = new FontCharMap(aCmapResult);

    return m_xCharMap;
}

bool QtFontFace::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    // read this only once per font
    if (m_bFontCapabilitiesRead)
    {
        rFontCapabilities = m_aFontCapabilities;
        return rFontCapabilities.oUnicodeRange || rFontCapabilities.oCodePageRange;
    }
    m_bFontCapabilitiesRead = true;

    QFont aFont = CreateFont();
    QRawFont aRawFont(QRawFont::fromFont(aFont));
    QByteArray aOS2Table = aRawFont.fontTable("OS/2");
    if (!aOS2Table.isEmpty())
    {
        vcl::getTTCoverage(m_aFontCapabilities.oUnicodeRange, m_aFontCapabilities.oCodePageRange,
                           reinterpret_cast<const unsigned char*>(aOS2Table.data()),
                           aOS2Table.size());
    }

    rFontCapabilities = m_aFontCapabilities;
    return rFontCapabilities.oUnicodeRange || rFontCapabilities.oCodePageRange;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
