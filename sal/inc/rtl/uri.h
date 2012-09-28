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

#ifndef _RTL_URI_H_
#define _RTL_URI_H_

#include "sal/config.h"

#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

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

    /** The RFC 2732 @<uric> char class.

        @descr
        The 'valid' characters are !$&'()*+,-./:;=?@[]_~ plus digits and
        letters.
     */
    rtl_UriCharClassUric,

    /** The RFC 2396 @<uric_no_slash> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:;=?@_~ plus digits and letters.
     */
    rtl_UriCharClassUricNoSlash,

    /** The RFC 2396 @<rel_segment> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.;=@_~ plus digits and letters.
     */
    rtl_UriCharClassRelSegment,

    /** The RFC 2396 @<reg_name> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:;=@_~ plus digits and letters.
     */
    rtl_UriCharClassRegName,

    /** The RFC 2396 @<userinfo> char class.

        @descr
        The 'valid' characters are !$&'()*+,-.:;=_~ plus digits and letters.
     */
    rtl_UriCharClassUserinfo,

    /** The RFC 2396 @<pchar> char class.

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

    /** Like rtl_UriEncodeIgnoreEscapes, but indicating failure when converting
        unmappable characters.

        @since UDK 3.2.0
     */
    rtl_UriEncodeStrict,

    /** Like rtl_UriEncodeKeepEscapes, but indicating failure when converting
        unmappable characters.

        @since UDK 3.2.7
     */
    rtl_UriEncodeStrictKeepEscapes,

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

    /** Like rtl_UriDecodeWithCharset, but indicating failure when converting
        unmappable characters.

        @since UDK 3.2.0
     */
    rtl_UriDecodeStrict,

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
SAL_DLLPUBLIC sal_Bool const * SAL_CALL rtl_getUriCharClass(rtl_UriCharClass eCharClass)
    SAL_THROW_EXTERN_C();

/** Encode a text as (part of) a URI.

    @param pText
    Any Unicode string.  Must not be null.

    @param pCharClass
    A char class, represented as an array of 128 booleans (true means keep the
    corresponding ASCII character unencoded, false means encode it).  Must not
    be null, and the boolean corresponding to the percent sign (0x25) must be
    false.  (See rtl_getUriCharClass() for a function mapping from
    rtl_UriCharClass to such arrays.)

    @param eMechanism
    The mechanism describing how escape sequences in the input text are
    handled.

    @param eCharset
    When Unicode characters from the input text have to be written using
    escape sequences (because they are either outside the ASCII range or do
    not belong to the given char class), they are first translated into this
    charset before being encoded using escape sequences.

    Also, if the encode mechanism is rtl_UriEncodeCheckEscapes, all escape
    sequences already present in the input text are interpreted as characters
    from this charset.

    @param pResult
    Returns an encoded representation of the input text.  Must itself not be
    null, and must point to either null or a valid string.

    If the encode mechanism is rtl_UriEncodeStrict, and pText cannot be
    converted to eCharset because it contains unmappable characters (which
    implies that pText is not empty), then an empty string is returned.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uriEncode(
                            rtl_uString * pText,
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
    sequences in the input text are interpreted as characters from this
    charset.  Those characters are translated to Unicode characters in the
    resulting output, if possible.

    When the decode mechanism is rtl_UriDecodeNone or rtl_UriDecodeToIuri,
    this parameter is ignored (and is best specified as
    RTL_TEXTENCODING_UTF8).

    @param pResult
    Returns a decoded representation of the input text.  Must itself not be
    null, and must point to either null or a valid string.

    If the decode mechanism is rtl_UriDecodeStrict, and pText cannot be
    converted to eCharset because it contains (encodings of) unmappable
    characters (which implies that pText is not empty), then an empty string is
    returned.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uriDecode(
                            rtl_uString * pText,
                            rtl_UriDecodeMechanism eMechanism,
                            rtl_TextEncoding eCharset,
                            rtl_uString ** pResult)
    SAL_THROW_EXTERN_C();

/** Convert a relative URI reference into an absolute one.

    A URI reference is a URI plus an optional @<"#" fragment> part.

    This function uses the algorithm described in RFC 2396, section 5.2, with
    the following clarifications:  (1) Backwards-compatible relative URIs
    starting with a scheme component (see RFC 2396, section 5.2, step 3) are not
    supported.  (2) Segments "." and ".." within the path of the base URI are
    not considered special, RFC 2396 seems a bit unlcear about that point.
    (3) Erroneous excess segments ".." within the path of the relative URI (if
    it is indeed relative) are left intact, as the examples in RFC 2396,
    section C.2, suggest.  (4) If the relative URI is a reference to the
    "current document," the "current document" is taken to be the base URI.

    This function signals exceptions by returning false and letting pException
    point to a message explaining the exception.

    @param pBaseUriRef
    An absolute, hierarchical URI reference that serves as the base URI.  If it
    has to be inspected (i.e., pRelUriRef is not an absolute URI already), and
    if it either is not an absolute URI (i.e., does not begin with a
    @<scheme ":"> part) or has a path that is non-empty but does not start
    with "/", an exception will be signaled.

    @param pRelUriRef
    An URI reference that may be either absolute or relative.  If it is
    absolute, it will be returned unmodified (and it need not be hierarchical
    then).

    @param pResult
    Returns an absolute URI reference.  Must itself not be null, and must point
    to either null or a valid string.  If an exception is signalled, it is left
    unchanged.

    @param pException
    Returns an explanatory message in case an exception is signalled.  Must
    itself not be null, and must point to either null or a valid string.  If no
    exception is signalled, it is left unchanged.

    @return
    True if no exception is signalled, otherwise false.
 */
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_uriConvertRelToAbs(
                                         rtl_uString * pBaseUriRef,
                                         rtl_uString * pRelUriRef,
                                         rtl_uString ** pResult,
                                         rtl_uString ** pException)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* _RTL_URI_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
