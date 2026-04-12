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

/*
 * Sun Font Tools
 *
 * Author: Alexander Gelfenbain
 *
 */

#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <hb-ot.h>
#include <font/CFFCharset.hxx>
#include <font/TrueTypeFont.hxx>
#include <font/TTFStructure.hxx>
#ifdef SYSTEM_LIBFIXMATH
#include <libfixmath/fix16.hpp>
#else
#include <tools/fix16.hxx>
#endif
#include <i18nlangtag/languagetag.hxx>
#include <rtl/crc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/stream.hxx>
#include <o3tl/safeint.hxx>
#include <osl/endian.h>
#include <unotools/tempfile.hxx>
#include <fontsubset.hxx>

namespace vcl
{
/*- Data access methods for data stored in big-endian format */
static sal_uInt16 GetUInt16(const sal_uInt8* ptr, size_t offset)
{
    sal_uInt16 t;
    assert(ptr != nullptr);

    t = (ptr + offset)[0] << 8 | (ptr + offset)[1];

    return t;
}

static sal_uInt32 GetUInt32(const sal_uInt8* ptr, size_t offset)
{
    sal_uInt32 t;
    assert(ptr != nullptr);

    t = (ptr + offset)[0] << 24 | (ptr + offset)[1] << 16 | (ptr + offset)[2] << 8
        | (ptr + offset)[3];

    return t;
}

/*- Public functions */

TrueTypeFont::TrueTypeFont(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum)
{
    hb_blob_t* pBlob = hb_blob_create(static_cast<const char*>(pBuffer), nLen,
                                      HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    m_pFace = hb_face_create(pBlob, facenum);
    hb_blob_destroy(pBlob);
}

TrueTypeFont::~TrueTypeFont()
{
    hb_face_destroy(m_pFace);
    hb_font_destroy(m_pFont);
}

hb_font_t* TrueTypeFont::getFont() const
{
    if (!m_pFont)
        m_pFont = hb_font_create(m_pFace);
    return m_pFont;
}

font::RawFontData TrueTypeFont::getTable(hb_tag_t tag) const
{
    return font::RawFontData(hb_face_reference_table(m_pFace, tag));
}

sal_uInt32 TrueTypeFont::countNonEmptyGlyphs() const
{
    hb_font_t* pFont = getFont();
    sal_uInt32 nGlyphs = hb_face_get_glyph_count(m_pFace);
    sal_uInt32 nCount = 0;
    for (sal_uInt32 i = 0; i < nGlyphs; ++i)
    {
        hb_glyph_extents_t aExtents;
        if (hb_font_get_glyph_extents(pFont, i, &aExtents) && (aExtents.width || aExtents.height))
            ++nCount;
    }
    return nCount;
}

OUString TrueTypeFont::getName(hb_ot_name_id_t nNameID, const LanguageTag& rLang) const
{
    hb_language_t aHbLang = HB_LANGUAGE_INVALID;
    if (rLang.getLanguageType() != LANGUAGE_DONTKNOW)
    {
        auto aLanguage(rLang.getBcp47().toUtf8());
        aHbLang = hb_language_from_string(aLanguage.getStr(), aLanguage.getLength());
    }

    auto nName = hb_ot_name_get_utf16(m_pFace, nNameID, aHbLang, nullptr, nullptr);
    if (!nName)
        return OUString();
    std::vector<uint16_t> aBuf(++nName); // make space for terminating NUL
    hb_ot_name_get_utf16(m_pFace, nNameID, aHbLang, &nName, aBuf.data());
    return OUString(reinterpret_cast<sal_Unicode*>(aBuf.data()), nName);
}

OUString TrueTypeFont::getFamilyName() const
{
    OUString sFamily = getName(HB_OT_NAME_ID_FONT_FAMILY);
    if (sFamily.isEmpty())
        sFamily = getName(HB_OT_NAME_ID_POSTSCRIPT_NAME);
    return sFamily;
}

OUString TrueTypeFont::getTypographicFamilyName() const
{
    OUString sFamily = getName(HB_OT_NAME_ID_TYPOGRAPHIC_FAMILY);
    if (sFamily.isEmpty())
        sFamily = getFamilyName();
    return sFamily;
}

OUString TrueTypeFont::getSubfamilyName() const { return getName(HB_OT_NAME_ID_FONT_SUBFAMILY); }

sal_uInt32 TrueTypeFont::getTypeFlags() const
{
    auto aOS2 = getTable(T_OS2);
    if (aOS2.size() >= 42)
        return GetUInt16(aOS2.data(), OS2_fsType_offset);
    return 0;
}

FontPitch TrueTypeFont::getFontPitch() const
{
    auto aPost = getTable(T_post);
    if (aPost.size() >= 12 + sizeof(sal_uInt32))
        return GetUInt32(aPost.data(), POST_isFixedPitch_offset) ? PITCH_FIXED : PITCH_VARIABLE;
    return PITCH_VARIABLE;
}

FontItalic TrueTypeFont::getFontItalic() const
{
    return hb_style_get_value(getFont(), HB_STYLE_TAG_SLANT_ANGLE) != 0 ? ITALIC_NORMAL
                                                                        : ITALIC_NONE;
}

FontWidth TrueTypeFont::getFontWidth() const
{
    float fWidth = hb_style_get_value(getFont(), HB_STYLE_TAG_WIDTH);
    if (fWidth <= 50)
        return WIDTH_ULTRA_CONDENSED;
    if (fWidth <= 62.5)
        return WIDTH_EXTRA_CONDENSED;
    if (fWidth <= 75)
        return WIDTH_CONDENSED;
    if (fWidth <= 87.5)
        return WIDTH_SEMI_CONDENSED;
    if (fWidth <= 100)
        return WIDTH_NORMAL;
    if (fWidth <= 112.5)
        return WIDTH_SEMI_EXPANDED;
    if (fWidth <= 125)
        return WIDTH_EXPANDED;
    if (fWidth <= 150)
        return WIDTH_EXTRA_EXPANDED;
    return WIDTH_ULTRA_EXPANDED;
}

FontWeight TrueTypeFont::getFontWeight() const
{
    float fWeight = hb_style_get_value(getFont(), HB_STYLE_TAG_WEIGHT);
    if (fWeight <= 100)
        return WEIGHT_THIN;
    if (fWeight <= 200)
        return WEIGHT_ULTRALIGHT;
    if (fWeight <= 300)
        return WEIGHT_LIGHT;
    if (fWeight < 500)
        return WEIGHT_NORMAL;
    if (fWeight == 500)
        return WEIGHT_MEDIUM;
    if (fWeight <= 600)
        return WEIGHT_SEMIBOLD;
    if (fWeight <= 700)
        return WEIGHT_BOLD;
    if (fWeight <= 800)
        return WEIGHT_ULTRABOLD;
    return WEIGHT_BLACK;
}

} // namespace vcl

int TestFontParsing(const void* data, sal_uInt32 size)
{
    // Exercise CFF parsing
    if (data && size > 0)
    {
        std::vector<sal_uInt16> aCIDs;
        vcl::font::ReadCFFGlyphCIDs(static_cast<const sal_uInt8*>(data), size, aCIDs);
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
