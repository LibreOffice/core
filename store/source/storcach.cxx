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

#include <sal/config.h>

#include "storcach.hxx"

#include <sal/log.hxx>
#include <sal/types.h>
#include <sal/macros.h>
#include <rtl/alloc.h>
#include <osl/diagnose.h>

#include <store/types.h>
#include "storbase.hxx"

#include <memory>
#include <string.h>

using namespace store;

// Entry

namespace store {
struct Entry
{
    // Representation
    std::shared_ptr<PageData> m_xPage;
    sal_uInt32 const m_nOffset;
    Entry *    m_pNext;

    // Allocation
    static void * operator new (size_t, void * p) { return p; }
    static void   operator delete (void *, void *) {}

    // Construction
    explicit Entry (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nOffset)
        : m_xPage(rxPage), m_nOffset(nOffset), m_pNext(nullptr)
    {}
};
};

// EntryCache interface
namespace
{

class EntryCache
{
    rtl_cache_type * m_entry_cache;

public:
    static EntryCache & get();

    Entry * create (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nOffset);

    void destroy (Entry * entry);

protected:
    EntryCache();
    ~EntryCache();
};

} // namespace

// EntryCache implementation
EntryCache & EntryCache::get()
{
    static EntryCache g_entry_cache;
    return g_entry_cache;
}

EntryCache::EntryCache()
{
    m_entry_cache = rtl_cache_create (
        "store_cache_entry_cache",
        sizeof(Entry),
        0, // objalign
        nullptr, // constructor
        nullptr, // destructor
        nullptr, // reclaim
        nullptr, // userarg
        nullptr, // default source
        0  // flags
        );
}

EntryCache::~EntryCache()
{
    rtl_cache_destroy (m_entry_cache);
    m_entry_cache = nullptr;
}

Entry * EntryCache::create (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nOffset)
{
    void * pAddr = rtl_cache_alloc (m_entry_cache);
    if (pAddr != nullptr)
    {
        // construct
        return new(pAddr) Entry (rxPage, nOffset);
    }
    return nullptr;
}

void EntryCache::destroy (Entry * entry)
{
    if (entry != nullptr)
    {
        // destruct
        entry->~Entry();

        // return to cache
        rtl_cache_free (m_entry_cache, entry);
    }
}

// highbit():= log2() + 1 (complexity O(1))
static int highbit(std::size_t n)
{
    int k = 1;

    if (n == 0)
        return 0;
#if SAL_TYPES_SIZEOFLONG == 8
    if (n & 0xffffffff00000000ul)
    {
        k |= 32;
        n >>= 32;
    }
#endif
    if (n & 0xffff0000)
    {
        k |= 16;
        n >>= 16;
    }
    if (n & 0xff00)
    {
        k |= 8;
        n >>= 8;
    }
    if (n & 0xf0)
    {
        k |= 4;
        n >>= 4;
    }
    if (n & 0x0c)
    {
        k |= 2;
        n >>= 2;
    }
    if (n & 0x02)
        k++;

    return k;
}


PageCache::PageCache (sal_uInt16 nPageSize)
    : m_hash_table   (m_hash_table_0),
      m_hash_size    (theTableSize),
      m_hash_shift   (highbit(m_hash_size) - 1),
      m_page_shift   (highbit(nPageSize) - 1),
      m_hash_entries (0),
      m_nHit         (0),
      m_nMissed      (0)
{
    static size_t const theSize = SAL_N_ELEMENTS(m_hash_table_0);
    static_assert(theSize == theTableSize, "must be equal");
    memset(m_hash_table_0, 0, sizeof(m_hash_table_0));
}

PageCache::~PageCache()
{
    double s_x = 0.0;
    std::size_t i, n = m_hash_size;
    for (i = 0; i < n; i++)
    {
        int x = 0;
        Entry * entry = m_hash_table[i];
        while (entry != nullptr)
        {
            m_hash_table[i] = entry->m_pNext;
            entry->m_pNext = nullptr;
            EntryCache::get().destroy (entry);
            entry = m_hash_table[i];
            x += 1;
        }
        s_x  += double(x);
    }
    double ave = s_x / double(n);
    SAL_INFO("store", "avg hash chain length: " << ave);

    if (m_hash_table != m_hash_table_0)
    {
        std::free (m_hash_table);
        m_hash_table = m_hash_table_0;
        m_hash_size  = theTableSize;
        m_hash_shift = highbit(m_hash_size) - 1;
    }
    SAL_INFO("store", "Hits: " << m_nHit << ", Misses: " <<  m_nMissed);
}

void PageCache::rescale_Impl (std::size_t new_size)
{
    std::size_t new_bytes = new_size * sizeof(Entry*);
    Entry ** new_table = static_cast<Entry**>(std::malloc(new_bytes));

    if (new_table == nullptr)
        return;

    Entry ** old_table = m_hash_table;
    std::size_t old_size  = m_hash_size;

    SAL_INFO(
        "store",
        "ave chain length: " << (m_hash_entries >> m_hash_shift)
            << ", total entries: " << m_hash_entries << " [old_size: "
            << old_size << " new_size: " << new_size << "]");

    memset (new_table, 0, new_bytes);

    m_hash_table = new_table;
    m_hash_size  = new_size;
    m_hash_shift = highbit(m_hash_size) - 1;

    std::size_t i;
    for (i = 0; i < old_size; i++)
    {
        Entry * curr = old_table[i];
        while (curr != nullptr)
        {
            Entry * next = curr->m_pNext;
            int index = hash_index_Impl(curr->m_nOffset);
            curr->m_pNext = m_hash_table[index];
            m_hash_table[index] = curr;
            curr = next;
        }
        old_table[i] = nullptr;
    }
    if (old_table != m_hash_table_0)
    {

        std::free (old_table);
    }
}

Entry * PageCache::lookup_Impl (Entry * entry, sal_uInt32 nOffset)
{
    int lookups = 0;
    while (entry != nullptr)
    {
        if (entry->m_nOffset == nOffset)
            break;

        lookups += 1;
        entry = entry->m_pNext;
    }
    if (lookups > 2)
    {
        std::size_t new_size = m_hash_size, ave = m_hash_entries >> m_hash_shift;
        for (; ave > 4; new_size *= 2, ave /= 2)
            continue;
        if (new_size != m_hash_size)
            rescale_Impl (new_size);
    }
    return entry;
}

storeError PageCache::lookupPageAt (std::shared_ptr<PageData> & rxPage, sal_uInt32 nOffset)
{
    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::lookupPageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    int index = hash_index_Impl(nOffset);
    Entry const * entry = lookup_Impl (m_hash_table[index], nOffset);
    if (entry != nullptr)
    {
        // Existing entry.
        rxPage = entry->m_xPage;

        // Update stats and leave.
        m_nHit += 1;
        return store_E_None;
    }

    // Cache miss. Update stats and leave.
    m_nMissed += 1;
    return store_E_NotExists;
}

storeError PageCache::insertPageAt (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nOffset)
{
    // [SECURITY:ValInput]
    PageData const * pagedata = rxPage.get();
    OSL_PRECOND(!(pagedata == nullptr), "store::PageCache::insertPageAt(): invalid Page");
    if (pagedata == nullptr)
        return store_E_InvalidParameter;

    sal_uInt32 const offset = pagedata->location();
    OSL_PRECOND(!(nOffset != offset), "store::PageCache::insertPageAt(): inconsistent Offset");
    if (nOffset != offset)
        return store_E_InvalidParameter;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::insertPageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    Entry * entry = EntryCache::get().create (rxPage, nOffset);
    if (entry != nullptr)
    {
        // Insert new entry.
        int index = hash_index_Impl(nOffset);
        entry->m_pNext = m_hash_table[index];
        m_hash_table[index] = entry;

        // Update stats and leave.
        m_hash_entries += 1;
        return store_E_None;
    }
    return store_E_OutOfMemory;
}

storeError PageCache::updatePageAt (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nOffset)
{
    // [SECURITY:ValInput]
    PageData const * pagedata = rxPage.get();
    OSL_PRECOND(!(pagedata == nullptr), "store::PageCache::updatePageAt(): invalid Page");
    if (pagedata == nullptr)
        return store_E_InvalidParameter;

    sal_uInt32 const offset = pagedata->location();
    OSL_PRECOND(!(nOffset != offset), "store::PageCache::updatePageAt(): inconsistent Offset");
    if (nOffset != offset)
        return store_E_InvalidParameter;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::updatePageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    int index = hash_index_Impl(nOffset);
    Entry *  entry  = lookup_Impl (m_hash_table[index], nOffset);
    if (entry != nullptr)
    {
        // Update existing entry.
        entry->m_xPage = rxPage;

        // Update stats and leave. // m_nUpdHit += 1;
        return store_E_None;
    }
    return insertPageAt (rxPage, nOffset);
}

storeError PageCache::removePageAt (sal_uInt32 nOffset)
{
    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::removePageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    Entry ** ppEntry = &(m_hash_table[hash_index_Impl(nOffset)]);
    while (*ppEntry != nullptr)
    {
        if ((*ppEntry)->m_nOffset == nOffset)
        {
            // Existing entry.
            Entry * entry = *ppEntry;

            // Dequeue and destroy entry.
            (*ppEntry) = entry->m_pNext;
            entry->m_pNext = nullptr;
            EntryCache::get().destroy (entry);

            // Update stats and leave.
            m_hash_entries -= 1;
            return store_E_None;
        }
        ppEntry = &((*ppEntry)->m_pNext);
    }
    return store_E_NotExists;
}

/*
 *
 * Old OStorePageCache implementation.
 *
 * (two-way association (sorted address array, LRU chain)).
 * (external PageData representation).
 *
 */

/*
 *
 * PageCache factory implementation.
 *
 */
namespace store {

storeError
PageCache_createInstance (
    rtl::Reference< store::PageCache > & rxCache,
    sal_uInt16                           nPageSize)
{
    rxCache = new PageCache (nPageSize);
    if (!rxCache.is())
        return store_E_OutOfMemory;

    return store_E_None;
}

} // namespace store

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
