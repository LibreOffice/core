/*************************************************************************
 *
 *  $RCSfile: string.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 14:56:13 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_STRING_HXX_
#define _RTL_STRING_HXX_

#ifdef __cplusplus

#ifndef _RTL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif

class ByteString;

namespace rtl
{

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

class OString
{
    friend class ::ByteString;

public:
    /** @internal */
    rtl_String * pData;

private:
    /** @internal */
    class DO_NOT_ACQUIRE;

    /** @internal */
    OString( rtl_String * value, DO_NOT_ACQUIRE * )
    {
        pData = value;
    }

public:
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

    /**
      New string from a character buffer array.

      @param    value       a NULL-terminated character array.
    */
    OString( const sal_Char * value ) SAL_THROW(())
    {
        pData = 0;
        rtl_string_newFromStr( &pData, value );
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
    */
    OString( const sal_Unicode * value, sal_Int32 length,
             rtl_TextEncoding encoding,
             sal_uInt32 convertFlags = OUSTRING_TO_OSTRING_CVTFLAGS ) SAL_THROW(())
    {
        pData = 0;
        rtl_uString2String( &pData, value, length, encoding, convertFlags );
    }

    /** Convert a ByteString (from the tools module) into an OString.

        Since both ByteString and OString internally use the same data
        structure, this is a very cheap operation.
     */
    OString( const ByteString & value ) SAL_THROW(());

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
      Append a string to this string.

      @param    str         a OString.
    */
    OString & operator+=( const OString & str ) SAL_THROW(())
    {
        rtl_string_newConcat( &pData, pData, str.pData );
        return *this;
    }

    /**
      Returns the length of this string.

      The length is equal to the number of characters in this string.

      @return   the length of the sequence of characters represented by this
                object.
    */
    sal_Int32 getLength() const SAL_THROW(()) { return pData->length; }

    /**
      Returns a pointer to the character buffer from this string.

      It isn't necessarily NULL terminated.

      @return   a pointer to the characters buffer from this object.
    */
    operator const sal_Char *() const SAL_THROW(()) { return pData->buffer; }

    /**
      Returns a pointer to the character buffer from this string.

      It isn't necessarily NULL terminated.

      @return   a pointer to the characters buffer from this object.
    */
    const sal_Char * getStr() const SAL_THROW(()) { return pData->buffer; }

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

      @param    str         the object to be compared.
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

    friend sal_Bool     operator == ( const OString& rStr1, const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.getLength() == rStr2.getLength() && rStr1.compareTo( rStr2 ) == 0; }
    friend sal_Bool     operator == ( const OString& rStr1, const sal_Char * pStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( pStr2 ) == 0; }
    friend sal_Bool     operator == ( const sal_Char * pStr1,   const OString& rStr2 ) SAL_THROW(())
                        { return OString( pStr1 ).compareTo( rStr2 ) == 0; }

    friend sal_Bool     operator != ( const OString& rStr1,     const OString& rStr2 ) SAL_THROW(())
                        { return !(operator == ( rStr1, rStr2 )); }
    friend sal_Bool     operator != ( const OString& rStr1, const sal_Char * pStr2 ) SAL_THROW(())
                        { return !(operator == ( rStr1, pStr2 )); }
    friend sal_Bool     operator != ( const sal_Char * pStr1,   const OString& rStr2 ) SAL_THROW(())
                        { return !(operator == ( pStr1, rStr2 )); }

    friend sal_Bool     operator <  ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) < 0; }
    friend sal_Bool     operator >  ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) > 0; }
    friend sal_Bool     operator <= ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) <= 0; }
    friend sal_Bool     operator >= ( const OString& rStr1,    const OString& rStr2 ) SAL_THROW(())
                        { return rStr1.compareTo( rStr2 ) >= 0; }

    /**
      Returns a hashcode for this string.

      @return   a hash code value for this object.
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
      specified character, searching backward starting at the specified index.

      @param    ch          character to be located.
      @return   the index of the last occurrence of the character in the
                character sequence represented by this string that
                is less or than or equal to fromIndex, or -1
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
      the specified substring, searching backward starting at the end.

      The returned index indicates the starting index of the substring
      in this string.
      If str doesn't include any character, always -1 is
      returned. This is also the case, if both strings are empty.

      @param    str         the substring to search for.
      @param    fromIndex   the index to start the search from.
      @return   If the string argument occurs one or more times as a substring
                within this string at the starting index, then the index
                of the first character of the last such substring is
                returned. If it does not occur as a substring starting
                at fromIndex or earlier, -1 is returned.
    */
    sal_Int32 lastIndexOf( const OString & str, sal_Int32 fromIndex ) const SAL_THROW(())
    {
        return rtl_str_lastIndexOfStr_WithLength( pData->buffer, fromIndex,
                                                  str.pData->buffer, str.pData->length );
    }

    /**
      Returns a new string that is a substring of this string.

      The substring begins at the specified beginIndex.  It is an error for
      beginIndex to be negative or to be greater than the length of this string.

      @param     beginIndex   the beginning index, inclusive.
      @return    the specified substring.
    */
    OString copy( sal_Int32 beginIndex ) const SAL_THROW(())
    {
        OSL_ASSERT(beginIndex >= 0 && beginIndex <= getLength());
        if ( beginIndex == 0 )
            return *this;
        else
        {
            rtl_String* pNew = 0;
            rtl_string_newFromStr_WithLength( &pNew, pData->buffer+beginIndex, getLength()-beginIndex );
            return OString( pNew, (DO_NOT_ACQUIRE*)0 );
        }
    }

    /**
      Returns a new string that is a substring of this string.

      The substring begins at the specified beginIndex and contains count
      characters.  It is an error for either beginIndex or count to be negative,
      or for beginIndex + count to be greater than the length of this string.

      @param     beginIndex   the beginning index, inclusive.
      @param     count        the number of characters.
      @return    the specified substring.
    */
    OString copy( sal_Int32 beginIndex, sal_Int32 count ) const SAL_THROW(())
    {
        OSL_ASSERT(beginIndex >= 0 && beginIndex <= getLength()
                   && count >= 0 && count <= getLength() - beginIndex);
        if ( (beginIndex == 0) && (count == getLength()) )
            return *this;
        else
        {
            rtl_String* pNew = 0;
            rtl_string_newFromStr_WithLength( &pNew, pData->buffer+beginIndex, count );
            return OString( pNew, (DO_NOT_ACQUIRE*)0 );
        }
    }

    /**
      Concatenates the specified string to the end of this string.

      @param    str   the string that is concatenated to the end
                      of this string.
      @return   a string that represents the concatenation of this string
                followed by the string argument.
    */
    OString concat( const OString & str ) const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newConcat( &pNew, pData, str.pData );
        return OString( pNew, (DO_NOT_ACQUIRE*)0 );
    }

    friend OString operator+( const OString & str1, const OString & str2  ) SAL_THROW(())
    {
        return str1.concat( str2 );
    }

    /**
      Returns a new string resulting from replacing n = count characters
      from position index in this string with newStr.

      @param  index   the replacing index in str.
                      The index must be greater or equal as 0 and
                      less or equal as the length of the string.
      @param  count   the count of charcters that will replaced
                      The count must be greater or equal as 0 and
                      less or equal as the length of the string minus index.
      @param  newStr  the new substring.
      @return the new string.
    */
    OString replaceAt( sal_Int32 index, sal_Int32 count, const OString& newStr ) const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newReplaceStrAt( &pNew, pData, index, count, newStr.pData );
        return OString( pNew, (DO_NOT_ACQUIRE*)0 );
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
    OString replace( sal_Char oldChar, sal_Char newChar ) const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newReplace( &pNew, pData, oldChar, newChar );
        return OString( pNew, (DO_NOT_ACQUIRE*)0 );
    }

    /**
      Converts from this string all ASCII uppercase characters (65-90)
      to ASCII lowercase characters (97-122).

      This function can't be used for language specific conversion.
      If the string doesn't contain characters which must be converted,
      then the new string is assigned with str.

      @return   the string, converted to ASCII lowercase.
    */
    OString toAsciiLowerCase() const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newToAsciiLowerCase( &pNew, pData );
        return OString( pNew, (DO_NOT_ACQUIRE*)0 );
    }

    /**
      Converts from this string all ASCII lowercase characters (97-122)
      to ASCII uppercase characters (65-90).

      This function can't be used for language specific conversion.
      If the string doesn't contain characters which must be converted,
      then the new string is assigned with str.

      @return   the string, converted to ASCII uppercase.
    */
    OString toAsciiUpperCase() const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newToAsciiUpperCase( &pNew, pData );
        return OString( pNew, (DO_NOT_ACQUIRE*)0 );
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
    OString trim() const SAL_THROW(())
    {
        rtl_String* pNew = 0;
        rtl_string_newTrim( &pNew, pData );
        return OString( pNew, (DO_NOT_ACQUIRE*)0 );
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

      @param    token       the number of the token to return. The number
                            must be greater or equal as 0.
      @param    cTok        the character which seperate the tokens.
      @param    index       the position at which the token is searched in the
                            string.
                            The index must be greater or equal as 0 and
                            less or equal as the length of the string.
                            This param is set to the position of the
                            next token or to -1, if it is the last token.
      @return   the token
    */
    OString getToken( sal_Int32 token, sal_Char cTok, sal_Int32& index ) const SAL_THROW(())
    {
        rtl_String * pNew = 0;
        index = rtl_string_getToken( &pNew, pData, token, cTok, index );
        return OString( pNew, (DO_NOT_ACQUIRE *)0 );
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
                0 if this string represents no number.
    */
    sal_Int32 toInt32( sal_Int16 radix = 10 ) const SAL_THROW(())
    {
        return rtl_str_toInt32( pData->buffer, radix );
    }

    /**
      Returns the int64 value from this string.

      This function can't be used for language specific conversion.

      @param    radix       the radix (between 2 and 36)
      @return   the int64 represented from this string.
                0 if this string represents no number.
    */
    sal_Int64 toInt64( sal_Int16 radix = 10 ) const SAL_THROW(())
    {
        return rtl_str_toInt64( pData->buffer, radix );
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
      Returns the string representation of the sal_Bool argument.

      If the sal_Bool is true, the string "true" is returned.
      If the sal_Bool is false, the string "false" is returned.
      This function can't be used for language specific conversion.

      @param    b   a sal_Bool.
      @return   a string with the string representation of the argument.
    */
    static OString valueOf( sal_Bool b ) SAL_THROW(())
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFBOOLEAN];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfBoolean( aBuf, b ) );
        return OString( pNewData, (DO_NOT_ACQUIRE*)0 );
    }

    /**
      Returns the string representation of the char argument.

      @param    c   a character.
      @return   a string with the string representation of the argument.
    */
    static OString valueOf( sal_Char c ) SAL_THROW(())
    {
        return OString( &c, 1 );
    }

    /**
      Returns the string representation of the int argument.

      This function can't be used for language specific conversion.

      @param    i           a int32.
      @param    radix       the radix (between 2 and 36)
      @return   a string with the string representation of the argument.
    */
    static OString valueOf( sal_Int32 i, sal_Int16 radix = 10 ) SAL_THROW(())
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFINT32];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfInt32( aBuf, i, radix ) );
        return OString( pNewData, (DO_NOT_ACQUIRE*)0 );
    }

    /**
      Returns the string representation of the long argument.

      This function can't be used for language specific conversion.

      @param    l           a int64.
      @param    radix       the radix (between 2 and 36)
      @return   a string with the string representation of the argument.
    */
    static OString valueOf( sal_Int64 l, sal_Int16 radix = 10 ) SAL_THROW(())
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFINT64];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfInt64( aBuf, l, radix ) );
        return OString( pNewData, (DO_NOT_ACQUIRE*)0 );
    }

    /**
      Returns the string representation of the float argument.

      This function can't be used for language specific conversion.

      @param    f           a float.
      @return   a string with the string representation of the argument.
    */
    static OString valueOf( float f ) SAL_THROW(())
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFFLOAT];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfFloat( aBuf, f ) );
        return OString( pNewData, (DO_NOT_ACQUIRE*)0 );
    }

    /**
      Returns the string representation of the double argument.

      This function can't be used for language specific conversion.

      @param    d           a double.
      @return   a string with the string representation of the argument.
    */
    static OString valueOf( double d ) SAL_THROW(())
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFDOUBLE];
        rtl_String* pNewData = 0;
        rtl_string_newFromStr_WithLength( &pNewData, aBuf, rtl_str_valueOfDouble( aBuf, d ) );
        return OString( pNewData, (DO_NOT_ACQUIRE*)0 );
    }
};

/* ======================================================================= */

/** A helper to use OStrings with hash maps.

    Instances of this class are unary function objects that can be used as
    hash function arguments to STLPort's hash_map and similar constructs.
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
    size_t operator()( const rtl::OString& rString ) const
        { return (size_t)rString.hashCode(); }
};

/* ======================================================================= */

} /* Namespace */

#endif /* __cplusplus */

#endif /* _RTL_STRING_HXX_ */
