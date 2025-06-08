/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <TextLayoutCache.hxx>

#include <scrptrun.h>

#include <o3tl/hash_combine.hxx>
#include <o3tl/lru_map.hxx>
#include <rtl/strbuf.hxx>
#include <unotools/configmgr.hxx>
#include <tools/lazydelete.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/dropcache.hxx>

#include <memory>

namespace vcl::text
{
TextLayoutCache::TextLayoutCache(sal_Unicode const* pStr, sal_Int32 const nEnd)
#if defined __cpp_lib_memory_resource
    : runs(&CacheOwner::GetMemoryResource())
#endif
{
    vcl::ScriptRun aScriptRun(reinterpret_cast<const UChar*>(pStr), nEnd);
    while (aScriptRun.next())
    {
        runs.emplace_back(aScriptRun.getScriptStart(), aScriptRun.getScriptEnd(),
                          aScriptRun.getScriptCode());
    }
}

namespace
{
struct TextLayoutCacheCost
{
    size_t operator()(const std::shared_ptr<const TextLayoutCache>& item) const
    {
        return item->runs.size() * sizeof(item->runs.front());
    }
};

struct TextLayoutCacheMap : public CacheOwner
{
    typedef o3tl::lru_map<OUString, std::shared_ptr<const TextLayoutCache>, FirstCharsStringHash,
                          FastStringCompareEqual, TextLayoutCacheCost>
        Cache;

#if defined __cpp_lib_memory_resource
    std::pmr::polymorphic_allocator<TextLayoutCache> allocator;
#endif
    Cache cache;

    TextLayoutCacheMap(int capacity)
#if defined __cpp_lib_memory_resource
        : allocator(&CacheOwner::GetMemoryResource())
        , cache(capacity, &CacheOwner::GetMemoryResource())
#else
        : cache(capacity)
#endif
    {
    }

    std::shared_ptr<const TextLayoutCache> Create(OUString const& rString)
    {
        auto it = cache.find(rString);
        if (it != cache.end())
            return it->second;
#if defined __cpp_lib_memory_resource
        auto ret = std::allocate_shared<TextLayoutCache>(allocator, rString.getStr(),
                                                         rString.getLength());
#else
        auto ret = std::make_shared<TextLayoutCache>(rString.getStr(), rString.getLength());
#endif
        cache.insert({ rString, ret });
        return ret;
    }

    virtual OUString getCacheName() const override { return "TextLayoutCache"; }

    virtual bool dropCaches() override
    {
        cache.clear();
        return true;
    }

    virtual void dumpState(rtl::OStringBuffer& rState) override
    {
        rState.append("\nTextLayoutCache:\t");
        rState.append(static_cast<sal_Int32>(cache.size()));

        TextLayoutCacheCost cost;
        size_t nTotalCost = 0;
        for (auto it = cache.begin(); it != cache.end(); ++it)
            nTotalCost += cost(it->second);

        rState.append("\t cost: ");
        rState.append(static_cast<sal_Int64>(nTotalCost));
    }
};

} // namespace

std::shared_ptr<const TextLayoutCache> TextLayoutCache::Create(OUString const& rString)
{
    static tools::DeleteOnDeinit<TextLayoutCacheMap> cache(
        !comphelper::IsFuzzing() ? officecfg::Office::Common::Cache::Font::TextRunsCacheSize::get()
                                 : 100);
    if (TextLayoutCacheMap* map = cache.get())
        return map->Create(rString);
    return std::make_shared<TextLayoutCache>(rString.getStr(), rString.getLength());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
