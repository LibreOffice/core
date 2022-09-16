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

#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>
#include <tools/color.hxx>
#include <tools/long.hxx>
#include <vcl/dllapi.h>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <fontattributes.hxx>
#include <fontsubset.hxx>

#include <hb.h>

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

    RawFontData GetRawFontData(uint32_t) const;

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
    bool CreateFontSubset(std::vector<sal_uInt8>&, const sal_GlyphId*, const sal_uInt8*, const int,
                          FontSubsetInfo&) const;

    bool HasColorLayers() const;
    const ColorPalette& GetColorPalette(size_t) const;
    std::vector<ColorLayer> GetGlyphColorLayers(sal_GlyphId) const;

    virtual hb_face_t* GetHbFace() const;
    virtual hb_blob_t* GetHbTable(hb_tag_t) const
    {
        assert(false);
        return nullptr;
    }

protected:
    mutable hb_face_t* mpHbFace;
    mutable FontCharMapRef mxCharMap;
    mutable vcl::FontCapabilities maFontCapabilities;
    mutable bool mbFontCapabilitiesRead;
    mutable std::vector<ColorPalette> maColorPalettes;

    explicit PhysicalFontFace(const FontAttributes&);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
