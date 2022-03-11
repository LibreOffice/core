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

/**
A cache for SalLayoutGlyphs objects.

Allows caching for OutputDevice::DrawText() and similar calls. Pass the text and the output device
for the call to OutputDevice::ImplLayout(). Items are cached per output device and its font.
If something more changes, call clear().
*/
class VCL_DLLPUBLIC SalLayoutGlyphsCache final
{
public:
    SalLayoutGlyphsCache(int size = 1000)
        : mCachedGlyphs(size)
    {
    }
    const SalLayoutGlyphs* GetLayoutGlyphs(const OUString& text,
                                           VclPtr<OutputDevice> outputDevice) const;
    void clear() { mCachedGlyphs.clear(); }

private:
    struct CachedGlyphsKey
    {
        OUString text;
        VclPtr<OutputDevice> outputDevice;
        size_t hashValue;
        CachedGlyphsKey(const OUString& t, const VclPtr<OutputDevice>& dev);
        bool operator==(const CachedGlyphsKey& other) const;
    };
    struct CachedGlyphsHash
    {
        size_t operator()(const CachedGlyphsKey& key) const { return key.hashValue; }
    };
    mutable o3tl::lru_map<CachedGlyphsKey, SalLayoutGlyphs, CachedGlyphsHash> mCachedGlyphs;
};

#endif // INCLUDED_VCL_GLYPHITEMCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
