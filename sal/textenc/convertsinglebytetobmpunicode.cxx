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

#include "sal/config.h"

#include <cstddef>

#include "rtl/textcvt.h"
#include "sal/types.h"

#include "context.hxx"
#include "converter.hxx"
#include "convertsinglebytetobmpunicode.hxx"
#include "unichars.hxx"

sal_Size rtl_textenc_convertSingleByteToBmpUnicode(
    void const * data, SAL_UNUSED_PARAMETER void *, sal_Char const * srcBuf,
    sal_Size srcBytes, sal_Unicode * destBuf, sal_Size destChars,
    sal_uInt32 flags, sal_uInt32 * info, sal_Size * srcCvtBytes)
{
    sal_Unicode const * map = static_cast<
        rtl::textenc::BmpUnicodeToSingleByteConverterData const * >(
            data)->byteToUnicode;
    sal_uInt32 infoFlags = 0;
    sal_Size converted = 0;
    sal_Unicode * destBufPtr = destBuf;
    sal_Unicode * destBufEnd = destBuf + destChars;
    for (; converted < srcBytes; ++converted) {
        bool undefined = true;
        sal_Char b = *srcBuf++;
        sal_Unicode c = map[static_cast< sal_uInt8 >(b)];
        if (c == 0xFFFF) {
            goto bad_input;
        }
        if (destBufEnd - destBufPtr < 1) {
            goto no_output;
        }
        *destBufPtr++ = c;
        continue;
    bad_input:
        switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                    undefined, false, b, flags, &destBufPtr, destBufEnd,
                    &infoFlags))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;
    no_output:
        --srcBuf;
        infoFlags |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
        break;
    }
    if (info != nullptr) {
        *info = infoFlags;
    }
    if (srcCvtBytes != nullptr) {
        *srcCvtBytes = converted;
    }
    return destBufPtr - destBuf;
}

sal_Size rtl_textenc_convertBmpUnicodeToSingleByte(
    void const * data, void * context,
    sal_Unicode const * srcBuf, sal_Size srcChars, sal_Char * destBuf,
    sal_Size destBytes, sal_uInt32 flags, sal_uInt32 * info,
    sal_Size * srcCvtChars)
{
    std::size_t entries = static_cast<
        rtl::textenc::BmpUnicodeToSingleByteConverterData const * >(
            data)->unicodeToByteEntries;
    rtl::textenc::BmpUnicodeToSingleByteRange const * ranges = static_cast<
        rtl::textenc::BmpUnicodeToSingleByteConverterData const * >(
            data)->unicodeToByte;
    sal_Unicode highSurrogate = 0;
    sal_uInt32 infoFlags = 0;
    sal_Size converted = 0;
    sal_Char * destBufPtr = destBuf;
    sal_Char * destBufEnd = destBuf + destBytes;
    if (context != nullptr) {
        highSurrogate = static_cast< ImplUnicodeToTextContext * >(context)->
            m_nHighSurrogate;
    }
    for (; converted < srcChars; ++converted) {
        bool undefined = true;
        sal_uInt32 c = *srcBuf++;
        if (highSurrogate == 0) {
            if (ImplIsHighSurrogate(c)) {
                highSurrogate = static_cast< sal_Unicode >(c);
                continue;
            }
        } else if (ImplIsLowSurrogate(c)) {
            c = ImplCombineSurrogates(highSurrogate, c);
        } else {
            undefined = false;
            goto bad_input;
        }
        if (ImplIsLowSurrogate(c) || ImplIsNoncharacter(c)) {
            undefined = false;
            goto bad_input;
        }
        // Linearly searching through the ranges if probably fastest, assuming
        // that most converted characters belong to the ASCII subset:
        for (std::size_t i = 0; i < entries; ++i) {
            if (c < ranges[i].unicode) {
                break;
            } else if (c <= sal::static_int_cast< sal_uInt32 >(
                           ranges[i].unicode + ranges[i].range))
            {
                if (destBufEnd - destBufPtr < 1) {
                    goto no_output;
                }
                *destBufPtr++ = static_cast< sal_Char >(
                    ranges[i].byte + (c - ranges[i].unicode));
                goto done;
            }
        }
        goto bad_input;
    done:
        highSurrogate = 0;
        continue;
    bad_input:
        switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                    undefined, c, flags, &destBufPtr, destBufEnd, &infoFlags, nullptr,
                    0, nullptr))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            highSurrogate = 0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            highSurrogate = 0;
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;
    no_output:
        --srcBuf;
        infoFlags |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        break;
    }
    if (highSurrogate != 0
        && ((infoFlags
             & (RTL_UNICODETOTEXT_INFO_ERROR
                | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL))
            == 0))
    {
        if ((flags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0) {
            infoFlags |= RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
        } else {
            switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                        false, 0, flags, &destBufPtr, destBufEnd, &infoFlags, nullptr,
                        0, nullptr))
            {
            case sal::detail::textenc::BAD_INPUT_STOP:
            case sal::detail::textenc::BAD_INPUT_CONTINUE:
                highSurrogate = 0;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                infoFlags |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
        }
    }
    if (context != nullptr) {
        static_cast< ImplUnicodeToTextContext * >(context)->m_nHighSurrogate
            = highSurrogate;
    }
    if (info != nullptr) {
        *info = infoFlags;
    }
    if (srcCvtChars != nullptr) {
        *srcCvtChars = converted;
    }
    return destBufPtr - destBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
