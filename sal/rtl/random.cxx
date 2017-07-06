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

#include <sal/types.h>
#include <osl/thread.h>
#include <osl/thread.hxx>
#include <osl/time.h>
#include <rtl/alloc.h>
#include <rtl/digest.h>
#include <rtl/random.h>
#include "oslrandom.h"
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
struct RandomData_Impl
{
    sal_Int16 m_nX;
    sal_Int16 m_nY;
    sal_Int16 m_nZ;
};

/** data.
 */
static double data (RandomData_Impl *pImpl);

/** RandomPool_Impl.
 */
#define RTL_RANDOM_DIGEST      rtl_Digest_AlgorithmMD5
#define RTL_RANDOM_SIZE_DIGEST RTL_DIGEST_LENGTH_MD5
#define RTL_RANDOM_SIZE_POOL   1023

struct RandomPool_Impl
{
    rtlDigest  m_hDigest;
    sal_uInt8  m_pDigest[RTL_RANDOM_SIZE_DIGEST];
    sal_uInt8  m_pData[RTL_RANDOM_SIZE_POOL + 1];
    sal_uInt32 m_nData;
    sal_uInt32 m_nIndex;
    sal_uInt32 m_nCount;
};

/** initPool.
 */
static bool initPool (
    RandomPool_Impl *pImpl);

/** seedPool.
 */
static void seedPool (
    RandomPool_Impl *pImpl, const sal_uInt8 *pBuffer, sal_Size nBufLen);

/** readPool.
 */
static void readPool (
    RandomPool_Impl *pImpl, sal_uInt8 *pBuffer, sal_Size nBufLen);

/*
 * data.
 */
static double data (RandomData_Impl *pImpl)
{
    double random;

    RTL_RANDOM_RNG (pImpl->m_nX, pImpl->m_nY, pImpl->m_nZ);
    random = (((double)(pImpl->m_nX) / 30328.0) +
              ((double)(pImpl->m_nY) / 30269.0) +
              ((double)(pImpl->m_nZ) / 30307.0)   );

    random -= ((double)((sal_uInt32)random));
    return random;
}

/*
 * initPool.
 */
static bool initPool (RandomPool_Impl *pImpl)
{
    pImpl->m_hDigest = rtl_digest_create (RTL_RANDOM_DIGEST);
    if (pImpl->m_hDigest)
    {
        oslThreadIdentifier tid;
        TimeValue           tv;
        RandomData_Impl     rd;
        double              seed;

        /* The use of uninitialized stack variables as a way to
         * enhance the entropy of the random pool triggers
         * memory checkers like purify and valgrind.
         */

        /*
        seedPool (pImpl, (sal_uInt8*)&tid, sizeof(tid));
        seedPool (pImpl, (sal_uInt8*)&tv, sizeof(tv));
        seedPool (pImpl, (sal_uInt8*)&rd, sizeof(rd));
        */

        tid = osl::Thread::getCurrentIdentifier();
        tid = RTL_RANDOM_RNG_2(RTL_RANDOM_RNG_1(tid));
        seedPool (pImpl, reinterpret_cast<sal_uInt8*>(&tid), sizeof(tid));

        osl_getSystemTime (&tv);
        tv.Seconds = RTL_RANDOM_RNG_2(tv.Seconds);
        tv.Nanosec = RTL_RANDOM_RNG_2(tv.Nanosec);
        seedPool (pImpl, reinterpret_cast<sal_uInt8*>(&tv), sizeof(tv));

        rd.m_nX = (sal_Int16)(((tid         >> 1) << 1) + 1);
        rd.m_nY = (sal_Int16)(((tv.Seconds >> 1) << 1) + 1);
        rd.m_nZ = (sal_Int16)(((tv.Nanosec >> 1) << 1) + 1);
        seedPool (pImpl, reinterpret_cast<sal_uInt8*>(&rd), sizeof(rd));

        while (pImpl->m_nData < RTL_RANDOM_SIZE_POOL)
        {
            seed = data (&rd);
            seedPool (pImpl, reinterpret_cast<sal_uInt8*>(&seed), sizeof(seed));
        }
        return true;
    }
    return false;
}

/*
 * seedPool.
 */
static void seedPool (
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
 * readPool.
 */
static void readPool (
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
rtlRandomPool SAL_CALL rtl_random_createPool() SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl = nullptr;
    char sanity[4];

    /* try to get system random number, if it fail fall back on own pool */
    pImpl = static_cast<RandomPool_Impl*>(rtl_allocateZeroMemory (sizeof(RandomPool_Impl)));
    if (pImpl)
    {
        if(!osl_get_system_random_data(sanity, 4))
        {
            if (!initPool (pImpl))
            {
                rtl_freeZeroMemory (pImpl, sizeof(RandomPool_Impl));
                pImpl = nullptr;
            }
        }
    }
    return static_cast<rtlRandomPool>(pImpl);
}

/*
 * rtl_random_destroyPool.
 */
void SAL_CALL rtl_random_destroyPool (rtlRandomPool Pool) SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl = static_cast<RandomPool_Impl *>(Pool);
    if (pImpl)
    {
        if(pImpl->m_hDigest)
        {
            rtl_digest_destroy (pImpl->m_hDigest);
        }
        rtl_freeZeroMemory (pImpl, sizeof (RandomPool_Impl));
    }
}

/*
 * rtl_random_addBytes.
 */
rtlRandomError SAL_CALL rtl_random_addBytes (
    rtlRandomPool Pool, const void *Buffer, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl   = static_cast<RandomPool_Impl *>(Pool);
    const sal_uInt8 *pBuffer = static_cast<const sal_uInt8 *>(Buffer);

    if ((pImpl == nullptr) || (pBuffer == nullptr))
        return rtl_Random_E_Argument;
    if(pImpl->m_hDigest)
    {
        seedPool (pImpl, pBuffer, Bytes);
    }
    return rtl_Random_E_None;
}

/*
 * rtl_random_getBytes.
 */
rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool Pool, void *Buffer, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl   = static_cast<RandomPool_Impl *>(Pool);
    sal_uInt8       *pBuffer = static_cast<sal_uInt8 *>(Buffer);

    if ((pImpl == nullptr) || (pBuffer == nullptr))
        return rtl_Random_E_Argument;

    if(pImpl->m_hDigest || !osl_get_system_random_data(static_cast<char*>(Buffer), Bytes))
    {
        if(!pImpl->m_hDigest)
        {
            if (!initPool (pImpl))
            {
                return rtl_Random_E_Unknown;
            }
        }
        readPool (pImpl, pBuffer, Bytes);
    }
    return rtl_Random_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
