/*************************************************************************
 *
 *  $RCSfile: string.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 15:15:42 $
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

#ifndef _RTL_STRING_H_
#define _RTL_STRING_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================= */

/**
  Returns the length of a string.
  The length is equal to the number of 8-bit characters in the
  string without the terminating NULL-character.

  @param    str         must be a NULL-terminated string.
  @return   the length of the sequence of characters represented by this
            string, excluding the terminating NULL-character.
*/
sal_Int32 SAL_CALL rtl_str_getLength( const sal_Char * str ) SAL_THROW_EXTERN_C();

/**
  Compares two strings.
  The comparison is based on the numeric value of each character in
  the strings and return a value indicating their relationship.
  This function can't be used for language specific sorting.
  Both strings must be NULL-terminated.

  @param    first       the first NULL-terminated string to be compared.
  @param    second      the second NULL-terminated string which is compared
                        with the first param.
  @return   <code>0</code> - if both strings are equal
            <code>< 0</code> - if the first string is less than the second string
            <code>> 0</code> - if the first string is greater than the second string
*/
sal_Int32 SAL_CALL rtl_str_compare( const sal_Char * first, const sal_Char * second ) SAL_THROW_EXTERN_C();

/**
  Compares two strings with a maximum count of characters for each string.
  The comparison is based on the numeric value of each character in
  the strings and return a value indicating their relationship.
  This function can't be used for language specific sorting.
  Both string lengths must be equal or greater as there given length.

  @param    first       the first string to be compared.
  @param    firstLen    the length of the first string or the number of
                        characters to compared. The first string length
                        must be greater or equal than this value.
  @param    second      the second string which is compared with the first
                        param.
  @param    secondLen   the length of the second string or the number of
                        characters to compared. The second string length
                        must be greater or equal than this value.
  @return   <code>0</code> - if both substrings are equal
            <code>< 0</code> - if the first substring is less than the second substring
            <code>> 0</code> - if the first substring is greater than the second substring
*/
sal_Int32 SAL_CALL rtl_str_compare_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

/**
  Returns a hashcode for a string.
  It is not allowed to store the hash code, because newer versions
  could return other hashcodes.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   a hash code value for str.
*/
sal_Int32 SAL_CALL rtl_str_hashCode( const sal_Char * str ) SAL_THROW_EXTERN_C();

/**
  Returns a hashcode for a substring.
  It is not allowed to store the hash code, because newer versions
  could return other hashcodes.

  @param    str         a string.
  @param    len         the maximum number of characters for creating the
                        hashcode. The string length must be greater or equal
                        than this value.
  @return   a hash code value for str.
*/
sal_Int32 SAL_CALL rtl_str_hashCode_WithLength( const sal_Char * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the first occurrence of the
  specified character.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    ch          character to be located.
  @return   the index of the first occurrence of the character in the
            character sequence represented by the string, or
            <code>-1</code> if the character does not occur.
*/
sal_Int32 SAL_CALL rtl_str_indexOfChar( const sal_Char * str, sal_Char ch ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the substring of the first occurrence of the
  specified character.
  The string length must be greater or equal as the given len.

  @param    str         a substring.
  @param    len         the maximum number of characters. The string length
                        must be greater or equal than this value.
  @param    ch          character to be located.
  @return   the index of the first occurrence of the character in the
            character sequence represented by the string, or
            <code>-1</code> if the character does not occur.
*/
sal_Int32 SAL_CALL rtl_str_indexOfChar_WithLength( const sal_Char * str, sal_Int32 len, sal_Char ch ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the last occurrence of the
  specified character, searching backward starting at the end.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    ch          character to be located.
  @return   the index of the last occurrence of the character in the
            character sequence represented by the string, or
            <code>-1</code> if the character does not occur.
            The return value is always lower as the string len.
*/
sal_Int32 SAL_CALL rtl_str_lastIndexOfChar( const sal_Char * str, sal_Char ch ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the last occurrence of the
  specified character, searching backward starting at the specified
  index (excluding the character at the specified index).

  @param    str         a substring.
  @param    len         the starting index. The string length
                        must be greater or equal than this value.
  @param    ch          character to be located.
  @return   the index of the last occurrence of the character in the
            character sequence represented by the string, or
            <code>-1</code> if the character does not occur.
            The return value is always lower as the len param.
*/
sal_Int32 SAL_CALL rtl_str_lastIndexOfChar_WithLength( const sal_Char * str, sal_Int32 len, sal_Char ch ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the first occurrence of the
  specified substring.
  If subStr doesn't include any character, always <code>-1</code> is
  returned. This is also the case, if both strings are empty.
  Both strings must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    subStr      a NULL-terminated substring to be searched for.
  @return   if the string argument occurs as a substring within the
            string, then the index of the first character of the first
            such substring is returned; if it does not occur as a
            substring, <code>-1</code> is returned.
*/
sal_Int32 SAL_CALL rtl_str_indexOfStr( const sal_Char * str, const sal_Char * subStr ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the first occurrence of the
  specified substring.
  If subLen is zero, always <code>-1</code> is returned. This is also
  the case, if str is also zero.
  Both string lengths must be equal or greater as there given length.

  @param    str         a string.
  @param    len         the length of the string or the number of
                        characters to compared. The string length
                        must be greater or equal than this value.
  @param    subStr      a substring to be searched for.
  @param    subLen      the length of the substring or the number of
                        characters to compared. The substring length
                        must be greater or equal than this value.
  @return   if the string argument occurs as a substring within the
            string, then the index of the first character of the first
            such substring is returned; if it does not occur as a
            substring, <code>-1</code> is returned.
*/
sal_Int32 SAL_CALL rtl_str_indexOfStr_WithLength( const sal_Char * str, sal_Int32 len, const sal_Char * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the last occurrence of
  the specified substring, searching backward starting at the end.
  The returned index indicates the starting index of the substring.
  If subStr doesn't include any character, always <code>-1</code> is
  returned. This is also the case, if both strings are empty.
  Both strings must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    subStr      a NULL-terminated substring to be searched for.
  @return   if the string argument occurs as a substring within the
            string, then the index of the first character of the last
            such substring is returned; if it does not occur as a
            substring, <code>-1</code> is returned. The return value is
            always lower as the string len.
*/
sal_Int32 SAL_CALL rtl_str_lastIndexOfStr( const sal_Char * str, const sal_Char * subStr ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the last occurrence of
  the specified substring, searching backward starting at the specified
  index (excluding the character at the specified index).
  The returned index indicates the starting index of the substring.
  If subLen is zero, always <code>-1</code> is returned. This is also
  the case, if str is also zero.
  Both string lengths must be equal or greater as there given length.

  @param    str         a string.
  @param    len         the length of the string or the number of
                        characters to compared. The string length
                        must be greater or equal than this value.
  @param    subStr      a substring to be searched for.
  @param    subLen      the length of the substring or the number of
                        characters to compared. The substring length
                        must be greater or equal than this value.
  @return   If the string argument occurs one or more times as a substring
            within the string, then the index of the first character of
            the last such substring is returned. If it does not occur as a
            substring <code>-1</code> is returned. The return value is
            always lower as the len param.
*/
sal_Int32 SAL_CALL rtl_str_lastIndexOfStr_WithLength( const sal_Char * str, sal_Int32 len, const sal_Char * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

/**
  Replaces all occurrences of oldChar in the string with newChar.
  If the character oldChar does not occur in the character sequence
  represented by the string, then the string is not modified.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    oldChar     the old character.
  @param    newChar     the new character.
*/
void SAL_CALL rtl_str_replaceChar( sal_Char * str, sal_Char oldChar, sal_Char newChar ) SAL_THROW_EXTERN_C();

/**
  Replaces all occurrences of oldChar in the string with newChar.
  If the character oldChar does not occur in the character sequence
  represented by the string, then the string is not modified.
  The string length must be greater or equal as the given len.

  @param    str         a string.
  @param    len         the length of the string or the number of
                        characters to replaced. The string length
                        must be greater or equal than this value.
  @param    oldChar     the old character.
  @param    newChar     the new character.
*/
void SAL_CALL rtl_str_replaceChar_WithLength( sal_Char * str, sal_Int32 len, sal_Char oldChar, sal_Char newChar ) SAL_THROW_EXTERN_C();

/**
  Returns the string representation of the sal_Bool argument.
  If the sal_Bool is true, the buffer is filled with the
  string "True" and 5 is returned.
  If the sal_Bool is false, the buffer is filled with the
  string "False" and 6 is returned.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_STR_MAX_VALUEOFBOOLEAN
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    b           a sal_Bool.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_str_valueOfBoolean( sal_Char * str, sal_Bool b ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFBOOLEAN 6

/**
  Returns the string representation of the char argument.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_STR_MAX_VALUEOFCHAR
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    ch          a char.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_str_valueOfChar( sal_Char * str, sal_Char ch ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFCHAR 2

/**
  Returns the string representation of the int argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_STR_MAX_VALUEOFINT32
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    i           a int32.
  @param    radix       the radix (between 2 and 36)
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_str_valueOfInt32( sal_Char * str, sal_Int32 i, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_STR_MIN_RADIX           2
#define RTL_STR_MAX_RADIX           36
#define RTL_STR_MAX_VALUEOFINT32    33

/**
  Returns the string representation of the long argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_STR_MAX_VALUEOFINT64
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    l           a int64.
  @param    radix       the radix (between 2 and 36)
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_str_valueOfInt64( sal_Char * str, sal_Int64 l, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFINT64 65

/**
  Returns the string representation of the float argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_STR_MAX_VALUEOFFLOAT
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    f           a float.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_str_valueOfFloat( sal_Char * str, float f ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFFLOAT 15

/**
  Returns the string representation of the double argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_STR_MAX_VALUEOFDOUBLE
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    d           a double.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_str_valueOfDouble( sal_Char * str, double d ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFDOUBLE 25

/**
  Returns the Boolean value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   sal_True, if the string is 1 or "True" in any ASCII case.
            sal_False in any other case.
*/
sal_Bool SAL_CALL rtl_str_toBoolean( const sal_Char * str ) SAL_THROW_EXTERN_C();

/**
  Returns the int32 value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    radix       the radix (between 2 and 36)
  @return   the int32 represented by the string.
            0 if the string represents no number.
*/
sal_Int32 SAL_CALL rtl_str_toInt32( const sal_Char * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/**
  Returns the int64 value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    radix       the radix (between 2 and 36)
  @return   the int64 represented by the string.
            0 if the string represents no number.
*/
sal_Int64 SAL_CALL rtl_str_toInt64( const sal_Char * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/**
  Returns the float value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   the float represented by the string.
            0.0 if the string represents no number.
*/
float SAL_CALL rtl_str_toFloat( const sal_Char * str ) SAL_THROW_EXTERN_C();

/**
  Returns the double value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   the double represented by the string.
            0.0 if the string represents no number.
*/
double SAL_CALL rtl_str_toDouble( const sal_Char * str ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(1)
#endif

/**
  The implementation structure of a byte string.
*/
typedef struct _rtl_String
{
    sal_Int32       refCount;
    sal_Int32       length;
    sal_Char        buffer[1];
} rtl_String;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

/* ----------------------------------------------------------------------- */

/**
  Increment the reference count of the string.

  @param    str         the string.
*/
void SAL_CALL rtl_string_acquire( rtl_String * str ) SAL_THROW_EXTERN_C();

/**
  Decrement the reference count of the string. If the count goes
  to zero than the string data is deleted.

  @param    str         the string.
*/
void SAL_CALL rtl_string_release( rtl_String * str ) SAL_THROW_EXTERN_C();

/**
  Allocates a new string containing no characters.

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
*/
void SAL_CALL rtl_string_new( rtl_String ** newStr ) SAL_THROW_EXTERN_C();

/**
  Allocates a new string containing space for the given
  numbers of characters.
  The reference count of the new string is 1 or an empty string.
  The values of all characters are set to 0 and the length of the
  string is 0, only data is allocated for holding characters.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
  @param    len         number of characters.
*/
void SAL_CALL rtl_string_new_WithLength( rtl_String ** newStr, sal_Int32 len ) SAL_THROW_EXTERN_C();

/**
  Allocates a new string that contains the same sequence of
  characters as the string argument.
  The reference count of the new string is 1 or an empty string.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
  @param    value       a valid string.
*/
void SAL_CALL rtl_string_newFromString( rtl_String ** newStr, const rtl_String * value ) SAL_THROW_EXTERN_C();

/**
  Allocates a new string that contains the same sequence of
  characters contained in the character array argument.
  The reference count of the new string is 1 or an empty string.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
  @param    value       a NULL-terminated character array.
*/
void SAL_CALL rtl_string_newFromStr( rtl_String ** newStr, const sal_Char * value ) SAL_THROW_EXTERN_C();

/**
  Allocates a new string that contains characters from
  the character array argument.
  The reference count of the new string is 1 or an empty string.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
  @param    value       a character array.
  @param    len         the number of character which should be copied.
                        The character array length must be greater or
                        equal than this value.
*/
void SAL_CALL rtl_string_newFromStr_WithLength( rtl_String ** newStr, const sal_Char * value, sal_Int32 len ) SAL_THROW_EXTERN_C();

/**
  Assign rightValue to *str. Release *str and aquire rightValue!

  @param    str         pointer to the string. The data must be 0 or
                        a valid string.
  @param    rightValue  a valid string.
*/
void SAL_CALL rtl_string_assign( rtl_String ** str, rtl_String * rightValue ) SAL_THROW_EXTERN_C();

/**
  Returns the length of this string.
  The length is equal to the number of characters in the string.

  @param    str         a valid string.
  @return   the length of the sequence of characters represented by the
            string.
*/
sal_Int32 SAL_CALL rtl_string_getLength( const rtl_String * str ) SAL_THROW_EXTERN_C();

/**
  Returns the pointer to the character array of the string.

  @param    str         a valid string.
  @return   a null terminated character array.
*/
sal_Char * SAL_CALL rtl_string_getStr( rtl_String * str ) SAL_THROW_EXTERN_C();

/**
  Concatenates the right string to the end of the left string and
  returns the result in a new instance.
  The new instance isn't in every case a new instance (for example,
  if one or both strings are empty). The new string object could
  be a shared instance and can't be modified without checking the
  refercence count.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
  @param    left        a valid string.
  @param    right       a valid string.
*/
void SAL_CALL rtl_string_newConcat( rtl_String ** newStr, rtl_String * left, rtl_String * right ) SAL_THROW_EXTERN_C();

/**
  Returns a new string resulting from replacing a number of characters (count)
  from the specified position (index) in the string (str) and inserting
  the string (subStr) at the specified position (index).
  If subStr is 0, than only a number of characters (count) are deleted
  at the specified position (index).
  The new instance isn't in every case a new instance. The new string
  object could be a shared instance and can't be modified without
  checking the refercence count.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
  @param    str         a valid string.
  @param    index       the replacing index  in str.
                        The index must be greater or equal as 0 and
                        less or equal as the length of the string.
  @param    count       the count of charcters that will replaced
                        The count must be greater or equal as 0 and
                        less or equal as the length of the string minus index.
  @param    subStr      0 or a valid string, which is inserted at nIndex.
                        If subStr is 0, only a number of characters (count)
                        are deleted at the specified position (index).
*/
void SAL_CALL rtl_string_newReplaceStrAt( rtl_String ** newStr, rtl_String * str, sal_Int32 index, sal_Int32 count, rtl_String * subStr ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

/* constAsciiStr must be a "..." or char const aFoo[] = "..." */
#define RTL_CONSTASCII_STRINGPARAM( constAsciiStr ) constAsciiStr, sizeof( constAsciiStr )-1
#define RTL_CONSTASCII_LENGTH( constAsciiStr ) (sizeof( constAsciiStr )-1)

/* ======================================================================= */

/* predefined constants for String-Conversion */
#define OUSTRING_TO_OSTRING_CVTFLAGS    (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |\
                                         RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |\
                                         RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |\
                                         RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 |\
                                         RTL_UNICODETOTEXT_FLAGS_NOCOMPOSITE)

/* ----------------------------------------------------------------------- */

/**
  Allocates a new byte string from the seequence of Unicode characters
  and convert the Unicode sequence to a byte sequence with the specified
  text encoding.
  The string length could be a different one as the specified length,
  because not all text conversions result in the same byte length
  (for example double byte encodings, UTF-7, UTF-8, ...).
  The reference count of the new string is 1 or an empty string.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr          pointer to the new string. The data must be 0 or
                            a valid string.
  @param    str             a Unicode character array.
  @param    len             the number of character which should be converted.
                            The Unicode character array length must be
                            greater or equal than this value.
  @param    encoding        the text encoding in which the Unicode character
                            sequence should be converted.
  @param    convertFlags    flags which controls the conversion.
                            see RTL_UNICODETOTEXT_FLAGS_...
*/
void SAL_CALL rtl_uString2String( rtl_String ** newStr, const sal_Unicode * str, sal_Int32 len, rtl_TextEncoding encoding, sal_uInt32 convertFlags ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

/**
 * Compares first string to second string object. Both
 * strings must be NULL-terminated.
 * The result is <code>true</code> if and only if second string represents
 * the same sequence of characters as the first string, where case is ignored.
 * <p>
 * Two characters are considered the same, ignoring case, if at
 * least one of the following is true:
 * <ul>
 * <li>The two characters are the same (as compared by the <code>==</code>
 *     operator).
 * <li>Applying the method <code>Character.toUppercase</code> to each
 *     character produces the same result.
 * <li>Applying the method <code>Character.toLowercase</code> to each
 *     character produces the same result.
 * </ul>
 * <p>
 * Two sequences of characters are the same, ignoring case, if the
 * sequences have the same length and corresponding characters are
 * the same, ignoring case.
 *
 * @param   first   the <code>string</code> to compared.
 * @param   second  the <code>string</code> to compare first
 *                  <code>String</code> against.
 * @return  <code>true</code> if the <code>String</code>s are equal,
 *          ignoring case; <code>false</code> otherwise.
 */
sal_Bool SAL_CALL rtl_str_equalsIgnoreCase_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen );
sal_Bool SAL_CALL rtl_str_equalsIgnoreCase( const sal_Char * first, const sal_Char * second );

/**
 * rtl_str_compareIgnoreCase() and rtl_str_compareIgnoreCase_WithLength()
 * functions  are  caseinsensitive  versions  of rtl_str_compare() and
 * rtl_str_compare_WithLength() respectively, described above. They assume
 * the  ASCII characterset and ignore differences in case when comparing
 * lower and upper case characters.  rtl_str_compareIgnoreCase_WithLength()
 * does not compare more than len characters. Both strings have to be NULL
 * terminated
 *
 * @param   first   the <code>String</code> to be compared.
 * @param   second  the <code>String</code> to compare first
 *                  <code>String</code> against.
 * @return  the value <code>0</code> if the argument string is equal to
 *          this string; a value less than <code>0</code> if a all lowercase
 *          version of first string is lexicographically less than a all
 *          lowercase version of the second string; and a
 *          value greater than <code>0</code> if first string is
 *          lexicographically greater than the second string, again both
 *          strings all lowercase.
 */
sal_Int32 SAL_CALL
rtl_str_compareIgnoreCase_WithLength(
    const sal_Char *first, const sal_Char *second, sal_Int32 len );
sal_Int32 SAL_CALL
rtl_str_compareIgnoreCase( const sal_Char *first, const sal_Char *second );

/**
 * Converts all of the characters in the <code>string</code> to lower case.
 * @param   str     a NULL-terminated string.
 */
void SAL_CALL rtl_str_toAsciiLowerCase_WithLength( sal_Char * str, sal_Int32 len );
void SAL_CALL rtl_str_toAsciiLowerCase( sal_Char * str );

/**
 * Converts all of the characters in the <code>string</code> to upper case.
 *
 * @param   str     a NULL-terminated string.
 */
void SAL_CALL rtl_str_toAsciiUpperCase_WithLength(sal_Char * str, sal_Int32 len);
void SAL_CALL rtl_str_toAsciiUpperCase(sal_Char * str);

/**
 * Removes white space from both ends of the string.
 * <p>
 * All characters that have codes less than or equal to
 * <code>'&#92;u0020'</code> (the space character) are considered to be
 * white space.
 *
 * @param   str     a NULL-terminated string.
 * @return  new length of the string.
 */
sal_Int32 SAL_CALL rtl_str_trim_WithLength( sal_Char * str, sal_Int32 len );
sal_Int32 SAL_CALL rtl_str_trim( sal_Char * str );


/*######################################################################*/

/**
 * A string with this reference count is static und must not deleted.
 * It is also not allowed to modifiy the reference count.
 */
#define RTL_STATIC_ASCIISTRING_REF  ((sal_Int32)0x80000000)
#define RTL_STRING_NEW(newStr)  rtl_string_new(newStr)

/**
 * Returns a new string resulting from replacing all occurrences of
 * <code>oldChar</code> in this string with <code>newChar</code>.
 * <p>
 * If the character <code>oldChar</code> does not occur in the
 * character sequence represented by the string, then the string is
 * returned.
 *
 * @param   oldChar   the old character.
 * @param   newChar   the new character.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_string_newReplace( rtl_String ** newStr,
                                     rtl_String * str,
                                     sal_Char oldChar,
                                     sal_Char newChar);

/**
 * Converts all of the characters in this <code>String</code> to lower
 * case using the rules of the given locale.
 * @param locale use the case transformation rules for this locale
 * @return the String, converted to lowercase.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_string_newToLowerCase( rtl_String ** newStr, rtl_String * str);

/**
 * Converts all of the characters in this <code>String</code> to upper
 * case using the rules of the given locale.
 * @param locale use the case transformation rules for this locale
 * @return the String, converted to uppercase.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_string_newToUpperCase( rtl_String ** newStr, rtl_String * str);

/**
 * Removes white space from both ends of the string.
 * <p>
 * All characters that have codes less than or equal to
 * <code>'&#92;u0020'</code> (the space character) are considered to be
 * white space.
 *
 * @return  this string, with white space removed from the front and end.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_string_newTrim( rtl_String ** newStr, rtl_String * str );

/**
 * Returns the number of tokens in this <code>String</code> seperated
 * by <code>cTok</code>.
 *
 * @param   cTok    the character which seperate the tokens
 * @return  the number of tokens.
 */
sal_Int32 SAL_CALL rtl_string_getTokenCount( rtl_String * str , sal_Char cTok);

/**
 * Returns a new string for the token specified by nToken. If nToken < 0 or
 * nToken > tokenCount then an empty string is returned.
 *
 * @param   nToken  the number of the token to return.
 * @param   cTok    the character which seperate the tokens
 * @param   newStr  the new string for the token
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_string_getToken( rtl_String ** newStr , rtl_String * str, sal_Int32 nToken, sal_Char cTok);

#ifdef __cplusplus
}
#endif

#endif /* _RTL_STRING_H_ */


