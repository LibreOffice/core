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

#ifndef INCLUDED_RTL_TENCINFO_H
#define INCLUDED_RTL_TENCINFO_H

#include "sal/config.h"

#include "rtl/textenc.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// See rtl_TextEncodingInfo.Flags below for documentation on these values:
#define RTL_TEXTENCODING_INFO_CONTEXT   ((sal_uInt32)0x00000001)
#define RTL_TEXTENCODING_INFO_ASCII     ((sal_uInt32)0x00000002)
#define RTL_TEXTENCODING_INFO_UNICODE   ((sal_uInt32)0x00000004)
#define RTL_TEXTENCODING_INFO_MULTIBYTE ((sal_uInt32)0x00000008)
#define RTL_TEXTENCODING_INFO_R2L       ((sal_uInt32)0x00000010)
#define RTL_TEXTENCODING_INFO_7BIT      ((sal_uInt32)0x00000020)
#define RTL_TEXTENCODING_INFO_SYMBOL    ((sal_uInt32)0x00000040)
#define RTL_TEXTENCODING_INFO_MIME      ((sal_uInt32)0x00000080)

/** Information about a text encoding.
 */
typedef struct _rtl_TextEncodingInfo
{
    /** The size (in bytes) of this structure.  Should be 12.
     */
    sal_uInt32          StructSize;

    /** The minimum number of bytes needed to encode any character in the
        given encoding.

        Can be rather meaningless for encodings that encode global state along
        with the characters (e.g., ISO-2022 encodings).
     */
    sal_uInt8           MinimumCharSize;

    /** The maximum number of bytes needed to encode any character in the
        given encoding.

        Can be rather meaningless for encodings that encode global state along
        with the characters (e.g., ISO-2022 encodings).
     */
    sal_uInt8           MaximumCharSize;

    /** The average number of bytes needed to encode a character in the given
        encoding.
     */
    sal_uInt8           AverageCharSize;

    /** An unused byte, for padding.
     */
    sal_uInt8           Reserved;

    /** Any combination of the RTL_TEXTENCODING_INFO flags.

        RTL_TEXTENCODING_INFO_CONTEXT:  The encoding uses some mechanism (like
        state-changing byte sequences) to switch between different modes (e.g.,
        to encode multiple character repertoires within the same byte ranges).

        Even if an encoding does not have the CONTEXT property, interpretation
        of certain byte values within that encoding can depend on context (e.g.,
        a certain byte value could be either a single-byte character or a
        subsequent byte of a multi-byte character).  Likewise, the single shift
        characters (SS2 and SS3) used by some of the EUC encodings (to denote
        that the following bytes constitute a character from another character
        repertoire) do not imply that encodings making use of these characters
        have the CONTEXT property.  Examples of encodings that do have the
        CONTEXT property are the ISO-2022 encodings and UTF-7.

        RTL_TEXTENCODING_INFO_ASCII:  The encoding is a superset of ASCII.  More
        specifically, any appearance of a byte in the range 0x20--7F denotes the
        corresponding ASCII character (from SPACE to DELETE); in particular,
        such a byte cannot be part of a multi-byte character.  Note that the
        ASCII control codes 0x00--1F are not included here, as they are used for
        special purposes in some encodings.

        If an encoding has this property, it is easy to search for occurrences of
        ASCII characters within strings of this encoding---you do not need to
        keep track whether a byte in the range 0x20--7F really represents an
        ASCII character or rather is part of some multi-byte character.

        The guarantees when mapping between Unicode and a given encoding with
        the ASCII property are as follows:  When mapping from Unicode to the
        given encoding, U+0020--007F map to 0x20--7F (but there can also be
        other Unicode characters mapping into the range 0x20--7F), and when
        mapping from the given encoding to Unicode, 0x20--7F map to U+0020--007F
        (again, there can also be other characters mapping into the range
        U+0020--007F).  In particular, this ensures round-trip conversion for
        the ASCII range.

        In principle, the ASCII property is orthogonal to the CONTEXT property.
        In practice, however, an encoding that has the ASCII property will most
        likely not also have the CONTEXT property.

        RTL_TEXTENCODING_INFO_UNICODE:  The encoding is based on the Unicode
        character repertoire.

        RTL_TEXTENCODING_INFO_MULTIBYTE:  A multi-byte encoding.

        RTL_TEXTENCODING_INFO_R2L:  An encoding used mainly or exclusively for
        languages written from right to left.

        RTL_TEXTENCODING_INFO_7BIT:  A 7-bit instead of an 8-bit encoding.

        RTL_TEXTENCODING_INFO_SYMBOL:  A (generic) encoding for symbol character
        sets.

        RTL_TEXTENCODING_INFO_MIME:  The encoding is registered as a MIME
        charset.
     */
    sal_uInt32          Flags;
} rtl_TextEncodingInfo;

/** Determine whether a text encoding uses single octets as basic units of
    information (and can thus be used with the conversion routines in
    rtl/textcvt.h).

    @param nEncoding
    Any rtl_TextEncoding value.

    @return
    True if the given encoding uses single octets as basic units of
    information, false otherwise.
 */
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_isOctetTextEncoding(rtl_TextEncoding nEncoding);

/** Return information about a text encoding.

    @param eTextEncoding
    Any rtl_TextEncoding value.

    @param pEncInfo
    Returns information about the given encoding.  Must not be null, and the
    StructSize member must be set correctly.

    @return
    True if information about the given encoding is available, false
    otherwise.
 */
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_getTextEncodingInfo(
        rtl_TextEncoding eTextEncoding, rtl_TextEncodingInfo* pEncInfo );

/** Map from a numeric Windows charset to a text encoding.

    @param nWinCharset
    Any numeric Windows charset.

    @return
    The corresponding rtl_TextEncoding value, or RTL_TEXTENCODING_DONTKNOW if
    no mapping is applicable.
    If nWinCharset is 255 (OEM_CHARSET), then return value is RTL_TEXTENCODING_IBM_850,
    regardless of current locale.
 */
SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromWindowsCharset(
        sal_uInt8 nWinCharset );

/** Map from a MIME charset to a text encoding.

    @param pMimeCharset
    Any MIME charset string.  Must not be null.

    @return
    The corresponding rtl_TextEncoding value, or RTL_TEXTENCODING_DONTKNOW if
    no mapping is applicable.
 */
SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMimeCharset(
        const sal_Char* pMimeCharset );

/** Map from a Unix charset to a text encoding.

    @param pUnixCharset
    Any Unix charset string.  Must not be null.

    @return
    The corresponding rtl_TextEncoding value, or RTL_TEXTENCODING_DONTKNOW if
    no mapping is applicable.
 */
SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromUnixCharset(
        const sal_Char* pUnixCharset );

/** Map from a text encoding to the best matching numeric Windows charset.

    @param eTextEncoding
    Any rtl_TextEncoding value.

    @return
    The best matching numeric Windows charset, or 1 if none matches.
 */
SAL_DLLPUBLIC sal_uInt8 SAL_CALL rtl_getBestWindowsCharsetFromTextEncoding(
        rtl_TextEncoding eTextEncoding );

/** Map from a text encoding to a corresponding MIME charset name, if
    available (see <http://www.iana.org/assignments/character-sets>).

    @param nEncoding
    Any rtl_TextEncoding value.

    @return
    The (preferred) MIME charset name corresponding to the given encoding, or
    NULL if none is available.
 */
SAL_DLLPUBLIC char const * SAL_CALL rtl_getMimeCharsetFromTextEncoding(
        rtl_TextEncoding nEncoding );

/** Map from a text encoding to the best matching MIME charset.

    @param eTextEncoding
    Any rtl_TextEncoding value.

    @return
    The best matching MIME charset string, or null if none matches.
 */
SAL_DLLPUBLIC const sal_Char* SAL_CALL rtl_getBestMimeCharsetFromTextEncoding(
        rtl_TextEncoding eTextEncoding );

/** Map from a text encoding to the best matching Unix charset.

    @param eTextEncoding
    Any rtl_TextEncoding value.

    @return
    The best matching Unix charset string, or null if none matches.
 */
SAL_DLLPUBLIC const sal_Char* SAL_CALL rtl_getBestUnixCharsetFromTextEncoding(
        rtl_TextEncoding eTextEncoding  );

/** Map from a Windows code page to a text encoding.

    @param nCodePage
    Any Windows code page number.

    @return
    The corresponding rtl_TextEncoding value (which will be an octet text
    encoding, see rtl_isOctetTextEncoding), or RTL_TEXTENCODING_DONTKNOW if no
    mapping is applicable.
 */
SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL
rtl_getTextEncodingFromWindowsCodePage(sal_uInt32 nCodePage);

/** Map from a text encoding to a Windows code page.

    @param nEncoding
    Any rtl_TextEncoding value.

    @return
    The corresponding Windows code page number, or 0 if no mapping is
    applicable.
 */
SAL_DLLPUBLIC sal_uInt32 SAL_CALL
rtl_getWindowsCodePageFromTextEncoding(rtl_TextEncoding nEncoding);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_TENCINFO_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
