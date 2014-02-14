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

#ifndef _STORE_STORBIOS_HXX_
#define _STORE_STORBIOS_HXX_

#include "sal/types.h"
#include "rtl/ref.hxx"
#include "osl/mutex.hxx"

#include "store/types.h"
#include "object.hxx"
#include "lockbyte.hxx"
#include "storbase.hxx"
#include "storcach.hxx"

/*========================================================================
 *
 * OStorePageBIOS.
 *
 *======================================================================*/
namespace store
{

struct SuperBlockPage;

class OStorePageBIOS : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStorePageBIOS (void);

    /** Conversion into Mutex&
     */
    inline operator osl::Mutex& (void) const;

    /** Initialization.
     *  @param  pLockBytes [in]
     *  @param  eAccessMode [in]
     *  @param  rnPageSize [inout]
     *  @return store_E_None upon success
     */
    virtual storeError initialize (
        ILockBytes *    pLockBytes,
        storeAccessMode eAccessMode,
        sal_uInt16 &    rnPageSize);

    rtl::Reference< PageData::Allocator > & allocator()
    {
        return m_xAllocator;
    }

    /** read.
     */
    storeError read (
        sal_uInt32 nAddr, void *pData, sal_uInt32 nSize);

    /** write.
     */
    storeError write (
        sal_uInt32 nAddr, const void *pData, sal_uInt32 nSize);

    /** isWriteable.
     */
    inline bool isWriteable (void) const;

    /** isValid.
     */
    inline bool isValid (void) const;

    /** Page Access.
     */
    storeError acquirePage (
        const OStorePageDescriptor& rDescr, storeAccessMode eMode);

    storeError releasePage (const OStorePageDescriptor& rDescr);

    sal_uInt32 getRefererCount (void);

    /** Page Allocation.
     */
    enum Allocation
    {
        ALLOCATE_FIRST = 0,
        ALLOCATE_BEST  = 1,
        ALLOCATE_EOF   = 2
    };

    storeError allocate (
        OStorePageObject& rPage, Allocation eAllocation = ALLOCATE_FIRST);

    storeError free (sal_uInt32 nAddr);

    /** Page I/O.
     */
    storeError loadObjectAt (
        OStorePageObject& rPage, sal_uInt32 nAddr);

    storeError saveObjectAt (
        OStorePageObject& rPage, sal_uInt32 nAddr);

    /** close.
     *  @return store_E_None upon success.
     */
    storeError close (void);

    /** flush.
     *  @return store_E_None upon success.
     */
    storeError flush (void);

    /** size.
     */
    storeError size (sal_uInt32 &rnSize);

    /** ScanContext.
     */
    struct ScanContext
    {
        /** Representation.
         */
        OStorePageDescriptor m_aDescr;
        sal_uInt32           m_nSize;
        sal_uInt32           m_nMagic;

        /** Construction.
         */
        inline ScanContext (void);

        /** isValid.
         */
        inline bool isValid (void) const;
    };

    /** scanBegin.
     */
    storeError scanBegin (
        ScanContext &rCtx,
        sal_uInt32   nMagic = 0);

    /** scanNext.
     */
    storeError scanNext (
        ScanContext      &rCtx,
        OStorePageObject &rPage);

protected:
    /** Destruction (OReference).
     */
    virtual ~OStorePageBIOS (void);

private:
    /** Representation.
     */
    rtl::Reference<ILockBytes>    m_xLockBytes;
    osl::Mutex                    m_aMutex;

    SuperBlockPage *              m_pSuper;

    bool                          m_bWriteable;

    rtl::Reference< PageData::Allocator > m_xAllocator;
    rtl::Reference< PageCache >   m_xCache;

    /** Page Access (control).
     */
public:
    struct Ace
    {
        Ace *      m_next;
        Ace *      m_prev;

        sal_uInt32 m_addr;
        sal_uInt32 m_used;

        Ace();
        ~Ace();

        static int SAL_CALL constructor (void * obj, void * arg);

        static Ace * find   (Ace * head, sal_uInt32 addr);
        static void  insert (Ace * head, Ace * entry);
    };

private:
    Ace m_ace_head;

    class AceCache;

    /** Initialization.
     */
    storeError initialize_Impl (
        ILockBytes *    pLockBytes,
        storeAccessMode eAccessMode,
        sal_uInt16 &    rnPageSize);
    void cleanup_Impl();

    /** Page Maintenance.
     */
    storeError loadObjectAt_Impl (
        OStorePageObject & rPage, sal_uInt32 nAddr);
    storeError saveObjectAt_Impl (
        OStorePageObject & rPage, sal_uInt32 nAddr);

    /** Not implemented.
     */
    OStorePageBIOS (const OStorePageBIOS&);
    OStorePageBIOS& operator= (const OStorePageBIOS&);
};

inline OStorePageBIOS::operator osl::Mutex& (void) const
{
    return (osl::Mutex&)m_aMutex;
}
inline bool OStorePageBIOS::isWriteable (void) const
{
    return m_bWriteable;
}
inline bool OStorePageBIOS::isValid (void) const
{
    return m_xLockBytes.is();
}

inline OStorePageBIOS::ScanContext::ScanContext (void)
    : m_aDescr (0, 0, 0), m_nSize (0), m_nMagic (0)
{
}
inline bool OStorePageBIOS::ScanContext::isValid (void) const
{
    return (m_aDescr.m_nAddr < m_nSize);
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_STORBIOS_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
