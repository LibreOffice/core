/*************************************************************************
 *
 *  $RCSfile: ustring.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:15 $
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

/**
 * Returns the length of this string.
 * The length is equal to the number of 16-bit
 * Unicode characters in the string.
 *
 * @param   str     must be a NULL-terminated string.
 * @return  the length of the sequence of characters represented by this
 *          string.
 */
sal_Int32 SAL_CALL rtl_ustr_getLength( const sal_Unicode * str );

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
 * @return  the value <code>0</code> if the argument string is equal to
 *          this string; a value less than <code>0</code> if first string
 *          is lexicographically less than the second string; and a
 *          value greater than <code>0</code> if first string is
 *          lexicographically greater than the second string.
 */
sal_Int32 SAL_CALL rtl_ustr_compare_WithLength( const sal_Unicode * first, sal_Int32 firstLen, const sal_Unicode * second, sal_Int32 secondLen );
sal_Int32 SAL_CALL rtl_ustr_compare( const sal_Unicode * first, const sal_Unicode * second );

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
 * Returns a hashcode for the string.
 *
 * @param   str  a NULL-terminated string.
 * @return  a hash code value for str.
 */
sal_Int32 SAL_CALL rtl_ustr_hashCode_WithLength( const sal_Unicode * str, sal_Int32 len );
sal_Int32 SAL_CALL rtl_ustr_hashCode( const sal_Unicode * str );

/**
 * Returns the index within the string of the first occurrence of the
 * specified character.
 *
 * @param   str  a NULL-terminated string.
 * @param   ch   a character.
 * @return  the index of the first occurrence of the character in the
 *          character sequence represented by the string, or
 *          <code>-1</code> if the character does not occur.
 */
sal_Int32 SAL_CALL rtl_ustr_indexOfChar_WithLength( const sal_Unicode * str, sal_Int32 len, sal_Unicode ch );
sal_Int32 SAL_CALL rtl_ustr_indexOfChar( const sal_Unicode * str, sal_Unicode ch );

/**
 * Returns the index within the string of the last occurrence of the
 * specified character, searching backward starting at the specified index.
 *
 * @param   str     a NULL-terminated string.
 * @param   ch      a character.
 * @return  the index of the last occurrence of the character in the
 *          character sequence represented by the string, or
 *          <code>-1</code> if the character does not occur.
 */
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar_WithLength( const sal_Unicode * str, sal_Int32 len, sal_Unicode ch );
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar( const sal_Unicode * str, sal_Unicode ch );

/**
 * Returns the index within the string of the first occurrence of the
 * specified substring.
 *
 * @param   str     a NULL-terminated string.
 * @param   subStr  a NULL-terminated substring to be searched for.
 * @return  if the string argument occurs as a substring within the
 *          string, then the index of the first character of the first
 *          such substring is returned; if it does not occur as a
 *          substring, <code>-1</code> is returned.
 */
sal_Int32 SAL_CALL rtl_ustr_indexOfStr_WithLength( const sal_Unicode * str, sal_Int32 len, const sal_Unicode * subStr, sal_Int32 subLen );
sal_Int32 SAL_CALL rtl_ustr_indexOfStr( const sal_Unicode * str, const sal_Unicode * subStr );

/**
 * Returns the index within this string of the last occurrence of
 * the specified substring.
 * The returned index indicates the start of the substring, and it
 * must be equal to or less than <code>fromIndex</code>.
 *
 * @param   str     a NULL-terminated string.
 * @param   subStr  a NULL-terminated substring to be searched for.
 * @return  If the string argument occurs one or more times as a substring
 *          within the string, then the index of the first character of
 *          the last such substring is returned. If it does not occur as a
 *          substring <code>-1</code> is returned.
 */
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr_WithLength( const sal_Unicode * str, sal_Int32 len, const sal_Unicode * subStr, sal_Int32 subLen );
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr( const sal_Unicode * ggstr, const sal_Unicode * subStr );

/**
 * Replaces all occurrences of <code>oldChar</code> in the string with
 * <code>newChar</code>.
 * <p>
 * If the character <code>oldChar</code> does not occur in the
 * character sequence represented by this object, then the string is
 *not modified.
 *
 * @param   str         a NULL-terminated string.
 * @param   oldChar     the old character.
 * @param   newChar     the new character.
 */
void SAL_CALL rtl_ustr_replaceChar_WithLength( sal_Unicode * str, sal_Int32 len, sal_Unicode oldChar, sal_Unicode newChar);
void SAL_CALL rtl_ustr_replaceChar( sal_Unicode * str, sal_Unicode oldChar, sal_Unicode newChar);

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
 * Returns the string representation of the <code>sal_Bool</code> argument.
 *
 * @param   str   a newly allocated string with the length <code>RTL_USTR_MAX_VALUEOFBOOLEAN</code>.
 * @param   b   a <code>sal_Bool</code>.
 * @return  if the argument is <code>true</code>, a string equal to
 *          <code>"true"</code> is returned; otherwise, a string equal to
 *          <code>"false"</code> is returned.
 */
#define RTL_USTR_MAX_VALUEOFBOOLEAN 6
sal_Int32 SAL_CALL rtl_ustr_valueOfBoolean( sal_Unicode * str, sal_Bool b );

/**
 * Returns the string representation of the <code>char</code> argument.
 *
 * @param   str   a newly allocated string with the length <code>RTL_USTR_MAX_VALUEOFCHAR</code>.
 * @param   ch   a <code>char</code>.
 * @return  a newly allocated string of length <code>1</code> containing
 *          as its single character the argument <code>ch</code>.
 */
#define RTL_USTR_MAX_VALUEOFCHAR 2
sal_Int32 SAL_CALL rtl_ustr_valueOfChar( sal_Unicode * str, sal_Unicode ch );

/**
 * Returns the string representation of the <code>int</code> argument.
 * <p>
 * The representation is exactly the one returned by the
 * <code>Integer.toString</code> method of one argument.
 *
 * @param   str   a newly allocated string with the length <code>RTL_USTR_MAX_VALUEOFINT32</code>.
 * @param   i   an <code>sal_Int32</code>.
 * @return  a newly allocated string containing a string representation of
 *          the <code>int</code> argument.
 * @see     java.lang.Integer#toString(int, int)
 */
#define RTL_USTR_MIN_RADIX          2
#define RTL_USTR_MAX_RADIX          36
#define RTL_USTR_MAX_VALUEOFINT32   33
sal_Int32 SAL_CALL rtl_ustr_valueOfInt32(sal_Unicode * str, sal_Int32 i, sal_Int16 radix );

/**
 * Returns the string representation of the <code>long</code> argument.
 * <p>
 * The representation is exactly the one returned by the
 * <code>Long.toString</code> method of one argument.
 *
 * @param   str   a newly allocated string with the length <code>RTL_USTR_MAX_VALUEOFINT64</code>.
 * @param   l   a <code>sal_Int64</code>.
 * @return  a newly allocated string containing a string representation of
 *          the <code>long</code> argument.
 * @see     java.lang.Long#toString(long)
 */
#define RTL_USTR_MAX_VALUEOFINT64   65
sal_Int32 SAL_CALL rtl_ustr_valueOfInt64(sal_Unicode * str, sal_Int64 l, sal_Int16 radix );

/**
 * Returns the string representation of the <code>float</code> argument.
 * <p>
 * The representation is exactly the one returned by the
 * <code>Float.toString</code> method of one argument.
 *
 * @param   f   a <code>float</code>.
 * @return  a newly allocated string containing a string representation of
 *          the <code>float</code> argument.
 * @see     java.lang.Float#toString(float)
 */
#define RTL_USTR_MAX_VALUEOFFLOAT   15
sal_Int32 SAL_CALL rtl_ustr_valueOfFloat(sal_Unicode * str, float f);

/**
 * Returns the string representation of the <code>double</code> argument.
 * <p>
 * The representation is exactly the one returned by the
 * <code>Double.toString</code> method of one argument.
 *
 * @param   d   a <code>double</code>.
 * @return  a newly allocated string containing a string representation of
 *          the <code>double</code> argument.
 * @see     java.lang.Double#toString(double)
 */
#define RTL_USTR_MAX_VALUEOFDOUBLE  25
sal_Int32 SAL_CALL rtl_ustr_valueOfDouble(sal_Unicode * str, double d);

/**
 * Returns the int32 value represented <code>str</code> argument.
 * <p>
 *
 * @param   str   a string representing a number
 * @return  the int32 represented by the string
 *          0 if the string represents no number.
 */
sal_Int32 SAL_CALL rtl_ustr_toInt32( sal_Unicode * str, sal_Int16 radix );

/**
 * Returns the int64 value represented <code>str</code> argument.
 * <p>
 *
 * @param   str   a string representing a number
 * @return  the int64 represented by the string
 *          0 if the string represents no number.
 */
sal_Int64 SAL_CALL rtl_ustr_toInt64( sal_Unicode * str, sal_Int16 radix );

 /**
 * Returns the float value represented <code>str</code> argument.
 * <p>
 *
 * @param   str   a string representing a number
 * @return  the float represented by the string
 *          0.0 if the string represents no number.
 */
float SAL_CALL rtl_ustr_toFloat( sal_Unicode * str );

/**
 * Returns the double value represented <code>str</code> argument.
 * <p>
 *
 * @param   str   a string representing a number
 * @return  the double represented by the string
 *          0.0 if the string represents no number.
 */
double SAL_CALL rtl_ustr_toDouble( sal_Unicode * str );

/**
 * A string with this reference count is static und must not deleted.
 * It is also not allowed to modifiy the reference count.
 */
#define RTL_STATIC_STRING_REF   ((sal_Int32)0x80000000)

#ifdef SAL_W32
#   pragma pack(push, 4)
#elif defined(SAL_OS2)
#   pragma pack(1)
#endif

struct _rtl_Locale;

/**
 * The implementation structure of a string.
 */
typedef struct _rtl_uString
{
    sal_Int32       refCount;
    sal_Int32       length;
    sal_Unicode     buffer[1];
} rtl_uString;

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
#endif

/**
 * Increment the reference count of the string.
 */
void SAL_CALL rtl_uString_acquire( rtl_uString * value );

/**
 * Decrement the reference count of the string. If the count goes to zero than the string is
 * deleted.
 */
void SAL_CALL rtl_uString_release( rtl_uString * value );

/**
 * Allocates a new <code>string</code> containing no characters.
 * Use the macro RTL_NEWDEFAULTSTRING() which optimize platform dependend
 * the access to the default string.
 */
void SAL_CALL rtl_uString_new( rtl_uString ** newStr);
#define RTL_USTRING_NEW(newStr) rtl_uString_new(newStr)

/**
 * Allocates a new <code>string</code> containing nLen characters.
 * The values of the characters are '\u0000' defined.
 */
void SAL_CALL rtl_uString_new_WithLength( rtl_uString ** newStr, sal_Int32 nLen );

/**
 * Allocates a new string that contains the same sequence of
 * characters as the string argument.<BR>
 *
 * @param   value   a <code>string</code>.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newFromString( rtl_uString ** newStr, rtl_uString * value);

/**
 * Allocates a new <code>string</code> so that it represents the
 * sequence of characters currently contained in the character array
 * argument.
 *
 * @param  value   the initial value of the string.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newFromStr( rtl_uString ** newStr, const sal_Unicode * value );

/**
 * Allocates a new <code>String</code> that contains characters from
 * the character array argument.
 *
 * @param      value    array that is the source of characters.
 * @param      len      the length of the array.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newFromStr_WithLength( rtl_uString ** newStr, const sal_Unicode * value, sal_Int32 Len);

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
 * Assign rightValue to *str. Release *str and aquire rightValue!
 */
void SAL_CALL rtl_uString_assign( rtl_uString ** str, rtl_uString * rightValue );

/**
 * Returns the length of this string.
 * The length is equal to the number of 16-bit
 * Unicode characters in the string.
 *
 * @return  the length of the sequence of characters represented by the
 *          string.
 */
sal_Int32 SAL_CALL rtl_uString_getLength( rtl_uString * str );

/**
 * Return the pointer to the sal_Unicode array of the <code>string</code>.
 *
 * @return  a null terminated sal_Unicode *.
 */
sal_Unicode * SAL_CALL rtl_uString_getStr( rtl_uString * str );

/**
 * Concatenates the right string to the end of the left string. Left and
 * right must be NULL-terminated strings.
 * <p>
 *
 * @return  a string that represents the concatenation of the strings.
 */
void SAL_CALL rtl_uString_newConcat( rtl_uString ** newStr, rtl_uString * left, rtl_uString * right );

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
 * Returns a new string resulting from replacing n = count characters
 * from position index in this string with <code>newStr</code>.
 * <p>
 *
 * @param   index   the index for beginning.
 * @param   count   the count of charcters that will replaced
 * @param   newStr  the new substring.
 * @return  the new string. The reference count is 1.
 */
void SAL_CALL rtl_uString_newReplaceStrAt( rtl_uString ** newStr,
                                           rtl_uString * str,
                                           sal_Int32 index,
                                           sal_Int32 count,
                                           rtl_uString * newSub);

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


void SAL_CALL rtl_string2UString( rtl_uString** newStr, const sal_Char* pStr, sal_Int32 nLen,
                                  rtl_TextEncoding encoding, sal_uInt32 nCvtFlags );

void SAL_CALL rtl_uString2String( rtl_String** newStr, const sal_Unicode* pWStr, sal_Int32 nWLen,
                                  rtl_TextEncoding encoding, sal_uInt32 nCvtFlags );

/* constAsciiStr must be a "..." or char const aFoo[] = "..." */
#define RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) constAsciiStr, sizeof( constAsciiStr )-1, RTL_TEXTENCODING_ASCII_US
#ifndef RTL_CONSTASCII_STRINGPARAM
#define RTL_CONSTASCII_STRINGPARAM( constAsciiStr ) constAsciiStr, sizeof( constAsciiStr )-1
#endif
#ifndef RTL_CONSTASCII_LENGTH
#define RTL_CONSTASCII_LENGTH( constAsciiStr ) (sizeof( constAsciiStr )-1)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _RTL_USTRING_H_ */


