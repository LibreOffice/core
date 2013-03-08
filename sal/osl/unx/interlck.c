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




#include "system.h"

#include <osl/interlck.h>
#include <osl/diagnose.h>

#if  ( defined ( SOLARIS ) || defined ( NETBSD ) ) && defined ( SPARC )
#error please use asm/interlck_sparc.s
#elif defined ( SOLARIS) && defined ( X86 )
#error please use asm/interlck_x86.s
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(X86) || defined(X86_64))
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

#elif defined ( GCC ) && defined ( ARM )

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
#if defined( ARMV7 ) || defined( ARMV6 )
    register oslInterlockedCount nCount __asm__ ("r1");
    int nResult;

    __asm__ __volatile__ (
"1: ldrex %0, [%3]\n"
"   add %0, %0, #1\n"
"   strex %1, %0, [%3]\n"
"   teq %1, #0\n"
"   bne 1b"
        : "=&r" (nCount), "=&r" (nResult), "=m" (*pCount)
        : "r" (pCount)
        : "memory");

    return nCount;
#else
    return __sync_add_and_fetch( pCount, 1 );
#endif
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
#if defined( ARMV7 ) || defined( ARMV6 )
    register oslInterlockedCount nCount __asm__ ("r1");
    int nResult;

    __asm__ __volatile__ (
"0: ldrex %0, [%3]\n"
"   sub %0, %0, #1\n"
"   strex %1, %0, [%3]\n"
"   teq %1, #0\n"
"   bne 0b"
        : "=&r" (nCount), "=&r" (nResult), "=m" (*pCount)
        : "r" (pCount)
        : "memory");
    return nCount;
#else
    return __sync_sub_and_fetch( pCount, 1 );
#endif
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

