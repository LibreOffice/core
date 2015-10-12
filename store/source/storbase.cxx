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

#include "storbase.hxx"

#include "boost/noncopyable.hpp"
#include "sal/types.h"
#include "rtl/alloc.h"
#include "rtl/ref.hxx"
#include "osl/diagnose.h"

#include "store/types.h"
#include "object.hxx"

#include <stdio.h>

using namespace store;

/*========================================================================
 *
 * SharedCount::Allocator.
 *
 *======================================================================*/
SharedCount::Allocator &
SharedCount::Allocator::get()
{
    static Allocator g_aSharedCountAllocator;
    return g_aSharedCountAllocator;
}

SharedCount::Allocator::Allocator()
{
    m_cache = rtl_cache_create (
        "store_shared_count_cache",
        sizeof(long),
        0, // objalign
        0, // constructor
        0, // destructor
        0, // reclaim
        0, // userarg
        0, // default source
        0  // flags
        );
}

SharedCount::Allocator::~Allocator()
{
    rtl_cache_destroy (m_cache), m_cache = 0;
}

/*========================================================================
 *
 * PageData::Allocator_Impl (default allocator).
 *
 *======================================================================*/
namespace store
{

class PageData::Allocator_Impl :
    public store::OStoreObject,
    public store::PageData::Allocator,
    private boost::noncopyable
{
public:
    /** Construction (two phase).
     */
    Allocator_Impl();

    storeError initialize (sal_uInt16 nPageSize);

protected:
    /** Destruction.
     */
    virtual ~Allocator_Impl();

private:
    /** Representation.
     */
    rtl_cache_type * m_page_cache;
    sal_uInt16       m_page_size;

    /** PageData::Allocator implementation.
     */
    virtual void allocate_Impl (void ** ppPage, sal_uInt16 * pnSize) override;
    virtual void deallocate_Impl (void * pPage) override;
};

} // namespace store

PageData::Allocator_Impl::Allocator_Impl()
    : m_page_cache(0), m_page_size(0)
{}

storeError
PageData::Allocator_Impl::initialize (sal_uInt16 nPageSize)
{
    char name[RTL_CACHE_NAME_LENGTH + 1];
    sal_Size size = sal::static_int_cast< sal_Size >(nPageSize);
    (void) snprintf (name, sizeof(name), "store_page_alloc_%" SAL_PRIuUINTPTR, size);

    m_page_cache = rtl_cache_create (name, size, 0, 0, 0, 0, 0, 0, 0);
    if (!m_page_cache)
        return store_E_OutOfMemory;

    m_page_size = nPageSize;
    return store_E_None;
}

PageData::Allocator_Impl::~Allocator_Impl()
{
    rtl_cache_destroy(m_page_cache), m_page_cache = 0;
}

void PageData::Allocator_Impl::allocate_Impl (void ** ppPage, sal_uInt16 * pnSize)
{
    OSL_PRECOND((ppPage != 0) && (pnSize != 0), "contract violation");
    if ((ppPage != 0) && (pnSize != 0))
        *ppPage = rtl_cache_alloc(m_page_cache), *pnSize = m_page_size;
}

void PageData::Allocator_Impl::deallocate_Impl (void * pPage)
{
    OSL_PRECOND(pPage != 0, "contract violation");
    rtl_cache_free(m_page_cache, pPage);
}

/*========================================================================
 *
 * PageData::Allocator factory.
 *
 *======================================================================*/

storeError
PageData::Allocator::createInstance (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize)
{
    rtl::Reference< PageData::Allocator_Impl > xAllocator (new PageData::Allocator_Impl());
    if (!xAllocator.is())
        return store_E_OutOfMemory;

    rxAllocator = &*xAllocator;
    return xAllocator->initialize (nPageSize);
}

/*========================================================================
 *
 * OStorePageObject.
 *
 *======================================================================*/
/*
 * ~OStorePageObject.
 */
OStorePageObject::~OStorePageObject()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
