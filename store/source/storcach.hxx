/*************************************************************************
 *
 *  $RCSfile: storcach.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:32 $
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

#ifndef _STORE_STORCACH_HXX
#define _STORE_STORCACH_HXX "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif
#ifndef _STORE_MACROS_HXX_
#include <store/macros.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace store {
#endif

struct OStorePageDescriptor;
struct OStorePageData;
class  OStorePageBIOS;

/*========================================================================
 *
 * OStorePageCache interface.
 * (OStorePageData in external representation)
 *
 *======================================================================*/
#define STORE_DEFAULT_CACHEPAGES 128
#define STORE_LIMIT_CACHEPAGES   256

struct OStorePageCacheEntry;

class OStorePageCache
{
    typedef OStorePageCacheEntry entry;

public:
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
        NAMESPACE_VOS(IMutex)      *pMutex = NULL);

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
        NAMESPACE_VOS(IMutex)      *pMutex = NULL,
        UpdateMode                  eMode  = UPDATE_WRITE_THROUGH);

    /** invalidate.
    */
    storeError invalidate (
        const OStorePageDescriptor &rDescr,
        NAMESPACE_VOS(IMutex)      *pMutex = NULL);

    /** flush.
    */
    storeError flush (
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

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
#ifdef _USE_NAMESPACE
}
#endif

#endif /* !_STORE_STORCACH_HXX */

