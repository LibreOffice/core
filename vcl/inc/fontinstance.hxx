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

#ifndef INCLUDED_VCL_INC_FONTINSTANCE_HXX
#define INCLUDED_VCL_INC_FONTINSTANCE_HXX

#include "fontselect.hxx"
#include "impfontmetricdata.hxx"

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/gen.hxx>
#include <tools/fontenum.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/vcllayout.hxx>

#include <unordered_map>
#include <memory>

#include <hb-ot.h>

class ConvertChar;
class ImplFontCache;
class PhysicalFontFace;

// TODO: allow sharing of metrics for related fonts

class VCL_PLUGIN_PUBLIC LogicalFontInstance : public salhelper::SimpleReferenceObject
{
    // just declaring the factory function doesn't work AKA
    // friend LogicalFontInstance* PhysicalFontFace::CreateFontInstance(const FontSelectPattern&) const;
    friend class PhysicalFontFace;
    friend class ImplFontCache;

public: // TODO: make data members private
    virtual ~LogicalFontInstance() override;

    ImplFontMetricDataRef mxFontMetric;        // Font attributes
    const ConvertChar* mpConversion;        // used e.g. for StarBats->StarSymbol

    long            mnLineHeight;
    short           mnOwnOrientation;       // text angle if lower layers don't rotate text themselves
    short           mnOrientation;          // text angle in 3600 system
    bool            mbInit;                 // true if maFontMetric member is valid

    void            AddFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );
    bool            GetFallbackForUnicode( sal_UCS4, FontWeight eWeight, OUString* pFontName ) const;
    void            IgnoreFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );

    inline hb_font_t* GetHbFont();
    void SetAverageWidthFactor(double nFactor) { m_nAveWidthFactor = nFactor; }
    double GetAverageWidthFactor() const { return m_nAveWidthFactor; }
    const FontSelectPattern& GetFontSelectPattern() const { return m_aFontSelData; }

    const PhysicalFontFace* GetFontFace() const { return m_pFontFace.get(); }
    const ImplFontCache* GetFontCache() const { return mpFontCache; }

    bool GetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const;
    virtual bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const = 0;

    int GetKashidaWidth();

    void GetScale(double* nXScale, double* nYScale);
    static inline void DecodeOpenTypeTag(const uint32_t nTableTag, char* pTagName);

protected:
    explicit LogicalFontInstance(const PhysicalFontFace&, const FontSelectPattern&);

    virtual bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const = 0;

    // Takes ownership of pHbFace.
    static hb_font_t* InitHbFont(hb_face_t* pHbFace);
    virtual hb_font_t* ImplInitHbFont() { assert(false); return nullptr; }
    inline void ReleaseHbFont();

private:
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
};

inline hb_font_t* LogicalFontInstance::GetHbFont()
{
    if (!m_pHbFont)
        m_pHbFont = ImplInitHbFont();
    return m_pHbFont;
}

inline void LogicalFontInstance::ReleaseHbFont()
{
    if (!m_pHbFont)
        return;
    hb_font_destroy(m_pHbFont);
    m_pHbFont = nullptr;
}

inline void LogicalFontInstance::DecodeOpenTypeTag(const uint32_t nTableTag, char* pTagName)
{
    pTagName[0] = static_cast<char>(nTableTag >> 24);
    pTagName[1] = static_cast<char>(nTableTag >> 16);
    pTagName[2] = static_cast<char>(nTableTag >> 8);
    pTagName[3] = static_cast<char>(nTableTag);
    pTagName[4] = 0;
}

#endif // INCLUDED_VCL_INC_FONTINSTANCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
