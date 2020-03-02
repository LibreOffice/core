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

#include <Qt5Font.hxx>
#include <Qt5Tools.hxx>

#include <QtGui/QFont>
#include <QtGui/QRawFont>

static QFont::Weight toWeight(FontWeight eWeight)
{
    switch (eWeight)
    {
        case WEIGHT_THIN:
            return QFont::Thin;
        case WEIGHT_ULTRALIGHT:
            return QFont::ExtraLight;
        case WEIGHT_LIGHT:
            return QFont::Light;
        case WEIGHT_SEMILIGHT:
            [[fallthrough]];
        case WEIGHT_DONTKNOW:
            [[fallthrough]];
        case WEIGHT_NORMAL:
            return QFont::Normal;
        case WEIGHT_MEDIUM:
            return QFont::Medium;
        case WEIGHT_SEMIBOLD:
            return QFont::DemiBold;
        case WEIGHT_BOLD:
            return QFont::Bold;
        case WEIGHT_ULTRABOLD:
            return QFont::ExtraBold;
        case WEIGHT_BLACK:
            return QFont::Black;
        case FontWeight_FORCE_EQUAL_SIZE:
            assert(false && "FontWeight_FORCE_EQUAL_SIZE not implementable for QFont");
    }

    // so we would get enum not handled warning
    return QFont::Normal;
}

static int toStretch(FontWidth eWidthType)
{
    switch (eWidthType)
    {
        case WIDTH_DONTKNOW:
            return QFont::AnyStretch;
        case WIDTH_ULTRA_CONDENSED:
            return QFont::UltraCondensed;
        case WIDTH_EXTRA_CONDENSED:
            return QFont::ExtraCondensed;
        case WIDTH_CONDENSED:
            return QFont::Condensed;
        case WIDTH_SEMI_CONDENSED:
            return QFont::SemiCondensed;
        case WIDTH_NORMAL:
            return QFont::Unstretched;
        case WIDTH_SEMI_EXPANDED:
            return QFont::SemiExpanded;
        case WIDTH_EXPANDED:
            return QFont::Expanded;
        case WIDTH_EXTRA_EXPANDED:
            return QFont::ExtraExpanded;
        case WIDTH_ULTRA_EXPANDED:
            return QFont::UltraExpanded;
        case FontWidth_FORCE_EQUAL_SIZE:
            assert(false && "FontWidth_FORCE_EQUAL_SIZE not implementable for QFont");
    }

    // so we would get enum not handled warning
    return QFont::AnyStretch;
}

static QFont::Style toStyle(FontItalic eItalic)
{
    switch (eItalic)
    {
        case ITALIC_DONTKNOW:
            [[fallthrough]];
        case ITALIC_NONE:
            return QFont::Style::StyleNormal;
        case ITALIC_OBLIQUE:
            return QFont::Style::StyleOblique;
        case ITALIC_NORMAL:
            return QFont::Style::StyleItalic;
        case FontItalic_FORCE_EQUAL_SIZE:
            assert(false && "FontItalic_FORCE_EQUAL_SIZE not implementable for QFont");
    }

    // so we would get enum not handled warning
    return QFont::Style::StyleNormal;
}

Qt5Font::Qt5Font(const PhysicalFontFace& rPFF, const FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
{
    setFamily(toQString(rPFF.GetFamilyName()));
    setWeight(toWeight(rPFF.GetWeight()));
    setPixelSize(rFSP.mnHeight);
    setStretch(toStretch(rPFF.GetWidthType()));
    setStyle(toStyle(rFSP.GetItalic()));
}

static hb_blob_t* getFontTable(hb_face_t*, hb_tag_t nTableTag, void* pUserData)
{
    char pTagName[5];
    LogicalFontInstance::DecodeOpenTypeTag(nTableTag, pTagName);

    Qt5Font* pFont = static_cast<Qt5Font*>(pUserData);
    QRawFont aRawFont(QRawFont::fromFont(*pFont));
    QByteArray aTable = aRawFont.fontTable(pTagName);
    const sal_uInt32 nLength = aTable.size();

    hb_blob_t* pBlob = nullptr;
    if (nLength > 0)
        pBlob = hb_blob_create(aTable.data(), nLength, HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr);
    return pBlob;
}

hb_font_t* Qt5Font::ImplInitHbFont()
{
    return InitHbFont(hb_face_create_for_tables(getFontTable, this, nullptr));
}

bool Qt5Font::GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const { return false; }

bool Qt5Font::ImplGetGlyphBoundRect(sal_GlyphId nId, tools::Rectangle& rRect, bool) const
{
    QRawFont aRawFont(QRawFont::fromFont(*this));
    rRect = toRectangle(aRawFont.boundingRect(nId).toAlignedRect());
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
