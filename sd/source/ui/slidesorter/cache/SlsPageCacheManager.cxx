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

#include <cache/SlsPageCacheManager.hxx>

#include "SlsBitmapCache.hxx"
#include <view/SlideSorterView.hxx>
#include <model/SlideSorterModel.hxx>

#include <deque>
#include <map>
#include <memory>
#include <unordered_map>

namespace {

/** Collection of data that is stored for all active preview caches.
*/
class CacheDescriptor
{
public:
    ::sd::slidesorter::cache::PageCacheManager::DocumentKey mpDocument;
    Size const maPreviewSize;

    CacheDescriptor(
        ::sd::slidesorter::cache::PageCacheManager::DocumentKey const & pDocument,
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
        return reinterpret_cast<size_t>(rDescriptor.mpDocument.get()) + rDescriptor.maPreviewSize.Width();
    } };
};

/** Collection of data that is stored for the inactive, recently used
    caches.
*/
class RecentlyUsedCacheDescriptor
{
public:
    Size maPreviewSize;
    std::shared_ptr< ::sd::slidesorter::cache::BitmapCache> mpCache;

    RecentlyUsedCacheDescriptor(
        const Size& rPreviewSize,
        const std::shared_ptr< ::sd::slidesorter::cache::BitmapCache>& rpCache)
        :maPreviewSize(rPreviewSize),mpCache(rpCache)
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
    explicit BestFittingCacheComparer (const Size& rPreferredSize)
        : maPreferredSize(rPreferredSize)
    {}
    bool operator()(const ::sd::slidesorter::cache::PageCacheManager::BestFittingPageCaches::value_type& rElement1,
        const ::sd::slidesorter::cache::PageCacheManager::BestFittingPageCaches::value_type& rElement2)
    {
        if (rElement2.first == maPreferredSize)
            return false;
        else if (rElement1.first == maPreferredSize)
            return true;
        else
            return (rElement1.first.Width()*rElement1.first.Height()
                > rElement2.first.Width()*rElement2.first.Height());
    }

private:
    Size const maPreferredSize;
};

} // end of anonymous namespace

namespace sd { namespace slidesorter { namespace cache {

/** Container for the active caches.
*/
class PageCacheManager::PageCacheContainer
    : public std::unordered_map<CacheDescriptor,
                             std::shared_ptr<BitmapCache>,
                             CacheDescriptor::Hash,
                             CacheDescriptor::Equal>
{
public:
    PageCacheContainer() {}

    /** Compare entries in the cache container with respect to the cache
        address only.
    */
    class CompareWithCache { public:
        explicit CompareWithCache(const std::shared_ptr<BitmapCache>& rpCache)
            : mpCache(rpCache) {}
        bool operator () (const PageCacheContainer::value_type& rValue) const
        { return rValue.second == mpCache; }
    private:
        std::shared_ptr<BitmapCache> mpCache;
    };
};

/** The recently used caches are stored in one queue for each document.
*/
class PageCacheManager::RecentlyUsedPageCaches
{
public:
    typedef DocumentKey                                 key_type;
    typedef RecentlyUsedQueue                           mapped_type;
    typedef std::pair<const key_type,mapped_type>       value_type;
    typedef std::map<key_type,mapped_type>::iterator    iterator;
private:
    std::map<key_type,mapped_type> maMap;
public:
    RecentlyUsedPageCaches () {};

    iterator end() { return maMap.end(); }
    void clear() { maMap.clear(); }
    iterator find(const key_type& key) { return maMap.find(key); }
    template<class... Args>
    std::pair<iterator,bool> emplace(Args&&... args) { return maMap.emplace(std::forward<Args>(args)...); }
};

class PageCacheManager::Deleter
{
public:
    void operator() (PageCacheManager* pObject) { delete pObject; }
};

//===== PageCacheManager ====================================================

std::weak_ptr<PageCacheManager> PageCacheManager::mpInstance;

std::shared_ptr<PageCacheManager> PageCacheManager::Instance()
{
    std::shared_ptr<PageCacheManager> pInstance;

    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    pInstance = mpInstance.lock();
    if (pInstance == nullptr)
    {
        pInstance = std::shared_ptr<PageCacheManager>(
            new PageCacheManager(),
            PageCacheManager::Deleter());
        mpInstance = pInstance;
    }

    return pInstance;
}

PageCacheManager::PageCacheManager()
    : mpPageCaches(new PageCacheContainer()),
      mpRecentlyUsedPageCaches(new RecentlyUsedPageCaches())
{
}

PageCacheManager::~PageCacheManager()
{
}

std::shared_ptr<BitmapCache> PageCacheManager::GetCache (
    const DocumentKey& pDocument,
    const Size& rPreviewSize)
{
    std::shared_ptr<BitmapCache> pResult;

    // Look for the cache in the list of active caches.
    CacheDescriptor aKey (pDocument, rPreviewSize);
    PageCacheContainer::iterator iCache (mpPageCaches->find(aKey));
    if (iCache != mpPageCaches->end())
        pResult = iCache->second;

    // Look for the cache in the list of recently used caches.
    if (pResult == nullptr)
        pResult = GetRecentlyUsedCache(pDocument, rPreviewSize);

    // Create the cache when no suitable one does exist.
    if (pResult == nullptr)
        pResult.reset(new BitmapCache());

    // The cache may be newly created and thus empty or is old and may
    // contain previews that are not up-to-date.  Recycle previews from
    // other caches to fill in the holes.
    Recycle(pResult, pDocument,rPreviewSize);

    // Put the new (or old) cache into the container.
    mpPageCaches->emplace(aKey, pResult);

    return pResult;
}

void PageCacheManager::Recycle (
    const std::shared_ptr<BitmapCache>& rpCache,
    const DocumentKey& pDocument,
    const Size& rPreviewSize)
{
    BestFittingPageCaches aCaches;

    // Add bitmap caches from active caches.
    for (auto& rActiveCache : *mpPageCaches)
    {
        if (rActiveCache.first.mpDocument == pDocument)
            aCaches.emplace_back(
                rActiveCache.first.maPreviewSize, rActiveCache.second);
    }

    // Add bitmap caches from recently used caches.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        for (const auto& rRecentCache : iQueue->second)
            aCaches.emplace_back(
                rRecentCache.maPreviewSize, rRecentCache.mpCache);
    }

    ::std::sort(aCaches.begin(), aCaches.end(), BestFittingCacheComparer(rPreviewSize));

    for (const auto& rBestCache : aCaches)
    {
        rpCache->Recycle(*rBestCache.second);
    }
}

void PageCacheManager::ReleaseCache (const std::shared_ptr<BitmapCache>& rpCache)
{
    PageCacheContainer::iterator iCache (::std::find_if(
        mpPageCaches->begin(),
        mpPageCaches->end(),
        PageCacheContainer::CompareWithCache(rpCache)));

    if (iCache != mpPageCaches->end())
    {
        assert(iCache->second == rpCache);

        PutRecentlyUsedCache(iCache->first.mpDocument,iCache->first.maPreviewSize,rpCache);

        mpPageCaches->erase(iCache);
    }
}

std::shared_ptr<BitmapCache> PageCacheManager::ChangeSize (
    const std::shared_ptr<BitmapCache>& rpCache,
    const Size&,
    const Size& rNewPreviewSize)
{
    std::shared_ptr<BitmapCache> pResult;

    if (rpCache != nullptr)
    {
        // Look up the given cache in the list of active caches.
        PageCacheContainer::iterator iCacheToChange (::std::find_if(
            mpPageCaches->begin(),
            mpPageCaches->end(),
            PageCacheContainer::CompareWithCache(rpCache)));
        if (iCacheToChange != mpPageCaches->end())
        {
            assert(iCacheToChange->second == rpCache);

            // Now, we can change the preview size of the existing one by
            // removing the cache from the list and re-insert it with the
            // updated size.
            const ::sd::slidesorter::cache::PageCacheManager::DocumentKey aKey (
                iCacheToChange->first.mpDocument);
            mpPageCaches->erase(iCacheToChange);
            mpPageCaches->emplace(
                CacheDescriptor(aKey,rNewPreviewSize),
                rpCache);

            pResult = rpCache;
        }
        else
        {
            assert(iCacheToChange != mpPageCaches->end());
        }
    }

    return pResult;
}

bool PageCacheManager::InvalidatePreviewBitmap (
    const  DocumentKey& pDocument,
    const SdrPage* pKey)
{
    bool bHasChanged (false);

    if (pDocument!=nullptr)
    {
        // Iterate over all caches that are currently in use and invalidate
        // the previews in those that belong to the document.
        PageCacheContainer::iterator iCache;
        for (auto& rCache : *mpPageCaches)
            if (rCache.first.mpDocument == pDocument)
                bHasChanged |= rCache.second->InvalidateBitmap(pKey);

        // Invalidate the previews in the recently used caches belonging to
        // the given document.
        RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
        if (iQueue != mpRecentlyUsedPageCaches->end())
        {
            for (const auto& rCache2 : iQueue->second)
                bHasChanged |= rCache2.mpCache->InvalidateBitmap(pKey);
        }
    }

    return bHasChanged;
}

void PageCacheManager::InvalidateAllPreviewBitmaps (const DocumentKey& pDocument)
{
    if (pDocument == nullptr)
        return;

    // Iterate over all caches that are currently in use and invalidate the
    // previews in those that belong to the document.
    for (auto& rCache : *mpPageCaches)
        if (rCache.first.mpDocument == pDocument)
            rCache.second->InvalidateCache();

    // Invalidate the previews in the recently used caches belonging to the
    // given document.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        for (const auto& rCache2 : iQueue->second)
            rCache2.mpCache->InvalidateCache();
    }
}

void PageCacheManager::InvalidateAllCaches()
{
    // Iterate over all caches that are currently in use and invalidate
    // them.
    for (auto& rCache : *mpPageCaches)
        rCache.second->InvalidateCache();

    // Remove all recently used caches, there is not much sense in storing
    // invalidated and unused caches.
    mpRecentlyUsedPageCaches->clear();
}

void PageCacheManager::ReleasePreviewBitmap (const SdrPage* pPage)
{
    for (auto& rCache : *mpPageCaches)
        rCache.second->ReleaseBitmap(pPage);
}

std::shared_ptr<BitmapCache> PageCacheManager::GetRecentlyUsedCache (
    const DocumentKey& pDocument,
    const Size& rPreviewSize)
{
    std::shared_ptr<BitmapCache> pCache;

    // Look for the cache in the list of recently used caches.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        RecentlyUsedQueue::iterator iCache = std::find_if(iQueue->second.begin(), iQueue->second.end(),
            [&rPreviewSize](const RecentlyUsedCacheDescriptor& rCache) { return rCache.maPreviewSize == rPreviewSize; });
        if (iCache != iQueue->second.end())
        {
            pCache = iCache->mpCache;
            iQueue->second.erase(iCache);
        }
    }

    return pCache;
}

void PageCacheManager::PutRecentlyUsedCache(
    DocumentKey const & pDocument,
    const Size& rPreviewSize,
    const std::shared_ptr<BitmapCache>& rpCache)
{
    // Look up the list of recently used caches for the given document.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue == mpRecentlyUsedPageCaches->end())
        iQueue = mpRecentlyUsedPageCaches->emplace(
            pDocument, RecentlyUsedQueue()
            ).first;

    if (iQueue != mpRecentlyUsedPageCaches->end())
    {
        iQueue->second.push_front(RecentlyUsedCacheDescriptor(rPreviewSize,rpCache));
        // Shorten the list of recently used caches to the allowed maximal length.
        while (iQueue->second.size() > mnMaximalRecentlyCacheCount)
            iQueue->second.pop_back();
    }
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
