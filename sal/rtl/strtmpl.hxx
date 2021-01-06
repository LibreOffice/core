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

/* ======================================================================= */
/* Internal C-String help functions which could be used without the        */
/* String-Class                                                            */
/* ======================================================================= */

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <type_traits>

#include <cstring>
#include <wchar.h>
#include <sal/log.hxx>
#include <rtl/character.hxx>

namespace
{
template <typename CharType>
void rtl_str_ImplCopy(CharType* _pDest, const CharType* _pSrc, sal_Int32 _nCount)
{
    // take advantage of builtin optimizations
    memcpy(_pDest, _pSrc, _nCount * sizeof(CharType));
}

/* ======================================================================= */
/* C-String functions which could be used without the String-Class         */
/* ======================================================================= */

template <typename T> sal_Int32 str_imp_getLength(const T* pStr)
{
    assert(pStr);
    if constexpr (std::is_class_v<T>)
    {
        return pStr->length;
    }
    else if constexpr (sizeof(T) == sizeof(char))
    {
        // take advantage of builtin optimizations
        return strlen(pStr);
    }
    else if constexpr (sizeof(T) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        return wcslen(reinterpret_cast<wchar_t const*>(pStr));
    }
    else
    {
        const T* pTempStr = pStr;
        while (*pTempStr)
            pTempStr++;
        return pTempStr - pStr;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_compare(const CharType* pStr1, const CharType* pStr2)
{
    assert(pStr1);
    assert(pStr2);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        return strcmp(pStr1, pStr2);
    }
    else if constexpr (sizeof(CharType) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        return wcscmp(reinterpret_cast<wchar_t const*>(pStr1),
                      reinterpret_cast<wchar_t const*>(pStr2));
    }
    else
    {
        sal_Int32 nRet;
        for (;;)
        {
            nRet = static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr1))
                   - static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr2));
            if (!(nRet == 0 && *pStr2))
                break;
            pStr1++;
            pStr2++;
        }

        return nRet;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_compare_WithLength(const CharType* pStr1, sal_Int32 nStr1Len,
                                     const CharType* pStr2, sal_Int32 nStr2Len)
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        sal_Int32 nMin = std::min(nStr1Len, nStr2Len);
        sal_Int32 nRet = memcmp(pStr1, pStr2, nMin);
        return nRet == 0 ? nStr1Len - nStr2Len : nRet;
    }
    else if constexpr (sizeof(CharType) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        sal_Int32 nMin = std::min(nStr1Len, nStr2Len);
        sal_Int32 nRet = wmemcmp(reinterpret_cast<wchar_t const*>(pStr1),
                                 reinterpret_cast<wchar_t const*>(pStr2), nMin);
        return nRet == 0 ? nStr1Len - nStr2Len : nRet;
    }
    else
    {
        sal_Int32 nRet = nStr1Len - nStr2Len;
        int nCount = (nRet <= 0) ? nStr1Len : nStr2Len;

        while (--nCount >= 0)
        {
            if (*pStr1 != *pStr2)
            {
                break;
            }
            ++pStr1;
            ++pStr2;
        }

        if (nCount >= 0)
            nRet = static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr1))
                   - static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr2));

        return nRet;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_shortenedCompare_WithLength(const CharType* pStr1, sal_Int32 nStr1Len,
                                              const CharType* pStr2, sal_Int32 nStr2Len,
                                              sal_Int32 nShortenedLength)
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    assert(nShortenedLength >= 0);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        sal_Int32 nMin = std::min({ nStr1Len, nStr2Len, nShortenedLength });
        sal_Int32 nRet = memcmp(pStr1, pStr2, nMin);
        if (nRet == 0 && nShortenedLength > std::min(nStr1Len, nStr2Len))
            return nStr1Len - nStr2Len;
        return nRet;
    }
    else if constexpr (sizeof(CharType) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        sal_Int32 nMin = std::min({ nStr1Len, nStr2Len, nShortenedLength });
        sal_Int32 nRet = wmemcmp(reinterpret_cast<wchar_t const*>(pStr1),
                                 reinterpret_cast<wchar_t const*>(pStr2), nMin);
        if (nRet == 0 && nShortenedLength > std::min(nStr1Len, nStr2Len))
            return nStr1Len - nStr2Len;
        return nRet;
    }
    else
    {
        const CharType* pStr1End = pStr1 + nStr1Len;
        const CharType* pStr2End = pStr2 + nStr2Len;
        while ((nShortenedLength > 0) && (pStr1 < pStr1End) && (pStr2 < pStr2End))
        {
            sal_Int32 nRet = static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr1))
                             - static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr2));
            if (nRet)
                return nRet;

            nShortenedLength--;
            pStr1++;
            pStr2++;
        }

        if (nShortenedLength <= 0)
            return 0;
        return nStr1Len - nStr2Len;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_reverseCompare_WithLength(const CharType* pStr1, sal_Int32 nStr1Len,
                                            const CharType* pStr2, sal_Int32 nStr2Len)

{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    const CharType* pStr1Run = pStr1 + nStr1Len;
    const CharType* pStr2Run = pStr2 + nStr2Len;
    while ((pStr1 < pStr1Run) && (pStr2 < pStr2Run))
    {
        pStr1Run--;
        pStr2Run--;
        sal_Int32 nRet = static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr1Run))
                         - static_cast<sal_Int32>(std::make_unsigned_t<CharType>(*pStr2Run));
        if (nRet)
            return nRet;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_compareIgnoreAsciiCase(const CharType* pStr1, const CharType* pStr2)

{
    assert(pStr1);
    assert(pStr2);
    sal_uInt32 c1;
    do
    {
        c1 = std::make_unsigned_t<CharType>(*pStr1);
        sal_Int32 nRet = rtl::compareIgnoreAsciiCase(c1, std::make_unsigned_t<CharType>(*pStr2));
        if (nRet != 0)
            return nRet;

        pStr1++;
        pStr2++;
    } while (c1);

    return 0;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_compareIgnoreAsciiCase_WithLength(const CharType* pStr1, sal_Int32 nStr1Len,
                                                    const CharType* pStr2, sal_Int32 nStr2Len)

{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    const CharType* pStr1End = pStr1 + nStr1Len;
    const CharType* pStr2End = pStr2 + nStr2Len;
    while ((pStr1 < pStr1End) && (pStr2 < pStr2End))
    {
        sal_Int32 nRet = rtl::compareIgnoreAsciiCase(std::make_unsigned_t<CharType>(*pStr1),
                                                     std::make_unsigned_t<CharType>(*pStr2));
        if (nRet != 0)
            return nRet;

        pStr1++;
        pStr2++;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32
str_imp_shortenedCompareIgnoreAsciiCase_WithLength(const CharType* pStr1, sal_Int32 nStr1Len,
                                                   const CharType* pStr2, sal_Int32 nStr2Len,
                                                   sal_Int32 nShortenedLength)
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    assert(nShortenedLength >= 0);
    const CharType* pStr1End = pStr1 + nStr1Len;
    const CharType* pStr2End = pStr2 + nStr2Len;
    while ((nShortenedLength > 0) && (pStr1 < pStr1End) && (pStr2 < pStr2End))
    {
        sal_Int32 nRet = rtl::compareIgnoreAsciiCase(std::make_unsigned_t<CharType>(*pStr1),
                                                     std::make_unsigned_t<CharType>(*pStr2));
        if (nRet != 0)
            return nRet;

        nShortenedLength--;
        pStr1++;
        pStr2++;
    }

    if (nShortenedLength <= 0)
        return 0;
    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_hashCode_WithLength(const CharType* pStr, sal_Int32 nLen)
{
    assert(nLen >= 0);
    sal_uInt32 h = static_cast<sal_uInt32>(nLen);
    while (nLen > 0)
    {
        h = (h * 37U) + std::make_unsigned_t<CharType>(*pStr);
        pStr++;
        nLen--;
    }
    return static_cast<sal_Int32>(h);
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_hashCode(const CharType* pStr)
{
    return str_imp_hashCode_WithLength(pStr, str_imp_getLength(pStr));
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_indexOfChar(const CharType* pStr, CharType c)
{
    assert(pStr);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        const CharType* p = strchr(pStr, c);
        return p ? p - pStr : -1;
    }
    else if constexpr (sizeof(CharType) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        wchar_t const* p = wcschr(reinterpret_cast<wchar_t const*>(pStr), static_cast<wchar_t>(c));
        return p ? p - reinterpret_cast<wchar_t const*>(pStr) : -1;
    }
    else
    {
        const CharType* pTempStr = pStr;
        while (*pTempStr)
        {
            if (*pTempStr == c)
                return pTempStr - pStr;

            pTempStr++;
        }

        return -1;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_indexOfChar_WithLength(const CharType* pStr, sal_Int32 nLen, CharType c)

{
    //    assert(nLen >= 0);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        CharType* p = static_cast<CharType*>(std::memchr(const_cast<CharType*>(pStr), c, nLen));
        return p ? p - pStr : -1;
    }
    else
    {
        const CharType* pTempStr = pStr;
        while (nLen > 0)
        {
            if (*pTempStr == c)
                return pTempStr - pStr;

            pTempStr++;
            nLen--;
        }

        return -1;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_lastIndexOfChar_WithLength(const CharType* pStr, sal_Int32 nLen, CharType c)

{
    assert(nLen >= 0);
    pStr += nLen;
    while (nLen > 0)
    {
        nLen--;
        pStr--;

        if (*pStr == c)
            return nLen;
    }

    return -1;
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_lastIndexOfChar(const CharType* pStr, CharType c)
{
    assert(pStr);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        const CharType* p = strrchr(pStr, c);
        return p ? p - pStr : -1;
    }
    else if constexpr (sizeof(CharType) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        wchar_t const* p = wcsrchr(reinterpret_cast<wchar_t const*>(pStr), static_cast<wchar_t>(c));
        return p ? p - reinterpret_cast<wchar_t const*>(pStr) : -1;
    }
    else
    {
        return str_imp_lastIndexOfChar_WithLength(pStr, str_imp_getLength(pStr), c);
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_indexOfStr_WithLength(const CharType* pStr, sal_Int32 nStrLen,
                                        const CharType* pSubStr, sal_Int32 nSubLen)

{
    assert(nStrLen >= 0);
    assert(nSubLen >= 0);
    /* faster search for a single character */
    if (nSubLen < 2)
    {
        /* an empty SubString is always not findable */
        if (nSubLen == 1)
        {
            CharType c = *pSubStr;
            const CharType* pTempStr = pStr;
            while (nStrLen > 0)
            {
                if (*pTempStr == c)
                    return pTempStr - pStr;

                pTempStr++;
                nStrLen--;
            }
        }
    }
    else
    {
        const CharType* pTempStr = pStr;
        while (nStrLen > 0)
        {
            if (*pTempStr == *pSubStr)
            {
                /* Compare SubString */
                if (nSubLen <= nStrLen)
                {
                    const CharType* pTempStr1 = pTempStr;
                    const CharType* pTempStr2 = pSubStr;
                    sal_Int32 nTempLen = nSubLen;
                    while (nTempLen)
                    {
                        if (*pTempStr1 != *pTempStr2)
                            break;

                        pTempStr1++;
                        pTempStr2++;
                        nTempLen--;
                    }

                    if (!nTempLen)
                        return pTempStr - pStr;
                }
                else
                    break;
            }

            nStrLen--;
            pTempStr++;
        }
    }

    return -1;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_indexOfStr(const CharType* pStr, const CharType* pSubStr)
{
    assert(pStr);
    assert(pSubStr);
    if constexpr (sizeof(CharType) == sizeof(char))
    {
        // take advantage of builtin optimizations
        const CharType* p = strstr(pStr, pSubStr);
        return p ? p - pStr : -1;
    }
    else if constexpr (sizeof(CharType) == sizeof(wchar_t))
    {
        // take advantage of builtin optimizations
        wchar_t const* p = wcsstr(reinterpret_cast<wchar_t const*>(pStr),
                                  reinterpret_cast<wchar_t const*>(pSubStr));
        return p ? p - reinterpret_cast<wchar_t const*>(pStr) : -1;
    }
    else
    {
        return str_imp_indexOfStr_WithLength(pStr, str_imp_getLength(pStr), pSubStr,
                                             str_imp_getLength(pSubStr));
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_lastIndexOfStr_WithLength(const CharType* pStr, sal_Int32 nStrLen,
                                            const CharType* pSubStr, sal_Int32 nSubLen)

{
    assert(nStrLen >= 0);
    assert(nSubLen >= 0);
    /* faster search for a single character */
    if (nSubLen < 2)
    {
        /* an empty SubString is always not findable */
        if (nSubLen == 1)
        {
            CharType c = *pSubStr;
            pStr += nStrLen;
            while (nStrLen > 0)
            {
                nStrLen--;
                pStr--;

                if (*pStr == c)
                    return nStrLen;
            }
        }
    }
    else
    {
        pStr += nStrLen;
        nStrLen -= nSubLen;
        pStr -= nSubLen;
        while (nStrLen >= 0)
        {
            const CharType* pTempStr1 = pStr;
            const CharType* pTempStr2 = pSubStr;
            sal_Int32 nTempLen = nSubLen;
            while (nTempLen)
            {
                if (*pTempStr1 != *pTempStr2)
                    break;

                pTempStr1++;
                pTempStr2++;
                nTempLen--;
            }

            if (!nTempLen)
                return nStrLen;

            nStrLen--;
            pStr--;
        }
    }

    return -1;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_lastIndexOfStr(const CharType* pStr, const CharType* pSubStr)
{
    return str_imp_lastIndexOfStr_WithLength(pStr, str_imp_getLength(pStr), pSubStr,
                                             str_imp_getLength(pSubStr));
}

/* ----------------------------------------------------------------------- */

template <typename CharType> void str_imp_replaceChar(CharType* pStr, CharType cOld, CharType cNew)
{
    assert(pStr);
    while (*pStr)
    {
        if (*pStr == cOld)
            *pStr = cNew;

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
void str_imp_replaceChar_WithLength(CharType* pStr, sal_Int32 nLen, CharType cOld, CharType cNew)

{
    assert(nLen >= 0);
    while (nLen > 0)
    {
        if (*pStr == cOld)
            *pStr = cNew;

        pStr++;
        nLen--;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType> void str_imp_toAsciiLowerCase(CharType* pStr)
{
    assert(pStr);
    while (*pStr)
    {
        *pStr = rtl::toAsciiLowerCase(std::make_unsigned_t<CharType>(*pStr));

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
void str_imp_toAsciiLowerCase_WithLength(CharType* pStr, sal_Int32 nLen)
{
    assert(nLen >= 0);
    while (nLen > 0)
    {
        *pStr = rtl::toAsciiLowerCase(std::make_unsigned_t<CharType>(*pStr));

        pStr++;
        nLen--;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType> void str_imp_toAsciiUpperCase(CharType* pStr)
{
    assert(pStr);
    while (*pStr)
    {
        *pStr = rtl::toAsciiUpperCase(std::make_unsigned_t<CharType>(*pStr));

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
void str_imp_toAsciiUpperCase_WithLength(CharType* pStr, sal_Int32 nLen)
{
    assert(nLen >= 0);
    while (nLen > 0)
    {
        *pStr = rtl::toAsciiUpperCase(std::make_unsigned_t<CharType>(*pStr));

        pStr++;
        nLen--;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_trim_WithLength(CharType* pStr, sal_Int32 nLen)
{
    assert(nLen >= 0);
    sal_Int32 nPreSpaces = 0;
    sal_Int32 nPostSpaces = 0;
    sal_Int32 nIndex = nLen - 1;

    while ((nPreSpaces < nLen)
           && rtl_ImplIsWhitespace(std::make_unsigned_t<CharType>(*(pStr + nPreSpaces))))
        nPreSpaces++;

    while ((nIndex > nPreSpaces)
           && rtl_ImplIsWhitespace(std::make_unsigned_t<CharType>(*(pStr + nIndex))))
    {
        nPostSpaces++;
        nIndex--;
    }

    if (nPostSpaces)
    {
        nLen -= nPostSpaces;
        *(pStr + nLen) = 0;
    }

    if (nPreSpaces)
    {
        nLen -= nPreSpaces;
        memmove(pStr, pStr + nPreSpaces, nLen * sizeof(CharType));
        pStr += nLen;
        *pStr = 0;
    }

    return nLen;
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_trim(CharType* pStr)
{
    return str_imp_trim_WithLength(pStr, str_imp_getLength(pStr));
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_valueOfBoolean(CharType* pStr, sal_Bool b)
{
    assert(pStr);
    if (b)
    {
        *pStr = 't';
        pStr++;
        *pStr = 'r';
        pStr++;
        *pStr = 'u';
        pStr++;
        *pStr = 'e';
        pStr++;
        *pStr = 0;
        return 4;
    }
    else
    {
        *pStr = 'f';
        pStr++;
        *pStr = 'a';
        pStr++;
        *pStr = 'l';
        pStr++;
        *pStr = 's';
        pStr++;
        *pStr = 'e';
        pStr++;
        *pStr = 0;
        return 5;
    }
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Int32 str_imp_valueOfChar(CharType* pStr, CharType c)
{
    assert(pStr);
    *pStr++ = c;
    *pStr = 0;
    return 1;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_valueOfInt32(CharType* pStr, sal_Int32 n, sal_Int16 nRadix)
{
    assert(pStr);
    assert(nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX);
    char aBuf[RTL_STR_MAX_VALUEOFINT32];
    char* pBuf = aBuf;
    sal_Int32 nLen = 0;
    sal_uInt32 nValue;

    /* Radix must be valid */
    if ((nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX))
        nRadix = 10;

    /* is value negative */
    if (n < 0)
    {
        *pStr = '-';
        pStr++;
        nLen++;
        nValue = n == SAL_MIN_INT32 ? static_cast<sal_uInt32>(n) : -n;
    }
    else
        nValue = n;

    /* create a recursive buffer with all values, except the last one */
    do
    {
        char nDigit = static_cast<char>(nValue % nRadix);
        nValue /= nRadix;
        if (nDigit > 9)
            *pBuf = (nDigit - 10) + 'a';
        else
            *pBuf = (nDigit + '0');
        pBuf++;
    } while (nValue > 0);

    /* copy the values in the right direction into the destination buffer */
    do
    {
        pBuf--;
        *pStr = *pBuf;
        pStr++;
        nLen++;
    } while (pBuf != aBuf);
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_valueOfInt64(CharType* pStr, sal_Int64 n, sal_Int16 nRadix)
{
    assert(pStr);
    assert(nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX);
    char aBuf[RTL_STR_MAX_VALUEOFINT64];
    char* pBuf = aBuf;
    sal_Int32 nLen = 0;
    sal_uInt64 nValue;

    /* Radix must be valid */
    if ((nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX))
        nRadix = 10;

    /* is value negative */
    if (n < 0)
    {
        *pStr = '-';
        pStr++;
        nLen++;
        nValue = n == SAL_MIN_INT64 ? static_cast<sal_uInt64>(n) : -n;
    }
    else
        nValue = n;

    /* create a recursive buffer with all values, except the last one */
    do
    {
        char nDigit = static_cast<char>(nValue % nRadix);
        nValue /= nRadix;
        if (nDigit > 9)
            *pBuf = (nDigit - 10) + 'a';
        else
            *pBuf = (nDigit + '0');
        pBuf++;
    } while (nValue > 0);

    /* copy the values in the right direction into the destination buffer */
    do
    {
        pBuf--;
        *pStr = *pBuf;
        pStr++;
        nLen++;
    } while (pBuf != aBuf);
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

template <typename CharType>
sal_Int32 str_imp_valueOfUInt64(CharType* pStr, sal_uInt64 n, sal_Int16 nRadix)
{
    assert(pStr);
    assert(nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX);
    char aBuf[RTL_STR_MAX_VALUEOFUINT64];
    char* pBuf = aBuf;
    sal_Int32 nLen = 0;

    /* Radix must be valid */
    if ((nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX))
        nRadix = 10;

    sal_uInt64 nValue = n;

    /* create a recursive buffer with all values, except the last one */
    do
    {
        char nDigit = static_cast<char>(nValue % nRadix);
        nValue /= nRadix;
        if (nDigit > 9)
            *pBuf = (nDigit - 10) + 'a';
        else
            *pBuf = (nDigit + '0');
        pBuf++;
    } while (nValue > 0);

    /* copy the values in the right direction into the destination buffer */
    do
    {
        pBuf--;
        *pStr = *pBuf;
        pStr++;
        nLen++;
    } while (pBuf != aBuf);
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

template <typename CharType> sal_Bool str_imp_toBoolean(const CharType* pStr)
{
    assert(pStr);
    if (*pStr == '1')
        return true;

    if ((*pStr == 'T') || (*pStr == 't'))
    {
        pStr++;
        if ((*pStr == 'R') || (*pStr == 'r'))
        {
            pStr++;
            if ((*pStr == 'U') || (*pStr == 'u'))
            {
                pStr++;
                if ((*pStr == 'E') || (*pStr == 'e'))
                    return true;
            }
        }
    }

    return false;
}

/* ----------------------------------------------------------------------- */
template <typename T, typename U, typename CharType>
T str_imp_toInt_WithLength(const CharType* pStr, sal_Int16 nRadix, sal_Int32 nStrLength)
{
    static_assert(std::numeric_limits<T>::is_signed, "is signed");
    assert(nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX);
    assert(nStrLength >= 0);
    bool bNeg;
    U n = 0;
    const CharType* pEnd = pStr + nStrLength;

    if ((nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX))
        nRadix = 10;

    /* Skip whitespaces */
    while (pStr != pEnd && rtl_ImplIsWhitespace(std::make_unsigned_t<CharType>(*pStr)))
        pStr++;

    if (*pStr == '-')
    {
        bNeg = true;
        pStr++;
    }
    else
    {
        if (*pStr == '+')
            pStr++;
        bNeg = false;
    }

    T nDiv;
    sal_Int16 nMod;
    if (bNeg)
    {
        nDiv = std::numeric_limits<T>::min() / nRadix;
        nMod = std::numeric_limits<T>::min() % nRadix;
        // Cater for C++03 implementations that round the quotient down
        // instead of truncating towards zero as mandated by C++11:
        if (nMod > 0)
        {
            --nDiv;
            nMod -= nRadix;
        }
        nDiv = -nDiv;
        nMod = -nMod;
    }
    else
    {
        nDiv = std::numeric_limits<T>::max() / nRadix;
        nMod = std::numeric_limits<T>::max() % nRadix;
    }

    while (pStr != pEnd)
    {
        sal_Int16 nDigit = rtl_ImplGetDigit(std::make_unsigned_t<CharType>(*pStr), nRadix);
        if (nDigit < 0)
            break;
        assert(nDiv > 0);
        if (static_cast<U>(nMod < nDigit ? nDiv - 1 : nDiv) < n)
            return 0;

        n *= nRadix;
        n += nDigit;

        pStr++;
    }

    if (bNeg)
        return n == static_cast<U>(std::numeric_limits<T>::min()) ? std::numeric_limits<T>::min()
                                                                  : -static_cast<T>(n);
    else
        return static_cast<T>(n);
}

template <typename CharType> sal_Int32 str_imp_toInt32(const CharType* pStr, sal_Int16 nRadix)
{
    assert(pStr);
    return str_imp_toInt_WithLength<sal_Int32, sal_uInt32>(pStr, nRadix, str_imp_getLength(pStr));
}

template <typename CharType> sal_Int64 str_imp_toInt64(const CharType* pStr, sal_Int16 nRadix)
{
    assert(pStr);
    return str_imp_toInt_WithLength<sal_Int64, sal_uInt64>(pStr, nRadix, str_imp_getLength(pStr));
}

template <typename CharType>
sal_Int64 str_imp_toInt64_WithLength(const CharType* pStr, sal_Int16 nRadix, sal_Int32 nStrLength)

{
    assert(pStr);
    return str_imp_toInt_WithLength<sal_Int64, sal_uInt64>(pStr, nRadix, nStrLength);
}

/* ----------------------------------------------------------------------- */
template <typename T, typename CharType> T str_imp_toUInt(const CharType* pStr, sal_Int16 nRadix)
{
    static_assert(!std::numeric_limits<T>::is_signed, "is not signed");
    assert(nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX);
    T n = 0;

    if ((nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX))
        nRadix = 10;

    /* Skip whitespaces */
    while (*pStr && rtl_ImplIsWhitespace(std::make_unsigned_t<CharType>(*pStr)))
        ++pStr;

    // skip optional explicit sign
    if (*pStr == '+')
        ++pStr;

    T nDiv = std::numeric_limits<T>::max() / nRadix;
    sal_Int16 nMod = std::numeric_limits<T>::max() % nRadix;
    while (*pStr)
    {
        sal_Int16 nDigit = rtl_ImplGetDigit(std::make_unsigned_t<CharType>(*pStr), nRadix);
        if (nDigit < 0)
            break;
        if ((nMod < nDigit ? nDiv - 1 : nDiv) < n)
            return 0;

        n *= nRadix;
        n += nDigit;

        ++pStr;
    }

    return n;
}

template <typename CharType> sal_uInt32 str_imp_toUInt32(const CharType* pStr, sal_Int16 nRadix)
{
    assert(pStr);
    return str_imp_toUInt<sal_uInt32>(pStr, nRadix);
}

template <typename CharType> sal_uInt64 str_imp_toUInt64(const CharType* pStr, sal_Int16 nRadix)
{
    assert(pStr);
    return str_imp_toUInt<sal_uInt64>(pStr, nRadix);
}

/* ======================================================================= */
/* Internal String-Class help functions                                    */
/* ======================================================================= */

template <typename StringData> struct StringDataChar
{
    using type = std::remove_extent_t<decltype(StringData::buffer)>;
    using unsigned_type = std::make_unsigned_t<type>;
};
template <class StringData> using Char_t = typename StringDataChar<StringData>::type;
template <class StringData> using UChar_t = typename StringDataChar<StringData>::unsigned_type;

template <typename StringData> StringData* str_imp_ImplAlloc(sal_Int32 nLen)
{
    StringData* pData = (sal::static_int_cast<sal_uInt32>(nLen)
                         <= ((SAL_MAX_UINT32 - sizeof(StringData)) / sizeof(Char_t<StringData>)))
                            ? static_cast<StringData*>(rtl_allocateString(
                                  sizeof(StringData) + nLen * sizeof(Char_t<StringData>)))
                            : nullptr;
    if (pData != nullptr)
    {
        pData->refCount = 1;
        pData->length = nLen;
        pData->buffer[nLen] = 0;
    }
    return pData;
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
auto* str_imp_ImplNewCopy(StringData** ppThis, StringData* pStr, sal_Int32 nCount)
{
    assert(nCount >= 0);
    StringData* pData = str_imp_ImplAlloc<StringData>(pStr->length);
    OSL_ASSERT(pData != nullptr);

    auto* pDest = pData->buffer;
    auto* pSrc = pStr->buffer;

    memcpy(pDest, pSrc, nCount * sizeof(*pSrc));

    *ppThis = pData;

    RTL_LOG_STRING_NEW(pData);
    return pDest + nCount;
}

/* ======================================================================= */
/* String-Class functions                                                  */
/* ======================================================================= */

/* ----------------------------------------------------------------------- */

template <typename StringData> void str_imp_acquire(StringData* pThis)
{
    if (!SAL_STRING_IS_STATIC(pThis))
        osl_atomic_increment(&((pThis)->refCount));
}

/* ----------------------------------------------------------------------- */

template <typename StringData> void str_imp_release(StringData* pThis)
{
    if (SAL_UNLIKELY(SAL_STRING_IS_STATIC(pThis)))
        return;

    /* OString doesn't have an 'intern' */
    if constexpr (sizeof(Char_t<StringData>) == sizeof(sal_Unicode))
    {
        if (SAL_STRING_IS_INTERN(pThis))
        {
            internRelease(pThis);
            return;
        }
    }

    if (!osl_atomic_decrement(&(pThis->refCount)))
    {
        RTL_LOG_STRING_DELETE(pThis);
        rtl_freeString(pThis);
    }
}

/* ----------------------------------------------------------------------- */

template <typename StringData> struct EmptyStringImpl
{
    static StringData data; // defined in respective units
};

template <typename StringData> void str_imp_new(StringData** ppThis)
{
    assert(ppThis);
    if (*ppThis)
        str_imp_release(*ppThis);

    *ppThis = &EmptyStringImpl<StringData>::data;
}

/* ----------------------------------------------------------------------- */

template <typename StringData> StringData* str_imp_alloc(sal_Int32 nLen)
{
    assert(nLen >= 0);
    return str_imp_ImplAlloc<StringData>(nLen);
}

/* ----------------------------------------------------------------------- */

template <typename StringData> void str_imp_new_WithLength(StringData** ppThis, sal_Int32 nLen)
{
    assert(ppThis);
    assert(nLen >= 0);
    if (nLen <= 0)
        str_imp_new(ppThis);
    else
    {
        if (*ppThis)
            str_imp_release(*ppThis);

        *ppThis = str_imp_ImplAlloc<StringData>(nLen);
        OSL_ASSERT(*ppThis != nullptr);
        (*ppThis)->length = 0;

        auto* pTempStr = (*ppThis)->buffer;
        memset(pTempStr, 0, nLen * sizeof(*pTempStr));
    }
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newFromString(StringData** ppThis, const StringData* pStr)
{
    assert(ppThis);
    assert(pStr);

    if (!pStr->length)
    {
        str_imp_new(ppThis);
        return;
    }

    StringData* pOrg = *ppThis;
    *ppThis = str_imp_ImplAlloc<StringData>(pStr->length);
    OSL_ASSERT(*ppThis != nullptr);
    rtl_str_ImplCopy((*ppThis)->buffer, pStr->buffer, pStr->length);
    RTL_LOG_STRING_NEW(*ppThis);

    /* must be done last, if pStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newFromStr(StringData** ppThis,
                        const std::remove_extent_t<decltype(StringData::buffer)>* pCharStr)

{
    assert(ppThis);
    sal_Int32 nLen;

#if OSL_DEBUG_LEVEL > 0
    //TODO: For now, only abort in non-production debug builds; once all places that rely on the
    // undocumented newFromStr behavior of treating a null pCharStr like an empty string have been
    // found and fixed, drop support for that behavior and turn this into a general assert:
    if (pCharStr == nullptr)
    {
        std::abort();
    }
#endif

    if (pCharStr)
    {
        nLen = str_imp_getLength(pCharStr);
    }
    else
        nLen = 0;

    if (!nLen)
    {
        str_imp_new(ppThis);
        return;
    }

    StringData* pOrg = *ppThis;
    *ppThis = str_imp_ImplAlloc<StringData>(nLen);
    OSL_ASSERT(*ppThis != nullptr);
    rtl_str_ImplCopy((*ppThis)->buffer, pCharStr, nLen);
    RTL_LOG_STRING_NEW(*ppThis);

    /* must be done last, if pCharStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newFromStr_WithLength(
    StringData** ppThis, const std::remove_extent_t<decltype(StringData::buffer)>* pCharStr,
    sal_Int32 nLen)
{
    assert(ppThis);
    assert(pCharStr != nullptr || nLen == 0);
    assert(nLen >= 0);

    if (nLen == 0)
    {
        str_imp_new(ppThis);
        return;
    }

    StringData* pOrg = *ppThis;
    *ppThis = str_imp_ImplAlloc<StringData>(nLen);
    OSL_ASSERT(*ppThis != nullptr);
    rtl_str_ImplCopy((*ppThis)->buffer, pCharStr, nLen);

    RTL_LOG_STRING_NEW(*ppThis);

    /* must be done last, if pCharStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData> void str_imp_assign(StringData** ppThis, StringData* pStr)
{
    assert(ppThis);
    /* must be done at first, if pStr == *ppThis */
    str_imp_acquire(pStr);

    if (*ppThis)
        str_imp_release(*ppThis);

    *ppThis = pStr;
}

/* ----------------------------------------------------------------------- */

// Used when creating from string literals.
template <typename StringData>
void str_imp_newFromLiteral(StringData** ppThis, const char* pCharStr, sal_Int32 nLen,
                            sal_Int32 allocExtra)
{
    assert(ppThis);
    assert(nLen >= 0);
    assert(allocExtra >= 0);
    if (nLen + allocExtra == 0)
    {
        str_imp_new(ppThis);
        return;
    }

    if (*ppThis)
        str_imp_release(*ppThis);

    *ppThis = str_imp_ImplAlloc<StringData>(nLen + allocExtra);
    assert(*ppThis != nullptr);

    (*ppThis)->length = nLen; // fix after possible allocExtra != 0
    (*ppThis)->buffer[nLen] = 0;
    auto* pBuffer = (*ppThis)->buffer;
    for (sal_Int32 nCount = nLen; nCount > 0; --nCount)
    {
        if constexpr (sizeof(Char_t<StringData>) == sizeof(sal_Unicode))
        {
            assert(static_cast<unsigned char>(*pCharStr) < 0x80); // ASCII range
        }
        SAL_WARN_IF((static_cast<unsigned char>(*pCharStr)) == '\0', "rtl.string",
                    "rtl_uString_newFromLiteral - Found embedded \\0 character");

        *pBuffer = *pCharStr;
        pBuffer++;
        pCharStr++;
    }

    RTL_LOG_STRING_NEW(*ppThis);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newFromSubString(StringData** ppThis, const StringData* pFrom, sal_Int32 beginIndex,
                              sal_Int32 count)
{
    assert(ppThis);
    if (beginIndex == 0 && count == pFrom->length)
    {
        str_imp_assign(ppThis, const_cast<StringData*>(pFrom));
        return;
    }
    if (count < 0 || beginIndex < 0 || beginIndex + count > pFrom->length)
    {
        assert(false); // fail fast at least in debug builds
        str_imp_newFromLiteral(ppThis, "!!br0ken!!", 10, 0);
        return;
    }

    str_imp_newFromStr_WithLength(ppThis, pFrom->buffer + beginIndex, count);
}

/* ----------------------------------------------------------------------- */

template <typename StringData> auto* str_imp_getStr(StringData* pThis)
{
    assert(pThis);
    return pThis->buffer;
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newConcat(StringData** ppThis, StringData* pLeft, StringData* pRight)

{
    assert(ppThis);
    StringData* pOrg = *ppThis;

    /* Test for 0-Pointer - if not, change newReplaceStrAt! */
    if (!pRight || !pRight->length)
    {
        *ppThis = pLeft;
        str_imp_acquire(pLeft);
    }
    else if (!pLeft || !pLeft->length)
    {
        *ppThis = pRight;
        str_imp_acquire(pRight);
    }
    else if (pLeft->length > std::numeric_limits<sal_Int32>::max() - pRight->length)
    {
        *ppThis = nullptr;
    }
    else
    {
        auto* pTempStr = str_imp_ImplAlloc<StringData>(pLeft->length + pRight->length);
        OSL_ASSERT(pTempStr != nullptr);
        *ppThis = pTempStr;
        if (*ppThis != nullptr)
        {
            rtl_str_ImplCopy(pTempStr->buffer, pLeft->buffer, pLeft->length);
            rtl_str_ImplCopy(pTempStr->buffer + pLeft->length, pRight->buffer, pRight->length);

            RTL_LOG_STRING_NEW(*ppThis);
        }
    }

    /* must be done last, if left or right == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData> void str_imp_ensureCapacity(StringData** ppThis, sal_Int32 size)
{
    assert(ppThis);
    StringData* const pOrg = *ppThis;
    if (pOrg->refCount == 1 && pOrg->length >= size)
        return;
    assert(pOrg->length <= size); // do not truncate
    auto* pTempStr = str_imp_ImplAlloc<StringData>(size);
    rtl_str_ImplCopy(pTempStr->buffer, pOrg->buffer, pOrg->length);
    // right now the length is still the same as of the original
    pTempStr->length = pOrg->length;
    pTempStr->buffer[pOrg->length] = '\0';
    *ppThis = pTempStr;
    RTL_LOG_STRING_NEW(*ppThis);

    str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newReplaceStrAt(StringData** ppThis, StringData* pStr, sal_Int32 nIndex,
                             sal_Int32 nCount, StringData* pNewSubStr)
{
    assert(ppThis);
    assert(nIndex >= 0 && nIndex <= pStr->length);
    assert(nCount >= 0);
    assert(nCount <= pStr->length - nIndex);
    /* Append? */
    if (nIndex >= pStr->length)
    {
        /* newConcat test, if pNewSubStr is 0 */
        str_imp_newConcat(ppThis, pStr, pNewSubStr);
        return;
    }

    /* negative index? */
    if (nIndex < 0)
    {
        nCount -= nIndex;
        nIndex = 0;
    }

    /* not more than the String length could be deleted */
    if (nCount >= pStr->length - nIndex)
    {
        nCount = pStr->length - nIndex;

        /* Assign of NewSubStr? */
        if (!nIndex && (nCount >= pStr->length))
        {
            if (!pNewSubStr)
                str_imp_new(ppThis);
            else
                str_imp_assign(ppThis, pNewSubStr);
            return;
        }
    }

    /* Assign of Str? */
    if (!nCount && (!pNewSubStr || !pNewSubStr->length))
    {
        str_imp_assign(ppThis, pStr);
        return;
    }

    StringData* pOrg = *ppThis;

    /* Calculate length of the new string */
    sal_Int32 nNewLen = pStr->length - nCount;
    if (pNewSubStr)
        nNewLen += pNewSubStr->length;

    /* Alloc New Buffer */
    *ppThis = str_imp_ImplAlloc<StringData>(nNewLen);
    OSL_ASSERT(*ppThis != nullptr);
    auto* pBuffer = (*ppThis)->buffer;
    if (nIndex)
    {
        rtl_str_ImplCopy(pBuffer, pStr->buffer, nIndex);
        pBuffer += nIndex;
    }
    if (pNewSubStr && pNewSubStr->length)
    {
        rtl_str_ImplCopy(pBuffer, pNewSubStr->buffer, pNewSubStr->length);
        pBuffer += pNewSubStr->length;
    }
    rtl_str_ImplCopy(pBuffer, pStr->buffer + nIndex + nCount, pStr->length - nIndex - nCount);

    RTL_LOG_STRING_NEW(*ppThis);
    /* must be done last, if pStr or pNewSubStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newReplace(StringData** ppThis, StringData* pStr, Char_t<StringData> cOld,
                        Char_t<StringData> cNew)
{
    assert(ppThis);
    assert(pStr);
    StringData* pOrg = *ppThis;
    bool bChanged = false;
    sal_Int32 nLen = pStr->length;
    const auto* pCharStr = pStr->buffer;

    while (nLen > 0)
    {
        if (*pCharStr == cOld)
        {
            /* Copy String */
            auto* pNewCharStr = str_imp_ImplNewCopy(ppThis, pStr, pCharStr - pStr->buffer);

            /* replace/copy rest of the string */
            if (pNewCharStr)
            {
                *pNewCharStr = cNew;
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while (nLen > 0)
                {
                    if (*pCharStr == cOld)
                        *pNewCharStr = cNew;
                    else
                        *pNewCharStr = *pCharStr;

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = true;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if (!bChanged)
    {
        *ppThis = pStr;
        str_imp_acquire(pStr);
    }

    RTL_LOG_STRING_NEW(*ppThis);
    /* must be done last, if pStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newToAsciiLowerCase(StringData** ppThis, StringData* pStr)
{
    assert(ppThis);
    assert(pStr);
    StringData* pOrg = *ppThis;
    bool bChanged = false;
    sal_Int32 nLen = pStr->length;
    const auto* pCharStr = pStr->buffer;

    while (nLen > 0)
    {
        if (rtl::isAsciiUpperCase(UChar_t<StringData>(*pCharStr)))
        {
            /* Copy String */
            auto* pNewCharStr = str_imp_ImplNewCopy(ppThis, pStr, pCharStr - pStr->buffer);

            /* replace/copy rest of the string */
            if (pNewCharStr)
            {
                *pNewCharStr = rtl::toAsciiLowerCase(UChar_t<StringData>(*pCharStr));
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while (nLen > 0)
                {
                    *pNewCharStr = rtl::toAsciiLowerCase(UChar_t<StringData>(*pCharStr));

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = true;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if (!bChanged)
    {
        *ppThis = pStr;
        str_imp_acquire(pStr);
    }

    RTL_LOG_STRING_NEW(*ppThis);
    /* must be done last, if pStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
void str_imp_newToAsciiUpperCase(StringData** ppThis, StringData* pStr)
{
    assert(ppThis);
    assert(pStr);
    StringData* pOrg = *ppThis;
    bool bChanged = false;
    sal_Int32 nLen = pStr->length;
    const auto* pCharStr = pStr->buffer;

    while (nLen > 0)
    {
        if (rtl::isAsciiLowerCase(UChar_t<StringData>(*pCharStr)))
        {
            /* Copy String */
            auto* pNewCharStr = str_imp_ImplNewCopy(ppThis, pStr, pCharStr - pStr->buffer);

            /* replace/copy rest of the string */
            if (pNewCharStr)
            {
                *pNewCharStr = rtl::toAsciiUpperCase(UChar_t<StringData>(*pCharStr));
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while (nLen > 0)
                {
                    *pNewCharStr = rtl::toAsciiUpperCase(UChar_t<StringData>(*pCharStr));

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = true;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if (!bChanged)
    {
        *ppThis = pStr;
        str_imp_acquire(pStr);
    }

    RTL_LOG_STRING_NEW(*ppThis);
    /* must be done last, if pStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData> void str_imp_newTrim(StringData** ppThis, StringData* pStr)
{
    assert(ppThis);
    assert(pStr);
    StringData* pOrg = *ppThis;
    const auto* pCharStr = pStr->buffer;
    sal_Int32 nPreSpaces = 0;
    sal_Int32 nPostSpaces = 0;
    sal_Int32 nLen = pStr->length;
    sal_Int32 nIndex = nLen - 1;

    while ((nPreSpaces < nLen)
           && rtl_ImplIsWhitespace(UChar_t<StringData>(*(pCharStr + nPreSpaces))))
        nPreSpaces++;

    while ((nIndex > nPreSpaces) && rtl_ImplIsWhitespace(UChar_t<StringData>(*(pCharStr + nIndex))))
    {
        nPostSpaces++;
        nIndex--;
    }

    if (!nPreSpaces && !nPostSpaces)
    {
        *ppThis = pStr;
        str_imp_acquire(pStr);
    }
    else
    {
        nLen -= nPostSpaces + nPreSpaces;
        *ppThis = str_imp_ImplAlloc<StringData>(nLen);
        assert(*ppThis);
        rtl_str_ImplCopy((*ppThis)->buffer, pStr->buffer + nPreSpaces, nLen);
    }

    RTL_LOG_STRING_NEW(*ppThis);
    /* must be done last, if pStr == *ppThis */
    if (pOrg)
        str_imp_release(pOrg);
}

/* ----------------------------------------------------------------------- */

template <typename StringData>
sal_Int32 str_imp_getToken(StringData** ppThis, StringData* pStr, sal_Int32 nToken,
                           Char_t<StringData> cTok, sal_Int32 nIndex)
{
    assert(ppThis);
    assert(pStr);
    const auto* pCharStr = pStr->buffer;
    sal_Int32 nLen = pStr->length - nIndex;
    sal_Int32 nTokCount = 0;

    // Set ppThis to an empty string and return -1 if either nToken or nIndex is
    // negative:
    if (nIndex < 0)
        nToken = -1;

    pCharStr += nIndex;
    const auto* pOrgCharStr = pCharStr;
    const auto* pCharStrStart = pCharStr;
    while (nLen > 0)
    {
        if (*pCharStr == cTok)
        {
            nTokCount++;

            if (nTokCount == nToken)
                pCharStrStart = pCharStr + 1;
            else
            {
                if (nTokCount > nToken)
                    break;
            }
        }

        pCharStr++;
        nLen--;
    }

    if ((nToken < 0) || (nTokCount < nToken) || (pCharStr == pCharStrStart))
    {
        str_imp_new(ppThis);
        if ((nToken < 0) || (nTokCount < nToken))
            return -1;
        else if (nLen > 0)
            return nIndex + (pCharStr - pOrgCharStr) + 1;
        else
            return -1;
    }
    else
    {
        str_imp_newFromStr_WithLength(ppThis, pCharStrStart, pCharStr - pCharStrStart);
        if (nLen)
            return nIndex + (pCharStr - pOrgCharStr) + 1;
        else
            return -1;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
