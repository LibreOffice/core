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

#include "SlsBitmapCache.hxx"
#include "SlsCacheCompactor.hxx"
#include "SlsBitmapCompressor.hxx"
#include "SlsCacheConfiguration.hxx"

#include "sdpage.hxx"
#include "drawdoc.hxx"

// Uncomment the following define for some more OSL_TRACE messages.
#ifdef DEBUG
//#define VERBOSE
#endif

// Define the default value for the maximal cache size that is used for
// previews that are currently not visible.  The visible previews are all
// held in memory at all times.  This default is used only when the
// configuration does not have a value.
static const sal_Int32 MAXIMAL_CACHE_SIZE = 4L*1024L*1024L;

using namespace ::com::sun::star::uno;

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache::CacheEntry
{
public:
    CacheEntry(const Bitmap& rBitmap, sal_Int32 nLastAccessTime, bool bIsPrecious);
    CacheEntry(sal_Int32 nLastAccessTime, bool bIsPrecious);
    ~CacheEntry (void) {};
    inline void Recycle (const CacheEntry& rEntry);
    inline sal_Int32 GetMemorySize (void) const;
    void Compress (const ::boost::shared_ptr<BitmapCompressor>& rpCompressor);
    inline void Decompress (void);

    bool IsUpToDate (void) const { return mbIsUpToDate; }
    void SetUpToDate (bool bIsUpToDate) { mbIsUpToDate = bIsUpToDate; }
    sal_Int32 GetAccessTime (void) const { return mnLastAccessTime; }
    void SetAccessTime (sal_Int32 nAccessTime) { mnLastAccessTime = nAccessTime; }

    Bitmap GetPreview (void) const { return maPreview; }
    inline void SetPreview (const Bitmap& rPreview);
    bool HasPreview (void) const;

    Bitmap GetMarkedPreview (void) const { return maMarkedPreview; }
    inline void SetMarkedPreview (const Bitmap& rMarkePreview);
    bool HasMarkedPreview (void) const;

    bool HasReplacement (void) const { return (mpReplacement.get() != NULL); }
    inline bool HasLosslessReplacement (void) const;
    void Clear (void) { maPreview.SetEmpty(); maMarkedPreview.SetEmpty();
        mpReplacement.reset(); mpCompressor.reset(); }
    void Invalidate (void) { mpReplacement.reset(); mpCompressor.reset(); mbIsUpToDate = false; }
    bool IsPrecious (void) const { return mbIsPrecious; }
    void SetPrecious (bool bIsPrecious) { mbIsPrecious = bIsPrecious; }

private:
    Bitmap maPreview;
    Bitmap maMarkedPreview;
    ::boost::shared_ptr<BitmapReplacement> mpReplacement;
    ::boost::shared_ptr<BitmapCompressor> mpCompressor;
    Size maBitmapSize;
    bool mbIsUpToDate;
    sal_Int32 mnLastAccessTime;
    // When this flag is set then the bitmap is not modified by a cache
    // compactor.
    bool mbIsPrecious;
};
class CacheEntry;

class CacheHash {
public:
    size_t operator()(const BitmapCache::CacheKey& p) const
    { return (size_t)p; }
};

class BitmapCache::CacheBitmapContainer
    : public ::boost::unordered_map<CacheKey, CacheEntry, CacheHash>
{
public:
    CacheBitmapContainer (void) {}
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

BitmapCache::BitmapCache (const sal_Int32 nMaximalNormalCacheSize)
    : maMutex(),
      mpBitmapContainer(new CacheBitmapContainer()),
      mnNormalCacheSize(0),
      mnPreciousCacheSize(0),
      mnCurrentAccessTime(0),
      mnMaximalNormalCacheSize(MAXIMAL_CACHE_SIZE),
      mpCacheCompactor(),
      mbIsFull(false)
{
    if (nMaximalNormalCacheSize > 0)
        mnMaximalNormalCacheSize = nMaximalNormalCacheSize;
    else
    {
        Any aCacheSize (CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CacheSize"))));
        if (aCacheSize.has<sal_Int32>())
            aCacheSize >>= mnMaximalNormalCacheSize;
    }

    mpCacheCompactor = CacheCompactor::Create(*this,mnMaximalNormalCacheSize);
}




BitmapCache::~BitmapCache (void)
{
    Clear();
}




void BitmapCache::Clear (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    mpBitmapContainer->clear();
    mnNormalCacheSize = 0;
    mnPreciousCacheSize = 0;
    mnCurrentAccessTime = 0;
}




bool BitmapCache::IsFull (void) const
{
    return mbIsFull;
}




sal_Int32 BitmapCache::GetSize (void)
{
    return mnNormalCacheSize;
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




Bitmap BitmapCache::GetBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry == mpBitmapContainer->end())
    {
        // Create an empty bitmap for the given key that acts as placeholder
        // until we are given the real one.  Mark it as not being up to date.
        SetBitmap(rKey, Bitmap(), false);
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




Bitmap BitmapCache::GetMarkedBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        iEntry->second.SetAccessTime(mnCurrentAccessTime++);
        return iEntry->second.GetMarkedPreview();
    }
    else
        return Bitmap();
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




void BitmapCache::InvalidateCache (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry;
    for (iEntry=mpBitmapContainer->begin(); iEntry!=mpBitmapContainer->end(); ++iEntry)
    {
        iEntry->second.Invalidate();
    }
    ReCalculateTotalCacheSize();
}




void BitmapCache::SetBitmap (
    const CacheKey& rKey,
    const Bitmap& rPreview,
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
        iEntry = mpBitmapContainer->insert(CacheBitmapContainer::value_type (
            rKey,
            CacheEntry(rPreview, mnCurrentAccessTime++, bIsPrecious))
            ).first;
    }

    if (iEntry != mpBitmapContainer->end())
        UpdateCacheSize(iEntry->second, ADD);
}




void BitmapCache::SetMarkedBitmap (
    const CacheKey& rKey,
    const Bitmap& rPreview)
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
        iEntry = mpBitmapContainer->insert(CacheBitmapContainer::value_type (
            rKey,
            CacheEntry(Bitmap(), mnCurrentAccessTime++, bIsPrecious))
            ).first;
        UpdateCacheSize(iEntry->second, ADD);
    }
}




void BitmapCache::ReCalculateTotalCacheSize (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    mnNormalCacheSize = 0;
    mnPreciousCacheSize = 0;
    CacheBitmapContainer::iterator iEntry;
    for (iEntry=mpBitmapContainer->begin(); iEntry!=mpBitmapContainer->end();  ++iEntry)
    {
        if (iEntry->second.IsPrecious())
            mnPreciousCacheSize += iEntry->second.GetMemorySize();
        else
            mnNormalCacheSize += iEntry->second.GetMemorySize();
    }
    mbIsFull = (mnNormalCacheSize  >= mnMaximalNormalCacheSize);

#ifdef VERBOSE
    OSL_TRACE("cache size is %d/%d", mnNormalCacheSize, mnPreciousCacheSize);
#endif
}




void BitmapCache::Recycle (const BitmapCache& rCache)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::const_iterator iOtherEntry;
    for (iOtherEntry=rCache.mpBitmapContainer->begin();
         iOtherEntry!=rCache.mpBitmapContainer->end();
         ++iOtherEntry)
    {
        CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(iOtherEntry->first));
        if (iEntry == mpBitmapContainer->end())
        {
            iEntry = mpBitmapContainer->insert(CacheBitmapContainer::value_type (
                iOtherEntry->first,
                CacheEntry(mnCurrentAccessTime++, true))
                ).first;
            UpdateCacheSize(iEntry->second, ADD);
        }
        if (iEntry != mpBitmapContainer->end())
        {
            UpdateCacheSize(iEntry->second, REMOVE);
            iEntry->second.Recycle(iOtherEntry->second);
            UpdateCacheSize(iEntry->second, ADD);
        }
    }
}




::std::auto_ptr<BitmapCache::CacheIndex> BitmapCache::GetCacheIndex (
    bool bIncludePrecious,
    bool bIncludeNoPreview) const
{
    ::osl::MutexGuard aGuard (maMutex);

    // Create a copy of the bitmap container.
    SortableBitmapContainer aSortedContainer;
    aSortedContainer.reserve(mpBitmapContainer->size());

    // Copy the relevant entries.
    CacheBitmapContainer::iterator iEntry;
    for (iEntry=mpBitmapContainer->begin(); iEntry!=mpBitmapContainer->end(); ++iEntry)
    {
        if ( ! bIncludePrecious && iEntry->second.IsPrecious())
            continue;

        if ( ! bIncludeNoPreview && ! iEntry->second.HasPreview())
            continue;

        aSortedContainer.push_back(SortableBitmapContainer::value_type(
            iEntry->first,iEntry->second));
    }

    // Sort the remaining entries.
    ::std::sort(aSortedContainer.begin(), aSortedContainer.end(), AccessTimeComparator());

    // Return a list with the keys of the sorted entries.
    ::std::auto_ptr<CacheIndex> pIndex(new CacheIndex());
    SortableBitmapContainer::iterator iIndexEntry;
    pIndex->reserve(aSortedContainer.size());
    for (iIndexEntry=aSortedContainer.begin(); iIndexEntry!=aSortedContainer.end(); ++iIndexEntry)
        pIndex->push_back(iIndexEntry->first);
    return pIndex;
}




void BitmapCache::Compress (
    const CacheKey& rKey,
    const ::boost::shared_ptr<BitmapCompressor>& rpCompressor)
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
#ifdef VERBOSE
                OSL_TRACE("cache size is %d > %d", mnNormalCacheSize,mnMaximalNormalCacheSize);
#endif
                mpCacheCompactor->RequestCompaction();
            }
            break;

        case REMOVE:
            rCacheSize -= nEntrySize;
            if (mnNormalCacheSize < mnMaximalNormalCacheSize)
                mbIsFull = false;
            break;

        default:
            OSL_ASSERT(false);
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
    const Bitmap& rPreview,
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




inline sal_Int32 BitmapCache::CacheEntry::GetMemorySize (void) const
{
    sal_Int32 nSize (0);
    nSize += maPreview.GetSizeBytes();
    nSize += maMarkedPreview.GetSizeBytes();
    if (mpReplacement.get() != NULL)
        nSize += mpReplacement->GetMemorySize();
    return nSize;
}




void BitmapCache::CacheEntry::Compress (const ::boost::shared_ptr<BitmapCompressor>& rpCompressor)
{
    if ( ! maPreview.IsEmpty())
    {
        if (mpReplacement.get() == NULL)
        {
            mpReplacement = rpCompressor->Compress(maPreview);

#ifdef VERBOSE
            sal_uInt32 nOldSize (maPreview.GetSizeBytes());
            sal_uInt32 nNewSize (mpReplacement.get()!=NULL ? mpReplacement->GetMemorySize() : 0);
            if (nOldSize == 0)
                nOldSize = 1;
            sal_Int32 nRatio (100L * nNewSize / nOldSize);
            OSL_TRACE("compressing bitmap for %x from %d to %d bytes (%d%%)",
                this,
                nOldSize,
                nNewSize,
                nRatio);
#endif

            mpCompressor = rpCompressor;
        }

        maPreview.SetEmpty();
        maMarkedPreview.SetEmpty();
    }
}




inline void BitmapCache::CacheEntry::Decompress (void)
{
    if (mpReplacement.get()!=NULL && mpCompressor.get()!=NULL && maPreview.IsEmpty())
    {
        maPreview = mpCompressor->Decompress(*mpReplacement);
        maMarkedPreview.SetEmpty();
        if ( ! mpCompressor->IsLossless())
            mbIsUpToDate = false;
    }
}



inline void BitmapCache::CacheEntry::SetPreview (const Bitmap& rPreview)
{
    maPreview = rPreview;
    maMarkedPreview.SetEmpty();
    mpReplacement.reset();
    mpCompressor.reset();
}




bool BitmapCache::CacheEntry::HasPreview (void) const
{
    return ! maPreview.IsEmpty();
}




inline void BitmapCache::CacheEntry::SetMarkedPreview (const Bitmap& rMarkedPreview)
{
    maMarkedPreview = rMarkedPreview;
}




bool BitmapCache::CacheEntry::HasMarkedPreview (void) const
{
    return ! maMarkedPreview.IsEmpty();
}




inline bool BitmapCache::CacheEntry::HasLosslessReplacement (void) const
{
    return mpReplacement.get()!=NULL
        && mpCompressor.get()!=NULL
        && mpCompressor->IsLossless();
}


} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
