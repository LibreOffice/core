/*************************************************************************
 *
 *  $RCSfile: convertsinglebytetobmpunicode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:41:43 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "context.h"
#include "converter.h"
#include "convertsinglebytetobmpunicode.hxx"
#include "unichars.h"

#include "osl/diagnose.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

#include <cstddef>

sal_Size rtl_textenc_convertSingleByteToBmpUnicode(
    ImplTextConverterData const * data, void * context, sal_Char const * srcBuf,
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
        switch (ImplHandleBadInputTextToUnicodeConversion(
                    undefined, false, b, flags, &destBufPtr, destBufEnd,
                    &infoFlags))
        {
        case IMPL_BAD_INPUT_STOP:
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            continue;

        case IMPL_BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;
    no_output:
        --srcBuf;
        infoFlags |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
        break;
    }
    if (info != 0) {
        *info = infoFlags;
    }
    if (srcCvtBytes != 0) {
        *srcCvtBytes = converted;
    }
    return destBufPtr - destBuf;
}

sal_Size rtl_textenc_convertBmpUnicodeToSingleByte(
    ImplTextConverterData const * data, void * context,
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
    if (context != 0) {
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
            } else if (c <= ranges[i].unicode + ranges[i].range) {
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
        switch (ImplHandleBadInputUnicodeToTextConversion(
                    undefined, c, flags, &destBufPtr, destBufEnd, &infoFlags, 0,
                    0, 0))
        {
        case IMPL_BAD_INPUT_STOP:
            highSurrogate = 0;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            highSurrogate = 0;
            continue;

        case IMPL_BAD_INPUT_NO_OUTPUT:
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
            switch (ImplHandleBadInputUnicodeToTextConversion(
                        false, 0, flags, &destBufPtr, destBufEnd, &infoFlags, 0,
                        0, 0))
            {
            case IMPL_BAD_INPUT_STOP:
            case IMPL_BAD_INPUT_CONTINUE:
                highSurrogate = 0;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                infoFlags |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
        }
    }
    if (context != 0) {
        static_cast< ImplUnicodeToTextContext * >(context)->m_nHighSurrogate
            = highSurrogate;
    }
    if (info != 0) {
        *info = infoFlags;
    }
    if (srcCvtChars != 0) {
        *srcCvtChars = converted;
    }
    return destBufPtr - destBuf;
}
