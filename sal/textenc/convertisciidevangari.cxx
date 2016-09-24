/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "converter.hxx"
#include "unichars.hxx"
#include "convertisciidevangari.hxx"
#include "convertsinglebytetobmpunicode.hxx"
#include <rtl/textcvt.h>

using namespace sal::detail::textenc;
using namespace rtl::textenc;

struct IsciiDevanagariToUnicode
{
    sal_uInt8 m_cPrevChar;
    IsciiDevanagariToUnicode()
        : m_cPrevChar(0)
    {
    }
    void reset()
    {
        m_cPrevChar = 0;
    }
    sal_Size convert(char const* pSrcBuf, sal_Size nSrcBytes,
        sal_Unicode* pDestBuf, sal_Size nDestChars, sal_uInt32 nFlags,
        sal_uInt32* pInfo, sal_Size* pSrcCvtBytes);
};

struct UnicodeToIsciiDevanagari
{
    sal_Unicode m_cPrevChar;
    sal_Unicode m_cHighSurrogate;
    UnicodeToIsciiDevanagari()
        : m_cPrevChar(0)
        , m_cHighSurrogate(0)
    {
    }
    void reset()
    {
        m_cPrevChar = 0;
        m_cHighSurrogate = 0;
    }
    sal_Size convert(sal_Unicode const* pSrcBuf, sal_Size nSrcChars,
        char* pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
        sal_uInt32 * pInfo, sal_Size * pSrcCvtChars);
};

static const sal_Unicode IsciiDevanagariMap[256] =
{
    0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
    0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
    0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
    0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
    0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
    0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
    0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
    0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
    0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
    0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
    0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
    0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
    0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
    0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
    0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
    0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0x0901,0x0902,0x0903,0x0905,0x0906,0x0907,0x0908,
    0x0909,0x090A,0x090B,0x090E,0x090F,0x0910,0x090D,0x0912,
    0x0913,0x0914,0x0911,0x0915,0x0916,0x0917,0x0918,0x0919,
    0x091A,0x091B,0x091C,0x091D,0x091E,0x091F,0x0920,0x0921,
    0x0922,0x0923,0x0924,0x0925,0x0926,0x0927,0x0928,0x0929,
    0x092A,0x092B,0x092C,0x092D,0x092E,0x092F,0x095F,0x0930,
    0x0931,0x0932,0x0933,0x0934,0x0935,0x0936,0x0937,0x0938,
    0x0939,0xFFFF,0x093E,0x093F,0x0940,0x0941,0x0942,0x0943,
    0x0946,0x0947,0x0948,0x0945,0x094A,0x094B,0x094C,0x0949,
    0x094D,0x093C,0x0964,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0x0966,0x0967,0x0968,0x0969,0x096A,0x096B,0x096C,
    0x096D,0x096E,0x096F,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF
};

sal_Size IsciiDevanagariToUnicode::convert(
    char const* pSrcBuf, sal_Size nSrcBytes,
    sal_Unicode* pDestBuf, sal_Size nDestChars, sal_uInt32 nFlags,
    sal_uInt32* pInfo, sal_Size* pSrcCvtBytes)
{
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode* pDestBufPtr = pDestBuf;
    sal_Unicode* pDestBufEnd = pDestBuf + nDestChars;

    while (nConverted < nSrcBytes)
    {
        if (pDestBufPtr == pDestBufEnd)
        {
            nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
            break;
        }

        sal_Unicode cChar = sal_Unicode();
        sal_uInt8 nIn = static_cast<sal_uInt8>(pSrcBuf[nConverted]);
        sal_uInt8 nNext = nConverted < nSrcBytes + 1 ? static_cast<sal_uInt8>(pSrcBuf[nConverted+1]) : 0;
        bool bNormal = true;
        bool bDouble = false;
        //halant + halant     E8 E8  -> halant + ZWNJ   094D 200C
        //halant + nukta    E8 E9   halant + ZWJ    094D 200D
        if (m_cPrevChar == 0xE8 && nIn == 0xE8)
        {
            cChar = 0x200C;
            bNormal = false;
        }
        else if (m_cPrevChar == 0xE8 && nIn == 0xE9)
        {
            cChar = 0x200D;
            bNormal = false;
        }
        else if (nNext == 0xE9)
        {
            bNormal = false;
            bDouble = true;
            switch(nIn)
            {
                case 0xA1:
                    cChar = 0x0950;
                    break;
                case 0xA6:
                    cChar = 0x090C;
                    break;
                case 0xA7:
                    cChar = 0x0961;
                    break;
                case 0xAA:
                    cChar = 0x0960;
                    break;
                case 0xB3:
                    cChar = 0x0958;
                    break;
                case 0xB4:
                    cChar = 0x0959;
                    break;
                case 0xB5:
                    cChar = 0x095A;
                    break;
                case 0xBA:
                    cChar = 0x095B;
                    break;
                case 0xBF:
                    cChar = 0x095C;
                    break;
                case 0xC0:
                    cChar = 0x095D;
                    break;
                case 0xC9:
                    cChar = 0x095E;
                    break;
                case 0xDB:
                    cChar = 0x0962;
                    break;
                case 0xDC:
                    cChar = 0x0963;
                    break;
                case 0xDF:
                    cChar = 0x0944;
                    break;
                case 0xEA:
                    cChar = 0x093D;
                    break;
                default:
                    bNormal = true;
                    bDouble = false;
                    break;
            }
        }

        if (bNormal)
            cChar = IsciiDevanagariMap[nIn];

        bool bUndefined = cChar == 0xffff;

        if (bUndefined)
        {
            BadInputConversionAction eAction = handleBadInputTextToUnicodeConversion(
                        bUndefined, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                        &nInfo);
            if (eAction == BAD_INPUT_CONTINUE)
                continue;
            if (eAction == BAD_INPUT_STOP)
                break;
            else if (eAction == BAD_INPUT_NO_OUTPUT)
            {
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
        }
        ++nConverted;
        if (bDouble)
            ++nConverted;

        *pDestBufPtr++ = cChar;
        m_cPrevChar = bNormal ? nIn : 0;
    }

    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

BmpUnicodeToSingleByteRange const unicodeToISCIIEncoding[] =
{
    { 0x0000, 0x007F - 0x0000, 0x00 }, { 0x0901, 0x0903 - 0x0901, 0xA1 },
    { 0x0905, 0x090B - 0x0905, 0xA4 }, { 0x090D, 0x090D - 0x090D, 0xAE },
    { 0x090E, 0x0910 - 0x090E, 0xAB }, { 0x0911, 0x0911 - 0x0911, 0xB2 },
    { 0x0912, 0x0914 - 0x0912, 0xAF }, { 0x0915, 0x092F - 0x0915, 0xB3 },
    { 0x0930, 0x0939 - 0x0930, 0xCF }, { 0x093C, 0x093C - 0x093C, 0xE9 },
    { 0x093E, 0x0943 - 0x093E, 0xDA }, { 0x0945, 0x0945 - 0x0945, 0xE3 },
    { 0x0946, 0x0948 - 0x0946, 0xE0 }, { 0x0949, 0x0949 - 0x0949, 0xE7 },
    { 0x094A, 0x094C - 0x094A, 0xE4 }, { 0x094D, 0x094D - 0x094D, 0xE8 },
    { 0x095F, 0x095F - 0x095F, 0xCE }, { 0x0964, 0x0964 - 0x0964, 0xEA },
    { 0x0966, 0x096F - 0x0966, 0xF1 }
};

sal_Size UnicodeToIsciiDevanagari::convert(sal_Unicode const* pSrcBuf, sal_Size nSrcChars,
    char* pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size* pSrcCvtChars)
{
    size_t entries = SAL_N_ELEMENTS(unicodeToISCIIEncoding);
    BmpUnicodeToSingleByteRange const * ranges = unicodeToISCIIEncoding;

    sal_Unicode cHighSurrogate = m_cHighSurrogate;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Char* pDestBufPtr = pDestBuf;
    sal_Char* pDestBufEnd = pDestBuf + nDestBytes;
    for (; nConverted < nSrcChars; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 c = *pSrcBuf++;
        sal_Char cSpecialChar = 0;
        if (cHighSurrogate == 0)
        {
            if (ImplIsHighSurrogate(c))
            {
                cHighSurrogate = static_cast< sal_Unicode >(c);
                continue;
            }
        }
        else if (ImplIsLowSurrogate(c))
        {
            c = ImplCombineSurrogates(cHighSurrogate, c);
        }
        else
        {
            bUndefined = false;
            goto bad_input;
        }
        if (ImplIsLowSurrogate(c) || ImplIsNoncharacter(c))
        {
            bUndefined = false;
            goto bad_input;
        }

        //halant + halant     E8 E8  -> halant + ZWNJ   094D 200C
        //halant + nukta    E8 E9   halant + ZWJ    094D 200D
        if (m_cPrevChar == 0x094D && c == 0x200C)
            cSpecialChar = '\xE8';
        else if (m_cPrevChar == 0x094D && c == 0x200D)
            cSpecialChar = '\xE9';
        if (cSpecialChar)
        {
            if (pDestBufEnd - pDestBufPtr < 1)
            {
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
            *pDestBufPtr++ = cSpecialChar;
            m_cPrevChar = 0;
            goto done;
        }
        switch (c)
        {
            case 0x0950:
                cSpecialChar = '\xA1';
                break;
            case 0x090C:
                cSpecialChar = '\xA6';
                break;
            case 0x0961:
                cSpecialChar = '\xA7';
                break;
            case 0x0960:
                cSpecialChar = '\xAA';
                break;
            case 0x0958:
                cSpecialChar = '\xB3';
                break;
            case 0x0959:
                cSpecialChar = '\xB4';
                break;
            case 0x095A:
                cSpecialChar = '\xB5';
                break;
            case 0x095B:
                cSpecialChar = '\xBA';
                break;
            case 0x095C:
                cSpecialChar = '\xBF';
                break;
            case 0x095D:
                cSpecialChar = '\xC0';
                break;
            case 0x095E:
                cSpecialChar = '\xC9';
                break;
            case 0x0962:
                cSpecialChar = '\xDB';
                break;
            case 0x0963:
                cSpecialChar = '\xDC';
                break;
            case 0x0944:
                cSpecialChar = '\xDF';
                break;
            case 0x093D:
                cSpecialChar = '\xEA';
                break;
            default:
                break;
        }
        if (cSpecialChar)
        {
            if (pDestBufEnd - pDestBufPtr < 2)
            {
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
            *pDestBufPtr++ = cSpecialChar;
            *pDestBufPtr++ = '\xE9';
            m_cPrevChar = 0;
            goto done;
        }

        // Linearly searching through the ranges if probably fastest, assuming
        // that most converted characters belong to the ASCII subset:
        for (size_t i = 0; i < entries; ++i)
        {
            if (c < ranges[i].unicode)
            {
                break;
            }
            else if (c <= sal::static_int_cast< sal_uInt32 >(
                           ranges[i].unicode + ranges[i].range))
            {
                if (pDestBufEnd - pDestBufPtr < 1)
                {
                    goto no_output;
                }
                *pDestBufPtr++ = static_cast< sal_Char >(
                    ranges[i].byte + (c - ranges[i].unicode));
                m_cPrevChar = c;
                goto done;
            }
        }
        goto bad_input;
    done:
        cHighSurrogate = 0;
        continue;
    bad_input:
        switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                    bUndefined, c, nFlags, &pDestBufPtr, pDestBufEnd, &nInfo, nullptr,
                    0, nullptr))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            cHighSurrogate = 0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            cHighSurrogate = 0;
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;
    no_output:
        --pSrcBuf;
        nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        break;
    }

    if (cHighSurrogate != 0
        && ((nInfo
             & (RTL_UNICODETOTEXT_INFO_ERROR
                | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL))
            == 0))
    {
        if ((nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
        {
            nInfo |= RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
        }
        else
        {
            switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                        false, 0, nFlags, &pDestBufPtr, pDestBufEnd, &nInfo, nullptr,
                        0, nullptr))
            {
            case sal::detail::textenc::BAD_INPUT_STOP:
            case sal::detail::textenc::BAD_INPUT_CONTINUE:
                cHighSurrogate = 0;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
        }
    }
    m_cHighSurrogate = cHighSurrogate;
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertIsciiDevanagariToUnicode(void const*,
    void* pContext, char const* pSrcBuf, sal_Size nSrcBytes,
    sal_Unicode* pDestBuf, sal_Size nDestChars, sal_uInt32 nFlags,
    sal_uInt32* pInfo, sal_Size* pSrcCvtBytes)
{
    IsciiDevanagariToUnicode *pCtx =
        static_cast<IsciiDevanagariToUnicode*>(pContext);
    return pCtx->convert(pSrcBuf, nSrcBytes, pDestBuf, nDestChars, nFlags,
        pInfo, pSrcCvtBytes);
}

sal_Size ImplConvertUnicodeToIsciiDevanagari(void const*,
    void * pContext, sal_Unicode const * pSrcBuf, sal_Size nSrcChars,
    char * pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size * pSrcCvtChars)
{
    UnicodeToIsciiDevanagari *pCtx =
        static_cast<UnicodeToIsciiDevanagari*>(pContext);
    return pCtx->convert(pSrcBuf, nSrcChars,
        pDestBuf, nDestBytes, nFlags, pInfo, pSrcCvtChars);
}

void *ImplCreateIsciiDevanagariToUnicodeContext()
{
    return new IsciiDevanagariToUnicode;
}

void ImplDestroyIsciiDevanagariToUnicodeContext(void * pContext)
{
    IsciiDevanagariToUnicode *pCtx =
        static_cast<IsciiDevanagariToUnicode*>(pContext);
    delete pCtx;
}

void ImplResetIsciiDevanagariToUnicodeContext(void * pContext)
{
    IsciiDevanagariToUnicode *pCtx =
        static_cast<IsciiDevanagariToUnicode*>(pContext);
    pCtx->reset();
}

void *ImplCreateUnicodeToIsciiDevanagariContext()
{
    return new UnicodeToIsciiDevanagari;
}

void ImplResetUnicodeToIsciiDevanagariContext(void * pContext)
{
    UnicodeToIsciiDevanagari *pCtx =
        static_cast<UnicodeToIsciiDevanagari*>(pContext);
    pCtx->reset();
}

void ImplDestroyUnicodeToIsciiDevanagariContext(void * pContext)
{
    UnicodeToIsciiDevanagari *pCtx =
        static_cast<UnicodeToIsciiDevanagari*>(pContext);
    delete pCtx;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
