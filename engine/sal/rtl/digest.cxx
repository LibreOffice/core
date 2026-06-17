/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <string.h>
#include <stdlib.h>

#include <sal/types.h>
#include <osl/endian.h>
#include <rtl/alloc.h>
#include <rtl/digest.h>

#define RTL_DIGEST_CREATE(T) (static_cast<T*>(rtl_allocateZeroMemory(sizeof(T))))

#define RTL_DIGEST_ROTL(a,n) (((a) << (n)) | ((a) >> (32 - (n))))

#define RTL_DIGEST_HTONL(l,c) \
    (*((c)++) = static_cast<sal_uInt8>(((l) >> 24) & 0xff), \
     *((c)++) = static_cast<sal_uInt8>(((l) >> 16) & 0xff), \
     *((c)++) = static_cast<sal_uInt8>(((l) >>  8) & 0xff), \
     *((c)++) = static_cast<sal_uInt8>(((l)       ) & 0xff))

#define RTL_DIGEST_LTOC(l,c) \
    *((c)++) = static_cast<sal_uInt8>(((l)      ) & 0xff); \
    *((c)++) = static_cast<sal_uInt8>(((l) >>  8) & 0xff); \
    *((c)++) = static_cast<sal_uInt8>(((l) >> 16) & 0xff); \
    *((c)++) = static_cast<sal_uInt8>(((l) >> 24) & 0xff);

typedef rtlDigestError (Digest_init_t) (
    void *ctx, const sal_uInt8 *Data, sal_uInt32 DatLen);

typedef void (Digest_delete_t) (void *ctx);

typedef rtlDigestError (Digest_update_t) (
    void *ctx, const void *Data, sal_uInt32 DatLen);

typedef rtlDigestError (Digest_get_t) (
    void *ctx, sal_uInt8 *Buffer, sal_uInt32 BufLen);

namespace {

struct Digest_Impl
{
    rtlDigestAlgorithm  m_algorithm;
    sal_uInt32          m_length;

    Digest_init_t      *m_init;
    Digest_delete_t    *m_delete;
    Digest_update_t    *m_update;
    Digest_get_t       *m_get;
};

}

static void swapLong(sal_uInt32 *pData, sal_uInt32 nDatLen)
{
    sal_uInt32 *X;
    int i, n;

    X = pData;
    n = nDatLen;

    for (i = 0; i < n; i++)
    {
        X[i] = OSL_SWAPDWORD(X[i]);
    }
}

rtlDigest SAL_CALL rtl_digest_create(rtlDigestAlgorithm Algorithm) noexcept
{
    rtlDigest Digest = nullptr;
    switch (Algorithm)
    {
        case rtl_Digest_AlgorithmSHA1_StarOfficeBug:
            Digest = rtl_digest_createSHA1_StarOfficeBug();
            break;

        default: /* rtl_Digest_AlgorithmInvalid */
            break;
    }
    return Digest;
}

rtlDigestAlgorithm SAL_CALL rtl_digest_queryAlgorithm(rtlDigest Digest) noexcept
{
    Digest_Impl *pImpl = static_cast<Digest_Impl *>(Digest);
    if (pImpl)
        return pImpl->m_algorithm;
    return rtl_Digest_AlgorithmInvalid;
}

sal_uInt32 SAL_CALL rtl_digest_queryLength(rtlDigest Digest) noexcept
{
    Digest_Impl *pImpl = static_cast<Digest_Impl *>(Digest);
    if (pImpl)
        return pImpl->m_length;
    return 0;
}

rtlDigestError SAL_CALL rtl_digest_init(
    rtlDigest Digest, const sal_uInt8 *pData, sal_uInt32 nDatLen) noexcept
{
    Digest_Impl *pImpl = static_cast<Digest_Impl *>(Digest);
    if (pImpl)
    {
        if (pImpl->m_init)
            return pImpl->m_init (Digest, pData, nDatLen);
        return rtl_Digest_E_None;
    }
    return rtl_Digest_E_Argument;
}

rtlDigestError SAL_CALL rtl_digest_update(
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen) noexcept
{
    Digest_Impl *pImpl = static_cast<Digest_Impl *>(Digest);
    if (pImpl && pImpl->m_update)
        return pImpl->m_update(Digest, pData, nDatLen);
    return rtl_Digest_E_Argument;
}

rtlDigestError SAL_CALL rtl_digest_get(
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen) noexcept
{
    Digest_Impl *pImpl = static_cast<Digest_Impl *>(Digest);
    if (pImpl && pImpl->m_get)
        return pImpl->m_get(Digest, pBuffer, nBufLen);
    return rtl_Digest_E_Argument;
}

void SAL_CALL rtl_digest_destroy(rtlDigest Digest) noexcept
{
    Digest_Impl *pImpl = static_cast<Digest_Impl *>(Digest);
    if (pImpl && pImpl->m_delete)
        pImpl->m_delete(Digest);
}

#define DIGEST_CBLOCK_MD5 64
#define DIGEST_LBLOCK_MD5 16

namespace {

struct DigestContextMD5
{
    sal_uInt32 m_nDatLen;
    sal_uInt32 m_pData[DIGEST_LBLOCK_MD5];
    sal_uInt32 m_nA, m_nB, m_nC, m_nD;
    sal_uInt32 m_nL, m_nH;
};

struct DigestMD5_Impl
{
    Digest_Impl      m_digest;
    DigestContextMD5 m_context;
};

}

static void initMD5 (DigestContextMD5 *ctx);
static void updateMD5 (DigestContextMD5 *ctx);

#define F(x,y,z) ((((y) ^ (z)) & (x)) ^ (z))
#define G(x,y,z) ((((x) ^ (y)) & (z)) ^ (y))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) (((x) | (~(z))) ^ (y))

#define R0(a,b,c,d,k,s,t) { \
    a += ((k) + (t) + F((b), (c), (d))); \
    a  = RTL_DIGEST_ROTL(a, s); \
    a += b; }

#define R1(a,b,c,d,k,s,t) { \
    a += ((k) + (t) + G((b), (c), (d))); \
    a  = RTL_DIGEST_ROTL(a, s); \
    a += b; }

#define R2(a,b,c,d,k,s,t) { \
    a += ((k) + (t) + H((b), (c), (d))); \
    a  = RTL_DIGEST_ROTL(a, s); \
    a += b; }

#define R3(a,b,c,d,k,s,t) { \
    a += ((k) + (t) + I((b), (c), (d))); \
    a  = RTL_DIGEST_ROTL(a, s); \
    a += b; }

const Digest_Impl MD5 =
{
    rtl_Digest_Algorithm_FORCE_EQUAL_SIZE, // this is only used internally now so no need to check it
    RTL_DIGEST_LENGTH_MD5,
    nullptr,
    nullptr, // unused now
    nullptr, // unused now
    nullptr, // unused now
};

static void initMD5(DigestContextMD5 *ctx)
{
    memset(ctx, 0, sizeof(DigestContextMD5));

    ctx->m_nA = sal_uInt32(0x67452301L);
    ctx->m_nB = sal_uInt32(0xefcdab89L);
    ctx->m_nC = sal_uInt32(0x98badcfeL);
    ctx->m_nD = sal_uInt32(0x10325476L);
}

static void updateMD5(DigestContextMD5 *ctx)
{
    sal_uInt32  A, B, C, D;
    sal_uInt32 *X;

    A = ctx->m_nA;
    B = ctx->m_nB;
    C = ctx->m_nC;
    D = ctx->m_nD;
    X = ctx->m_pData;

    R0 (A, B, C, D, X[ 0],  7, 0xd76aa478L);
    R0 (D, A, B, C, X[ 1], 12, 0xe8c7b756L);
    R0 (C, D, A, B, X[ 2], 17, 0x242070dbL);
    R0 (B, C, D, A, X[ 3], 22, 0xc1bdceeeL);
    R0 (A, B, C, D, X[ 4],  7, 0xf57c0fafL);
    R0 (D, A, B, C, X[ 5], 12, 0x4787c62aL);
    R0 (C, D, A, B, X[ 6], 17, 0xa8304613L);
    R0 (B, C, D, A, X[ 7], 22, 0xfd469501L);
    R0 (A, B, C, D, X[ 8],  7, 0x698098d8L);
    R0 (D, A, B, C, X[ 9], 12, 0x8b44f7afL);
    R0 (C, D, A, B, X[10], 17, 0xffff5bb1L);
    R0 (B, C, D, A, X[11], 22, 0x895cd7beL);
    R0 (A, B, C, D, X[12],  7, 0x6b901122L);
    R0 (D, A, B, C, X[13], 12, 0xfd987193L);
    R0 (C, D, A, B, X[14], 17, 0xa679438eL);
    R0 (B, C, D, A, X[15], 22, 0x49b40821L);

    R1 (A, B, C, D, X[ 1],  5, 0xf61e2562L);
    R1 (D, A, B, C, X[ 6],  9, 0xc040b340L);
    R1 (C, D, A, B, X[11], 14, 0x265e5a51L);
    R1 (B, C, D, A, X[ 0], 20, 0xe9b6c7aaL);
    R1 (A, B, C, D, X[ 5],  5, 0xd62f105dL);
    R1 (D, A, B, C, X[10],  9, 0x02441453L);
    R1 (C, D, A, B, X[15], 14, 0xd8a1e681L);
    R1 (B, C, D, A, X[ 4], 20, 0xe7d3fbc8L);
    R1 (A, B, C, D, X[ 9],  5, 0x21e1cde6L);
    R1 (D, A, B, C, X[14],  9, 0xc33707d6L);
    R1 (C, D, A, B, X[ 3], 14, 0xf4d50d87L);
    R1 (B, C, D, A, X[ 8], 20, 0x455a14edL);
    R1 (A, B, C, D, X[13],  5, 0xa9e3e905L);
    R1 (D, A, B, C, X[ 2],  9, 0xfcefa3f8L);
    R1 (C, D, A, B, X[ 7], 14, 0x676f02d9L);
    R1 (B, C, D, A, X[12], 20, 0x8d2a4c8aL);

    R2 (A, B, C, D, X[ 5],  4, 0xfffa3942L);
    R2 (D, A, B, C, X[ 8], 11, 0x8771f681L);
    R2 (C, D, A, B, X[11], 16, 0x6d9d6122L);
    R2 (B, C, D, A, X[14], 23, 0xfde5380cL);
    R2 (A, B, C, D, X[ 1],  4, 0xa4beea44L);
    R2 (D, A, B, C, X[ 4], 11, 0x4bdecfa9L);
    R2 (C, D, A, B, X[ 7], 16, 0xf6bb4b60L);
    R2 (B, C, D, A, X[10], 23, 0xbebfbc70L);
    R2 (A, B, C, D, X[13],  4, 0x289b7ec6L);
    R2 (D, A, B, C, X[ 0], 11, 0xeaa127faL);
    R2 (C, D, A, B, X[ 3], 16, 0xd4ef3085L);
    R2 (B, C, D, A, X[ 6], 23, 0x04881d05L);
    R2 (A, B, C, D, X[ 9],  4, 0xd9d4d039L);
    R2 (D, A, B, C, X[12], 11, 0xe6db99e5L);
    R2 (C, D, A, B, X[15], 16, 0x1fa27cf8L);
    R2 (B, C, D, A, X[ 2], 23, 0xc4ac5665L);

    R3 (A, B, C, D, X[ 0],  6, 0xf4292244L);
    R3 (D, A, B, C, X[ 7], 10, 0x432aff97L);
    R3 (C, D, A, B, X[14], 15, 0xab9423a7L);
    R3 (B, C, D, A, X[ 5], 21, 0xfc93a039L);
    R3 (A, B, C, D, X[12],  6, 0x655b59c3L);
    R3 (D, A, B, C, X[ 3], 10, 0x8f0ccc92L);
    R3 (C, D, A, B, X[10], 15, 0xffeff47dL);
    R3 (B, C, D, A, X[ 1], 21, 0x85845dd1L);
    R3 (A, B, C, D, X[ 8],  6, 0x6fa87e4fL);
    R3 (D, A, B, C, X[15], 10, 0xfe2ce6e0L);
    R3 (C, D, A, B, X[ 6], 15, 0xa3014314L);
    R3 (B, C, D, A, X[13], 21, 0x4e0811a1L);
    R3 (A, B, C, D, X[ 4],  6, 0xf7537e82L);
    R3 (D, A, B, C, X[11], 10, 0xbd3af235L);
    R3 (C, D, A, B, X[ 2], 15, 0x2ad7d2bbL);
    R3 (B, C, D, A, X[ 9], 21, 0xeb86d391L);

    ctx->m_nA += A;
    ctx->m_nB += B;
    ctx->m_nC += C;
    ctx->m_nD += D;
}

namespace {

rtlDigestError rtl_digest_updateMD5(
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen) noexcept
{
    DigestMD5_Impl *pImpl = static_cast<DigestMD5_Impl *>(Digest);
    const sal_uInt8 *d = static_cast<const sal_uInt8 *>(pData);

    DigestContextMD5 *ctx;
    sal_uInt32 len;

    if (!pImpl || !pData)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_Algorithm_FORCE_EQUAL_SIZE)
        return rtl_Digest_E_Algorithm;

    if (nDatLen == 0)
        return rtl_Digest_E_None;

    ctx = &(pImpl->m_context);

    len = ctx->m_nL + (nDatLen << 3);
    if (len < ctx->m_nL)
        ctx->m_nH += 1;

    ctx->m_nH += (nDatLen >> 29);
    ctx->m_nL = len;

    if (ctx->m_nDatLen)
    {
        sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(ctx->m_pData) + ctx->m_nDatLen;
        sal_uInt32 n = DIGEST_CBLOCK_MD5 - ctx->m_nDatLen;

        if (nDatLen < n)
        {
            memcpy(p, d, nDatLen);
            ctx->m_nDatLen += nDatLen;

            return rtl_Digest_E_None;
        }

        memcpy(p, d, n);
        d       += n;
        nDatLen -= n;

#ifdef OSL_BIGENDIAN
        swapLong(ctx->m_pData, DIGEST_LBLOCK_MD5);
#endif /* OSL_BIGENDIAN */

        updateMD5(ctx);
        ctx->m_nDatLen = 0;
    }

    while (nDatLen >= DIGEST_CBLOCK_MD5)
    {
        memcpy(ctx->m_pData, d, DIGEST_CBLOCK_MD5);
        d += DIGEST_CBLOCK_MD5;
        nDatLen -= DIGEST_CBLOCK_MD5;

#ifdef OSL_BIGENDIAN
        swapLong(ctx->m_pData, DIGEST_LBLOCK_MD5);
#endif /* OSL_BIGENDIAN */

        updateMD5(ctx);
    }

    memcpy(ctx->m_pData, d, nDatLen);
    ctx->m_nDatLen = nDatLen;

    return rtl_Digest_E_None;
}

rtlDigestError rtl_digest_rawMD5(
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen) noexcept
{
    DigestMD5_Impl *pImpl = static_cast<DigestMD5_Impl *>(Digest);
    sal_uInt8 *p = pBuffer;

    DigestContextMD5 *ctx;

    if (!pImpl || !pBuffer)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_Algorithm_FORCE_EQUAL_SIZE)
        return rtl_Digest_E_Algorithm;

    if (pImpl->m_digest.m_length > nBufLen)
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    /* not finalized */
    RTL_DIGEST_LTOC(ctx->m_nA, p);
    RTL_DIGEST_LTOC(ctx->m_nB, p);
    RTL_DIGEST_LTOC(ctx->m_nC, p);
    RTL_DIGEST_LTOC(ctx->m_nD, p);
    initMD5(ctx);

    return rtl_Digest_E_None;
}

}

rtlDigestError SAL_CALL rtl_digest_MD5_MSOffice(
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen) noexcept
{
    DigestMD5_Impl digest;
    rtlDigestError result;

    digest.m_digest = MD5;
    initMD5(&(digest.m_context));

    result = rtl_digest_updateMD5(&digest, pData, nDatLen);
    if (result == rtl_Digest_E_None)
        result = rtl_digest_rawMD5(&digest, pBuffer, nBufLen);

    rtl_secureZeroMemory(&digest, sizeof(digest));
    return result;
}


#define DIGEST_CBLOCK_SHA 64
#define DIGEST_LBLOCK_SHA 16

typedef sal_uInt32 DigestSHA_update_t(sal_uInt32 x);

static sal_uInt32 updateSHA_1(sal_uInt32 x);

namespace {

struct DigestContextSHA
{
    DigestSHA_update_t *m_update;
    sal_uInt32          m_nDatLen;
    sal_uInt32          m_pData[DIGEST_LBLOCK_SHA];
    sal_uInt32          m_nA, m_nB, m_nC, m_nD, m_nE;
    sal_uInt32          m_nL, m_nH;
};

struct DigestSHA_Impl
{
    Digest_Impl      m_digest;
    DigestContextSHA m_context;
};

}

static void initSHA(
    DigestContextSHA *ctx, DigestSHA_update_t *fct);

static void updateSHA(DigestContextSHA *ctx);
static void endSHA(DigestContextSHA *ctx);

#define K_00_19 sal_uInt32(0x5a827999L)
#define K_20_39 sal_uInt32(0x6ed9eba1L)
#define K_40_59 sal_uInt32(0x8f1bbcdcL)
#define K_60_79 sal_uInt32(0xca62c1d6L)

#define F_00_19(b,c,d) ((((c) ^ (d)) & (b)) ^ (d))
#define F_20_39(b,c,d) ((b) ^ (c) ^ (d))
#define F_40_59(b,c,d) (((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define F_60_79(b,c,d) F_20_39(b,c,d)

#define BODY_X(i) \
    (X[(i)&0x0f] ^ X[((i)+2)&0x0f] ^ X[((i)+8)&0x0f] ^ X[((i)+13)&0x0f])

#define BODY_00_15(u,i,a,b,c,d,e,f) \
    (f)  = X[i]; \
    (f) += (e) + K_00_19 + RTL_DIGEST_ROTL((a), 5) + F_00_19((b), (c), (d)); \
    (b)  = RTL_DIGEST_ROTL((b), 30);

#define BODY_16_19(u,i,a,b,c,d,e,f) \
    (f)  = BODY_X((i)); \
    (f)  = X[(i)&0x0f] = (u)((f)); \
    (f) += (e) + K_00_19 + RTL_DIGEST_ROTL((a), 5) + F_00_19((b), (c), (d)); \
    (b)  = RTL_DIGEST_ROTL((b), 30);

#define BODY_20_39(u,i,a,b,c,d,e,f) \
    (f)  = BODY_X((i)); \
    (f)  = X[(i)&0x0f] = (u)((f)); \
    (f) += (e) + K_20_39 + RTL_DIGEST_ROTL((a), 5) + F_20_39((b), (c), (d)); \
    (b)  = RTL_DIGEST_ROTL((b), 30);

#define BODY_40_59(u,i,a,b,c,d,e,f) \
    (f)  = BODY_X((i)); \
    (f)  = X[(i)&0x0f] = (u)((f)); \
    (f) += (e) + K_40_59 + RTL_DIGEST_ROTL((a), 5) + F_40_59((b), (c), (d)); \
    (b)  = RTL_DIGEST_ROTL((b), 30);

#define BODY_60_79(u,i,a,b,c,d,e,f) \
    (f)  = BODY_X((i)); \
    (f)  = X[(i)&0x0f] = (u)((f)); \
    (f) += (e) + K_60_79 + RTL_DIGEST_ROTL((a), 5) + F_60_79((b), (c), (d)); \
    (b)  = RTL_DIGEST_ROTL((b), 30);

static void initSHA(
    DigestContextSHA *ctx, DigestSHA_update_t *fct)
{
    memset(ctx, 0, sizeof(DigestContextSHA));
    ctx->m_update = fct;

    ctx->m_nA = sal_uInt32(0x67452301L);
    ctx->m_nB = sal_uInt32(0xefcdab89L);
    ctx->m_nC = sal_uInt32(0x98badcfeL);
    ctx->m_nD = sal_uInt32(0x10325476L);
    ctx->m_nE = sal_uInt32(0xc3d2e1f0L);
}

static void updateSHA(DigestContextSHA *ctx)
{
    sal_uInt32  A, B, C, D, E, T;
    sal_uInt32 *X;

    DigestSHA_update_t *U;
    U = ctx->m_update;

    A = ctx->m_nA;
    B = ctx->m_nB;
    C = ctx->m_nC;
    D = ctx->m_nD;
    E = ctx->m_nE;
    X = ctx->m_pData;

    BODY_00_15 (U,  0, A, B, C, D, E, T);
    BODY_00_15 (U,  1, T, A, B, C, D, E);
    BODY_00_15 (U,  2, E, T, A, B, C, D);
    BODY_00_15 (U,  3, D, E, T, A, B, C);
    BODY_00_15 (U,  4, C, D, E, T, A, B);
    BODY_00_15 (U,  5, B, C, D, E, T, A);
    BODY_00_15 (U,  6, A, B, C, D, E, T);
    BODY_00_15 (U,  7, T, A, B, C, D, E);
    BODY_00_15 (U,  8, E, T, A, B, C, D);
    BODY_00_15 (U,  9, D, E, T, A, B, C);
    BODY_00_15 (U, 10, C, D, E, T, A, B);
    BODY_00_15 (U, 11, B, C, D, E, T, A);
    BODY_00_15 (U, 12, A, B, C, D, E, T);
    BODY_00_15 (U, 13, T, A, B, C, D, E);
    BODY_00_15 (U, 14, E, T, A, B, C, D);
    BODY_00_15 (U, 15, D, E, T, A, B, C);
    BODY_16_19 (U, 16, C, D, E, T, A, B);
    BODY_16_19 (U, 17, B, C, D, E, T, A);
    BODY_16_19 (U, 18, A, B, C, D, E, T);
    BODY_16_19 (U, 19, T, A, B, C, D, E);

    BODY_20_39 (U, 20, E, T, A, B, C, D);
    BODY_20_39 (U, 21, D, E, T, A, B, C);
    BODY_20_39 (U, 22, C, D, E, T, A, B);
    BODY_20_39 (U, 23, B, C, D, E, T, A);
    BODY_20_39 (U, 24, A, B, C, D, E, T);
    BODY_20_39 (U, 25, T, A, B, C, D, E);
    BODY_20_39 (U, 26, E, T, A, B, C, D);
    BODY_20_39 (U, 27, D, E, T, A, B, C);
    BODY_20_39 (U, 28, C, D, E, T, A, B);
    BODY_20_39 (U, 29, B, C, D, E, T, A);
    BODY_20_39 (U, 30, A, B, C, D, E, T);
    BODY_20_39 (U, 31, T, A, B, C, D, E);
    BODY_20_39 (U, 32, E, T, A, B, C, D);
    BODY_20_39 (U, 33, D, E, T, A, B, C);
    BODY_20_39 (U, 34, C, D, E, T, A, B);
    BODY_20_39 (U, 35, B, C, D, E, T, A);
    BODY_20_39 (U, 36, A, B, C, D, E, T);
    BODY_20_39 (U, 37, T, A, B, C, D, E);
    BODY_20_39 (U, 38, E, T, A, B, C, D);
    BODY_20_39 (U, 39, D, E, T, A, B, C);

    BODY_40_59 (U, 40, C, D, E, T, A, B);
    BODY_40_59 (U, 41, B, C, D, E, T, A);
    BODY_40_59 (U, 42, A, B, C, D, E, T);
    BODY_40_59 (U, 43, T, A, B, C, D, E);
    BODY_40_59 (U, 44, E, T, A, B, C, D);
    BODY_40_59 (U, 45, D, E, T, A, B, C);
    BODY_40_59 (U, 46, C, D, E, T, A, B);
    BODY_40_59 (U, 47, B, C, D, E, T, A);
    BODY_40_59 (U, 48, A, B, C, D, E, T);
    BODY_40_59 (U, 49, T, A, B, C, D, E);
    BODY_40_59 (U, 50, E, T, A, B, C, D);
    BODY_40_59 (U, 51, D, E, T, A, B, C);
    BODY_40_59 (U, 52, C, D, E, T, A, B);
    BODY_40_59 (U, 53, B, C, D, E, T, A);
    BODY_40_59 (U, 54, A, B, C, D, E, T);
    BODY_40_59 (U, 55, T, A, B, C, D, E);
    BODY_40_59 (U, 56, E, T, A, B, C, D);
    BODY_40_59 (U, 57, D, E, T, A, B, C);
    BODY_40_59 (U, 58, C, D, E, T, A, B);
    BODY_40_59 (U, 59, B, C, D, E, T, A);

    BODY_60_79 (U, 60, A, B, C, D, E, T);
    BODY_60_79 (U, 61, T, A, B, C, D, E);
    BODY_60_79 (U, 62, E, T, A, B, C, D);
    BODY_60_79 (U, 63, D, E, T, A, B, C);
    BODY_60_79 (U, 64, C, D, E, T, A, B);
    BODY_60_79 (U, 65, B, C, D, E, T, A);
    BODY_60_79 (U, 66, A, B, C, D, E, T);
    BODY_60_79 (U, 67, T, A, B, C, D, E);
    BODY_60_79 (U, 68, E, T, A, B, C, D);
    BODY_60_79 (U, 69, D, E, T, A, B, C);
    BODY_60_79 (U, 70, C, D, E, T, A, B);
    BODY_60_79 (U, 71, B, C, D, E, T, A);
    BODY_60_79 (U, 72, A, B, C, D, E, T);
    BODY_60_79 (U, 73, T, A, B, C, D, E);
    BODY_60_79 (U, 74, E, T, A, B, C, D);
    BODY_60_79 (U, 75, D, E, T, A, B, C);
    BODY_60_79 (U, 76, C, D, E, T, A, B);
    BODY_60_79 (U, 77, B, C, D, E, T, A);
    BODY_60_79 (U, 78, A, B, C, D, E, T);
    BODY_60_79 (U, 79, T, A, B, C, D, E);

    ctx->m_nA += E;
    ctx->m_nB += T;
    ctx->m_nC += A;
    ctx->m_nD += B;
    ctx->m_nE += C;
}

static void endSHA(DigestContextSHA *ctx)
{
    static const sal_uInt8 end[4] =
    {
        0x80, 0x00, 0x00, 0x00
    };
    const sal_uInt8 *p = end;

    sal_uInt32 *X;
    int i;

    X = ctx->m_pData;
    i = (ctx->m_nDatLen >> 2);

#ifdef OSL_BIGENDIAN
    swapLong(X, i + 1);
#endif /* OSL_BIGENDIAN */

    switch (ctx->m_nDatLen & 0x03)
    {
        case 1: X[i] &= 0x000000ff; break;
        case 2: X[i] &= 0x0000ffff; break;
        case 3: X[i] &= 0x00ffffff; break;
    }

    switch (ctx->m_nDatLen & 0x03)
    {
        case 0: X[i]  = static_cast<sal_uInt32>(*(p++)) <<  0;
            [[fallthrough]];
        case 1: X[i] |= static_cast<sal_uInt32>(*(p++)) <<  8;
            [[fallthrough]];
        case 2: X[i] |= static_cast<sal_uInt32>(*(p++)) << 16;
            [[fallthrough]];
        case 3: X[i] |= static_cast<sal_uInt32>(*(p++)) << 24;
    }

    swapLong(X, i + 1);

    i += 1;

    // tdf#114939 NB: this is WRONG and should be ">" not ">=" but is not
    // fixed as this buggy SHA1 implementation is needed for compatibility
    if (i >= (DIGEST_LBLOCK_SHA - 2))
    {
        for (; i < DIGEST_LBLOCK_SHA; i++)
        {
            X[i] = 0;
        }

        updateSHA(ctx);
        i = 0;
    }

    for (; i < (DIGEST_LBLOCK_SHA - 2); i++)
    {
        X[i] = 0;
    }

    X[DIGEST_LBLOCK_SHA - 2] = ctx->m_nH;
    X[DIGEST_LBLOCK_SHA - 1] = ctx->m_nL;

    updateSHA(ctx);
}

const Digest_Impl SHA_1 =
{
    rtl_Digest_AlgorithmSHA1_StarOfficeBug,
    RTL_DIGEST_LENGTH_SHA1,
    nullptr,
    rtl_digest_destroySHA1,
    rtl_digest_updateSHA1,
    rtl_digest_getSHA1
};

static sal_uInt32 updateSHA_1(sal_uInt32 x)
{
    return RTL_DIGEST_ROTL(x, 1);
}

rtlDigestError SAL_CALL rtl_digest_SHA1_StarOfficeBug(
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen) noexcept
{
    DigestSHA_Impl digest;
    rtlDigestError result;

    digest.m_digest = SHA_1;
    initSHA(&(digest.m_context), updateSHA_1);

    result = rtl_digest_updateSHA1(&digest, pData, nDatLen);
    if (result == rtl_Digest_E_None)
        result = rtl_digest_getSHA1(&digest, pBuffer, nBufLen);

    rtl_secureZeroMemory(&digest, sizeof(digest));
    return result;
}

rtlDigest SAL_CALL rtl_digest_createSHA1_StarOfficeBug() noexcept
{
    DigestSHA_Impl *pImpl = RTL_DIGEST_CREATE(DigestSHA_Impl);
    if (pImpl)
    {
        pImpl->m_digest = SHA_1;
        initSHA(&(pImpl->m_context), updateSHA_1);
    }
    return static_cast<rtlDigest>(pImpl);
}

rtlDigestError SAL_CALL rtl_digest_updateSHA1(
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen) noexcept
{
    DigestSHA_Impl *pImpl = static_cast< DigestSHA_Impl * >(Digest);
    const sal_uInt8 *d = static_cast< const sal_uInt8 * >(pData);

    DigestContextSHA *ctx;
    sal_uInt32 len;

    if (!pImpl || !pData)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_AlgorithmSHA1_StarOfficeBug)
        return rtl_Digest_E_Algorithm;

    if (nDatLen == 0)
        return rtl_Digest_E_None;

    ctx = &(pImpl->m_context);

    len = ctx->m_nL + (nDatLen << 3);
    if (len < ctx->m_nL)
        ctx->m_nH += 1;

    ctx->m_nH += (nDatLen >> 29);
    ctx->m_nL = len;

    if (ctx->m_nDatLen)
    {
        sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(ctx->m_pData) + ctx->m_nDatLen;
        sal_uInt32 n = DIGEST_CBLOCK_SHA - ctx->m_nDatLen;

        if (nDatLen < n)
        {
            memcpy(p, d, nDatLen);
            ctx->m_nDatLen += nDatLen;

            return rtl_Digest_E_None;
        }

        memcpy(p, d, n);
        d += n;
        nDatLen -= n;

#ifndef OSL_BIGENDIAN
        swapLong(ctx->m_pData, DIGEST_LBLOCK_SHA);
#endif /* OSL_BIGENDIAN */

        updateSHA(ctx);
        ctx->m_nDatLen = 0;
    }

    while (nDatLen >= DIGEST_CBLOCK_SHA)
    {
        memcpy(ctx->m_pData, d, DIGEST_CBLOCK_SHA);
        d += DIGEST_CBLOCK_SHA;
        nDatLen -= DIGEST_CBLOCK_SHA;

#ifndef OSL_BIGENDIAN
        swapLong(ctx->m_pData, DIGEST_LBLOCK_SHA);
#endif /* OSL_BIGENDIAN */

        updateSHA(ctx);
    }

    memcpy(ctx->m_pData, d, nDatLen);
    ctx->m_nDatLen = nDatLen;

    return rtl_Digest_E_None;
}

rtlDigestError SAL_CALL rtl_digest_getSHA1 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen) noexcept
{
    DigestSHA_Impl *pImpl = static_cast<DigestSHA_Impl *>(Digest);
    sal_uInt8 *p = pBuffer;

    DigestContextSHA *ctx;

    if (!pImpl || !pBuffer)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_AlgorithmSHA1_StarOfficeBug)
        return rtl_Digest_E_Algorithm;

    if (pImpl->m_digest.m_length > nBufLen)
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    endSHA(ctx);
    RTL_DIGEST_HTONL(ctx->m_nA, p);
    RTL_DIGEST_HTONL(ctx->m_nB, p);
    RTL_DIGEST_HTONL(ctx->m_nC, p);
    RTL_DIGEST_HTONL(ctx->m_nD, p);
    RTL_DIGEST_HTONL(ctx->m_nE, p);
    initSHA(ctx, updateSHA_1);

    return rtl_Digest_E_None;
}

void SAL_CALL rtl_digest_destroySHA1(rtlDigest Digest) noexcept
{
    DigestSHA_Impl *pImpl = static_cast< DigestSHA_Impl * >(Digest);
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA1_StarOfficeBug)
            rtl_freeZeroMemory(pImpl, sizeof(DigestSHA_Impl));
        else
            free(pImpl);
    }
}

#define DIGEST_CBLOCK_HMAC_SHA1 64

namespace {

struct ContextHMAC_SHA1
{
    DigestSHA_Impl m_hash;
    sal_uInt8      m_opad[DIGEST_CBLOCK_HMAC_SHA1];
};

struct DigestHMAC_SHA1_Impl
{
    Digest_Impl      m_digest;
    ContextHMAC_SHA1 m_context;
};

}

static void initHMAC_SHA1(ContextHMAC_SHA1 * ctx);
static void ipadHMAC_SHA1(ContextHMAC_SHA1 * ctx);
static void opadHMAC_SHA1(ContextHMAC_SHA1 * ctx);

const Digest_Impl HMAC_SHA1 =
{
    rtl_Digest_Algorithm_FORCE_EQUAL_SIZE, // this is only used internally now so no need to check it
    RTL_DIGEST_LENGTH_SHA1,
    nullptr, // unused now
    nullptr, // unused now
    nullptr, // unused now
    nullptr, // unused now
};

static void initHMAC_SHA1(ContextHMAC_SHA1 * ctx)
{
    DigestSHA_Impl *pImpl = &(ctx->m_hash);

    pImpl->m_digest = SHA_1;
    initSHA(&(pImpl->m_context), updateSHA_1);

    memset(ctx->m_opad, 0, DIGEST_CBLOCK_HMAC_SHA1);
}

static void ipadHMAC_SHA1(ContextHMAC_SHA1 * ctx)
{
    sal_uInt32 i;

    for (i = 0; i < DIGEST_CBLOCK_HMAC_SHA1; i++)
    {
        ctx->m_opad[i] ^= 0x36;
    }

    rtl_digest_updateSHA1(&(ctx->m_hash), ctx->m_opad, DIGEST_CBLOCK_HMAC_SHA1);

    for (i = 0; i < DIGEST_CBLOCK_HMAC_SHA1; i++)
    {
        ctx->m_opad[i] ^= 0x36;
    }
}

static void opadHMAC_SHA1(ContextHMAC_SHA1 * ctx)
{
    sal_uInt32 i;

    for (i = 0; i < DIGEST_CBLOCK_HMAC_SHA1; i++)
    {
        ctx->m_opad[i] ^= 0x5c;
    }
}

namespace {

/** Initialize a HMAC_SHA1 digest.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_init()

    @param[in] Digest   digest handle.
    @param[in] pKeyData key material buffer.
    @param[in] nKeyLen  key material length.

    @retval rtl_Digest_E_None upon success.
 */
rtlDigestError rtl_digest_initHMAC_SHA1(
    rtlDigest Digest, const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen) noexcept
{
    DigestHMAC_SHA1_Impl *pImpl = static_cast<DigestHMAC_SHA1_Impl*>(Digest);
    ContextHMAC_SHA1 *ctx;

    if (!pImpl || !pKeyData)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_Algorithm_FORCE_EQUAL_SIZE)
        return rtl_Digest_E_Algorithm;

    ctx = &(pImpl->m_context);
    initHMAC_SHA1(ctx);

    if (nKeyLen > DIGEST_CBLOCK_HMAC_SHA1)
    {
        /* Initialize 'opad' with hashed 'KeyData' */
        rtl_digest_updateSHA1(&(ctx->m_hash), pKeyData, nKeyLen);
        rtl_digest_getSHA1(&(ctx->m_hash), ctx->m_opad, RTL_DIGEST_LENGTH_SHA1);
    }
    else
    {
        /* Initialize 'opad' with plain 'KeyData' */
        memcpy(ctx->m_opad, pKeyData, nKeyLen);
    }

    ipadHMAC_SHA1(ctx);
    opadHMAC_SHA1(ctx);

    return rtl_Digest_E_None;
}

/** Update a HMAC_SHA1 digest with given data.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_update()
 */
rtlDigestError rtl_digest_updateHMAC_SHA1(
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen) noexcept
{
    DigestHMAC_SHA1_Impl *pImpl = static_cast<DigestHMAC_SHA1_Impl*>(Digest);
    ContextHMAC_SHA1 *ctx;

    if (!pImpl || !pData)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_Algorithm_FORCE_EQUAL_SIZE)
        return rtl_Digest_E_Algorithm;

    ctx = &(pImpl->m_context);
    rtl_digest_updateSHA1(&(ctx->m_hash), pData, nDatLen);

    return rtl_Digest_E_None;
}

/** Finalize a HMAC_SHA1 digest and retrieve the digest value.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_get()
 */
rtlDigestError rtl_digest_getHMAC_SHA1(
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen) noexcept
{
    DigestHMAC_SHA1_Impl *pImpl = static_cast<DigestHMAC_SHA1_Impl*>(Digest);
    ContextHMAC_SHA1 *ctx;

    if (!pImpl || !pBuffer)
        return rtl_Digest_E_Argument;

    if (pImpl->m_digest.m_algorithm != rtl_Digest_Algorithm_FORCE_EQUAL_SIZE)
        return rtl_Digest_E_Algorithm;

    if (pImpl->m_digest.m_length > nBufLen)
        return rtl_Digest_E_BufferSize;

    nBufLen = pImpl->m_digest.m_length;

    ctx = &(pImpl->m_context);
    rtl_digest_getSHA1(&(ctx->m_hash), pBuffer, nBufLen);

    rtl_digest_updateSHA1(&(ctx->m_hash), ctx->m_opad, sizeof(ctx->m_opad));
    rtl_digest_updateSHA1(&(ctx->m_hash), pBuffer, nBufLen);
    rtl_digest_getSHA1(&(ctx->m_hash), pBuffer, nBufLen);

    opadHMAC_SHA1(ctx);
    ipadHMAC_SHA1(ctx);
    opadHMAC_SHA1(ctx);

    return rtl_Digest_E_None;
}

}

#define RTL_DIGEST_LENGTH_HMAC_SHA1 RTL_DIGEST_LENGTH_SHA1

#define DIGEST_CBLOCK_PBKDF2 RTL_DIGEST_LENGTH_HMAC_SHA1

static void updatePBKDF2(
    rtlDigest        hDigest,
    sal_uInt8        T[DIGEST_CBLOCK_PBKDF2],
    const sal_uInt8 *pSaltData, sal_uInt32 nSaltLen,
    sal_uInt32       nCount,    sal_uInt32 nIndex)
{
    /* T_i = F (P, S, c, i) */
    sal_uInt8 U[DIGEST_CBLOCK_PBKDF2];
    sal_uInt32 i, k;

    /* U_(1) = PRF (P, S || INDEX) */
    rtl_digest_updateHMAC_SHA1(hDigest, pSaltData, nSaltLen);
    rtl_digest_updateHMAC_SHA1(hDigest, &nIndex, sizeof(nIndex));
    rtl_digest_getHMAC_SHA1(hDigest, U, DIGEST_CBLOCK_PBKDF2);

    /* T = U_(1) */
    for (k = 0; k < DIGEST_CBLOCK_PBKDF2; k++)
    {
        T[k] = U[k];
    }

    /* T ^= U_(2) ^ ... ^ U_(c) */
    for (i = 1; i < nCount; i++)
    {
        /* U_(i) = PRF (P, U_(i-1)) */
        rtl_digest_updateHMAC_SHA1(hDigest, U, DIGEST_CBLOCK_PBKDF2);
        rtl_digest_getHMAC_SHA1(hDigest, U, DIGEST_CBLOCK_PBKDF2);

        /* T ^= U_(i) */
        for (k = 0; k < DIGEST_CBLOCK_PBKDF2; k++)
        {
            T[k] ^= U[k];
        }
    }

    rtl_secureZeroMemory(U, DIGEST_CBLOCK_PBKDF2);
}

rtlDigestError SAL_CALL rtl_digest_PBKDF2(
    sal_uInt8 *pKeyData , sal_uInt32 nKeyLen,
    const sal_uInt8 *pPassData, sal_uInt32 nPassLen,
    const sal_uInt8 *pSaltData, sal_uInt32 nSaltLen,
    sal_uInt32 nCount) noexcept
{
    DigestHMAC_SHA1_Impl digest;
    sal_uInt32 i = 1;

    if (!pKeyData || !pPassData || !pSaltData)
        return rtl_Digest_E_Argument;

    digest.m_digest = HMAC_SHA1;
    rtl_digest_initHMAC_SHA1(&digest, pPassData, nPassLen);

    /* DK = T_(1) || T_(2) || ... || T_(l) */
    while (nKeyLen >= DIGEST_CBLOCK_PBKDF2)
    {
        /* T_(i) = F (P, S, c, i); DK ||= T_(i) */
        updatePBKDF2(
            &digest, pKeyData,
            pSaltData, nSaltLen,
            nCount, OSL_NETDWORD(i));

        /* Next 'KeyData' block */
        pKeyData += DIGEST_CBLOCK_PBKDF2;
        nKeyLen  -= DIGEST_CBLOCK_PBKDF2;
        i += 1;
    }

    if (nKeyLen > 0)
    {
        /* Last 'KeyData' block */
        sal_uInt8 T[DIGEST_CBLOCK_PBKDF2];

        /* T_i = F (P, S, c, i) */
        updatePBKDF2(
            &digest, T,
            pSaltData, nSaltLen,
            nCount, OSL_NETDWORD(i));

        /* DK ||= T_(i) */
        memcpy(pKeyData, T, nKeyLen);
        rtl_secureZeroMemory(T, DIGEST_CBLOCK_PBKDF2);
    }

    rtl_secureZeroMemory(&digest, sizeof(digest));
    return rtl_Digest_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
