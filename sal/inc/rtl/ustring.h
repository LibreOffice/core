/*************************************************************************
 *
 *  $RCSfile: ustring.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 15:16:05 $
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

#ifndef _RTL_USTRING_H_
#define _RTL_USTRING_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================= */

/**
  Returns the length of a string.
  The length is equal to the number of 16-bit Unicode characters in the
  string without the terminating NULL-character.

  @param    str         must be a NULL-terminated string.
  @return   the length of the sequence of characters represented by this
            string, excluding the terminating NULL-character.
*/
sal_Int32 SAL_CALL rtl_ustr_getLength( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_compare( const sal_Unicode * first, const sal_Unicode * second ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_compare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

/**
  Returns a hashcode for a string.
  It is not allowed to store the hash code, because newer versions
  could return other hashcodes.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   a hash code value for str.
*/
sal_Int32 SAL_CALL rtl_ustr_hashCode( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_hashCode_WithLength( const sal_Unicode * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfChar( const sal_Unicode * str, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfChar_WithLength( const sal_Unicode * str, sal_Int32 len, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar( const sal_Unicode * str, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar_WithLength( const sal_Unicode * str, sal_Int32 len, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfStr( const sal_Unicode * str, const sal_Unicode * subStr ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfStr_WithLength( const sal_Unicode * str, sal_Int32 len, const sal_Unicode * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

/**
  Returns the index within the string of the last occurrence of
  the specified substring, searching backward.
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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr( const sal_Unicode * str, const sal_Unicode * subStr ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr_WithLength( const sal_Unicode * str, sal_Int32 len, const sal_Unicode * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

/**
  Replaces all occurrences of oldChar in the string with newChar.
  If the character oldChar does not occur in the character sequence
  represented by the string, then the string is not modified.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    oldChar     the old character.
  @param    newChar     the new character.
*/
void SAL_CALL rtl_ustr_replaceChar( sal_Unicode * str, sal_Unicode oldChar, sal_Unicode newChar ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_ustr_replaceChar_WithLength( sal_Unicode * str, sal_Int32 len, sal_Unicode oldChar, sal_Unicode newChar ) SAL_THROW_EXTERN_C();

/**
  Returns the string representation of the sal_Bool argument.
  If the sal_Bool is true, the buffer is filled with the
  string "True" and 5 is returned.
  If the sal_Bool is false, the buffer is filled with the
  string "False" and 6 is returned.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_USTR_MAX_VALUEOFBOOLEAN
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    b           a sal_Bool.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_ustr_valueOfBoolean( sal_Unicode * str, sal_Bool b ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFBOOLEAN RTL_STR_MAX_VALUEOFBOOLEAN

/**
  Returns the string representation of the char argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_USTR_MAX_VALUEOFCHAR
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    ch          a char.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_ustr_valueOfChar( sal_Unicode * str, sal_Unicode ch ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFCHAR RTL_STR_MAX_VALUEOFCHAR

/**
  Returns the string representation of the int argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_USTR_MAX_VALUEOFINT32
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    i           a int32.
  @param    radix       the radix (between 2 and 36)
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_ustr_valueOfInt32( sal_Unicode * str, sal_Int32 i, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MIN_RADIX          RTL_STR_MIN_RADIX
#define RTL_USTR_MAX_RADIX          RTL_STR_MAX_RADIX
#define RTL_USTR_MAX_VALUEOFINT32   RTL_STR_MAX_VALUEOFINT32

/**
  Returns the string representation of the long argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_USTR_MAX_VALUEOFINT64
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    l           a int64.
  @param    radix       the radix (between 2 and 36)
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_ustr_valueOfInt64( sal_Unicode * str, sal_Int64 l, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFINT64 RTL_STR_MAX_VALUEOFINT64

/**
  Returns the string representation of the float argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_USTR_MAX_VALUEOFFLOAT
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    f           a float.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_ustr_valueOfFloat( sal_Unicode * str, float f ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFFLOAT RTL_STR_MAX_VALUEOFFLOAT

/**
  Returns the string representation of the double argument.
  This function can't be used for language specific conversion.

  @param    str         a buffer, which is big enough to hold the result
                        and the terminating NULL-character.
                        You should use the RTL_USTR_MAX_VALUEOFDOUBLE
                        define to create a buffer, which is big enough.
                        It defines the maximum number of characters
                        with the terminating NULL-character.
  @param    d           a double.
  @return   the length of the string.
*/
sal_Int32 SAL_CALL rtl_ustr_valueOfDouble( sal_Unicode * str, double d ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFDOUBLE RTL_STR_MAX_VALUEOFDOUBLE

/**
  Returns the Boolean value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   sal_True, if the string is 1 or "True" in any ASCII case.
            sal_False in any other case.
*/
sal_Bool SAL_CALL rtl_ustr_toBoolean( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

/**
  Returns the int32 value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    radix       the radix (between 2 and 36)
  @return   the int32 represented by the string.
            0 if the string represents no number.
*/
sal_Int32 SAL_CALL rtl_ustr_toInt32( const sal_Unicode * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/**
  Returns the int64 value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @param    radix       the radix (between 2 and 36)
  @return   the int64 represented by the string.
            0 if the string represents no number.
*/
sal_Int64 SAL_CALL rtl_ustr_toInt64( const sal_Unicode * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/**
  Returns the float value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   the float represented by the string.
            0.0 if the string represents no number.
*/
float SAL_CALL rtl_ustr_toFloat( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

/**
  Returns the double value from the given string.
  This function can't be used for language specific conversion.
  The string must be NULL-terminated.

  @param    str         a NULL-terminated string.
  @return   the double represented by the string.
            0.0 if the string represents no number.
*/
double SAL_CALL rtl_ustr_toDouble( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

#ifdef SAL_W32
#pragma pack(push, 4)
#elif defined(SAL_OS2)
#pragma pack(1)
#endif

/**
  The implementation structure of a unicode string.
*/
typedef struct _rtl_uString
{
    sal_Int32       refCount;
    sal_Int32       length;
    sal_Unicode     buffer[1];
} rtl_uString;

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
void SAL_CALL rtl_uString_acquire( rtl_uString * str ) SAL_THROW_EXTERN_C();

/**
  Decrement the reference count of the string. If the count goes
  to zero than the string data is deleted.

  @param    str         the string.
*/
void SAL_CALL rtl_uString_release( rtl_uString * str ) SAL_THROW_EXTERN_C();

/**
  Allocates a new string containing no characters.

  @param    newStr      pointer to the new string. The data must be 0 or
                        a valid string.
*/
void SAL_CALL rtl_uString_new( rtl_uString ** newStr ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_new_WithLength( rtl_uString ** newStr, sal_Int32 nLen ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newFromString( rtl_uString ** newStr, const rtl_uString * value ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newFromStr( rtl_uString ** newStr, const sal_Unicode * value ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newFromStr_WithLength( rtl_uString ** newStr, const sal_Unicode * value, sal_Int32 len ) SAL_THROW_EXTERN_C();

/**
  Assign rightValue to *str. Release *str and aquire rightValue!

  @param    str         pointer to the string. The data must be 0 or
                        a valid string.
  @param    rightValue  a valid string.
*/
void SAL_CALL rtl_uString_assign( rtl_uString ** str, rtl_uString * rightValue ) SAL_THROW_EXTERN_C();

/**
  Returns the length of this string.
  The length is equal to the number of characters in the string.

  @param    str         a valid string.
  @return   the length of the sequence of characters represented by the
            string.
*/
sal_Int32 SAL_CALL rtl_uString_getLength( const rtl_uString * str ) SAL_THROW_EXTERN_C();

/**
  Returns the pointer to the character array of the string.

  @param    str         pointer to the string. The data must be
                        a valid string.
  @return   a null terminated character array.
*/
sal_Unicode * SAL_CALL rtl_uString_getStr( rtl_uString * str ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newConcat( rtl_uString ** newStr, rtl_uString * left, rtl_uString * right ) SAL_THROW_EXTERN_C();

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
  @param    index       the index for beginning.
                        The index must be greater or equal as 0 and
                        less or equal as the length of the string.
  @param    count       the count of charcters that will replaced
                        The count must be greater or equal as 0 and
                        less or equal as the length of the string minus index.
  @param    subStr      0 or a valid string, which is inserted at nIndex.
                        If subStr is 0, only a number of characters (count)
                        are deleted at the specified position (index).
*/
void SAL_CALL rtl_uString_newReplaceStrAt( rtl_uString ** newStr, rtl_uString * str, sal_Int32 index, sal_Int32 count, rtl_uString * subStr ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

/* predefined constants for String-Conversion */
#define OSTRING_TO_OUSTRING_CVTFLAGS    (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |\
                                         RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |\
                                         RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT)

/* ----------------------------------------------------------------------- */

/**
  Allocates a new Unicode string from a seequence of bytes and convert
  the byte sequence to Unicode with the specified text encoding.
  The string length could be a different one as the specified length,
  because not all text conversions result in the same unicode length
  (for example double byte encodings, UTF-7, UTF-8, ...).
  The reference count of the new string is 1 or an empty string.
  This function doesn't handle "Out of Memory" or other
  "bad memory allocations".

  @param    newStr          pointer to the new string. The data must be 0 or
                            a valid string.
  @param    str             a byte array.
  @param    len             the number of character which should be converted.
                            The byte array length must be greater or
                            equal than this value.
  @param    encoding        the text encoding from the byte array.
  @param    convertFlags    flags which controls the conversion.
                            see RTL_TEXTTOUNICODE_FLAGS_...
*/
void SAL_CALL rtl_string2UString( rtl_uString ** newStr, const sal_Char * str, sal_Int32 len, rtl_TextEncoding encoding, sal_uInt32 convertFlags ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

/* constAsciiStr must be a "..." or char const aFoo[] = "..." */
#define RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) constAsciiStr, sizeof( constAsciiStr )-1, RTL_TEXTENCODING_ASCII_US

/* ======================================================================= */
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
sal_Bool SAL_CALL rtl_ustr_equalsIgnoreCase_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen );
sal_Bool SAL_CALL rtl_ustr_equalsIgnoreCase( const sal_Unicode * first, const sal_Unicode * second );

/**
 * Compares two strings lexicographically. Both
 * strings must be NULL-terminated.
 * The comparison is based on the Unicode value of each character in
 * the strings.
 *
 * @param   first   the <code>String</code> to be compared.
 * @param   second  the <code>String</code> to compare first
 *                  <code>String</code> against.
 * @param   shortenedLength the number of characters which should be compared.
 *                  This length can be longer, shorter or equal than the both other strings.
 *
 * @return  the value <code>0</code> if the argument string is equal to
 *          this string; a value less than <code>0</code> if first string
 *          is lexicographically less than the second string; and a
 *          value greater than <code>0</code> if first string is
 *          lexicographically greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_shortenedCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen,
                                                     const sal_Unicode * second, sal_Int32 secondLen, sal_Int32 shortenedLength );

/**
 * Compares lexicographically a 16-Bit Unicode character string directly
 * with a 8-Bit ASCII character string. Since this function is optimized
 * for performance. the ASCII character values are not converted in any
 * way. The caller has to make sure that all ASCII characters are in the
 * allowed range between 0 and 127. strings must be NULL-terminated.
 *
 * @param   first   the <code>String</code> to be compared.
 * @param   second  the <code>String</code> to compare first
 *                  <code>String</code> against.
 *
 * @return  the value <code>0</code> if the argument string is equal to
 *          this string; a value less than <code>0</code> if first string
 *          is lexicographically less than the second string; and a
 *          value greater than <code>0</code> if first string is
 *          lexicographically greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compare_WithLength( const sal_Unicode * first, sal_Int32 firstLen,
                                                     const sal_Char * second );
sal_Int32 SAL_CALL rtl_ustr_ascii_compare( const sal_Unicode * first, const sal_Char * second );

/**
 * Compares the string reverse lexicographically with a 8-Bit ASCII
 * character string. <STRONG>The secondLength parameter is the length of
 * the ASCII string and not the number of characters which should be
 * compared.</STRONG> The reverse comparison is based on the
 * numerical values of each Unicode/ASCII character in the
 * strings with a 8-Bit ASCII character string. Since this
 * method is optimized for performance. the ASCII character
 * values are not converted in any way. The caller has to
 * ensure that all ASCII characters are in the allowed
 * range between 0 and 127.
 * The ASCII string must be NULL-terminated.
 *
 * @param   first   the <code>String</code> to be compared.
 * @param   second  the <code>String</code> to compare first
 *                  <code>String</code> against.
 * @param   secondLength the
 *
 * @return  the value <code>0</code> if the argument string is equal to
 *          this string; a value less than <code>0</code> if first string
 *          is lexicographically less than the second string; and a
 *          value greater than <code>0</code> if first string is
 *          lexicographically greater than the second string.
 *          <STRONG>The compare order is from the last character to the
 *          first one</STRONG>.
 */
sal_Int32 SAL_CALL rtl_ustr_asciil_reverseCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen,
                                                            const sal_Char * second, sal_Int32 secondLength );

/**
 * Compares lexicographically a 16-Bit Unicode character string directly
 * with a 8-Bit ASCII character string. Since this function is optimized
 * for performance. the ASCII character values are not converted in any
 * way. The caller has to make sure that all ASCII characters are in the
 * allowed range between 0 and 127. strings must be NULL-terminated.
 *
 * @param   first   the <code>String</code> to be compared.
 * @param   second  the <code>String</code> to compare first
 *                  <code>String</code> against.
 * @param   shortenedLength the number of characters which should be compared.
 *                  This length can be longer, shorter or equal than the both other strings.
 *
 * @return  the value <code>0</code> if the argument string is equal to
 *          this string; a value less than <code>0</code> if first string
 *          is lexicographically less than the second string; and a
 *          value greater than <code>0</code> if first string is
 *          lexicographically greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_shortenedCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen,
                                                     const sal_Char * second, sal_Int32 shortenedLength );

/**
 * Converts all of the characters in the <code>string</code> to lower case.
 * @param   str     a NULL-terminated string.
 */
void SAL_CALL rtl_ustr_toAsciiLowerCase_WithLength( sal_Unicode * str, sal_Int32 len );
void SAL_CALL rtl_ustr_toAsciiLowerCase( sal_Unicode * str );

/**
 * Converts all of the characters in the <code>string</code> to upper case.
 *
 * @param   str     a NULL-terminated string.
 */
void SAL_CALL rtl_ustr_toAsciiUpperCase_WithLength(sal_Unicode * str, sal_Int32 len);
void SAL_CALL rtl_ustr_toAsciiUpperCase(sal_Unicode * str);

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
sal_Int32 SAL_CALL rtl_ustr_trim_WithLength( sal_Unicode * str, sal_Int32 len );
sal_Int32 SAL_CALL rtl_ustr_trim( sal_Unicode * str );

/**
 * A string with this reference count is static und must not deleted.
 * It is also not allowed to modifiy the reference count.
 */
#define RTL_STATIC_STRING_REF   ((sal_Int32)0x80000000)

#ifdef SAL_W32
#pragma pack(push, 4)
#elif defined(SAL_OS2)
#pragma pack(1)
#endif

struct _rtl_Locale;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

#define RTL_USTRING_NEW(newStr) rtl_uString_new(newStr)

/**
 * Allocates a new <code>string</code> so that it represents the
 * sequence of characters currently contained in the character array
 * argument.
 *
 * @param  value   the initial value of the string.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newFromWStr( rtl_uString ** newStr, const wchar_t * value );

/**
 * Allocates a new <code>String</code> that contains characters from
 * the character array argument.
 *
 * @param      value    array that is the source of characters.
 * @param      len      the length of the array.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newFromWStr_WithLength( rtl_uString ** newStr, const wchar_t * value, sal_Int32 Len);

/**
 * Allocates a new <code>string</code> so that it represents the
 * sequence of ASCII characters currently contained in the sal_Char
 * array argument. Since this function is optimized for performance.
 * the ASCII character values are not converted in any way. The
 * caller has to make sure that all ASCII characters are in the
 * allowed range between 0 and 127.
 *
 * @param  value   the initial value of the string in ASCII characters.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newFromAscii( rtl_uString ** newStr, const sal_Char * value );

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
void SAL_CALL rtl_uString_newReplace( rtl_uString ** newStr,
                                      rtl_uString * str,
                                      sal_Unicode oldChar,
                                      sal_Unicode newChar);

/**
 * Converts all of the characters in this <code>String</code> to lower
 * case using the rules of the given locale.
 * @param locale use the case transformation rules for this locale
 * @return the String, converted to lowercase.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newToLowerCase( rtl_uString ** newStr, rtl_uString * str, struct _rtl_Locale * locale );

/**
 * Converts all of the characters in this <code>String</code> to upper
 * case using the rules of the given locale.
 * @param locale use the case transformation rules for this locale
 * @return the String, converted to uppercase.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newToUpperCase( rtl_uString ** newStr, rtl_uString * str, struct _rtl_Locale * locale );

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
void SAL_CALL rtl_uString_newTrim( rtl_uString ** newStr, rtl_uString * str );

/**
 * Returns the number of tokens in this <code>String</code> seperated
 * by <code>cTok</code>.
 *
 * @param   cTok    the character which seperate the tokens
 * @return  the number of tokens.
 */
sal_Int32 SAL_CALL rtl_uString_getTokenCount( rtl_uString * str , sal_Unicode cTok);

/**
 * Returns a new string for the token specified by nToken. If nToken < 0 or
 * nToken > tokenCount then an empty string is returned.
 *
 * @param   nToken  the number of the token to return.
 * @param   cTok    the character which seperate the tokens
 * @param   newStr  the new string for the token
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_getToken( rtl_uString ** newStr , rtl_uString * str, sal_Int32 nToken, sal_Unicode cTok);

#ifdef __cplusplus
}
#endif

#endif /* _RTL_USTRING_H_ */


