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
#include <unotools/configmgr.hxx>
#include <vcl/lazydelete.hxx>
#include <officecfg/Office/Common.hxx>

namespace vcl::text
{
TextLayoutCache::TextLayoutCache(sal_Unicode const* pStr, sal_Int32 const nEnd)
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
} // namespace

std::shared_ptr<const TextLayoutCache> TextLayoutCache::Create(OUString const& rString)
{
    typedef o3tl::lru_map<OUString, std::shared_ptr<const TextLayoutCache>, FirstCharsStringHash,
                          FastStringCompareEqual, TextLayoutCacheCost>
        Cache;
    static vcl::DeleteOnDeinit<Cache> cache(
        !utl::ConfigManager::IsFuzzing()
            ? officecfg::Office::Common::Cache::Font::TextRunsCacheSize::get()
            : 100);
    if (Cache* map = cache.get())
    {
        auto it = map->find(rString);
        if (it != map->end())
            return it->second;
        auto ret = std::make_shared<const TextLayoutCache>(rString.getStr(), rString.getLength());
        map->insert({ rString, ret });
        return ret;
    }
    return std::make_shared<const TextLayoutCache>(rString.getStr(), rString.getLength());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
