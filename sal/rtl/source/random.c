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

#define _RTL_RANDOM_C_ "$Revision: 1.6 $"

#include <sal/types.h>
#include <osl/thread.h>
#include <osl/time.h>
#include <rtl/alloc.h>
#include <rtl/digest.h>
#include <rtl/random.h>
#include <osl/time.h>

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
    RandomPool_Impl *pImpl, const sal_uInt8 *pBuffer, sal_Size nBufLen);

/** __rtl_random_readPool.
 */
static void __rtl_random_readPool (
    RandomPool_Impl *pImpl, sal_uInt8 *pBuffer, sal_Size nBufLen);

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

        /* The use of uninitialized stack variables as a way to
         * enhance the entropy of the random pool triggers
         * memory checkers like purify and valgrind.
         */

        /*
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&id, sizeof(id));
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&tv, sizeof(tv));
        __rtl_random_seedPool (pImpl, (sal_uInt8*)&rd, sizeof(rd));
        */

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
    RandomPool_Impl *pImpl, const sal_uInt8 *pBuffer, sal_Size nBufLen)
{
    sal_Size i;
    sal_sSize  j, k;

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
    RandomPool_Impl *pImpl, sal_uInt8 *pBuffer, sal_Size nBufLen)
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
    rtlRandomPool Pool, const void *Buffer, sal_Size Bytes)
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
    rtlRandomPool Pool, void *Buffer, sal_Size Bytes)
{
    RandomPool_Impl *pImpl   = (RandomPool_Impl *)Pool;
    sal_uInt8       *pBuffer = (sal_uInt8 *)Buffer;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Random_E_Argument;

    __rtl_random_readPool (pImpl, pBuffer, Bytes);
    return rtl_Random_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
