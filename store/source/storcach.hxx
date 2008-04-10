/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: storcach.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _STORE_STORCACH_HXX
#define _STORE_STORCACH_HXX "$Revision: 1.6 $"

#include <sal/types.h>
#include <osl/mutex.hxx>
#include <store/types.h>

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif

namespace store
{

struct OStorePageDescriptor;
struct OStorePageData;
class  OStorePageBIOS;

/*========================================================================
 *
 * OStorePageCache interface.
 * (OStorePageData in external representation)
 *
 *======================================================================*/
#define STORE_LIMIT_CACHEPAGES   256
#define STORE_DEFAULT_CACHEPAGES STORE_LIMIT_CACHEPAGES

struct OStorePageCacheEntry;

class OStorePageCache
{
    typedef OStorePageCacheEntry entry;

public:
    /** Allocation.
     */
    static void * operator new (std::size_t n) SAL_THROW(());
    static void   operator delete (void * p, std::size_t) SAL_THROW(());

    /** Construction.
    */
    OStorePageCache (
        sal_uInt16 nPages = STORE_DEFAULT_CACHEPAGES);

    /** Destruction.
    */
    ~OStorePageCache (void);

    /** load.
    */
    storeError load (
        const OStorePageDescriptor &rDescr,
        OStorePageData             &rData,
        OStorePageBIOS             &rBIOS,
        osl::Mutex                 *pMutex = NULL);

    /** update.
    */
    enum UpdateMode
    {
        UPDATE_WRITE_THROUGH = 0,
        UPDATE_WRITE_DELAYED = 1
    };

    storeError update (
        const OStorePageDescriptor &rDescr,
        const OStorePageData       &rData,
        OStorePageBIOS             &rBIOS,
        osl::Mutex                 *pMutex = NULL,
        UpdateMode                  eMode  = UPDATE_WRITE_THROUGH);

    /** invalidate.
    */
    storeError invalidate (
        const OStorePageDescriptor &rDescr,
        osl::Mutex                 *pMutex = NULL);

    /** flush.
    */
    storeError flush (
        OStorePageBIOS &rBIOS,
        osl::Mutex     *pMutex = NULL);

    /** hitRatio [nHit / (nHit + nMissed)].
     */
    inline double hitRatio (void) const;

    /** usageRatio [nUsed / nSize].
    */
    inline double usageRatio (void) const;

private:
    /** Representation.
    */
    sal_uInt16  m_nSize;
    sal_uInt16  m_nUsed;
    entry      *m_pData[STORE_LIMIT_CACHEPAGES];
    entry      *m_pHead;

    sal_uInt32  m_nHit;
    sal_uInt32  m_nMissed;
    sal_uInt32  m_nUpdHit;
    sal_uInt32  m_nUpdLRU;
    sal_uInt32  m_nWrtBack;

    /** Implementation.
    */
    sal_uInt16 find (const OStorePageDescriptor &rDescr) const;
    void       move (sal_uInt16 nSI, sal_uInt16 nDI);

    /** insert.
    */
    enum InsertMode
    {
        INSERT_CLEAN = 0,
        INSERT_DIRTY = 1
    };

    storeError insert (
        sal_uInt16                  nIndex,
        const OStorePageDescriptor &rDescr,
        const OStorePageData       &rData,
        OStorePageBIOS             &rBIOS,
        InsertMode                  eMode = INSERT_CLEAN);

    /** Not implemented.
    */
    OStorePageCache (const OStorePageCache& rOther);
    OStorePageCache& operator= (const OStorePageCache& rOther);
};

/*
 * hitRatio [nHit / (nHit + nMissed)].
 */
inline double OStorePageCache::hitRatio (void) const
{
    if (m_nHit || m_nMissed)
        return ((double)m_nHit / (double)(m_nHit + m_nMissed));
    else
        return 1.0;
}

/*
 * usageRatio [nUsed / nSize].
 */
inline double OStorePageCache::usageRatio (void) const
{
    if (m_nUsed < m_nSize)
        return ((double)m_nUsed / (double)m_nSize);
    else
        return 1.0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_STORCACH_HXX */

