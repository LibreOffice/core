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
#pragma once

#include <sal/config.h>

#include <algorithm>
#include <vector>
#include <comphelper/comphelperdllapi.h>
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/lang/Locale.hpp>

namespace com::sun::star::i18n { class XBreakIterator; }
namespace com::sun::star::i18n { class XCollator; }
namespace com::sun::star::uno { class XComponentContext; }

// OUString helper functions that are not widespread or mature enough to
// go into the stable URE API:
namespace comphelper::string {

/** Removes all occurrences of a character from within the source string

    @param rIn      The input OUStringBuffer
    @param c        The character to be removed

    @return         The resulting OUStringBuffer
 */
inline OUStringBuffer& remove(OUStringBuffer &rIn,
    sal_Unicode c)
{
    sal_Int32 index = 0;
    while (true)
    {
        if (index >= rIn.getLength())
            break;
        index = rIn.indexOf(c, index);
        if (index == -1)
            break;
        rIn.remove(index, 1);
    }
    return rIn;
}

/** Strips occurrences of a character from the start of the source string

    @param rIn      The input OString
    @param c        The character to be stripped from the start

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC OString stripStart(const OString& rIn,
    char c);
COMPHELPER_DLLPUBLIC std::string_view stripStart(std::string_view rIn,
    char c);

/** Strips occurrences of a character from the start of the source string

    @param rIn      The input OUString
    @param c        The character to be stripped from the start

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC OUString stripStart(const OUString& rIn,
    sal_Unicode c);
COMPHELPER_DLLPUBLIC std::u16string_view stripStart(std::u16string_view rIn,
    sal_Unicode c);

/** Strips occurrences of a character from the end of the source string

    @param rIn      The input OString
    @param c        The character to be stripped from the end

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC OString stripEnd(const OString& rIn,
    char c);
COMPHELPER_DLLPUBLIC std::string_view stripEnd(std::string_view rIn,
    char c);

/** Strips occurrences of a character from the end of the source string

    @param rIn      The input OUString
    @param c        The character to be stripped from the end

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC OUString stripEnd(const OUString& rIn,
    sal_Unicode c);
COMPHELPER_DLLPUBLIC std::u16string_view stripEnd(std::u16string_view rIn,
    sal_Unicode c);

/** Strips occurrences of a character from the start and end of the source string

    @param rIn      The input OString
    @param c        The character to be stripped from the start and end

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC OString strip(const OString& rIn,
    char c);
COMPHELPER_DLLPUBLIC std::string_view strip(std::string_view rIn,
    char c);

/** Strips occurrences of a character from the start and end of the source string

    @param rIn      The input OUString
    @param c        The character to be stripped from the start and end

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC OUString strip(const OUString& rIn,
    sal_Unicode c);
COMPHELPER_DLLPUBLIC std::u16string_view strip(std::u16string_view rIn,
    sal_Unicode c);

/** Returns number of tokens in an OUString

  @param    rIn     the input OString
  @param    cTok    the character which separate the tokens.
  @return   the number of tokens
*/
COMPHELPER_DLLPUBLIC sal_Int32 getTokenCount(std::string_view rIn, char cTok);

/** Returns number of tokens in an OUString

  @param    rIn     the input OUString
  @param    cTok    the character which separate the tokens.
  @return   the number of tokens
*/
COMPHELPER_DLLPUBLIC sal_Int32 getTokenCount(std::u16string_view rIn, sal_Unicode cTok);

/** Reverse an OUString's UTF-16 code units.

  @param    rIn     the input OUString
  @return   the reversed input
*/
COMPHELPER_DLLPUBLIC OUString reverseString(std::u16string_view rStr);

/** Reverse an OUString's Unicode code points.
*/
COMPHELPER_DLLPUBLIC OUString reverseCodePoints(std::u16string_view str);


namespace detail
{
    template<typename B> B& truncateToLength(B& rBuffer, sal_Int32 nLen)
    {
        if (nLen < rBuffer.getLength())
            rBuffer.setLength(nLen);
        return rBuffer;
    }
}

/** Truncate a buffer to a given length.

    If the StringBuffer has more characters than nLength it will be truncated
    on the right to nLength characters.

    Has no effect if the StringBuffer is <= nLength

    @param rBuf   StringBuffer to operate on
    @param nLength   Length to truncate the buffer to

    @return         rBuf;
 */
inline OUStringBuffer& truncateToLength(
    OUStringBuffer& rBuffer, sal_Int32 nLength)
{
    return detail::truncateToLength(rBuffer, nLength);
}

namespace detail
{
    template<typename B, typename U> B& padToLength(B& rBuffer, sal_Int32 nLen, U cFill)
    {
        const sal_Int32 nPadLen = nLen - rBuffer.getLength();
        if (nPadLen > 0)
            std::fill_n(rBuffer.appendUninitialized(nPadLen), nPadLen, cFill);
        return rBuffer;
    }
}

/** Pad a buffer to a given length using a given char.

    If the StringBuffer has less characters than nLength it will be expanded on
    the right to nLength characters, with the expansion filled using cFill.

    Has no effect if the StringBuffer is >= nLength

    @param rBuf   StringBuffer to operate on
    @param nLength   Length to pad the buffer to
    @param cFill  character to fill expansion with

    @return         rBuf;
 */
inline OStringBuffer& padToLength(
    OStringBuffer& rBuffer, sal_Int32 nLength,
    char cFill = '\0')
{
    return detail::padToLength(rBuffer, nLength, cFill);
}

inline OUStringBuffer& padToLength(
    OUStringBuffer& rBuffer, sal_Int32 nLength,
    sal_Unicode cFill = '\0')
{
    return detail::padToLength(rBuffer, nLength, cFill);
}

/** Similar to OUString::replaceAt, but for an OUStringBuffer.

    Replace n = count characters
    from position index in this string with newStr.
 */
COMPHELPER_DLLPUBLIC void replaceAt(OUStringBuffer& rIn, sal_Int32 index, sal_Int32 count, std::u16string_view newStr );

/** Replace a token in a string
    @param rIn       OUString in which the token is to be replaced
    @param nToken    which nToken to replace
    @param cTok      token delimiter
    @param rNewToken replacement token

    @return original string with token nToken replaced by rNewToken
 */
COMPHELPER_DLLPUBLIC OUString setToken(const OUString& rIn, sal_Int32 nToken, sal_Unicode cTok,
    std::u16string_view rNewToken);

/** Find any of a list of code units in the string.
    @param rIn      OUString to search
    @param pChars   0-terminated array of sal_Unicode code units to search for
    @param nPos     start position

    @return position of first occurrence of any of the elements of pChars
            or -1 if none of the code units occur in the string
 */
COMPHELPER_DLLPUBLIC sal_Int32 indexOfAny(std::u16string_view rIn,
        sal_Unicode const*const pChars, sal_Int32 const nPos);

/** Remove any of a list of code units in the string.
    @param rIn      OUString to search
    @param pChars   0-terminated array of sal_Unicode code units to search for

    @return OUString that has all of the pChars code units removed
 */
COMPHELPER_DLLPUBLIC OUString removeAny(std::u16string_view rIn,
        sal_Unicode const*const pChars);

/** Convert a sequence of strings to a single comma separated string.

    Note that no escaping of commas or anything fancy is done.

    @param i_rSeq   A list of strings to be concatenated.

    @return         A single string containing the concatenation of the given
                    list, interspersed with the string ", ".
 */
COMPHELPER_DLLPUBLIC OUString convertCommaSeparated(
    css::uno::Sequence< OUString > const & i_rSeq);

/// Return a string which is the concatenation of the strings in the sequence.
COMPHELPER_DLLPUBLIC OString join(std::string_view rSeparator, const std::vector<OString>& rSequence);

/** Convert a decimal string to a number.

    The string must be base-10, no sign but can contain any
    codepoint listed in the "Number, Decimal Digit" Unicode
    category.

    No verification is made about the validity of the string,
    passing string not containing decimal digit code points
    gives unspecified results

    If your string is guaranteed to contain only ASCII digit
    use OUString::toInt32 instead.

    @param str  The string to convert containing only decimal
                digit codepoints.

    @return     The value of the string as an int32.
 */
COMPHELPER_DLLPUBLIC sal_uInt32 decimalStringToNumber(
    std::u16string_view str );

COMPHELPER_DLLPUBLIC std::vector<OUString>
    split(std::u16string_view rString, const sal_Unicode cSeparator);

/** Convert a single comma separated string to a sequence of strings.

    Note that no escaping of commas or anything fancy is done.

    @param i_rString    A string containing comma-separated words.

    @return         A sequence of strings resulting from splitting the given
                    string at ',' tokens and stripping whitespace.
 */
COMPHELPER_DLLPUBLIC css::uno::Sequence< OUString >
    convertCommaSeparated( std::u16string_view i_rString );

/**
  Compares two strings using natural order.

  For non digit characters, the comparison use the same algorithm as
  rtl_str_compare. When a number is encountered during the comparison,
  natural order is used. Thus, Heading 10 will be considered as greater
  than Heading 2. Numerical comparison is done using decimal representation.

  Beware that "MyString 001" and "MyString 1" will be considered as equal
  since leading 0 are meaningless.

  @param    str         the object to be compared.
  @return   0 - if both strings are equal
            < 0 - if this string is less than the string argument
            > 0 - if this string is greater than the string argument
*/
COMPHELPER_DLLPUBLIC sal_Int32 compareNatural( const OUString &rLHS, const OUString &rRHS,
    const css::uno::Reference< css::i18n::XCollator > &rCollator,
    const css::uno::Reference< css::i18n::XBreakIterator > &rBI,
    const css::lang::Locale &rLocale );

class COMPHELPER_DLLPUBLIC NaturalStringSorter
{
private:
    css::lang::Locale const                          m_aLocale;
    css::uno::Reference< css::i18n::XCollator >      m_xCollator;
    css::uno::Reference< css::i18n::XBreakIterator > m_xBI;
public:
    NaturalStringSorter(
        const css::uno::Reference< css::uno::XComponentContext > &rContext,
        css::lang::Locale aLocale);
    sal_Int32 compare(const OUString &rLHS, const OUString &rRHS) const
    {
        return compareNatural(rLHS, rRHS, m_xCollator, m_xBI, m_aLocale);
    }
    const css::lang::Locale& getLocale() const { return m_aLocale; }
};

/** Determine if an OString contains solely ASCII numeric digits

    @param rString  An OString

    @return         false if string contains any characters outside
                    the ASCII '0'-'9' range
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool isdigitAsciiString(std::string_view rString);

/** Determine if an OUString contains solely ASCII numeric digits

    @param rString  An OUString

    @return         false if string contains any characters outside
                    the ASCII '0'-'9' range
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool isdigitAsciiString(std::u16string_view rString);

/** Sanitize an OUString to not have invalid surrogates

    @param rString  An OUString

    @return         same string if no surrogates or surrogates are valid.
                    Otherwise the string truncated to the valid sequence.
 */
COMPHELPER_DLLPUBLIC OUString sanitizeStringSurrogates(const OUString& rString);

} // namespace comphelper::string

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
