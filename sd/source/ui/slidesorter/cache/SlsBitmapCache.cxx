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

#include <memory>
#include "SlsBitmapCache.hxx"
#include "SlsCacheCompactor.hxx"
#include "SlsBitmapCompressor.hxx"
#include "SlsCacheConfiguration.hxx"

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sal/log.hxx>

// Define the default value for the maximal cache size that is used for
// previews that are currently not visible.  The visible previews are all
// held in memory at all times.  This default is used only when the
// configuration does not have a value.
static const sal_Int32 MAXIMAL_CACHE_SIZE = 4*1024L*1024L;

using namespace ::com::sun::star::uno;

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache::CacheEntry
{
public:
    CacheEntry(const BitmapEx& rBitmap, sal_Int32 nLastAccessTime, bool bIsPrecious);
    CacheEntry(sal_Int32 nLastAccessTime, bool bIsPrecious);
    inline void Recycle (const CacheEntry& rEntry);
    inline sal_Int32 GetMemorySize() const;
    void Compress (const std::shared_ptr<BitmapCompressor>& rpCompressor);
    inline void Decompress();

    bool IsUpToDate() const { return mbIsUpToDate; }
    void SetUpToDate (bool bIsUpToDate) { mbIsUpToDate = bIsUpToDate; }
    sal_Int32 GetAccessTime() const { return mnLastAccessTime; }
    void SetAccessTime (sal_Int32 nAccessTime) { mnLastAccessTime = nAccessTime; }

    const BitmapEx& GetPreview() const { return maPreview; }
    inline void SetPreview (const BitmapEx& rPreview);
    bool HasPreview() const;

    const BitmapEx& GetMarkedPreview() const { return maMarkedPreview; }
    inline void SetMarkedPreview (const BitmapEx& rMarkePreview);

    bool HasReplacement() const { return (mpReplacement != nullptr); }
    inline bool HasLosslessReplacement() const;
    void Invalidate() { mpReplacement.reset(); mpCompressor.reset(); mbIsUpToDate = false; }
    bool IsPrecious() const { return mbIsPrecious; }
    void SetPrecious (bool bIsPrecious) { mbIsPrecious = bIsPrecious; }

private:
    BitmapEx maPreview;
    BitmapEx maMarkedPreview;
    std::shared_ptr<BitmapReplacement> mpReplacement;
    std::shared_ptr<BitmapCompressor> mpCompressor;
    bool mbIsUpToDate;
    sal_Int32 mnLastAccessTime;
    // When this flag is set then the bitmap is not modified by a cache
    // compactor.
    bool mbIsPrecious;
};

class CacheHash {
public:
    size_t operator()(const BitmapCache::CacheKey& p) const
    { return reinterpret_cast<size_t>(p); }
};

class BitmapCache::CacheBitmapContainer
    : public std::unordered_map<CacheKey, CacheEntry, CacheHash>
{
public:
    CacheBitmapContainer() {}
};

namespace {

typedef ::std::vector<
    ::std::pair< ::sd::slidesorter::cache::BitmapCache::CacheKey,
      ::sd::slidesorter::cache::BitmapCache::CacheEntry>
    > SortableBitmapContainer;

    /** Compare elements of the bitmap cache according to their last access
        time.
    */
    class AccessTimeComparator
    {
    public:
        bool operator () (
            const SortableBitmapContainer::value_type& e1,
            const SortableBitmapContainer::value_type& e2)
        {
            return e1.second.GetAccessTime() < e2.second.GetAccessTime();
        }
    };

} // end of anonymous namespace

//=====  BitmapCache  =========================================================

BitmapCache::BitmapCache ()
    : maMutex(),
      mpBitmapContainer(new CacheBitmapContainer()),
      mnNormalCacheSize(0),
      mnPreciousCacheSize(0),
      mnCurrentAccessTime(0),
      mnMaximalNormalCacheSize(MAXIMAL_CACHE_SIZE),
      mpCacheCompactor(),
      mbIsFull(false)
{
    Any aCacheSize (CacheConfiguration::Instance()->GetValue("CacheSize"));
    if (aCacheSize.has<sal_Int32>())
        aCacheSize >>= mnMaximalNormalCacheSize;

    mpCacheCompactor = CacheCompactor::Create(*this,mnMaximalNormalCacheSize);
}

BitmapCache::~BitmapCache()
{
    Clear();
}

void BitmapCache::Clear()
{
    ::osl::MutexGuard aGuard (maMutex);

    mpBitmapContainer->clear();
    mnNormalCacheSize = 0;
    mnPreciousCacheSize = 0;
    mnCurrentAccessTime = 0;
}

bool BitmapCache::HasBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    return (iEntry != mpBitmapContainer->end()
        && (iEntry->second.HasPreview() || iEntry->second.HasReplacement()));
}

bool BitmapCache::BitmapIsUpToDate (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    bool bIsUpToDate = false;
    CacheBitmapContainer::iterator aIterator (mpBitmapContainer->find(rKey));
    if (aIterator != mpBitmapContainer->end())
        bIsUpToDate = aIterator->second.IsUpToDate();

    return bIsUpToDate;
}

BitmapEx BitmapCache::GetBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry == mpBitmapContainer->end())
    {
        // Create an empty bitmap for the given key that acts as placeholder
        // until we are given the real one.  Mark it as not being up to date.
        SetBitmap(rKey, BitmapEx(), false);
        iEntry = mpBitmapContainer->find(rKey);
        iEntry->second.SetUpToDate(false);
    }
    else
    {
        iEntry->second.SetAccessTime(mnCurrentAccessTime++);

        // Maybe we have to decompress the preview.
        if ( ! iEntry->second.HasPreview() && iEntry->second.HasReplacement())
        {
            UpdateCacheSize(iEntry->second, REMOVE);
            iEntry->second.Decompress();
            UpdateCacheSize(iEntry->second, ADD);
        }
    }
    return iEntry->second.GetPreview();
}

BitmapEx BitmapCache::GetMarkedBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        iEntry->second.SetAccessTime(mnCurrentAccessTime++);
        return iEntry->second.GetMarkedPreview();
    }
    else
        return BitmapEx();
}

void BitmapCache::ReleaseBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (mpBitmapContainer->find(rKey));
    if (aIterator != mpBitmapContainer->end())
    {
        UpdateCacheSize(aIterator->second, REMOVE);
        mpBitmapContainer->erase(aIterator);
    }
}

bool BitmapCache::InvalidateBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        iEntry->second.SetUpToDate(false);

        // When there is a preview then we release the replacement.  The
        // preview itself is kept until a new one is created.
        if (iEntry->second.HasPreview())
        {
            UpdateCacheSize(iEntry->second, REMOVE);
            iEntry->second.Invalidate();
            UpdateCacheSize(iEntry->second, ADD);
        }
        return true;
    }
    else
        return false;
}

void BitmapCache::InvalidateCache()
{
    ::osl::MutexGuard aGuard (maMutex);

    for (auto& rEntry : *mpBitmapContainer)
    {
        rEntry.second.Invalidate();
    }
    ReCalculateTotalCacheSize();
}

void BitmapCache::SetBitmap (
    const CacheKey& rKey,
    const BitmapEx& rPreview,
    bool bIsPrecious)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        UpdateCacheSize(iEntry->second, REMOVE);
        iEntry->second.SetPreview(rPreview);
        iEntry->second.SetUpToDate(true);
        iEntry->second.SetAccessTime(mnCurrentAccessTime++);
    }
    else
    {
        iEntry = mpBitmapContainer->emplace(
            rKey,
            CacheEntry(rPreview, mnCurrentAccessTime++, bIsPrecious)
            ).first;
    }

    if (iEntry != mpBitmapContainer->end())
        UpdateCacheSize(iEntry->second, ADD);
}

void BitmapCache::SetMarkedBitmap (
    const CacheKey& rKey,
    const BitmapEx& rPreview)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        UpdateCacheSize(iEntry->second, REMOVE);
        iEntry->second.SetMarkedPreview(rPreview);
        iEntry->second.SetAccessTime(mnCurrentAccessTime++);
        UpdateCacheSize(iEntry->second, ADD);
    }
}

void BitmapCache::SetPrecious (const CacheKey& rKey, bool bIsPrecious)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        if (iEntry->second.IsPrecious() != bIsPrecious)
        {
            UpdateCacheSize(iEntry->second, REMOVE);
            iEntry->second.SetPrecious(bIsPrecious);
            UpdateCacheSize(iEntry->second, ADD);
        }
    }
    else if (bIsPrecious)
    {
        iEntry = mpBitmapContainer->emplace(
            rKey,
            CacheEntry(BitmapEx(), mnCurrentAccessTime++, bIsPrecious)
            ).first;
        UpdateCacheSize(iEntry->second, ADD);
    }
}

void BitmapCache::ReCalculateTotalCacheSize()
{
    ::osl::MutexGuard aGuard (maMutex);

    mnNormalCacheSize = 0;
    mnPreciousCacheSize = 0;
    for (const auto& rEntry : *mpBitmapContainer)
    {
        if (rEntry.second.IsPrecious())
            mnPreciousCacheSize += rEntry.second.GetMemorySize();
        else
            mnNormalCacheSize += rEntry.second.GetMemorySize();
    }
    mbIsFull = (mnNormalCacheSize  >= mnMaximalNormalCacheSize);

    SAL_INFO("sd.sls", OSL_THIS_FUNC << ": cache size is " << mnNormalCacheSize << "/" << mnPreciousCacheSize);
}

void BitmapCache::Recycle (const BitmapCache& rCache)
{
    ::osl::MutexGuard aGuard (maMutex);

    for (const auto& rOtherEntry : *rCache.mpBitmapContainer)
    {
        CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rOtherEntry.first));
        if (iEntry == mpBitmapContainer->end())
        {
            iEntry = mpBitmapContainer->emplace(
                rOtherEntry.first,
                CacheEntry(mnCurrentAccessTime++, true)
                ).first;
            UpdateCacheSize(iEntry->second, ADD);
        }
        if (iEntry != mpBitmapContainer->end())
        {
            UpdateCacheSize(iEntry->second, REMOVE);
            iEntry->second.Recycle(rOtherEntry.second);
            UpdateCacheSize(iEntry->second, ADD);
        }
    }
}

::std::unique_ptr<BitmapCache::CacheIndex> BitmapCache::GetCacheIndex() const
{
    ::osl::MutexGuard aGuard (maMutex);

    // Create a copy of the bitmap container.
    SortableBitmapContainer aSortedContainer;
    aSortedContainer.reserve(mpBitmapContainer->size());

    // Copy the relevant entries.
    for (const auto& rEntry : *mpBitmapContainer)
    {
        if ( rEntry.second.IsPrecious())
            continue;

        if ( ! rEntry.second.HasPreview())
            continue;

        aSortedContainer.emplace_back(rEntry.first, rEntry.second);
    }

    // Sort the remaining entries.
    ::std::sort(aSortedContainer.begin(), aSortedContainer.end(), AccessTimeComparator());

    // Return a list with the keys of the sorted entries.
    ::std::unique_ptr<CacheIndex> pIndex(new CacheIndex);
    pIndex->reserve(aSortedContainer.size());
    for (const auto& rIndexEntry : aSortedContainer)
        pIndex->push_back(rIndexEntry.first);
    return pIndex;
}

void BitmapCache::Compress (
    const CacheKey& rKey,
    const std::shared_ptr<BitmapCompressor>& rpCompressor)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end() && iEntry->second.HasPreview())
    {
        UpdateCacheSize(iEntry->second, REMOVE);
        iEntry->second.Compress(rpCompressor);
        UpdateCacheSize(iEntry->second, ADD);
    }
}

void BitmapCache::UpdateCacheSize (const CacheEntry& rEntry, CacheOperation eOperation)
{
    sal_Int32 nEntrySize (rEntry.GetMemorySize());
    sal_Int32& rCacheSize (rEntry.IsPrecious() ? mnPreciousCacheSize : mnNormalCacheSize);
    switch (eOperation)
    {
        case ADD:
            rCacheSize += nEntrySize;
            if ( ! rEntry.IsPrecious() && mnNormalCacheSize>mnMaximalNormalCacheSize)
            {
                mbIsFull = true;
                SAL_INFO("sd.sls", OSL_THIS_FUNC << ": cache size is " << mnNormalCacheSize << " > " << mnMaximalNormalCacheSize);
                mpCacheCompactor->RequestCompaction();
            }
            break;

        case REMOVE:
            rCacheSize -= nEntrySize;
            if (mnNormalCacheSize < mnMaximalNormalCacheSize)
                mbIsFull = false;
            break;

        default:
            assert(false);
            break;
    }
}

//===== CacheEntry ============================================================

BitmapCache::CacheEntry::CacheEntry(
    sal_Int32 nLastAccessTime,
    bool bIsPrecious)
    : maPreview(),
      maMarkedPreview(),
      mbIsUpToDate(true),
      mnLastAccessTime(nLastAccessTime),
      mbIsPrecious(bIsPrecious)
{
}

BitmapCache::CacheEntry::CacheEntry(
    const BitmapEx& rPreview,
    sal_Int32 nLastAccessTime,
    bool bIsPrecious)
    : maPreview(rPreview),
      maMarkedPreview(),
      mbIsUpToDate(true),
      mnLastAccessTime(nLastAccessTime),
      mbIsPrecious(bIsPrecious)
{
}

inline void BitmapCache::CacheEntry::Recycle (const CacheEntry& rEntry)
{
    if ((rEntry.HasPreview() || rEntry.HasLosslessReplacement())
        && ! (HasPreview() || HasLosslessReplacement()))
    {
        maPreview = rEntry.maPreview;
        maMarkedPreview = rEntry.maMarkedPreview;
        mpReplacement = rEntry.mpReplacement;
        mpCompressor = rEntry.mpCompressor;
        mnLastAccessTime = rEntry.mnLastAccessTime;
        mbIsUpToDate = rEntry.mbIsUpToDate;
    }
}

inline sal_Int32 BitmapCache::CacheEntry::GetMemorySize() const
{
    sal_Int32 nSize (0);
    nSize += maPreview.GetSizeBytes();
    nSize += maMarkedPreview.GetSizeBytes();
    if (mpReplacement != nullptr)
        nSize += mpReplacement->GetMemorySize();
    return nSize;
}

void BitmapCache::CacheEntry::Compress (const std::shared_ptr<BitmapCompressor>& rpCompressor)
{
    if (  maPreview.IsEmpty())
        return;

    if (mpReplacement == nullptr)
    {
        mpReplacement = rpCompressor->Compress(maPreview);

#ifdef DEBUG_SD_SLSBITMAPCACHE
        sal_uInt32 nOldSize (maPreview.GetSizeBytes());
        sal_uInt32 nNewSize (mpReplacement.get()!=NULL ? mpReplacement->GetMemorySize() : 0);
        if (nOldSize == 0)
            nOldSize = 1;
        sal_Int32 nRatio (100L * nNewSize / nOldSize);
        SAL_INFO("sd.sls", OSL_THIS_FUNC << ": compressing bitmap for " << %x << " from " << nOldSize << " to " << nNewSize << " bytes (" << nRatio << "%)");
#endif

        mpCompressor = rpCompressor;
    }

    maPreview.SetEmpty();
    maMarkedPreview.SetEmpty();
}

inline void BitmapCache::CacheEntry::Decompress()
{
    if (mpReplacement != nullptr && mpCompressor != nullptr && maPreview.IsEmpty())
    {
        maPreview = mpCompressor->Decompress(*mpReplacement);
        maMarkedPreview.SetEmpty();
        if ( ! mpCompressor->IsLossless())
            mbIsUpToDate = false;
    }
}

inline void BitmapCache::CacheEntry::SetPreview (const BitmapEx& rPreview)
{
    maPreview = rPreview;
    maMarkedPreview.SetEmpty();
    mpReplacement.reset();
    mpCompressor.reset();
}

bool BitmapCache::CacheEntry::HasPreview() const
{
    return ! maPreview.IsEmpty();
}

inline void BitmapCache::CacheEntry::SetMarkedPreview (const BitmapEx& rMarkedPreview)
{
    maMarkedPreview = rMarkedPreview;
}

inline bool BitmapCache::CacheEntry::HasLosslessReplacement() const
{
    return mpReplacement != nullptr && mpCompressor != nullptr && mpCompressor->IsLossless();
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
