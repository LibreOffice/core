/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsBitmapCache.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:16:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SlsBitmapCache.hxx"
#include "SlsCacheCompactor.hxx"
#include "SlsBitmapCompressor.hxx"
#include "SlsCacheConfiguration.hxx"

#include "taskpane/SlideSorterCacheDisplay.hxx"
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
    CacheEntry(const ::boost::shared_ptr<BitmapEx>& rpBitmap,
        sal_Int32 nLastAccessTime, bool bIsPrecious);
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
    ::boost::shared_ptr<BitmapEx> GetPreview (void) const { return mpPreview; }
    inline void SetPreview (const ::boost::shared_ptr<BitmapEx>& rpPreview);
    bool HasPreview (void) const { return (mpPreview.get() != NULL); }
    bool HasReplacement (void) const { return (mpReplacement.get() != NULL); }
    inline bool HasLosslessReplacement (void) const;
    void Clear (void) { mpPreview.reset(); mpReplacement.reset(); mpCompressor.reset(); }
    void Invalidate (void) { mpReplacement.reset(); mpCompressor.reset(); mbIsUpToDate = false; }
    bool IsPrecious (void) const { return mbIsPrecious; }
    void SetPrecious (bool bIsPrecious) { mbIsPrecious = bIsPrecious; }

private:
    ::boost::shared_ptr<BitmapEx> mpPreview;
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
    : public ::std::hash_map<CacheKey, CacheEntry, CacheHash>
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




bool BitmapCache::IsEmpty (void) const
{
    return mpBitmapContainer->empty();
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




::boost::shared_ptr<BitmapEx> BitmapCache::GetBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry == mpBitmapContainer->end())
    {
        // Create an empty bitmap for the given key that acts as placeholder
        // until we are given the real one.  Mark it as not being up to date.
        SetBitmap (rKey, ::boost::shared_ptr<BitmapEx>(new BitmapEx()), false);
        iEntry = mpBitmapContainer->find(rKey);
        iEntry->second.SetUpToDate(false);
        SSCD_SET_UPTODATE(iEntry->first,false);
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




void BitmapCache::InvalidateBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        iEntry->second.SetUpToDate(false);
        SSCD_SET_UPTODATE(iEntry->first,false);

        // When there is a preview then we release the replacement.  The
        // preview itself is kept until a new one is created.
        if (iEntry->second.HasPreview())
        {
            UpdateCacheSize(iEntry->second, REMOVE);
            iEntry->second.Invalidate();
            SSCD_SET_UPTODATE(iEntry->first,false);
            UpdateCacheSize(iEntry->second, ADD);
        }
    }
}




void BitmapCache::InvalidateCache (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry;
    for (iEntry=mpBitmapContainer->begin(); iEntry!=mpBitmapContainer->end(); ++iEntry)
    {
        iEntry->second.Invalidate();
        SSCD_SET_UPTODATE(iEntry->first,false);
    }
    ReCalculateTotalCacheSize();
}




void BitmapCache::SetBitmap (
    const CacheKey& rKey,
    const ::boost::shared_ptr<BitmapEx>& rpPreview,
    bool bIsPrecious)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator iEntry (mpBitmapContainer->find(rKey));
    if (iEntry != mpBitmapContainer->end())
    {
        UpdateCacheSize(iEntry->second, REMOVE);
        iEntry->second.SetPreview(rpPreview);
        iEntry->second.SetUpToDate(true);
        SSCD_SET_UPTODATE(iEntry->first,true);
        iEntry->second.SetAccessTime(mnCurrentAccessTime++);
    }
    else
    {
        iEntry = mpBitmapContainer->insert(CacheBitmapContainer::value_type (
            rKey,
            CacheEntry (rpPreview, mnCurrentAccessTime++, bIsPrecious))
            ).first;
    }

    if (iEntry != mpBitmapContainer->end())
        UpdateCacheSize(iEntry->second, ADD);
}




bool BitmapCache::IsPrecious (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (mpBitmapContainer->find(rKey));
    if (aIterator != mpBitmapContainer->end())
        return aIterator->second.IsPrecious();
    else
        return false;
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
            CacheEntry (::boost::shared_ptr<BitmapEx>(new BitmapEx()),
                mnCurrentAccessTime++, bIsPrecious))).first;
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
    : mpPreview(),
      mbIsUpToDate(true),
      mnLastAccessTime(nLastAccessTime),
      mbIsPrecious(bIsPrecious)
{
}




BitmapCache::CacheEntry::CacheEntry(
    const ::boost::shared_ptr<BitmapEx>& rpPreview,
    sal_Int32 nLastAccessTime,
    bool bIsPrecious)
    : mpPreview(rpPreview),
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
        mpPreview = rEntry.mpPreview;
        mpReplacement = rEntry.mpReplacement;
        mpCompressor = rEntry.mpCompressor;
        mnLastAccessTime = rEntry.mnLastAccessTime;
        mbIsUpToDate = rEntry.mbIsUpToDate;
    }
}




inline sal_Int32 BitmapCache::CacheEntry::GetMemorySize (void) const
{
    sal_Int32 nSize (0);
    if (mpPreview.get() != NULL)
        nSize += mpPreview->GetSizeBytes();
    if (mpReplacement.get() != NULL)
        nSize += mpReplacement->GetMemorySize();
    return nSize;
}




void BitmapCache::CacheEntry::Compress (const ::boost::shared_ptr<BitmapCompressor>& rpCompressor)
{
    if (mpPreview.get() != NULL)
    {
        if (mpReplacement.get() == NULL)
        {
            mpReplacement = rpCompressor->Compress(mpPreview);

#ifdef VERBOSE
            sal_uInt32 nOldSize (mpPreview->GetSizeBytes());
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

        mpPreview.reset();
    }
}




inline void BitmapCache::CacheEntry::Decompress (void)
{
    if (mpReplacement.get()!=NULL && mpCompressor.get()!=NULL && mpPreview.get()==NULL)
    {
        mpPreview = mpCompressor->Decompress(*mpReplacement);
        if ( ! mpCompressor->IsLossless())
            mbIsUpToDate = false;
    }
}



inline void BitmapCache::CacheEntry::SetPreview (const ::boost::shared_ptr<BitmapEx>& rpPreview)
{
    mpPreview = rpPreview;
    mpReplacement.reset();
    mpCompressor.reset();
}




inline bool BitmapCache::CacheEntry::HasLosslessReplacement (void) const
{
    return mpReplacement.get()!=NULL
        && mpCompressor.get()!=NULL
        && mpCompressor->IsLossless();
}


} } } // end of namespace ::sd::slidesorter::cache
