/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageCache.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:19:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "SlsGenericPageCache.hxx"
#include "SlsRequestFactory.hxx"
#include "SlsIdleDetector.hxx"
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




void PageCache::RequestPreviewBitmap (
    CacheKey aKey,
    const Size& rSize)
{
    return mpImplementation->RequestPreviewBitmap(aKey, rSize);
}




void PageCache::InvalidatePreviewBitmap (
    CacheKey aKey)
{
    mpImplementation->InvalidatePreviewBitmap(aKey);
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
