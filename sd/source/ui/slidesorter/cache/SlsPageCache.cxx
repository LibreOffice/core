/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_sd.hxx"

#include "SlsGenericPageCache.hxx"
#include "SlsRequestFactory.hxx"
#include "cache/SlsPageCache.hxx"
#include "model/SlideSorterModel.hxx"
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>

using namespace ::com::sun::star;


namespace sd { namespace slidesorter { namespace cache {


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




PageCache::~PageCache (void)
{
}




void PageCache::ChangeSize (
    const Size& rPreviewSize,
    const bool bDoSuperSampling)
{
    mpImplementation->ChangePreviewSize(rPreviewSize, bDoSuperSampling);
}




Bitmap PageCache::GetPreviewBitmap (
    const CacheKey aKey,
    const bool bResize)
{
    return mpImplementation->GetPreviewBitmap(aKey, bResize);
}




Bitmap PageCache::GetMarkedPreviewBitmap (
    const CacheKey aKey,
    const bool bResize)
{
    return mpImplementation->GetMarkedPreviewBitmap(aKey, bResize);
}




void PageCache::SetMarkedPreviewBitmap (
    const CacheKey aKey,
    const Bitmap& rMarkedBitmap)
{
    mpImplementation->SetMarkedPreviewBitmap(aKey, rMarkedBitmap);
}




void PageCache::RequestPreviewBitmap (const CacheKey aKey)
{
    return mpImplementation->RequestPreviewBitmap(aKey);
}




void PageCache::InvalidatePreviewBitmap (
    const CacheKey aKey,
    const bool bRequestPreview)
{
    if (mpImplementation->InvalidatePreviewBitmap(aKey) && bRequestPreview)
        RequestPreviewBitmap(aKey);
}




void PageCache::ReleasePreviewBitmap (const CacheKey aKey)
{
    mpImplementation->ReleasePreviewBitmap(aKey);
}




void PageCache::InvalidateCache (const bool bUpdateCache)
{
    mpImplementation->InvalidateCache(bUpdateCache);
}




void PageCache::SetPreciousFlag (
    const CacheKey aKey,
    const bool bIsPrecious)
{
    mpImplementation->SetPreciousFlag(aKey, bIsPrecious);
}




void PageCache::Pause (void)
{
    mpImplementation->Pause();
}




void PageCache::Resume (void)
{
    mpImplementation->Resume();
}


} } } // end of namespace ::sd::slidesorter::cache
