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

#include "cache/SlsPageCacheManager.hxx"

#include "SlsBitmapCache.hxx"
#include "view/SlideSorterView.hxx"
#include "model/SlideSorterModel.hxx"

#include <deque>
#include <map>
#include <memory>

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
    std::shared_ptr< ::sd::slidesorter::cache::PageCacheManager::Cache> mpCache;

    RecentlyUsedCacheDescriptor(
        const Size& rPreviewSize,
        const std::shared_ptr< ::sd::slidesorter::cache::PageCacheManager::Cache>& rpCache)
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
    : public std::unordered_map<CacheDescriptor,
                             std::shared_ptr<PageCacheManager::Cache>,
                             CacheDescriptor::Hash,
                             CacheDescriptor::Equal>
{
public:
    PageCacheContainer() {}

    /** Compare entries in the cache container with respect to the cache
        address only.
    */
    class CompareWithCache { public:
        explicit CompareWithCache(const std::shared_ptr<PageCacheManager::Cache>& rpCache)
            : mpCache(rpCache) {}
        bool operator () (const PageCacheContainer::value_type& rValue) const
        { return rValue.second == mpCache; }
    private:
        std::shared_ptr<PageCacheManager::Cache> mpCache;
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
    std::pair<iterator,bool> insert(const value_type& value) { return maMap.insert(value); }
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
    if (pInstance.get() == nullptr)
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
      mpRecentlyUsedPageCaches(new RecentlyUsedPageCaches()),
      mnMaximalRecentlyCacheCount(2)
{
}

PageCacheManager::~PageCacheManager()
{
}

std::shared_ptr<PageCacheManager::Cache> PageCacheManager::GetCache (
    const DocumentKey& pDocument,
    const Size& rPreviewSize)
{
    std::shared_ptr<Cache> pResult;

    // Look for the cache in the list of active caches.
    CacheDescriptor aKey (pDocument, rPreviewSize);
    PageCacheContainer::iterator iCache (mpPageCaches->find(aKey));
    if (iCache != mpPageCaches->end())
        pResult = iCache->second;

    // Look for the cache in the list of recently used caches.
    if (pResult.get() == nullptr)
        pResult = GetRecentlyUsedCache(pDocument, rPreviewSize);

    // Create the cache when no suitable one does exist.
    if (pResult.get() == nullptr)
        pResult.reset(new Cache());

    // The cache may be newly created and thus empty or is old and may
    // contain previews that are not up-to-date.  Recycle previews from
    // other caches to fill in the holes.
    Recycle(pResult, pDocument,rPreviewSize);

    // Put the new (or old) cache into the container.
    if (pResult.get() != nullptr)
        mpPageCaches->insert(PageCacheContainer::value_type(aKey, pResult));

    return pResult;
}

void PageCacheManager::Recycle (
    const std::shared_ptr<Cache>& rpCache,
    const DocumentKey& pDocument,
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

void PageCacheManager::ReleaseCache (const std::shared_ptr<Cache>& rpCache)
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

std::shared_ptr<PageCacheManager::Cache> PageCacheManager::ChangeSize (
    const std::shared_ptr<Cache>& rpCache,
    const Size& rOldPreviewSize,
    const Size& rNewPreviewSize)
{
    (void)rOldPreviewSize;

    std::shared_ptr<Cache> pResult;

    if (rpCache.get() != nullptr)
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
    const  DocumentKey& pDocument,
    const SdrPage* pKey)
{
    bool bHasChanged (false);

    if (pDocument!=nullptr)
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

void PageCacheManager::InvalidateAllPreviewBitmaps (const DocumentKey& pDocument)
{
    if (pDocument == nullptr)
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

void PageCacheManager::InvalidateAllCaches()
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

std::shared_ptr<PageCacheManager::Cache> PageCacheManager::GetRecentlyUsedCache (
    const DocumentKey& pDocument,
    const Size& rPreviewSize)
{
    std::shared_ptr<Cache> pCache;

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
    const std::shared_ptr<Cache>& rpCache)
{
    // Look up the list of recently used caches for the given document.
    RecentlyUsedPageCaches::iterator iQueue (mpRecentlyUsedPageCaches->find(pDocument));
    if (iQueue == mpRecentlyUsedPageCaches->end())
        iQueue = mpRecentlyUsedPageCaches->insert(
            RecentlyUsedPageCaches::value_type(pDocument, RecentlyUsedQueue())
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
