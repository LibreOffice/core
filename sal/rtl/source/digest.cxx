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

#include <string.h>

#include <sal/types.h>
#include <sal/macros.h>
#include <osl/endian.h>
#include <rtl/alloc.h>
#include <rtl/digest.h>

/*========================================================================
 *
 * rtlDigest internals.
 *
 *======================================================================*/
#define RTL_DIGEST_CREATE(T) ((T*)(rtl_allocateZeroMemory(sizeof(T))))

#define RTL_DIGEST_ROTL(a,n) (((a) << (n)) | ((a) >> (32 - (n))))

#define RTL_DIGEST_HTONL(l,c) \
    (*((c)++) = (sal_uInt8)(((l) >> 24L) & 0xff), \
     *((c)++) = (sal_uInt8)(((l) >> 16L) & 0xff), \
     *((c)++) = (sal_uInt8)(((l) >>  8L) & 0xff), \
     *((c)++) = (sal_uInt8)(((l)       ) & 0xff))

#define RTL_DIGEST_LTOC(l,c) \
    (*((c)++) = (sal_uInt8)(((l)       ) & 0xff), \
     *((c)++) = (sal_uInt8)(((l) >>  8L) & 0xff), \
     *((c)++) = (sal_uInt8)(((l) >> 16L) & 0xff), \
     *((c)++) = (sal_uInt8)(((l) >> 24L) & 0xff))

typedef rtlDigestError (SAL_CALL Digest_init_t) (
    void *ctx, const sal_uInt8 *Data, sal_uInt32 DatLen);

typedef void (SAL_CALL Digest_delete_t) (void *ctx);

typedef rtlDigestError (SAL_CALL Digest_update_t) (
    void *ctx, const void *Data, sal_uInt32 DatLen);

typedef rtlDigestError (SAL_CALL Digest_get_t) (
    void *ctx, sal_uInt8 *Buffer, sal_uInt32 BufLen);

struct Digest_Impl
{
    rtlDigestAlgorithm  m_algorithm;
    sal_uInt32          m_length;

    Digest_init_t      *m_init;
    Digest_delete_t    *m_delete;
    Digest_update_t    *m_update;
    Digest_get_t       *m_get;
};

/*
 * __rtl_digest_swapLong.
 */
static void __rtl_digest_swapLong (sal_uInt32 *pData, sal_uInt32 nDatLen)
{
    sal_uInt32 *X;
    int         i, n;

    X = pData;
    n = nDatLen;

    for (i = 0; i < n; i++)
        X[i] = OSL_SWAPDWORD(X[i]);
}

/*========================================================================
 *
 * rtlDigest implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_create.
 */
rtlDigest SAL_CALL rtl_digest_create (rtlDigestAlgorithm Algorithm)
    SAL_THROW_EXTERN_C()
{
    rtlDigest Digest = (rtlDigest)NULL;
    switch (Algorithm)
    {
        case rtl_Digest_AlgorithmMD2:
            Digest = rtl_digest_createMD2();
            break;

        case rtl_Digest_AlgorithmMD5:
            Digest = rtl_digest_createMD5();
            break;

        case rtl_Digest_AlgorithmSHA:
            Digest = rtl_digest_createSHA();
            break;

        case rtl_Digest_AlgorithmSHA1:
            Digest = rtl_digest_createSHA1();
            break;

        case rtl_Digest_AlgorithmHMAC_MD5:
            Digest = rtl_digest_createHMAC_MD5();
            break;

        case rtl_Digest_AlgorithmHMAC_SHA1:
            Digest = rtl_digest_createHMAC_SHA1();
            break;

        default: /* rtl_Digest_AlgorithmInvalid */
            break;
    }
    return Digest;
}

/*
 * rtl_digest_queryAlgorithm.
 */
rtlDigestAlgorithm SAL_CALL rtl_digest_queryAlgorithm (rtlDigest Digest)
    SAL_THROW_EXTERN_C()
{
    Digest_Impl *pImpl = (Digest_Impl *)Digest;
    if (pImpl)
        return pImpl->m_algorithm;
    else
        return rtl_Digest_AlgorithmInvalid;
}

/*
 * rtl_digest_queryLength.
 */
sal_uInt32 SAL_CALL rtl_digest_queryLength (rtlDigest Digest)
    SAL_THROW_EXTERN_C()
{
    Digest_Impl *pImpl = (Digest_Impl *)Digest;
    if (pImpl)
        return pImpl->m_length;
    else
        return 0;
}

/*
 * rtl_digest_init.
 */
rtlDigestError SAL_CALL rtl_digest_init (
    rtlDigest Digest, const sal_uInt8 *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    Digest_Impl *pImpl = (Digest_Impl *)Digest;
    if (pImpl)
    {
        if (pImpl->m_init)
            return pImpl->m_init (Digest, pData, nDatLen);
        else
            return rtl_Digest_E_None;
    }
    return rtl_Digest_E_Argument;
}

/*
 * rtl_digest_update.
 */
rtlDigestError SAL_CALL rtl_digest_update (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    Digest_Impl *pImpl = (Digest_Impl *)Digest;
    if (pImpl && pImpl->m_update)
        return pImpl->m_update (Digest, pData, nDatLen);
    else
        return rtl_Digest_E_Argument;
}

/*
 * rtl_digest_get.
 */
rtlDigestError SAL_CALL rtl_digest_get (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    Digest_Impl *pImpl = (Digest_Impl *)Digest;
    if (pImpl && pImpl->m_get)
        return pImpl->m_get (Digest, pBuffer, nBufLen);
    else
        return rtl_Digest_E_Argument;
}

/*
 * rtl_digest_destroy.
 */
void SAL_CALL rtl_digest_destroy (rtlDigest Digest) SAL_THROW_EXTERN_C()
{
    Digest_Impl *pImpl = (Digest_Impl *)Digest;
    if (pImpl && pImpl->m_delete)
        pImpl->m_delete (Digest);
}

/*========================================================================
 *
 * rtl_digest_MD2 internals.
 *
 *======================================================================*/
#define DIGEST_CBLOCK_MD2 16
#define DIGEST_LBLOCK_MD2 16

struct DigestContextMD2
{
    sal_uInt32 m_nDatLen;
    sal_uInt8  m_pData[DIGEST_CBLOCK_MD2];
    sal_uInt32 m_state[DIGEST_LBLOCK_MD2];
    sal_uInt32 m_chksum[DIGEST_LBLOCK_MD2];
};

struct DigestMD2_Impl
{
    Digest_Impl      m_digest;
    DigestContextMD2 m_context;
};

static void __rtl_digest_initMD2   (DigestContextMD2 *ctx);
static void __rtl_digest_updateMD2 (DigestContextMD2 *ctx);
static void __rtl_digest_endMD2    (DigestContextMD2 *ctx);

static const sal_uInt32 S[256] =
{
    0x29, 0x2E, 0x43, 0xC9, 0xA2, 0xD8, 0x7C, 0x01,
    0x3D, 0x36, 0x54, 0xA1, 0xEC, 0xF0, 0x06, 0x13,
    0x62, 0xA7, 0x05, 0xF3, 0xC0, 0xC7, 0x73, 0x8C,
    0x98, 0x93, 0x2B, 0xD9, 0xBC, 0x4C, 0x82, 0xCA,
    0x1E, 0x9B, 0x57, 0x3C, 0xFD, 0xD4, 0xE0, 0x16,
    0x67, 0x42, 0x6F, 0x18, 0x8A, 0x17, 0xE5, 0x12,
    0xBE, 0x4E, 0xC4, 0xD6, 0xDA, 0x9E, 0xDE, 0x49,
    0xA0, 0xFB, 0xF5, 0x8E, 0xBB, 0x2F, 0xEE, 0x7A,
    0xA9, 0x68, 0x79, 0x91, 0x15, 0xB2, 0x07, 0x3F,
    0x94, 0xC2, 0x10, 0x89, 0x0B, 0x22, 0x5F, 0x21,
    0x80, 0x7F, 0x5D, 0x9A, 0x5A, 0x90, 0x32, 0x27,
    0x35, 0x3E, 0xCC, 0xE7, 0xBF, 0xF7, 0x97, 0x03,
    0xFF, 0x19, 0x30, 0xB3, 0x48, 0xA5, 0xB5, 0xD1,
    0xD7, 0x5E, 0x92, 0x2A, 0xAC, 0x56, 0xAA, 0xC6,
    0x4F, 0xB8, 0x38, 0xD2, 0x96, 0xA4, 0x7D, 0xB6,
    0x76, 0xFC, 0x6B, 0xE2, 0x9C, 0x74, 0x04, 0xF1,
    0x45, 0x9D, 0x70, 0x59, 0x64, 0x71, 0x87, 0x20,
    0x86, 0x5B, 0xCF, 0x65, 0xE6, 0x2D, 0xA8, 0x02,
    0x1B, 0x60, 0x25, 0xAD, 0xAE, 0xB0, 0xB9, 0xF6,
    0x1C, 0x46, 0x61, 0x69, 0x34, 0x40, 0x7E, 0x0F,
    0x55, 0x47, 0xA3, 0x23, 0xDD, 0x51, 0xAF, 0x3A,
    0xC3, 0x5C, 0xF9, 0xCE, 0xBA, 0xC5, 0xEA, 0x26,
    0x2C, 0x53, 0x0D, 0x6E, 0x85, 0x28, 0x84, 0x09,
    0xD3, 0xDF, 0xCD, 0xF4, 0x41, 0x81, 0x4D, 0x52,
    0x6A, 0xDC, 0x37, 0xC8, 0x6C, 0xC1, 0xAB, 0xFA,
    0x24, 0xE1, 0x7B, 0x08, 0x0C, 0xBD, 0xB1, 0x4A,
    0x78, 0x88, 0x95, 0x8B, 0xE3, 0x63, 0xE8, 0x6D,
    0xE9, 0xCB, 0xD5, 0xFE, 0x3B, 0x00, 0x1D, 0x39,
    0xF2, 0xEF, 0xB7, 0x0E, 0x66, 0x58, 0xD0, 0xE4,
    0xA6, 0x77, 0x72, 0xF8, 0xEB, 0x75, 0x4B, 0x0A,
    0x31, 0x44, 0x50, 0xB4, 0x8F, 0xED, 0x1F, 0x1A,
    0xDB, 0x99, 0x8D, 0x33, 0x9F, 0x11, 0x83, 0x14,
};

/*
 * __rtl_digest_MD2.
 */
static const Digest_Impl __rtl_digest_MD2 =
{
    rtl_Digest_AlgorithmMD2,
    RTL_DIGEST_LENGTH_MD2,

    NULL,
    rtl_digest_destroyMD2,
    rtl_digest_updateMD2,
    rtl_digest_getMD2
};

/*
 * __rtl_digest_initMD2.
 */
static void __rtl_digest_initMD2 (DigestContextMD2 *ctx)
{
    memset (ctx, 0, sizeof (DigestContextMD2));
}

/*
 * __rtl_digest_updateMD2.
 */
static void __rtl_digest_updateMD2 (DigestContextMD2 *ctx)
{
    sal_uInt8  *X;
    sal_uInt32 *sp1, *sp2;
    sal_uInt32  i, k, t;

    sal_uInt32 state[48];

    X   = ctx->m_pData;
    sp1 = ctx->m_state;
    sp2 = ctx->m_chksum;

    k = sp2[DIGEST_LBLOCK_MD2 - 1];
    for (i = 0; i < 16; i++)
    {
        state[i +  0] = sp1[i];
        state[i + 16] = t = X[i];
        state[i + 32] = t ^ sp1[i];
        k = sp2[i] ^= S[t^k];
    }

    t = 0;
    for (i = 0; i < 18; i++)
    {
        for (k = 0; k < 48; k += 8)
        {
            t = state[k + 0] ^= S[t];
            t = state[k + 1] ^= S[t];
            t = state[k + 2] ^= S[t];
            t = state[k + 3] ^= S[t];
            t = state[k + 4] ^= S[t];
            t = state[k + 5] ^= S[t];
            t = state[k + 6] ^= S[t];
            t = state[k + 7] ^= S[t];
        }
        t = ((t + i) & 0xff);
    }

    memcpy (sp1, state, 16 * sizeof(sal_uInt32));
    memset (state, 0, 48 * sizeof(sal_uInt32));
}

/*
 * __rtl_digest_endMD2.
 */
static void __rtl_digest_endMD2 (DigestContextMD2 *ctx)
{
    sal_uInt8  *X;
    sal_uInt32 *C;
    sal_uInt32           i, n;

    X = ctx->m_pData;
    C = ctx->m_chksum;
    n = DIGEST_CBLOCK_MD2 - ctx->m_nDatLen;

    for (i = ctx->m_nDatLen; i < DIGEST_CBLOCK_MD2; i++)
        X[i] = (sal_uInt8)(n & 0xff);
    __rtl_digest_updateMD2 (ctx);

    for (i = 0; i < DIGEST_CBLOCK_MD2; i++)
        X[i] = (sal_uInt8)(C[i] & 0xff);
    __rtl_digest_updateMD2 (ctx);
}

/*========================================================================
 *
 * rtl_digest_MD2 implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_MD2.
 */
rtlDigestError SAL_CALL rtl_digest_MD2 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen) SAL_THROW_EXTERN_C()
{
    DigestMD2_Impl digest;
    rtlDigestError result;

    digest.m_digest = __rtl_digest_MD2;
    __rtl_digest_initMD2 (&(digest.m_context));

    result = rtl_digest_updateMD2 (&digest, pData, nDatLen);
    if (result == rtl_Digest_E_None)
        result = rtl_digest_getMD2 (&digest, pBuffer, nBufLen);

    memset (&digest, 0, sizeof (digest));
    return (result);
}

/*
 * rtl_digest_createMD2.
 */
rtlDigest SAL_CALL rtl_digest_createMD2() SAL_THROW_EXTERN_C()
{
    DigestMD2_Impl *pImpl = (DigestMD2_Impl*)NULL;
    pImpl = RTL_DIGEST_CREATE(DigestMD2_Impl);
    if (pImpl)
    {
        pImpl->m_digest = __rtl_digest_MD2;
        __rtl_digest_initMD2 (&(pImpl->m_context));
    }
    return ((rtlDigest)pImpl);
}

/*
 * rtl_digest_updateMD2.
 */
rtlDigestError SAL_CALL rtl_digest_updateMD2 (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    DigestMD2_Impl   *pImpl = (DigestMD2_Impl *)Digest;
    const sal_uInt8  *d     = (const sal_uInt8 *)pData;

    DigestContextMD2 *ctx;

    if ((pImpl == NULL) || (pData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD2))
        return rtl_Digest_E_Algorithm;

    if (nDatLen == 0)
        return rtl_Digest_E_None;

    ctx = &(pImpl->m_context);

    if (ctx->m_nDatLen)
    {
        sal_uInt8  *p = ctx->m_pData + ctx->m_nDatLen;
        sal_uInt32  n = DIGEST_CBLOCK_MD2 - ctx->m_nDatLen;

        if (nDatLen < n)
        {
            memcpy (p, d, nDatLen);
            ctx->m_nDatLen += nDatLen;

            return rtl_Digest_E_None;
        }

        memcpy (p, d, n);
        d       += n;
        nDatLen -= n;

        __rtl_digest_updateMD2 (ctx);
        ctx->m_nDatLen = 0;
    }

    while (nDatLen >= DIGEST_CBLOCK_MD2)
    {
        memcpy (ctx->m_pData, d, DIGEST_CBLOCK_MD2);
        d       += DIGEST_CBLOCK_MD2;
        nDatLen -= DIGEST_CBLOCK_MD2;

        __rtl_digest_updateMD2 (ctx);
    }

    memcpy (ctx->m_pData, d, nDatLen);
    ctx->m_nDatLen = nDatLen;

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_getMD2.
 */
rtlDigestError SAL_CALL rtl_digest_getMD2 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestMD2_Impl   *pImpl = (DigestMD2_Impl *)Digest;
    sal_uInt32        i;

    DigestContextMD2 *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD2))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    __rtl_digest_endMD2 (ctx);
    for (i = 0; i < DIGEST_CBLOCK_MD2; i++)
        pBuffer[i] = (sal_uInt8)(ctx->m_state[i] & 0xff);
    __rtl_digest_initMD2 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_destroyMD2.
 */
void SAL_CALL rtl_digest_destroyMD2 (rtlDigest Digest) SAL_THROW_EXTERN_C()
{
    DigestMD2_Impl *pImpl = (DigestMD2_Impl *)Digest;
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD2)
            rtl_freeZeroMemory (pImpl, sizeof (DigestMD2_Impl));
        else
            rtl_freeMemory (pImpl);
    }
}

/*========================================================================
 *
 * rtl_digest_MD5 internals.
 *
 *======================================================================*/
#define DIGEST_CBLOCK_MD5 64
#define DIGEST_LBLOCK_MD5 16

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

static void __rtl_digest_initMD5   (DigestContextMD5 *ctx);
static void __rtl_digest_updateMD5 (DigestContextMD5 *ctx);
static void __rtl_digest_endMD5    (DigestContextMD5 *ctx);

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

/*
 * __rtl_digest_MD5.
 */
static const Digest_Impl __rtl_digest_MD5 =
{
    rtl_Digest_AlgorithmMD5,
    RTL_DIGEST_LENGTH_MD5,

    NULL,
    rtl_digest_destroyMD5,
    rtl_digest_updateMD5,
    rtl_digest_getMD5
};

/*
 * __rtl_digest_initMD5.
 */
static void __rtl_digest_initMD5 (DigestContextMD5 *ctx)
{
    memset (ctx, 0, sizeof (DigestContextMD5));

    ctx->m_nA = (sal_uInt32)0x67452301L;
    ctx->m_nB = (sal_uInt32)0xefcdab89L;
    ctx->m_nC = (sal_uInt32)0x98badcfeL;
    ctx->m_nD = (sal_uInt32)0x10325476L;
}

/*
 * __rtl_digest_updateMD5.
 */
static void __rtl_digest_updateMD5 (DigestContextMD5 *ctx)
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

/*
 * __rtl_digest_endMD5.
 */
static void __rtl_digest_endMD5 (DigestContextMD5 *ctx)
{
    static const sal_uInt8 end[4] =
    {
        0x80, 0x00, 0x00, 0x00
    };
    const sal_uInt8 *p = end;

    sal_uInt32 *X;
    int         i;

    X = ctx->m_pData;
    i = (ctx->m_nDatLen >> 2);

#ifdef OSL_BIGENDIAN
    __rtl_digest_swapLong (X, i + 1);
#endif /* OSL_BIGENDIAN */

    switch (ctx->m_nDatLen & 0x03)
    {
        case 1: X[i] &= 0x000000ff; break;
        case 2: X[i] &= 0x0000ffff; break;
        case 3: X[i] &= 0x00ffffff; break;
    }

    switch (ctx->m_nDatLen & 0x03)
    {
        case 0: X[i]  = ((sal_uInt32)(*(p++))) <<  0L;
        case 1: X[i] |= ((sal_uInt32)(*(p++))) <<  8L;
        case 2: X[i] |= ((sal_uInt32)(*(p++))) << 16L;
        case 3: X[i] |= ((sal_uInt32)(*p)) << 24L;
    }

    i += 1;

    if (i >= (DIGEST_LBLOCK_MD5 - 2))
    {
        for (; i < DIGEST_LBLOCK_MD5; i++)
            X[i] = 0;
        __rtl_digest_updateMD5 (ctx);
        i = 0;
    }

    for (; i < (DIGEST_LBLOCK_MD5 - 2); i++)
        X[i] = 0;

    X[DIGEST_LBLOCK_MD5 - 2] = ctx->m_nL;
    X[DIGEST_LBLOCK_MD5 - 1] = ctx->m_nH;

    __rtl_digest_updateMD5 (ctx);
}

/*========================================================================
 *
 * rtl_digest_MD5 implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_MD5.
 */
rtlDigestError SAL_CALL rtl_digest_MD5 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen) SAL_THROW_EXTERN_C()
{
    DigestMD5_Impl digest;
    rtlDigestError result;

    digest.m_digest = __rtl_digest_MD5;
    __rtl_digest_initMD5 (&(digest.m_context));

    result = rtl_digest_update (&digest, pData, nDatLen);
    if (result == rtl_Digest_E_None)
        result = rtl_digest_getMD5 (&digest, pBuffer, nBufLen);

    memset (&digest, 0, sizeof (digest));
    return (result);
}

/*
 * rtl_digest_createMD5.
 */
rtlDigest SAL_CALL rtl_digest_createMD5() SAL_THROW_EXTERN_C()
{
    DigestMD5_Impl *pImpl = (DigestMD5_Impl*)NULL;
    pImpl = RTL_DIGEST_CREATE(DigestMD5_Impl);
    if (pImpl)
    {
        pImpl->m_digest = __rtl_digest_MD5;
        __rtl_digest_initMD5 (&(pImpl->m_context));
    }
    return ((rtlDigest)pImpl);
}

/*
 * rtl_digest_updateMD5.
 */
rtlDigestError SAL_CALL rtl_digest_updateMD5 (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    DigestMD5_Impl   *pImpl = (DigestMD5_Impl *)Digest;
    const sal_uInt8  *d     = (const sal_uInt8 *)pData;

    DigestContextMD5 *ctx;
    sal_uInt32        len;

    if ((pImpl == NULL) || (pData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD5))
        return rtl_Digest_E_Algorithm;

    if (nDatLen == 0)
        return rtl_Digest_E_None;

    ctx = &(pImpl->m_context);

    len = ctx->m_nL + (nDatLen << 3);
    if (len < ctx->m_nL) ctx->m_nH += 1;
    ctx->m_nH += (nDatLen >> 29);
    ctx->m_nL  = len;

    if (ctx->m_nDatLen)
    {
        sal_uInt8  *p = (sal_uInt8 *)(ctx->m_pData) + ctx->m_nDatLen;
        sal_uInt32  n = DIGEST_CBLOCK_MD5 - ctx->m_nDatLen;

        if (nDatLen < n)
        {
            memcpy (p, d, nDatLen);
            ctx->m_nDatLen += nDatLen;

            return rtl_Digest_E_None;
        }

        memcpy (p, d, n);
        d       += n;
        nDatLen -= n;

#ifdef OSL_BIGENDIAN
        __rtl_digest_swapLong (ctx->m_pData, DIGEST_LBLOCK_MD5);
#endif /* OSL_BIGENDIAN */

        __rtl_digest_updateMD5 (ctx);
        ctx->m_nDatLen = 0;
    }

    while (nDatLen >= DIGEST_CBLOCK_MD5)
    {
        memcpy (ctx->m_pData, d, DIGEST_CBLOCK_MD5);
        d       += DIGEST_CBLOCK_MD5;
        nDatLen -= DIGEST_CBLOCK_MD5;

#ifdef OSL_BIGENDIAN
        __rtl_digest_swapLong (ctx->m_pData, DIGEST_LBLOCK_MD5);
#endif /* OSL_BIGENDIAN */

        __rtl_digest_updateMD5 (ctx);
    }

    memcpy (ctx->m_pData, d, nDatLen);
    ctx->m_nDatLen = nDatLen;

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_getMD5.
 */
rtlDigestError SAL_CALL rtl_digest_getMD5 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestMD5_Impl   *pImpl = (DigestMD5_Impl *)Digest;
    sal_uInt8        *p     = pBuffer;

    DigestContextMD5 *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD5))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    __rtl_digest_endMD5 (ctx);
    RTL_DIGEST_LTOC (ctx->m_nA, p);
    RTL_DIGEST_LTOC (ctx->m_nB, p);
    RTL_DIGEST_LTOC (ctx->m_nC, p);
    RTL_DIGEST_LTOC (ctx->m_nD, p);
    __rtl_digest_initMD5 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_rawMD5.
 */
rtlDigestError SAL_CALL rtl_digest_rawMD5 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestMD5_Impl   *pImpl = (DigestMD5_Impl *)Digest;
    sal_uInt8        *p     = pBuffer;

    DigestContextMD5 *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD5))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    /* __rtl_digest_endMD5 (ctx); *//* not finalized */
    RTL_DIGEST_LTOC (ctx->m_nA, p);
    RTL_DIGEST_LTOC (ctx->m_nB, p);
    RTL_DIGEST_LTOC (ctx->m_nC, p);
    RTL_DIGEST_LTOC (ctx->m_nD, p);
    __rtl_digest_initMD5 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_destroyMD5.
 */
void SAL_CALL rtl_digest_destroyMD5 (rtlDigest Digest) SAL_THROW_EXTERN_C()
{
    DigestMD5_Impl *pImpl = (DigestMD5_Impl *)Digest;
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmMD5)
            rtl_freeZeroMemory (pImpl, sizeof (DigestMD5_Impl));
        else
            rtl_freeMemory (pImpl);
    }
}

/*========================================================================
 *
 * rtl_digest_(SHA|SHA1) common internals.
 *
 *======================================================================*/
#define DIGEST_CBLOCK_SHA 64
#define DIGEST_LBLOCK_SHA 16

typedef sal_uInt32 DigestSHA_update_t (sal_uInt32 x);

static sal_uInt32 __rtl_digest_updateSHA_0 (sal_uInt32 x);
static sal_uInt32 __rtl_digest_updateSHA_1 (sal_uInt32 x);

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

static void __rtl_digest_initSHA (
    DigestContextSHA *ctx, DigestSHA_update_t *fct);

static void __rtl_digest_updateSHA (DigestContextSHA *ctx);
static void __rtl_digest_endSHA    (DigestContextSHA *ctx);

#define K_00_19 (sal_uInt32)0x5a827999L
#define K_20_39 (sal_uInt32)0x6ed9eba1L
#define K_40_59 (sal_uInt32)0x8f1bbcdcL
#define K_60_79 (sal_uInt32)0xca62c1d6L

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

/*
 * __rtl_digest_initSHA.
 */
static void __rtl_digest_initSHA (
    DigestContextSHA *ctx, DigestSHA_update_t *fct)
{
    memset (ctx, 0, sizeof (DigestContextSHA));
    ctx->m_update = fct;

    ctx->m_nA = (sal_uInt32)0x67452301L;
    ctx->m_nB = (sal_uInt32)0xefcdab89L;
    ctx->m_nC = (sal_uInt32)0x98badcfeL;
    ctx->m_nD = (sal_uInt32)0x10325476L;
    ctx->m_nE = (sal_uInt32)0xc3d2e1f0L;
}

/*
 * __rtl_digest_updateSHA.
 */
static void __rtl_digest_updateSHA (DigestContextSHA *ctx)
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

/*
 * __rtl_digest_endSHA.
 */
static void __rtl_digest_endSHA (DigestContextSHA *ctx)
{
    static const sal_uInt8 end[4] =
    {
        0x80, 0x00, 0x00, 0x00
    };
    const sal_uInt8 *p = end;

    sal_uInt32 *X;
    int         i;

    X = ctx->m_pData;
    i = (ctx->m_nDatLen >> 2);

#ifdef OSL_BIGENDIAN
    __rtl_digest_swapLong (X, i + 1);
#endif /* OSL_BIGENDIAN */

    switch (ctx->m_nDatLen & 0x03)
    {
        case 1: X[i] &= 0x000000ff; break;
        case 2: X[i] &= 0x0000ffff; break;
        case 3: X[i] &= 0x00ffffff; break;
    }

    switch (ctx->m_nDatLen & 0x03)
    {
        case 0: X[i]  = ((sal_uInt32)(*(p++))) <<  0L;
        case 1: X[i] |= ((sal_uInt32)(*(p++))) <<  8L;
        case 2: X[i] |= ((sal_uInt32)(*(p++))) << 16L;
        case 3: X[i] |= ((sal_uInt32)(*(p++))) << 24L;
    }

    __rtl_digest_swapLong (X, i + 1);

    i += 1;

    if (i >= (DIGEST_LBLOCK_SHA - 2))
    {
        for (; i < DIGEST_LBLOCK_SHA; i++)
            X[i] = 0;
        __rtl_digest_updateSHA (ctx);
        i = 0;
    }

    for (; i < (DIGEST_LBLOCK_SHA - 2); i++)
        X[i] = 0;

    X[DIGEST_LBLOCK_SHA - 2] = ctx->m_nH;
    X[DIGEST_LBLOCK_SHA - 1] = ctx->m_nL;

    __rtl_digest_updateSHA (ctx);
}

/*========================================================================
 *
 * rtl_digest_SHA internals.
 *
 *======================================================================*/
/*
 * __rtl_digest_SHA_0.
 */
static const Digest_Impl __rtl_digest_SHA_0 =
{
    rtl_Digest_AlgorithmSHA,
    RTL_DIGEST_LENGTH_SHA,

    NULL,
    rtl_digest_destroySHA,
    rtl_digest_updateSHA,
    rtl_digest_getSHA
};

/*
 * __rtl_digest_updateSHA_0.
 */
static sal_uInt32 __rtl_digest_updateSHA_0 (sal_uInt32 x)
{
    return x;
}

/*========================================================================
 *
 * rtl_digest_SHA implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_SHA.
 */
rtlDigestError SAL_CALL rtl_digest_SHA (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen) SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl digest;
    rtlDigestError result;

    digest.m_digest = __rtl_digest_SHA_0;
    __rtl_digest_initSHA (&(digest.m_context), __rtl_digest_updateSHA_0);

    result = rtl_digest_updateSHA (&digest, pData, nDatLen);
    if (result == rtl_Digest_E_None)
        result = rtl_digest_getSHA (&digest, pBuffer, nBufLen);

    memset (&digest, 0, sizeof (digest));
    return (result);
}

/*
 * rtl_digest_createSHA.
 */
rtlDigest SAL_CALL rtl_digest_createSHA() SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl *pImpl = (DigestSHA_Impl*)NULL;
    pImpl = RTL_DIGEST_CREATE(DigestSHA_Impl);
    if (pImpl)
    {
        pImpl->m_digest = __rtl_digest_SHA_0;
        __rtl_digest_initSHA (&(pImpl->m_context), __rtl_digest_updateSHA_0);
    }
    return ((rtlDigest)pImpl);
}

/*
 * rtl_digest_updateSHA.
 */
rtlDigestError SAL_CALL rtl_digest_updateSHA (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl   *pImpl = (DigestSHA_Impl *)Digest;
    const sal_uInt8  *d     = (const sal_uInt8 *)pData;

    DigestContextSHA *ctx;
    sal_uInt32        len;

    if ((pImpl == NULL) || (pData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA))
        return rtl_Digest_E_Algorithm;

    if (nDatLen == 0)
        return rtl_Digest_E_None;

    ctx = &(pImpl->m_context);

    len = ctx->m_nL + (nDatLen << 3);
    if (len < ctx->m_nL) ctx->m_nH += 1;
    ctx->m_nH += (nDatLen >> 29);
    ctx->m_nL  = len;

    if (ctx->m_nDatLen)
    {
        sal_uInt8  *p = (sal_uInt8 *)(ctx->m_pData) + ctx->m_nDatLen;
        sal_uInt32  n = DIGEST_CBLOCK_SHA - ctx->m_nDatLen;

        if (nDatLen < n)
        {
            memcpy (p, d, nDatLen);
            ctx->m_nDatLen += nDatLen;

            return rtl_Digest_E_None;
        }

        memcpy (p, d, n);
        d       += n;
        nDatLen -= n;

#ifndef OSL_BIGENDIAN
        __rtl_digest_swapLong (ctx->m_pData, DIGEST_LBLOCK_SHA);
#endif /* OSL_BIGENDIAN */

        __rtl_digest_updateSHA (ctx);
        ctx->m_nDatLen = 0;
    }

    while (nDatLen >= DIGEST_CBLOCK_SHA)
    {
        memcpy (ctx->m_pData, d, DIGEST_CBLOCK_SHA);
        d       += DIGEST_CBLOCK_SHA;
        nDatLen -= DIGEST_CBLOCK_SHA;

#ifndef OSL_BIGENDIAN
        __rtl_digest_swapLong (ctx->m_pData, DIGEST_LBLOCK_SHA);
#endif /* OSL_BIGENDIAN */

        __rtl_digest_updateSHA (ctx);
    }

    memcpy (ctx->m_pData, d, nDatLen);
    ctx->m_nDatLen = nDatLen;

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_getSHA.
 */
rtlDigestError SAL_CALL rtl_digest_getSHA (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl   *pImpl = (DigestSHA_Impl *)Digest;
    sal_uInt8        *p     = pBuffer;

    DigestContextSHA *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    __rtl_digest_endSHA (ctx);
    RTL_DIGEST_HTONL (ctx->m_nA, p);
    RTL_DIGEST_HTONL (ctx->m_nB, p);
    RTL_DIGEST_HTONL (ctx->m_nC, p);
    RTL_DIGEST_HTONL (ctx->m_nD, p);
    RTL_DIGEST_HTONL (ctx->m_nE, p);
    __rtl_digest_initSHA (ctx, __rtl_digest_updateSHA_0);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_destroySHA.
 */
void SAL_CALL rtl_digest_destroySHA (rtlDigest Digest) SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl *pImpl = (DigestSHA_Impl *)Digest;
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA)
            rtl_freeZeroMemory (pImpl, sizeof (DigestSHA_Impl));
        else
            rtl_freeMemory (pImpl);
    }
}

/*========================================================================
 *
 * rtl_digest_SHA1 internals.
 *
 *======================================================================*/
/*
 * __rtl_digest_SHA_1.
 */
static const Digest_Impl __rtl_digest_SHA_1 =
{
    rtl_Digest_AlgorithmSHA1,
    RTL_DIGEST_LENGTH_SHA1,

    NULL,
    rtl_digest_destroySHA1,
    rtl_digest_updateSHA1,
    rtl_digest_getSHA1
};

/*
 * __rtl_digest_updateSHA_1.
 */
static sal_uInt32 __rtl_digest_updateSHA_1 (sal_uInt32 x)
{
    return RTL_DIGEST_ROTL (x, 1);
}

/*========================================================================
 *
 * rtl_digest_SHA1 implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_SHA1.
 */
rtlDigestError SAL_CALL rtl_digest_SHA1 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen) SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl digest;
    rtlDigestError result;

    digest.m_digest = __rtl_digest_SHA_1;
    __rtl_digest_initSHA (&(digest.m_context), __rtl_digest_updateSHA_1);

    result = rtl_digest_updateSHA1 (&digest, pData, nDatLen);
    if (result == rtl_Digest_E_None)
        result = rtl_digest_getSHA1 (&digest, pBuffer, nBufLen);

    memset (&digest, 0, sizeof (digest));
    return (result);
}

/*
 * rtl_digest_createSHA1.
 */
rtlDigest SAL_CALL rtl_digest_createSHA1() SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl *pImpl = (DigestSHA_Impl*)NULL;
    pImpl = RTL_DIGEST_CREATE(DigestSHA_Impl);
    if (pImpl)
    {
        pImpl->m_digest = __rtl_digest_SHA_1;
        __rtl_digest_initSHA (&(pImpl->m_context), __rtl_digest_updateSHA_1);
    }
    return ((rtlDigest)pImpl);
}

/*
 * rtl_digest_updateSHA1.
 */
rtlDigestError SAL_CALL rtl_digest_updateSHA1 (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl   *pImpl = (DigestSHA_Impl *)Digest;
    const sal_uInt8  *d     = (const sal_uInt8 *)pData;

    DigestContextSHA *ctx;
    sal_uInt32        len;

    if ((pImpl == NULL) || (pData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA1))
        return rtl_Digest_E_Algorithm;

    if (nDatLen == 0)
        return rtl_Digest_E_None;

    ctx = &(pImpl->m_context);

    len = ctx->m_nL + (nDatLen << 3);
    if (len < ctx->m_nL) ctx->m_nH += 1;
    ctx->m_nH += (nDatLen >> 29);
    ctx->m_nL  = len;

    if (ctx->m_nDatLen)
    {
        sal_uInt8  *p = (sal_uInt8 *)(ctx->m_pData) + ctx->m_nDatLen;
        sal_uInt32  n = DIGEST_CBLOCK_SHA - ctx->m_nDatLen;

        if (nDatLen < n)
        {
            memcpy (p, d, nDatLen);
            ctx->m_nDatLen += nDatLen;

            return rtl_Digest_E_None;
        }

        memcpy (p, d, n);
        d       += n;
        nDatLen -= n;

#ifndef OSL_BIGENDIAN
        __rtl_digest_swapLong (ctx->m_pData, DIGEST_LBLOCK_SHA);
#endif /* OSL_BIGENDIAN */

        __rtl_digest_updateSHA (ctx);
        ctx->m_nDatLen = 0;
    }

    while (nDatLen >= DIGEST_CBLOCK_SHA)
    {
        memcpy (ctx->m_pData, d, DIGEST_CBLOCK_SHA);
        d       += DIGEST_CBLOCK_SHA;
        nDatLen -= DIGEST_CBLOCK_SHA;

#ifndef OSL_BIGENDIAN
        __rtl_digest_swapLong (ctx->m_pData, DIGEST_LBLOCK_SHA);
#endif /* OSL_BIGENDIAN */

        __rtl_digest_updateSHA (ctx);
    }

    memcpy (ctx->m_pData, d, nDatLen);
    ctx->m_nDatLen = nDatLen;

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_getSHA1.
 */
rtlDigestError SAL_CALL rtl_digest_getSHA1 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl   *pImpl = (DigestSHA_Impl *)Digest;
    sal_uInt8        *p     = pBuffer;

    DigestContextSHA *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA1))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    ctx = &(pImpl->m_context);

    __rtl_digest_endSHA (ctx);
    RTL_DIGEST_HTONL (ctx->m_nA, p);
    RTL_DIGEST_HTONL (ctx->m_nB, p);
    RTL_DIGEST_HTONL (ctx->m_nC, p);
    RTL_DIGEST_HTONL (ctx->m_nD, p);
    RTL_DIGEST_HTONL (ctx->m_nE, p);
    __rtl_digest_initSHA (ctx, __rtl_digest_updateSHA_1);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_destroySHA1.
 */
void SAL_CALL rtl_digest_destroySHA1 (rtlDigest Digest) SAL_THROW_EXTERN_C()
{
    DigestSHA_Impl *pImpl = (DigestSHA_Impl *)Digest;
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmSHA1)
            rtl_freeZeroMemory (pImpl, sizeof (DigestSHA_Impl));
        else
            rtl_freeMemory (pImpl);
    }
}

/*========================================================================
 *
 * rtl_digest_HMAC_MD5 internals.
 *
 *======================================================================*/
#define DIGEST_CBLOCK_HMAC_MD5 64

struct ContextHMAC_MD5
{
    DigestMD5_Impl m_hash;
    sal_uInt8      m_opad[DIGEST_CBLOCK_HMAC_MD5];
};

struct DigestHMAC_MD5_Impl
{
    Digest_Impl     m_digest;
    ContextHMAC_MD5 m_context;
};

static void __rtl_digest_initHMAC_MD5 (ContextHMAC_MD5 * ctx);
static void __rtl_digest_ipadHMAC_MD5 (ContextHMAC_MD5 * ctx);
static void __rtl_digest_opadHMAC_MD5 (ContextHMAC_MD5 * ctx);

/*
 * __rtl_digest_HMAC_MD5.
 */
static const Digest_Impl __rtl_digest_HMAC_MD5 =
{
    rtl_Digest_AlgorithmHMAC_MD5,
    RTL_DIGEST_LENGTH_MD5,

    rtl_digest_initHMAC_MD5,
    rtl_digest_destroyHMAC_MD5,
    rtl_digest_updateHMAC_MD5,
    rtl_digest_getHMAC_MD5
};

/*
 * __rtl_digest_initHMAC_MD5.
 */
static void __rtl_digest_initHMAC_MD5 (ContextHMAC_MD5 * ctx)
{
    DigestMD5_Impl *pImpl = &(ctx->m_hash);

    pImpl->m_digest = __rtl_digest_MD5;
    __rtl_digest_initMD5 (&(pImpl->m_context));

    memset (ctx->m_opad, 0, DIGEST_CBLOCK_HMAC_MD5);
}

/*
 * __rtl_digest_ipadHMAC_MD5.
 */
static void __rtl_digest_ipadHMAC_MD5 (ContextHMAC_MD5 * ctx)
{
    sal_uInt32 i;

    for (i = 0; i < DIGEST_CBLOCK_HMAC_MD5; i++)
        ctx->m_opad[i] ^= 0x36;
    rtl_digest_updateMD5 (
        &(ctx->m_hash), ctx->m_opad, DIGEST_CBLOCK_HMAC_MD5);
    for (i = 0; i < DIGEST_CBLOCK_HMAC_MD5; i++)
        ctx->m_opad[i] ^= 0x36;
}

/*
 * __rtl_digest_opadHMAC_MD5.
 */
static void __rtl_digest_opadHMAC_MD5 (ContextHMAC_MD5 * ctx)
{
    sal_uInt32 i;

    for (i = 0; i < DIGEST_CBLOCK_HMAC_MD5; i++)
        ctx->m_opad[i] ^= 0x5c;
}

/*========================================================================
 *
 * rtl_digest_HMAC_MD5 implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_HMAC_MD5.
 */
rtlDigestError SAL_CALL rtl_digest_HMAC_MD5 (
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen,
    const void      *pData,    sal_uInt32 nDatLen,
    sal_uInt8       *pBuffer,  sal_uInt32 nBufLen) SAL_THROW_EXTERN_C()
{
    DigestHMAC_MD5_Impl digest;
    rtlDigestError      result;

    digest.m_digest = __rtl_digest_HMAC_MD5;

    result = rtl_digest_initHMAC_MD5 (&digest, pKeyData, nKeyLen);
    if (result == rtl_Digest_E_None)
    {
        result = rtl_digest_updateHMAC_MD5 (&digest, pData, nDatLen);
        if (result == rtl_Digest_E_None)
            result = rtl_digest_getHMAC_MD5 (&digest, pBuffer, nBufLen);
    }

    memset (&digest, 0, sizeof (digest));
    return (result);
}

/*
 * rtl_digest_createHMAC_MD5.
 */
rtlDigest SAL_CALL rtl_digest_createHMAC_MD5() SAL_THROW_EXTERN_C()
{
    DigestHMAC_MD5_Impl *pImpl = (DigestHMAC_MD5_Impl*)NULL;
    pImpl = RTL_DIGEST_CREATE(DigestHMAC_MD5_Impl);
    if (pImpl)
    {
        pImpl->m_digest = __rtl_digest_HMAC_MD5;
        __rtl_digest_initHMAC_MD5 (&(pImpl->m_context));
    }
    return ((rtlDigest)pImpl);
}

/*
 * rtl_digest_initHMAC_MD5.
 */
rtlDigestError SAL_CALL rtl_digest_initHMAC_MD5 (
    rtlDigest Digest, const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_MD5_Impl *pImpl = (DigestHMAC_MD5_Impl*)Digest;
    ContextHMAC_MD5     *ctx;

    if ((pImpl == NULL) || (pKeyData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_MD5))
        return rtl_Digest_E_Algorithm;

    ctx = &(pImpl->m_context);
    __rtl_digest_initHMAC_MD5 (ctx);

    if (nKeyLen > DIGEST_CBLOCK_HMAC_MD5)
    {
        /* Initialize 'opad' with hashed 'KeyData' */
        rtl_digest_updateMD5 (
            &(ctx->m_hash), pKeyData, nKeyLen);
        rtl_digest_getMD5 (
            &(ctx->m_hash), ctx->m_opad, RTL_DIGEST_LENGTH_MD5);
    }
    else
    {
        /* Initialize 'opad' with plain 'KeyData' */
        memcpy (ctx->m_opad, pKeyData, nKeyLen);
    }

    __rtl_digest_ipadHMAC_MD5 (ctx);
    __rtl_digest_opadHMAC_MD5 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_updateHMAC_MD5.
 */
rtlDigestError SAL_CALL rtl_digest_updateHMAC_MD5 (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_MD5_Impl *pImpl = (DigestHMAC_MD5_Impl*)Digest;
    ContextHMAC_MD5     *ctx;

    if ((pImpl == NULL) || (pData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_MD5))
        return rtl_Digest_E_Algorithm;

    ctx = &(pImpl->m_context);
    rtl_digest_updateMD5 (&(ctx->m_hash), pData, nDatLen);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_getHMAC_MD5.
 */
rtlDigestError SAL_CALL rtl_digest_getHMAC_MD5 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_MD5_Impl *pImpl = (DigestHMAC_MD5_Impl*)Digest;
    ContextHMAC_MD5     *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_MD5))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    nBufLen = pImpl->m_digest.m_length;

    ctx = &(pImpl->m_context);
    rtl_digest_getMD5 (&(ctx->m_hash), pBuffer, nBufLen);

    rtl_digest_updateMD5 (&(ctx->m_hash), ctx->m_opad, 64);
    rtl_digest_updateMD5 (&(ctx->m_hash), pBuffer, nBufLen);
    rtl_digest_getMD5    (&(ctx->m_hash), pBuffer, nBufLen);

    __rtl_digest_opadHMAC_MD5 (ctx);
    __rtl_digest_ipadHMAC_MD5 (ctx);
    __rtl_digest_opadHMAC_MD5 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_destroyHMAC_MD5.
 */
void SAL_CALL rtl_digest_destroyHMAC_MD5 (rtlDigest Digest) SAL_THROW_EXTERN_C()
{
    DigestHMAC_MD5_Impl *pImpl = (DigestHMAC_MD5_Impl*)Digest;
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_MD5)
            rtl_freeZeroMemory (pImpl, sizeof (DigestHMAC_MD5_Impl));
        else
            rtl_freeMemory (pImpl);
    }
}

/*========================================================================
 *
 * rtl_digest_HMAC_SHA1 internals.
 *
 *======================================================================*/
#define DIGEST_CBLOCK_HMAC_SHA1 64

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

static void __rtl_digest_initHMAC_SHA1 (ContextHMAC_SHA1 * ctx);
static void __rtl_digest_ipadHMAC_SHA1 (ContextHMAC_SHA1 * ctx);
static void __rtl_digest_opadHMAC_SHA1 (ContextHMAC_SHA1 * ctx);

/*
 * __rtl_digest_HMAC_SHA1.
 */
static const Digest_Impl __rtl_digest_HMAC_SHA1 =
{
    rtl_Digest_AlgorithmHMAC_SHA1,
    RTL_DIGEST_LENGTH_SHA1,

    rtl_digest_initHMAC_SHA1,
    rtl_digest_destroyHMAC_SHA1,
    rtl_digest_updateHMAC_SHA1,
    rtl_digest_getHMAC_SHA1
};

/*
 * __rtl_digest_initHMAC_SHA1.
 */
static void __rtl_digest_initHMAC_SHA1 (ContextHMAC_SHA1 * ctx)
{
    DigestSHA_Impl *pImpl = &(ctx->m_hash);

    pImpl->m_digest = __rtl_digest_SHA_1;
    __rtl_digest_initSHA (&(pImpl->m_context), __rtl_digest_updateSHA_1);

    memset (ctx->m_opad, 0, DIGEST_CBLOCK_HMAC_SHA1);
}

/*
 * __rtl_digest_ipadHMAC_SHA1.
 */
static void __rtl_digest_ipadHMAC_SHA1 (ContextHMAC_SHA1 * ctx)
{
    sal_uInt32 i;

    for (i = 0; i < DIGEST_CBLOCK_HMAC_SHA1; i++)
        ctx->m_opad[i] ^= 0x36;
    rtl_digest_updateSHA1 (
        &(ctx->m_hash), ctx->m_opad, DIGEST_CBLOCK_HMAC_SHA1);
    for (i = 0; i < DIGEST_CBLOCK_HMAC_SHA1; i++)
        ctx->m_opad[i] ^= 0x36;
}

/*
 * __rtl_digest_opadHMAC_SHA1.
 */
static void __rtl_digest_opadHMAC_SHA1 (ContextHMAC_SHA1 * ctx)
{
    sal_uInt32 i;

    for (i = 0; i < DIGEST_CBLOCK_HMAC_SHA1; i++)
        ctx->m_opad[i] ^= 0x5c;
}

/*========================================================================
 *
 * rtl_digest_HMAC_SHA1 implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_HMAC_SHA1.
 */
rtlDigestError SAL_CALL rtl_digest_HMAC_SHA1 (
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen,
    const void      *pData,    sal_uInt32 nDatLen,
    sal_uInt8       *pBuffer,  sal_uInt32 nBufLen) SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl digest;
    rtlDigestError       result;

    digest.m_digest = __rtl_digest_HMAC_SHA1;

    result = rtl_digest_initHMAC_SHA1 (&digest, pKeyData, nKeyLen);
    if (result == rtl_Digest_E_None)
    {
        result = rtl_digest_updateHMAC_SHA1 (&digest, pData, nDatLen);
        if (result == rtl_Digest_E_None)
            result = rtl_digest_getHMAC_SHA1 (&digest, pBuffer, nBufLen);
    }

    memset (&digest, 0, sizeof (digest));
    return (result);
}

/*
 * rtl_digest_createHMAC_SHA1.
 */
rtlDigest SAL_CALL rtl_digest_createHMAC_SHA1() SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl *pImpl = (DigestHMAC_SHA1_Impl*)NULL;
    pImpl = RTL_DIGEST_CREATE(DigestHMAC_SHA1_Impl);
    if (pImpl)
    {
        pImpl->m_digest = __rtl_digest_HMAC_SHA1;
        __rtl_digest_initHMAC_SHA1 (&(pImpl->m_context));
    }
    return ((rtlDigest)pImpl);
}

/*
 * rtl_digest_initHMAC_SHA1.
 */
rtlDigestError SAL_CALL rtl_digest_initHMAC_SHA1 (
    rtlDigest Digest, const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl *pImpl = (DigestHMAC_SHA1_Impl*)Digest;
    ContextHMAC_SHA1     *ctx;

    if ((pImpl == NULL) || (pKeyData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_SHA1))
        return rtl_Digest_E_Algorithm;

    ctx = &(pImpl->m_context);
    __rtl_digest_initHMAC_SHA1 (ctx);

    if (nKeyLen > DIGEST_CBLOCK_HMAC_SHA1)
    {
        /* Initialize 'opad' with hashed 'KeyData' */
        rtl_digest_updateSHA1 (
            &(ctx->m_hash), pKeyData, nKeyLen);
        rtl_digest_getSHA1 (
            &(ctx->m_hash), ctx->m_opad, RTL_DIGEST_LENGTH_SHA1);
    }
    else
    {
        /* Initialize 'opad' with plain 'KeyData' */
        memcpy (ctx->m_opad, pKeyData, nKeyLen);
    }

    __rtl_digest_ipadHMAC_SHA1 (ctx);
    __rtl_digest_opadHMAC_SHA1 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_updateHMAC_SHA1.
 */
rtlDigestError SAL_CALL rtl_digest_updateHMAC_SHA1 (
    rtlDigest Digest, const void *pData, sal_uInt32 nDatLen)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl *pImpl = (DigestHMAC_SHA1_Impl*)Digest;
    ContextHMAC_SHA1     *ctx;

    if ((pImpl == NULL) || (pData == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_SHA1))
        return rtl_Digest_E_Algorithm;

    ctx = &(pImpl->m_context);
    rtl_digest_updateSHA1 (&(ctx->m_hash), pData, nDatLen);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_getHMAC_SHA1.
 */
rtlDigestError SAL_CALL rtl_digest_getHMAC_SHA1 (
    rtlDigest Digest, sal_uInt8 *pBuffer, sal_uInt32 nBufLen)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl *pImpl = (DigestHMAC_SHA1_Impl*)Digest;
    ContextHMAC_SHA1     *ctx;

    if ((pImpl == NULL) || (pBuffer == NULL))
        return rtl_Digest_E_Argument;

    if (!(pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_SHA1))
        return rtl_Digest_E_Algorithm;

    if (!(pImpl->m_digest.m_length <= nBufLen))
        return rtl_Digest_E_BufferSize;

    nBufLen = pImpl->m_digest.m_length;

    ctx = &(pImpl->m_context);
    rtl_digest_getSHA1 (&(ctx->m_hash), pBuffer, nBufLen);

    rtl_digest_updateSHA1 (&(ctx->m_hash), ctx->m_opad, sizeof(ctx->m_opad));
    rtl_digest_updateSHA1 (&(ctx->m_hash), pBuffer, nBufLen);
    rtl_digest_getSHA1    (&(ctx->m_hash), pBuffer, nBufLen);

    __rtl_digest_opadHMAC_SHA1 (ctx);
    __rtl_digest_ipadHMAC_SHA1 (ctx);
    __rtl_digest_opadHMAC_SHA1 (ctx);

    return rtl_Digest_E_None;
}

/*
 * rtl_digest_destroyHMAC_SHA1.
 */
void SAL_CALL rtl_digest_destroyHMAC_SHA1 (rtlDigest Digest)
    SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl *pImpl = (DigestHMAC_SHA1_Impl*)Digest;
    if (pImpl)
    {
        if (pImpl->m_digest.m_algorithm == rtl_Digest_AlgorithmHMAC_SHA1)
            rtl_freeZeroMemory (pImpl, sizeof (DigestHMAC_SHA1_Impl));
        else
            rtl_freeMemory (pImpl);
    }
}

/*========================================================================
 *
 * rtl_digest_PBKDF2 internals.
 *
 *======================================================================*/
#define DIGEST_CBLOCK_PBKDF2 RTL_DIGEST_LENGTH_HMAC_SHA1

/*
 * __rtl_digest_updatePBKDF2.
 */
static void __rtl_digest_updatePBKDF2 (
    rtlDigest        hDigest,
    sal_uInt8        T[DIGEST_CBLOCK_PBKDF2],
    const sal_uInt8 *pSaltData, sal_uInt32 nSaltLen,
    sal_uInt32       nCount,    sal_uInt32 nIndex)
{
    /* T_i = F (P, S, c, i) */
    sal_uInt8 U[DIGEST_CBLOCK_PBKDF2];
    sal_uInt32 i, k;

    /* U_(1) = PRF (P, S || INDEX) */
    rtl_digest_updateHMAC_SHA1 (hDigest, pSaltData, nSaltLen);
    rtl_digest_updateHMAC_SHA1 (hDigest, &nIndex, sizeof(nIndex));
    rtl_digest_getHMAC_SHA1    (hDigest, U, DIGEST_CBLOCK_PBKDF2);

    /* T = U_(1) */
    for (k = 0; k < DIGEST_CBLOCK_PBKDF2; k++) T[k] = U[k];

    /* T ^= U_(2) ^ ... ^ U_(c) */
    for (i = 1; i < nCount; i++)
    {
        /* U_(i) = PRF (P, U_(i-1)) */
        rtl_digest_updateHMAC_SHA1 (hDigest, U, DIGEST_CBLOCK_PBKDF2);
        rtl_digest_getHMAC_SHA1    (hDigest, U, DIGEST_CBLOCK_PBKDF2);

        /* T ^= U_(i) */
        for (k = 0; k < DIGEST_CBLOCK_PBKDF2; k++) T[k] ^= U[k];
    }

    memset (U, 0, DIGEST_CBLOCK_PBKDF2);
}

/*========================================================================
 *
 * rtl_digest_PBKDF2 implementation.
 *
 *======================================================================*/
/*
 * rtl_digest_PBKDF2.
 */
rtlDigestError SAL_CALL rtl_digest_PBKDF2 (
    sal_uInt8       *pKeyData , sal_uInt32 nKeyLen,
    const sal_uInt8 *pPassData, sal_uInt32 nPassLen,
    const sal_uInt8 *pSaltData, sal_uInt32 nSaltLen,
    sal_uInt32       nCount) SAL_THROW_EXTERN_C()
{
    DigestHMAC_SHA1_Impl digest;
    sal_uInt32           i = 1;

    if ((pKeyData == NULL) || (pPassData == NULL) || (pSaltData == NULL))
        return rtl_Digest_E_Argument;

    digest.m_digest = __rtl_digest_HMAC_SHA1;
    rtl_digest_initHMAC_SHA1 (&digest, pPassData, nPassLen);

    /* DK = T_(1) || T_(2) || ... || T_(l) */
    while (nKeyLen >= DIGEST_CBLOCK_PBKDF2)
    {
        /* T_(i) = F (P, S, c, i); DK ||= T_(i) */
        __rtl_digest_updatePBKDF2 (
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
        __rtl_digest_updatePBKDF2 (
            &digest, T,
            pSaltData, nSaltLen,
            nCount, OSL_NETDWORD(i));

        /* DK ||= T_(i) */
        memcpy (pKeyData, T, nKeyLen);
        memset (T, 0, DIGEST_CBLOCK_PBKDF2);
    }

    memset (&digest, 0, sizeof (digest));
    return rtl_Digest_E_None;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
