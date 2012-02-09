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

#ifndef INCLUDED_COMPHELPER_STRING_HXX
#define INCLUDED_COMPHELPER_STRING_HXX

#include "sal/config.h"

#include <cstddef>
#include "comphelper/comphelperdllapi.h"
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>

// rtl::OUString helper functions that are not widespread or mature enough to
// go into the stable URE API:
namespace comphelper { namespace string {

namespace detail
{
    template <typename T, typename U> T* string_alloc(sal_Int32 nLen)
    {
        //Clearly this is somewhat cosy with the sal implmentation

        //rtl_[u]String contains U buffer[1], so an input of nLen
        //allocates a buffer of nLen + 1 and we'll ensure a null termination
        T *newStr = (T*)rtl_allocateMemory(sizeof(T) + sizeof(U) * nLen);
        newStr->refCount = 1;
        newStr->length = nLen;
        newStr->buffer[nLen]=0;
        return newStr;
    }
}

/** Allocate a new string containing space for a given number of characters.

    The reference count of the new string will be 1. The length of the string
    will be nLen. This function does not handle out-of-memory conditions.

    The characters of the capacity are not cleared, and the length is set to
    nLen, unlike the similar method of rtl_uString_new_WithLength which
    zeros out the buffer, and sets the length to 0. So should be somewhat
    more efficient for allocating a new string.

    call rtl_uString_release to release the string
    alternatively pass ownership to an OUString with
    rtl::OUString(newStr, SAL_NO_ACQUIRE);

    @param newStr
    pointer to the new string.

    @param len
    the number of characters.
 */
COMPHELPER_DLLPUBLIC inline rtl_uString * SAL_CALL rtl_uString_alloc(sal_Int32 nLen)
{
    return detail::string_alloc<rtl_uString, sal_Unicode>(nLen);
}

/** Allocate a new string containing space for a given number of characters.

    The reference count of the new string will be 1. The length of the string
    will be nLen. This function does not handle out-of-memory conditions.

    The characters of the capacity are not cleared, and the length is set to
    nLen, unlike the similar method of rtl_String_new_WithLength which
    zeros out the buffer, and sets the length to 0. So should be somewhat
    more efficient for allocating a new string.

    call rtl_String_release to release the string
    alternatively pass ownership to an OUString with
    rtl::OUString(newStr, SAL_NO_ACQUIRE);

    @param newStr
    pointer to the new string.

    @param len
    the number of characters.
 */
COMPHELPER_DLLPUBLIC inline rtl_String * SAL_CALL rtl_string_alloc(sal_Int32 nLen)
{
    return detail::string_alloc<rtl_String, sal_Char>(nLen);
}

/**
   Replace the first occurrence of a substring with another string.

   @param source
   The source string, in which the search will take place.

   @param from
   The ASCII substring to search for.  Must point to at least fromLength ASCII
   characters.

   @param fromLength
   The length of the from substring.  Must not be negative.

   @param to
   The string to use as replacement.

   @param beginAt
   The index at which to begin the search.  Must be between zero and the length
   of source, inclusive.

   @param replacedAt
   If non-null, receives the starting index at which the replacement took place
   or -1 if from was not found.

   @return
   The resulting string, in which the replacement has taken place.
*/
COMPHELPER_DLLPUBLIC rtl::OUString searchAndReplaceAsciiL(
    rtl::OUString const & source, char const * from, sal_Int32 fromLength,
    rtl::OUString const & to, sal_Int32 beginAt = 0,
    sal_Int32 * replacedAt = NULL);

/** replaces, in the given source string, all occurrences of a given ASCII pattern
    with another ASCII pattern
*/
COMPHELPER_DLLPUBLIC ::rtl::OUString searchAndReplaceAllAsciiWithAscii(
    const ::rtl::OUString& source, const sal_Char* from, const sal_Char* to,
    const sal_Int32 beginAt = 0 );

/** does an in-place replacement of the first occurrence of a sub string with
    another string

    @param source
        the string to search and replace in.
    @param asciiPattern
        the ASCII sub string to search for. Must point to a 0-terminated string.
    @param replace
        The string to use as replacement.
    @param beginAt
        The index at which to begin the search.  Must be between zero and the length
        of source, inclusive.

    @param replacedAt
        If non-null, receives the starting index at which the replacement took place
        or -1 if from was not found.

    @return
        a reference to <code>source</code>
*/
COMPHELPER_DLLPUBLIC ::rtl::OUString&
    searchAndReplaceAsciiI( ::rtl::OUString & source, sal_Char const * asciiPattern,
                            ::rtl::OUString const & replace, sal_Int32 beginAt = 0,
                            sal_Int32 * replacedAt = NULL );

/** Replaces each substring of this OString that matches the search OString
    with the specified replacement OString

    @param rIn      The input OString
    @param rSearch  The substring to be replaced
    @param rReplace The replacement substring

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC rtl::OString replace(const rtl::OString &rIn,
    const rtl::OString &rSearch, const rtl::OString &rReplace);

/** Replaces each substring of this OUString that matches the search OUString
    with the specified replacement OUString

    @param rIn      The input OUString
    @param rSearch  The substring to be replaced
    @param rReplace The replacement substring

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC rtl::OUString replace(const rtl::OUString &rIn,
    const rtl::OUString &rSearch, const rtl::OUString &rReplace);

/** Removes all occurrences of a character from within the source string

    @param rIn      The input OString
    @param c        The character to be removed

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC rtl::OString remove(const rtl::OString &rIn,
    sal_Char c);

/** Removes all occurrences of a character from within the source string

    @param rIn      The input OUString
    @param c        The character to be removed

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC rtl::OUString remove(const rtl::OUString &rIn,
    sal_Unicode c);

/** Strips occurrences of a character from the start of the source string

    @param rIn      The input OString
    @param c        The character to be stripped from the start

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC rtl::OString stripStart(const rtl::OString &rIn,
    sal_Char c);

/** Strips occurrences of a character from the start of the source string

    @param rIn      The input OUString
    @param c        The character to be stripped from the start

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC rtl::OUString stripStart(const rtl::OUString &rIn,
    sal_Unicode c);

/** Strips occurrences of a character from the end of the source string

    @param rIn      The input OString
    @param c        The character to be stripped from the end

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC rtl::OString stripEnd(const rtl::OString &rIn,
    sal_Char c);

/** Strips occurrences of a character from the end of the source string

    @param rIn      The input OUString
    @param c        The character to be stripped from the end

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC rtl::OUString stripEnd(const rtl::OUString &rIn,
    sal_Unicode c);

/** Strips occurrences of a character from the start and end of the source string

    @param rIn      The input OString
    @param c        The character to be stripped from the start and end

    @return         The resulting OString
 */
COMPHELPER_DLLPUBLIC rtl::OString strip(const rtl::OString &rIn,
    sal_Char c);

/** Strips occurrences of a character from the start and end of the source string

    @param rIn      The input OUString
    @param c        The character to be stripped from the start and end

    @return         The resulting OUString
 */
COMPHELPER_DLLPUBLIC rtl::OUString strip(const rtl::OUString &rIn,
    sal_Unicode c);

/** Returns a token in an OString

  @param    rIn         the input OString
  @param    nToken      the number of the token to return
  @param    cTok        the character which seperate the tokens.
  @return   the token   if token is negative or doesn't exist an empty token
                        is returned
*/
COMPHELPER_DLLPUBLIC inline rtl::OString getToken(const rtl::OString &rIn,
    sal_Int32 nToken, sal_Char cTok) SAL_THROW(())
{
    sal_Int32 nIndex = 0;
    return rIn.getToken(nToken, cTok, nIndex);
}

/** Returns a token in an OUString

  @param    rIn         the input OUString
  @param    nToken      the number of the token to return
  @param    cTok        the character which seperate the tokens.
  @return   the token   if token is negative or doesn't exist an empty token
                        is returned
*/
COMPHELPER_DLLPUBLIC inline rtl::OUString getToken(const rtl::OUString &rIn,
    sal_Int32 nToken, sal_Unicode cTok) SAL_THROW(())
{
    sal_Int32 nIndex = 0;
    return rIn.getToken(nToken, cTok, nIndex);
}

/** Returns number of tokens in an OUString

  @param    rIn     the input OString
  @param    cTok    the character which seperate the tokens.
  @return   the number of tokens
*/
COMPHELPER_DLLPUBLIC sal_Int32 getTokenCount(const rtl::OString &rIn, sal_Char cTok);

/** Returns number of tokens in an OUString

  @param    rIn     the input OUString
  @param    cTok    the character which seperate the tokens.
  @return   the number of tokens
*/
COMPHELPER_DLLPUBLIC sal_Int32 getTokenCount(const rtl::OUString &rIn, sal_Unicode cTok);

/**
  Match against a substring appearing in another string.

  The result is true if and only if the second string appears as a substring
  of the first string, at the given position.
  This function can't be used for language specific comparison.

  @param    rStr        The string that pMatch will be compared to.
  @param    pMatch      The substring rStr is to be compared against
  @param    nMatchLen   The length of pMatch
  @param    fromIndex   The index to start the comparion from.
                        The index must be greater or equal than 0
                        and less or equal as the string length.
  @return   sal_True if pMatch match with the characters in the string
            at the given position;
            sal_False, otherwise.
*/
COMPHELPER_DLLPUBLIC inline sal_Bool matchL(const rtl::OString& rStr, const char *pMatch, sal_Int32 nMatchLen, sal_Int32 fromIndex = 0) SAL_THROW(())
{
    return rtl_str_shortenedCompare_WithLength( rStr.pData->buffer+fromIndex,
        rStr.pData->length-fromIndex, pMatch, nMatchLen, nMatchLen ) == 0;
}

/**
  Match against a substring appearing in this string, ignoring the case of
  ASCII letters.

  The result is true if and only if the second string appears as a substring
  of this string, at the given position.
  Character values between 65 and 90 (ASCII A-Z) are interpreted as
  values between 97 and 122 (ASCII a-z).
  This function can't be used for language specific comparison.

  @param    rStr        The string that pMatch will be compared to.
  @param    pMatch      The substring rStr is to be compared against
  @param    nMatchLen   The length of pMatch
  @param    fromIndex   the index to start the comparion from.
                        The index must be greater or equal than 0
                        and less or equal as the string length.
  @return   sal_True if str match with the characters in the string
            at the given position;
            sal_False, otherwise.
*/
COMPHELPER_DLLPUBLIC inline sal_Bool matchIgnoreAsciiCaseL(const rtl::OString& rStr, const char *pMatch, sal_Int32 nMatchLen, sal_Int32 fromIndex = 0) SAL_THROW(())
{
    return rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( rStr.pData->buffer+fromIndex, rStr.pData->length-fromIndex,
                                                               pMatch, nMatchLen,
                                                               nMatchLen ) == 0;
}

/**
  Returns the index within this string of the first occurrence of the
  specified substring, starting at the specified index.

  If str doesn't include any character, always -1 is
  returned. This is also the case, if both strings are empty.

  @param    rStr        The string that pSearch will be searched within.
  @param    pSearch     the substring to search for.
  @param    nSearchLen  the length of pSearch
  @param    fromIndex   the index to start the search from.
  @return   If the string argument occurs one or more times as a substring
            within this string at the starting index, then the index
            of the first character of the first such substring is
            returned. If it does not occur as a substring starting
            at fromIndex or beyond, -1 is returned.
*/
COMPHELPER_DLLPUBLIC inline sal_Int32 indexOfL(const rtl::OString& rStr, const char *pSearch, sal_Int32 nSearchLen, sal_Int32 fromIndex = 0) SAL_THROW(())
{
    sal_Int32 ret = rtl_str_indexOfStr_WithLength(rStr.pData->buffer+fromIndex,
        rStr.pData->length-fromIndex, pSearch, nSearchLen);
    return (ret < 0 ? ret : ret+fromIndex);
}

namespace detail
{
    template<typename B> B& truncateToLength(B& rBuffer, sal_Int32 nLen)
    {
        if (nLen < rBuffer.getLength())
            rBuffer.remove(nLen, rBuffer.getLength()-nLen);
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
COMPHELPER_DLLPUBLIC inline rtl::OStringBuffer& truncateToLength(
    rtl::OStringBuffer& rBuffer, sal_Int32 nLength) SAL_THROW(())
{
    return detail::truncateToLength(rBuffer, nLength);
}

COMPHELPER_DLLPUBLIC inline rtl::OUStringBuffer& truncateToLength(
    rtl::OUStringBuffer& rBuffer, sal_Int32 nLength) SAL_THROW(())
{
    return detail::truncateToLength(rBuffer, nLength);
}

namespace detail
{
    template<typename B, typename U> B& padToLength(B& rBuffer, sal_Int32 nLen,
        U cFill = '\0')
    {
        sal_Int32 nOrigLen = rBuffer.getLength();
        if (nLen > nOrigLen)
        {
            rBuffer.setLength(nLen);
            for (sal_Int32 i = nOrigLen; i < nLen; ++i)
                rBuffer[i] = cFill;
        }
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
COMPHELPER_DLLPUBLIC inline rtl::OStringBuffer& padToLength(
    rtl::OStringBuffer& rBuffer, sal_Int32 nLength,
    sal_Char cFill = '\0') SAL_THROW(())
{
    return detail::padToLength(rBuffer, nLength, cFill);
}

COMPHELPER_DLLPUBLIC inline rtl::OUStringBuffer& padToLength(
    rtl::OUStringBuffer& rBuffer, sal_Int32 nLength,
    sal_Unicode cFill = '\0') SAL_THROW(())
{
    return detail::padToLength(rBuffer, nLength, cFill);
}

COMPHELPER_DLLPUBLIC rtl::OUString removeTrailingChars(
    const rtl::OUString& rStr, sal_Unicode cChar);

/** Convert a sequence of strings to a single comma separated string.

    Note that no escaping of commas or anything fancy is done.

    @param i_rSeq   A list of strings to be concatenated.

    @return         A single string containing the concatenation of the given
                    list, interspersed with the string ", ".
 */
COMPHELPER_DLLPUBLIC ::rtl::OUString convertCommaSeparated(
    ::com::sun::star::uno::Sequence< ::rtl::OUString > const & i_rSeq);

/** Convert a decimal string to a number.

    The string must be base-10, no sign but can contain any
    codepoint listed in the "Number, Decimal Digit" Unicode
    category.

    No verification is made about the validity of the string,
    passing string not containing decimal digit code points
    gives unspecified results

    If your string is guaranteed to contain only ASCII digit
    use rtl::OUString::toInt32 instead.

    @param str  The string to convert containing only decimal
                digit codepoints.

    @return     The value of the string as an int32.
 */
COMPHELPER_DLLPUBLIC sal_uInt32 decimalStringToNumber(
    ::rtl::OUString const & str );

/** Convert a single comma separated string to a sequence of strings.

    Note that no escaping of commas or anything fancy is done.

    @param i_rString    A string containing comma-separated words.

    @return         A sequence of strings resulting from splitting the given
                    string at ',' tokens and stripping whitespace.
 */
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Sequence< ::rtl::OUString >
    convertCommaSeparated( ::rtl::OUString const & i_rString );

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
COMPHELPER_DLLPUBLIC sal_Int32 compareNatural( const ::rtl::OUString &rLHS, const ::rtl::OUString &rRHS,
    const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator > &rCollator,
    const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > &rBI,
    const ::com::sun::star::lang::Locale &rLocale );

class COMPHELPER_DLLPUBLIC NaturalStringSorter
{
private:
    ::com::sun::star::lang::Locale m_aLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator > m_xCollator;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > m_xBI;
public:
    NaturalStringSorter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext,
        const ::com::sun::star::lang::Locale &rLocale);
    sal_Int32 compare(const rtl::OUString &rLHS, const rtl::OUString &rRHS) const
    {
        return compareNatural(rLHS, rRHS, m_xCollator, m_xBI, m_aLocale);
    }
    const ::com::sun::star::lang::Locale& getLocale() const { return m_aLocale; }
};

/** Determine if an OString contains solely ASCII numeric digits

    @param rString  An OString

    @return         false if string contains any characters outside
                    the ASCII '0'-'9' range
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool isdigitAsciiString(const rtl::OString &rString);

/** Determine if an OUString contains solely ASCII numeric digits

    @param rString  An OUString

    @return         false if string contains any characters outside
                    the ASCII '0'-'9' range
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool isdigitAsciiString(const rtl::OUString &rString);

/** Determine if an OString contains solely ASCII alphanumeric chars/digits

    @param rString  An OString

    @return         false if string contains any characters outside
                    the ASCII 'a'-'z', 'A'-'Z' and '0'-'9' ranges
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool isalnumAsciiString(const rtl::OString &rString);

/** Determine if an OString contains solely ASCII lower-case chars

    @param rString  An OString

    @return         false if string contains any characters outside
                    the ASCII 'a'-'z' ranges
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool islowerAsciiString(const rtl::OString &rString);

/** Determine if an OString contains solely ASCII upper-case chars

    @param rString  An OString

    @return         false if string contains any characters outside
                    the ASCII 'A'-'Z' ranges
                    true otherwise, including for empty string
 */
COMPHELPER_DLLPUBLIC bool isupperAsciiString(const rtl::OString &rString);

COMPHELPER_DLLPUBLIC inline bool isdigitAscii(sal_Unicode c)
{
    return ((c >= '0') && (c <= '9'));
}

COMPHELPER_DLLPUBLIC inline bool isxdigitAscii(sal_Unicode c)
{
    return isdigitAscii(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

COMPHELPER_DLLPUBLIC inline bool islowerAscii(sal_Unicode c)
{
    return ((c >= 'a') && (c <= 'z'));
}

COMPHELPER_DLLPUBLIC inline bool isupperAscii(sal_Unicode c)
{
    return ((c >= 'A') && (c <= 'Z'));
}

COMPHELPER_DLLPUBLIC inline bool isalphaAscii(sal_Unicode c)
{
    return islowerAscii(c) || isupperAscii(c);
}

COMPHELPER_DLLPUBLIC inline bool isalnumAscii(sal_Unicode c)
{
    return isalphaAscii(c) || isdigitAscii(c);
}

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
