/*************************************************************************
 *
 *  $RCSfile: ustring.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:45:52 $
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

/** Return the length of a string.

    The length is equal to the number of 16-bit Unicode characters in the
    string, without the terminating NUL character.

    @param str
    a null-terminated string.

    @return
    the length of the sequence of characters represented by this string,
    excluding the terminating NUL character.
 */
sal_Int32 SAL_CALL rtl_ustr_getLength( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_compare( const sal_Unicode * first, const sal_Unicode * second ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_compare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_shortenedCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen, sal_Int32 shortenedLen ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_reverseCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_compareIgnoreAsciiCase( const sal_Unicode * first, const sal_Unicode * second ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_compareIgnoreAsciiCase_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen, sal_Int32 shortenedLen ) SAL_THROW_EXTERN_C();

/** Compare two strings.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.  Both strings must be
    null-terminated.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first null-terminated string to be compared.

    @param second
    the second null-terminated ASCII string which is compared with the first
    one.

    @return
    0 if both substrings are equal, a value less than 0 if the first substring
    is less than the second substring, and a value greater than 0 if the first
    substring is greater than the second substring.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compare( const sal_Unicode * first, const sal_Char * second ) SAL_THROW_EXTERN_C();

/** Compare two strings.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second null-terminated ASCII string which is compared with the first
    one.

    @return
    0 if both substrings are equal, a value less than 0 if the first substring
    is less than the second substring, and a value greater than 0 if the first
    substring is greater than the second substring.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Char * second ) SAL_THROW_EXTERN_C();

/** Compare two strings with a maximum count of characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second null-terminated ASCII string which is compared with the first
    one.

    @param shortenedLen
    the maximum number of characters to compare.  This length can be greater
    or smaller than the lengths of the two strings.

    @return
    0 if both substrings are equal, a value less than 0 if the first substring
    is less than the second substring, and a value greater than 0 if the first
    substring is greater than the second substring.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_shortenedCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 shortenedLen ) SAL_THROW_EXTERN_C();

/** Compare two strings from back to front.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  This function
    cannot be used for language-specific sorting.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second ASCII string which is compared with the first one.  Need not be
    null-terminated, but must be at least as long as the specified secondLen.

    @param secondLen
    the length of the second string.

    @return
    0 if both strings are equal, a value less than 0 if the first string
    compares less than the second string, and a value greater than 0 if the
    first string compares greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_asciil_reverseCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 secondLen ) SAL_THROW_EXTERN_C();

/** Compare two strings, ignoring the case of ASCII characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  Character
    values between 65 and 90 (ASCII A--Z) are interpreted as values between 97
    and 122 (ASCII a--z).  This function cannot be used for language-specific
    sorting.  Both strings must be null-terminated.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first null-terminated string to be compared.

    @param second
    the second null-terminated ASCII string which is compared with the first
    one.

    @return
    0 if both strings are equal, a value less than 0 if the first string is
    less than the second string, and a value greater than 0 if the first
    string is greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compareIgnoreAsciiCase( const sal_Unicode * first, const sal_Char * second ) SAL_THROW_EXTERN_C();

/** Compare two strings, ignoring the case of ASCII characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  Character
    values between 65 and 90 (ASCII A--Z) are interpreted as values between 97
    and 122 (ASCII a--z).  This function cannot be used for language-specific
    sorting.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second null-terminated ASCII string which is compared with the first
    one.

    @return
    0 if both strings are equal, a value less than 0 if the first string is
    less than the second string, and a value greater than 0 if the first
    string is greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Char * second ) SAL_THROW_EXTERN_C();

/** Compare two strings with a maximum count of characters, ignoring the case
    of ASCII characters.

    The comparison is based on the numeric value of each character in the
    strings and returns a value indicating their relationship.  Character
    values between 65 and 90 (ASCII A--Z) are interpreted as values between 97
    and 122 (ASCII a--z).  This function cannot be used for language-specific
    sorting.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param first
    the first string to be compared.  Need not be null-terminated, but must be
    at least as long as the specified firstLen.

    @param firstLen
    the length of the first string.

    @param second
    the second null-terminated ASCII string which is compared with the first
    one.

    @param shortenedLen
    the maximum number of characters to compare.  This length can be greater
    or smaller than the lengths of the two strings.

    @return
    0 if both substrings are equal, a value less than 0 if the first substring
    is less than the second substring, and a value greater than 0 if the first
    substring is greater than the second substring.
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Char * second, sal_Int32 shortenedLen ) SAL_THROW_EXTERN_C();

/** Return a hash code for a string.

    It is not allowed to store the hash code persistently, because later
    versions could return other hash codes.  The string must be
    null-terminated.

    @param str
    a null-terminated string.

    @return
    a hash code for the given string.
 */
sal_Int32 SAL_CALL rtl_ustr_hashCode( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_hashCode_WithLength( const sal_Unicode * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfChar( const sal_Unicode * str, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfChar_WithLength( const sal_Unicode * str, sal_Int32 len, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar( const sal_Unicode * str, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar_WithLength( const sal_Unicode * str, sal_Int32 len, sal_Unicode ch ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfStr( const sal_Unicode * str, const sal_Unicode * subStr ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_indexOfStr_WithLength( const sal_Unicode * str, sal_Int32 len, const sal_Unicode * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr( const sal_Unicode * str, const sal_Unicode * subStr ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr_WithLength( const sal_Unicode * str, sal_Int32 len, const sal_Unicode * subStr, sal_Int32 subLen ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_ustr_replaceChar( sal_Unicode * str, sal_Unicode oldChar, sal_Unicode newChar ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_ustr_replaceChar_WithLength( sal_Unicode * str, sal_Int32 len, sal_Unicode oldChar, sal_Unicode newChar ) SAL_THROW_EXTERN_C();

/** Convert all ASCII uppercase letters to lowercase within a string.

    The characters with values between 65 and 90 (ASCII A--Z) are replaced
    with values between 97 and 122 (ASCII a--z).  The string must be
    null-terminated.

    @param str
    a null-terminated string.
 */
void SAL_CALL rtl_ustr_toAsciiLowerCase( sal_Unicode * str ) SAL_THROW_EXTERN_C();

/** Convert all ASCII uppercase letters to lowercase within a string.

    The characters with values between 65 and 90 (ASCII A--Z) are replaced
    with values between 97 and 122 (ASCII a--z).

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.
 */
void SAL_CALL rtl_ustr_toAsciiLowerCase_WithLength( sal_Unicode * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Convert all ASCII lowercase letters to uppercase within a string.

    The characters with values between 97 and 122 (ASCII a--z) are replaced
    with values between 65 and 90 (ASCII A--Z).  The string must be
    null-terminated.

    @param str
    a null-terminated string.
 */
void SAL_CALL rtl_ustr_toAsciiUpperCase( sal_Unicode * str ) SAL_THROW_EXTERN_C();

/** Convert all ASCII lowercase letters to uppercase within a string.

    The characters with values between 97 and 122 (ASCII a--z) are replaced
    with values between 65 and 90 (ASCII A--Z).

    @param str
    a string.  Need not be null-terminated, but must be at least as long as
    the specified len.

    @param len
    the length of the string.
 */
void SAL_CALL rtl_ustr_toAsciiUpperCase_WithLength( sal_Unicode * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Remove white space from both ends of a string.

    All characters with values less than or equal to 32 (the space character)
    are considered to be white space.  This function cannot be used for
    language-specific operations.  The string must be null-terminated.

    @param str
    a null-terminated string.

    @return
    the new length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_trim( sal_Unicode * str ) SAL_THROW_EXTERN_C();

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
sal_Int32 SAL_CALL rtl_ustr_trim_WithLength( sal_Unicode * str, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Create the string representation of a boolean.

    If b is true, the buffer is filled with the string "true" and 5 is
    returned.  If b is false, the buffer is filled with the string "false" and
    6 is returned.  This function cannot be used for language-specific
    operations.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_USTR_MAX_VALUEOFBOOLEAN define to
    create a buffer that is big enough.

    @param b
    a boolean value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfBoolean( sal_Unicode * str, sal_Bool b ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFBOOLEAN RTL_STR_MAX_VALUEOFBOOLEAN

/** Create the string representation of a character.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_USTR_MAX_VALUEOFCHAR define to create a
    buffer that is big enough.

    @param ch
    a character value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfChar( sal_Unicode * str, sal_Unicode ch ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFCHAR RTL_STR_MAX_VALUEOFCHAR

/** Create the string representation of an integer.

    This function cannot be used for language-specific operations.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_USTR_MAX_VALUEOFINT32 define to create
    a buffer that is big enough.

    @param i
    an integer value.

    @param radix
    the radix.  Must be between RTL_USTR_MIN_RADIX (2) and RTL_USTR_MAX_RADIX
    (36), inclusive.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfInt32( sal_Unicode * str, sal_Int32 i, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MIN_RADIX          RTL_STR_MIN_RADIX
#define RTL_USTR_MAX_RADIX          RTL_STR_MAX_RADIX
#define RTL_USTR_MAX_VALUEOFINT32   RTL_STR_MAX_VALUEOFINT32

/** Create the string representation of a long integer.

    This function cannot be used for language-specific operations.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_USTR_MAX_VALUEOFINT64 define to create
    a buffer that is big enough.

    @param l
    a long integer value.

    @param radix
    the radix.  Must be between RTL_USTR_MIN_RADIX (2) and RTL_USTR_MAX_RADIX
    (36), inclusive.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfInt64( sal_Unicode * str, sal_Int64 l, sal_Int16 radix ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFINT64 RTL_STR_MAX_VALUEOFINT64

/** Create the string representation of a float.

    This function cannot be used for language-specific conversion.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_USTR_MAX_VALUEOFFLOAT define to create
    a buffer that is big enough.

    @param f
    a float value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfFloat( sal_Unicode * str, float f ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFFLOAT RTL_STR_MAX_VALUEOFFLOAT

/** Create the string representation of a double.

    This function cannot be used for language-specific conversion.

    @param str
    a buffer that is big enough to hold the result and the terminating NUL
    character.  You should use the RTL_USTR_MAX_VALUEOFDOUBLE define to create
    a buffer that is big enough.

    @param d
    a double value.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfDouble( sal_Unicode * str, double d ) SAL_THROW_EXTERN_C();
#define RTL_USTR_MAX_VALUEOFDOUBLE RTL_STR_MAX_VALUEOFDOUBLE

/** Interpret a string as a boolean.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @return
    true if the string is "1" or "true" in any ASCII case, false otherwise.
 */
sal_Bool SAL_CALL rtl_ustr_toBoolean( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

/** Interpret a string as an integer.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @param radix
    the radix.  Must be between RTL_USTR_MIN_RADIX (2) and RTL_USTR_MAX_RADIX
    (36), inclusive.

    @return
    the integer value represented by the string, or 0 if the string does not
    represent an integer.
 */
sal_Int32 SAL_CALL rtl_ustr_toInt32( const sal_Unicode * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/** Interpret a string as a long integer.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @param radix
    the radix.  Must be between RTL_USTR_MIN_RADIX (2) and RTL_USTR_MAX_RADIX
    (36), inclusive.

    @return
    the long integer value represented by the string, or 0 if the string does
    not represent a long integer.
 */
sal_Int64 SAL_CALL rtl_ustr_toInt64( const sal_Unicode * str, sal_Int16 radix ) SAL_THROW_EXTERN_C();

/** Interpret a string as a float.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @return
    the float value represented by the string, or 0.0 if the string does not
    represent a float.
 */
float SAL_CALL rtl_ustr_toFloat( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

/** Interpret a string as a double.

    This function cannot be used for language-specific conversion.  The string
    must be null-terminated.

    @param str
    a null-terminated string.

    @return
    the float value represented by the string, or 0.0 if the string does not
    represent a double.
 */
double SAL_CALL rtl_ustr_toDouble( const sal_Unicode * str ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

#ifdef SAL_W32
#pragma pack(push, 4)
#elif defined(SAL_OS2)
#pragma pack(1)
#endif

/** The implementation of a Unicode string.

    @internal
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

/** Increment the reference count of a string.

    @param str
    a string.
 */
void SAL_CALL rtl_uString_acquire( rtl_uString * str ) SAL_THROW_EXTERN_C();

/** Decrement the reference count of a string.

    If the count goes to zero than the string data is deleted.

    @param str
    a string.
 */
void SAL_CALL rtl_uString_release( rtl_uString * str ) SAL_THROW_EXTERN_C();

/** Allocate a new string containing no characters.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.
 */
void SAL_CALL rtl_uString_new( rtl_uString ** newStr ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_new_WithLength( rtl_uString ** newStr, sal_Int32 nLen ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newFromString( rtl_uString ** newStr, const rtl_uString * value ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newFromStr( rtl_uString ** newStr, const sal_Unicode * value ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newFromStr_WithLength( rtl_uString ** newStr, const sal_Unicode * value, sal_Int32 len ) SAL_THROW_EXTERN_C();

/** Allocate a new string that contains a copy of a character array.

    If the length of value is greater than zero, the reference count of the
    new string will be 1.  This function does not handle out-of-memory
    conditions.

    Since this function is optimized for performance, the ASCII character
    values are not converted in any way.  The caller has to make sure that
    all ASCII characters are in the allowed range of 0 and 127, inclusive.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param value
    a null-terminated ASCII character array.
 */
void SAL_CALL rtl_uString_newFromAscii( rtl_uString ** newStr, const sal_Char * value ) SAL_THROW_EXTERN_C();

/** Assign a new value to a string.

    First releases any value str might currently hold, then acquires
    rightValue.

    @param str
    pointer to the string.  The pointed-to data must be null or a valid
    string.

    @param rightValue
    a valid string.
 */
void SAL_CALL rtl_uString_assign( rtl_uString ** str, rtl_uString * rightValue ) SAL_THROW_EXTERN_C();

/** Return the length of a string.

    The length is equal to the number of characters in the string.

    @param str
    a valid string.

    @return
    the length of the string.
 */
sal_Int32 SAL_CALL rtl_uString_getLength( const rtl_uString * str ) SAL_THROW_EXTERN_C();

/** Return a pointer to the underlying character array of a string.

    @param str
    a valid string.

    @return
    a pointer to the null-terminated character array.
 */
sal_Unicode * SAL_CALL rtl_uString_getStr( rtl_uString * str ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newConcat( rtl_uString ** newStr, rtl_uString * left, rtl_uString * right ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newReplaceStrAt( rtl_uString ** newStr, rtl_uString * str, sal_Int32 index, sal_Int32 count, rtl_uString * subStr ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newReplace( rtl_uString ** newStr, rtl_uString * str, sal_Unicode oldChar, sal_Unicode newChar ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newToAsciiLowerCase( rtl_uString ** newStr, rtl_uString * str ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newToAsciiUpperCase( rtl_uString ** newStr, rtl_uString * str ) SAL_THROW_EXTERN_C();

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
void SAL_CALL rtl_uString_newTrim( rtl_uString ** newStr, rtl_uString * str ) SAL_THROW_EXTERN_C();

/** Create a new string by extracting a single token from another string.

    Starting at index, the token's next token is searched for.  If there is no
    such token, the result is an empty string.  Otherwise, all characters from
    the start of that token and up to, but not including the next occurrence
    of cTok make up the resulting token.  The return value is the position of
    the next token, or -1 if no more tokens follow.

    Example code could look like
      rtl_uString * pToken = NULL;
      sal_Int32 nIndex = 0;
      do
      {
          ...
          nIndex = rtl_uString_getToken(&pToken, pStr, 0, ';', nIndex);
          ...
      }
      while (nIndex >= 0);

    The new string does not necessarily have a reference count of 1, so it
    must not be modified without checking the reference count.  This function
    does not handle out-of-memory conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a valid string.

    @param token
    the number of the token to return, starting at index.  Must be
    non-negative.

    @param cTok
    the character that seperates the tokens.

    @param index
    the position at which searching for the token starts.  Must be between 0
    and the length of str, inclusive.

    @return
    the index of the next token, or -1 if no more tokens follow.
 */
sal_Int32 SAL_CALL rtl_uString_getToken( rtl_uString ** newStr , rtl_uString * str, sal_Int32 token, sal_Unicode cTok, sal_Int32 index ) SAL_THROW_EXTERN_C();

/* ======================================================================= */

/* constAsciiStr must be a "..." or char const aFoo[] = "..." */
#define RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) constAsciiStr, sizeof( constAsciiStr )-1, RTL_TEXTENCODING_ASCII_US

/* ======================================================================= */

/* predefined constants for String-Conversion */
#define OSTRING_TO_OUSTRING_CVTFLAGS    (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |\
                                         RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |\
                                         RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT)

/* ----------------------------------------------------------------------- */

/** Create a new Unicode string by converting a byte string, using a specific
    text encoding.

    The lengths of the byte string and the Unicode string may differ (e.g.,
    for double-byte encodings, UTF-7, UTF-8).

    If the length of the byte string is greater than zero, the reference count
    of the new string will be 1.  This function does not handle out-of-memory
    conditions.

    @param newStr
    pointer to the new string.  The pointed-to data must be null or a valid
    string.

    @param str
    a byte character array.  Need not be null-terminated, but must be at
    least as long as the specified len.

    @param len
    the length of the byte character array.

    @param encoding
    the text encoding to use for conversion.

    @param convertFlags
    flags which control the conversion.  Either use
    OSTRING_TO_OUSTRING_CVTFLAGS, or see
    <http://udk.openoffice.org/cpp/man/spec/textconversion.html> for more
    details.
 */
void SAL_CALL rtl_string2UString( rtl_uString ** newStr, const sal_Char * str, sal_Int32 len, rtl_TextEncoding encoding, sal_uInt32 convertFlags ) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif /* _RTL_USTRING_H_ */
