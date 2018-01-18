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

#include "system.hxx"

#include <config_global.h>
#include <osl/interlck.h>

#if  ( defined (__sun) || defined ( NETBSD ) ) && defined ( SPARC )
#error please use asm/interlck_sparc.s
#elif defined (__sun) && defined ( X86 )
#error please use asm/interlck_x86.s
#elif HAVE_GCC_BUILTIN_ATOMIC
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    return __sync_add_and_fetch(pCount, 1);
}
oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    return __sync_sub_and_fetch(pCount, 1);
}
#elif defined ( __GNUC__ ) && ( defined ( X86 ) || defined ( X86_64 ) )
/* That's possible on x86-64 too since oslInterlockedCount is a sal_Int32 */

oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    register oslInterlockedCount nCount asm("%eax");
    nCount = 1;
    __asm__ __volatile__ (
        "lock\n\t"
        "xaddl %0, %1\n\t"
    :   "+r" (nCount), "+m" (*pCount)
    :   /* nothing */
    :   "memory");
    return ++nCount;
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    register oslInterlockedCount nCount asm("%eax");
    nCount = -1;
    __asm__ __volatile__ (
        "lock\n\t"
        "xaddl %0, %1\n\t"
    :   "+r" (nCount), "+m" (*pCount)
    :   /* nothing */
    :   "memory");
    return --nCount;
}
#else
/* use only if nothing else works, expensive due to single mutex for all reference counts */

static pthread_mutex_t InterLock = PTHREAD_MUTEX_INITIALIZER;

oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    oslInterlockedCount Count;

    pthread_mutex_lock(&InterLock);
    Count = ++(*pCount);
    pthread_mutex_unlock(&InterLock);

    return Count;
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    oslInterlockedCount Count;

    pthread_mutex_lock(&InterLock);
    Count = --(*pCount);
    pthread_mutex_unlock(&InterLock);

    return Count;
}

#endif /* default */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
