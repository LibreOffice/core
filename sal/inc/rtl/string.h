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

#ifndef _RTL_STRING_H_
#define _RTL_STRING_H_

#include <sal/types.h>
#include <osl/interlck.h>
#include <rtl/textcvt.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================= */

/** Return the length of a string.

    The length is equal to the number of 8-bit characters in the string,
    without the terminating NUL character.

    @param str
    a null-terminated string.

    @return
    the length of the sequence of characters represented by this string,
    excluding the terminating NUL character.
 */
sal_Int32 SAL_CALL rtl_str_getLength( const sal_Char * str ) SAL_THROW_EXTERN_C();

/** Compare two strings.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.  Both strings must be
    null-terminated.

    @param first
    the first null-terminated string to be compared.

    @param second
    the second null-terminated string which is compared with the first one.

    @return
    0 if both strings are equal, a value less than 0 if the first string is
    less than the second string, and a value greater than 0 if the first
    string is greater than the second string.
 */
sal_Int32 SAL_CALL rtl_str_compare( const sal_Char * first, const sal_Char * second ) SAL_THROW_EXTERN_C();

/** Compare two strings.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second string which is compared with the first one.  Need not be
    null-terminated, but must be at least as long as the specified secondLen.

    @param secondLen
    the length of the second string.

    @return
    0 if both strings are equal, a value less than 0 if the first string is
    less than the second string, and a value greater than 0 if the first
    string is greater than the second string.
 */
sal_Int32 SAL_CALL rtl_str_compare_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

/** Compare two strings with a maximum count of characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second string which is compared with the first one.  Need not be
    null-terminated, but must be at least as long as the specified secondLen.

    @param secondLen
    the length of the second string.

    @param shortenedLen
    the maximum number of characters to compare.  This length can be greater
    or smaller than the lengths of the two strings.

    @return
    0 if both substrings are equal, a value less than 0 if the first substring
    is less than the second substring, and a value greater than 0 if the first
    substring is greater than the second substring.
 */
sal_Int32 SAL_CALL rtl_str_shortenedCompare_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen, sal_Int32 shortenedLen ) SAL_THROW_EXTERN_C();

/** Compare two strings from back to front.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second string which is compared with the first one.  Need not be
    null-terminated, but must be at least as long as the specified secondLen.

    @param secondLen
    the length of the second string.

    @return
    0 if both strings are equal, a value less than 0 if the first string
    compares less than the second string, and a value greater than 0 if the
    first string compares greater than the second string.
 */
sal_Int32 SAL_CALL rtl_str_reverseCompare_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

/** Compare two strings, ignoring the case of ASCII characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  Character
    values between 65 and 90 (ASCII A--Z) are interpreted as values between 97
    and 122 (ASCII a--z).  This function cannot be used for language-specific
    sorting.  Both strings must be null-terminated.

    @param first
    the first null-terminated string to be compared.

    @param second
    the second null-terminated string which is compared with the first one.

    @return
    0 if both strings are equal, a value less than 0 if the first string is
    less than the second string, and a value greater than 0 if the first
    string is greater than the second string.
 */
sal_Int32 SAL_CALL rtl_str_compareIgnoreAsciiCase( const sal_Char * first, const sal_Char * second ) SAL_THROW_EXTERN_C();

/** Compare two strings, ignoring the case of ASCII characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  Character
    values between 65 and 90 (ASCII A--Z) are interpreted as values between 97
    and 122 (ASCII a--z).  This function cannot be used for language-specific
    sorting.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second string which is compared with the first one.  Need not be
    null-terminated, but must be at least as long as the specified secondLen.

    @param secondLen
    the length of the second string.

    @return
    0 if both strings are equal, a value less than 0 if the first string is
    less than the second string, and a value greater than 0 if the first
    string is greater than the second string.
 */
sal_Int32 SAL_CALL rtl_str_compareIgnoreAsciiCase_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

/** Compare two strings with a maximum count of characters, ignoring the case
    of ASCII characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  Character
    values between 65 and 90 (ASCII A--Z) are interpreted as values between 97
    and 122 (ASCII a--z).  This function cannot be used for language-specific
    sorting.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second string which is compared with the first one.  Need not be
    null-terminated, but must be at least as long as the specified secondLen.

    @param secondLen
    the length of the second string.

    @param shortenedLen
    the maximum number of characters to compare.  This length can be greater
    or smaller than the lengths of the two strings.

    @return
    0 if both substrings are equal, a value less than 0 if the first substring
    is less than the second substring, and a value greater than 0 if the first
    substring is greater than the second substring.
 */
sal_Int32 SAL_CALL rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( const sal_Char * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen, sal_Int32 shortenedLen ) SAL_THROW_EXTERN_C();

/** Return a hash code for a string.

    It is not allowed to store the hash code persistently, because later
    versions could return other hash codes.  The string must be
    null-terminated.

    @param str
    a null-terminated string.

    @return
    a hash code for the given string.
 */
sal_Int32 SAL_CALL rtl_str_hashCode( const sal_Char * str ) SAL_THROW_EXTERN_C();

/** Return a hash code for a string.

    It is not allowed to store the hash code persistently, because later
    versions could return other hash codes.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.

    @return
    a hash code for the given string.
 */
sal_Int32 SAL_CALL rtl_str_hashCode_WithLength( const sal_Char * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Search for the first occurrence of a character within a string.

    The string must be null-terminated.

    @param str
    a null-terminated string.

    @param ch
    the character to be searched for.

    @return
    the index (starting at 0) of the first occurrence of the character in the
    string, or -1 if the character does not occur.
 */
sal_Int32 SAL_CALL rtl_str_indexOfChar( const sal_Char * str, sal_Char ch ) SAL_THROW_EXTERN_C();

/** Search for the first occurrence of a character within a string.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.

    @param ch
    the character to be searched for.

    @return
    the index (starting at 0) of the first occurrence of the character in the
    string, or -1 if the character does not occur.
 */
sal_Int32 SAL_CALL rtl_str_indexOfChar_WithLength( const sal_Char * str, sal_Int32 len, sal_Char ch ) SAL_THROW_EXTERN_C();

/** Search for the last occurrence of a character within a string.

    The string must be null-terminated.

    @param str
    a null-terminated string.

    @param ch
    the character to be searched for.

    @return
    the index (starting at 0) of the last occurrence of the character in the
    string, or -1 if the character does not occur.  The returned value is
    always smaller than the string length.
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfChar( const sal_Char * str, sal_Char ch ) SAL_THROW_EXTERN_C();

/** Search for the last occurrence of a character within a string.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.

    @param ch
    the character to be searched for.

    @return
    the index (starting at 0) of the last occurrence of the character in the
    string, or -1 if the character does not occur.  The returned value is
    always smaller than the string length.
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfChar_WithLength( const sal_Char * str, sal_Int32 len, sal_Char ch ) SAL_THROW_EXTERN_C();

/** Search for the first occurrence of a substring within a string.

    If subStr is empty, or both str and subStr are empty, -1 is returned.
    Both strings must be null-terminated.

    @param str
    a null-terminated string.

    @param subStr
    the null-terminated substring to be searched for.

    @return
    the index (starting at 0) of the first character of the first occurrence
    of the substring within the string, or -1 if the substring does not occur.
 */
sal_Int32 SAL_CALL rtl_str_indexOfStr( const sal_Char * str, const sal_Char * subStr ) SAL_THROW_EXTERN_C();

/** Search for the first occurrence of a substring within a string.

    If subStr is empty, or both str and subStr are empty, -1 is returned.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.

    @param subStr
    the substring to be searched for.  Need not be null-terminated, but must
    be at least as long as the specified subLen.

    @param subLen
    the length of the substring.

    @return
    the index (starting at 0) of the first character of the first occurrence
    of the substring within the string, or -1 if the substring does not occur.
 */
sal_Int32 SAL_CALL rtl_str_indexOfStr_WithLength( const sal_Char * str, sal_Int32 len, const sal_Char * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

/** Search for the last occurrence of a substring within a string.

    If subStr is empty, or both str and subStr are empty, -1 is returned.
    Both strings must be null-terminated.

    @param str
    a null-terminated string.

    @param subStr
    the null-terminated substring to be searched for.

    @return
    the index (starting at 0) of the first character of the last occurrence
    of the substring within the string, or -1 if the substring does not occur.
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfStr( const sal_Char * str, const sal_Char * subStr ) SAL_THROW_EXTERN_C();

/** Search for the last occurrence of a substring within a string.

    If subStr is empty, or both str and subStr are empty, -1 is returned.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.

    @param subStr
    the substring to be searched for.  Need not be null-terminated, but must
    be at least as long as the specified subLen.

    @param subLen
    the length of the substring.

    @return
    the index (starting at 0) of the first character of the first occurrence
    of the substring within the string, or -1 if the substring does not occur.
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfStr_WithLength( const sal_Char * str, sal_Int32 len, const sal_Char * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

/** Replace all occurrences of a single character within a string.

    If oldChar does not occur within str, then the string is not modified.
    The string must be null-terminated.

    @param str
    a null-terminated string.

    @param oldChar
    the old character.

    @param newChar
    the new character.
 */
void SAL_CALL rtl_str_replaceChar( sal_Char * str, sal_Char oldChar, sal_Char newChar ) SAL_THROW_EXTERN_C();

/** Replace all occurrences of a single character within a string.

    If oldChar does not occur within str, then the string is not modified.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.

    @param oldChar
    the old character.

    @param newChar
    the new character.
 */
void SAL_CALL rtl_str_replaceChar_WithLength( sal_Char * str, sal_Int32 len, sal_Char oldChar, sal_Char newChar ) SAL_THROW_EXTERN_C();

/** Convert all ASCII uppercase letters to lowercase within a string.

    The characters with values between 65 and 90 (ASCII A--Z) are replaced
    with values between 97 and 122 (ASCII a--z).  The string must be
    null-terminated.

    @param str
    a null-terminated string.
 */
void SAL_CALL rtl_str_toAsciiLowerCase( sal_Char * str ) SAL_THROW_EXTERN_C();

/** Convert all ASCII uppercase letters to lowercase within a string.

    The characters with values between 65 and 90 (ASCII A--Z) are replaced
    with values between 97 and 122 (ASCII a--z).

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.
 */
void SAL_CALL rtl_str_toAsciiLowerCase_WithLength( sal_Char * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Convert all ASCII lowercase letters to uppercase within a string.

    The characters with values between 97 and 122 (ASCII a--z) are replaced
    with values between 65 and 90 (ASCII A--Z).  The string must be
    null-terminated.

    @param str
    a null-terminated string.
 */
void SAL_CALL rtl_str_toAsciiUpperCase( sal_Char * str ) SAL_THROW_EXTERN_C();

/** Convert all ASCII lowercase letters to uppercase within a string.

    The characters with values between 97 and 122 (ASCII a--z) are replaced
    with values between 65 and 90 (ASCII A--Z).

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.
 */
void SAL_CALL rtl_str_toAsciiUpperCase_WithLength( sal_Char * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Remove white space from both ends of a string.

    All characters with values less than or equal to 32 (the space character)
    are considered to be white space.  This function cannot be used for
    language-specific operations.  The string must be null-terminated.

    @param str
    a null-terminated string.

    @return
    the new length of the string.
 */
sal_Int32 SAL_CALL rtl_str_trim( sal_Char * str ) SAL_THROW_EXTERN_C();

/** Remove white space from both ends of the string.

    All characters with values less than or equal to 32 (the space character)
    are considered to be white space.  This function cannot be used for
    language-specific operations.  The string must be null-terminated.

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the original length of the string.

    @return
    the new length of the string.
 */
sal_Int32 SAL_CALL rtl_str_trim_WithLength( sal_Char * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Create the string representation of a boolean.

    If b is true, the buffer is filled with the string "true" and 5 is
    returned.  If b is false, the buffer is filled with the string "false" and
    6 is returned.  This function cannot be used for language-specific
    operations.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_STR_MAX_VALUEOFBOOLEAN define to create
    a buffer that is big enough.

    @param b
    a boolean value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_str_valueOfBoolean( sal_Char * str, sal_Bool b ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFBOOLEAN 6

/** Create the string representation of a character.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_STR_MAX_VALUEOFCHAR define to create a
    buffer that is big enough.

    @param ch
    a character value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_str_valueOfChar( sal_Char * str, sal_Char ch ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFCHAR 2

/** Create the string representation of an integer.

    This function cannot be used for language-specific operations.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_STR_MAX_VALUEOFINT32 define to create a
    buffer that is big enough.

    @param i
    an integer value.

    @param radix
    the radix.  Must be between RTL_STR_MIN_RADIX (2) and RTL_STR_MAX_RADIX
    (36), inclusive.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_str_valueOfInt32( sal_Char * str, sal_Int32 i, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_STR_MIN_RADIX           2
#define RTL_STR_MAX_RADIX           36
#define RTL_STR_MAX_VALUEOFINT32    33

/** Create the string representation of a long integer.

    This function cannot be used for language-specific operations.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_STR_MAX_VALUEOFINT64 define to create a
    buffer that is big enough.

    @param l
    a long integer value.

    @param radix
    the radix.  Must be between RTL_STR_MIN_RADIX (2) and RTL_STR_MAX_RADIX
    (36), inclusive.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_str_valueOfInt64( sal_Char * str, sal_Int64 l, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFINT64 65

/** Create the string representation of a float.

    This function cannot be used for language-specific conversion.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_STR_MAX_VALUEOFFLOAT define to create a
    buffer that is big enough.

    @param f
    a float value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_str_valueOfFloat( sal_Char * str, float f ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFFLOAT 15

/** Create the string representation of a double.

    This function cannot be used for language-specific conversion.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_STR_MAX_VALUEOFDOUBLE define to create
    a buffer that is big enough.

    @param d
    a double value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_str_valueOfDouble( sal_Char * str, double d ) SAL_THROW_EXTERN_C();
#define RTL_STR_MAX_VALUEOFDOUBLE 25

/** Interpret a string as a boolean.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @return
    true if the string is "1" or "true" in any ASCII case, false otherwise.
 */
sal_Bool SAL_CALL rtl_str_toBoolean( const sal_Char * str ) SAL_THROW_EXTERN_C();

/** Interpret a string as an integer.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @param radix
    the radix.  Must be between RTL_STR_MIN_RADIX (2) and RTL_STR_MAX_RADIX
    (36), inclusive.

    @return
    the integer value represented by the string, or 0 if the string does not
    represent an integer.
 */
sal_Int32 SAL_CALL rtl_str_toInt32( const sal_Char * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/** Interpret a string as a long integer.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @param radix
    the radix.  Must be between RTL_STR_MIN_RADIX (2) and RTL_STR_MAX_RADIX
    (36), inclusive.

    @return
    the long integer value represented by the string, or 0 if the string does
    not represent a long integer.
 */
sal_Int64 SAL_CALL rtl_str_toInt64( const sal_Char * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/** Interpret a string as a float.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @return
    the float value represented by the string, or 0.0 if the string does not
    represent a float.
 */
float SAL_CALL rtl_str_toFloat( const sal_Char * str ) SAL_THROW_EXTERN_C();

/** Interpret a string as a double.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @return
    the float value represented by the string, or 0.0 if the string does not
    represent a double.
 */
double SAL_CALL rtl_str_toDouble( const sal_Char * str ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

#ifdef SAL_W32
#   pragma pack(push, 8)
#elif defined(SAL_OS2)
#   pragma pack(push, 4)
#endif

/** The implementation of a byte string.

    @internal
 */
typedef struct _rtl_String
{
    oslInterlockedCount refCount; /* opaque */
    sal_Int32           length;
    sal_Char            buffer[1];
} rtl_String;

#if defined( SAL_W32) ||  defined(SAL_OS2)
#pragma pack(pop)
#endif

/* ----------------------------------------------------------------------- */

/** Increment the reference count of a string.

    @param str
    a string.
 */
void SAL_CALL rtl_string_acquire( rtl_String * str ) SAL_THROW_EXTERN_C();

/** Decrement the reference count of a string.

    If the count goes to zero than the string data is deleted.

    @param str
    a string.
 */
void SAL_CALL rtl_string_release( rtl_String * str ) SAL_THROW_EXTERN_C();

/** Allocate a new string containing no characters.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.
 */
void SAL_CALL rtl_string_new( rtl_String ** newStr ) SAL_THROW_EXTERN_C();

/** Allocate a new string containing space for a given number of characters.

    If len is greater than zero, the reference count of the new string will be
    1.  The values of all characters are set to 0 and the length of the string
    is 0.  This function does not handle out-of-memory conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param len
    the number of characters.
 */
void SAL_CALL rtl_string_new_WithLength( rtl_String ** newStr, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Allocate a new string that contains a copy of another string.

    If the length of value is greater than zero, the reference count of the
    new string will be 1.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param value
    a valid string.
 */
void SAL_CALL rtl_string_newFromString( rtl_String ** newStr, const rtl_String * value ) SAL_THROW_EXTERN_C();

/** Allocate a new string that contains a copy of a character array.

    If the length of value is greater than zero, the reference count of the
    new string will be 1.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param value
    a null-terminated character array.
 */
void SAL_CALL rtl_string_newFromStr( rtl_String ** newStr, const sal_Char * value ) SAL_THROW_EXTERN_C();

/** Allocate a new string that contains a copy of a character array.

    If the length of value is greater than zero, the reference count of the
    new string will be 1.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param value
    a character array.  Need not be null-terminated, but must be at least as
    long as the specified len.

    @param len
    the length of the character array.
 */
void SAL_CALL rtl_string_newFromStr_WithLength( rtl_String ** newStr, const sal_Char * value, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Assign a new value to a string.

    First releases any value str might currently hold, then acquires
    rightValue.

    @param str
    pointer to the string.  The pointed-to data must be null or a valid
    string.

    @param rightValue
    a valid string.
 */
void SAL_CALL rtl_string_assign( rtl_String ** str, rtl_String * rightValue ) SAL_THROW_EXTERN_C();

/** Return the length of a string.

    The length is equal to the number of characters in the string.

    @param str
    a valid string.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_string_getLength( const rtl_String * str ) SAL_THROW_EXTERN_C();

/** Return a pointer to the underlying character array of a string.

    @param str
    a valid string.

    @return
    a pointer to the null-terminated character array.
 */
sal_Char * SAL_CALL rtl_string_getStr( rtl_String * str ) SAL_THROW_EXTERN_C();

/** Create a new string that is the concatenation of two other strings.

    The new string does not necessarily have a reference count of 1 (in cases
    where one of the two other strings is empty), so it must not be modified
    without checking the reference count.  This function does not handle
    out-of-memory conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param left
    a valid string.

    @param right
    a valid string.
 */
void SAL_CALL rtl_string_newConcat( rtl_String ** newStr, rtl_String * left, rtl_String * right ) SAL_THROW_EXTERN_C();

/** Create a new string by replacing a substring of another string.

    The new string results from replacing a number of characters (count),
    starting at the specified position (index) in the original string (str),
    with some new substring (subStr).  If subStr is null, than only a number
    of characters is deleted.

    The new string does not necessarily have a reference count of 1, so it
    must not be modified without checking the reference count.  This function
    does not handle out-of-memory conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a valid string.

    @param index
    the index into str at which to start replacement.  Must be between 0 and
    the length of str, inclusive.

    @param count
    the number of charcters to remove.  Must not be negative, and the sum of
    index and count must not exceed the length of str.

    @param subStr
    either null or a valid string to be inserted.
 */
void SAL_CALL rtl_string_newReplaceStrAt( rtl_String ** newStr, rtl_String * str, sal_Int32 idx, sal_Int32 count, rtl_String * subStr ) SAL_THROW_EXTERN_C();

/** Create a new string by replacing all occurrences of a single character
    within another string.

    The new string results from replacing all occurrences of oldChar in str
    with newChar.

    The new string does not necessarily have a reference count of 1 (in cases
    where oldChar does not occur in str), so it must not be modified without
    checking the reference count.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a valid string.

    @param oldChar
    the old character.

    @param newChar
    the new character.
 */
void SAL_CALL rtl_string_newReplace( rtl_String ** newStr, rtl_String * str, sal_Char oldChar, sal_Char newChar ) SAL_THROW_EXTERN_C();

/** Create a new string by converting all ASCII uppercase letters to lowercase
    within another string.

    The new string results from replacing all characters with values between
    65 and 90 (ASCII A--Z) by values between 97 and 122 (ASCII a--z).

    This function cannot be used for language-specific conversion.  The new
    string does not necessarily have a reference count of 1 (in cases where
    no characters need to be converted), so it must not be modified without
    checking the reference count.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a valid string.
 */
void SAL_CALL rtl_string_newToAsciiLowerCase( rtl_String ** newStr, rtl_String * str ) SAL_THROW_EXTERN_C();

/** Create a new string by converting all ASCII lowercase letters to uppercase
    within another string.

    The new string results from replacing all characters with values between
    97 and 122 (ASCII a--z) by values between 65 and 90 (ASCII A--Z).

    This function cannot be used for language-specific conversion.  The new
    string does not necessarily have a reference count of 1 (in cases where
    no characters need to be converted), so it must not be modified without
    checking the reference count.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a valid string.
 */
void SAL_CALL rtl_string_newToAsciiUpperCase( rtl_String ** newStr, rtl_String * str ) SAL_THROW_EXTERN_C();

/** Create a new string by removing white space from both ends of another
    string.

    The new string results from removing all characters with values less than
    or equal to 32 (the space character) form both ends of str.

    This function cannot be used for language-specific conversion.  The new
    string does not necessarily have a reference count of 1 (in cases where
    no characters need to be removed), so it must not be modified without
    checking the reference count.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a valid string.
 */
void SAL_CALL rtl_string_newTrim( rtl_String ** newStr, rtl_String * str ) SAL_THROW_EXTERN_C();

/** Create a new string by extracting a single token from another string.

    Starting at index, the token's next token is searched for.  If there is no
    such token, the result is an empty string.  Otherwise, all characters from
    the start of that token and up to, but not including the next occurrence
    of cTok make up the resulting token.  The return value is the position of
    the next token, or -1 if no more tokens follow.

    Example code could look like
      rtl_String * pToken = NULL;
      sal_Int32 nIndex = 0;
      do
      {
          ...
          nIndex = rtl_string_getToken(&pToken, pStr, 0, ';', nIndex);
          ...
      }
      while (nIndex >= 0);

    The new string does not necessarily have a reference count of 1, so it
    must not be modified without checking the reference count.  This function
    does not handle out-of-memory conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.  If either token or index is negative, an empty token is stored in
    newStr (and -1 is returned).

    @param str
    a valid string.

    @param token
    the number of the token to return, starting at index.

    @param cTok
    the character that seperates the tokens.

    @param index
    the position at which searching for the token starts.  Must not be greater
    than the length of str.

    @return
    the index of the next token, or -1 if no more tokens follow.
 */
sal_Int32 SAL_CALL rtl_string_getToken( rtl_String ** newStr , rtl_String * str, sal_Int32 token, sal_Char cTok, sal_Int32 idx ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

/** Supply an ASCII string literal together with its length.

    This macro can be used to compute (some of) the arguments in function calls
    like rtl::OString(RTL_CONSTASCII_STRINGPARAM("foo")) or
    rtl::OUString::equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("foo")).

    @param constAsciiStr
    must be an expression of type "(possibly cv-qualified reference to) array of
    (possibly cv-qualified) char."  Each element of the referenced array must
    represent an ASCII value in the range 0x00--0x7F.  The last element of the
    referenced array is not considered part of the represented ASCII string, and
    its value should be 0x00.  Depending on where this macro is used, the nature
    of the supplied expression might be further restricted.
*/
#define RTL_CONSTASCII_STRINGPARAM( constAsciiStr ) constAsciiStr, ((sal_Int32)SAL_N_ELEMENTS(constAsciiStr)-1)

/** Supply the length of an ASCII string literal.

    This macro can be used to compute arguments in function calls like
    rtl::OUString::match(other, RTL_CONSTASCII_LENGTH("prefix")).

    @param constAsciiStr
    must be an expression of type "(possibly cv-qualified reference to) array of
    (possibly cv-qualified) char."  Each element of the referenced array must
    represent an ASCII value in the range 0x00--0x7F.  The last element of the
    referenced array is not considered part of the represented ASCII string, and
    its value should be 0x00.  Depending on where this macro is used, the nature
    of the supplied expression might be further restricted.
*/
#define RTL_CONSTASCII_LENGTH( constAsciiStr ) ((sal_Int32)(SAL_N_ELEMENTS(constAsciiStr)-1))

/* ======================================================================= */

/* predefined constants for String-Conversion */
#define OUSTRING_TO_OSTRING_CVTFLAGS    (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |\
                                         RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |\
                                         RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |\
                                         RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 |\
                                         RTL_UNICODETOTEXT_FLAGS_NOCOMPOSITE)

/* ----------------------------------------------------------------------- */

/** Create a new byte string by converting a Unicode string, using a specific
    text encoding.

    The lengths of the byte string and the Unicode string may differ (e.g.,
    for double-byte encodings, UTF-7, UTF-8).

    If the length of the Unicode string is greater than zero, the reference
    count of the new string will be 1.

    If an out-of-memory condition occurs, newStr will point to a null pointer
    upon return.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a Unicode character array.  Need not be null-terminated, but must be at
    least as long as the specified len.

    @param len
    the length of the Unicode character array.

    @param encoding
    the text encoding to use for conversion.

    @param convertFlags
    flags which control the conversion.  Either use
    OUSTRING_TO_OSTRING_CVTFLAGS, or see
    <http://udk.openoffice.org/cpp/man/spec/textconversion.html> for more
    details.
 */
void SAL_CALL rtl_uString2String( rtl_String ** newStr, const sal_Unicode * str, sal_Int32 len, rtl_TextEncoding encoding, sal_uInt32 convertFlags ) SAL_THROW_EXTERN_C();

/**
  Converts a Unicode string to a byte string, signalling failure.

  @param pTarget
  An out parameter receiving the converted string.  Must not be null itself, and
  must contain either null or a pointer to a valid rtl_String; the contents are
  not modified if conversion fails (rtl_convertUStringToString returns false).

  @param pSource
  The Unicode string.  May only be null if nLength is zero.

  @param nLength
  The length of the Unicode string.  Must be non-negative.

  @param nEncoding
  The text encoding to convert into.  Must be an octet encoding (i.e.,
  rtl_isOctetTextEncoding(nEncoding) must return true).

  @param nFlags
  A combination of RTL_UNICODETOTEXT_FLAGS that detail how to do the conversion
  (see rtl_convertUnicodeToText).  RTL_UNICODETOTEXT_FLAGS_FLUSH need not be
  included, it is implicitly assumed.  Typical uses are either
  RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
  RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR (fail if a Unicode character cannot be
  converted to the target nEncoding) or OUSTRING_TO_OSTRING_CVTFLAGS (make a
  best efforts conversion).

  @return
  True if the conversion succeeded, false otherwise.
 */
sal_Bool SAL_CALL rtl_convertUStringToString(rtl_String ** pTarget,
                                             sal_Unicode const * pSource,
                                             sal_Int32 nLength,
                                             rtl_TextEncoding nEncoding,
                                             sal_uInt32 nFlags)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif /* _RTL_STRING_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
