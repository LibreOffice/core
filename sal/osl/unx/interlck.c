/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "system.h"

#include <osl/armarch.h>
#include <osl/interlck.h>
#include <osl/diagnose.h>

#if  ( defined ( SOLARIS ) || defined ( NETBSD ) ) && defined ( SPARC )
#error please use asm/interlck_sparc.s
#elif defined ( SOLARIS) && defined ( X86 )
#error please use asm/interlck_x86.s
#elif defined ( ARM ) && (( __GNUC__ < 4 ) || (( __GNUC__ == 4) && ( __GNUC_MINOR__ < 6 ))) && ( __ARM_ARCH__ >= 6)
// assembler implementation for gcc <4.6 on arm
// originally contributed by Eric Bachard / OOo4Kids
// #i117017# and lp#726529
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    register oslInterlockedCount nCount __asm__ ("r1");
    int nResult;

    __asm__ __volatile__ (
"1:   ldrex %0, [%3]\n"
"     add %0, %0, #1\n"
"     strex %1, %0, [%3]\n"
"     teq %1, #0\n"
"     bne 1b"
        : "=&r" (nCount), "=&r" (nResult), "=m" (*pCount)
        : "r" (pCount)
        : "memory");

    return nCount;
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    register oslInterlockedCount nCount __asm__ ("r1");
    int nResult;

    __asm__ __volatile__ (
"0:   ldrex %0, [%3]\n"
"     sub %0, %0, #1\n"
"     strex %1, %0, [%3]\n"
"     teq %1, #0\n"
"     bne 0b"
        : "=&r" (nCount), "=&r" (nResult), "=m" (*pCount)
        : "r" (pCount)
        : "memory");

    return nCount;
}
#elif defined ( GCC ) && ( defined ( X86 ) || defined ( X86_64 ) )
/* That's possible on x86-64 too since oslInterlockedCount is a sal_Int32 */

extern int osl_isSingleCPU;

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    // Fast case for old, slow, single CPU Intel machines for whom
    // interlocking is a performance nightmare.
    register oslInterlockedCount nCount asm("%eax");
    nCount = 1;

    if ( osl_isSingleCPU ) {
        __asm__ __volatile__ (
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
        return ++nCount;
    }
#if ( __GNUC__ > 4 ) || (( __GNUC__ == 4)  && ( __GNUC_MINOR__ >= 4 ))
    else
        return __sync_add_and_fetch (pCount, 1);
#else
    else {
        __asm__ __volatile__ (
            "lock\n\t"
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
    }
    return ++nCount;
#endif
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    register oslInterlockedCount nCount asm("%eax");
    nCount = -1;

    if ( osl_isSingleCPU ) {
        __asm__ __volatile__ (
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
        return --nCount;
    }
#if ( __GNUC__ > 4 ) || (( __GNUC__ == 4)  && ( __GNUC_MINOR__ >= 4 ))
    else
        return __sync_sub_and_fetch (pCount, 1);
#else
    else {
        __asm__ __volatile__ (
            "lock\n\t"
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
    }
    return --nCount;
#endif
}

#elif ( __GNUC__ > 4 ) || (( __GNUC__ == 4) && ( __GNUC_MINOR__ >= 4 ))
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    return __sync_add_and_fetch(pCount, 1);
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    return __sync_sub_and_fetch(pCount, 1);
}
#else
/* use only if nothing else works, expensive due to single mutex for all reference counts */

static pthread_mutex_t InterLock = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    oslInterlockedCount Count;

    pthread_mutex_lock(&InterLock);
    Count = ++(*pCount);
    pthread_mutex_unlock(&InterLock);

    return (Count);
}

/*****************************************************************************/
/* osl_decrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    oslInterlockedCount Count;

    pthread_mutex_lock(&InterLock);
    Count = --(*pCount);
    pthread_mutex_unlock(&InterLock);

    return (Count);
}

#endif /* default */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
