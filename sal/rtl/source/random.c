/*************************************************************************
 *
 *  $RCSfile: random.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:50:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _RTL_RANDOM_C_ "$Revision: 1.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif

#ifndef _RTL_RANDOM_H_
#include <rtl/random.h>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

/*========================================================================
 *
 * rtlRandom internals.
 *
 *======================================================================*/
#define RTL_RANDOM_RNG_1(a) ((a) * 16807L)
#define RTL_RANDOM_RNG_2(a) ((a) * 65539L)

#define RTL_RANDOM_RNG(x, y, z) \
{ \
    (x) = 170 * ((x) % 178) - 63 * ((x) / 178); \
    if ((x) < 0) (x) += 30328L; \
    \
    (y) = 171 * ((y) % 177) -  2 * ((y) / 177); \
    if ((y) < 0) (y) += 30269L; \
    \
    (z) = 172 * ((z) % 176) - 35 * ((z) / 176); \
    if ((z) < 0) (z) += 30307L; \
}

/** RandomData_Impl.
 */
typedef struct random_data_impl_st
{
    sal_Int16 m_nX;
    sal_Int16 m_nY;
    sal_Int16 m_nZ;
} RandomData_Impl;

/** __rtl_random_data.
 */
static double __rtl_random_data (RandomData_Impl *pImpl);

/** RandomPool_Impl.
 */
#define RTL_RANDOM_DIGEST      rtl_Digest_AlgorithmMD5
#define RTL_RANDOM_SIZE_DIGEST RTL_DIGEST_LENGTH_MD5
#define RTL_RANDOM_SIZE_POOL   1023

typedef struct random_pool_impl_st
{
    rtlDigest  m_hDigest;
    sal_uInt8  m_pDigest[RTL_RANDOM_SIZE_DIGEST];
    sal_uInt8  m_pData[RTL_RANDOM_SIZE_POOL + 1];
    sal_uInt32 m_nData;
    sal_uInt32 m_nIndex;
    sal_uInt32 m_nCount;
} RandomPool_Impl;

/** __rtl_random_initPool.
 */
static sal_Bool __rtl_random_initPool (
    RandomPool_Impl *pImpl);

/** __rtl_random_seedPool.
 */
static void __rtl_random_seedPool (
    RandomPool_Impl *pImpl, const sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

/** __rtl_random_readPool.
 */
static void __rtl_random_readPool (
    RandomPool_Impl *pImpl, sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

/*
 * __rtl_random_data.
 */
static double __rtl_random_data (RandomData_Impl *pImpl)
{
    register double random;

    RTL_RANDOM_RNG (pImpl->m_nX, pImpl->m_nY, pImpl->m_nZ);
    random = (((double)(pImpl->m_nX) / 30328.0) +
              ((double)(pImpl->m_nY) / 30269.0) +
              ((double)(pImpl->m_nZ) / 30307.0)   );

    random -= ((double)((sal_uInt32)(random)));
    return (random);
}

/*
 * __rtl_random_initPool.
 */
static sal_Bool __rtl_random_initPool (RandomPool_Impl *pImpl)
{
    pImpl->m_hDigest = rtl_digest_create (RTL_RANDOM_DIGEST);
    if (pImpl->m_hDigest)
    {
        oslThreadIdentifier id;
        TimeValue           tv;
        RandomData_Impl     rd;
        double              seed;

        __rtl_random_seedPool (pImpl, (sal_uInt8*)&id, sizeof(id));
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&tv, sizeof(tv));
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&rd, sizeof(rd));

        id = osl_getThreadIdentifier (NULL);
        id = RTL_RANDOM_RNG_2(RTL_RANDOM_RNG_1(id));
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&id, sizeof(id));

        osl_getSystemTime (&tv);
        tv.Seconds = RTL_RANDOM_RNG_2(tv.Seconds);
        tv.Nanosec = RTL_RANDOM_RNG_2(tv.Nanosec);
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&tv, sizeof(tv));

        rd.m_nX = (sal_Int16)(((id         >> 1) << 1) + 1);
        rd.m_nY = (sal_Int16)(((tv.Seconds >> 1) << 1) + 1);
        rd.m_nZ = (sal_Int16)(((tv.Nanosec >> 1) << 1) + 1);
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&rd, sizeof(rd));

        while (pImpl->m_nData < RTL_RANDOM_SIZE_POOL)
        {
            seed = __rtl_random_data (&rd);
            __rtl_random_seedPool (pImpl, (sal_uInt8*)&seed, sizeof(seed));
        }
        return sal_True;
    }
    return sal_False;
}

/*
 * __rtl_random_seedPool.
 */
static void __rtl_random_seedPool (
    RandomPool_Impl *pImpl, const sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
{
    sal_uInt32 i;
    sal_Int32  j, k;

    for (i = 0; i < nBufLen; i += RTL_RANDOM_SIZE_DIGEST)
    {
        j = nBufLen - i;
        if (j > RTL_RANDOM_SIZE_DIGEST)
            j = RTL_RANDOM_SIZE_DIGEST;

        rtl_digest_update (
            pImpl->m_hDigest, pImpl->m_pDigest, RTL_RANDOM_SIZE_DIGEST);

        k = (pImpl->m_nIndex + j) - RTL_RANDOM_SIZE_POOL;
        if (k > 0)
        {
            rtl_digest_update (
                pImpl->m_hDigest, &(pImpl->m_pData[pImpl->m_nIndex]), j - k);
            rtl_digest_update (
                pImpl->m_hDigest, &(pImpl->m_pData[0]), k);
        }
        else
        {
            rtl_digest_update (
                pImpl->m_hDigest, &(pImpl->m_pData[pImpl->m_nIndex]), j);
        }

        rtl_digest_update (pImpl->m_hDigest, pBuffer, j);
        pBuffer += j;

        rtl_digest_get (
            pImpl->m_hDigest, pImpl->m_pDigest, RTL_RANDOM_SIZE_DIGEST);
        for (k = 0; k < j; k++)
        {
            pImpl->m_pData[pImpl->m_nIndex++] ^= pImpl->m_pDigest[k];
            if (pImpl->m_nIndex >= RTL_RANDOM_SIZE_POOL)
            {
                pImpl->m_nData  = RTL_RANDOM_SIZE_POOL;
                pImpl->m_nIndex = 0;
            }
        }
    }

    if (pImpl->m_nIndex > pImpl->m_nData)
        pImpl->m_nData = pImpl->m_nIndex;
}

/*
 * __rtl_random_readPool.
 */
static void __rtl_random_readPool (
    RandomPool_Impl *pImpl, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
{
    sal_Int32 j, k;

    while (nBufLen > 0)
    {
        j = nBufLen;
        if (j > RTL_RANDOM_SIZE_DIGEST/2)
            j = RTL_RANDOM_SIZE_DIGEST/2;
        nBufLen -= j;

        rtl_digest_update (
            pImpl->m_hDigest,
            &(pImpl->m_pDigest[RTL_RANDOM_SIZE_DIGEST/2]),
            RTL_RANDOM_SIZE_DIGEST/2);
        rtl_digest_update (
            pImpl->m_hDigest, pBuffer, j);

        k = (pImpl->m_nIndex + j) - pImpl->m_nData;
        if (k > 0)
        {
            rtl_digest_update (
                pImpl->m_hDigest, &(pImpl->m_pData[pImpl->m_nIndex]), j - k);
            rtl_digest_update (
                pImpl->m_hDigest, &(pImpl->m_pData[0]), k);
        }
        else
        {
            rtl_digest_update (
                pImpl->m_hDigest, &(pImpl->m_pData[pImpl->m_nIndex]), j);
        }

        rtl_digest_get (
            pImpl->m_hDigest, pImpl->m_pDigest, RTL_RANDOM_SIZE_DIGEST);
        for (k = 0; k < j; k++)
        {
            if (pImpl->m_nIndex >= pImpl->m_nData) pImpl->m_nIndex = 0;
            pImpl->m_pData[pImpl->m_nIndex++] ^= pImpl->m_pDigest[k];
            *pBuffer++ = pImpl->m_pDigest[k + RTL_RANDOM_SIZE_DIGEST/2];
        }
    }

    pImpl->m_nCount++;
    rtl_digest_update (
        pImpl->m_hDigest, &(pImpl->m_nCount), sizeof(pImpl->m_nCount));
    rtl_digest_update (
        pImpl->m_hDigest, pImpl->m_pDigest, RTL_RANDOM_SIZE_DIGEST);
    rtl_digest_get (
        pImpl->m_hDigest, pImpl->m_pDigest, RTL_RANDOM_SIZE_DIGEST);
}

/*========================================================================
 *
 * rtlRandom implementation.
 *
 *======================================================================*/
/*
 * rtl_random_createPool.
 */
rtlRandomPool SAL_CALL rtl_random_createPool (void)
{
    RandomPool_Impl *pImpl = (RandomPool_Impl*)NULL;
    pImpl = (RandomPool_Impl*)rtl_allocateZeroMemory (sizeof(RandomPool_Impl));
    if (pImpl)
    {
        if (!__rtl_random_initPool (pImpl))
        {
            rtl_freeZeroMemory (pImpl, sizeof(RandomPool_Impl));
            pImpl = (RandomPool_Impl*)NULL;
        }
    }
    return ((rtlRandomPool)pImpl);
}

/*
 * rtl_random_destroyPool.
 */
void SAL_CALL rtl_random_destroyPool (rtlRandomPool Pool)
{
    RandomPool_Impl *pImpl = (RandomPool_Impl *)Pool;
    if (pImpl)
    {
        rtl_digest_destroy (pImpl->m_hDigest);
        rtl_freeZeroMemory (pImpl, sizeof (RandomPool_Impl));
    }
}

/*
 * rtl_random_addBytes.
 */
rtlRandomError SAL_CALL rtl_random_addBytes (
    rtlRandomPool Pool, const void *Buffer, sal_uInt32 Bytes)
{
    RandomPool_Impl *pImpl   = (RandomPool_Impl *)Pool;
    const sal_uInt8 *pBuffer = (const sal_uInt8 *)Buffer;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Random_E_Argument;

    __rtl_random_seedPool (pImpl, pBuffer, Bytes);
    return rtl_Random_E_None;
}

/*
 * rtl_random_getBytes.
 */
rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool Pool, void *Buffer, sal_uInt32 Bytes)
{
    RandomPool_Impl *pImpl   = (RandomPool_Impl *)Pool;
    sal_uInt8       *pBuffer = (sal_uInt8 *)Buffer;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Random_E_Argument;

    __rtl_random_readPool (pImpl, pBuffer, Bytes);
    return rtl_Random_E_None;
}

