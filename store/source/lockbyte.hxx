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

#ifndef INCLUDED_STORE_SOURCE_LOCKBYTE_HXX
#define INCLUDED_STORE_SOURCE_LOCKBYTE_HXX

#include <sal/config.h>

#include <memory>

#include <sal/types.h>

#include <rtl/ustring.h>
#include <salhelper/simplereferenceobject.hxx>

#include <store/types.h>
#include "storbase.hxx"

namespace rtl { template <class reference_type> class Reference; }

namespace store
{

class ILockBytes : public virtual salhelper::SimpleReferenceObject
{
public:
    /**
        @param  rxAllocator [out]
        @param  nPageSize [in]
    */
    storeError initialize (
        rtl::Reference< PageData::Allocator > & rxAllocator,
        sal_uInt16                              nPageSize);

    /**
        @param  rPage [out]
        @param  nOffset [in]
     */
    storeError readPageAt (
        std::shared_ptr<PageData> & rPage,
        sal_uInt32   nOffset);

    /**
        @param  rPage [in]
        @param  nOffset [in]
     */
    storeError writePageAt (
        std::shared_ptr<PageData> const & rPage,
        sal_uInt32         nOffset);

    /**
        @param  nOffset [in]
        @param  pBuffer [out]
        @param  nBytes [in]
        @return store_E_None upon success
     */
    storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes);

    /**
        @param  nOffset [in]
        @param  pBuffer [in]
        @param  nBytes [in]
        @return store_E_None upon success
     */
    storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes);

    /**
        @param  rnSize [out]
        @return store_E_None upon success
     */
    storeError getSize (sal_uInt32 & rnSize);

    /**
        @param  nSize [in]
        @return store_E_None upon success
     */
    storeError setSize (sal_uInt32 nSize);

    /**
        @return store_E_None upon success
     */
    storeError flush();

protected:
    virtual ~ILockBytes() override {}

private:
    /** Implementation (abstract).
     */
    virtual storeError initialize_Impl (
        rtl::Reference< PageData::Allocator > & rxAllocator,
        sal_uInt16                              nPageSize) = 0;

    virtual storeError readPageAt_Impl (
        std::shared_ptr<PageData> & rPage,
        sal_uInt32   nOffset) = 0;

    virtual storeError writePageAt_Impl (
        std::shared_ptr<PageData> const & rPage,
        sal_uInt32         nOffset) = 0;

    virtual storeError readAt_Impl (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes) = 0;

    virtual storeError writeAt_Impl (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes) = 0;

    virtual storeError getSize_Impl (
        sal_uInt32 & rnSize) = 0;

    virtual storeError setSize_Impl (
        sal_uInt32 nSize) = 0;

    virtual storeError flush_Impl() = 0;
};

storeError FileLockBytes_createInstance (
  rtl::Reference< store::ILockBytes > & rxLockBytes,
  rtl_uString *   pFilename,
  storeAccessMode eAccessMode
);

storeError MemoryLockBytes_createInstance (
  rtl::Reference< store::ILockBytes > & rxLockBytes
);

} // namespace store

#endif // INCLUDED_STORE_SOURCE_LOCKBYTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
