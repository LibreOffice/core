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

#include <Qt5Font.hxx>
#include <Qt5Tools.hxx>

#include <QtGui/QFont>
#include <QtGui/QRawFont>

static inline void applyWeight(QtFont& rFont, FontWeight eWeight)
{
    switch (eWeight)
    {
        case WEIGHT_THIN:
            rFont.setWeight(QFont::Thin);
            break;
        case WEIGHT_ULTRALIGHT:
            rFont.setWeight(QFont::ExtraLight);
            break;
        case WEIGHT_LIGHT:
            rFont.setWeight(QFont::Light);
            break;
        case WEIGHT_SEMILIGHT:
            [[fallthrough]];
        case WEIGHT_NORMAL:
            rFont.setWeight(QFont::Normal);
            break;
        case WEIGHT_MEDIUM:
            rFont.setWeight(QFont::Medium);
            break;
        case WEIGHT_SEMIBOLD:
            rFont.setWeight(QFont::DemiBold);
            break;
        case WEIGHT_BOLD:
            rFont.setWeight(QFont::Bold);
            break;
        case WEIGHT_ULTRABOLD:
            rFont.setWeight(QFont::ExtraBold);
            break;
        case WEIGHT_BLACK:
            rFont.setWeight(QFont::Black);
            break;
        default:
            break;
    }
}

static inline void applyStretch(QtFont& rFont, FontWidth eWidthType)
{
    switch (eWidthType)
    {
        case WIDTH_DONTKNOW:
            rFont.setStretch(QFont::AnyStretch);
            break;
        case WIDTH_ULTRA_CONDENSED:
            rFont.setStretch(QFont::UltraCondensed);
            break;
        case WIDTH_EXTRA_CONDENSED:
            rFont.setStretch(QFont::ExtraCondensed);
            break;
        case WIDTH_CONDENSED:
            rFont.setStretch(QFont::Condensed);
            break;
        case WIDTH_SEMI_CONDENSED:
            rFont.setStretch(QFont::SemiCondensed);
            break;
        case WIDTH_NORMAL:
            rFont.setStretch(QFont::Unstretched);
            break;
        case WIDTH_SEMI_EXPANDED:
            rFont.setStretch(QFont::SemiExpanded);
            break;
        case WIDTH_EXPANDED:
            rFont.setStretch(QFont::Expanded);
            break;
        case WIDTH_EXTRA_EXPANDED:
            rFont.setStretch(QFont::ExtraExpanded);
            break;
        case WIDTH_ULTRA_EXPANDED:
            rFont.setStretch(QFont::UltraExpanded);
            break;
        default:
            break;
    }
}

static inline void applyStyle(QtFont& rFont, FontItalic eItalic)
{
    switch (eItalic)
    {
        case ITALIC_NONE:
            rFont.setStyle(QFont::Style::StyleNormal);
            break;
        case ITALIC_OBLIQUE:
            rFont.setStyle(QFont::Style::StyleOblique);
            break;
        case ITALIC_NORMAL:
            rFont.setStyle(QFont::Style::StyleItalic);
            break;
        default:
            break;
    }
}

QtFont::QtFont(const PhysicalFontFace& rPFF, const vcl::font::FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
{
    setFamily(toQString(rPFF.GetFamilyName()));
    applyWeight(*this, rPFF.GetWeight());
    setPixelSize(rFSP.mnHeight);
    applyStretch(*this, rPFF.GetWidthType());
    applyStyle(*this, rFSP.GetItalic());
}

static hb_blob_t* getFontTable(hb_face_t*, hb_tag_t nTableTag, void* pUserData)
{
    char pTagName[5];
    LogicalFontInstance::DecodeOpenTypeTag(nTableTag, pTagName);

    QtFont* pFont = static_cast<QtFont*>(pUserData);
    QRawFont aRawFont(QRawFont::fromFont(*pFont));
    QByteArray aTable = aRawFont.fontTable(pTagName);
    const sal_uInt32 nLength = aTable.size();

    hb_blob_t* pBlob = nullptr;
    if (nLength > 0)
        pBlob = hb_blob_create(aTable.data(), nLength, HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr);
    return pBlob;
}

hb_font_t* QtFont::ImplInitHbFont()
{
    return InitHbFont(hb_face_create_for_tables(getFontTable, this, nullptr));
}

bool QtFont::GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const { return false; }

bool QtFont::ImplGetGlyphBoundRect(sal_GlyphId nId, tools::Rectangle& rRect, bool) const
{
    QRawFont aRawFont(QRawFont::fromFont(*this));
    rRect = toRectangle(aRawFont.boundingRect(nId).toAlignedRect());
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
