/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_store.hxx"

#include "storbase.hxx"

#include "sal/types.h"
#include "rtl/alloc.h"
#include "rtl/ref.hxx"
#include "osl/diagnose.h"

#include "store/types.h"
#include "object.hxx"

#ifndef INCLUDED_STDIO_H
#include <stdio.h>
#define INCLUDED_STDIO_H
#endif

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
    public store::PageData::Allocator
{
public:
    /** Construction (two phase).
     */
    Allocator_Impl();

    storeError initialize (sal_uInt16 nPageSize);

    /** Delegate multiple inherited rtl::IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire()
    {
        return OStoreObject::acquire();
    }
    virtual oslInterlockedCount SAL_CALL release()
    {
        return OStoreObject::release();
    }

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
    virtual void allocate_Impl (void ** ppPage, sal_uInt16 * pnSize);
    virtual void deallocate_Impl (void * pPage);

    /** Not implemented.
     */
    Allocator_Impl (Allocator_Impl const &);
    Allocator_Impl & operator= (Allocator_Impl const &);
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
    (void) snprintf (name, sizeof(name), "store_page_alloc_%lu", size);

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
OStorePageObject::~OStorePageObject (void)
{
}
