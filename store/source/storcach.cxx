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

#include "sal/config.h"

#include "boost/static_assert.hpp"

#include "storcach.hxx"

#include "sal/types.h"
#include "sal/macros.h"
#include "rtl/alloc.h"
#include "osl/diagnose.h"

#include "store/types.h"
#include "object.hxx"
#include "storbase.hxx"

#include <stddef.h>

using namespace store;

// PageCache (non-virtual interface) implementation.
storeError PageCache::lookupPageAt (PageHolder & rxPage, sal_uInt32 nOffset)
{
    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::lookupPageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    return lookupPageAt_Impl (rxPage, nOffset);
}

storeError PageCache::insertPageAt (PageHolder const & rxPage, sal_uInt32 nOffset)
{
    // [SECURITY:ValInput]
    PageData const * pagedata = rxPage.get();
    OSL_PRECOND(!(pagedata == 0), "store::PageCache::insertPageAt(): invalid Page");
    if (pagedata == 0)
        return store_E_InvalidParameter;

    sal_uInt32 const offset = pagedata->location();
    OSL_PRECOND(!(nOffset != offset), "store::PageCache::insertPageAt(): inconsistent Offset");
    if (nOffset != offset)
        return store_E_InvalidParameter;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::insertPageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    return insertPageAt_Impl (rxPage, nOffset);
}

storeError PageCache::updatePageAt (PageHolder const & rxPage, sal_uInt32 nOffset)
{
    // [SECURITY:ValInput]
    PageData const * pagedata = rxPage.get();
    OSL_PRECOND(!(pagedata == 0), "store::PageCache::updatePageAt(): invalid Page");
    if (pagedata == 0)
        return store_E_InvalidParameter;

    sal_uInt32 const offset = pagedata->location();
    OSL_PRECOND(!(nOffset != offset), "store::PageCache::updatePageAt(): inconsistent Offset");
    if (nOffset != offset)
        return store_E_InvalidParameter;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::updatePageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    return updatePageAt_Impl (rxPage, nOffset);
}

storeError PageCache::removePageAt (sal_uInt32 nOffset)
{
    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::PageCache::removePageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    return removePageAt_Impl (nOffset);
}

// Entry
namespace
{

struct Entry
{
    // Representation
    PageHolder m_xPage;
    sal_uInt32 m_nOffset;
    Entry *    m_pNext;

    // Allocation
    static void * operator new (size_t, void * p) { return p; }
    static void   operator delete (void *, void *) {}

    // Construction
    explicit Entry (PageHolder const & rxPage = PageHolder(), sal_uInt32 nOffset = STORE_PAGE_NULL)
        : m_xPage(rxPage), m_nOffset(nOffset), m_pNext(0)
    {}

    // Destruction
    ~Entry() {}
};

} // namespace

// EntryCache interface
namespace
{

class EntryCache
{
    rtl_cache_type * m_entry_cache;

public:
    static EntryCache & get();

    Entry * create (PageHolder const & rxPage, sal_uInt32 nOffset);

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
        0, // constructor
        0, // destructor
        0, // reclaim
        0, // userarg
        0, // default source
        0  // flags
        );
}

EntryCache::~EntryCache()
{
    rtl_cache_destroy (m_entry_cache), m_entry_cache = 0;
}

Entry * EntryCache::create (PageHolder const & rxPage, sal_uInt32 nOffset)
{
    void * pAddr = rtl_cache_alloc (m_entry_cache);
    if (pAddr != 0)
    {
        // construct
        return new(pAddr) Entry (rxPage, nOffset);
    }
    return 0;
}

void EntryCache::destroy (Entry * entry)
{
    if (entry != 0)
    {
        // destruct
        entry->~Entry();

        // return to cache
        rtl_cache_free (m_entry_cache, entry);
    }
}

// highbit():= log2() + 1 (complexity O(1))
static int highbit(sal_Size n)
{
    int k = 1;

    if (n == 0)
        return (0);
#if SAL_TYPES_SIZEOFLONG == 8
    if (n & 0xffffffff00000000ul)
        k |= 32, n >>= 32;
#endif
    if (n & 0xffff0000)
        k |= 16, n >>= 16;
    if (n & 0xff00)
        k |= 8, n >>= 8;
    if (n & 0xf0)
        k |= 4, n >>= 4;
    if (n & 0x0c)
        k |= 2, n >>= 2;
    if (n & 0x02)
        k++;

    return (k);
}

//PageCache_Impl implementation
namespace store
{

class PageCache_Impl :
    public store::OStoreObject,
    public store::PageCache
{
    // Representation
    static size_t const theTableSize = 32;
    BOOST_STATIC_ASSERT(STORE_IMPL_ISP2(theTableSize));

    Entry **     m_hash_table;
    Entry *      m_hash_table_0[theTableSize];
    size_t       m_hash_size;
    size_t       m_hash_shift;
    size_t const m_page_shift;

    size_t       m_hash_entries; // total number of entries in table.
    size_t       m_nHit;
    size_t       m_nMissed;

    inline int hash_Impl(sal_uInt32 a, size_t s, size_t q, size_t m)
    {
        return ((((a) + ((a) >> (s)) + ((a) >> ((s) << 1))) >> (q)) & (m));
    }
    inline int hash_index_Impl (sal_uInt32 nOffset)
    {
        return hash_Impl(nOffset, m_hash_shift, m_page_shift, m_hash_size - 1);
    }

    Entry * lookup_Impl (Entry * entry, sal_uInt32 nOffset);
    void rescale_Impl (sal_Size new_size);

    // PageCache Implementation
    virtual storeError lookupPageAt_Impl (
        PageHolder & rxPage,
        sal_uInt32   nOffset);

    virtual storeError insertPageAt_Impl (
        PageHolder const & rxPage,
        sal_uInt32         nOffset);

    virtual storeError updatePageAt_Impl (
        PageHolder const & rxPage,
        sal_uInt32         nOffset);

    virtual storeError removePageAt_Impl (
        sal_uInt32 nOffset);

    // Not implemented
    PageCache_Impl (PageCache_Impl const &);
    PageCache_Impl & operator= (PageCache_Impl const &);

public:
    // Construction
    explicit PageCache_Impl (sal_uInt16 nPageSize);

    // Delegate multiple inherited IReference
    virtual oslInterlockedCount SAL_CALL acquire();
    virtual oslInterlockedCount SAL_CALL release();

protected:
    // Destruction
    virtual ~PageCache_Impl (void);
};

} // namespace store

PageCache_Impl::PageCache_Impl (sal_uInt16 nPageSize)
    : m_hash_table   (m_hash_table_0),
      m_hash_size    (theTableSize),
      m_hash_shift   (highbit(m_hash_size) - 1),
      m_page_shift   (highbit(nPageSize) - 1),
      m_hash_entries (0),
      m_nHit         (0),
      m_nMissed      (0)
{
    static size_t const theSize = SAL_N_ELEMENTS(m_hash_table_0);
    BOOST_STATIC_ASSERT(theSize == theTableSize);
    memset(m_hash_table_0, 0, sizeof(m_hash_table_0));
}

PageCache_Impl::~PageCache_Impl()
{
    double s_x = 0.0;
    sal_Size i, n = m_hash_size;
    for (i = 0; i < n; i++)
    {
        int x = 0;
        Entry * entry = m_hash_table[i];
        while (entry != 0)
        {
            m_hash_table[i] = entry->m_pNext, entry->m_pNext = 0;
            EntryCache::get().destroy (entry);
            entry = m_hash_table[i];
            x += 1;
        }
        s_x  += double(x);
    }
    double ave = s_x / double(n);
    OSL_TRACE("ave hash chain length: %g", ave);
    (void) ave;

    if (m_hash_table != m_hash_table_0)
    {
        rtl_freeMemory (m_hash_table);
        m_hash_table = m_hash_table_0;
        m_hash_size  = theTableSize;
        m_hash_shift = highbit(m_hash_size) - 1;
    }
    OSL_TRACE("Hits: %zu, Misses: %zu", m_nHit, m_nMissed);
}

oslInterlockedCount PageCache_Impl::acquire()
{
    return OStoreObject::acquire();
}

oslInterlockedCount PageCache_Impl::release()
{
    return OStoreObject::release();
}

void PageCache_Impl::rescale_Impl (sal_Size new_size)
{
    sal_Size new_bytes = new_size * sizeof(Entry*);
    Entry ** new_table = (Entry**)(rtl_allocateMemory(new_bytes));

    if (new_table != 0)
    {
        Entry ** old_table = m_hash_table;
        sal_Size old_size  = m_hash_size;

        OSL_TRACE("ave chain length: %zu, total entries: %zu [old_size: %zu new_size: %zu]",
                  m_hash_entries >> m_hash_shift, m_hash_entries, old_size, new_size);

        memset (new_table, 0, new_bytes);

        m_hash_table = new_table;
        m_hash_size  = new_size;
        m_hash_shift = highbit(m_hash_size) - 1;

        sal_Size i;
        for (i = 0; i < old_size; i++)
        {
            Entry * curr = old_table[i];
            while (curr != 0)
            {
                Entry * next = curr->m_pNext;
                int index = hash_index_Impl(curr->m_nOffset);
                curr->m_pNext = m_hash_table[index], m_hash_table[index] = curr;
                curr = next;
            }
            old_table[i] = 0;
        }
        if (old_table != m_hash_table_0)
        {

            rtl_freeMemory (old_table);
        }
    }
}

Entry * PageCache_Impl::lookup_Impl (Entry * entry, sal_uInt32 nOffset)
{
    int lookups = 0;
    while (entry != 0)
    {
        if (entry->m_nOffset == nOffset)
            break;

        lookups += 1;
        entry = entry->m_pNext;
    }
    if (lookups > 2)
    {
        sal_Size new_size = m_hash_size, ave = m_hash_entries >> m_hash_shift;
        for (; ave > 4; new_size *= 2, ave /= 2)
            continue;
        if (new_size != m_hash_size)
            rescale_Impl (new_size);
    }
    return entry;
}

storeError PageCache_Impl::lookupPageAt_Impl (
    PageHolder & rxPage,
    sal_uInt32   nOffset)
{
    int index = hash_index_Impl(nOffset);
    Entry const * entry = lookup_Impl (m_hash_table[index], nOffset);
    if (entry != 0)
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

storeError PageCache_Impl::insertPageAt_Impl (
    PageHolder const & rxPage,
    sal_uInt32         nOffset)
{
    Entry * entry = EntryCache::get().create (rxPage, nOffset);
    if (entry != 0)
    {
        // Insert new entry.
        int index = hash_index_Impl(nOffset);
        entry->m_pNext = m_hash_table[index], m_hash_table[index] = entry;

        // Update stats and leave.
        m_hash_entries += 1;
        return store_E_None;
    }
    return store_E_OutOfMemory;
}

storeError PageCache_Impl::updatePageAt_Impl (
    PageHolder const & rxPage,
    sal_uInt32         nOffset)
{
    int index = hash_index_Impl(nOffset);
    Entry *  entry  = lookup_Impl (m_hash_table[index], nOffset);
    if (entry != 0)
    {
        // Update existing entry.
        entry->m_xPage = rxPage;

        // Update stats and leave. // m_nUpdHit += 1;
        return store_E_None;
    }
    return insertPageAt_Impl (rxPage, nOffset);
}

storeError PageCache_Impl::removePageAt_Impl (
    sal_uInt32 nOffset)
{
    Entry ** ppEntry = &(m_hash_table[hash_index_Impl(nOffset)]);
    while (*ppEntry != 0)
    {
        if ((*ppEntry)->m_nOffset == nOffset)
        {
            // Existing entry.
            Entry * entry = (*ppEntry);

            // Dequeue and destroy entry.
            (*ppEntry) = entry->m_pNext, entry->m_pNext = 0;
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
 * (external OStorePageData representation).
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
    rxCache = new PageCache_Impl (nPageSize);
    if (!rxCache.is())
        return store_E_OutOfMemory;

    return store_E_None;
}

} // namespace store

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
