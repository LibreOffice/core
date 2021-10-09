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
#include <rtl/ustring.hxx>
#include <unotools/fontdefs.hxx>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/degree.hxx>
#include <tools/gen.hxx>
#include <tools/fontenum.hxx>
#include <tools/degree.hxx>

#include <vcl/glyphitem.hxx>

#include "FontInstanceData.hxx"
#include "FontSelectPattern.hxx"
#include "PhysicalFontFace.hxx"

#include <optional>
#include <unordered_map>
#include <memory>

#include <hb.h>

class ConvertChar;
class ImplFontCache;

namespace vcl::font
{
class PhysicalFontFace;

// TODO: allow sharing of metrics for related fonts

class VCL_PLUGIN_PUBLIC LogicalFontInstance : public salhelper::SimpleReferenceObject
{
    // just declaring the factory function doesn't work AKA
    // friend LogicalFontInstance* PhysicalFontFace::CreateFontInstance(const FontSelectPattern&) const;
    friend class PhysicalFontFace;
    friend class ::ImplFontCache;

public: // TODO: make data members private
    virtual ~LogicalFontInstance() override;

    OUString const& GetStyleName() const { return mxFontMetric->GetStyleName(); }
    tools::Long GetWidth() const { return mxFontMetric->GetWidth(); }
    tools::Long GetDescent() const{ return mxFontMetric->GetDescent(); }
    tools::Long GetAscent() const{ return mxFontMetric->GetAscent(); }
    tools::Long GetUnderlineSize()  const{ return mxFontMetric->GetUnderlineSize(); }
    tools::Long GetUnderlineOffset() const{ return mxFontMetric->GetUnderlineOffset(); }
    tools::Long GetAboveWavelineUnderlineSize() const{ return mxFontMetric->GetAboveWavelineUnderlineSize(); }
    tools::Long GetAboveWavelineUnderlineOffset() const{ return mxFontMetric->GetAboveWavelineUnderlineOffset(); }
    tools::Long GetWavelineUnderlineSize() const{ return mxFontMetric->GetWavelineUnderlineSize(); }
    tools::Long GetWavelineUnderlineOffset() const{ return mxFontMetric->GetWavelineUnderlineOffset(); }
    tools::Long GetAboveUnderlineSize() const { return mxFontMetric->GetAboveUnderlineSize(); }
    tools::Long GetAboveUnderlineOffset() const { return mxFontMetric->GetAboveUnderlineOffset(); }
    tools::Long GetBoldUnderlineSize() const { return mxFontMetric->GetBoldUnderlineSize(); }
    tools::Long GetBoldUnderlineOffset() const { return mxFontMetric->GetBoldUnderlineOffset(); }
    tools::Long GetAboveBoldUnderlineSize() const { return mxFontMetric->GetAboveBoldUnderlineSize(); }
    tools::Long GetAboveBoldUnderlineOffset() const { return mxFontMetric->GetAboveBoldUnderlineOffset(); }
    tools::Long GetDoubleUnderlineSize() const { return mxFontMetric->GetDoubleUnderlineSize(); }
    tools::Long GetDoubleUnderlineOffset1() const { return mxFontMetric->GetDoubleUnderlineOffset1(); }
    tools::Long GetDoubleUnderlineOffset2() const { return mxFontMetric->GetDoubleUnderlineOffset2(); }
    tools::Long GetAboveDoubleUnderlineSize() const { return mxFontMetric->GetAboveDoubleUnderlineSize(); }
    tools::Long GetAboveDoubleUnderlineOffset1() const { return mxFontMetric->GetAboveDoubleUnderlineOffset1(); }
    tools::Long GetAboveDoubleUnderlineOffset2() const { return mxFontMetric->GetAboveDoubleUnderlineOffset2(); }
    tools::Long GetStrikeoutSize() const { return mxFontMetric->GetStrikeoutSize(); }
    tools::Long GetStrikeoutOffset() const { return mxFontMetric->GetStrikeoutOffset(); }
    tools::Long GetBoldStrikeoutSize() const { return mxFontMetric->GetBoldStrikeoutSize(); }
    tools::Long GetBoldStrikeoutOffset() const { return mxFontMetric->GetBoldStrikeoutOffset(); }
    tools::Long GetDoubleStrikeoutSize() const { return mxFontMetric->GetDoubleStrikeoutSize(); }
    tools::Long GetDoubleStrikeoutOffset1() const { return mxFontMetric->GetDoubleStrikeoutOffset1(); }
    tools::Long GetDoubleStrikeoutOffset2() const { return mxFontMetric->GetDoubleStrikeoutOffset2(); }
    tools::Long GetInternalLeading() const { return mxFontMetric->GetInternalLeading(); }
    tools::Long GetExternalLeading() const{ return mxFontMetric->GetExternalLeading(); }
    tools::Long GetMinKashida() const { return mxFontMetric->GetMinKashida(); }
    tools::Long GetBulletOffset() const { return mxFontMetric->GetBulletOffset(); }
    tools::Long GetSlant() const { return mxFontMetric->GetSlant(); }
    int GetQuality() const { return mxFontMetric->GetQuality(); }
    FontFamily GetFamilyType() const { return mxFontMetric->GetFamilyType(); }
    FontPitch GetPitch() const { return mxFontMetric->GetPitch(); }
    FontWeight GetWeight() const { return mxFontMetric->GetWeight(); }
    FontItalic GetItalic() const { return mxFontMetric->GetItalic(); }
    FontWidth GetWidthType() const { return mxFontMetric->GetWidthType(); }
    bool IsSymbolFont() const { return mxFontMetric->IsSymbolFont(); }
    bool IsFullstopCentered() const { return mxFontMetric->IsFullstopCentered(); }

    tools::Long GetLineHeight() const { return mnLineHeight; }
    Degree10 GetOwnOrientation() const { return mnOwnOrientation; }
    Degree10 GetOrientationFromData() const { return mxFontMetric->GetOrientation(); }
    bool IsInit() const { return mbInit; }
    void SetLineHeight(tools::Long nLineHeight) { mnLineHeight = nLineHeight; }
    void SetOwnOrientation(Degree10 nOwnOrientation) { mnOwnOrientation = nOwnOrientation; }
    void SetOrientationInData(Degree10 nOrientation) { mnOrientation = nOrientation; }
    void SetInitFlag(bool bIsInit) { mbInit = bIsInit; }

    FontCharMapRef GetFontCharMap() { return m_pFontFace->GetFontCharMap(); }
    bool GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) { return m_pFontFace->GetFontCapabilities(rFontCapabilities); }
    void SetFontOrientation();

    void InitTextLineSize(sal_Int32 nDPIY, vcl::Font const& rFont, sal_Int32 mnBulletOffset);
    void InitAboveTextLineSize() { mxFontMetric->InitAboveTextLineSize(); }
    void InitFlags(vcl::Font const& rFont, tools::Rectangle const& rRect) { mxFontMetric->InitFlags(rFont, rRect); }

    FontInstanceDataRef GetFontInstanceData() { return mxFontMetric; }

    // Conversion functions
    bool CanConvertChars() { return (mpConversion ? true : false); }
    void SetCharConversion(ConvertChar const* pConversion) { mpConversion = pConversion; }
    void RecodeString(OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen) { mpConversion->RecodeString(rStr, nIndex, nLen); }

    void            AddFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );
    bool            GetFallbackForUnicode( sal_UCS4, FontWeight eWeight, OUString* pFontName ) const;
    void            IgnoreFallbackForUnicode( sal_UCS4, FontWeight eWeight, std::u16string_view rFontName );

    inline hb_font_t* GetHbFont();
    bool IsGraphiteFont();
    const FontSelectPattern& GetFontSelectPattern() const { return m_aFontSelData; }

    const PhysicalFontFace* GetFontFace() const { return m_pFontFace.get(); }
    PhysicalFontFace* GetFontFace() { return m_pFontFace.get(); }

    bool GetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const;
    virtual bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const = 0;

    int GetKashidaWidth();

    void GetScale(double* nXScale, double* nYScale);
    static inline void DecodeOpenTypeTag(const uint32_t nTableTag, char* pTagName);

protected:
    explicit LogicalFontInstance(const PhysicalFontFace&, const FontSelectPattern&);

    void SetAverageWidthFactor(double nFactor) { m_nAveWidthFactor = std::abs(nFactor); }
    double GetAverageWidthFactor() const { return m_nAveWidthFactor; }

    virtual bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const = 0;

    // Takes ownership of pHbFace.
    static hb_font_t* InitHbFont(hb_face_t* pHbFace);
    virtual hb_font_t* ImplInitHbFont() { assert(false); return hb_font_get_empty(); }

private:
    FontInstanceDataRef mxFontMetric;        // Font attributes
    const ConvertChar* mpConversion;        // used e.g. for StarBats->StarSymbol

    // cache of Unicode characters and replacement font names
    // TODO: a fallback map can be shared with many other ImplFontEntries
    // TODO: at least the ones which just differ in orientation, stretching or height
    typedef ::std::unordered_map< ::std::pair<sal_UCS4,FontWeight>, OUString > UnicodeFallbackList;
    std::unique_ptr<UnicodeFallbackList> mpUnicodeFallbackList;
    mutable ImplFontCache * mpFontCache;
    const FontSelectPattern m_aFontSelData;
    hb_font_t* m_pHbFont;
    double m_nAveWidthFactor;
    rtl::Reference<PhysicalFontFace> m_pFontFace;
    std::optional<bool> m_xbIsGraphiteFont;

    tools::Long mnLineHeight;
    Degree10 mnOwnOrientation; // text angle if lower layers don't rotate text themselves
    Degree10 mnOrientation; // text angle in 3600 system
    bool mbInit; // true if maFontMetric member is valid
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
