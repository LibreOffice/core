/*************************************************************************
 *
 *  $RCSfile: uri.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2001-05-29 07:33:34 $
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

#ifndef _RTL_URI_H_
#define _RTL_URI_H_

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

/**  Various predefined URI 'char classes.'

     @descr
     A 'char class' defines which (ASCII) characters can be written 'as they
     are' in a part of a Uri, and which characters have to be written using
     escape sequences ('%' followed by two hex digits).  Characters outside
     the ASCII range are always written using escape sequences.

     @descr
     If there are other frequently used char classes, they can be added to
     this enumeration; the function rtl_getUriCharClass() has to be adapted
     then, too.
 */
typedef enum
{
    /** The empty char class.

        @descr
        All characters are written using escape sequences.
     */
    rtl_UriCharClassNone,

    /** The RFC 2732 <uric> char class.

        @descr
        The 'valid' characters are !$&'()*+,-./:;=?@[]_~ plus digits and
        letters.
     */
    rtl_UriCharClassUric,

    /** The RFC 2396 <uric_no_slash> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:;=?@_~ plus digits and letters.
     */
    rtl_UriCharClassUricNoSlash,

    /** The RFC 2396 <rel_segment> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.;=@_~ plus digits and letters.
     */
    rtl_UriCharClassRelSegment,

    /** The RFC 2396 <reg_name> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:;=@_~ plus digits and letters.
     */
    rtl_UriCharClassRegName,

    /** The RFC 2396 <userinfo> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:;=_~ plus digits and letters.
     */
    rtl_UriCharClassUserinfo,

    /** The RFC 2396 <pchar> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:=@_~ plus digits and letters.
     */
    rtl_UriCharClassPchar,

    /** The char class for the values of uno URL parameters.

        @descr
        The 'valid' characters are !$&'()*+-./:?@_~ plus digits and letters.
     */
    rtl_UriCharClassUnoParamValue,

    rtl_UriCharClass_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
}
rtl_UriCharClass;

/** The mechanism describing how escape sequences in the input of
    rtl_uriEncode() are handled.
 */
typedef enum
{
    /** The special meaning of '%' is ignored (i.e., there are by definition
        no escape sequences in the input).

        @descr
        This mechanism is useful to encode user input as part of a URI (e.g.,
        the user-supplied password in an ftp URL---'%20abcde' is a valid
        password, so do not assume that the '%20' is an escaped space).
     */
    rtl_UriEncodeIgnoreEscapes,

    /** All escape sequences ('%' followed by two hex digits) are kept intact,
        even if they represent characters that need not be escaped or if they
        do not even map to characters in the given charset.

        @descr
        This mechanism is useful when passing on complete URIs more or less
        unmodified (e.g., within an HTTP proxy): missing escape sequences are
        added, but existing escape sequences are not touched (except that any
        lower case hex digits are replaced by upper case hex digits).
     */
    rtl_UriEncodeKeepEscapes,

    /** All escape sequences ('%' followed by two hex digits) are resolved in
        a first step; only those that represent characters that need to be
        escaped are kept intact.

        @descr
        This mechanism is useful to properly encode complete URIs entered by
        the user: the URI is brought into a 'canonic form,' but care is taken
        not to damage (valid) escape sequences the (careful) user already
        entered as such.
     */
    rtl_UriEncodeCheckEscapes,

    rtl_UriEncode_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
}
rtl_UriEncodeMechanism;

/** The mechanism describing how rtl_uriDecode() translates (part of) a URI
    into a Unicode string.
 */
typedef enum
{
    /** The text is returned completely unmodified.
     */
    rtl_UriDecodeNone,

    /** The text is returned in the form of an IURI (cf.
        draft-masinter-url-i18n-05.txt).

        @descr
        All escape sequences representing ASCII characters (%00--%7F) are
        kept, all other escape sequences are interpreted as UTF-8 characters
        and translated to Unicode, if possible.
     */
    rtl_UriDecodeToIuri,

    /** The text is decoded.

        @descr
        All escape sequences representing characters from the given charset
        are decoded and translated to Unicode, if possible.
     */
    rtl_UriDecodeWithCharset,

    rtl_UriDecode_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
}
rtl_UriDecodeMechanism;

/** Map a predefined rtl_UriCharClass to a form usable by rtl_uriEncode().

    @descr
    The function rtl_uriEncode() expects an array of 128 booleans, and this
    function maps rtl_UriCharClass enumeration members to such arrays.

    @param eCharClass
    Any valid member of rtl_UriCharClass.

    @return
    An array of 128 booleans, to be used in calls to rtl_uriEncode().
 */
sal_Bool const * rtl_getUriCharClass(rtl_UriCharClass eCharClass)
    SAL_THROW_EXTERN_C();

/** Encode a text as (part of) a URI.

    @param pText
    Any Unicode string.  Must not be null.

    @param pCharClass
    A char class, represented as an array of 128 booleans.  Must not be null.
    (See rtl_getUriCharClass() for a function mapping from rtl_UriCharClass to
    such arrays.)

    @param eMechanism
    The mechanism describing how escape sequences in the input text are
    handled.

    @param eCharset
    When Unicode characters from the input text have to be written using
    escape sequences (because they are either outside the ASCII range or do
    not belong to the given char class), they are first translated into this
    charset, before being encoded using escape sequences.

    Also, if the encode mechanism is rtl_UriEncodeCheckEscapes, all escape
    sequences already present in the input text are interpreted as characters
    of this charset.

    @param pResult
    An encoded representation of the input text.  Must itself not be null, and
    must point to either null or a valid string.
 */
void rtl_uriEncode(rtl_uString * pText,
                   sal_Bool const * pCharClass,
                   rtl_UriEncodeMechanism eMechanism,
                   rtl_TextEncoding eCharset,
                   rtl_uString ** pResult)
    SAL_THROW_EXTERN_C();

/** Decode (a part of) a URI.

    @param pText
    Any Unicode string.  Must not be null.  (If the input is indeed part of a
    valid URI, this string will only contain a subset of the ASCII characters,
    but this function also handles other Unicode characters properly.)

    @param eMechanism
    The mechanism describing how the input text is translated into a Unicode
    string.

    @param eCharset
    When the decode mechanism is rtl_UriDecodeWithCharset, all escape
    sequences in the input text are interpreted as characters of this charset.
    Those characters are translated to Unicode characters in the resulting
    output, if possible.

    When the decode mechanism is rtl_UriDecodeNone or rtl_UriDecodeToIuri,
    this parameter is ignored (and is best specified as
    RTL_TEXTENCODING_UTF8).

    @param pResult
    A decoded representation of the input text.  Must itself not be null, and
    must point to either null or a valid string.
 */
void rtl_uriDecode(rtl_uString * pText,
                   rtl_UriDecodeMechanism eMechanism,
                   rtl_TextEncoding eCharset,
                   rtl_uString ** pResult)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* _RTL_URI_H_ */
