/*************************************************************************
 *
 *  $RCSfile: SlsBitmapCache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:09:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "SlsBitmapCache.hxx"

namespace sd { namespace slidesorter { namespace cache {


//=====  BitmapCache  =========================================================

BitmapCache::BitmapCache (void)
    : mnSize (0),
      mnCurrentAccessTime(0)
{
}




BitmapCache::~BitmapCache (void)
{
    Clear();
}




void BitmapCache::Clear (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    maBitmapContainer.clear();
    mnSize = 0;
    mnCurrentAccessTime = 0;
}




sal_Int32 BitmapCache::GetSize (void)
{
    return mnSize;
}




bool BitmapCache::HasBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    return (maBitmapContainer.find(rKey) != maBitmapContainer.end());
}




bool BitmapCache::BitmapIsUpToDate (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    bool bIsUpToDate = false;
    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator != maBitmapContainer.end())
        bIsUpToDate = aIterator->second.mbIsUpToDate;

    return bIsUpToDate;
}




const BitmapEx& BitmapCache::GetBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator == maBitmapContainer.end())
    {
        SetBitmap (rKey, BitmapEx(), false);
        aIterator = maBitmapContainer.find(rKey);
    }
    else
        aIterator->second.mnLastAccessTime = mnCurrentAccessTime++;
    return aIterator->second.maBitmap;
}




void BitmapCache::ReleaseBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator != maBitmapContainer.end())
    {
        mnSize -= aIterator->second.maBitmap.GetSizeBytes();
        maBitmapContainer.erase (aIterator);
    }
}




void BitmapCache::InvalidateBitmap (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator != maBitmapContainer.end())
    {
        aIterator->second.mbIsUpToDate = false;
    }
}




void BitmapCache::SetBitmap (
    const CacheKey& rKey,
    const BitmapEx& rBitmap,
    bool bIsPrecious)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator != maBitmapContainer.end())
    {
        // Here we technically first insert the bitmap into the cache and
        // then make room for it.
        mnSize -= aIterator->second.maBitmap.GetSizeBytes();
        aIterator->second.maBitmap = rBitmap;
        aIterator->second.mbIsUpToDate = true;
        aIterator->second.mnLastAccessTime = mnCurrentAccessTime++;
    }
    else
    {
        maBitmapContainer.insert (CacheBitmapContainer::value_type (
            rKey,
            CacheEntry (rBitmap, mnCurrentAccessTime++, bIsPrecious)));
    }
    mnSize += rBitmap.GetSizeBytes();

    OSL_TRACE ("bitmap cache occupies %d bytes for %d bitmaps",
        mnSize, maBitmapContainer.size());
}




bool BitmapCache::IsPrecious (const CacheKey& rKey)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator != maBitmapContainer.end())
        return aIterator->second.mbIsPrecious;
    else
        return false;
}




void BitmapCache::SetPrecious (const CacheKey& rKey, bool bIsPrecious)
{
    ::osl::MutexGuard aGuard (maMutex);

    CacheBitmapContainer::iterator aIterator (maBitmapContainer.find(rKey));
    if (aIterator != maBitmapContainer.end())
        aIterator->second.mbIsPrecious = bIsPrecious;
    else if (bIsPrecious)
        maBitmapContainer.insert (CacheBitmapContainer::value_type (
            rKey,
            CacheEntry (BitmapEx(), mnCurrentAccessTime++, bIsPrecious)));
}




const CacheBitmapContainer& BitmapCache::GetContainer (void) const
{
    return maBitmapContainer;
}




} } } // end of namespace ::sd::slidesorter::cache
