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

#ifndef INCLUDED_STORE_SOURCE_STORBIOS_HXX
#define INCLUDED_STORE_SOURCE_STORBIOS_HXX

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>

#include <store/types.h>
#include "object.hxx"
#include "storbase.hxx"

namespace store
{

class ILockBytes;
class PageCache;
struct SuperBlockPage;

class OStorePageBIOS : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStorePageBIOS();

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
        sal_uInt32 nAddr, void *pData, sal_uInt32 nSize) const;

    /** write.
     */
    storeError write (
        sal_uInt32 nAddr, const void *pData, sal_uInt32 nSize) const;

    /** isWriteable.
     */
    inline bool isWriteable() const;

    /** isValid.
     */
    inline bool isValid() const;

    /** Page Access.
     */
    storeError acquirePage (
        const OStorePageDescriptor& rDescr, storeAccessMode eMode);

    storeError releasePage (const OStorePageDescriptor& rDescr);

    storeError allocate (OStorePageObject& rPage);

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
    storeError close();

    /** flush.
     *  @return store_E_None upon success.
     */
    storeError flush();

protected:
    /** Destruction (OReference).
     */
    virtual ~OStorePageBIOS() override;

private:
    /** Representation.
     */
    rtl::Reference<ILockBytes>    m_xLockBytes;
    osl::Mutex                    m_aMutex;

    std::unique_ptr<SuperBlockPage> m_pSuper;

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
        OStorePageObject & rPage, sal_uInt32 nAddr) const;
    storeError saveObjectAt_Impl (
        OStorePageObject & rPage, sal_uInt32 nAddr) const;

    OStorePageBIOS (const OStorePageBIOS&) = delete;
    OStorePageBIOS& operator= (const OStorePageBIOS&) = delete;
};

inline OStorePageBIOS::operator osl::Mutex& (void) const
{
    return const_cast<osl::Mutex&>(m_aMutex);
}
inline bool OStorePageBIOS::isWriteable() const
{
    return m_bWriteable;
}
inline bool OStorePageBIOS::isValid() const
{
    return m_xLockBytes.is();
}

} // namespace store

#endif // INCLUDED_STORE_SOURCE_STORBIOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
