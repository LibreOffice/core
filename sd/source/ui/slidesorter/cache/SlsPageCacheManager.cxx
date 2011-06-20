/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "cache/SlsPageCacheManager.hxx"

#include "SlsBitmapCache.hxx"
#include "view/SlideSorterView.hxx"
#include "model/SlideSorterModel.hxx"

#include <deque>
#include <map>
#include <boost/weak_ptr.hpp>

namespace {

/** Collection of data that is stored for all active preview caches.
*/
class CacheDescriptor
{
public:
    ::sd::slidesorter::cache::PageCacheManager::DocumentKey mpDocument;
    Size maPreviewSize;

    CacheDescriptor(
        ::sd::slidesorter::cache::PageCacheManager::DocumentKey pDocument,
        const Size& rPreviewSize)
        :mpDocument(pDocument),maPreviewSize(rPreviewSize)
    {}
    /// Test for equality with respect to all members.
    class Equal {public: bool operator() (
        const CacheDescriptor& rDescriptor1, const CacheDescriptor& rDescriptor2) const {
        return rDescriptor1.mpDocument==rDescriptor2.mpDocument
            && rDescriptor1.maPreviewSize==rDescriptor2.maPreviewSize;
    } };
    /// Hash function that takes all members into account.
    class Hash {public: size_t operator() (const CacheDescriptor& rDescriptor) const {
        return (size_t)rDescriptor.mpDocument.get() + rDescriptor.maPreviewSize.Width();
    } };
};




/** Collection of data that is stored for the inactive, recently used
    caches.
*/
class RecentlyUsedCacheDescriptor
{
public:
    ::sd::slidesorter::cache::PageCacheManager::DocumentKey mpDocument;
    Size maPreviewSize;
    ::boost::shared_ptr< ::sd::slidesorter::cache::PageCacheManager::Cache> mpCache;

    RecentlyUsedCacheDescriptor(
        ::sd::slidesorter::cache::PageCacheManager::DocumentKey pDocument,
        const Size& rPreviewSize,
        const ::boost::shared_ptr< ::sd::slidesorter::cache::PageCacheManager::Cache>& rpCache)
        :mpDocument(pDocument),maPreviewSize(rPreviewSize),mpCache(rpCache)
    {}
};




/** The list of recently used caches is organized as queue.  When elements
    are added the list is shortened to the maximally allowed number of
    elements by removing the least recently used elements.
*/
typedef ::std::deque<RecentlyUsedCacheDescriptor> RecentlyUsedQueue;




/** Compare the caches by preview size.  Those that match the given size
    come first, then, regardless of the given size, the largest ones before
    the smaller ones.
*/
class BestFittingCacheComparer
{
public:
    BestFittingCacheComparer (const Size& rPreferredSize)
        : maPreferredSize(rPreferredSize)
    {}
    bool operator()(const ::sd::slidesorter::cache::PageCacheManager::BestFittingPageCaches::value_type& rElement1,
        const ::sd::slidesorter::cache::PageCacheManager::BestFittingPageCaches::value_type& rElement2)
    {
        if (rElement1.first == maPreferredSize)
            return true;
        else if (rElement2.first == maPreferredSize)
            return false;
        else
            return (rElement1.first.Width()*rElement1.first.Height()
                > rElement2.first.Width()*rElement2.first.Height());
    }

private:
    Size maPreferredSize;
};

} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace cache {

/** Container for the active caches.
*/
class PageCacheManager::PageCacheContainer
    : public ::boost::unordered_map<CacheDescriptor,
                             ::boost::shared_ptr<PageCacheManager::Cache>,
                             CacheDescriptor::Hash,
                             CacheDescriptor::Equal>
{
public:
    PageCacheContainer (void) {}

    /** Compare entries in the cache container with respect to the cache
        address only.
    */
    class CompareWithCache { public:
        CompareWithCache(const ::boost::shared_ptr<PageCacheManager::Cache>& rpCache)
            : mpCache(rpCache) {}
        bool operator () (const PageCacheContainer::value_type& rValue) const
        { return rValue.second == mpCache; }
    private:
        ::boost::shared_ptr<PageCacheManager::Cache> mpCache;
    };
};


/** The recently used caches are stored in one queue for each document.
*/
class PageCacheManager::RecentlyUsedPageCaches
    : public ::std::map<DocumentKey,RecentlyUsedQueue>
{
public:
    RecentlyUsedPageCaches (void) {};
};




class PageCacheManager::Deleter
{
public:
    void operator() (PageCacheManager* pObject) { delete pObject; }
};



//===== PageCacheManager ====================================================

::boost::weak_ptr<PageCacheManager> PageCacheManager::mpInstance;

::boost::shared_ptr<PageCacheManager> PageCacheManager::Instance (void)
{
    ::boost::shared_ptr<PageCacheManager> pInstance;

    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    pInstance = mpInstance.lock();
    if (pInstance.get() == NULL)
    {
        pInstance = ::boost::shared_ptr<PageCacheManager>(
            new PageCacheManager(),
            PageCacheManager::Deleter());
        mpInstance = pInstance;
    }

    return pInstance;
}




PageCacheManager::PageCacheManager (void)
    : mpPageCaches(new PageCacheContainer()),
      mpRecentlyUsedPageCaches(new RecentlyUsedPageCaches()),
      mnMaximalRecentlyCacheCount(2)
{
}




PageCacheManager::~PageCacheManager (void)
{
}




::boost::shared_ptr<PageCacheManager::Cache> PageCacheManager::GetCache (
    DocumentKey pDocument,
    const Size& rPreviewSize)
{
    ::boost::shared_ptr<Cache> pResult;

    // Look for the cache in the list of active caches.
    CacheDescriptor aKey (pDocument, rPreviewSize);
    PageCacheContainer::iterator iCache (mpPageCaches->find(aKey));
    if (iCache != mpPageCaches->end())
        pResult = iCache->second;

    // Look for the cache in the list of recently used caches.
    if (pResult.get() == NULL)
        pResult = GetRecentlyUsedCache(pDocument, rPreviewSize);

    // Create the cache when no suitable one does exist.
    if (pResult.get() == NULL)
        pResult.reset(new Cache());

    // The cache may be newly created and thus empty or is old and may
    // contain previews that are not up-to-date.  Recycle previews from
    // other caches to fill in the holes.
    Recycle(pResult, pDocument,rPreviewSize);

    // Put the new (or old) cache into the container.
    if (pResult.get() != NULL)
        mpPageCaches->insert(PageCacheContainer::value_type(aKey, pResult));

    return pResult;
}




void PageCacheManager::Recycle (
    const ::boost::shared_ptr<Cache>& rpCache,
    DocumentKey pDocument,
    const Size& rPreviewSize)
{
    BestFittingPageCaches aCaches;

    // Add bitmap caches from active caches.
    PageCacheContainer::iterator iActiveCache;
    for (iActiveCache=mpPageCaches->begin(); iActiveCache!=mpPageCaches->end(); ++iActiveCache)
    {
        if (iActiveCache->first.mpDocument == pDocument)
            aCaches.push_back(BestFittingPageCaches::value_type(
                iActiveCache->first.maPreviewSize, iActiveCache->second));
    }

    // Add bitmap caches from recently used caches.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        RecentlyUsedQueue::const_iterator iRecentCache;
        for (iRecentCache=iQueue->second.begin();iRecentCache!=iQueue->second.end();++iRecentCache)
            aCaches.push_back(BestFittingPageCaches::value_type(
                iRecentCache->maPreviewSize, iRecentCache->mpCache));
    }

    ::std::sort(aCaches.begin(), aCaches.end(), BestFittingCacheComparer(rPreviewSize));

    BestFittingPageCaches::const_iterator iBestCache;
    for (iBestCache=aCaches.begin(); iBestCache!=aCaches.end(); ++iBestCache)
    {
        rpCache->Recycle(*iBestCache->second);
    }
}




void PageCacheManager::ReleaseCache (const ::boost::shared_ptr<Cache>& rpCache)
{
    PageCacheContainer::iterator iCache (::std::find_if(
        mpPageCaches->begin(),
        mpPageCaches->end(),
        PageCacheContainer::CompareWithCache(rpCache)));

    if (iCache != mpPageCaches->end())
    {
        OSL_ASSERT(iCache->second == rpCache);

        PutRecentlyUsedCache(iCache->first.mpDocument,iCache->first.maPreviewSize,rpCache);

        mpPageCaches->erase(iCache);
    }
}




::boost::shared_ptr<PageCacheManager::Cache> PageCacheManager::ChangeSize (
    const ::boost::shared_ptr<Cache>& rpCache,
    const Size& rOldPreviewSize,
    const Size& rNewPreviewSize)
{
    (void)rOldPreviewSize;

    ::boost::shared_ptr<Cache> pResult;

    if (rpCache.get() != NULL)
    {
        // Look up the given cache in the list of active caches.
        PageCacheContainer::iterator iCacheToChange (::std::find_if(
            mpPageCaches->begin(),
            mpPageCaches->end(),
            PageCacheContainer::CompareWithCache(rpCache)));
        if (iCacheToChange != mpPageCaches->end())
        {
            OSL_ASSERT(iCacheToChange->second == rpCache);

            // Now, we can change the preview size of the existing one by
            // removing the cache from the list and re-insert it with the
            // updated size.
            const ::sd::slidesorter::cache::PageCacheManager::DocumentKey aKey (
                iCacheToChange->first.mpDocument);
            mpPageCaches->erase(iCacheToChange);
            mpPageCaches->insert(PageCacheContainer::value_type(
                CacheDescriptor(aKey,rNewPreviewSize),
                rpCache));

            pResult = rpCache;
        }
        else
        {
            OSL_ASSERT(iCacheToChange != mpPageCaches->end());
        }
    }

    return pResult;
}




bool PageCacheManager::InvalidatePreviewBitmap (
    DocumentKey pDocument,
    const SdrPage* pKey)
{
    bool bHasChanged (false);

    if (pDocument!=NULL)
    {
        // Iterate over all caches that are currently in use and invalidate
        // the previews in those that belong to the document.
        PageCacheContainer::iterator iCache;
        for (iCache=mpPageCaches->begin(); iCache!=mpPageCaches->end();  ++iCache)
            if (iCache->first.mpDocument == pDocument)
                bHasChanged |= iCache->second->InvalidateBitmap(pKey);

        // Invalidate the previews in the recently used caches belonging to
        // the given document.
        RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
        if (iQueue != mpRecentlyUsedPageCaches->end())
        {
            RecentlyUsedQueue::const_iterator iCache2;
            for (iCache2=iQueue->second.begin(); iCache2!=iQueue->second.end(); ++iCache2)
                bHasChanged |= iCache2->mpCache->InvalidateBitmap(pKey);
        }
    }

    return bHasChanged;
}




void PageCacheManager::InvalidateAllPreviewBitmaps (DocumentKey pDocument)
{
    if (pDocument == NULL)
        return;

    // Iterate over all caches that are currently in use and invalidate the
    // previews in those that belong to the document.
    PageCacheContainer::iterator iCache;
    for (iCache=mpPageCaches->begin(); iCache!=mpPageCaches->end();  ++iCache)
        if (iCache->first.mpDocument == pDocument)
            iCache->second->InvalidateCache();

    // Invalidate the previews in the recently used caches belonging to the
    // given document.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        RecentlyUsedQueue::const_iterator iCache2;
        for (iCache2=iQueue->second.begin(); iCache2!=iQueue->second.end(); ++iCache2)
            iCache2->mpCache->InvalidateCache();
    }
}




void PageCacheManager::InvalidateAllCaches (void)
{
    // Iterate over all caches that are currently in use and invalidate
    // them.
    PageCacheContainer::iterator iCache;
    for (iCache=mpPageCaches->begin(); iCache!=mpPageCaches->end();  ++iCache)
        iCache->second->InvalidateCache();

    // Remove all recently used caches, there is not much sense in storing
    // invalidated and unused caches.
    mpRecentlyUsedPageCaches->clear();
}




void PageCacheManager::ReleasePreviewBitmap (const SdrPage* pPage)
{
    PageCacheContainer::iterator iCache;
    for (iCache=mpPageCaches->begin(); iCache!=mpPageCaches->end(); ++iCache)
        iCache->second->ReleaseBitmap(pPage);
}




::boost::shared_ptr<PageCacheManager::Cache> PageCacheManager::GetRecentlyUsedCache (
    DocumentKey pDocument,
    const Size& rPreviewSize)
{
    ::boost::shared_ptr<Cache> pCache;

    // Look for the cache in the list of recently used caches.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        RecentlyUsedQueue::iterator iCache;
        for (iCache=iQueue->second.begin(); iCache!= iQueue->second.end(); ++iCache)
            if (iCache->maPreviewSize == rPreviewSize)
            {
                pCache = iCache->mpCache;
                iQueue->second.erase(iCache);
                break;
            }
    }

    return pCache;
}




void PageCacheManager::PutRecentlyUsedCache(
    DocumentKey pDocument,
    const Size& rPreviewSize,
    const ::boost::shared_ptr<Cache>& rpCache)
{
    // Look up the list of recently used caches for the given document.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue == mpRecentlyUsedPageCaches->end())
        iQueue = mpRecentlyUsedPageCaches->insert(
            RecentlyUsedPageCaches::value_type(pDocument, RecentlyUsedQueue())
            ).first;

    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        iQueue->second.push_front(RecentlyUsedCacheDescriptor(pDocument,rPreviewSize,rpCache));
        // Shorten the list of recently used caches to the allowed maximal length.
        while (iQueue->second.size() > mnMaximalRecentlyCacheCount)
            iQueue->second.pop_back();
    }
}



} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
