/*************************************************************************
 *
 *  $RCSfile: SlsBitmapCache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:09:31 $
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

#ifndef SD_SLIDESORTER_BITMAP_CACHE_HXX
#define SD_SLIDESORTER_BITMAP_CACHE_HXX

class SdrPage;

#include <vcl/bitmapex.hxx>
#include <osl/mutex.hxx>
#include <hash_map>

namespace sd { namespace slidesorter { namespace cache {


/** The key for looking up preview bitmaps is a pointer to an SdrPage
    object.  The prior use of PageObjectViewObjectContact objects (which
    ultimatly use them) turned out to be less suitable because their
    life time is shorter then that of the page objects.  Frequent
    destruction and re-creation of the preview bitmaps was the result.
    */
typedef const SdrPage* CacheKey;


class CacheHash {
public:
    size_t operator()(const CacheKey& p) const
    { return (size_t)p; }
};


class CacheEntry {
public:
    CacheEntry(
        const BitmapEx& rBitmap,
        sal_Int32 nLastAccessTime,
        bool bIsPrecious)
        : maBitmap(rBitmap),
          mbIsUpToDate(true),
          mnLastAccessTime(nLastAccessTime),
          mbIsPrecious(bIsPrecious)
    {}
    BitmapEx maBitmap;
    bool mbIsUpToDate;
    sal_Int32 mnLastAccessTime;
    // When this flag is set then the bitmap is not modified by a cache
    // compactor.
    bool mbIsPrecious;
};


typedef ::std::hash_map<CacheKey, CacheEntry, CacheHash> CacheBitmapContainer;




class BitmapCache
{
public:
    /** Create a new cache for bitmap objects.
    */
    BitmapCache (void);

    /** The destructor clears the cache and relases all bitmaps still in it.
    */
    ~BitmapCache (void);

    /** Remove all preview bitmaps from the cache.  After this call the
        cache is empty.
    */
    void Clear (void);

    /** Return the memory size that is occupied by all bitmaps in the cache.
    */
    sal_Int32 GetSize (void);

    bool HasBitmap (const CacheKey& rKey);

    bool BitmapIsUpToDate (const CacheKey& rKey);

    /** Return the preview bitmap for the given contact object.
    */
    const BitmapEx& GetBitmap (const CacheKey& rKey);

    void ReleaseBitmap (const CacheKey& rKey);

    void InvalidateBitmap (const CacheKey& rKey);

    /** Add or replace a bitmap for the given key.
    */
    void SetBitmap (
        const CacheKey& rKey,
        const BitmapEx& rBitmap,
        bool bIsPrecious);

    bool IsPrecious (const CacheKey& rKey);
    void SetPrecious (const CacheKey& rKey, bool bIsPrecious);

    const CacheBitmapContainer& GetContainer (void) const;

private:
    ::osl::Mutex maMutex;

    CacheBitmapContainer maBitmapContainer;

    /** Total size of bytes that are occupied by the bitmaps in the cache.
        */
    sal_Int32 mnSize;

    /** At the moment the access time is not an actual time or date value
        but a counter that is increased with every access.  It thus defines
        the same ordering as a true time.
    */
    sal_Int32 mnCurrentAccessTime;
};



} } } // end of namespace ::sd::slidesorter::cache

#endif
