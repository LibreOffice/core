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

#include <tools/gen.hxx>
#include "SlsGenericPageCache.hxx"
#include <cache/SlsPageCache.hxx>

using namespace ::com::sun::star;

namespace sd::slidesorter::cache {

//===== PageCache =============================================================

PageCache::PageCache (
    const Size& rPreviewSize,
    const bool bDoSuperSampling,
    const SharedCacheContext& rpCacheContext)
   : mpImplementation(
        new GenericPageCache(
            rPreviewSize,
            bDoSuperSampling,
            rpCacheContext))
{
}

PageCache::~PageCache()
{
}

void PageCache::ChangeSize (
    const Size& rPreviewSize,
    const bool bDoSuperSampling)
{
    mpImplementation->ChangePreviewSize(rPreviewSize, bDoSuperSampling);
}

BitmapEx PageCache::GetPreviewBitmap (
    const CacheKey aKey,
    const bool bResize)
{
    return mpImplementation->GetPreviewBitmap(aKey, bResize);
}

BitmapEx PageCache::GetMarkedPreviewBitmap (
    const CacheKey aKey)
{
    return mpImplementation->GetMarkedPreviewBitmap(aKey);
}

void PageCache::SetMarkedPreviewBitmap (
    const CacheKey aKey,
    const BitmapEx& rMarkedBitmap)
{
    mpImplementation->SetMarkedPreviewBitmap(aKey, rMarkedBitmap);
}

void PageCache::RequestPreviewBitmap (const CacheKey aKey)
{
    return mpImplementation->RequestPreviewBitmap(aKey, true);
}

void PageCache::InvalidatePreviewBitmap (
    const CacheKey aKey)
{
    if (mpImplementation->InvalidatePreviewBitmap(aKey))
        RequestPreviewBitmap(aKey);
}

void PageCache::InvalidateCache()
{
    mpImplementation->InvalidateCache();
}

void PageCache::SetPreciousFlag (
    const CacheKey aKey,
    const bool bIsPrecious)
{
    mpImplementation->SetPreciousFlag(aKey, bIsPrecious);
}

} // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
