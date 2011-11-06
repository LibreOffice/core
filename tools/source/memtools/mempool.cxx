/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
