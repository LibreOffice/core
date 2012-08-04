/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#include "stringutil.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/math.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

ScSetStringParam::ScSetStringParam() :
    mpNumFormatter(NULL),
    mbDetectNumberFormat(true),
    mbSetTextCellFormat(false),
    mbHandleApostrophe(true)
{
}

// ============================================================================-

bool ScStringUtil::parseSimpleNumber(
    const OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, double& rVal)
{
    // Actually almost the entire pre-check is unnecessary and we could call
    // rtl::math::stringToDouble() just after having exchanged ascii space with
    // non-breaking space, if it wasn't for check of grouped digits. The NaN
    // and Inf cases that are accepted by stringToDouble() could be detected
    // using rtl::math::isFinite() on the result.

    /* TODO: The grouped digits check isn't even valid for locales that do not
     * group in thousands ... e.g. Indian locales. But that's something also
     * the number scanner doesn't implement yet, only the formatter. */

    OUStringBuffer aBuf;

    sal_Int32 i = 0;
    sal_Int32 n = rStr.getLength();
    const sal_Unicode* p = rStr.getStr();
    const sal_Unicode* pLast = p + (n-1);
    sal_Int32 nPosDSep = -1, nPosGSep = -1;
    sal_uInt32 nDigitCount = 0;
    sal_Int32 nPosExponent = -1;

    // Skip preceding spaces.
    for (i = 0; i < n; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (c != 0x0020 && c != 0x00A0)
            // first non-space character.  Exit.
            break;
    }

    if (i == n)
        // the whole string is space.  Fail.
        return false;

    n -= i; // Subtract the length of the preceding spaces.

    // Determine the last non-space character.
    for (; p != pLast; --pLast, --n)
    {
        sal_Unicode c = *pLast;
        if (c != 0x0020 && c != 0x00A0)
            // Non space character. Exit.
            break;
    }

    for (i = 0; i < n; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (c == 0x0020 && gsep == 0x00A0)
            // ascii space to unicode space if that is group separator
            c = 0x00A0;

        if (sal_Unicode('0') <= c && c <= sal_Unicode('9'))
        {
            // this is a digit.
            aBuf.append(c);
            ++nDigitCount;
        }
        else if (c == dsep)
        {
            // this is a decimal separator.

            if (nPosDSep >= 0)
                // a second decimal separator -> not a valid number.
                return false;

            if (nPosGSep >= 0 && i - nPosGSep != 4)
                // the number has a group separator and the decimal sep is not
                // positioned correctly.
                return false;

            nPosDSep = i;
            nPosGSep = -1;
            aBuf.append(c);
            nDigitCount = 0;
        }
        else if (c == gsep)
        {
            // this is a group (thousand) separator.

            if (i == 0)
                // not allowed as the first character.
                return false;

            if (nPosDSep >= 0)
                // not allowed after the decimal separator.
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                // must be exactly 3 digits since the last group separator.
                return false;

            if (nPosExponent >= 0)
                // not allowed in exponent.
                return false;

            nPosGSep = i;
            nDigitCount = 0;
        }
        else if (c == sal_Unicode('-') || c == sal_Unicode('+'))
        {
            // A sign must be the first character if it's given, or immediately
            // follow the exponent character if present.
            if (i == 0 || (nPosExponent >= 0 && i == nPosExponent + 1))
                aBuf.append(c);
            else
                return false;
        }
        else if (c == sal_Unicode('E') || c == sal_Unicode('e'))
        {
            // this is an exponent designator.

            if (nPosExponent >= 0)
                // Only one exponent allowed.
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                // must be exactly 3 digits since the last group separator.
                return false;

            aBuf.append(c);
            nPosExponent = i;
            nPosDSep = -1;
            nPosGSep = -1;
            nDigitCount = 0;
        }
        else
            return false;
    }

    // finished parsing the number.

    if (nPosGSep >= 0 && nDigitCount != 3)
        // must be exactly 3 digits since the last group separator.
        return false;

    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nParseEnd = 0;
    OUString aString( aBuf.makeStringAndClear());
    rVal = ::rtl::math::stringToDouble( aString, dsep, gsep, &eStatus, &nParseEnd);
    if (eStatus != rtl_math_ConversionStatus_Ok || nParseEnd < aString.getLength())
        // Not a valid number or not entire string consumed.
        return false;

    return true;
}

xub_StrLen ScStringUtil::GetQuotedTokenCount(const UniString &rIn, const UniString& rQuotedPairs, sal_Unicode cTok )
{
    assert( !(rQuotedPairs.Len()%2) );
    assert( rQuotedPairs.Search(cTok) );

    // Leerer String: TokenCount per Definition 0
    if ( !rIn.Len() )
        return 0;

    xub_StrLen      nTokCount       = 1;
    sal_Int32       nLen            = rIn.Len();
    xub_StrLen      nQuotedLen      = rQuotedPairs.Len();
    sal_Unicode         cQuotedEndChar  = 0;
    const sal_Unicode*  pQuotedStr      = rQuotedPairs.GetBuffer();
    const sal_Unicode*  pStr            = rIn.GetBuffer();
    sal_Int32       nIndex          = 0;
    while ( nIndex < nLen )
    {
        sal_Unicode c = *pStr;
        if ( cQuotedEndChar )
        {
            // Ende des Quotes erreicht ?
            if ( c == cQuotedEndChar )
                cQuotedEndChar = 0;
        }
        else
        {
            // Ist das Zeichen ein Quote-Anfang-Zeichen ?
            xub_StrLen nQuoteIndex = 0;
            while ( nQuoteIndex < nQuotedLen )
            {
                if ( pQuotedStr[nQuoteIndex] == c )
                {
                    cQuotedEndChar = pQuotedStr[nQuoteIndex+1];
                    break;
                }
                else
                    nQuoteIndex += 2;
            }

            // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
            if ( c == cTok )
                ++nTokCount;
        }

        ++pStr,
        ++nIndex;
    }

    return nTokCount;
}

UniString ScStringUtil::GetQuotedToken(const UniString &rIn, xub_StrLen nToken, const UniString& rQuotedPairs,
                               sal_Unicode cTok, xub_StrLen& rIndex )
{
    assert( !(rQuotedPairs.Len()%2) );
    assert( rQuotedPairs.Search(cTok) == STRING_NOTFOUND );

    const sal_Unicode*  pStr            = rIn.GetBuffer();
    const sal_Unicode*  pQuotedStr      = rQuotedPairs.GetBuffer();
    sal_Unicode         cQuotedEndChar  = 0;
    xub_StrLen      nQuotedLen      = rQuotedPairs.Len();
    xub_StrLen      nLen            = rIn.Len();
    xub_StrLen      nTok            = 0;
    xub_StrLen      nFirstChar      = rIndex;
    xub_StrLen      i               = nFirstChar;

    // Bestimme die Token-Position und Laenge
    pStr += i;
    while ( i < nLen )
    {
        sal_Unicode c = *pStr;
        if ( cQuotedEndChar )
        {
            // Ende des Quotes erreicht ?
            if ( c == cQuotedEndChar )
                cQuotedEndChar = 0;
        }
        else
        {
            // Ist das Zeichen ein Quote-Anfang-Zeichen ?
            xub_StrLen nQuoteIndex = 0;
            while ( nQuoteIndex < nQuotedLen )
            {
                if ( pQuotedStr[nQuoteIndex] == c )
                {
                    cQuotedEndChar = pQuotedStr[nQuoteIndex+1];
                    break;
                }
                else
                    nQuoteIndex += 2;
            }

            // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
            if ( c == cTok )
            {
                ++nTok;

                if ( nTok == nToken )
                    nFirstChar = i+1;
                else
                {
                    if ( nTok > nToken )
                        break;
                }
            }
        }

        ++pStr,
        ++i;
    }

    if ( nTok >= nToken )
    {
        if ( i < nLen )
            rIndex = i+1;
        else
            rIndex = STRING_NOTFOUND;
        return rIn.Copy( nFirstChar, i-nFirstChar );
    }
    else
    {
        rIndex = STRING_NOTFOUND;
        return UniString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
