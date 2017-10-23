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

#ifndef INCLUDED_RTL_URI_H
#define INCLUDED_RTL_URI_H

#include "sal/config.h"

#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

/**  Various predefined URI 'char classes.'

     A 'char class' defines which (ASCII) characters can be written 'as they
     are' in a part of a Uri, and which characters have to be written using
     escape sequences ('%' followed by two hex digits).  Characters outside
     the ASCII range are always written using escape sequences.

     If there are other frequently used char classes, they can be added to
     this enumeration; the function rtl_getUriCharClass() has to be adapted
     then, too.
 */
typedef enum
{
    /** The empty char class.

        All characters are written using escape sequences.
     */
    rtl_UriCharClassNone,

    /** The RFC 2732 @<uric> char class.

        @verbatim
        The 'valid' characters are !$&'()*+,-./:;=?@[]_~ plus digits and
        letters.
        @endverbatim
     */
    rtl_UriCharClassUric,

    /** The RFC 2396 @<uric_no_slash> char class.

        @verbatim
        The 'valid' characters are !$&'()*+,-.:;=?@_~ plus digits and letters.
        @endverbatim
     */
    rtl_UriCharClassUricNoSlash,

    /** The RFC 2396 @<rel_segment> char class.

        @verbatim
        The 'valid' characters are !$&'()*+,-.;=@_~ plus digits and letters.
        @endverbatim
     */
    rtl_UriCharClassRelSegment,

    /** The RFC 2396 @<reg_name> char class.

        @verbatim
        The 'valid' characters are !$&'()*+,-.:;=@_~ plus digits and letters.
        @endverbatim
     */
    rtl_UriCharClassRegName,

    /** The RFC 2396 @<userinfo> char class.

        @verbatim
        The 'valid' characters are !$&'()*+,-.:;=_~ plus digits and letters.
        @endverbatim
     */
    rtl_UriCharClassUserinfo,

    /** The RFC 2396 @<pchar> char class.

        @verbatim
        The 'valid' characters are !$&'()*+,-.:=@_~ plus digits and letters.
        @endverbatim
     */
    rtl_UriCharClassPchar,

    /** The char class for the values of uno URL parameters.

        @verbatim
        The 'valid' characters are !$&'()*+-./:?@_~ plus digits and letters.
        @endverbatim
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

        This mechanism is useful to encode user input as part of a URI (e.g.,
        the user-supplied password in an ftp URL---'%20abcde' is a valid
        password, so do not assume that the '%20' is an escaped space).
     */
    rtl_UriEncodeIgnoreEscapes,

    /** All escape sequences ('%' followed by two hex digits) are kept intact,
        even if they represent characters that need not be escaped or if they
        do not even map to characters in the given charset.

        This mechanism is useful when passing on complete URIs more or less
        unmodified (e.g., within an HTTP proxy): missing escape sequences are
        added, but existing escape sequences are not touched (except that any
        lower case hex digits are replaced by upper case hex digits).
     */
    rtl_UriEncodeKeepEscapes,

    /** All escape sequences ('%' followed by two hex digits) are resolved in
        a first step; only those that represent characters that need to be
        escaped are kept intact.

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

        All escape sequences representing ASCII characters (%00--%7F) are
        kept, all other escape sequences are interpreted as UTF-8 characters
        and translated to Unicode, if possible.
     */
    rtl_UriDecodeToIuri,

    /** The text is decoded.

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

/** Convert a relative URI reference into an absolute URI.

    This function uses the strict parser algorithm described in RFC 3986,
    section 5.2.

    This function signals exceptions by returning false and letting pException
    point to a message explaining the exception.

    @param pBaseUriRef
    An absolute URI that serves as the base URI.  If it has to be inspected
    (i.e., pRelUriRef is not an absolute URI already), and it is not an absolute
    URI (i.e., does not begin with a @<scheme ":"> part), an exception will be
    signaled.

    @param pRelUriRef
    An URI reference that may be either absolute or relative.  If it is
    absolute, it will be returned unmodified.

    @param pResult
    Returns an absolute URI.  Must itself not be null, and must point to either
    null or a valid string.  If an exception is signalled, it is left unchanged.

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

#endif // INCLUDED_RTL_URI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
