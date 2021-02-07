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

#include <sal/config.h>

#include <cassert>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <osl/mutex.h>
#include <rtl/tencinfo.h>

#include <string.h>
#include <sal/alloca.h>
#include <sal/log.hxx>

#include "hash.hxx"
#include "strimp.hxx"
#include <rtl/character.hxx>
#include <rtl/ustring.h>

#include <rtl/math.h>

/* ======================================================================= */

#if USE_SDT_PROBES
#define RTL_LOG_STRING_BITS         16
#endif

#include "strtmpl.hxx"

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
template<>
rtl_uString rtl::str::EmptyStringImpl<rtl_uString>::data =
{
    sal_Int32(SAL_STRING_INTERN_FLAG|SAL_STRING_STATIC_FLAG|1), /*sal_Int32    refCount; */
    0,                                               /*sal_Int32    length;   */
    { 0 }                                            /*sal_Unicode  buffer[1];*/
};

/* ======================================================================= */

sal_Int32 rtl_ustr_indexOfAscii_WithLength(
    sal_Unicode const * str, sal_Int32 len,
    char const * subStr, sal_Int32 subLen) SAL_THROW_EXTERN_C()
{
    assert(len >= 0);
    assert(subLen >= 0);
    if (subLen > 0 && subLen <= len)
    {
        sal_Unicode const* end = str + len;
        sal_Unicode const* cursor = str;

        while(cursor < end)
        {
            cursor = std::char_traits<sal_Unicode>::find(cursor, end - cursor, *subStr);
            if(!cursor || (end - cursor < subLen))
            {
                /* no enough left to actually have a match */
                break;
            }
            /* now it is worth trying a full match */
            if (rtl_ustr_asciil_reverseEquals_WithLength(cursor, subStr, subLen))
            {
                return cursor - str;
            }
            cursor += 1;
        }
    }
    return -1;
}

sal_Int32 rtl_ustr_lastIndexOfAscii_WithLength(
    sal_Unicode const * str, sal_Int32 len,
    char const * subStr, sal_Int32 subLen) SAL_THROW_EXTERN_C()
{
    assert(len >= 0);
    assert(subLen >= 0);
    if (subLen > 0 && subLen <= len) {
        sal_Int32 i;
        for (i = len - subLen; i >= 0; --i) {
            if (rtl_ustr_asciil_reverseEquals_WithLength(
                    str + i, subStr, subLen))
            {
                return i;
            }
        }
    }
    return -1;
}

sal_Int32 SAL_CALL rtl_ustr_valueOfFloat(sal_Unicode * pStr, float f)
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    rtl_uString * pResult = nullptr;
    sal_Int32 nLen;
    rtl_math_doubleToUString(
        &pResult, nullptr, 0, f, rtl_math_StringFormat_G,
        RTL_USTR_MAX_VALUEOFFLOAT - RTL_CONSTASCII_LENGTH("-x.E-xxx"), '.', nullptr,
        0, true);
    nLen = pResult->length;
    OSL_ASSERT(nLen < RTL_USTR_MAX_VALUEOFFLOAT);
    memcpy(pStr, pResult->buffer, (nLen + 1) * sizeof(sal_Unicode));
    rtl_uString_release(pResult);
    return nLen;
}

sal_Int32 SAL_CALL rtl_ustr_valueOfDouble(sal_Unicode * pStr, double d)
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    rtl_uString * pResult = nullptr;
    sal_Int32 nLen;
    rtl_math_doubleToUString(
        &pResult, nullptr, 0, d, rtl_math_StringFormat_G,
        RTL_USTR_MAX_VALUEOFDOUBLE - RTL_CONSTASCII_LENGTH("-x.E-xxx"), '.', nullptr,
        0, true);
    nLen = pResult->length;
    OSL_ASSERT(nLen < RTL_USTR_MAX_VALUEOFDOUBLE);
    memcpy(pStr, pResult->buffer, (nLen + 1) * sizeof(sal_Unicode));
    rtl_uString_release(pResult);
    return nLen;
}

namespace {

// Avoid -fsanitize=undefined warning e.g. "runtime error: value 1e+99 is
// outside the range of representable values of type 'float'":
float doubleToFloat(double x) {
    return
        x < -std::numeric_limits<float>::max()
        ? -std::numeric_limits<float>::infinity()
        : x > std::numeric_limits<float>::max()
        ? std::numeric_limits<float>::infinity()
        : static_cast<float>(x);
}

}

float SAL_CALL rtl_ustr_toFloat(sal_Unicode const * pStr) SAL_THROW_EXTERN_C()
{
    assert(pStr);
    return doubleToFloat(rtl_math_uStringToDouble(pStr,
                                            pStr + rtl_ustr_getLength(pStr),
                                            '.', 0, nullptr, nullptr));
}

double SAL_CALL rtl_ustr_toDouble(sal_Unicode const * pStr) SAL_THROW_EXTERN_C()
{
    assert(pStr);
    return rtl_math_uStringToDouble(pStr, pStr + rtl_ustr_getLength(pStr), '.',
                                    0, nullptr, nullptr);
}

/* ======================================================================= */

sal_Int32 SAL_CALL rtl_ustr_ascii_compare( const sal_Unicode* pStr1,
                                           const char* pStr2 )
    SAL_THROW_EXTERN_C()
{
    assert(pStr1);
    assert(pStr2);
    sal_Int32 nRet;
    for (;;)
    {
        nRet = static_cast<sal_Int32>(*pStr1)-
                     static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
        if (!(nRet == 0 && *pStr2 ))
            break;
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_ascii_compare - Found char > 127" );
        pStr1++;
        pStr2++;
    }

    return nRet;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL rtl_ustr_ascii_compare_WithLength( const sal_Unicode* pStr1,
                                                      sal_Int32 nStr1Len,
                                                      const char* pStr2 )
    SAL_THROW_EXTERN_C()
{
    assert(pStr1);
    assert(nStr1Len >= 0);
    assert(pStr2);
    sal_Int32 nRet = 0;
    for (;;)
    {
        nRet = (nStr1Len ? static_cast<sal_Int32>(*pStr1) : 0) -
               static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
        if (!(nRet == 0 && nStr1Len && *pStr2 ))
            break;
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_ascii_compare_WithLength - Found char > 127" );
        pStr1++;
        pStr2++;
        nStr1Len--;
    }

    return nRet;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL rtl_ustr_ascii_shortenedCompare_WithLength( const sal_Unicode* pStr1,
                                                               sal_Int32 nStr1Len,
                                                               const char* pStr2,
                                                               sal_Int32 nShortenedLength )
    SAL_THROW_EXTERN_C()
{
    assert(nStr1Len >= 0);
    assert(nShortenedLength >= 0);
    const sal_Unicode*  pStr1End = pStr1 + nStr1Len;
    sal_Int32           nRet;
    while ( (nShortenedLength > 0) &&
            (pStr1 < pStr1End) && *pStr2 )
    {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_ascii_shortenedCompare_WithLength - Found char > 127" );

        nRet = static_cast<sal_Int32>(*pStr1)-
               static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
        if ( nRet != 0 )
            return nRet;

        nShortenedLength--;
        pStr1++;
        pStr2++;
    }

    if ( nShortenedLength <= 0 )
        return 0;

    if ( *pStr2 )
    {
        OSL_ENSURE( pStr1 == pStr1End, "pStr1 == pStr1End failed" );
        // first is a substring of the second string => less (negative value)
        nRet = -1;
    }
    else
    {
        // greater or equal
        nRet = pStr1End - pStr1;
    }

    return nRet;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL rtl_ustr_asciil_reverseCompare_WithLength( const sal_Unicode* pStr1,
                                                              sal_Int32 nStr1Len,
                                                              const char* pStr2,
                                                              sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
{
    assert(nStr1Len >= 0 && nStr2Len >= 0);
    const sal_Unicode*  pStr1Run = pStr1+nStr1Len;
    const char*     pStr2Run = pStr2+nStr2Len;
    sal_Int32           nRet;
    while ( (pStr1 < pStr1Run) && (pStr2 < pStr2Run) )
    {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_asciil_reverseCompare_WithLength - Found char > 127" );
        pStr1Run--;
        pStr2Run--;
        nRet = static_cast<sal_Int32>(*pStr1Run)- static_cast<sal_Int32>(*pStr2Run);
        if ( nRet )
            return nRet;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

sal_Bool SAL_CALL rtl_ustr_asciil_reverseEquals_WithLength( const sal_Unicode* pStr1,
                                                              const char* pStr2,
                                                              sal_Int32 nStrLen )
    SAL_THROW_EXTERN_C()
{
    assert(nStrLen >= 0);
    const sal_Unicode*  pStr1Run = pStr1+nStrLen;
    const char*     pStr2Run = pStr2+nStrLen;
    while ( pStr1 < pStr1Run )
    {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_asciil_reverseEquals_WithLength - Found char > 127" );
        pStr1Run--;
        pStr2Run--;
        if( *pStr1Run != static_cast<sal_Unicode>(*pStr2Run) )
            return false;
    }

    return true;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL rtl_ustr_ascii_compareIgnoreAsciiCase( const sal_Unicode* pStr1,
                                                          const char* pStr2 )
    SAL_THROW_EXTERN_C()
{
    assert(pStr1);
    assert(pStr2);
    sal_Int32   nRet;
    sal_Int32   c1;
    sal_Int32   c2;
    do
    {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_ascii_compareIgnoreAsciiCase - Found char > 127" );
        /* If character between 'A' and 'Z', then convert it to lowercase */
        c1 = static_cast<sal_Int32>(*pStr1);
        c2 = static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
        if ( nRet != 0 )
            return nRet;

        pStr1++;
        pStr2++;
    }
    while ( c2 );

    return 0;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( const sal_Unicode* pStr1,
                                                                     sal_Int32 nStr1Len,
                                                                     const char* pStr2 )
    SAL_THROW_EXTERN_C()
{
    assert(nStr1Len >= 0);
    assert(pStr2);
    sal_Int32   nRet;
    sal_Int32   c1;
    sal_Int32   c2;
    do
    {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength - Found char > 127" );
        if ( !nStr1Len )
            return *pStr2 == '\0' ? 0 : -1;

        /* If character between 'A' and 'Z', then convert it to lowercase */
        c1 = static_cast<sal_Int32>(*pStr1);
        c2 = static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
        if ( nRet != 0 )
            return nRet;

        pStr1++;
        pStr2++;
        nStr1Len--;
    }
    while( c2 );

    return 0;
}

sal_Int32 rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(
    sal_Unicode const * first, sal_Int32 firstLen,
    char const * second, sal_Int32 secondLen) SAL_THROW_EXTERN_C()
{
    assert(firstLen >= 0 && secondLen >= 0);
    sal_Int32 i;
    sal_Int32 len = std::min(firstLen, secondLen);
    for (i = 0; i < len; ++i) {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*second)) > 127, "rtl.string",
                    "rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths - Found char > 127" );
        sal_Int32 c1 = *first++;
        sal_Int32 c2 = static_cast<unsigned char>(*second++);
        sal_Int32 d;
        if (c1 >= 65 && c1 <= 90) {
            c1 += 32;
        }
        if (c2 >= 65 && c2 <= 90) {
            c2 += 32;
        }
        d = c1 - c2;
        if (d != 0) {
            return d;
        }
    }
    return firstLen - secondLen;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( const sal_Unicode* pStr1,
                                                                              sal_Int32 nStr1Len,
                                                                              const char* pStr2,
                                                                              sal_Int32 nShortenedLength )
    SAL_THROW_EXTERN_C()
{
    assert(nStr1Len >= 0);
    assert(nShortenedLength >= 0);
    const sal_Unicode*  pStr1End = pStr1 + nStr1Len;
    sal_Int32           nRet;
    sal_Int32           c1;
    sal_Int32           c2;
    while ( (nShortenedLength > 0) &&
            (pStr1 < pStr1End) && *pStr2 )
    {
        /* Check ASCII range */
        SAL_WARN_IF( (static_cast<unsigned char>(*pStr2)) > 127, "rtl.string",
                    "rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength - Found char > 127" );

        /* If character between 'A' and 'Z', then convert it to lowercase */
        c1 = static_cast<sal_Int32>(*pStr1);
        c2 = static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
        if ( nRet != 0 )
            return nRet;

        nShortenedLength--;
        pStr1++;
        pStr2++;
    }

    if ( nShortenedLength <= 0 )
        return 0;

    if ( *pStr2 )
    {
        OSL_ENSURE( pStr1 == pStr1End, "pStr1 == pStr1End failed" );
        // first is a substring of the second string => less (negative value)
        nRet = -1;
    }
    else
    {
        // greater or equal
        nRet = pStr1End - pStr1;
    }

    return nRet;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_uString_newFromAscii( rtl_uString** ppThis,
                                        const char* pCharStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    sal_Int32 nLen;

    if ( pCharStr )
    {
        const char* pTempStr = pCharStr;
        while( *pTempStr )
            pTempStr++;
        nLen = pTempStr-pCharStr;
    }
    else
        nLen = 0;

    if ( !nLen )
    {
        rtl_uString_new( ppThis );
        return;
    }

    if ( *ppThis )
        rtl_uString_release( *ppThis );

    *ppThis = rtl_uString_ImplAlloc( nLen );
    OSL_ASSERT(*ppThis != nullptr);
    if ( !(*ppThis) )
        return;

    sal_Unicode* pBuffer = (*ppThis)->buffer;
    do
    {
        assert(static_cast<unsigned char>(*pCharStr) < 0x80); // ASCII range
        *pBuffer = *pCharStr;
        pBuffer++;
        pCharStr++;
    }
    while ( *pCharStr );

    RTL_LOG_STRING_NEW( *ppThis );
}

void SAL_CALL rtl_uString_newFromCodePoints(
    rtl_uString ** newString, sal_uInt32 const * codePoints,
    sal_Int32 codePointCount) SAL_THROW_EXTERN_C()
{
    sal_Int32 n;
    sal_Int32 i;
    sal_Unicode * p;
    assert(newString != nullptr);
    assert((codePoints != nullptr || codePointCount == 0) && codePointCount >= 0);
    if (codePointCount == 0) {
        rtl_uString_new(newString);
        return;
    }
    if (*newString != nullptr) {
        rtl_uString_release(*newString);
    }
    n = codePointCount;
    for (i = 0; i < codePointCount; ++i) {
        OSL_ASSERT(rtl::isUnicodeCodePoint(codePoints[i]));
        if (codePoints[i] >= 0x10000) {
            ++n;
        }
    }
    /* Builds on the assumption that sal_Int32 uses 32 bit two's complement
       representation with wrap around (the necessary number of UTF-16 code
       units will be no larger than 2 * SAL_MAX_INT32, represented as
       sal_Int32 -2): */
    if (n < 0) {
        // coverity[dead_error_begin] - assumes wrap around
        *newString = nullptr;
        return;
    }
    *newString = rtl_uString_ImplAlloc(n);
    if (*newString == nullptr) {
        return;
    }
    p = (*newString)->buffer;
    for (i = 0; i < codePointCount; ++i) {
        p += rtl::splitSurrogates(codePoints[i], p);
    }
    RTL_LOG_STRING_NEW( *newString );
}

void rtl_uString_newConcatAsciiL(
    rtl_uString ** newString, rtl_uString * left, char const * right,
    sal_Int32 rightLength)
{
    assert(newString != nullptr);
    assert(left != nullptr);
    assert(right != nullptr);
    assert(rightLength >= 0);
    if (left->length > std::numeric_limits<sal_Int32>::max() - rightLength) {
#if !defined(__COVERITY__)
        throw std::length_error("rtl_uString_newConcatAsciiL");
#else
        //coverity doesn't report std::bad_alloc as an unhandled exception when
        //potentially thrown from destructors but does report std::length_error
        throw std::bad_alloc();
#endif
    }
    sal_Int32 n = left->length + rightLength;
    rtl_uString_assign(newString, left);
    rtl_uString_ensureCapacity(newString, n);
    sal_Unicode * p = (*newString)->buffer + (*newString)->length;
    for (sal_Int32 i = 0; i != rightLength; ++i) {
        p[i] = static_cast<unsigned char>(right[i]);
    }
    (*newString)->buffer[n] = 0;
    (*newString)->length = n;
}

void rtl_uString_newConcatUtf16L(
    rtl_uString ** newString, rtl_uString * left, sal_Unicode const * right,
    sal_Int32 rightLength)
{
    assert(newString != nullptr);
    assert(left != nullptr);
    assert(right != nullptr || rightLength == 0);
    assert(rightLength >= 0);
    if (left->length > std::numeric_limits<sal_Int32>::max() - rightLength) {
#if !defined(__COVERITY__)
        throw std::length_error("rtl_uString_newConcatUtf16L");
#else
        //coverity doesn't report std::bad_alloc as an unhandled exception when
        //potentially thrown from destructors but does report std::length_error
        throw std::bad_alloc();
#endif
    }
    sal_Int32 n = left->length + rightLength;
    rtl_uString_assign(newString, left);
    rtl_uString_ensureCapacity(newString, n);
    if (rightLength != 0) {
        memcpy(
            (*newString)->buffer + (*newString)->length, right,
            rightLength * sizeof (sal_Unicode));
    }
    (*newString)->buffer[n] = 0;
    (*newString)->length = n;
}

/* ======================================================================= */

static int rtl_ImplGetFastUTF8UnicodeLen( const char* pStr, sal_Int32 nLen, bool * ascii )
{
    int             n;
    const char* pEndStr;

    *ascii = true;
    n = 0;
    pEndStr  = pStr+nLen;
    while ( pStr < pEndStr )
    {
        unsigned char c = static_cast<unsigned char>(*pStr);

        if ( !(c & 0x80) )
            pStr++;
        else
        {
            if ( (c & 0xE0) == 0xC0 )
                pStr += 2;
            else if ( (c & 0xF0) == 0xE0 )
                pStr += 3;
            else if ( (c & 0xF8) == 0xF0 )
                pStr += 4;
            else if ( (c & 0xFC) == 0xF8 )
                pStr += 5;
            else if ( (c & 0xFE) == 0xFC )
                pStr += 6;
            else
                pStr++;
            *ascii = false;
        }

        n++;
    }

    return n;
}

/* ----------------------------------------------------------------------- */

static void rtl_string2UString_status( rtl_uString** ppThis,
                                       const char* pStr,
                                       sal_Int32 nLen,
                                       rtl_TextEncoding eTextEncoding,
                                       sal_uInt32 nCvtFlags,
                                       sal_uInt32 *pInfo )
{
    OSL_ENSURE(nLen == 0 || rtl_isOctetTextEncoding(eTextEncoding),
               "rtl_string2UString_status() - Wrong TextEncoding" );

    if ( !nLen )
    {
        rtl_uString_new( ppThis );
        if (pInfo != nullptr) {
            *pInfo = 0;
        }
    }
    else
    {
        if ( *ppThis )
            rtl_uString_release( *ppThis );

        /* Optimization for US-ASCII */
        if ( eTextEncoding == RTL_TEXTENCODING_ASCII_US )
        {
            sal_Unicode* pBuffer;
            *ppThis = rtl_uString_ImplAlloc( nLen );
            if (*ppThis == nullptr) {
                if (pInfo != nullptr) {
                    *pInfo = RTL_TEXTTOUNICODE_INFO_ERROR |
                        RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                }
                return;
            }
            pBuffer = (*ppThis)->buffer;
            sal_Int32 nLenCopy(nLen);
            const char *pStrCopy(pStr);
            do
            {
                /* Check ASCII range */
                if (static_cast<unsigned char>(*pStrCopy) > 127)
                {
                    rtl_uString_release(*ppThis);
                    goto retry; // cancel loop - try again with the converter
                }

                *pBuffer = *pStrCopy;
                pBuffer++;
                pStrCopy++;
                nLenCopy--;
            }
            while (nLenCopy);
            if (pInfo != nullptr) {
                *pInfo = 0;
            }
            RTL_LOG_STRING_NEW( *ppThis );
            return;
        }
retry:
        {
            rtl_uString*                pTemp;
            rtl_uString*                pTemp2 = nullptr;
            rtl_TextToUnicodeConverter  hConverter;
            sal_uInt32                  nInfo;
            sal_Size                    nSrcBytes;
            sal_Size                    nDestChars;
            sal_Size                    nNewLen;

            /* Optimization for UTF-8 - we try to calculate the exact length */
            /* For all other encoding we try the maximum - and reallocate
               the buffer if needed */
            if ( eTextEncoding == RTL_TEXTENCODING_UTF8 )
            {
                bool ascii;
                nNewLen = rtl_ImplGetFastUTF8UnicodeLen( pStr, nLen, &ascii );
                /* Includes the string only ASCII, then we could copy
                   the buffer faster */
                if ( ascii )
                {
                    sal_Unicode* pBuffer;
                    *ppThis = rtl_uString_ImplAlloc( nLen );
                    if (*ppThis == nullptr)
                    {
                        if (pInfo != nullptr) {
                            *pInfo = RTL_TEXTTOUNICODE_INFO_ERROR |
                                RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                        }
                        return;
                    }
                    pBuffer = (*ppThis)->buffer;
                    do
                    {
                        assert((static_cast<unsigned char>(*pStr)) <= 127);
                        *pBuffer = *pStr;
                        pBuffer++;
                        pStr++;
                        nLen--;
                    }
                    while ( nLen );
                    if (pInfo != nullptr) {
                        *pInfo = 0;
                    }
                    RTL_LOG_STRING_NEW( *ppThis );
                    return;
                }
            }
            else
                nNewLen = nLen;

            nCvtFlags |= RTL_TEXTTOUNICODE_FLAGS_FLUSH;
            hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );

            pTemp = rtl_uString_ImplAlloc( nNewLen );
            if (pTemp == nullptr) {
                if (pInfo != nullptr) {
                    *pInfo = RTL_TEXTTOUNICODE_INFO_ERROR |
                        RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                }
                return;
            }
            nDestChars = rtl_convertTextToUnicode( hConverter, nullptr,
                                                   pStr, nLen,
                                                   pTemp->buffer, nNewLen,
                                                   nCvtFlags,
                                                   &nInfo, &nSrcBytes );

            /* Buffer not big enough, try again with enough space */
            /* Shouldn't be the case, but if we get textencoding which
               could results in more unicode characters we have this
               code here. Could be the case for apple encodings */
            while ( nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL )
            {
                rtl_freeString( pTemp );
                nNewLen += 8;
                pTemp = rtl_uString_ImplAlloc( nNewLen );
                if (pTemp == nullptr) {
                    if (pInfo != nullptr) {
                        *pInfo = RTL_TEXTTOUNICODE_INFO_ERROR |
                            RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                    }
                    return;
                }
                nDestChars = rtl_convertTextToUnicode( hConverter, nullptr,
                                                       pStr, nLen,
                                                       pTemp->buffer, nNewLen,
                                                       nCvtFlags,
                                                       &nInfo, &nSrcBytes );
            }

            if (pInfo)
                *pInfo = nInfo;

            /* Set the buffer to the correct size or if there is too
               much overhead, reallocate to the correct size */
            if ( nNewLen > nDestChars+8 )
            {
                pTemp2 = rtl_uString_ImplAlloc( nDestChars );
            }
            if (pTemp2 != nullptr)
            {
                rtl::str::Copy(pTemp2->buffer, pTemp->buffer, nDestChars);
                rtl_freeString(pTemp);
                pTemp = pTemp2;
            }
            else
            {
                pTemp->length = nDestChars;
                pTemp->buffer[nDestChars] = 0;
            }

            rtl_destroyTextToUnicodeConverter( hConverter );
            *ppThis = pTemp;

            /* Results the conversion in an empty buffer -
               create an empty string */
            if ( pTemp && !nDestChars )
                rtl_uString_new( ppThis );
        }
    }
    RTL_LOG_STRING_NEW( *ppThis );
}

void SAL_CALL rtl_string2UString( rtl_uString** ppThis,
                                  const char* pStr,
                                  sal_Int32 nLen,
                                  rtl_TextEncoding eTextEncoding,
                                  sal_uInt32 nCvtFlags ) SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(nLen >= 0);
    rtl_string2UString_status( ppThis, pStr, nLen, eTextEncoding,
                               nCvtFlags, nullptr );
}

/* ----------------------------------------------------------------------- */

namespace {

enum StrLifecycle {
    CANNOT_RETURN,
    CAN_RETURN = 1
};

}

static oslMutex
getInternMutex()
{
    static oslMutex pPoolGuard = osl_createMutex();

    return pPoolGuard;
}

/* returns true if we found a dup in the pool */
static void rtl_ustring_intern_internal( rtl_uString ** newStr,
                                         rtl_uString  * str,
                                         StrLifecycle   can_return )
{
    oslMutex pPoolMutex;

    pPoolMutex = getInternMutex();

    osl_acquireMutex( pPoolMutex );

    *newStr = rtl_str_hash_intern (str, can_return);

    osl_releaseMutex( pPoolMutex );

    RTL_LOG_STRING_INTERN_NEW(*newStr, str);

    if( can_return && *newStr != str )
    { /* we dupped, then found a match */
        rtl_freeString( str );
    }
}

void SAL_CALL rtl_uString_intern( rtl_uString ** newStr,
                                  rtl_uString  * str) SAL_THROW_EXTERN_C()
{
    assert(newStr);
    assert(str);
    if (SAL_STRING_IS_INTERN(str))
    {
        rtl::str::acquire(str);
        *newStr = str;
    }
    else
    {
        rtl_uString *pOrg = *newStr;
        *newStr = nullptr;
        rtl_ustring_intern_internal( newStr, str, CANNOT_RETURN );
        if (pOrg)
            rtl_uString_release (pOrg);
    }
}

static int rtl_canGuessUOutputLength( int len, rtl_TextEncoding eTextEncoding )
{
    // FIXME: Maybe we should use a bit flag in the higher bits of the
    // eTextEncoding value itself to determine the encoding type.  But if we
    // do, be sure to mask the value in certain places that expect the values
    // to be numbered serially from 0 and up.  One such place is
    // Impl_getTextEncodingData().

    switch ( eTextEncoding )
    {
        // 1 to 1 (with no zero elements)
        case RTL_TEXTENCODING_IBM_437:
        case RTL_TEXTENCODING_IBM_850:
        case RTL_TEXTENCODING_IBM_860:
        case RTL_TEXTENCODING_IBM_861:
        case RTL_TEXTENCODING_IBM_863:
        case RTL_TEXTENCODING_IBM_865:
            return len;
    }
    return 0;
}

void SAL_CALL rtl_uString_internConvert( rtl_uString   ** newStr,
                                         const char * str,
                                         sal_Int32        len,
                                         rtl_TextEncoding eTextEncoding,
                                         sal_uInt32       convertFlags,
                                         sal_uInt32     * pInfo )
    SAL_THROW_EXTERN_C()
{
    assert(newStr);
    assert(len >= 0);
    rtl_uString *scratch;

    if (*newStr)
    {
        rtl_uString_release (*newStr);
        *newStr = nullptr;
    }

    if ( len < 256 )
    { // try various optimisations
        sal_Int32 ulen;
        if ( eTextEncoding == RTL_TEXTENCODING_ASCII_US )
        {
            int i;
            rtl_uString *pScratch;
            pScratch = static_cast< rtl_uString * >(
                alloca(sizeof (rtl_uString) + len * sizeof (sal_Unicode)));
            for (i = 0; i < len; i++)
            {
                /* Check ASCII range */
                SAL_WARN_IF( (static_cast<unsigned char>(str[i])) > 127, "rtl.string",
                            "rtl_ustring_internConvert() - Found char > 127 and RTL_TEXTENCODING_ASCII_US is specified" );
                pScratch->buffer[i] = str[i];
            }
            pScratch->length = len;
            rtl_ustring_intern_internal( newStr, pScratch, CANNOT_RETURN );
            return;
        }
        if ( (ulen = rtl_canGuessUOutputLength(len, eTextEncoding)) != 0 )
        {
            rtl_uString *pScratch;
            rtl_TextToUnicodeConverter hConverter;
            sal_Size nSrcBytes;
            sal_uInt32 nInfo;

            pScratch = static_cast< rtl_uString * >(
                alloca(
                    sizeof (rtl_uString) + ulen * sizeof (sal_Unicode)));

            hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );
            rtl_convertTextToUnicode(
                hConverter, nullptr, str, len, pScratch->buffer, ulen, convertFlags, &nInfo, &nSrcBytes );
            rtl_destroyTextToUnicodeConverter( hConverter );

            if (pInfo)
                *pInfo = nInfo;

            pScratch->length = ulen;
            rtl_ustring_intern_internal( newStr, pScratch, CANNOT_RETURN );
            return;
        }

        /* FIXME: we want a nice UTF-8 / alloca shortcut here */
    }

    scratch = nullptr;
    rtl_string2UString_status( &scratch, str, len, eTextEncoding, convertFlags,
                               pInfo );
    if (!scratch) {
        return;
    }
    rtl_ustring_intern_internal( newStr, scratch, CAN_RETURN );
}

static void
internRelease (rtl_uString *pThis)
{
    rtl_uString *pFree = nullptr;
    if ( SAL_STRING_REFCOUNT(
             osl_atomic_decrement( &(pThis->refCount) ) ) == 0)
    {
        RTL_LOG_STRING_INTERN_DELETE(pThis);
        oslMutex pPoolMutex = getInternMutex();
        osl_acquireMutex( pPoolMutex );

        rtl_str_hash_remove (pThis);

        /* May have been separately acquired */
        if ( SAL_STRING_REFCOUNT(
                 osl_atomic_increment( &(pThis->refCount) ) ) == 1 )
        {
            /* we got the last ref */
            pFree = pThis;
        }
        else /* very unusual */
        {
            internRelease (pThis);
        }

        osl_releaseMutex( pPoolMutex );
    }
    if (pFree)
        rtl_freeString (pFree);
}

sal_uInt32 SAL_CALL rtl_uString_iterateCodePoints(
    rtl_uString const * string, sal_Int32 * indexUtf16,
    sal_Int32 incrementCodePoints)
{
    sal_Int32 n;
    sal_Unicode cu;
    sal_uInt32 cp;
    assert(string != nullptr && indexUtf16 != nullptr);
    n = *indexUtf16;
    assert(n >= 0 && n <= string->length);
    while (incrementCodePoints < 0) {
        assert(n > 0);
        cu = string->buffer[--n];
        if (rtl::isLowSurrogate(cu) && n != 0 &&
            rtl::isHighSurrogate(string->buffer[n - 1]))
        {
            --n;
        }
        ++incrementCodePoints;
    }
    assert(n >= 0 && n < string->length);
    cu = string->buffer[n];
    if (rtl::isHighSurrogate(cu) && string->length - n >= 2 &&
        rtl::isLowSurrogate(string->buffer[n + 1]))
    {
        cp = rtl::combineSurrogates(cu, string->buffer[n + 1]);
    } else {
        cp = cu;
    }
    while (incrementCodePoints > 0) {
        assert(n < string->length);
        cu = string->buffer[n++];
        if (rtl::isHighSurrogate(cu) && n != string->length &&
            rtl::isLowSurrogate(string->buffer[n]))
        {
            ++n;
        }
        --incrementCodePoints;
    }
    assert(n >= 0 && n <= string->length);
    *indexUtf16 = n;
    return cp;
}

sal_Bool rtl_convertStringToUString(
    rtl_uString ** target, char const * source, sal_Int32 length,
    rtl_TextEncoding encoding, sal_uInt32 flags) SAL_THROW_EXTERN_C()
{
    assert(target);
    assert(length >= 0);
    sal_uInt32 info;
    rtl_string2UString_status(target, source, length, encoding, flags, &info);
    return (info & RTL_TEXTTOUNICODE_INFO_ERROR) == 0;
}

void rtl_uString_newReplaceFirst(
    rtl_uString ** newStr, rtl_uString * str, rtl_uString const * from,
    rtl_uString const * to, sal_Int32 * index) SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(from != nullptr);
    assert(to != nullptr);
    sal_Int32 i = rtl_ustr_indexOfStr_WithLength(
        str->buffer + *index, str->length - *index, from->buffer, from->length);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(from->length <= str->length);
        if (str->length - from->length > SAL_MAX_INT32 - to->length) {
            std::abort();
        }
        sal_Int32 n = str->length - from->length + to->length;
        rtl_uString_acquire(str); // in case *newStr == str
        rtl_uString_new_WithLength(newStr, n);
        if (n != 0) {
            (*newStr)->length = n;
            assert(i >= 0 && i < str->length);
            memcpy(
                (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
            memcpy(
                (*newStr)->buffer + i, to->buffer,
                to->length * sizeof (sal_Unicode));
            memcpy(
                (*newStr)->buffer + i + to->length,
                str->buffer + i + from->length,
                (str->length - i - from->length) * sizeof (sal_Unicode));
        }
        rtl_uString_release(str);
    }
    *index = i;
}

void rtl_uString_newReplaceFirstAsciiL(
    rtl_uString ** newStr, rtl_uString * str, char const * from,
    sal_Int32 fromLength, rtl_uString const * to, sal_Int32 * index)
    SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(fromLength >= 0);
    assert(to != nullptr);
    sal_Int32 i = rtl_ustr_indexOfAscii_WithLength(
        str->buffer + *index, str->length - *index, from, fromLength);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(fromLength <= str->length);
        if (str->length - fromLength > SAL_MAX_INT32 - to->length) {
            std::abort();
        }
        sal_Int32 n = str->length - fromLength + to->length;
        rtl_uString_acquire(str); // in case *newStr == str
        rtl_uString_new_WithLength(newStr, n);
        if (n != 0) {
            (*newStr)->length = n;
            assert(i >= 0 && i < str->length);
            memcpy(
                (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
            memcpy(
                (*newStr)->buffer + i, to->buffer,
                to->length * sizeof (sal_Unicode));
            memcpy(
                (*newStr)->buffer + i + to->length,
                str->buffer + i + fromLength,
                (str->length - i - fromLength) * sizeof (sal_Unicode));
        }
        rtl_uString_release(str);
    }
    *index = i;
}

void rtl_uString_newReplaceFirstToAsciiL(
    rtl_uString ** newStr, rtl_uString * str, rtl_uString const * from,
    char const * to, sal_Int32 toLength, sal_Int32 * index)
    SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(from != nullptr);
    assert(toLength >= 0);
    sal_Int32 i = rtl_ustr_indexOfStr_WithLength(
        str->buffer + *index, str->length - *index, from->buffer, from->length);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(from->length <= str->length);
        if (str->length - from->length > SAL_MAX_INT32 - toLength) {
            std::abort();
        }
        sal_Int32 n = str->length - from->length + toLength;
        rtl_uString_acquire(str); // in case *newStr == str
        rtl_uString_new_WithLength(newStr, n);
        if (n != 0) {
            (*newStr)->length = n;
            assert(i >= 0 && i < str->length);
            memcpy(
                (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
            for (sal_Int32 j = 0; j != toLength; ++j) {
                assert(static_cast< unsigned char >(to[j]) <= 0x7F);
                (*newStr)->buffer[i + j] = to[j];
            }
            memcpy(
                (*newStr)->buffer + i + toLength,
                str->buffer + i + from->length,
                (str->length - i - from->length) * sizeof (sal_Unicode));
        }
        rtl_uString_release(str);
    }
    *index = i;
}

void rtl_uString_newReplaceFirstAsciiLAsciiL(
    rtl_uString ** newStr, rtl_uString * str, char const * from,
    sal_Int32 fromLength, char const * to, sal_Int32 toLength,
    sal_Int32 * index) SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(fromLength >= 0);
    assert(to != nullptr);
    assert(toLength >= 0);
    sal_Int32 i = rtl_ustr_indexOfAscii_WithLength(
        str->buffer + *index, str->length - *index, from, fromLength);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(fromLength <= str->length);
        if (str->length - fromLength > SAL_MAX_INT32 - toLength) {
            std::abort();
        }
        sal_Int32 n = str->length - fromLength + toLength;
        rtl_uString_acquire(str); // in case *newStr == str
        rtl_uString_new_WithLength(newStr, n);
        if (n != 0) {
            (*newStr)->length = n;
            assert(i >= 0 && i < str->length);
            memcpy(
                (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
            for (sal_Int32 j = 0; j != toLength; ++j) {
                assert(static_cast< unsigned char >(to[j]) <= 0x7F);
                (*newStr)->buffer[i + j] = to[j];
            }
            memcpy(
                (*newStr)->buffer + i + toLength,
                str->buffer + i + fromLength,
                (str->length - i - fromLength) * sizeof (sal_Unicode));
        }
        rtl_uString_release(str);
    }
    *index = i;
}

void rtl_uString_newReplaceFirstAsciiLUtf16L(
    rtl_uString ** newStr, rtl_uString * str, char const * from,
    sal_Int32 fromLength, sal_Unicode const * to, sal_Int32 toLength,
    sal_Int32 * index) SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(fromLength >= 0);
    assert(to != nullptr || toLength == 0);
    assert(toLength >= 0);
    sal_Int32 i = rtl_ustr_indexOfAscii_WithLength(
        str->buffer + *index, str->length - *index, from, fromLength);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(fromLength <= str->length);
        if (str->length - fromLength > SAL_MAX_INT32 - toLength) {
            rtl_uString_release(*newStr);
            *newStr = nullptr;
        } else {
            sal_Int32 n = str->length - fromLength + toLength;
            rtl_uString_acquire(str); // in case *newStr == str
            rtl_uString_new_WithLength(newStr, n);
            if (n != 0 && /*TODO:*/ *newStr != nullptr) {
                (*newStr)->length = n;
                assert(i >= 0 && i < str->length);
                memcpy(
                    (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
                if (toLength != 0) {
                    memcpy(
                        (*newStr)->buffer + i, to, toLength * sizeof (sal_Unicode));
                }
                memcpy(
                    (*newStr)->buffer + i + toLength,
                    str->buffer + i + fromLength,
                    (str->length - i - fromLength) * sizeof (sal_Unicode));
            }
            rtl_uString_release(str);
        }
    }
    *index = i;
}

void rtl_uString_newReplaceFirstUtf16LAsciiL(
    rtl_uString ** newStr, rtl_uString * str, sal_Unicode const * from,
    sal_Int32 fromLength, char const * to, sal_Int32 toLength,
    sal_Int32 * index) SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(fromLength >= 0);
    assert(to != nullptr);
    assert(toLength >= 0);
    sal_Int32 i = rtl_ustr_indexOfStr_WithLength(
        str->buffer + *index, str->length - *index, from, fromLength);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(fromLength <= str->length);
        if (str->length - fromLength > SAL_MAX_INT32 - toLength) {
            rtl_uString_release(*newStr);
            *newStr = nullptr;
        } else {
            sal_Int32 n = str->length - fromLength + toLength;
            rtl_uString_acquire(str); // in case *newStr == str
            rtl_uString_new_WithLength(newStr, n);
            if (n != 0 && /*TODO:*/ *newStr != nullptr) {
                (*newStr)->length = n;
                assert(i >= 0 && i < str->length);
                memcpy(
                    (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
                for (sal_Int32 j = 0; j != toLength; ++j) {
                    assert(static_cast< unsigned char >(to[j]) <= 0x7F);
                    (*newStr)->buffer[i + j] = to[j];
                }
                memcpy(
                    (*newStr)->buffer + i + toLength,
                    str->buffer + i + fromLength,
                    (str->length - i - fromLength) * sizeof (sal_Unicode));
            }
            rtl_uString_release(str);
        }
    }
    *index = i;
}

void rtl_uString_newReplaceFirstUtf16LUtf16L(
    rtl_uString ** newStr, rtl_uString * str, sal_Unicode const * from,
    sal_Int32 fromLength, sal_Unicode const * to, sal_Int32 toLength,
    sal_Int32 * index) SAL_THROW_EXTERN_C()
{
    assert(str != nullptr);
    assert(index != nullptr);
    assert(*index >= 0 && *index <= str->length);
    assert(from != nullptr || fromLength == 0);
    assert(fromLength >= 0);
    assert(to != nullptr || toLength == 0);
    assert(toLength >= 0);
    sal_Int32 i = rtl_ustr_indexOfStr_WithLength(
        str->buffer + *index, str->length - *index, from, fromLength);
    if (i == -1) {
        rtl_uString_assign(newStr, str);
    } else {
        assert(i <= str->length - *index);
        i += *index;
        assert(fromLength <= str->length);
        if (str->length - fromLength > SAL_MAX_INT32 - toLength) {
            rtl_uString_release(*newStr);
            *newStr = nullptr;
        } else {
            sal_Int32 n = str->length - fromLength + toLength;
            rtl_uString_acquire(str); // in case *newStr == str
            rtl_uString_new_WithLength(newStr, n);
            if (n != 0 && /*TODO:*/ *newStr != nullptr) {
                (*newStr)->length = n;
                assert(i >= 0 && i < str->length);
                memcpy(
                    (*newStr)->buffer, str->buffer, i * sizeof (sal_Unicode));
                if (toLength != 0) {
                    memcpy(
                        (*newStr)->buffer + i, to, toLength * sizeof (sal_Unicode));
                }
                memcpy(
                    (*newStr)->buffer + i + toLength,
                    str->buffer + i + fromLength,
                    (str->length - i - fromLength) * sizeof (sal_Unicode));
            }
            rtl_uString_release(str);
        }
    }
    *index = i;
}

void rtl_uString_newReplaceAll(
    rtl_uString ** newStr, rtl_uString * str, rtl_uString const * from,
    rtl_uString const * to) SAL_THROW_EXTERN_C()
{
    rtl_uString_newReplaceAllFromIndex( newStr, str, from, to, 0 );
}

void rtl_uString_newReplaceAllFromIndex(
    rtl_uString ** newStr, rtl_uString * str, rtl_uString const * from,
    rtl_uString const * to, sal_Int32 fromIndex) SAL_THROW_EXTERN_C()
{
    assert(to != nullptr);
    assert(fromIndex >= 0 && fromIndex <= str->length);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = fromIndex;; i += to->length) {
        rtl_uString_newReplaceFirst(newStr, *newStr, from, to, &i);
        if (i == -1) {
            break;
        }
    }
}

void rtl_uString_newReplaceAllAsciiL(
    rtl_uString ** newStr, rtl_uString * str, char const * from,
    sal_Int32 fromLength, rtl_uString const * to) SAL_THROW_EXTERN_C()
{
    assert(to != nullptr);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = 0;; i += to->length) {
        rtl_uString_newReplaceFirstAsciiL(
            newStr, *newStr, from, fromLength, to, &i);
        if (i == -1) {
            break;
        }
    }
}

void rtl_uString_newReplaceAllToAsciiL(
    rtl_uString ** newStr, rtl_uString * str, rtl_uString const * from,
    char const * to, sal_Int32 toLength) SAL_THROW_EXTERN_C()
{
    assert(from != nullptr);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = 0;; i += toLength) {
        rtl_uString_newReplaceFirstToAsciiL(
            newStr, *newStr, from, to, toLength, &i);
        if (i == -1) {
            break;
        }
    }
}

void rtl_uString_newReplaceAllAsciiLAsciiL(
    rtl_uString ** newStr, rtl_uString * str, char const * from,
    sal_Int32 fromLength, char const * to, sal_Int32 toLength)
    SAL_THROW_EXTERN_C()
{
    assert(toLength >= 0);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = 0;; i += toLength) {
        rtl_uString_newReplaceFirstAsciiLAsciiL(
            newStr, *newStr, from, fromLength, to, toLength, &i);
        if (i == -1) {
            break;
        }
    }
}

void rtl_uString_newReplaceAllAsciiLUtf16L(
    rtl_uString ** newStr, rtl_uString * str, char const * from,
    sal_Int32 fromLength, sal_Unicode const * to, sal_Int32 toLength)
    SAL_THROW_EXTERN_C()
{
    assert(toLength >= 0);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = 0;; i += toLength) {
        rtl_uString_newReplaceFirstAsciiLUtf16L(
            newStr, *newStr, from, fromLength, to, toLength, &i);
        if (i == -1 || *newStr == nullptr) {
            break;
        }
    }
}

void rtl_uString_newReplaceAllUtf16LAsciiL(
    rtl_uString ** newStr, rtl_uString * str, sal_Unicode const * from,
    sal_Int32 fromLength, char const * to, sal_Int32 toLength)
    SAL_THROW_EXTERN_C()
{
    assert(toLength >= 0);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = 0;; i += toLength) {
        rtl_uString_newReplaceFirstUtf16LAsciiL(
            newStr, *newStr, from, fromLength, to, toLength, &i);
        if (i == -1 || *newStr == nullptr) {
            break;
        }
    }
}

void rtl_uString_newReplaceAllUtf16LUtf16L(
    rtl_uString ** newStr, rtl_uString * str, sal_Unicode const * from,
    sal_Int32 fromLength, sal_Unicode const * to, sal_Int32 toLength)
    SAL_THROW_EXTERN_C()
{
    rtl_uString_newReplaceAllFromIndexUtf16LUtf16L(newStr, str, from, fromLength, to, toLength, 0);
}

void rtl_uString_newReplaceAllFromIndexUtf16LUtf16L(
    rtl_uString ** newStr, rtl_uString * str, sal_Unicode const * from,
    sal_Int32 fromLength, sal_Unicode const * to, sal_Int32 toLength, sal_Int32 fromIndex)
    SAL_THROW_EXTERN_C()
{
    assert(toLength >= 0);
    assert(fromIndex >= 0 && fromIndex <= str->length);
    rtl_uString_assign(newStr, str);
    for (sal_Int32 i = fromIndex;; i += toLength) {
        rtl_uString_newReplaceFirstUtf16LUtf16L(
            newStr, *newStr, from, fromLength, to, toLength, &i);
        if (i == -1 || *newStr == nullptr) {
            break;
        }
    }
}

sal_Int32 SAL_CALL rtl_ustr_getLength(const sal_Unicode* pStr) SAL_THROW_EXTERN_C()
{
    return rtl::str::getLength(pStr);
}

sal_Int32 SAL_CALL rtl_ustr_compare(const sal_Unicode* pStr1, const sal_Unicode* pStr2)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::compare(pStr1, pStr2);
}

sal_Int32 SAL_CALL rtl_ustr_compare_WithLength(const sal_Unicode* pStr1, sal_Int32 nStr1Len,
                                               const sal_Unicode* pStr2, sal_Int32 nStr2Len)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::compare_WithLength(pStr1, nStr1Len, pStr2, nStr2Len);
}

sal_Int32 SAL_CALL rtl_ustr_shortenedCompare_WithLength(
    const sal_Unicode* pStr1, sal_Int32 nStr1Len, const sal_Unicode* pStr2, sal_Int32 nStr2Len,
    sal_Int32 nShortenedLength) SAL_THROW_EXTERN_C()
{
    return rtl::str::shortenedCompare_WithLength(pStr1, nStr1Len, pStr2, nStr2Len, nShortenedLength);
}

sal_Int32 SAL_CALL rtl_ustr_reverseCompare_WithLength(const sal_Unicode* pStr1, sal_Int32 nStr1Len,
                                                      const sal_Unicode* pStr2, sal_Int32 nStr2Len)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::reverseCompare_WithLength(pStr1, nStr1Len, pStr2, nStr2Len);
}

sal_Int32 SAL_CALL rtl_ustr_compareIgnoreAsciiCase(const sal_Unicode* pStr1,
                                                   const sal_Unicode* pStr2) SAL_THROW_EXTERN_C()
{
    return rtl::str::compareIgnoreAsciiCase(pStr1, pStr2);
}

sal_Int32 SAL_CALL rtl_ustr_compareIgnoreAsciiCase_WithLength(const sal_Unicode* pStr1,
                                                              sal_Int32 nStr1Len,
                                                              const sal_Unicode* pStr2,
                                                              sal_Int32 nStr2Len)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::compareIgnoreAsciiCase_WithLength(pStr1, nStr1Len, pStr2, nStr2Len);
}

sal_Int32 SAL_CALL rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength(
    const sal_Unicode* pStr1, sal_Int32 nStr1Len, const sal_Unicode* pStr2, sal_Int32 nStr2Len,
    sal_Int32 nShortenedLength) SAL_THROW_EXTERN_C()
{
    return rtl::str::shortenedCompareIgnoreAsciiCase_WithLength(pStr1, nStr1Len, pStr2, nStr2Len,
                                                              nShortenedLength);
}

sal_Int32 SAL_CALL rtl_ustr_hashCode(const sal_Unicode* pStr) SAL_THROW_EXTERN_C()
{
    return rtl::str::hashCode(pStr);
}

sal_Int32 SAL_CALL rtl_ustr_hashCode_WithLength(const sal_Unicode* pStr, sal_Int32 nLen)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::hashCode_WithLength(pStr, nLen);
}

sal_Int32 SAL_CALL rtl_ustr_indexOfChar(const sal_Unicode* pStr, sal_Unicode c) SAL_THROW_EXTERN_C()
{
    return rtl::str::indexOfChar(pStr, c);
}

sal_Int32 SAL_CALL rtl_ustr_indexOfChar_WithLength(const sal_Unicode* pStr, sal_Int32 nLen,
                                                   sal_Unicode c) SAL_THROW_EXTERN_C()
{
    return rtl::str::indexOfChar_WithLength(pStr, nLen, c);
}

sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar(const sal_Unicode* pStr, sal_Unicode c)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::lastIndexOfChar(pStr, c);
}

sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar_WithLength(const sal_Unicode* pStr, sal_Int32 nLen,
                                                       sal_Unicode c) SAL_THROW_EXTERN_C()
{
    return rtl::str::lastIndexOfChar_WithLength(pStr, nLen, c);
}

sal_Int32 SAL_CALL rtl_ustr_indexOfStr(const sal_Unicode* pStr, const sal_Unicode* pSubStr)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::indexOfStr(pStr, pSubStr);
}

sal_Int32 SAL_CALL rtl_ustr_indexOfStr_WithLength(const sal_Unicode* pStr, sal_Int32 nStrLen,
                                                  const sal_Unicode* pSubStr, sal_Int32 nSubLen)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::indexOfStr_WithLength(pStr, nStrLen, pSubStr, nSubLen);
}

sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr(const sal_Unicode* pStr, const sal_Unicode* pSubStr)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::lastIndexOfStr(pStr, pSubStr);
}

sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr_WithLength(const sal_Unicode* pStr, sal_Int32 nStrLen,
                                                      const sal_Unicode* pSubStr, sal_Int32 nSubLen)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::lastIndexOfStr_WithLength(pStr, nStrLen, pSubStr, nSubLen);
}

void SAL_CALL rtl_ustr_replaceChar(sal_Unicode* pStr, sal_Unicode cOld, sal_Unicode cNew)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::replaceChar(pStr, cOld, cNew);
}

void SAL_CALL rtl_ustr_replaceChar_WithLength(sal_Unicode* pStr, sal_Int32 nLen, sal_Unicode cOld,
                                              sal_Unicode cNew) SAL_THROW_EXTERN_C()
{
    return rtl::str::replaceChar_WithLength(pStr, nLen, cOld, cNew);
}

void SAL_CALL rtl_ustr_toAsciiLowerCase(sal_Unicode* pStr) SAL_THROW_EXTERN_C()
{
    return rtl::str::toAsciiLowerCase(pStr);
}

void SAL_CALL rtl_ustr_toAsciiLowerCase_WithLength(sal_Unicode* pStr, sal_Int32 nLen)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::toAsciiLowerCase_WithLength(pStr, nLen);
}

void SAL_CALL rtl_ustr_toAsciiUpperCase(sal_Unicode* pStr) SAL_THROW_EXTERN_C()
{
    return rtl::str::toAsciiUpperCase(pStr);
}

void SAL_CALL rtl_ustr_toAsciiUpperCase_WithLength(sal_Unicode* pStr, sal_Int32 nLen)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::toAsciiUpperCase_WithLength(pStr, nLen);
}

sal_Int32 SAL_CALL rtl_ustr_trim(sal_Unicode* pStr) SAL_THROW_EXTERN_C()
{
    return rtl::str::trim(pStr);
}

sal_Int32 SAL_CALL rtl_ustr_trim_WithLength(sal_Unicode* pStr, sal_Int32 nLen) SAL_THROW_EXTERN_C()
{
    return rtl::str::trim_WithLength(pStr, nLen);
}

sal_Int32 SAL_CALL rtl_ustr_valueOfBoolean(sal_Unicode* pStr, sal_Bool b) SAL_THROW_EXTERN_C()
{
    return rtl::str::valueOfBoolean(pStr, b);
}

sal_Int32 SAL_CALL rtl_ustr_valueOfChar(sal_Unicode* pStr, sal_Unicode c) SAL_THROW_EXTERN_C()
{
    return rtl::str::valueOfChar(pStr, c);
}

sal_Int32 SAL_CALL rtl_ustr_valueOfInt32(sal_Unicode* pStr, sal_Int32 n, sal_Int16 nRadix)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::valueOfInt32(pStr, n, nRadix);
}

sal_Int32 SAL_CALL rtl_ustr_valueOfInt64(sal_Unicode* pStr, sal_Int64 n, sal_Int16 nRadix)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::valueOfInt64(pStr, n, nRadix);
}

sal_Int32 SAL_CALL rtl_ustr_valueOfUInt64(sal_Unicode* pStr, sal_uInt64 n, sal_Int16 nRadix)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::valueOfUInt64(pStr, n, nRadix);
}

sal_Bool SAL_CALL rtl_ustr_toBoolean(const sal_Unicode* pStr) SAL_THROW_EXTERN_C()
{
    return rtl::str::toBoolean(pStr);
}

sal_Int32 SAL_CALL rtl_ustr_toInt32(const sal_Unicode* pStr, sal_Int16 nRadix) SAL_THROW_EXTERN_C()
{
    return rtl::str::toInt32(pStr, nRadix);
}

sal_Int64 SAL_CALL rtl_ustr_toInt64(const sal_Unicode* pStr, sal_Int16 nRadix) SAL_THROW_EXTERN_C()
{
    return rtl::str::toInt64(pStr, nRadix);
}

sal_Int64 SAL_CALL rtl_ustr_toInt64_WithLength(const sal_Unicode* pStr, sal_Int16 nRadix,
                                               sal_Int32 nStrLength) SAL_THROW_EXTERN_C()
{
    return rtl::str::toInt64_WithLength(pStr, nRadix, nStrLength);
}

sal_uInt32 SAL_CALL rtl_ustr_toUInt32(const sal_Unicode* pStr, sal_Int16 nRadix)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::toUInt32(pStr, nRadix);
}

sal_uInt64 SAL_CALL rtl_ustr_toUInt64(const sal_Unicode* pStr, sal_Int16 nRadix)
    SAL_THROW_EXTERN_C()
{
    return rtl::str::toUInt64(pStr, nRadix);
}

rtl_uString* rtl_uString_ImplAlloc(sal_Int32 nLen)
{
    return rtl::str::Alloc<rtl_uString>(nLen);
}

void SAL_CALL rtl_uString_acquire(rtl_uString* pThis) SAL_THROW_EXTERN_C()
{
    return rtl::str::acquire(pThis);
}

void SAL_CALL rtl_uString_release(rtl_uString* pThis) SAL_THROW_EXTERN_C()
{
    return rtl::str::release(pThis);
}

void SAL_CALL rtl_uString_new(rtl_uString** ppThis) SAL_THROW_EXTERN_C()
{
    return rtl::str::new_(ppThis);
}

rtl_uString* SAL_CALL rtl_uString_alloc(sal_Int32 nLen) SAL_THROW_EXTERN_C()
{
    return rtl::str::alloc<rtl_uString>(nLen);
}

void SAL_CALL rtl_uString_new_WithLength(rtl_uString** ppThis, sal_Int32 nLen) SAL_THROW_EXTERN_C()
{
    rtl::str::new_WithLength(ppThis, nLen);
}

void SAL_CALL rtl_uString_newFromString(rtl_uString** ppThis, const rtl_uString* pStr)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newFromString(ppThis, pStr);
}

void SAL_CALL rtl_uString_newFromStr(rtl_uString** ppThis, const sal_Unicode* pCharStr)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newFromStr(ppThis, pCharStr);
}

void SAL_CALL rtl_uString_newFromStr_WithLength(rtl_uString** ppThis, const sal_Unicode* pCharStr,
                                                sal_Int32 nLen) SAL_THROW_EXTERN_C()
{
    rtl::str::newFromStr_WithLength(ppThis, pCharStr, nLen);
}

void SAL_CALL rtl_uString_newFromSubString(rtl_uString** ppThis, const rtl_uString* pFrom,
                                           sal_Int32 beginIndex, sal_Int32 count)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newFromSubString(ppThis, pFrom, beginIndex, count);
}

// Used when creating from string literals.
void SAL_CALL rtl_uString_newFromLiteral(rtl_uString** ppThis, const char* pCharStr, sal_Int32 nLen,
                                         sal_Int32 allocExtra) SAL_THROW_EXTERN_C()
{
    rtl::str::newFromLiteral(ppThis, pCharStr, nLen, allocExtra);
}

void SAL_CALL rtl_uString_assign(rtl_uString** ppThis, rtl_uString* pStr) SAL_THROW_EXTERN_C()
{
    rtl::str::assign(ppThis, pStr);
}

sal_Int32 SAL_CALL rtl_uString_getLength(const rtl_uString* pThis) SAL_THROW_EXTERN_C()
{
    return rtl::str::getLength(pThis);
}

sal_Unicode* SAL_CALL rtl_uString_getStr(rtl_uString* pThis) SAL_THROW_EXTERN_C()
{
    return rtl::str::getStr(pThis);
}

void SAL_CALL rtl_uString_newConcat(rtl_uString** ppThis, rtl_uString* pLeft, rtl_uString* pRight)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newConcat(ppThis, pLeft, pRight);
}

void SAL_CALL rtl_uString_ensureCapacity(rtl_uString** ppThis, sal_Int32 size) SAL_THROW_EXTERN_C()
{
    rtl::str::ensureCapacity(ppThis, size);
}

void SAL_CALL rtl_uString_newReplaceStrAt(rtl_uString** ppThis, rtl_uString* pStr, sal_Int32 nIndex,
                                          sal_Int32 nCount, rtl_uString* pNewSubStr)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newReplaceStrAt(ppThis, pStr, nIndex, nCount, pNewSubStr);
}

void SAL_CALL rtl_uString_newReplace(rtl_uString** ppThis, rtl_uString* pStr, sal_Unicode cOld,
                                     sal_Unicode cNew) SAL_THROW_EXTERN_C()
{
    rtl::str::newReplace(ppThis, pStr, cOld, cNew);
}

void SAL_CALL rtl_uString_newToAsciiLowerCase(rtl_uString** ppThis, rtl_uString* pStr)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newToAsciiLowerCase(ppThis, pStr);
}

void SAL_CALL rtl_uString_newToAsciiUpperCase(rtl_uString** ppThis, rtl_uString* pStr)
    SAL_THROW_EXTERN_C()
{
    rtl::str::newToAsciiUpperCase(ppThis, pStr);
}

void SAL_CALL rtl_uString_newTrim(rtl_uString** ppThis, rtl_uString* pStr) SAL_THROW_EXTERN_C()
{
    rtl::str::newTrim(ppThis, pStr);
}

sal_Int32 SAL_CALL rtl_uString_getToken(rtl_uString** ppThis, rtl_uString* pStr, sal_Int32 nToken,
                                        sal_Unicode cTok, sal_Int32 nIndex) SAL_THROW_EXTERN_C()
{
    return rtl::str::getToken(ppThis, pStr, nToken, cTok, nIndex);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
