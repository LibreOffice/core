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

static QFont::Weight GetQFontWeight(FontWeight eWeight)
{
    switch (eWeight)
    {
        case WEIGHT_THIN:
            return QFont::Thin;
        case WEIGHT_ULTRALIGHT:
            return QFont::ExtraLight;
        case WEIGHT_LIGHT:
            return QFont::Light;
        case FontWeight_FORCE_EQUAL_SIZE:
            assert(false && "FontWeight_FORCE_EQUAL_SIZE not implementable for QFont");
        case WEIGHT_SEMILIGHT:
        case WEIGHT_DONTKNOW:
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
    }

    // so we would get enum not handled warning
    return QFont::Normal;
}

Qt5Font::Qt5Font(const PhysicalFontFace& rPFF, const FontSelectPatternAttributes& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
{
    setFamily(toQString(rPFF.GetFamilyName()));
    setWeight(GetQFontWeight(rPFF.GetWeight()));
    setPixelSize(rFSP.mnHeight);
    switch (rFSP.GetItalic())
    {
        case ITALIC_DONTKNOW:
        case FontItalic_FORCE_EQUAL_SIZE:
            break;
        case ITALIC_NONE:
            setStyle(Style::StyleNormal);
            break;
        case ITALIC_OBLIQUE:
            setStyle(Style::StyleOblique);
            break;
        case ITALIC_NORMAL:
            setStyle(Style::StyleItalic);
            break;
    }
}

Qt5Font::~Qt5Font() {}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
