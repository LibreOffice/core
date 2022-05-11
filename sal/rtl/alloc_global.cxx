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

#include "alloc_impl.hxx"
#include <rtl/alloc.h>
#include <sal/log.hxx>
#include <sal/macros.h>

#include <algorithm>
#include <cassert>
#include <string.h>
#include <stdio.h>

#include <rtllifecycle.h>
#include <oslmemory.h>

void* SAL_CALL rtl_allocateMemory(sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF(
        n >= SAL_MAX_INT32, "sal.rtl",
        "suspicious massive alloc " << n);
    return malloc (n);
}

void* SAL_CALL rtl_reallocateMemory(void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF(
        n >= SAL_MAX_INT32, "sal.rtl",
        "suspicious massive alloc " << n);
    return realloc (p, n);
}

void SAL_CALL rtl_freeMemory(void * p) SAL_THROW_EXTERN_C()
{
    free (p);
}

void * SAL_CALL rtl_allocateZeroMemory(sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF( n >= SAL_MAX_INT32, "sal.rtl", "suspicious massive alloc " << n);
    return calloc(n, 1);
}

void SAL_CALL rtl_freeZeroMemory(void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        rtl_secureZeroMemory (p, n);
        rtl_freeMemory (p);
    }
}

void* SAL_CALL rtl_allocateAlignedMemory(sal_Size Alignment, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    return osl_aligned_alloc(Alignment, Bytes);
}

void SAL_CALL rtl_freeAlignedMemory(void* Ptr) SAL_THROW_EXTERN_C()
{
    osl_aligned_free(Ptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
