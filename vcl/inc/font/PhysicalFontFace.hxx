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

#pragma once

#include <sal/config.h>

#include <i18nlangtag/languagetag.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/color.hxx>
#include <vcl/dllapi.h>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <fontattributes.hxx>
#include <fontsubset.hxx>

#include <hb.h>
#include <hb-ot.h>

class LogicalFontInstance;
struct FontMatchStatus;
namespace vcl::font
{
class FontSelectPattern;
}

namespace vcl
{
class PhysicalFontFamily;
}

namespace vcl::font
{
class FontSelectPattern;

struct FontMatchStatus
{
public:
    int mnFaceMatch;
    const OUString* mpTargetStyleName;
};

struct RawFontData
{
public:
    RawFontData(hb_blob_t* pBlob = nullptr)
        : mpBlob(pBlob ? pBlob : hb_blob_get_empty())
    {
    }

    RawFontData(const RawFontData& rOther)
        : mpBlob(hb_blob_reference(rOther.mpBlob))
    {
    }

    ~RawFontData() { hb_blob_destroy(mpBlob); }

    RawFontData& operator=(const RawFontData& rOther)
    {
        hb_blob_destroy(mpBlob);
        mpBlob = hb_blob_reference(rOther.mpBlob);
        return *this;
    }

    size_t size() const { return hb_blob_get_length(mpBlob); }
    bool empty() const { return size() == 0; }
    const uint8_t* data() const
    {
        return reinterpret_cast<const uint8_t*>(hb_blob_get_data(mpBlob, nullptr));
    }

private:
    hb_blob_t* mpBlob;
};

struct ColorLayer
{
    sal_GlyphId nGlyphIndex;
    uint32_t nColorIndex;
};

typedef std::vector<Color> ColorPalette;

// https://learn.microsoft.com/en-us/typography/opentype/spec/name#name-ids
typedef enum : hb_ot_name_id_t {
    NAME_ID_COPYRIGHT = 0,
    NAME_ID_FONT_FAMILY = 1,
    NAME_ID_FONT_SUBFAMILY = 2,
    NAME_ID_UNIQUE_ID = 3,
    NAME_ID_FULL_NAME = 4,
    NAME_ID_VERSION_STRING = 5,
    NAME_ID_POSTSCRIPT_NAME = 6,
    NAME_ID_TRADEMARK = 7,
    NAME_ID_MANUFACTURER = 8,
    NAME_ID_DESIGNER = 9,
    NAME_ID_DESCRIPTION = 10,
    NAME_ID_VENDOR_URL = 11,
    NAME_ID_DESIGNER_URL = 12,
    NAME_ID_LICENSE = 13,
    NAME_ID_LICENSE_URL = 14,
    //NAME_ID_RESERVED = 15,
    NAME_ID_TYPOGRAPHIC_FAMILY = 16,
    NAME_ID_TYPOGRAPHIC_SUBFAMILY = 17,
    NAME_ID_MAC_FULL_NAME = 18,
    NAME_ID_SAMPLE_TEXT = 19,
    NAME_ID_CID_FINDFONT_NAME = 20,
    NAME_ID_WWS_FAMILY = 21,
    NAME_ID_WWS_SUBFAMILY = 22,
    NAME_ID_LIGHT_BACKGROUND = 23,
    NAME_ID_DARK_BACKGROUND = 24,
    NAME_ID_VARIATIONS_PS_PREFIX = 25,
} NameID;

// TODO: no more direct access to members
// TODO: get rid of height/width for scalable fonts
// TODO: make cloning cheaper

/**
 * abstract base class for physical font faces
 *
 * It acts as a factory for its corresponding LogicalFontInstances and
 * can be extended to cache device and font instance specific data.
 */
class VCL_PLUGIN_PUBLIC PhysicalFontFace : public FontAttributes,
                                           public salhelper::SimpleReferenceObject
{
public:
    ~PhysicalFontFace();

    virtual rtl::Reference<LogicalFontInstance>
    CreateFontInstance(const vcl::font::FontSelectPattern&) const = 0;

    virtual sal_IntPtr GetFontId() const = 0;
    virtual FontCharMapRef GetFontCharMap() const;
    virtual bool GetFontCapabilities(vcl::FontCapabilities&) const;

    SAL_DLLPRIVATE RawFontData GetRawFontData(uint32_t) const;

    bool IsBetterMatch(const vcl::font::FontSelectPattern&, FontMatchStatus&) const;
    sal_Int32 CompareIgnoreSize(const PhysicalFontFace&) const;

    // CreateFontSubset: a method to get a subset of glyphs of a font inside a
    // new valid font file
    // returns true if creation of subset was successful
    // parameters: rOutBuffer: vector to write the subset to
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    SAL_DLLPRIVATE bool CreateFontSubset(std::vector<sal_uInt8>&, const sal_GlyphId*,
                                         const sal_uInt8*, const int, FontSubsetInfo&) const;

    bool IsColorFont() const { return HasColorLayers() || HasColorBitmaps(); }

    bool HasColorLayers() const;
    SAL_DLLPRIVATE std::vector<ColorLayer> GetGlyphColorLayers(sal_GlyphId) const;

    SAL_DLLPRIVATE const std::vector<ColorPalette>& GetColorPalettes() const;

    bool HasColorBitmaps() const;
    SAL_DLLPRIVATE RawFontData GetGlyphColorBitmap(sal_GlyphId, tools::Rectangle&) const;

    OString GetGlyphName(sal_GlyphId, bool = false) const;

    uint32_t UnitsPerEm() const { return hb_face_get_upem(GetHbFace()); }

    OUString GetName(NameID, const LanguageTag&) const;
    OUString GetName(NameID aNameID) const { return GetName(aNameID, LanguageTag(LANGUAGE_NONE)); }

    virtual hb_face_t* GetHbFace() const;
    virtual hb_blob_t* GetHbTable(hb_tag_t) const
    {
        assert(false);
        return nullptr;
    }

    virtual const std::vector<hb_variation_t>& GetVariations(const LogicalFontInstance&) const;

protected:
    mutable hb_face_t* mpHbFace;
    mutable hb_font_t* mpHbUnscaledFont;
    mutable FontCharMapRef mxCharMap;
    mutable std::optional<vcl::FontCapabilities> mxFontCapabilities;
    mutable std::optional<std::vector<ColorPalette>> mxColorPalettes;
    mutable std::optional<std::vector<hb_variation_t>> mxVariations;

    explicit PhysicalFontFace(const FontAttributes&);

    SAL_DLLPRIVATE hb_font_t* GetHbUnscaledFont() const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
