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

#include <tools/mempool.hxx>
#include <sal/log.hxx>
#include <rtl/alloc.h>

#include <stdio.h>

FixedMemPool::FixedMemPool(char const * pTypeName, sal_uInt16 nTypeSize)
  : m_pTypeName (pTypeName)
{
    char name[RTL_CACHE_NAME_LENGTH + 1];
    snprintf (name, sizeof(name), "FixedMemPool_%d", (int)nTypeSize);
    m_pImpl = reinterpret_cast<FixedMemPool_Impl*>(rtl_cache_create (name, nTypeSize, 0, nullptr, nullptr, nullptr, nullptr, nullptr, 0));
    SAL_INFO("tools.memtools","FixedMemPool::ctor(\"" << m_pTypeName << "\"): " << m_pImpl);
}

FixedMemPool::~FixedMemPool()
{
    SAL_INFO("tools.memtools","FixedMemPool::dtor(\"" << m_pTypeName << "\"): " << m_pImpl);
    rtl_cache_destroy (reinterpret_cast<rtl_cache_type*>(m_pImpl));
    m_pImpl = nullptr;
}

void* FixedMemPool::Alloc()
{
    return rtl_cache_alloc (reinterpret_cast<rtl_cache_type*>(m_pImpl));
}

void FixedMemPool::Free( void* pFree )
{
    rtl_cache_free (reinterpret_cast<rtl_cache_type*>(m_pImpl), pFree);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
