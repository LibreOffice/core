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

#ifndef INCLUDED_VCL_GLYPHITEMCACHE_HXX
#define INCLUDED_VCL_GLYPHITEMCACHE_HXX

#include <sal/types.h>
#include <vcl/dllapi.h>

#include <o3tl/lru_map.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>

#include <optional>

/**
A cache for SalLayoutGlyphs objects.

Allows caching for OutputDevice::DrawText() and similar calls. Pass the text and the output device
for the call to OutputDevice::ImplLayout(). Items are cached per output device and its font.
If something more changes, call clear().
*/
class VCL_DLLPUBLIC SalLayoutGlyphsCache final
{
public:
    // NOTE: The lifetime of the returned value is guaranteed only until the next call
    // to any function in this class.
    const SalLayoutGlyphs* GetLayoutGlyphs(const VclPtr<const OutputDevice>& outputDevice,
                                           const OUString& text,
                                           const vcl::text::TextLayoutCache* layoutCache = nullptr)
    {
        return GetLayoutGlyphs(outputDevice, text, 0, text.getLength(), 0, layoutCache);
    }
    const SalLayoutGlyphs* GetLayoutGlyphs(const VclPtr<const OutputDevice>& outputDevice,
                                           const OUString& text, sal_Int32 nIndex, sal_Int32 nLen,
                                           tools::Long nLogicWidth = 0,
                                           const vcl::text::TextLayoutCache* layoutCache = nullptr);
    const SalLayoutGlyphs* GetLayoutGlyphs(const VclPtr<const OutputDevice>& outputDevice,
                                           const OUString& text, sal_Int32 nIndex, sal_Int32 nLen,
                                           sal_Int32 nDrawMinCharPos, sal_Int32 nDrawEndCharPos,
                                           tools::Long nLogicWidth = 0,
                                           const vcl::text::TextLayoutCache* layoutCache = nullptr);
    void clear();

    /// Normally, we cannot cache glyphs when doing font fallback, because the font fallbacks
    /// can cache during the lifetime of the cache, and they are not included in the cache key.
    /// But during some processes, we can turn this on, as long as we remember to turn it off
    /// at the end.
    void SetCacheGlyphsWhenDoingFallbackFonts(bool bOK);

    static SalLayoutGlyphsCache* self();
    SalLayoutGlyphsCache(int size) // needs to be public for vcl::DeleteOnDeinit
        : mCachedGlyphs(size)
    {
    }

private:
    struct SAL_DLLPRIVATE CachedGlyphsKey
    {
        OUString text;
        sal_Int32 index;
        sal_Int32 len;
        tools::Long logicWidth;
        FontMetric fontMetric;
        double fontScaleX;
        double fontScaleY;
        MapMode mapMode;
        LanguageType digitLanguage;
        vcl::text::ComplexTextLayoutFlags layoutMode;
        bool rtl : 1;
        bool disabledLigatures : 1; // because of fixed pitch
        bool artificialItalic : 1;
        bool artificialBold : 1;
        size_t hashValue;
        CachedGlyphsKey(const VclPtr<const OutputDevice>& dev, OUString t, sal_Int32 i, sal_Int32 l,
                        tools::Long w);
        bool operator==(const CachedGlyphsKey& other) const;
    };
    struct CachedGlyphsHash
    {
        size_t operator()(const CachedGlyphsKey& key) const { return key.hashValue; }
    };
    struct SAL_DLLPRIVATE GlyphsCost
    {
        size_t operator()(const SalLayoutGlyphs&) const;
    };
    typedef o3tl::lru_map<CachedGlyphsKey, SalLayoutGlyphs, CachedGlyphsHash,
                          std::equal_to<CachedGlyphsKey>, GlyphsCost>
        GlyphsCache;
    GlyphsCache mCachedGlyphs;
    // Last temporary glyphs returned (pointer is returned, so the object needs to be kept somewhere).
    std::optional<CachedGlyphsKey> mLastTemporaryKey;
    SalLayoutGlyphs mLastTemporaryGlyphs;
    // If set, info about the last call which wanted a substring of the full text.
    std::optional<CachedGlyphsKey> mLastSubstringKey;
    bool mbCacheGlyphsWhenDoingFallbackFonts = false;

    SalLayoutGlyphsCache(const SalLayoutGlyphsCache&) = delete;
    SalLayoutGlyphsCache& operator=(const SalLayoutGlyphsCache&) = delete;
};

#endif // INCLUDED_VCL_GLYPHITEMCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
