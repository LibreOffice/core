/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interlck.c,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:57:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    oslInterlockedCount nCount;

    __asm__ __volatile__ (
        "movl $1, %0\n\t"
        "lock\n\t"
        "xaddl %0, %2\n\t"
        "incl %0"
    :   "=&r" (nCount), "=m" (*pCount)
    :   "m" (*pCount)
    :   "memory");

    return nCount;
}

oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    oslInterlockedCount nCount;

    __asm__ __volatile__ (
        "movl $-1, %0\n\t"
        "lock\n\t"
        "xaddl %0, %2\n\t"
        "decl %0"
    :   "=&r" (nCount), "=m" (*pCount)
    :   "m" (*pCount)
    :   "memory");

    return nCount;
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
