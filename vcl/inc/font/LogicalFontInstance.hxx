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

#include <font/FontSelectPattern.hxx>
#include <font/FontMetricData.hxx>
#include <glyphid.hxx>

#include <optional>
#include <unordered_map>

#include <hb.h>

class ConvertChar;
class ImplFontCache;

constexpr float ARTIFICIAL_ITALIC_MATRIX_XX = 1 << 16;
constexpr float ARTIFICIAL_ITALIC_MATRIX_XY = (1 << 16) / 3.f;
constexpr float ARTIFICIAL_ITALIC_SKEW = ARTIFICIAL_ITALIC_MATRIX_XY / ARTIFICIAL_ITALIC_MATRIX_XX;

// extend std namespace to add custom hash needed for LogicalFontInstance

namespace std
{
template <> struct hash<pair<sal_UCS4, FontWeight>>
{
    size_t operator()(const pair<sal_UCS4, FontWeight>& rData) const
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

    FontMetricDataRef mxFontMetric; // Font attributes
    const ConvertChar* mpConversion; // used e.g. for StarBats->StarSymbol

    tools::Long mnLineHeight;
    Degree10 mnOwnOrientation; // text angle if lower layers don't rotate text themselves
    Degree10 mnOrientation; // text angle in 3600 system
    bool mbInit; // true if maFontMetric member is valid

    SAL_DLLPRIVATE void AddFallbackForUnicode(sal_UCS4 cChar, FontWeight eWeight,
                                              const OUString& rFontName, bool bEmbolden,
                                              const ItalicMatrix& rMatrix);
    SAL_DLLPRIVATE bool GetFallbackForUnicode(sal_UCS4 cInChar, FontWeight eInWeight,
                                              OUString* pOutFontName, bool* pOutEmbolden,
                                              ItalicMatrix* pOutItalicMatrix) const;
    SAL_DLLPRIVATE void IgnoreFallbackForUnicode(sal_UCS4, FontWeight eWeight,
                                                 std::u16string_view rFontName);

    inline hb_font_t* GetHbFont();
    SAL_DLLPRIVATE bool IsGraphiteFont();
    // NeedOffsetCorrection: Return if the font need offset correction in TTB direction.
    // nYOffset is the original offset. It is used to check if the correction is necessary.
    SAL_DLLPRIVATE bool NeedOffsetCorrection(sal_Int32 nYOffset);
    void SetAverageWidthFactor(double nFactor) { m_nAveWidthFactor = std::abs(nFactor); }
    double GetAverageWidthFactor() const { return m_nAveWidthFactor; }
    const vcl::font::FontSelectPattern& GetFontSelectPattern() const { return m_aFontSelData; }

    const vcl::font::PhysicalFontFace* GetFontFace() const { return m_pFontFace.get(); }
    vcl::font::PhysicalFontFace* GetFontFace() { return m_pFontFace.get(); }
    const ImplFontCache* GetFontCache() const { return mpFontCache; }

    bool GetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const;
    virtual bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const = 0;
    SAL_DLLPRIVATE basegfx::B2DPolyPolygon GetGlyphOutlineUntransformed(sal_GlyphId) const;

    sal_GlyphId GetGlyphIndex(uint32_t, uint32_t = 0) const;

    SAL_DLLPRIVATE double GetGlyphWidth(sal_GlyphId, bool = false, bool = true) const;

    double GetKashidaWidth() const;

    SAL_DLLPRIVATE void GetScale(double* nXScale, double* nYScale) const;

    bool NeedsArtificialItalic() const;
    bool NeedsArtificialBold() const;

protected:
    explicit LogicalFontInstance(const vcl::font::PhysicalFontFace&,
                                 const vcl::font::FontSelectPattern&);

    SAL_DLLPRIVATE hb_font_t* InitHbFont();
    virtual void ImplInitHbFont(hb_font_t*) {}

private:
    SAL_DLLPRIVATE hb_font_t* GetHbFontUntransformed() const;

    struct MapEntry
    {
        OUString sFontName;
        bool bEmbolden;
        ItalicMatrix aItalicMatrix;
    };
    // cache of Unicode characters and replacement font names and attributes
    // TODO: a fallback map can be shared with many other ImplFontEntries
    // TODO: at least the ones which just differ in orientation, stretching or height
    typedef ::std::unordered_map<::std::pair<sal_UCS4, FontWeight>, MapEntry> UnicodeFallbackList;
    UnicodeFallbackList maUnicodeFallbackList;
    mutable ImplFontCache* mpFontCache;
    const vcl::font::FontSelectPattern m_aFontSelData;
    hb_font_t* m_pHbFont;
    mutable hb_font_t* m_pHbFontUntransformed = nullptr;
    double m_nAveWidthFactor;
    rtl::Reference<vcl::font::PhysicalFontFace> m_pFontFace;
    std::optional<bool> m_xbIsGraphiteFont;

    enum class FontFamilyEnum
    {
        Unclassified,
        DFKaiSB
    };

    // The value is initialized and used in NeedOffsetCorrection().
    std::optional<FontFamilyEnum> m_xeFontFamilyEnum;

    mutable hb_draw_funcs_t* m_pHbDrawFuncs = nullptr;
    basegfx::B2DPolygon m_aDrawPolygon;
};

inline hb_font_t* LogicalFontInstance::GetHbFont()
{
    if (!m_pHbFont)
        m_pHbFont = InitHbFont();
    return m_pHbFont;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
