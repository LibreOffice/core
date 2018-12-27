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

#ifndef INCLUDED_STORE_SOURCE_STORCACH_HXX
#define INCLUDED_STORE_SOURCE_STORCACH_HXX

#include <sal/config.h>

#include <memory>

#include <sal/types.h>

#include <store/types.h>
#include "object.hxx"

namespace rtl { template <class reference_type> class Reference; }
namespace store { struct PageData; }

namespace store
{

struct Entry;

class PageCache :
    public store::OStoreObject
{
    // Representation
    static size_t const theTableSize = 32;
    static_assert((theTableSize & (theTableSize-1)) == 0, "table size should be a power of 2");

    Entry **     m_hash_table;
    Entry *      m_hash_table_0[theTableSize];
    size_t       m_hash_size;
    size_t       m_hash_shift;
    size_t const m_page_shift;

    size_t       m_hash_entries; // total number of entries in table.
    size_t       m_nHit;
    size_t       m_nMissed;

    static int hash_Impl(sal_uInt32 a, size_t s, size_t q, size_t m)
    {
        return static_cast<int>(((a + (a >> s) + (a >> (s << 1))) >> q) & m);
    }
    int hash_index_Impl (sal_uInt32 nOffset)
    {
        return hash_Impl(nOffset, m_hash_shift, m_page_shift, m_hash_size - 1);
    }

    Entry * lookup_Impl (Entry * entry, sal_uInt32 nOffset);
    void rescale_Impl (std::size_t new_size);

public:
    explicit PageCache (sal_uInt16 nPageSize);

    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;

    /** load.
     */
    storeError lookupPageAt (
        std::shared_ptr<PageData> & rxPage,
        sal_uInt32   nOffset);

    /** insert.
     */
    storeError insertPageAt (
        std::shared_ptr<PageData> const & rxPage,
        sal_uInt32         nOffset);

    /** update, or insert.
     */
    storeError updatePageAt (
        std::shared_ptr<PageData> const & rxPage,
        sal_uInt32         nOffset);

    /** remove (invalidate).
     */
    storeError removePageAt (
        sal_uInt32 nOffset);

protected:
    virtual ~PageCache() override;
};

storeError PageCache_createInstance (
    rtl::Reference< store::PageCache > & rxCache,
    sal_uInt16                           nPageSize
);

} // namespace store

#endif // INCLUDED_STORE_SOURCE_STORCACH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
