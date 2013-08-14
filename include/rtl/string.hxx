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

#ifndef _RTL_STRING_HXX_
#define _RTL_STRING_HXX_

#include "sal/config.h"

#include <cassert>
#include <new>
#include <ostream>
#include <string.h>

#include <osl/diagnose.h>
#include <rtl/textenc.h>
#include <rtl/string.h>
#include <rtl/stringutils.hxx>

#ifdef RTL_FAST_STRING
#include <rtl/stringconcat.hxx>
#endif

#include "sal/log.hxx"

// The unittest uses slightly different code to help check that the proper
// calls are made. The class is put into a different namespace to make
// sure the compiler generates a different (if generating also non-inline)
// copy of the function and does not merge them together. The class
// is "brought" into the proper rtl namespace by a typedef below.
#ifdef RTL_STRING_UNITTEST
#define rtl rtlunittest
#endif

namespace rtl
{

/// @cond INTERNAL
#ifdef RTL_STRING_UNITTEST
#undef rtl
// helper macro to make functions appear more readable
#define RTL_STRING_CONST_FUNCTION rtl_string_unittest_const_literal_function = true;
#else
#define RTL_STRING_CONST_FUNCTION
#endif
/// @endcond

/* ======================================================================= */

/**
  This String class provide base functionality for C++ like 8-Bit
  character array handling. The advantage of this class is, that it
  handle all the memory managament for you - and it do it
  more efficient. If you assign a string to another string, the
  data of both strings are shared (without any copy operation or
  memory allocation) as long as you do not change the string. This class
  stores also the length of the string, so that many operations are
  faster as the C-str-functions.

  This class provide only readonly string handling. So you could create
  a string and you could only query the content from this string.
  It provide also functionality to change the string, but this results
  in every case in a new string instance (in the most cases with an
  memory allocation). You don't have functionality to change the
  content of the string. If you want change the string content, than
  you should us the OStringBuffer class, which provide these
  functionality and avoid to much memory allocation.

  The design of this class is similar to the string classes in Java
  and so more people should have fewer understanding problems when they
  use this class.
*/

class SAL_WARN_UNUSED OString
{
public:
    /// @cond INTERNAL
    rtl_String * pData;
    /// @endcond

    /**
      New string containing no characters.
    */
    OString() SAL_THROW(())
    {
        pData = 0;
        rtl_string_new( &pData );
    }

    /**
      New string from OString.

      @param    str         a OString.
    */
    OString( const OString & str ) SAL_THROW(())
    {
        pData = str.pData;
        rtl_string_acquire( pData );
    }

    /**
      New string from OString data.

      @param    str         a OString data.
    */
    OString( rtl_String * str ) SAL_THROW(())
    {
        pData = str;
        rtl_string_acquire( pData );
    }

    /** New string from OString data without acquiring it.  Takeover of ownership.

        The SAL_NO_ACQUIRE dummy parameter is only there to distinguish this
        from other constructors.

      @param    str         a OString data.
    */
    inline OString( rtl_String * str, __sal_NoAcquire ) SAL_THROW(())
    {
        pData = str;
    }

    /**
      New string from a single character.

      @param    value       a character.
    */
    explicit OString( sal_Char value ) SAL_THROW(())
        : pData (0)
    {
        rtl_string_newFromStr_WithLength( &pData, &value, 1 );
    }

    /**
      New string from a character buffer array.

      Note: The argument type is always either char* or const char*. The template is
      used only for technical reasons, as is the second argument.

      @param    value       a NULL-terminated character array.
    */
    template< typename T >
    OString( const T& value, typename internal::CharPtrDetector< T, internal::Dummy >::Type = internal::Dummy() ) SAL_THROW(())
    {
        pData = 0;
        rtl_string_newFromStr( &pData, value );
    }

    template< typename T >
    OString( T& value, typename internal::NonConstCharArrayDetector< T, internal::Dummy >::Type = internal::Dummy() ) SAL_THROW(())
    {
        pData = 0;
        rtl_string_newFromStr( &pData, value );
    }

    /**
      New string from a string literal.

      If there are any embedded \0's in the string literal, the result is undefined.
      Use the overload that explicitly accepts length.

      @since LibreOffice 3.6

      @param    literal       a string literal
    */
    template< typename T >
    OString( T& literal, typename internal::ConstCharArrayDetector< T, internal::Dummy >::Type = internal::Dummy() ) SAL_THROW(())
    {
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        pData = 0;
        if( internal::ConstCharArrayDetector< T, void >::size - 1 == 0 ) // empty string
            rtl_string_new( &pData );
        else
            rtl_string_newFromLiteral( &pData, literal, internal::ConstCharArrayDetector< T, void >::size - 1, 0 );
#ifdef RTL_STRING_UNITTEST
        rtl_string_unittest_const_literal = true;
#endif
    }

    /**
      New string from a character buffer array.

      @param    value       a character array.
      @param    length      the number of character which should be copied.
                            The character array length must be greater or
                            equal than this value.
    */
    OString( const sal_Char * value, sal_Int32 length ) SAL_THROW(())
    {
        pData = 0;
        rtl_string_newFromStr_WithLength( &pData, value, length );
    }

    /**
      New string from a Unicode character buffer array.

      @param    value           a Unicode character array.
      @param    length          the number of character which should be converted.
                                The Unicode character array length must be
                                greater or equal than this value.
      @param    encoding        the text encoding in which the Unicode character
                                sequence should be converted.
      @param    convertFlags    flags which controls the conversion.
                                see RTL_UNICODETOTEXT_FLAGS_...

      @exception std::bad_alloc is thrown if an out-of-memory condition occurs
    */
    OString( const sal_Unicode * value, sal_Int32 length,
             rtl_TextEncoding encoding,
             sal_uInt32 convertFlags = OUSTRING_TO_OSTRING_CVTFLAGS )
    {
        pData = 0;
        rtl_uString2String( &pData, value, length, encoding, convertFlags );
        if (pData == 0) {
            throw std::bad_alloc();
        }
    }

#ifdef RTL_FAST_STRING
    /**
     @overload
     @internal
    */
    template< typename T1, typename T2 >
    OString( const OStringConcat< T1, T2 >& c )
    {
        const sal_Int32 l = c.length();
        pData = rtl_string_alloc( l );
        if (l != 0)
        {
            char* end = c.addData( pData->buffer );
            pData->length = end - pData->buffer;
            *end = '\0';
        }
    }
#endif

    /**
      Release the string data.
    */
    ~OString() SAL_THROW(())
    {
        rtl_string_release( pData );
    }

    /**
      Assign a new string.

      @param    str         a OString.
    */
    OString & operator=( const OString & str ) SAL_THROW(())
    {
        rtl_string_assign( &pData, str.pData );
        return *this;
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, OString& >::Type operator=( T& literal ) SAL_THROW(())
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        if( internal::ConstCharArrayDetector< T, void >::size - 1 == 0 ) // empty string
            rtl_string_new( &pData );
        else
            rtl_string_newFromLiteral( &pData, literal, internal::ConstCharArrayDetector< T, void >::size - 1, 0 );
        return *this;
    }

    /**
      Append a string to this string.

      @param    str         a OString.
    */
    OString & operator+=( const OString & str ) SAL_THROW(())
    {
        rtl_string_newConcat( &pData, pData, str.pData );
        return *this;
    }

#ifdef RTL_FAST_STRING
    /**
     @overload
     @internal
    */
    template< typename T1, typename T2 >
    OString& operator+=( const OStringConcat< T1, T2 >& c )
    {
        const int l = c.length();
        if( l == 0 )
            return *this;
        rtl_string_ensureCapacity( &pData, pData->length + l );
        char* end = c.addData( pData->buffer + pData->length );
        *end = '\0';
        pData->length = end - pData->buffer;
        return *this;
    }
#endif
    /**
      Returns the length of this string.

      The length is equal to the number of characters in this string.

      @return   the length of the sequence of characters represented by this
                object.
    */
    sal_Int32 getLength() const SAL_THROW(()) { return pData->length; }

    /**
      Checks if a string is empty.

      @return   true if the string is empty;
                false, otherwise.

      @since LibreOffice 3.4
    */
    bool isEmpty() const SAL_THROW(())
    {
        return pData->length == 0;
    }

    /**
      Returns a pointer to the characters of this string.

      <p>The returned pointer is guaranteed to point to a null-terminated byte
      string.  But note that this string object may contain embedded null
      characters, which will thus also be embedded in the returned
      null-terminated byte string.</p>

      @return a pointer to a null-terminated byte string representing the
      characters of this string object.
    */
    const sal_Char * getStr() const SAL_THROW(()) { return pData->buffer; }

    /**
      Access to individual characters.

      @param index must be non-negative and less than length.

      @return the character at the given index.

      @since LibreOffice 3.5
    */
    sal_Char operator [](sal_Int32 index) const {
        assert(index >= 0 && index <= getLength());
            //TODO: should really check for < getLength(), but there is quite
            // some clever code out there that violates this function's
            // documented precondition and relies on s[s.getLength()] == 0 and
            // that would need to be fixed first
        return getStr()[index];
    }

    /**
      Compares two strings.

      The comparison is based on the numeric value of each character in
      the strings and return a value indicating their relationship.
      This function can't be used for language specific sorting.

      @param    str         the object to be compared.
      @return   0 - if both strings are equal
                < 0 - if this string is less than the string argument
                > 0 - if this string is greater than the string argument
    */
    sal_Int32 compareTo( const OString & str ) const SAL_THROW(())
    {
        return rtl_str_compare_WithLength( pData->buffer, pData->length,
                                           str.pData->buffer, str.pData->length );
    }

    /**
      Compares two strings with an maximum count of characters.

      The comparison is based on the numeric value of each character in
      the strings and return a value indicating their relationship.
      This function can't be used for language specific sorting.

      @param    rObj        the object to be compared.
      @param    maxLength   the maximum count of characters to be compared.
      @return   0 - if both strings are equal
                < 0 - if this string is less than the string argument
                > 0 - if this string is greater than the string argument
    */
    sal_Int32 compareTo( const OString & rObj, sal_Int32 maxLength ) const SAL_THROW(())
    {
        return rtl_str_shortenedCompare_WithLength( pData->buffer, pData->length,
                                                    rObj.pData->buffer, rObj.pData->length, maxLength );
    }

    /**
      Compares two strings in reverse order.

      The comparison is based on the numeric value of each character in
      the strings and return a value indicating their relationship.
      This function can't be used for language specific sorting.

      @param    str         the object to be compared.
      @return   0 - if both strings are equal
                < 0 - if this string is less than the string argument
                > 0 - if this string is greater than the string argument
    */
    sal_Int32 reverseCompareTo( const OString & str ) const SAL_THROW(())
    {
        return rtl_str_reverseCompare_WithLength( pData->buffer, pData->length,
                                                  str.pData->buffer, str.pData->length );
    }

    /**
      Perform a comparison of two strings.

      The result is true if and only if second string
      represents the same sequence of characters as the first string.
      This function can't be used for language specific comparison.

      @param    str         the object to be compared.
      @return   sal_True if the strings are equal;
                sal_False, otherwise.
    */
    sal_Bool equals( const OString & str ) const SAL_THROW(())
    {
        if ( pData->length != str.pData->length )
            return sal_False;
        if ( pData == str.pData )
            return sal_True;
        return rtl_str_reverseCompare_WithLength( pData->buffer, pData->length,
                                                  str.pData->buffer, str.pData->length ) == 0;
    }

    /**
      Perform a comparison of two strings.

      The result is true if and only if second string
      represents the same sequence of characters as the first string.
      The ASCII string must be NULL-terminated and must be greater or
      equal as length.
      This function can't be used for language specific comparison.


      @param    value       a character array.
      @param    length      the length of the character array.
      @return   sal_True if the strings are equal;
                sal_False, otherwise.
    */
    sal_Bool equalsL( const sal_Char* value, sal_Int32 length ) const SAL_THROW(())
    {
        if ( pData->length != length )
            return sal_False;

        return rtl_str_reverseCompare_WithLength( pData->buffer, pData->length,
                                                  value, length ) == 0;
    }

    /**
      Perform a ASCII lowercase comparison of two strings.

      The result is true if and only if second string
      represents the same sequence of characters as the first string,
      ignoring the case.
      Character values between 65 and 90 (ASCII A-Z) are interpreted as
      values between 97 and 122 (ASCII a-z).
      This function can't be used for language specific comparison.

      @param    str         the object to be compared.
      @return   sal_True if the strings are equal;
                sal_False, otherwise.
    */
    sal_Bool equalsIgnoreAsciiCase( const OString & str ) const SAL_THROW(())
    {
        if ( pData->length != str.pData->length )
            return sal_False;
        if ( pData == str.pData )
            return sal_True;
        return rtl_str_compareIgnoreAsciiCase_WithLength( pData->buffer, pData->length,
                                                          str.pData->buffer, str.pData->length ) == 0;
    }

    /**
      Perform a ASCII lowercase comparison of two strings.

      The result is true if and only if second string
      represents the same sequence of characters as the first string,
      ignoring the case.
      Character values between 65 and 90 (ASCII A-Z) are interpreted as
      values between 97 and 122 (ASCII a-z).
      Since this method is optimized for performance, the ASCII character
      values are not converted in any way. The caller has to make sure that
      all ASCII characters are in the allowed range between 0 and
      127. The ASCII string must be NULL-terminated.
      This function can't be used for language specific comparison.

      Note: The argument type is always either char* or const char*, the return type is bool.
      The template is used only for technical reasons.

      @param    asciiStr        the 8-Bit ASCII character string to be compared.
      @return   sal_True if the strings are equal;
                sal_False, otherwise.
    */
    template< typename T >
    typename internal::CharPtrDetector< T, bool >::Type equalsIgnoreAsciiCase( const T& asciiStr ) const SAL_THROW(())
    {
        return rtl_str_compareIgnoreAsciiCase( pData->buffer, asciiStr ) == 0;
    }

    template< typename T >
    typename internal::NonConstCharArrayDetector< T, bool >::Type equalsIgnoreAsciiCase( T& asciiStr ) const SAL_THROW(())
    {
        return rtl_str_compareIgnoreAsciiCase( pData->buffer, asciiStr ) == 0;
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, bool >::Type  equalsIgnoreAsciiCase( T& literal ) const SAL_THROW(())
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        if ( pData->length != internal::ConstCharArrayDetector< T, void >::size - 1 )
            return false;
        return rtl_str_compareIgnoreAsciiCase_WithLength( pData->buffer, pData->length,
                                                          literal, internal::ConstCharArrayDetector< T, void >::size - 1 ) == 0;
    }

    /**
      Perform a ASCII lowercase comparison of two strings.

      The result is true if and only if second string
      represents the same sequence of characters as the first string,
      ignoring the case.
      Character values between 65 and 90 (ASCII A-Z) are interpreted as
      values between 97 and 122 (ASCII a-z).
      Since this method is optimized for performance, the ASCII character
      values are not converted in any way. The caller has to make sure that
      all ASCII characters are in the allowed range between 0 and
      127. The ASCII string must be greater or equal in length as asciiStrLength.
      This function can't be used for language specific comparison.

      @param    asciiStr        the 8-Bit ASCII character string to be compared.
      @param    asciiStrLength  the length of the ascii string
      @return   sal_True if the strings are equal;
                sal_False, otherwise.
    */
    sal_Bool equalsIgnoreAsciiCaseL( const sal_Char * asciiStr, sal_Int32 asciiStrLength ) const SAL_THROW(())
    {
        if ( pData->length != asciiStrLength )
            return sal_False;

        return rtl_str_compareIgnoreAsciiCase_WithLength( pData->buffer, pData->length,
                                                          asciiStr, asciiStrLength ) == 0;
    }

    /**
      Match against a substring appearing in this string.

      The result is true if and only if the second string appears as a substring
      of this string, at the given position.
      This function can't be used for language specific comparison.

      @param    str         the object (substring) to be compared.
      @param    fromIndex   the index to start the comparion from.
                            The index must be greater or equal than 0
                            and less or equal as the string length.
      @return   sal_True if str match with the characters in the string
                at the given position;
                sal_False, otherwise.
    */
    sal_Bool match( const OString & str, sal_Int32 fromIndex = 0 ) const SAL_THROW(())
    {
        return rtl_str_shortenedCompare_WithLength( pData->buffer+fromIndex, pData->length-fromIndex,
                                                    str.pData->buffer, str.pData->length, str.pData->length ) == 0;
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, bool >::Type  match( T& literal, sal_Int32 fromIndex = 0 ) const SAL_THROW(())
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        return rtl_str_shortenedCompare_WithLength(
            pData->buffer + fromIndex, pData->length - fromIndex,
            literal, internal::ConstCharArrayDetector< T, void >::size - 1, internal::ConstCharArrayDetector< T, void >::size - 1) == 0;
    }

    /**
      Match against a substring appearing in this string.

      @param str  the substring to be compared; must not be null and must point
      to memory of at least strLength bytes

      @param strLength  the length of the substring; must be non-negative

      @param fromIndex  the index into this string to start the comparison at;
      must be non-negative and not greater than this string's length

      @return true if and only if the given str is contained as a substring of
      this string at the given fromIndex

      @since LibreOffice 3.6
    */
    bool matchL(
        char const * str, sal_Int32 strLength, sal_Int32 fromIndex = 0)
        const
    {
        return rtl_str_shortenedCompare_WithLength(
            pData->buffer + fromIndex, pData->length - fromIndex,
            str, strLength, strLength) == 0;
    }

    // This overload is left undefined, to detect calls of matchL that
    // erroneously use RTL_CONSTASCII_USTRINGPARAM instead of
    // RTL_CONSTASCII_STRINGPARAM (but would lead to ambiguities on 32 bit
    // platforms):
#if SAL_TYPES_SIZEOFLONG == 8
    void matchL(char const *, sal_Int32, rtl_TextEncoding) const;
#endif

    /**
      Match against a substring appearing in this string, ignoring the case of
      ASCII letters.

      The result is true if and only if the second string appears as a substring
      of this string, at the given position.
      Character values between 65 and 90 (ASCII A-Z) are interpreted as
      values between 97 and 122 (ASCII a-z).
      This function can't be used for language specific comparison.

      @param    str         the object (substring) to be compared.
      @param    fromIndex   the index to start the comparion from.
                            The index must be greater or equal than 0
                            and less or equal as the string length.
      @return   sal_True if str match with the characters in the string
                at the given position;
                sal_False, otherwise.
    */
    sal_Bool matchIgnoreAsciiCase( const OString & str, sal_Int32 fromIndex = 0 ) const SAL_THROW(())
    {
        return rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( pData->buffer+fromIndex, pData->length-fromIndex,
                                                                   str.pData->buffer, str.pData->length,
                                                                   str.pData->length ) == 0;
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, bool >::Type matchIgnoreAsciiCase( T& literal, sal_Int32 fromIndex = 0 ) const
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        return rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( pData->buffer+fromIndex, pData->length-fromIndex,
            literal, internal::ConstCharArrayDetector< T, void >::size - 1, internal::ConstCharArrayDetector< T, void >::size - 1 ) == 0;
    }

    /**
      Check whether this string starts with a given substring.

      @param str  the substring to be compared

      @return true if and only if the given str appears as a substring at the
      start of this string

      @since LibreOffice 4.0
    */
    bool startsWith(OString const & str) const {
        return match(str, 0);
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 4.0
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, bool >::Type startsWith( T& literal ) const
    {
        RTL_STRING_CONST_FUNCTION
        return match(literal, 0);
    }

    /**
      Check whether this string ends with a given substring.

      @param str  the substring to be compared

      @return true if and only if the given str appears as a substring at the
      end of this string

      @since LibreOffice 3.6
    */
    bool endsWith(OString const & str) const {
        return str.getLength() <= getLength()
            && match(str, getLength() - str.getLength());
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, bool >::Type endsWith( T& literal ) const
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        return internal::ConstCharArrayDetector< T, void >::size - 1 <= getLength()
            && match(literal, getLength() - ( internal::ConstCharArrayDetector< T, void >::size - 1 ));
    }

    /**
      Check whether this string ends with a given substring.

      @param str  the substring to be compared; must not be null and must point
      to memory of at least strLength bytes

      @param strLength  the length of the substring; must be non-negative

      @return true if and only if the given str appears as a substring at the
      end of this string

      @since LibreOffice 3.6
    */
    bool endsWithL(char const * str, sal_Int32 strLength) const {
        return strLength <= getLength()
            && matchL(str, strLength, getLength() - strLength);
    }

    friend sal_Bool     operator == ( const OString& rStr1, const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.equals(rStr2); }
    friend sal_Bool     operator != ( const OString& rStr1,     const OString& rStr2 ) SAL_THROW(())
                        { return !(operator == ( rStr1, rStr2 )); }
    friend sal_Bool     operator <  ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) < 0; }
    friend sal_Bool     operator >  ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) > 0; }
    friend sal_Bool     operator <= ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) <= 0; }
    friend sal_Bool     operator >= ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) >= 0; }

    template< typename T >
    friend typename internal::CharPtrDetector< T, bool >::Type operator==( const OString& rStr1, const T& value ) SAL_THROW(())
    {
        return rStr1.compareTo( value ) == 0;
    }

    template< typename T >
    friend typename internal::NonConstCharArrayDetector< T, bool >::Type operator==( const OString& rStr1, T& value ) SAL_THROW(())
    {
        return rStr1.compareTo( value ) == 0;
    }

    template< typename T >
    friend typename internal::CharPtrDetector< T, bool >::Type operator==( const T& value, const OString& rStr2 ) SAL_THROW(())
    {
        return rStr2.compareTo( value ) == 0;
    }

    template< typename T >
    friend typename internal::NonConstCharArrayDetector< T, bool >::Type operator==( T& value, const OString& rStr2 ) SAL_THROW(())
    {
        return rStr2.compareTo( value ) == 0;
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    friend typename internal::ConstCharArrayDetector< T, bool >::Type operator==( const OString& rStr, T& literal ) SAL_THROW(())
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        return rStr.getLength() == internal::ConstCharArrayDetector< T, void >::size - 1
            && rtl_str_compare_WithLength( rStr.pData->buffer, rStr.pData->length, literal,
                internal::ConstCharArrayDetector< T, void >::size - 1 ) == 0;
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    friend typename internal::ConstCharArrayDetector< T, bool >::Type operator==( T& literal, const OString& rStr ) SAL_THROW(())
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        return rStr.getLength() == internal::ConstCharArrayDetector< T, void >::size - 1
            && rtl_str_compare_WithLength( rStr.pData->buffer, rStr.pData->length, literal,
                internal::ConstCharArrayDetector< T, void >::size - 1 ) == 0;
    }

    template< typename T >
    friend typename internal::CharPtrDetector< T, bool >::Type operator!=( const OString& rStr1, const T& value ) SAL_THROW(())
    {
        return !(operator == ( rStr1, value ));
    }

    template< typename T >
    friend typename internal::NonConstCharArrayDetector< T, bool >::Type operator!=( const OString& rStr1, T& value ) SAL_THROW(())
    {
        return !(operator == ( rStr1, value ));
    }

    template< typename T >
    friend typename internal::CharPtrDetector< T, bool >::Type operator!=( const T& value,   const OString& rStr2 ) SAL_THROW(())
    {
        return !(operator == ( value, rStr2 ));
    }

    template< typename T >
    friend typename internal::NonConstCharArrayDetector< T, bool >::Type operator!=( T& value,   const OString& rStr2 ) SAL_THROW(())
    {
        return !(operator == ( value, rStr2 ));
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    friend typename internal::ConstCharArrayDetector< T, bool >::Type operator!=( const OString& rStr, T& literal ) SAL_THROW(())
    {
        return !( rStr == literal );
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    friend typename internal::ConstCharArrayDetector< T, bool >::Type operator!=( T& literal, const OString& rStr ) SAL_THROW(())
    {
        return !( literal == rStr );
    }

    /**
      Returns a hashcode for this string.

      @return   a hash code value for this object.

      @see rtl::OStringHash for convenient use of boost::unordered_map
    */
    sal_Int32 hashCode() const SAL_THROW(())
    {
        return rtl_str_hashCode_WithLength( pData->buffer, pData->length );
    }

    /**
      Returns the index within this string of the first occurrence of the
      specified character, starting the search at the specified index.

      @param    ch          character to be located.
      @param    fromIndex   the index to start the search from.
                            The index must be greater or equal than 0
                            and less or equal as the string length.
      @return   the index of the first occurrence of the character in the
                character sequence represented by this string that is
                greater than or equal to fromIndex, or
                -1 if the character does not occur.
    */
    sal_Int32 indexOf( sal_Char ch, sal_Int32 fromIndex = 0 ) const SAL_THROW(())
    {
        sal_Int32 ret = rtl_str_indexOfChar_WithLength( pData->buffer+fromIndex, pData->length-fromIndex, ch );
        return (ret < 0 ? ret : ret+fromIndex);
    }

    /**
      Returns the index within this string of the last occurrence of the
      specified character, searching backward starting at the end.

      @param    ch          character to be located.
      @return   the index of the last occurrence of the character in the
                character sequence represented by this string, or
                -1 if the character does not occur.
    */
    sal_Int32 lastIndexOf( sal_Char ch ) const SAL_THROW(())
    {
        return rtl_str_lastIndexOfChar_WithLength( pData->buffer, pData->length, ch );
    }

    /**
      Returns the index within this string of the last occurrence of the
      specified character, searching backward starting before the specified
      index.

      @param    ch          character to be located.
      @param    fromIndex   the index before which to start the search.
      @return   the index of the last occurrence of the character in the
                character sequence represented by this string that
                is less than fromIndex, or -1
                if the character does not occur before that point.
    */
    sal_Int32 lastIndexOf( sal_Char ch, sal_Int32 fromIndex ) const SAL_THROW(())
    {
        return rtl_str_lastIndexOfChar_WithLength( pData->buffer, fromIndex, ch );
    }

    /**
      Returns the index within this string of the first occurrence of the
      specified substring, starting at the specified index.

      If str doesn't include any character, always -1 is
      returned. This is also the case, if both strings are empty.

      @param    str         the substring to search for.
      @param    fromIndex   the index to start the search from.
      @return   If the string argument occurs one or more times as a substring
                within this string at the starting index, then the index
                of the first character of the first such substring is
                returned. If it does not occur as a substring starting
                at fromIndex or beyond, -1 is returned.
    */
    sal_Int32 indexOf( const OString & str, sal_Int32 fromIndex = 0 ) const SAL_THROW(())
    {
        sal_Int32 ret = rtl_str_indexOfStr_WithLength( pData->buffer+fromIndex, pData->length-fromIndex,
                                                       str.pData->buffer, str.pData->length );
        return (ret < 0 ? ret : ret+fromIndex);
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, sal_Int32 >::Type indexOf( T& literal, sal_Int32 fromIndex = 0 ) const SAL_THROW(())
    {
        RTL_STRING_CONST_FUNCTION
        assert( strlen( literal ) == internal::ConstCharArrayDetector< T >::size - 1 );
        sal_Int32 n = rtl_str_indexOfStr_WithLength(
            pData->buffer + fromIndex, pData->length - fromIndex, literal, internal::ConstCharArrayDetector< T, void >::size - 1);
        return n < 0 ? n : n + fromIndex;
    }

    /**
      Returns the index within this string of the first occurrence of the
      specified substring, starting at the specified index.

      If str doesn't include any character, always -1 is
      returned. This is also the case, if both strings are empty.

      @param    str         the substring to search for.
      @param    len         the length of the substring.
      @param    fromIndex   the index to start the search from.
      @return   If the string argument occurs one or more times as a substring
                within this string at the starting index, then the index
                of the first character of the first such substring is
                returned. If it does not occur as a substring starting
                at fromIndex or beyond, -1 is returned.

      @since LibreOffice 3.6
    */
    sal_Int32 indexOfL(char const * str, sal_Int32 len, sal_Int32 fromIndex = 0)
        const SAL_THROW(())
    {
        sal_Int32 n = rtl_str_indexOfStr_WithLength(
            pData->buffer + fromIndex, pData->length - fromIndex, str, len);
        return n < 0 ? n : n + fromIndex;
    }

    // This overload is left undefined, to detect calls of indexOfL that
    // erroneously use RTL_CONSTASCII_USTRINGPARAM instead of
    // RTL_CONSTASCII_STRINGPARAM (but would lead to ambiguities on 32 bit
    // platforms):
#if SAL_TYPES_SIZEOFLONG == 8
    void indexOfL(char const *, sal_Int32, rtl_TextEncoding) const;
#endif

    /**
      Returns the index within this string of the last occurrence of
      the specified substring, searching backward starting at the end.

      The returned index indicates the starting index of the substring
      in this string.
      If str doesn't include any character, always -1 is
      returned. This is also the case, if both strings are empty.

      @param    str         the substring to search for.
      @return   If the string argument occurs one or more times as a substring
                within this string, then the index of the first character of
                the last such substring is returned. If it does not occur as
                a substring, -1 is returned.
    */
    sal_Int32 lastIndexOf( const OString & str ) const SAL_THROW(())
    {
        return rtl_str_lastIndexOfStr_WithLength( pData->buffer, pData->length,
                                                  str.pData->buffer, str.pData->length );
    }

    /**
      Returns the index within this string of the last occurrence of
      the specified substring, searching backward starting before the specified
      index.

      The returned index indicates the starting index of the substring
      in this string.
      If str doesn't include any character, always -1 is
      returned. This is also the case, if both strings are empty.

      @param    str         the substring to search for.
      @param    fromIndex   the index before which to start the search.
      @return   If the string argument occurs one or more times as a substring
                within this string before the starting index, then the index
                of the first character of the last such substring is
                returned. Otherwise, -1 is returned.
    */
    sal_Int32 lastIndexOf( const OString & str, sal_Int32 fromIndex ) const SAL_THROW(())
    {
        return rtl_str_lastIndexOfStr_WithLength( pData->buffer, fromIndex,
                                                  str.pData->buffer, str.pData->length );
    }

    /**
      Returns a new string that is a substring of this string.

      The substring begins at the specified beginIndex. If
      beginIndex is negative or be greater than the length of
      this string, behaviour is undefined.

      @param     beginIndex   the beginning index, inclusive.
      @return    the specified substring.
    */
    SAL_WARN_UNUSED_RESULT OString copy( sal_Int32 beginIndex ) const SAL_THROW(())
    {
        rtl_String *pNew = 0;
        rtl_string_newFromSubString( &pNew, pData, beginIndex, getLength() - beginIndex );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Returns a new string that is a substring of this string.

      The substring begins at the specified beginIndex and contains count
      characters.  If either beginIndex or count are negative,
      or beginIndex + count are greater than the length of this string
      then behaviour is undefined.

      @param     beginIndex   the beginning index, inclusive.
      @param     count        the number of characters.
      @return    the specified substring.
    */
    SAL_WARN_UNUSED_RESULT OString copy( sal_Int32 beginIndex, sal_Int32 count ) const SAL_THROW(())
    {
        rtl_String *pNew = 0;
        rtl_string_newFromSubString( &pNew, pData, beginIndex, count );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Concatenates the specified string to the end of this string.

      @param    str   the string that is concatenated to the end
                      of this string.
      @return   a string that represents the concatenation of this string
                followed by the string argument.
    */
    SAL_WARN_UNUSED_RESULT OString concat( const OString & str ) const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newConcat( &pNew, pData, str.pData );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

#ifndef RTL_FAST_STRING
    friend OString operator+( const OString & str1, const OString & str2  ) SAL_THROW(())
    {
        return str1.concat( str2 );
    }
#endif

    /**
      Returns a new string resulting from replacing n = count characters
      from position index in this string with newStr.

      @param  index   the replacing index in str.
                      The index must be greater or equal as 0 and
                      less or equal as the length of the string.
      @param  count   the count of characters that will replaced
                      The count must be greater or equal as 0 and
                      less or equal as the length of the string minus index.
      @param  newStr  the new substring.
      @return the new string.
    */
    SAL_WARN_UNUSED_RESULT OString replaceAt( sal_Int32 index, sal_Int32 count, const OString& newStr ) const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newReplaceStrAt( &pNew, pData, index, count, newStr.pData );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Returns a new string resulting from replacing all occurrences of
      oldChar in this string with newChar.

      If the character oldChar does not occur in the character sequence
      represented by this object, then the string is assigned with
      str.

      @param    oldChar     the old character.
      @param    newChar     the new character.
      @return   a string derived from this string by replacing every
                occurrence of oldChar with newChar.
    */
    SAL_WARN_UNUSED_RESULT OString replace( sal_Char oldChar, sal_Char newChar ) const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newReplace( &pNew, pData, oldChar, newChar );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Returns a new string resulting from replacing the first occurrence of a
      given substring with another substring.

      @param from  the substring to be replaced

      @param to  the replacing substring

      @param[in,out] index  pointer to a start index; if the pointer is
      non-null: upon entry to the function, its value is the index into the this
      string at which to start searching for the \p from substring, the value
      must be non-negative and not greater than this string's length; upon exit
      from the function its value is the index into this string at which the
      replacement took place or -1 if no replacement took place; if the pointer
      is null, searching always starts at index 0

      @since LibreOffice 3.6
    */
    SAL_WARN_UNUSED_RESULT OString replaceFirst(
        OString const & from, OString const & to, sal_Int32 * index = 0) const
    {
        rtl_String * s = 0;
        sal_Int32 i = 0;
        rtl_string_newReplaceFirst(
            &s, pData, from.pData->buffer, from.pData->length,
            to.pData->buffer, to.pData->length, index == 0 ? &i : index);
        return OString(s, SAL_NO_ACQUIRE);
    }

    /**
      Returns a new string resulting from replacing all occurrences of a given
      substring with another substring.

      Replacing subsequent occurrences picks up only after a given replacement.
      That is, replacing from "xa" to "xx" in "xaa" results in "xxa", not "xxx".

      @param from  the substring to be replaced

      @param to  the replacing substring

      @since LibreOffice 3.6
    */
    SAL_WARN_UNUSED_RESULT OString replaceAll(OString const & from, OString const & to) const {
        rtl_String * s = 0;
        rtl_string_newReplaceAll(
            &s, pData, from.pData->buffer, from.pData->length,
            to.pData->buffer, to.pData->length);
        return OString(s, SAL_NO_ACQUIRE);
    }

    /**
      Converts from this string all ASCII uppercase characters (65-90)
      to ASCII lowercase characters (97-122).

      This function can't be used for language specific conversion.
      If the string doesn't contain characters which must be converted,
      then the new string is assigned with str.

      @return   the string, converted to ASCII lowercase.
    */
    SAL_WARN_UNUSED_RESULT OString toAsciiLowerCase() const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newToAsciiLowerCase( &pNew, pData );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Converts from this string all ASCII lowercase characters (97-122)
      to ASCII uppercase characters (65-90).

      This function can't be used for language specific conversion.
      If the string doesn't contain characters which must be converted,
      then the new string is assigned with str.

      @return   the string, converted to ASCII uppercase.
    */
    SAL_WARN_UNUSED_RESULT OString toAsciiUpperCase() const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newToAsciiUpperCase( &pNew, pData );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Returns a new string resulting from removing white space from both ends
      of the string.

      All characters that have codes less than or equal to
      32 (the space character) are considered to be white space.
      If the string doesn't contain white spaces at both ends,
      then the new string is assigned with str.

      @return   the string, with white space removed from the front and end.
    */
    SAL_WARN_UNUSED_RESULT OString trim() const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newTrim( &pNew, pData );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Returns a token in the string.

      Example:
        sal_Int32 nIndex = 0;
        do
        {
            ...
            OString aToken = aStr.getToken( 0, ';', nIndex );
            ...
        }
        while ( nIndex >= 0 );

      @param    token       the number of the token to return.
      @param    cTok        the character which separate the tokens.
      @param    index       the position at which the token is searched in the
                            string.
                            The index must not be greater thanthe length of the
                            string.
                            This param is set to the position of the
                            next token or to -1, if it is the last token.
      @return   the token; if either token or index is negative, an empty token
                is returned (and index is set to -1)
    */
    OString getToken( sal_Int32 token, sal_Char cTok, sal_Int32& index ) const SAL_THROW(())
    {
        rtl_String * pNew = 0;
        index = rtl_string_getToken( &pNew, pData, token, cTok, index );
        return OString( pNew, SAL_NO_ACQUIRE );
    }

    /**
      Returns a token from the string.

      The same as getToken(sal_Int32, sal_Char, sal_Int32 &), but always passing
      in 0 as the start index in the third argument.

      @param count  the number of the token to return, starting with 0
      @param separator  the character which separates the tokens

      @return  the given token, or an empty string

      @since LibreOffice 3.6
     */
    OString getToken(sal_Int32 count, char separator) const {
        sal_Int32 n = 0;
        return getToken(count, separator, n);
    }

    /**
      Returns the Boolean value from this string.

      This function can't be used for language specific conversion.

      @return   sal_True, if the string is 1 or "True" in any ASCII case.
                sal_False in any other case.
    */
    sal_Bool toBoolean() const SAL_THROW(())
    {
        return rtl_str_toBoolean( pData->buffer );
    }

    /**
      Returns the first character from this string.

      @return   the first character from this string or 0, if this string
                is emptry.
    */
    sal_Char toChar() const SAL_THROW(())
    {
        return pData->buffer[0];
    }

    /**
      Returns the int32 value from this string.

      This function can't be used for language specific conversion.

      @param    radix       the radix (between 2 and 36)
      @return   the int32 represented from this string.
                0 if this string represents no number or one of too large
                magnitude.
    */
    sal_Int32 toInt32( sal_Int16 radix = 10 ) const SAL_THROW(())
    {
        return rtl_str_toInt32( pData->buffer, radix );
    }

    /**
      Returns the uint32 value from this string.

      This function can't be used for language specific conversion.

      @param    radix       the radix (between 2 and 36)
      @return   the uint32 represented from this string.
                0 if this string represents no number or one of too large
                magnitude.

      @since LibreOffice 4.2
    */
    sal_uInt32 toUInt32( sal_Int16 radix = 10 ) const SAL_THROW(())
    {
        return rtl_str_toUInt32( pData->buffer, radix );
    }

    /**
      Returns the int64 value from this string.

      This function can't be used for language specific conversion.

      @param    radix       the radix (between 2 and 36)
      @return   the int64 represented from this string.
                0 if this string represents no number or one of too large
                magnitude.
    */
    sal_Int64 toInt64( sal_Int16 radix = 10 ) const SAL_THROW(())
    {
        return rtl_str_toInt64( pData->buffer, radix );
    }

    /**
      Returns the uint64 value from this string.

      This function can't be used for language specific conversion.

      @param    radix       the radix (between 2 and 36)
      @return   the uint64 represented from this string.
                0 if this string represents no number or one of too large
                magnitude.

      @since LibreOffice 4.1
    */
    sal_uInt64 toUInt64( sal_Int16 radix = 10 ) const SAL_THROW(())
    {
        return rtl_str_toUInt64( pData->buffer, radix );
    }

    /**
      Returns the float value from this string.

      This function can't be used for language specific conversion.

      @return   the float represented from this string.
                0.0 if this string represents no number.
    */
    float toFloat() const SAL_THROW(())
    {
        return rtl_str_toFloat( pData->buffer );
    }

    /**
      Returns the double value from this string.

      This function can't be used for language specific conversion.

      @return   the double represented from this string.
                0.0 if this string represents no number.
    */
    double toDouble() const SAL_THROW(())
    {
        return rtl_str_toDouble( pData->buffer );
    }

    /**
      Returns the string representation of the integer argument.

      This function can't be used for language specific conversion.

      @param    i           an integer value
      @param    radix       the radix (between 2 and 36)
      @return   a string with the string representation of the argument.
      @since LibreOffice 4.1
    */
    static OString number( int i, sal_Int16 radix = 10 )
    {
        return number( static_cast< long long >( i ), radix );
    }
    /// @overload
    /// @since LibreOffice 4.1
    static OString number( unsigned int i, sal_Int16 radix = 10 )
    {
        return number( static_cast< unsigned long long >( i ), radix );
    }
    /// @overload
    /// @since LibreOffice 4.1
    static OString number( long i, sal_Int16 radix = 10 )
    {
        return number( static_cast< long long >( i ), radix );
    }
    /// @overload
    /// @since LibreOffice 4.1
    static OString number( unsigned long i, sal_Int16 radix = 10 )
    {
        return number( static_cast< unsigned long long >( i ), radix );
    }
    /// @overload
    /// @since LibreOffice 4.1
    static OString number( long long ll, sal_Int16 radix = 10 )
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFINT64];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfInt64( aBuf, ll, radix ) );
        return OString( pNewData, SAL_NO_ACQUIRE );
    }
    /// @overload
    /// @since LibreOffice 4.1
    static OString number( unsigned long long ll, sal_Int16 radix = 10 )
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFUINT64];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfUInt64( aBuf, ll, radix ) );
        return OString( pNewData, SAL_NO_ACQUIRE );
    }

    /**
      Returns the string representation of the float argument.

      This function can't be used for language specific conversion.

      @param    f           a float.
      @return   a string with the string representation of the argument.
      @since LibreOffice 4.1
    */
    static OString number( float f )
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFFLOAT];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfFloat( aBuf, f ) );
        return OString( pNewData, SAL_NO_ACQUIRE );
    }

    /**
      Returns the string representation of the double argument.

      This function can't be used for language specific conversion.

      @param    d           a double.
      @return   a string with the string representation of the argument.
      @since LibreOffice 4.1
    */
    static OString number( double d )
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFDOUBLE];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfDouble( aBuf, d ) );
        return OString( pNewData, SAL_NO_ACQUIRE );
    }

    /**
      Returns the string representation of the sal_Bool argument.

      If the sal_Bool is true, the string "true" is returned.
      If the sal_Bool is false, the string "false" is returned.
      This function can't be used for language specific conversion.

      @param    b   a sal_Bool.
      @return   a string with the string representation of the argument.
      @deprecated use boolean()
    */
    SAL_DEPRECATED_INTERNAL("use boolean()") static OString valueOf( sal_Bool b ) SAL_THROW(())
    {
        return boolean(b);
    }

    /**
      Returns the string representation of the boolean argument.

      If the argument is true, the string "true" is returned.
      If the argument is false, the string "false" is returned.
      This function can't be used for language specific conversion.

      @param    b   a bool.
      @return   a string with the string representation of the argument.
      @since LibreOffice 4.1
    */
    static OString boolean( bool b ) SAL_THROW(())
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFBOOLEAN];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfBoolean( aBuf, b ) );
        return OString( pNewData, SAL_NO_ACQUIRE );
    }

    /**
      Returns the string representation of the char argument.

      @param    c   a character.
      @return   a string with the string representation of the argument.
      @deprecated use operator, function or constructor taking char or sal_Unicode argument
    */
    SAL_DEPRECATED_INTERNAL("convert to OString or use directly") static OString valueOf( sal_Char c ) SAL_THROW(())
    {
        return OString( &c, 1 );
    }

    /**
      Returns the string representation of the int argument.

      This function can't be used for language specific conversion.

      @param    i           a int32.
      @param    radix       the radix (between 2 and 36)
      @return   a string with the string representation of the argument.
      @deprecated use number()
    */
    SAL_DEPRECATED_INTERNAL("use number()") static OString valueOf( sal_Int32 i, sal_Int16 radix = 10 ) SAL_THROW(())
    {
        return number( i, radix );
    }

    /**
      Returns the string representation of the long argument.

      This function can't be used for language specific conversion.

      @param    ll          a int64.
      @param    radix       the radix (between 2 and 36)
      @return   a string with the string representation of the argument.
      @deprecated use number()
    */
    SAL_DEPRECATED_INTERNAL("use number()") static OString valueOf( sal_Int64 ll, sal_Int16 radix = 10 ) SAL_THROW(())
    {
        return number( ll, radix );
    }

    /**
      Returns the string representation of the float argument.

      This function can't be used for language specific conversion.

      @param    f           a float.
      @return   a string with the string representation of the argument.
      @deprecated use number()
    */
    SAL_DEPRECATED_INTERNAL("use number()") static OString valueOf( float f ) SAL_THROW(())
    {
        return number(f);
    }

    /**
      Returns the string representation of the double argument.

      This function can't be used for language specific conversion.

      @param    d           a double.
      @return   a string with the string representation of the argument.
      @deprecated use number()
    */
    SAL_DEPRECATED_INTERNAL("use number()") static OString valueOf( double d ) SAL_THROW(())
    {
        return number(d);
    }

};

/* ======================================================================= */

#ifdef RTL_FAST_STRING
/**
A simple wrapper around string literal. It is usually not necessary to use, can
be mostly used to force OString operator+ working with operands that otherwise would
not trigger it.

This class is not part of public API and is meant to be used only in LibreOffice code.
@since LibreOffice 4.0
*/
struct SAL_WARN_UNUSED OStringLiteral
{
    template< int N >
    OStringLiteral( const char (&str)[ N ] ) : size( N - 1 ), data( str ) { assert( strlen( str ) == N - 1 ); }
    int size;
    const char* data;
};

/**
 @internal
*/
template<>
struct ToStringHelper< OString >
    {
    static int length( const OString& s ) { return s.getLength(); }
    static char* addData( char* buffer, const OString& s ) { return addDataHelper( buffer, s.getStr(), s.getLength()); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

/**
 @internal
*/
template<>
struct ToStringHelper< OStringLiteral >
    {
    static int length( const OStringLiteral& str ) { return str.size; }
    static char* addData( char* buffer, const OStringLiteral& str ) { return addDataHelper( buffer, str.data, str.size ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

/**
 @internal
*/
template< typename charT, typename traits, typename T1, typename T2 >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const OStringConcat< T1, T2 >& concat)
{
    return stream << OString( concat );
}
#else
// non-RTL_FAST_STRING needs this to compile
/// @cond INTERNAL
typedef OString OStringLiteral;
/// @endcond
#endif


/** A helper to use OStrings with hash maps.

    Instances of this class are unary function objects that can be used as
    hash function arguments to boost::unordered_map and similar constructs.
 */
struct OStringHash
{
    /** Compute a hash code for a string.

        @param rString
        a string.

        @return
        a hash code for the string.  This hash code should not be stored
        persistently, as its computation may change in later revisions.
     */
    size_t operator()( const OString& rString ) const
        { return (size_t)rString.hashCode(); }
};

/** Equality functor for classic c-strings (i.e., null-terminated char* strings). */
struct CStringEqual
{
    bool operator()( const char* p1, const char* p2) const
        { return rtl_str_compare(p1, p2) == 0; }
};

/** Hashing functor for classic c-strings (i.e., null-terminated char* strings). */
struct CStringHash
{
    size_t operator()(const char* p) const
        { return rtl_str_hashCode(p); }
};

/* ======================================================================= */

/**
    Support for rtl::OString in std::ostream (and thus in
    CPPUNIT_ASSERT or SAL_INFO macros, for example).

    @since LibreOffice 4.0
 */
template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, OString const & string)
{
    return stream << string.getStr();
        // best effort; potentially loses data due to embedded null characters
}

} /* Namespace */

#ifdef RTL_STRING_UNITTEST
namespace rtl
{
typedef rtlunittest::OString OString;
}
#undef RTL_STRING_CONST_FUNCTION
#endif

#ifdef RTL_USING
using ::rtl::OString;
using ::rtl::OStringHash;
using ::rtl::OStringLiteral;
#endif

#endif /* _RTL_STRING_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
