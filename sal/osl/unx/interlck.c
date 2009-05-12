/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interlck.c,v $
 * $Revision: 1.14 $
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

#include <osl/interlck.h>
#include <osl/diagnose.h>

#if  ( defined ( SOLARIS ) || defined ( NETBSD ) ) && defined ( SPARC )
#error please use asm/interlck_sparc.s
#elif defined ( SOLARIS) && defined ( X86 )
#error please use asm/interlck_x86.s
#elif defined ( GCC ) && ( defined ( X86 ) || defined ( X86_64 ) )
/* That's possible on x86-64 too since oslInterlockedCount is a sal_Int32 */

extern int osl_isSingleCPU;

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    register oslInterlockedCount nCount asm("%eax");

    nCount = 1;

    if ( osl_isSingleCPU ) {
        __asm__ __volatile__ (
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
    }
    else {
        __asm__ __volatile__ (
            "lock\n\t"
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
    }

    return ++nCount;
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
    }
    else {
        __asm__ __volatile__ (
            "lock\n\t"
            "xaddl %0, %1\n\t"
        :   "+r" (nCount), "+m" (*pCount)
        :   /* nothing */
        :   "memory");
    }

    return --nCount;
}

#elif defined ( GCC ) && defined ( POWERPC )

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    /* "addi" doesn't work with r0 as second parameter */
    register oslInterlockedCount nCount __asm__ ("r4");

    __asm__ __volatile__ (
        "1: lwarx   %0,0,%2\n\t"
        "   addi    %0,%0,1\n\t"
        "   stwcx.  %0,0,%2\n\t"
        "   bne-    1b\n\t"
        "   isync"
        : "=&r" (nCount), "=m" (*pCount)
        : "r" (pCount)
        : "memory");

    return nCount;
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    /* "subi" doesn't work with r0 as second parameter */
    register oslInterlockedCount nCount __asm__ ("r4");

    __asm__ __volatile__ (
        "1: lwarx   %0,0,%2\n\t"
        "   subi    %0,%0,1\n\t"
        "   stwcx.  %0,0,%2\n\t"
        "   bne-    1b\n\t"
        "   isync"
        : "=&r" (nCount), "=m" (*pCount)
        : "r" (pCount)
        : "memory");

    return nCount;
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
