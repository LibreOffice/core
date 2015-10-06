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

#include "sal/types.h"
#include "rtl/ref.hxx"

#include "store/types.h"
#include "storbase.hxx"
#include "object.hxx"
#include "boost/noncopyable.hpp"

namespace store
{

struct Entry;

/*========================================================================
 *
 * PageCache interface.
 *
 *======================================================================*/

class PageCache :
    public store::OStoreObject,
    private boost::noncopyable
{
    // Representation
    static size_t const theTableSize = 32;
    static_assert(STORE_IMPL_ISP2(theTableSize), "must be the case");

    Entry **     m_hash_table;
    Entry *      m_hash_table_0[theTableSize];
    size_t       m_hash_size;
    size_t       m_hash_shift;
    size_t const m_page_shift;

    size_t       m_hash_entries; // total number of entries in table.
    size_t       m_nHit;
    size_t       m_nMissed;

    static inline int hash_Impl(sal_uInt32 a, size_t s, size_t q, size_t m)
    {
        return static_cast<int>((((a) + ((a) >> (s)) + ((a) >> ((s) << 1))) >> (q)) & (m));
    }
    inline int hash_index_Impl (sal_uInt32 nOffset)
    {
        return hash_Impl(nOffset, m_hash_shift, m_page_shift, m_hash_size - 1);
    }

    Entry * lookup_Impl (Entry * entry, sal_uInt32 nOffset);
    void rescale_Impl (sal_Size new_size);

public:
    // Construction
    explicit PageCache (sal_uInt16 nPageSize);

    /** load.
     */
    storeError lookupPageAt (
        PageHolder & rxPage,
        sal_uInt32   nOffset);

    /** insert.
     */
    storeError insertPageAt (
        PageHolder const & rxPage,
        sal_uInt32         nOffset);

    /** update, or insert.
     */
    storeError updatePageAt (
        PageHolder const & rxPage,
        sal_uInt32         nOffset);

    /** remove (invalidate).
     */
    storeError removePageAt (
        sal_uInt32 nOffset);

protected:
    // Destruction
    virtual ~PageCache();
};

/*========================================================================
 *
 * PageCache factory.
 *
 *======================================================================*/

storeError
PageCache_createInstance (
    rtl::Reference< store::PageCache > & rxCache,
    sal_uInt16                           nPageSize
);

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif // INCLUDED_STORE_SOURCE_STORCACH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
