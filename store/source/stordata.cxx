/*************************************************************************
 *
 *  $RCSfile: stordata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 20:59:02 $
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
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _STORE_STORDATA_CXX_ "$Revision: 1.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif
#ifndef _STORE_STORDATA_HXX_
#include <stordata.hxx>
#endif

using namespace store;

/*========================================================================
 *
 * OStoreIndirectionPageData implementation.
 *
 *======================================================================*/
/*
 * OStoreIndirectionPageData.
 */
OStoreIndirectionPageData::OStoreIndirectionPageData (sal_uInt16 nPageSize)
    : OStorePageData (nPageSize)
{
    initialize();
}

/*
 * initialize.
 */
void OStoreIndirectionPageData::initialize (void)
{
    base::m_aGuard.m_nMagic = STORE_MAGIC_INDIRECTPAGE;
    base::m_aDescr.m_nUsed += self::size();
    self::m_aGuard.m_nMagic = 0;

    sal_uInt16 i, n = capacityCount();
    for (i = 0; i < n; i++)
        m_pData[i] = STORE_PAGE_NULL;
}

/*
 * swap.
 */
void OStoreIndirectionPageData::swap (const D& rDescr)
{
#ifdef OSL_BIGENDIAN
    m_aGuard.swap();

    sal_uInt16 i, n = capacityCount (rDescr);
    for (i = 0; i < n; i++)
        m_pData[i] = OSL_SWAPDWORD(m_pData[i]);
#endif /* OSL_BIGENDIAN */
}

/*========================================================================
 *
 * OStoreIndirectionPageObject implementation.
 *
 *======================================================================*/
/*
 * swap.
 */
void OStoreIndirectionPageObject::swap (const D& rDescr)
{
#ifdef OSL_BIGENDIAN
    base::swap (rDescr);
    m_rPage.swap (rDescr);
#endif /* OSL_BIGENDIAN */
}

/*
 * guard.
 */
void OStoreIndirectionPageObject::guard (const D& rDescr)
{
    base::guard (rDescr);
    m_rPage.guard (rDescr);
}

/*
 * verify.
 */
storeError OStoreIndirectionPageObject::verify (const D& rDescr)
{
    storeError eErrCode = base::verify (rDescr);
    if (eErrCode != store_E_None)
        return eErrCode;
    else
        return m_rPage.verify (rDescr);
}

/*
 * get (single indirect).
 */
storeError OStoreIndirectionPageObject::get (
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    if (!(nSingle < m_rPage.capacityCount()))
        STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);

    // Obtain data page location.
    sal_uInt32 nAddr = m_rPage.m_pData[nSingle];
    if (nAddr == STORE_PAGE_NULL)
        STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

    // Load data page.
    rData.location (nAddr);
    storeError eErrCode = rBIOS.load (rData);

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * get (double indirect).
 */
storeError OStoreIndirectionPageObject::get (
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    page                 *&rpSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    if (!((nDouble < m_rPage.capacityCount()) &&
          (nSingle < m_rPage.capacityCount())    ))
        STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);

    // Check single indirect page location.
    if (m_rPage.m_pData[nDouble] == STORE_PAGE_NULL)
        STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

    // Check single indirect page buffer.
    if (rpSingle == NULL)
    {
        sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
        rpSingle = new(nPageSize) page(nPageSize);
    }

    // Load single indirect page.
    OStoreIndirectionPageObject aSingle (*rpSingle);
    aSingle.location (m_rPage.m_pData[nDouble]);

    storeError eErrCode = rBIOS.load (aSingle);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Get single indirect.
    eErrCode = aSingle.get (nSingle, rData, rBIOS, NULL);

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * get (triple indirect).
 */
storeError OStoreIndirectionPageObject::get (
    sal_uInt16             nTriple,
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    page                 *&rpDouble,
    page                 *&rpSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    if (!((nTriple < m_rPage.capacityCount()) &&
          (nDouble < m_rPage.capacityCount()) &&
          (nSingle < m_rPage.capacityCount())    ))
        STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);

    // Check double indirect page location.
    if (m_rPage.m_pData[nTriple] == STORE_PAGE_NULL)
        STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

    // Check double indirect page buffer.
    if (rpDouble == NULL)
    {
        sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
        rpDouble = new(nPageSize) page(nPageSize);
    }

    // Load double indirect page.
    OStoreIndirectionPageObject aDouble (*rpDouble);
    aDouble.location (m_rPage.m_pData[nTriple]);

    storeError eErrCode = rBIOS.load (aDouble);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Get double indirect.
    eErrCode = aDouble.get (nDouble, nSingle, rpSingle, rData, rBIOS, NULL);

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * put (single indirect).
 */
storeError OStoreIndirectionPageObject::put (
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    storeError eErrCode = store_E_InvalidAccess;
    if (!(nSingle < m_rPage.capacityCount()))
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Obtain data page location.
    rData.location (m_rPage.m_pData[nSingle]);
    if (rData.location() == STORE_PAGE_NULL)
    {
        // Allocate data page.
        eErrCode = rBIOS.allocate (rData);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Save data page location.
        m_rPage.m_pData[nSingle] = rData.location();
        touch();

        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);
    }
    else
    {
        // Save data page.
        eErrCode = rBIOS.save (rData);
    }

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * put (double indirect).
 */
storeError OStoreIndirectionPageObject::put (
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    page                 *&rpSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    storeError eErrCode = store_E_InvalidAccess;
    if (!((nDouble < m_rPage.capacityCount()) &&
          (nSingle < m_rPage.capacityCount())    ))
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Check single indirect page buffer.
    if (rpSingle == NULL)
    {
        sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
        rpSingle = new(nPageSize) page(nPageSize);
    }

    // Obtain single indirect page location.
    OStoreIndirectionPageObject aSingle (*rpSingle);
    aSingle.location (m_rPage.m_pData[nDouble]);
    if (aSingle.location() == STORE_PAGE_NULL)
    {
        // Initialize single indirect page buffer.
        rpSingle->initialize();

        // Allocate single indirect page.
        eErrCode = rBIOS.allocate (aSingle);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Save single indirect page location.
        m_rPage.m_pData[nDouble] = aSingle.location();
        touch();

        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);
    }
    else
    {
        // Load single indirect page.
        eErrCode = rBIOS.load (aSingle);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    // Put single indirect.
    eErrCode = aSingle.put (nSingle, rData, rBIOS, NULL);

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * put (triple indirect).
 */
storeError OStoreIndirectionPageObject::put (
    sal_uInt16             nTriple,
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    page                 *&rpDouble,
    page                 *&rpSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    storeError eErrCode = store_E_InvalidAccess;
    if (!((nTriple < m_rPage.capacityCount()) &&
          (nDouble < m_rPage.capacityCount()) &&
          (nSingle < m_rPage.capacityCount())    ))
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Check double indirect page buffer.
    if (rpDouble == NULL)
    {
        sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
        rpDouble = new(nPageSize) page(nPageSize);
    }

    // Obtain double indirect page location.
    OStoreIndirectionPageObject aDouble (*rpDouble);
    aDouble.location (m_rPage.m_pData[nTriple]);
    if (aDouble.location() == STORE_PAGE_NULL)
    {
        // Initialize double indirect page buffer.
        rpDouble->initialize();

        // Allocate double indirect page.
        eErrCode = rBIOS.allocate (aDouble);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Save double indirect page location.
        m_rPage.m_pData[nTriple] = aDouble.location();
        touch();

        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);
    }
    else
    {
        // Load double indirect page.
        eErrCode = rBIOS.load (aDouble);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    // Put double indirect.
    eErrCode = aDouble.put (nDouble, nSingle, rpSingle, rData, rBIOS, NULL);

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * truncate (single indirect).
 */
storeError OStoreIndirectionPageObject::truncate (
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    sal_uInt16 i, n = m_rPage.capacityCount();
    if (!(nSingle < n))
        STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);

    // Save PageDescriptor.
    D aDescr (m_rPage.m_aDescr);

    // Acquire Lock.
    storeError eErrCode = rBIOS.acquireLock (aDescr.m_nAddr, aDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Truncate.
    for (i = n; i > nSingle; i--)
    {
        // Obtain data page location.
        sal_uInt32 nAddr = m_rPage.m_pData[i - 1];
        if (nAddr == STORE_PAGE_NULL) continue;

        // Free data page.
        rData.location (nAddr);
        eErrCode = rBIOS.free (rData);
        if (eErrCode != store_E_None)
        {
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        // Clear pointer to data page.
        m_rPage.m_pData[i - 1] = STORE_PAGE_NULL;
        touch();
    }

    // Check for modified page.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
        {
            // Must not happen.
            OSL_TRACE("OStoreIndirectionPageObject::truncate(): save failed");

            // Release Lock and Leave.
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }
    }

    // Release Lock and Leave.
    eErrCode = rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * truncate (double indirect).
 */
storeError OStoreIndirectionPageObject::truncate (
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    page                 *&rpSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Acquire Mutex.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    if (!((nDouble < m_rPage.capacityCount()) &&
          (nSingle < m_rPage.capacityCount())    ))
        STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);

    // Save PageDescriptor.
    D aDescr (m_rPage.m_aDescr);

    // Acquire Lock.
    storeError eErrCode = rBIOS.acquireLock (aDescr.m_nAddr, aDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Truncate.
    sal_uInt16 i, n = m_rPage.capacityCount();
    for (i = n; i > nDouble + 1; i--)
    {
        // Obtain single indirect page location.
        sal_uInt32 nAddr = m_rPage.m_pData[i - 1];
        if (nAddr == STORE_PAGE_NULL) continue;

        // Check single indirect page buffer.
        if (rpSingle == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpSingle = new(nPageSize) page(nPageSize);
        }

        // Load single indirect page.
        OStoreIndirectionPageObject aSingle (*rpSingle);
        aSingle.location (nAddr);

        eErrCode = rBIOS.load (aSingle);
        if (eErrCode == store_E_None)
        {
            // Truncate to zero direct pages.
            eErrCode = aSingle.truncate (0, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }

            // Free single indirect page.
            eErrCode = rBIOS.free (aSingle);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }
        else
        {
            if (eErrCode != store_E_InvalidChecksum)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }

        // Clear pointer to single indirect page.
        m_rPage.m_pData[i - 1] = STORE_PAGE_NULL;
        touch();
    }

    // Obtain last single indirect page location.
    sal_uInt32 nAddr = m_rPage.m_pData[nDouble];
    if (nAddr != STORE_PAGE_NULL)
    {
        // Check single indirect page buffer.
        if (rpSingle == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpSingle = new(nPageSize) page(nPageSize);
        }

        // Load last single indirect page.
        OStoreIndirectionPageObject aSingle (*rpSingle);
        aSingle.location (nAddr);

        eErrCode = rBIOS.load (aSingle);
        if (eErrCode == store_E_None)
        {
            // Truncate to 'nSingle' direct pages.
            eErrCode = aSingle.truncate (nSingle, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }
        else
        {
            if (eErrCode != store_E_InvalidChecksum)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }

        // Check for complete truncation.
        if (nSingle == 0)
        {
            // Free last single indirect page.
            eErrCode = rBIOS.free (aSingle);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }

            // Clear pointer to last single indirect page.
            m_rPage.m_pData[nDouble] = STORE_PAGE_NULL;
            touch();
        }
    }

    // Check for modified page.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
        {
            // Must not happen.
            OSL_TRACE("OStoreIndirectionPageObject::truncate(): save failed");

            // Release Lock and Leave.
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }
    }

    // Release Lock and Leave.
    eErrCode = rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * truncate (triple indirect).
 */
storeError OStoreIndirectionPageObject::truncate (
    sal_uInt16             nTriple,
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    page                 *&rpDouble,
    page                 *&rpSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check arguments.
    if (!((nTriple < m_rPage.capacityCount()) &&
          (nDouble < m_rPage.capacityCount()) &&
          (nSingle < m_rPage.capacityCount())    ))
        STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);

    // Save PageDescriptor.
    D aDescr (m_rPage.m_aDescr);

    // Acquire Lock.
    storeError eErrCode = rBIOS.acquireLock (aDescr.m_nAddr, aDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Truncate.
    sal_uInt16 i, n = m_rPage.capacityCount();
    for (i = n; i > nTriple + 1; i--)
    {
        // Obtain double indirect page location.
        sal_uInt32 nAddr = m_rPage.m_pData[i - 1];
        if (nAddr == STORE_PAGE_NULL) continue;

        // Check double indirect page buffer.
        if (rpDouble == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpDouble = new(nPageSize) page(nPageSize);
        }

        // Load double indirect page.
        OStoreIndirectionPageObject aDouble (*rpDouble);
        aDouble.location (nAddr);

        eErrCode = rBIOS.load (aDouble);
        if (eErrCode == store_E_None)
        {
            // Truncate to zero single indirect pages.
            eErrCode = aDouble.truncate (
                0, 0, rpSingle, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }

            // Free double indirect page.
            eErrCode = rBIOS.free (aDouble);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }
        else
        {
            if (eErrCode != store_E_InvalidChecksum)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }

        // Clear pointer to double indirect page.
        m_rPage.m_pData[i - 1] = STORE_PAGE_NULL;
        touch();
    }

    // Obtain last double indirect page location.
    sal_uInt32 nAddr = m_rPage.m_pData[nTriple];
    if (nAddr != STORE_PAGE_NULL)
    {
        // Check double indirect page buffer.
        if (rpDouble == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpDouble = new(nPageSize) page(nPageSize);
        }

        // Load last double indirect page.
        OStoreIndirectionPageObject aDouble (*rpDouble);
        aDouble.location (nAddr);

        eErrCode = rBIOS.load (aDouble);
        if (eErrCode == store_E_None)
        {
            // Truncate to 'nDouble', 'nSingle' pages.
            eErrCode = aDouble.truncate (
                nDouble, nSingle, rpSingle, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }
        else
        {
            if (eErrCode != store_E_InvalidChecksum)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }
        }

        // Check for complete truncation.
        if ((nDouble + nSingle) == 0)
        {
            // Free last double indirect page.
            eErrCode = rBIOS.free (aDouble);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }

            // Clear pointer to last double indirect page.
            m_rPage.m_pData[nTriple] = STORE_PAGE_NULL;
            touch();
        }
    }

    // Check for modified page.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
        {
            // Must not happen.
            OSL_TRACE("OStoreIndirectionPageObject::truncate(): save failed");

            // Release Lock and Leave.
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }
    }

    // Release Lock and Leave.
    eErrCode = rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*========================================================================
 *
 * OStoreDirectoryDataBlock::LinkTable implementation.
 *
 *======================================================================*/
/*
 * LinkTable::LinkTable.
 */
OStoreDirectoryDataBlock::LinkTable::LinkTable (void)
{
    initialize();
}

/*
 * LinkTable::initialize.
 */
void OStoreDirectoryDataBlock::LinkTable::initialize (void)
{
    sal_Int32 i;
    for (i = 0; i < STORE_LIMIT_DATAPAGE_DIRECT; i++)
        m_pDirect[i] = STORE_PAGE_NULL;
    for (i = 0; i < STORE_LIMIT_DATAPAGE_SINGLE; i++)
        m_pSingle[i] = STORE_PAGE_NULL;
    for (i = 0; i < STORE_LIMIT_DATAPAGE_DOUBLE; i++)
        m_pDouble[i] = STORE_PAGE_NULL;
    for (i = 0; i < STORE_LIMIT_DATAPAGE_TRIPLE; i++)
        m_pTriple[i] = STORE_PAGE_NULL;
}

/*
 * LinkTable::swap.
 */
void OStoreDirectoryDataBlock::LinkTable::swap (void)
{
#ifdef OSL_BIGENDIAN
    sal_Int32 i;
    for (i = 0; i < STORE_LIMIT_DATAPAGE_DIRECT; i++)
        m_pDirect[i] = OSL_SWAPDWORD(m_pDirect[i]);
    for (i = 0; i < STORE_LIMIT_DATAPAGE_SINGLE; i++)
        m_pSingle[i] = OSL_SWAPDWORD(m_pSingle[i]);
    for (i = 0; i < STORE_LIMIT_DATAPAGE_DOUBLE; i++)
        m_pDouble[i] = OSL_SWAPDWORD(m_pDouble[i]);
    for (i = 0; i < STORE_LIMIT_DATAPAGE_TRIPLE; i++)
        m_pTriple[i] = OSL_SWAPDWORD(m_pTriple[i]);
#endif /* OSL_BIGENDIAN */
}

/*========================================================================
 *
 * OStoreDirectoryPageObject implementation.
 *
 *======================================================================*/
/*
 * swap.
 */
void OStoreDirectoryPageObject::swap (const D& rDescr)
{
#ifdef OSL_BIGENDIAN
    base::swap (rDescr);
    m_rPage.swap (rDescr);
#endif /* OSL_BIGENDIAN */
}

/*
 * guard.
 */
void OStoreDirectoryPageObject::guard (const D& rDescr)
{
    base::guard (rDescr);
    m_rPage.guard (rDescr);
}

/*
 * verify.
 */
storeError OStoreDirectoryPageObject::verify (const D& rDescr)
{
    storeError eErrCode = base::verify (rDescr);
    if (eErrCode != store_E_None)
        return eErrCode;
    else
        return m_rPage.verify (rDescr);
}

/*
 * scope (external data page).
 */
OStoreDirectoryPageData::ChunkScope
OStoreDirectoryPageObject::scope (
    sal_uInt32                       nPage,
    page::DataBlock::LinkDescriptor &rDescr) const
{
    typedef OStoreIndirectionPageData indirect;
    sal_uInt32 index0, index1, index2, index3;

    // direct.
    sal_uInt32 nCount = m_rPage.m_aDataBlock.directCount();
    sal_uInt32 nLimit = nCount;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        index0 = nPage;

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);

        // Done.
        return page::SCOPE_DIRECT;
    }
    nPage -= nLimit;

    // single indirect.
    sal_uInt32 nCapacity = indirect::capacityCount(m_rPage.m_aDescr);
    nCount = m_rPage.m_aDataBlock.singleCount();
    nLimit = nCount * nCapacity;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        sal_uInt32 n = nPage;

        // Reduce to single indirect i(1), direct n = i(0).
        index1 = n / nCapacity;
        index0 = n % nCapacity;

        // Verify reduction.
        n = index1 * nCapacity + index0;
        OSL_POSTCOND(n == nPage, "wrong math on indirect indices");
        if (n != nPage)
            return page::SCOPE_UNKNOWN;

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);
        rDescr.m_nIndex1 = (sal_uInt16)(index1 & 0xffff);

        // Done.
        return page::SCOPE_SINGLE;
    }
    nPage -= nLimit;

    // double indirect.
    nCount = m_rPage.m_aDataBlock.doubleCount();
    nLimit = nCount * nCapacity * nCapacity;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        sal_uInt32 n = nPage;

        // Reduce to double indirect i(2), single indirect n = i(0).
        index2 = n / (nCapacity * nCapacity);
        n      = n % (nCapacity * nCapacity);

        // Reduce to single indirect i(1), direct n = i(0).
        index1 = n / nCapacity;
        index0 = n % nCapacity;

        // Verify reduction.
        n = index2 * nCapacity * nCapacity +
            index1 * nCapacity + index0;
        OSL_POSTCOND(n == nPage, "wrong math on double indirect indices");
        if (n != nPage)
            return page::SCOPE_UNKNOWN;

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);
        rDescr.m_nIndex1 = (sal_uInt16)(index1 & 0xffff);
        rDescr.m_nIndex2 = (sal_uInt16)(index2 & 0xffff);

        // Done.
        return page::SCOPE_DOUBLE;
    }
    nPage -= nLimit;

    // triple indirect.
    nCount = m_rPage.m_aDataBlock.tripleCount();
    nLimit = nCount * nCapacity * nCapacity * nCapacity;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        sal_uInt32 n = nPage;

        // Reduce to triple indirect i(3), double indirect n.
        index3 = n / (nCapacity * nCapacity * nCapacity);
        n      = n % (nCapacity * nCapacity * nCapacity);

        // Reduce to double indirect i(2), single indirect n.
        index2 = n / (nCapacity * nCapacity);
        n      = n % (nCapacity * nCapacity);

        // Reduce to single indirect i(1), direct n = i(0).
        index1 = n / nCapacity;
        index0 = n % nCapacity;

        // Verify reduction.
        n = index3 * nCapacity * nCapacity * nCapacity +
            index2 * nCapacity * nCapacity +
            index1 * nCapacity + index0;
        OSL_POSTCOND(n == nPage, "wrong math on triple indirect indices");
        if (n != nPage)
            return page::SCOPE_UNKNOWN;

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);
        rDescr.m_nIndex1 = (sal_uInt16)(index1 & 0xffff);
        rDescr.m_nIndex2 = (sal_uInt16)(index2 & 0xffff);
        rDescr.m_nIndex3 = (sal_uInt16)(index3 & 0xffff);

        // Done.
        return page::SCOPE_TRIPLE;
    }

    // Unreachable (more than triple indirect).
    return page::SCOPE_UNREACHABLE;
}

/*
 * get (external data page).
 */
storeError OStoreDirectoryPageObject::get (
    sal_uInt32             nPage,
    indirect             *&rpSingle,
    indirect             *&rpDouble,
    indirect             *&rpTriple,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Determine scope and link indices.
    page::DataBlock::LinkDescriptor aLink;
    page::ChunkScope eScope = scope (nPage, aLink);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        sal_uInt32 nAddr = directLink (aLink.m_nIndex0);
        if (nAddr == STORE_PAGE_NULL)
            STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

        rData.location (nAddr);
        eErrCode = rBIOS.load (rData);
    }
    else if (eScope == page::SCOPE_SINGLE)
    {
        sal_uInt32 nAddr = singleLink (aLink.m_nIndex1);
        if (nAddr == STORE_PAGE_NULL)
            STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

        if (rpSingle == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpSingle = new(nPageSize) indirect(nPageSize);
        }

        OStoreIndirectionPageObject aSingle (*rpSingle);
        aSingle.location (nAddr);

        eErrCode = rBIOS.load (aSingle);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        eErrCode = aSingle.get (
            aLink.m_nIndex0,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_DOUBLE)
    {
        sal_uInt32 nAddr = doubleLink (aLink.m_nIndex2);
        if (nAddr == STORE_PAGE_NULL)
            STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

        if (rpDouble == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpDouble = new(nPageSize) indirect(nPageSize);
        }

        OStoreIndirectionPageObject aDouble (*rpDouble);
        aDouble.location (nAddr);

        eErrCode = rBIOS.load (aDouble);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        eErrCode = aDouble.get (
            aLink.m_nIndex1,
            aLink.m_nIndex0,
            rpSingle,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_TRIPLE)
    {
        sal_uInt32 nAddr = tripleLink (aLink.m_nIndex3);
        if (nAddr == STORE_PAGE_NULL)
            STORE_METHOD_LEAVE(pMutex, store_E_NotExists);

        if (rpTriple == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpTriple = new(nPageSize) indirect(nPageSize);
        }

        OStoreIndirectionPageObject aTriple (*rpTriple);
        aTriple.location (nAddr);

        eErrCode = rBIOS.load (aTriple);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        eErrCode = aTriple.get (
            aLink.m_nIndex2,
            aLink.m_nIndex1,
            aLink.m_nIndex0,
            rpDouble,
            rpSingle,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_UNREACHABLE)
    {
        // Out of scope.
        eErrCode = store_E_CantSeek;
    }
    else
    {
        // Unknown scope.
        OSL_TRACE("OStoreDirectoryPageObject::get(): scope failed");
        eErrCode = store_E_Unknown;
    }

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * put (external data page).
 */
storeError OStoreDirectoryPageObject::put (
    sal_uInt32             nPage,
    indirect             *&rpSingle,
    indirect             *&rpDouble,
    indirect             *&rpTriple,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Determine scope and link indices.
    page::DataBlock::LinkDescriptor aLink;
    page::ChunkScope eScope = scope (nPage, aLink);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        rData.location (directLink (aLink.m_nIndex0));
        if (rData.location() == STORE_PAGE_NULL)
        {
            eErrCode = rBIOS.allocate (rData);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            directLink (aLink.m_nIndex0, rData.location());
        }
        else
        {
            eErrCode = rBIOS.save (rData);
        }
    }
    else if (eScope == page::SCOPE_SINGLE)
    {
        if (rpSingle == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpSingle = new(nPageSize) indirect(nPageSize);
        }

        OStoreIndirectionPageObject aSingle (*rpSingle);
        aSingle.location (singleLink (aLink.m_nIndex1));
        if (aSingle.location() == STORE_PAGE_NULL)
        {
            rpSingle->initialize();

            eErrCode = rBIOS.allocate (aSingle);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            singleLink (aLink.m_nIndex1, aSingle.location());
        }
        else
        {
            eErrCode = rBIOS.load (aSingle);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        eErrCode = aSingle.put (
            aLink.m_nIndex0,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_DOUBLE)
    {
        if (rpDouble == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpDouble = new(nPageSize) indirect(nPageSize);
        }

        OStoreIndirectionPageObject aDouble (*rpDouble);
        aDouble.location (doubleLink (aLink.m_nIndex2));
        if (aDouble.location() == STORE_PAGE_NULL)
        {
            rpDouble->initialize();

            eErrCode = rBIOS.allocate (aDouble);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            doubleLink (aLink.m_nIndex2, aDouble.location());
        }
        else
        {
            eErrCode = rBIOS.load (aDouble);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        eErrCode = aDouble.put (
            aLink.m_nIndex1,
            aLink.m_nIndex0,
            rpSingle,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_TRIPLE)
    {
        if (rpTriple == NULL)
        {
            sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
            rpTriple = new(nPageSize) indirect(nPageSize);
        }

        OStoreIndirectionPageObject aTriple (*rpTriple);
        aTriple.location (tripleLink (aLink.m_nIndex3));
        if (aTriple.location() == STORE_PAGE_NULL)
        {
            rpTriple->initialize();

            eErrCode = rBIOS.allocate (aTriple);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            tripleLink (aLink.m_nIndex3, aTriple.location());
        }
        else
        {
            eErrCode = rBIOS.load (aTriple);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        eErrCode = aTriple.put (
            aLink.m_nIndex2,
            aLink.m_nIndex1,
            aLink.m_nIndex0,
            rpDouble,
            rpSingle,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_UNREACHABLE)
    {
        // Out of scope.
        eErrCode = store_E_CantSeek;
    }
    else
    {
        // Unknown scope.
        OSL_TRACE("OStoreDirectoryPageObject::put(): scope failed");
        eErrCode = store_E_Unknown;
    }

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * truncate (external data page).
 */
storeError OStoreDirectoryPageObject::truncate (
    sal_uInt32             nPage,
    indirect             *&rpSingle,
    indirect             *&rpDouble,
    indirect             *&rpTriple,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Determine scope and link indices.
    page::DataBlock::LinkDescriptor aLink;
    page::ChunkScope eScope = scope (nPage, aLink);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        // Truncate all triple indirect pages.
        eErrCode = truncate (
            page::SCOPE_TRIPLE, 0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Truncate all double indirect pages.
        eErrCode = truncate (
            page::SCOPE_DOUBLE, 0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Truncate all single indirect pages.
        eErrCode = truncate (
            page::SCOPE_SINGLE, 0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Truncate direct pages, including 'aLink.m_nIndex0'.
        eErrCode = truncate (
            eScope, aLink.m_nIndex0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
    }
    else if (eScope == page::SCOPE_SINGLE)
    {
        // Truncate all triple indirect pages.
        eErrCode = truncate (
            page::SCOPE_TRIPLE, 0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Truncate all double indirect pages.
        eErrCode = truncate (
            page::SCOPE_DOUBLE, 0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Truncate single indirect pages, downto 'aLink.m_nIndex1'.
        eErrCode = truncate (
            eScope, aLink.m_nIndex1 + 1,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Obtain last single indirect page location.
        sal_uInt32 nAddr = singleLink (aLink.m_nIndex1);
        if (nAddr != STORE_PAGE_NULL)
        {
            // Check single indirect page buffer.
            if (rpSingle == NULL)
            {
                sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
                rpSingle = new(nPageSize) indirect(nPageSize);
            }

            // Load last single indirect page.
            OStoreIndirectionPageObject aSingle (*rpSingle);
            aSingle.location (nAddr);

            eErrCode = rBIOS.load (aSingle);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            // Truncate to ... pages.
            eErrCode = aSingle.truncate (
                aLink.m_nIndex0,
                rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            // Check for complete truncation.
            if (aLink.m_nIndex0 == 0)
            {
                // Free last single indirect page.
                eErrCode = rBIOS.free (aSingle);
                if (eErrCode != store_E_None)
                    STORE_METHOD_LEAVE(pMutex, eErrCode);

                // Clear pointer to last single indirect page.
                singleLink (aLink.m_nIndex1, STORE_PAGE_NULL);
            }
        }
    }
    else if (eScope == page::SCOPE_DOUBLE)
    {
        // Truncate all triple indirect pages.
        eErrCode = truncate (
            page::SCOPE_TRIPLE, 0,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Truncate double indirect pages, downto 'aLink.m_nIndex2'.
        eErrCode = truncate (
            eScope, aLink.m_nIndex2 + 1,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Obtain last double indirect page location.
        sal_uInt32 nAddr = doubleLink (aLink.m_nIndex2);
        if (nAddr != STORE_PAGE_NULL)
        {
            // Check double indirect page buffer.
            if (rpDouble == NULL)
            {
                sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
                rpDouble = new(nPageSize) indirect(nPageSize);
            }

            // Load last double indirect page.
            OStoreIndirectionPageObject aDouble (*rpDouble);
            aDouble.location (nAddr);

            eErrCode = rBIOS.load (aDouble);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            // Truncate to ... pages.
            eErrCode = aDouble.truncate (
                aLink.m_nIndex1,
                aLink.m_nIndex0,
                rpSingle,
                rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            // Check for complete truncation.
            if ((aLink.m_nIndex1 + aLink.m_nIndex0) == 0)
            {
                // Free last double indirect page.
                eErrCode = rBIOS.free (aDouble);
                if (eErrCode != store_E_None)
                    STORE_METHOD_LEAVE(pMutex, eErrCode);

                // Clear pointer to last double indirect page.
                doubleLink (aLink.m_nIndex2, STORE_PAGE_NULL);
            }
        }
    }
    else if (eScope == page::SCOPE_TRIPLE)
    {
        // Truncate triple indirect pages, downto 'aLink.m_nIndex3'.
        eErrCode = truncate (
            eScope, aLink.m_nIndex3 + 1,
            rpSingle, rpDouble, rpTriple,
            rData, rBIOS, NULL);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);

        // Obtain last triple indirect page location.
        sal_uInt32 nAddr = tripleLink (aLink.m_nIndex3);
        if (nAddr != STORE_PAGE_NULL)
        {
            // Check triple indirect page buffer.
            if (rpTriple == NULL)
            {
                sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
                rpTriple = new(nPageSize) indirect(nPageSize);
            }

            // Load last triple indirect page.
            OStoreIndirectionPageObject aTriple (*rpTriple);
            aTriple.location (nAddr);

            eErrCode = rBIOS.load (aTriple);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            // Truncate to ... pages.
            eErrCode = aTriple.truncate (
                aLink.m_nIndex2,
                aLink.m_nIndex1,
                aLink.m_nIndex0,
                rpDouble, rpSingle,
                rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
                STORE_METHOD_LEAVE(pMutex, eErrCode);

            // Check for complete truncation.
            if ((aLink.m_nIndex2 + aLink.m_nIndex1 + aLink.m_nIndex0) == 0)
            {
                // Free last triple indirect page.
                eErrCode = rBIOS.free (aTriple);
                if (eErrCode != store_E_None)
                    STORE_METHOD_LEAVE(pMutex, eErrCode);

                // Clear pointer to last triple indirect page.
                tripleLink (aLink.m_nIndex3, STORE_PAGE_NULL);
            }
        }
    }
    else if (eScope == page::SCOPE_UNREACHABLE)
    {
        // Out of scope.
        eErrCode = store_E_CantSeek;
    }
    else
    {
        // Unknown scope.
        OSL_TRACE("OStoreDirectoryPageObject::put(): scope failed");
        eErrCode = store_E_Unknown;
    }

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * truncate (external data page scope; private).
 */
storeError OStoreDirectoryPageObject::truncate (
    page::ChunkScope       eScope,
    sal_uInt16             nRemain,
    indirect             *&rpSingle,
    indirect             *&rpDouble,
    indirect             *&rpTriple,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS,
    osl::Mutex            *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        // Truncate direct data pages.
        sal_uInt16 i, n = m_rPage.m_aDataBlock.directCount();
        for (i = n; i > nRemain; i--)
        {
            // Obtain data page location.
            sal_uInt32 nAddr = directLink (i - 1);
            if (nAddr == STORE_PAGE_NULL) continue;

            // Free data page.
            rData.location (nAddr);
            eErrCode = rBIOS.free (rData);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to data page.
            directLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    if (eScope == page::SCOPE_SINGLE)
    {
        // Truncate single indirect pages.
        sal_uInt16 i, n = m_rPage.m_aDataBlock.singleCount();
        for (i = n; i > nRemain; i--)
        {
            // Obtain single indirect page location.
            sal_uInt32 nAddr = singleLink (i - 1);
            if (nAddr == STORE_PAGE_NULL) continue;

            // Check single indirect page buffer.
            if (rpSingle == NULL)
            {
                sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
                rpSingle = new(nPageSize) indirect(nPageSize);
            }

            OStoreIndirectionPageObject aSingle (*rpSingle);
            aSingle.location (nAddr);

            // Load single indirect page.
            eErrCode = rBIOS.load (aSingle);
            if (eErrCode != store_E_None)
                break;

            // Truncate to zero data pages.
            eErrCode = aSingle.truncate (
                0, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
                break;

            // Free single indirect page.
            eErrCode = rBIOS.free (aSingle);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to single indirect page.
            singleLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    if (eScope == page::SCOPE_DOUBLE)
    {
        // Truncate double indirect pages.
        sal_uInt16 i, n = m_rPage.m_aDataBlock.doubleCount();
        for (i = n; i > nRemain; i--)
        {
            // Obtain double indirect page location.
            sal_uInt32 nAddr = doubleLink (i - 1);
            if (nAddr == STORE_PAGE_NULL) continue;

            // Check double indirect page buffer.
            if (rpDouble == NULL)
            {
                sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
                rpDouble = new(nPageSize) indirect(nPageSize);
            }

            OStoreIndirectionPageObject aDouble (*rpDouble);
            aDouble.location (nAddr);

            // Load double indirect page.
            eErrCode = rBIOS.load (aDouble);
            if (eErrCode != store_E_None)
                break;

            // Truncate to zero single indirect pages.
            eErrCode = aDouble.truncate (
                0, 0, rpSingle, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
                break;

            // Free double indirect page.
            eErrCode = rBIOS.free (aDouble);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to double indirect page.
            doubleLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    if (eScope == page::SCOPE_TRIPLE)
    {
        // Truncate triple indirect pages.
        sal_uInt16 i, n = m_rPage.m_aDataBlock.tripleCount();
        for (i = n; i > nRemain; i--)
        {
            // Obtain triple indirect page location.
            sal_uInt32 nAddr = tripleLink (i - 1);
            if (nAddr == STORE_PAGE_NULL) continue;

            // Check triple indirect page buffer.
            if (rpTriple == NULL)
            {
                sal_uInt16 nPageSize = m_rPage.m_aDescr.m_nSize;
                rpTriple = new(nPageSize) indirect(nPageSize);
            }

            OStoreIndirectionPageObject aTriple (*rpTriple);
            aTriple.location (nAddr);

            // Load triple indirect page.
            eErrCode = rBIOS.load (aTriple);
            if (eErrCode != store_E_None)
                break;

            // Truncate to zero double indirect pages.
            eErrCode = aTriple.truncate (
                0, 0, 0, rpDouble, rpSingle, rData, rBIOS, NULL);
            if (eErrCode != store_E_None)
                break;

            // Free triple indirect page.
            eErrCode = rBIOS.free (aTriple);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to triple indirect page.
            tripleLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    // Invalid scope.
    STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);
}

