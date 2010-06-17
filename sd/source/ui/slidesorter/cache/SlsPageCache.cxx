/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    const SharedCacheContext& rpCacheContext)
   : mpImplementation(
        new GenericPageCache(
            rPreviewSize,
            rpCacheContext))
{
}




PageCache::~PageCache (void)
{
}




void PageCache::ChangeSize(const Size& rPreviewSize)
{
    mpImplementation->ChangePreviewSize(rPreviewSize);
}




BitmapEx PageCache::GetPreviewBitmap (
    CacheKey aKey,
    const Size& rSize)
{
    return mpImplementation->GetPreviewBitmap(aKey, rSize);
}




void PageCache::ReleasePreviewBitmap (
    CacheKey aKey)
{
    mpImplementation->ReleasePreviewBitmap(aKey);
}




void PageCache::InvalidateCache (bool bUpdateCache)
{
    mpImplementation->InvalidateCache(bUpdateCache);
}




void PageCache::SetPreciousFlag (
    CacheKey aKey,
    bool bIsPrecious)
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
