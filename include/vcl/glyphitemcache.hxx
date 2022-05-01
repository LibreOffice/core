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
#include <o3tl/hash_combine.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>
#include <tools/gen.hxx>

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
    const SalLayoutGlyphs* GetLayoutGlyphs(VclPtr<const OutputDevice> outputDevice,
                                           const OUString& text,
                                           const vcl::text::TextLayoutCache* layoutCache = nullptr)
    {
        return GetLayoutGlyphs(outputDevice, text, 0, text.getLength(), 0, layoutCache);
    }
    const SalLayoutGlyphs* GetLayoutGlyphs(VclPtr<const OutputDevice> outputDevice,
                                           const OUString& text, sal_Int32 nIndex, sal_Int32 nLen,
                                           tools::Long nLogicWidth = 0,
                                           const vcl::text::TextLayoutCache* layoutCache = nullptr);
    void clear() { mCachedGlyphs.clear(); }

    static SalLayoutGlyphsCache* self();
    SalLayoutGlyphsCache(int size) // needs to be public for vcl::DeleteOnDeinit
        : mCachedGlyphs(size)
    {
    }

private:
    struct CachedGlyphsKey
    {
        OUString text;
        sal_Int32 index;
        sal_Int32 len;
        tools::Long logicWidth;
        vcl::Font font;
        double fontScaleX;
        double fontScaleY;
        MapMode mapMode;
        bool rtl;
        vcl::text::ComplexTextLayoutFlags layoutMode;
        LanguageType digitLanguage;
        size_t hashValue;
        CachedGlyphsKey(const VclPtr<const OutputDevice>& dev, const OUString& t, sal_Int32 i,
                        sal_Int32 l, tools::Long w);
        bool operator==(const CachedGlyphsKey& other) const;
    };
    struct CachedGlyphsHash
    {
        size_t operator()(const CachedGlyphsKey& key) const { return key.hashValue; }
    };
    struct GlyphsCost
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

    SalLayoutGlyphsCache(const SalLayoutGlyphsCache&) = delete;
    SalLayoutGlyphsCache& operator=(const SalLayoutGlyphsCache&) = delete;
};

#endif // INCLUDED_VCL_GLYPHITEMCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
