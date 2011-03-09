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
#include "precompiled_tools.hxx"

#include "tools/mempool.hxx"
#include "osl/diagnose.h"
#include "rtl/alloc.h"

#ifndef INCLUDED_STDIO_H
#include <stdio.h>
#endif

/*************************************************************************
|*
|*    FixedMemPool::FixedMemPool()
|*
*************************************************************************/

FixedMemPool::FixedMemPool (
    char const * pTypeName, sal_uInt16 nTypeSize, sal_uInt16, sal_uInt16)
  : m_pTypeName (pTypeName)
{
    char name[RTL_CACHE_NAME_LENGTH + 1];
    snprintf (name, sizeof(name), "FixedMemPool_%d", (int)nTypeSize);
    m_pImpl = (FixedMemPool_Impl*)rtl_cache_create (name, nTypeSize, 0, NULL, NULL, NULL, 0, NULL, 0);
    OSL_TRACE("FixedMemPool::ctor(\"%s\"): %p", m_pTypeName, m_pImpl);
}

/*************************************************************************
|*
|*    FixedMemPool::~FixedMemPool()
|*
*************************************************************************/

FixedMemPool::~FixedMemPool()
{
    OSL_TRACE("FixedMemPool::dtor(\"%s\"): %p", m_pTypeName, m_pImpl);
    rtl_cache_destroy ((rtl_cache_type*)(m_pImpl)), m_pImpl = 0;
}

/*************************************************************************
|*
|*    FixedMemPool::Alloc()
|*
*************************************************************************/

void* FixedMemPool::Alloc()
{
    return rtl_cache_alloc ((rtl_cache_type*)(m_pImpl));
}

/*************************************************************************
|*
|*    FixedMemPool::Free()
|*
*************************************************************************/

void FixedMemPool::Free( void* pFree )
{
    rtl_cache_free ((rtl_cache_type*)(m_pImpl), pFree);
}
