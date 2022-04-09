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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <o3tl/hash_combine.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/gen.hxx>
#include <tools/fontenum.hxx>
#include <tools/degree.hxx>

#include "font/FontSelectPattern.hxx"
#include "impfontmetricdata.hxx"
#include "glyphid.hxx"

#include <optional>
#include <unordered_map>
#include <memory>

#include <hb.h>

class ConvertChar;
class ImplFontCache;

// extend std namespace to add custom hash needed for LogicalFontInstance

namespace std
{
    template <> struct hash< pair< sal_UCS4, FontWeight > >
    {
        size_t operator()(const pair< sal_UCS4, FontWeight >& rData) const
        {
            std::size_t seed = 0;
            o3tl::hash_combine(seed, rData.first);
            o3tl::hash_combine(seed, rData.second);
            return seed;
        }
    };
}

// TODO: allow sharing of metrics for related fonts

class VCL_PLUGIN_PUBLIC LogicalFontInstance : public salhelper::SimpleReferenceObject
{
    // just declaring the factory function doesn't work AKA
    // friend LogicalFontInstance* PhysicalFontFace::CreateFontInstance(const FontSelectPattern&) const;
    friend class vcl::font::PhysicalFontFace;
    friend class ImplFontCache;

public: // TODO: make data members private
    virtual ~LogicalFontInstance() override;

    ImplFontMetricDataRef mxFontMetric;        // Font attributes
    const ConvertChar* mpConversion;        // used e.g. for StarBats->StarSymbol

    tools::Long            mnLineHeight;
    Degree10        mnOwnOrientation;       // text angle if lower layers don't rotate text themselves
    Degree10        mnOrientation;          // text angle in 3600 system
    bool            mbInit;                 // true if maFontMetric member is valid

    void            AddFallbackForUnicode(sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName,
                                          bool bEmbolden, const ItalicMatrix& rMatrix);
    bool            GetFallbackForUnicode(sal_UCS4 cInChar, FontWeight eInWeight,
                                          OUString* pOutFontName, bool* pOutEmbolden, ItalicMatrix* pOutItalicMatrix) const;
    void            IgnoreFallbackForUnicode( sal_UCS4, FontWeight eWeight, std::u16string_view rFontName );

    inline hb_font_t* GetHbFont();
    bool IsGraphiteFont();
    // NeedOffsetCorrection: Return if the font need offset correction in TTB direction.
    // nYOffset is the original offset. It is used to check if the correction is necessary.
    bool NeedOffsetCorrection(sal_Int32 nYOffset);
    void SetAverageWidthFactor(double nFactor) { m_nAveWidthFactor = std::abs(nFactor); }
    double GetAverageWidthFactor() const { return m_nAveWidthFactor; }
    const vcl::font::FontSelectPattern& GetFontSelectPattern() const { return m_aFontSelData; }

    const vcl::font::PhysicalFontFace* GetFontFace() const { return m_pFontFace.get(); }
    vcl::font::PhysicalFontFace* GetFontFace() { return m_pFontFace.get(); }
    const ImplFontCache* GetFontCache() const { return mpFontCache; }

    bool GetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const;
    virtual bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const = 0;

    int GetKashidaWidth();

    void GetScale(double* nXScale, double* nYScale);
    static inline void DecodeOpenTypeTag(const uint32_t nTableTag, char* pTagName);

protected:
    explicit LogicalFontInstance(const vcl::font::PhysicalFontFace&, const vcl::font::FontSelectPattern&);

    virtual bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const = 0;

    // Takes ownership of pHbFace.
    static hb_font_t* InitHbFont(hb_face_t* pHbFace);
    virtual hb_font_t* ImplInitHbFont() { assert(false); return hb_font_get_empty(); }

private:
    struct MapEntry
    {
        OUString sFontName;
        bool bEmbolden;
        ItalicMatrix aItalicMatrix;
    };
    // cache of Unicode characters and replacement font names and attributes
    // TODO: a fallback map can be shared with many other ImplFontEntries
    // TODO: at least the ones which just differ in orientation, stretching or height
    typedef ::std::unordered_map< ::std::pair<sal_UCS4,FontWeight>, MapEntry > UnicodeFallbackList;
    UnicodeFallbackList maUnicodeFallbackList;
    mutable ImplFontCache * mpFontCache;
    const vcl::font::FontSelectPattern m_aFontSelData;
    hb_font_t* m_pHbFont;
    double m_nAveWidthFactor;
    rtl::Reference<vcl::font::PhysicalFontFace> m_pFontFace;
    std::optional<bool> m_xbIsGraphiteFont;

    enum class FontFamilyEnum { Unclassified, DFKaiSB };

    // The value is initialized and used in NeedOffsetCorrection().
    std::optional<FontFamilyEnum> m_xeFontFamilyEnum;
};

inline hb_font_t* LogicalFontInstance::GetHbFont()
{
    if (!m_pHbFont)
        m_pHbFont = ImplInitHbFont();
    return m_pHbFont;
}

inline void LogicalFontInstance::DecodeOpenTypeTag(const uint32_t nTableTag, char* pTagName)
{
    pTagName[0] = static_cast<char>(nTableTag >> 24);
    pTagName[1] = static_cast<char>(nTableTag >> 16);
    pTagName[2] = static_cast<char>(nTableTag >> 8);
    pTagName[3] = static_cast<char>(nTableTag);
    pTagName[4] = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
