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
#ifndef TOOLS_INETMIME_HXX
#define TOOLS_INETMIME_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include "tools/toolsdllapi.h"
#include <rtl/alloc.h>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <tools/debug.hxx>
#include <tools/errcode.hxx>
#include <tools/string.hxx>

class DateTime;
class INetContentTypeParameterList;
class INetMIMECharsetList_Impl;
class INetMIMEOutputSink;

//============================================================================
class TOOLS_DLLPUBLIC INetMIME
{
public:
    enum { SOFT_LINE_LENGTH_LIMIT = 76,
           HARD_LINE_LENGTH_LIMIT = 998 };

    /** The various types of message header field bodies, with respect to
        encoding and decoding them.

        @descr  At the moment, five different types of header fields suffice
        to describe how to encoded and decode any known message header field
        body, but need for more types may arise in the future as new header
        fields are introduced.

        @descr  The following is an exhaustive list of all the header fields
        currently known to our implementation.  For every header field, it
        includes a 'canonic' (with regard to capitalization) name, a grammar
        rule for the body (using RFC 822 and RFC 2234 conventions), a list of
        relevant sources of information, and the HeaderFieldType value to use
        with that header field.  The list is based on RFC 2076 and draft-
        palme-mailext-headers-02.txt (see also <http://www.dsv.su.se/~jpalme/
        ietf/jp-ietf-home.html#anchor1003783>).

        Approved: address  ;RFC 1036; HEADER_FIELD_ADDRESS
        bcc: #address  ;RFCs 822, 2047; HEADER_FIELD_ADDRESS
        cc: 1#address  ;RFCs 822, 2047; HEADER_FIELD_ADDRESS
        Comments: *text  ;RFCs 822, RFC 2047; HEADER_FIELD_TEXT
        Content-Base: absoluteURI  ;RFC 2110; HEADER_FIELD_TEXT
        Content-Description: *text  ;RFC 2045, RFC 2047; HEADER_FIELD_TEXT
        Content-Disposition: disposition-type *(";" disposition-parm)
            ;RFC 1806; HEADER_FIELD_STRUCTURED
        Content-ID: msg-id  ;RFC 2045, RFC 2047; HEADER_FIELD_MESSAGE_ID
        Content-Location: absoluteURI / relativeURI  ;RFC 2110;
            HEADER_FIELD_TEXT
        Content-Transfer-Encoding: mechanism  ;RFC 2045, RFC 2047;
            HEADER_FIELD_STRUCTURED
        Content-Type: type "/" subtype *(";" parameter)  ;RFC 2045, RFC 2047;
            HEADER_FIELD_STRUCTURED
        Control:  *text ;RFC 1036; HEADER_FIELD_TEXT
        Date: date-time  ;RFC 822, RFC 1123, RFC 2047; HEADER_FIELD_STRUCTURED
        Distribution: 1#atom  ;RFC 1036; HEADER_FIELD_STRUCTURED
        Encrypted: 1#2word  ;RFC 822, RFC 2047; HEADER_FIELD_STRUCTURED
        Expires: date-time  ;RFC 1036; HEADER_FIELD_STRUCTURED
        Followup-To: 1#(atom *("." atom))  ;RFC 1036; HEADER_FIELD_STRUCTURED
        From: mailbox / 1#mailbox  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        In-Reply-To: *(phrase / msg-id)  ;RFC 822, RFC 2047;
            HEADER_FIELD_ADDRESS
        Keywords: #phrase  ;RFC 822, RFC 2047; HEADER_FIELD_PHRASE
        MIME-Version: 1*DIGIT "." 1*DIGIT  ;RFC 2045, RFC 2047;
            HEADER_FIELD_STRUCTURED
        Message-ID: msg-id  ;RFC 822, RFC 2047; HEADER_FIELD_MESSAGE_ID
        Newsgroups: 1#(atom *("." atom))  ;RFC 1036, RFC 2047;
            HEADER_FIELD_STRUCTURED
        Organization: *text  ;RFC 1036; HEADER_FIELD_TEXT
        Received: ["from" domain] ["by" domain] ["via" atom] *("with" atom)
            ["id" msg-id] ["for" addr-spec] ";" date-time  ;RFC 822, RFC 1123,
            RFC 2047; HEADER_FIELD_STRUCTURED
        References: *(phrase / msg-id)  ;RFC 822, RFC 2047;
            HEADER_FIELD_ADDRESS
        Reply-To: 1#address  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Resent-Date: date-time  ;RFC 822, RFC 1123, RFC 2047;
            HEADER_FIELD_STRUCTURED
        Resent-From: mailbox / 1#mailbox  ;RFC 822, RFC 2047;
            HEADER_FIELD_ADDRESS
        Resent-Message-ID: msg-id  ;RFC 822, RFC 2047; HEADER_FIELD_MESSAGE_ID
        Resent-Reply-To: 1#address  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Resent-Sender: mailbox  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Resent-To: 1#address  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Resent-bcc: #address  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Resent-cc: 1#address  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Return-path: route-addr / ("<" ">")  ;RFC 822, RFC 1123, RFC 2047;
            HEADER_FIELD_STRUCTURED
        Return-Receipt-To: address  ;Not Internet standard;
            HEADER_FIELD_ADDRES
        Sender: mailbox  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        Subject: *text  ;RFC 822, RFC 2047; HEADER_FIELD_TEXT
        Summary: *text  ;RFC 1036; HEADER_FIELD_TEXT
        To: 1#address  ;RFC 822, RFC 2047; HEADER_FIELD_ADDRESS
        X-CHAOS-Marked: "YES" / "NO"  ;local; HEADER_FIELD_STRUCTURED
        X-CHAOS-Read: "YES" / "NO"  ;local; HEADER_FIELD_STRUCTURED
        X-CHAOS-Recipients: #*("<" atom word ">")  ;local;
            HEADER_FIELD_STRUCTURED
        X-CHAOS-Size: 1*DIGIT  ;local; HEADER_FIELD_STRUCTURED
        X-Mailer: *text  ;Not Internet standard; HEADER_FIELD_TEXT
        X-Mozilla-Status: 4HEXDIG  ;Mozilla; HEADER_FIELD_STRUCTURED
        X-Newsreader: *text  ;Not Internet standard; HEADER_FIELD_TEXT
        X-Priority: "1" / "2" / "3" / "4" / "5"  ;Not Internet standard;
            HEADER_FIELD_STRUCTURED
        Xref: sub-domain
            1*((atom / string) *("." (atom / string)) ":" msg-number)
            ;RFCs 1036, 2047, local; HEADER_FIELD_STRUCTURED
     */
    enum HeaderFieldType
    {
        HEADER_FIELD_TEXT,
        HEADER_FIELD_STRUCTURED,
        HEADER_FIELD_PHRASE,
        HEADER_FIELD_MESSAGE_ID,
        HEADER_FIELD_ADDRESS
    };

    /** Check for US-ASCII character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII character (0x00--0x7F).
     */
    static inline bool isUSASCII(sal_uInt32 nChar);

    /** Check for ISO 8859-1 character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a ISO 8859-1 character (0x00--0xFF).
     */
    static inline bool isISO88591(sal_uInt32 nChar);

    /** Check for US-ASCII control character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII control character (US-ASCII
        0x00--0x1F or 0x7F).
     */
    static inline bool isControl(sal_uInt32 nChar);

    /** Check for US-ASCII white space character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII white space character (US-ASCII
        0x09 or 0x20).
     */
    static inline bool isWhiteSpace(sal_uInt32 nChar);

    /** Check for US-ASCII visible character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII visible character (US-ASCII
        0x21--0x7E).
     */
    static inline bool isVisible(sal_uInt32 nChar);

    /** Check for US-ASCII digit character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII (decimal) digit character (US-
        ASCII '0'--'9').
     */
    static inline bool isDigit(sal_uInt32 nChar);

    /** Check for US-ASCII canonic hexadecimal digit character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII canonic (i.e., upper case)
        hexadecimal digit character (US-ASCII '0'--'9' or 'A'--'F').
     */
    static inline bool isCanonicHexDigit(sal_uInt32 nChar);

    /** Check for US-ASCII hexadecimal digit character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII hexadecimal digit character (US-
        ASCII '0'--'9', 'A'--'F', 'a'--'f').
     */
    static inline bool isHexDigit(sal_uInt32 nChar);

    /** Check for US-ASCII upper case character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII upper case alphabetic character
        (US-ASCII 'A'--'Z').
     */
    static inline bool isUpperCase(sal_uInt32 nChar);

    /** Check for US-ASCII lower case character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII lower case alphabetic character
        (US-ASCII 'a'--'z').
     */
    static inline bool isLowerCase(sal_uInt32 nChar);

    /** Check for US-ASCII alphabetic character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII alphabetic character (US-ASCII
        'A'--'Z' or 'a'--'z').
     */
    static inline bool isAlpha(sal_uInt32 nChar);

    /** Check for US-ASCII alphanumeric character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII alphanumeric character (US-ASCII
        '0'--'9', 'A'--'Z' or 'a'--'z').
     */
    static inline bool isAlphanumeric(sal_uInt32 nChar);

    /** Check for US-ASCII Base 64 digit character.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is a US-ASCII Base 64 digit character (US-ASCII
        'A'--'Z', 'a'--'z', '0'--'9', '+', or '/').
     */
    static inline bool isBase64Digit(sal_uInt32 nChar);

    /** Check whether some character is valid within an RFC 822 <atom>.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is valid within an RFC 822 <atom> (US-ASCII
        'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '%', '&', ''', '*', '+',
        '-', '/', '=', '?', '^', '_', '`', '{', '|', '}', or '~').
     */
    static bool isAtomChar(sal_uInt32 nChar);

    /** Check whether some character is valid within an RFC 2045 <token>.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is valid within an RFC 2047 <token> (US-ASCII
        'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '%', '&', ''', '*', '+',
        '-', '.', '^', '_', '`', '{', '|', '}', or '~').
     */
    static bool isTokenChar(sal_uInt32 nChar);

    /** Check whether some character is valid within an RFC 2047 <token>.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is valid within an RFC 2047 <token> (US-ASCII
        'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '%', '&', ''', '*', '+',
        '-', '^', '_', '`', '{', '|', '}', or '~').
     */
    static bool isEncodedWordTokenChar(sal_uInt32 nChar);

    /** Check whether some character is valid within an RFC 2060 <atom>.

        @param nChar  Some UCS-4 character.

        @return  True if nChar is valid within an RFC 2060 <atom> (US-ASCII
        'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '&', ''', '+', ',', '-',
        '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '`',
        '|', '}', or '~').
     */
    static bool isIMAPAtomChar(sal_uInt32 nChar);

    /** Translate an US-ASCII character to upper case.

        @param nChar  Some UCS-4 character.

        @return  If nChar is a US-ASCII upper case character (US-ASCII
        'A'--'Z'), return the corresponding US-ASCII lower case character (US-
        ASCII 'a'--'z'); otherwise, return nChar unchanged.
     */
    static inline sal_uInt32 toUpperCase(sal_uInt32 nChar);

    /** Translate an US-ASCII character to lower case.

        @param nChar  Some UCS-4 character.

        @return  If nChar is a US-ASCII lower case character (US-ASCII
        'a'--'z'), return the corresponding US-ASCII upper case character (US-
        ASCII 'A'--'Z'); otherwise, return nChar unchanged.
     */
    static inline sal_uInt32 toLowerCase(sal_uInt32 nChar);

    /** Get the digit weight of a US-ASCII character.

        @param nChar  Some UCS-4 character.

        @return  If nChar is a US-ASCII (decimal) digit character (US-ASCII
        '0'--'9'), return the corresponding weight (0--9); otherwise,
        return -1.
     */
    static inline int getWeight(sal_uInt32 nChar);

    /** Get the hexadecimal digit weight of a US-ASCII character.

        @param nChar  Some UCS-4 character.

        @return  If nChar is a US-ASCII hexadecimal digit character (US-ASCII
        '0'--'9', 'A'--'F', or 'a'--'f'), return the corresponding weight
        (0--15); otherwise, return -1.
     */
    static inline int getHexWeight(sal_uInt32 nChar);

    /** Get the Base 64 digit weight of a US-ASCII character.

        @param nChar  Some UCS-4 character.

        @return  If nChar is a US-ASCII Base 64 digit character (US-ASCII
        'A'--'F', or 'a'--'f', '0'--'9', '+', or '/'), return the
        corresponding weight (0--63); if nChar is the US-ASCII Base 64 padding
        character (US-ASCII '='), return -1; otherwise, return -2.
     */
    static inline int getBase64Weight(sal_uInt32 nChar);

    /** Get a hexadecimal digit encoded as US-ASCII.

        @param nWeight  Must be in the range 0--15, inclusive.

        @return  The canonic (i.e., upper case) hexadecimal digit
        corresponding to nWeight (US-ASCII '0'--'9' or 'A'--'F').
     */
    static sal_uInt32 getHexDigit(int nWeight);

    static inline bool isHighSurrogate(sal_uInt32 nUTF16);

    static inline bool isLowSurrogate(sal_uInt32 nUTF16);

    static inline sal_uInt32 toUTF32(sal_Unicode cHighSurrogate,
                                     sal_Unicode cLowSurrogate);

    /** Check two US-ASCII strings for equality, ignoring case.

        @param pBegin1  Points to the start of the first string, must not be
        null.

        @param pEnd1  Points past the end of the first string, must be >=
        pBegin1.

        @param pString2  Points to the start of the null terminated second
        string, must not be null.

        @return  True if the two strings are equal, ignoring the case of US-
        ASCII alphabetic characters (US-ASCII 'A'--'Z' and 'a'--'z').
     */
    static bool equalIgnoreCase(const sal_Char * pBegin1,
                                const sal_Char * pEnd1,
                                const sal_Char * pString2);

    /** Check two US-ASCII strings for equality, ignoring case.

        @param pBegin1  Points to the start of the first string, must not be
        null.

        @param pEnd1  Points past the end of the first string, must be >=
        pBegin1.

        @param pString2  Points to the start of the null terminated second
        string, must not be null.

        @return  True if the two strings are equal, ignoring the case of US-
        ASCII alphabetic characters (US-ASCII 'A'--'Z' and 'a'--'z').
     */
    static bool equalIgnoreCase(const sal_Unicode * pBegin1,
                                const sal_Unicode * pEnd1,
                                const sal_Char * pString2);

    static inline bool startsWithLineBreak(const sal_Char * pBegin,
                                           const sal_Char * pEnd);

    static inline bool startsWithLineBreak(const sal_Unicode * pBegin,
                                           const sal_Unicode * pEnd);

    static inline bool startsWithLineFolding(const sal_Char * pBegin,
                                             const sal_Char * pEnd);

    static inline bool startsWithLineFolding(const sal_Unicode * pBegin,
                                             const sal_Unicode * pEnd);

    static bool startsWithLinearWhiteSpace(const sal_Char * pBegin,
                                           const sal_Char * pEnd);

    static const sal_Unicode * skipLinearWhiteSpace(const sal_Unicode *
                                                        pBegin,
                                                    const sal_Unicode * pEnd);

    static const sal_Char * skipComment(const sal_Char * pBegin,
                                        const sal_Char * pEnd);

    static const sal_Unicode * skipComment(const sal_Unicode * pBegin,
                                           const sal_Unicode * pEnd);

    static const sal_Char * skipLinearWhiteSpaceComment(const sal_Char *
                                                            pBegin,
                                                        const sal_Char *
                                                            pEnd);

    static const sal_Unicode * skipLinearWhiteSpaceComment(const sal_Unicode *
                                                               pBegin,
                                                           const sal_Unicode *
                                                               pEnd);

    static inline bool needsQuotedStringEscape(sal_uInt32 nChar);

    static const sal_Char * skipQuotedString(const sal_Char * pBegin,
                                             const sal_Char * pEnd);

    static const sal_Unicode * skipQuotedString(const sal_Unicode * pBegin,
                                                const sal_Unicode * pEnd);

    static bool scanUnsigned(const sal_Char *& rBegin, const sal_Char * pEnd,
                             bool bLeadingZeroes, sal_uInt32 & rValue);

    static bool scanUnsigned(const sal_Unicode *& rBegin,
                             const sal_Unicode * pEnd, bool bLeadingZeroes,
                             sal_uInt32 & rValue);

    static const sal_Unicode * scanQuotedBlock(const sal_Unicode * pBegin,
                                               const sal_Unicode * pEnd,
                                               sal_uInt32 nOpening,
                                               sal_uInt32 nClosing,
                                               sal_Size & rLength,
                                               bool & rModify);

    static sal_Char const * scanParameters(sal_Char const * pBegin,
                                           sal_Char const * pEnd,
                                           INetContentTypeParameterList *
                                               pParameters);

    static sal_Unicode const * scanParameters(sal_Unicode const * pBegin,
                                              sal_Unicode const * pEnd,
                                              INetContentTypeParameterList *
                                                  pParameters);

    static inline rtl_TextEncoding translateToMIME(rtl_TextEncoding
                                                       eEncoding);

    static inline rtl_TextEncoding translateFromMIME(rtl_TextEncoding
                                                         eEncoding);

    static const sal_Char * getCharsetName(rtl_TextEncoding eEncoding);

    static rtl_TextEncoding getCharsetEncoding(const sal_Char * pBegin,
                                               const sal_Char * pEnd);

    static inline bool isMIMECharsetEncoding(rtl_TextEncoding eEncoding);

    static INetMIMECharsetList_Impl *
    createPreferredCharsetList(rtl_TextEncoding eEncoding);

    static sal_Unicode * convertToUnicode(const sal_Char * pBegin,
                                          const sal_Char * pEnd,
                                          rtl_TextEncoding eEncoding,
                                          sal_Size & rSize);

    static sal_Char * convertFromUnicode(const sal_Unicode * pBegin,
                                         const sal_Unicode * pEnd,
                                         rtl_TextEncoding eEncoding,
                                         sal_Size & rSize);

    /** Get the number of octets required to encode an UCS-4 character using
        UTF-8 encoding.

        @param nChar  Some UCS-4 character.

        @return  The number of octets required (in the range 1--6, inclusive).
     */
    static inline int getUTF8OctetCount(sal_uInt32 nChar);

    static inline void writeEscapeSequence(INetMIMEOutputSink & rSink,
                                           sal_uInt32 nChar);

    static void writeUTF8(INetMIMEOutputSink & rSink, sal_uInt32 nChar);

    static void writeHeaderFieldBody(INetMIMEOutputSink & rSink,
                                     HeaderFieldType eType,
                                     const rtl::OUString& rBody,
                                     rtl_TextEncoding ePreferredEncoding,
                                     bool bInitialSpace = true);

    static bool translateUTF8Char(const sal_Char *& rBegin,
                                  const sal_Char * pEnd,
                                  rtl_TextEncoding eEncoding,
                                  sal_uInt32 & rCharacter);

    static rtl::OUString decodeHeaderFieldBody(HeaderFieldType eType,
                                           const rtl::OString& rBody);

// #i70651#: Prevent warnings on Mac OS X.
#ifdef MACOSX
#pragma GCC system_header
#endif

    /** Get the UTF-32 character at the head of a UTF-16 encoded string.

        @param rBegin  Points to the start of the UTF-16 encoded string, must
        not be null.  On exit, it points past the first UTF-32 character's
        encoding.

        @param pEnd  Points past the end of the UTF-16 encoded string, must be
        strictly greater than rBegin.

        @return  The UCS-4 character at the head of the UTF-16 encoded string.
        If the string does not start with the UTF-16 encoding of a UCS-32
        character, the first UTF-16 value is returned.
     */
    static inline sal_uInt32 getUTF32Character(const sal_Unicode *& rBegin,
                                               const sal_Unicode * pEnd);

    /** Put the UTF-16 encoding of a UTF-32 character into a buffer.

        @param pBuffer  Points to a buffer, must not be null.

        @param nUTF32  An UTF-32 character, must be in the range 0..0x10FFFF.

        @return  A pointer past the UTF-16 characters put into the buffer
        (i.e., pBuffer + 1 or pBuffer + 2).
     */
    static inline sal_Unicode * putUTF32Character(sal_Unicode * pBuffer,
                                                  sal_uInt32 nUTF32);
};

// static
inline bool INetMIME::isUSASCII(sal_uInt32 nChar)
{
    return nChar <= 0x7F;
}

// static
inline bool INetMIME::isISO88591(sal_uInt32 nChar)
{
    return nChar <= 0xFF;
}

// static
inline bool INetMIME::isControl(sal_uInt32 nChar)
{
    return nChar <= 0x1F || nChar == 0x7F;
}

// static
inline bool INetMIME::isWhiteSpace(sal_uInt32 nChar)
{
    return nChar == '\t' || nChar == ' ';
}

// static
inline bool INetMIME::isVisible(sal_uInt32 nChar)
{
    return nChar >= '!' && nChar <= '~';
}

// static
inline bool INetMIME::isDigit(sal_uInt32 nChar)
{
    return nChar >= '0' && nChar <= '9';
}

// static
inline bool INetMIME::isCanonicHexDigit(sal_uInt32 nChar)
{
    return isDigit(nChar) || (nChar >= 'A' && nChar <= 'F');
}

// static
inline bool INetMIME::isHexDigit(sal_uInt32 nChar)
{
    return isCanonicHexDigit(nChar) || (nChar >= 'a' && nChar <= 'f');
}

// static
inline bool INetMIME::isUpperCase(sal_uInt32 nChar)
{
    return nChar >= 'A' && nChar <= 'Z';
}

// static
inline bool INetMIME::isLowerCase(sal_uInt32 nChar)
{
    return nChar >= 'a' && nChar <= 'z';
}

// static
inline bool INetMIME::isAlpha(sal_uInt32 nChar)
{
    return isUpperCase(nChar) || isLowerCase(nChar);
}

// static
inline bool INetMIME::isAlphanumeric(sal_uInt32 nChar)
{
    return isAlpha(nChar) || isDigit(nChar);
}

// static
inline bool INetMIME::isBase64Digit(sal_uInt32 nChar)
{
    return isUpperCase(nChar) || isLowerCase(nChar) || isDigit(nChar)
           || nChar == '+' || nChar == '/';
}

// static
inline sal_uInt32 INetMIME::toUpperCase(sal_uInt32 nChar)
{
    return isLowerCase(nChar) ? nChar - ('a' - 'A') : nChar;
}

// static
inline sal_uInt32 INetMIME::toLowerCase(sal_uInt32 nChar)
{
    return isUpperCase(nChar) ? nChar + ('a' - 'A') : nChar;
}

// static
inline int INetMIME::getWeight(sal_uInt32 nChar)
{
    return isDigit(nChar) ? int(nChar - '0') : -1;
}

// static
inline int INetMIME::getHexWeight(sal_uInt32 nChar)
{
    return isDigit(nChar) ? int(nChar - '0') :
           nChar >= 'A' && nChar <= 'F' ? int(nChar - 'A' + 10) :
           nChar >= 'a' && nChar <= 'f' ? int(nChar - 'a' + 10) : -1;
}

// static
inline int INetMIME::getBase64Weight(sal_uInt32 nChar)
{
    return isUpperCase(nChar) ? int(nChar - 'A') :
           isLowerCase(nChar) ? int(nChar - 'a' + 26) :
           isDigit(nChar) ? int(nChar - '0' + 52) :
           nChar == '+' ? 62 :
           nChar == '/' ? 63 :
           nChar == '=' ? -1 : -2;
}

// static
inline bool INetMIME::isHighSurrogate(sal_uInt32 nUTF16)
{
    return nUTF16 >= 0xD800 && nUTF16 <= 0xDBFF;
}

// static
inline bool INetMIME::isLowSurrogate(sal_uInt32 nUTF16)
{
    return nUTF16 >= 0xDC00 && nUTF16 <= 0xDFFF;
}

// static
inline sal_uInt32 INetMIME::toUTF32(sal_Unicode cHighSurrogate,
                                    sal_Unicode cLowSurrogate)
{
    DBG_ASSERT(isHighSurrogate(cHighSurrogate)
               && isLowSurrogate(cLowSurrogate),
               "INetMIME::toUTF32(): Bad chars");
    return ((sal_uInt32(cHighSurrogate) & 0x3FF) << 10)
               | (sal_uInt32(cLowSurrogate) & 0x3FF);
}

// static
inline bool INetMIME::startsWithLineBreak(const sal_Char * pBegin,
                                          const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::startsWithLineBreak(): Bad sequence");

    return pEnd - pBegin >= 2 && pBegin[0] == 0x0D && pBegin[1] == 0x0A;
        // CR, LF
}

// static
inline bool INetMIME::startsWithLineBreak(const sal_Unicode * pBegin,
                                              const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::startsWithLineBreak(): Bad sequence");

    return pEnd - pBegin >= 2 && pBegin[0] == 0x0D && pBegin[1] == 0x0A;
        // CR, LF
}

// static
inline bool INetMIME::startsWithLineFolding(const sal_Char * pBegin,
                                            const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::startsWithLineFolding(): Bad sequence");

    return pEnd - pBegin >= 3 && pBegin[0] == 0x0D && pBegin[1] == 0x0A
           && isWhiteSpace(pBegin[2]); // CR, LF
}

// static
inline bool INetMIME::startsWithLineFolding(const sal_Unicode * pBegin,
                                            const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::startsWithLineFolding(): Bad sequence");

    return pEnd - pBegin >= 3 && pBegin[0] == 0x0D && pBegin[1] == 0x0A
           && isWhiteSpace(pBegin[2]); // CR, LF
}

// static
inline bool INetMIME::startsWithLinearWhiteSpace(const sal_Char * pBegin,
                                                 const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::startsWithLinearWhiteSpace(): Bad sequence");

    return pBegin != pEnd
           && (isWhiteSpace(*pBegin) || startsWithLineFolding(pBegin, pEnd));
}

// static
inline bool INetMIME::needsQuotedStringEscape(sal_uInt32 nChar)
{
    return nChar == '"' || nChar == '\\';
}

// static
inline rtl_TextEncoding INetMIME::translateToMIME(rtl_TextEncoding eEncoding)
{
#if defined WNT
    return eEncoding == RTL_TEXTENCODING_MS_1252 ?
               RTL_TEXTENCODING_ISO_8859_1 : eEncoding;
#else // WNT
    return eEncoding;
#endif // WNT
}

// static
inline rtl_TextEncoding INetMIME::translateFromMIME(rtl_TextEncoding
                                                        eEncoding)
{
#if defined WNT
    return eEncoding == RTL_TEXTENCODING_ISO_8859_1 ?
               RTL_TEXTENCODING_MS_1252 : eEncoding;
#else
    return eEncoding;
#endif
}

// static
inline bool INetMIME::isMIMECharsetEncoding(rtl_TextEncoding eEncoding)
{
    return ( rtl_isOctetTextEncoding(eEncoding) == sal_True );
}

// static
inline int INetMIME::getUTF8OctetCount(sal_uInt32 nChar)
{
    DBG_ASSERT(nChar < 0x80000000, "INetMIME::getUTF8OctetCount(): Bad char");

    return nChar < 0x80 ? 1 :
           nChar < 0x800 ? 2 :
           nChar <= 0x10000 ? 3 :
           nChar <= 0x200000 ? 4 :
           nChar <= 0x4000000 ? 5 : 6;
}

// static
inline sal_uInt32 INetMIME::getUTF32Character(const sal_Unicode *& rBegin,
                                              const sal_Unicode * pEnd)
{
    DBG_ASSERT(rBegin && rBegin < pEnd,
               "INetMIME::getUTF32Character(): Bad sequence");
    if (rBegin + 1 < pEnd && rBegin[0] >= 0xD800 && rBegin[0] <= 0xDBFF
        && rBegin[1] >= 0xDC00 && rBegin[1] <= 0xDFFF)
    {
        sal_uInt32 nUTF32 = sal_uInt32(*rBegin++ & 0x3FF) << 10;
        return (nUTF32 | (*rBegin++ & 0x3FF)) + 0x10000;
    }
    else
        return *rBegin++;
}

// static
inline sal_Unicode * INetMIME::putUTF32Character(sal_Unicode * pBuffer,
                                                 sal_uInt32 nUTF32)
{
    DBG_ASSERT(nUTF32 <= 0x10FFFF, "INetMIME::putUTF32Character(): Bad char");
    if (nUTF32 < 0x10000)
        *pBuffer++ = sal_Unicode(nUTF32);
    else
    {
        nUTF32 -= 0x10000;
        *pBuffer++ = sal_Unicode(0xD800 | (nUTF32 >> 10));
        *pBuffer++ = sal_Unicode(0xDC00 | (nUTF32 & 0x3FF));
    }
    return pBuffer;
}

//============================================================================
class INetMIMEOutputSink
{
public:
    static sal_uInt32 const NO_LINE_LENGTH_LIMIT = SAL_MAX_UINT32;

private:
    sal_uInt32 m_nColumn;
    sal_uInt32 m_nLineLengthLimit;

protected:
    /** Write a sequence of octets.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    virtual void writeSequence(const sal_Char * pBegin,
                               const sal_Char * pEnd) = 0;

    /** Write a null terminated sequence of octets (without the terminating
        null).

        @param pOctets  A null terminated sequence of octets, must not be
        null.

        @return  The length of pOctets (without the terminating null).
     */
    virtual sal_Size writeSequence(const sal_Char * pSequence);

    /** Write a sequence of octets.

        @descr  The supplied sequence of UCS-4 characters is interpreted as a
        sequence of octets.  It is an error if any of the elements of the
        sequence has a numerical value greater than 255.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    virtual void writeSequence(const sal_uInt32 * pBegin,
                               const sal_uInt32 * pEnd);

    /** Write a sequence of octets.

        @descr  The supplied sequence of Unicode characters is interpreted as
        a sequence of octets.  It is an error if any of the elements of the
        sequence has a numerical value greater than 255.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    virtual void writeSequence(const sal_Unicode * pBegin,
                               const sal_Unicode * pEnd);

public:
    INetMIMEOutputSink(sal_uInt32 nTheColumn = 0,
                       sal_uInt32 nTheLineLengthLimit
                           = INetMIME::SOFT_LINE_LENGTH_LIMIT):
        m_nColumn(nTheColumn), m_nLineLengthLimit(nTheLineLengthLimit) {}

    virtual ~INetMIMEOutputSink() {}

    /** Get the current column.

        @return  The current column (starting from zero).
     */
    sal_uInt32 getColumn() const { return m_nColumn; }

    sal_uInt32 getLineLengthLimit() const { return m_nLineLengthLimit; }

    void setLineLengthLimit(sal_uInt32 nTheLineLengthLimit)
    { m_nLineLengthLimit = nTheLineLengthLimit; }

    virtual ErrCode getError() const;

    /** Write a sequence of octets.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    inline void write(const sal_Char * pBegin, const sal_Char * pEnd);

    /** Write a sequence of octets.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param nLength  The length of the sequence.
     */
    void write(const sal_Char * pBegin, sal_Size nLength)
    { write(pBegin, pBegin + nLength); }

    /** Write a sequence of octets.

        @descr  The supplied sequence of UCS-4 characters is interpreted as a
        sequence of octets.  It is an error if any of the elements of the
        sequence has a numerical value greater than 255.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    inline void write(const sal_uInt32 * pBegin, const sal_uInt32 * pEnd);

    /** Write a sequence of octets.

        @descr  The supplied sequence of Unicode characters is interpreted as
        a sequence of octets.  It is an error if any of the elements of the
        sequence has a numerical value greater than 255.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    inline void write(const sal_Unicode * pBegin, const sal_Unicode * pEnd);

    /** Write a sequence of octets.

        @param rOctets  A rtl::OString, interpreted as a sequence of octets.

        @param nBegin  The offset of the first character to write.

        @param nEnd  The offset past the last character to write.
     */
    void write(const rtl::OString& rOctets, xub_StrLen nBegin,
                      xub_StrLen nEnd)
    {
        writeSequence(rOctets.getStr() + nBegin, rOctets.getStr() + nEnd);
        m_nColumn += nEnd - nBegin;
    }

    /** Write a single octet.

        @param nOctet  Some octet.

        @return  This instance.
     */
    inline INetMIMEOutputSink & operator <<(sal_Char nOctet);

    /** Write a null terminated sequence of octets (without the terminating
        null).

        @param pOctets  A null terminated sequence of octets, must not be
        null.

        @return  This instance.
     */
    inline INetMIMEOutputSink & operator <<(const sal_Char * pOctets);

    /** Write a sequence of octets.

        @param rOctets  A rtl::OString, interpreted as a sequence of octets.

        @return  This instance.
     */
    INetMIMEOutputSink & operator <<(const rtl::OString& rOctets)
    {
        writeSequence(rOctets.getStr(), rOctets.getStr() + rOctets.getLength());
        m_nColumn += rOctets.getLength();
        return *this;
    }

    /** Call a manipulator function.

        @param  pManipulator  A manipulator function.

        @return  Whatever the manipulator function returns.
     */
    INetMIMEOutputSink &
    operator <<(INetMIMEOutputSink & (* pManipulator)(INetMIMEOutputSink &))
    { return pManipulator(*this); }

    /** Write a line end (CR LF).
     */
    void writeLineEnd();

    /** A manipulator function that writes a line end (CR LF).

        @param rSink  Some sink.

        @return  The sink rSink.
     */
    static inline INetMIMEOutputSink & endl(INetMIMEOutputSink & rSink);
};

inline void INetMIMEOutputSink::write(const sal_Char * pBegin,
                                      const sal_Char * pEnd)
{
    writeSequence(pBegin, pEnd);
    m_nColumn += pEnd - pBegin;
}

inline void INetMIMEOutputSink::write(const sal_uInt32 * pBegin,
                                      const sal_uInt32 * pEnd)
{
    writeSequence(pBegin, pEnd);
    m_nColumn += pEnd - pBegin;
}

inline void INetMIMEOutputSink::write(const sal_Unicode * pBegin,
                                      const sal_Unicode * pEnd)
{
    writeSequence(pBegin, pEnd);
    m_nColumn += pEnd - pBegin;
}

inline INetMIMEOutputSink & INetMIMEOutputSink::operator <<(sal_Char nOctet)
{
    writeSequence(&nOctet, &nOctet + 1);
    ++m_nColumn;
    return *this;
}

inline INetMIMEOutputSink & INetMIMEOutputSink::operator <<(const sal_Char *
                                                                pOctets)
{
    m_nColumn += writeSequence(pOctets);
    return *this;
}

// static
inline INetMIMEOutputSink & INetMIMEOutputSink::endl(INetMIMEOutputSink &
                                                         rSink)
{
    rSink.writeLineEnd();
    return rSink;
}

// static
inline void INetMIME::writeEscapeSequence(INetMIMEOutputSink & rSink,
                                          sal_uInt32 nChar)
{
    DBG_ASSERT(nChar <= 0xFF, "INetMIME::writeEscapeSequence(): Bad char");
    rSink << '=' << sal_uInt8(getHexDigit(nChar >> 4))
          << sal_uInt8(getHexDigit(nChar & 15));
}

//============================================================================
class INetMIMEStringOutputSink: public INetMIMEOutputSink
{
    rtl::OStringBuffer m_aBuffer;

    using INetMIMEOutputSink::writeSequence;

    virtual void writeSequence(const sal_Char * pBegin,
                               const sal_Char * pEnd);

public:
    inline INetMIMEStringOutputSink(sal_uInt32 nColumn = 0,
                                    sal_uInt32 nLineLengthLimit
                                        = INetMIME::SOFT_LINE_LENGTH_LIMIT):
        INetMIMEOutputSink(nColumn, nLineLengthLimit) {}

    virtual ErrCode getError() const;

    rtl::OString takeBuffer()
    {
        return m_aBuffer.makeStringAndClear();
    }
};

//============================================================================
class INetMIMEEncodedWordOutputSink
{
public:
    enum Context { CONTEXT_TEXT = 1,
                   CONTEXT_COMMENT = 2,
                   CONTEXT_PHRASE = 4 };

    enum Space { SPACE_NO, SPACE_ENCODED, SPACE_ALWAYS };

private:
    enum { BUFFER_SIZE = 256 };

    enum Coding { CODING_NONE, CODING_QUOTED, CODING_ENCODED,
                  CODING_ENCODED_TERMINATED };

    enum EncodedWordState { STATE_INITIAL, STATE_FIRST_EQUALS,
                            STATE_FIRST_QUESTION, STATE_CHARSET,
                            STATE_SECOND_QUESTION, STATE_ENCODING,
                            STATE_THIRD_QUESTION, STATE_ENCODED_TEXT,
                            STATE_FOURTH_QUESTION, STATE_SECOND_EQUALS,
                            STATE_BAD };

    INetMIMEOutputSink & m_rSink;
    Context m_eContext;
    Space m_eInitialSpace;
    sal_uInt32 m_nExtraSpaces;
    INetMIMECharsetList_Impl * m_pEncodingList;
    sal_Unicode * m_pBuffer;
    sal_uInt32 m_nBufferSize;
    sal_Unicode * m_pBufferEnd;
    Coding m_ePrevCoding;
    rtl_TextEncoding m_ePrevMIMEEncoding;
    Coding m_eCoding;
    sal_uInt32 m_nQuotedEscaped;
    EncodedWordState m_eEncodedWordState;

    inline bool needsEncodedWordEscape(sal_uInt32 nChar) const;

    void finish(bool bWriteTrailer);

public:
    inline INetMIMEEncodedWordOutputSink(INetMIMEOutputSink & rTheSink,
                                         Context eTheContext,
                                         Space eTheInitialSpace,
                                         rtl_TextEncoding ePreferredEncoding);

    ~INetMIMEEncodedWordOutputSink();

    INetMIMEEncodedWordOutputSink & operator <<(sal_uInt32 nChar);

    inline void write(const sal_Char * pBegin, const sal_Char * pEnd);

    inline void write(const sal_Unicode * pBegin, const sal_Unicode * pEnd);

    inline bool flush();
};

inline INetMIMEEncodedWordOutputSink::INetMIMEEncodedWordOutputSink(
           INetMIMEOutputSink & rTheSink, Context eTheContext,
           Space eTheInitialSpace, rtl_TextEncoding ePreferredEncoding):
    m_rSink(rTheSink),
    m_eContext(eTheContext),
    m_eInitialSpace(eTheInitialSpace),
    m_nExtraSpaces(0),
    m_pEncodingList(INetMIME::createPreferredCharsetList(ePreferredEncoding)),
    m_ePrevCoding(CODING_NONE),
    m_eCoding(CODING_NONE),
    m_nQuotedEscaped(0),
    m_eEncodedWordState(STATE_INITIAL)
{
    m_nBufferSize = BUFFER_SIZE;
    m_pBuffer = static_cast< sal_Unicode * >(rtl_allocateMemory(
                                                 m_nBufferSize
                                                     * sizeof (sal_Unicode)));
    m_pBufferEnd = m_pBuffer;
}

inline void INetMIMEEncodedWordOutputSink::write(const sal_Char * pBegin,
                                                 const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIMEEncodedWordOutputSink::write(): Bad sequence");

    while (pBegin != pEnd)
        operator <<(*pBegin++);
}

inline void INetMIMEEncodedWordOutputSink::write(const sal_Unicode * pBegin,
                                                 const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIMEEncodedWordOutputSink::write(): Bad sequence");

    while (pBegin != pEnd)
        operator <<(*pBegin++);
}

inline bool INetMIMEEncodedWordOutputSink::flush()
{
    finish(true);
    return m_ePrevCoding != CODING_NONE;
}

//============================================================================
struct INetContentTypeParameter
{
    /** The name of the attribute, in US-ASCII encoding and converted to lower
        case.  If a parameter value is split as described in RFC 2231, there
        will only be one item for the complete parameter, with the attribute
        name lacking any section suffix.
     */
    const rtl::OString m_sAttribute;

    /** The optional character set specification (see RFC 2231), in US-ASCII
        encoding and converted to lower case.
     */
    const rtl::OString m_sCharset;

    /** The optional language specification (see RFC 2231), in US-ASCII
        encoding and converted to lower case.
     */
    const rtl::OString m_sLanguage;

    /** The attribute value.  If the value is a quoted-string, it is
        'unpacked.'  If a character set is specified, and the value can be
        converted to Unicode, this is done.  Also, if no character set is
        specified, it is first tried to convert the value from UTF-8 encoding
        to Unicode, and if that doesn't work (because the value is not in
        UTF-8 encoding), it is converted from ISO-8859-1 encoding to Unicode
        (which will always work).  But if a character set is specified and the
        value cannot be converted from that character set to Unicode, special
        action is taken to produce a value that can possibly be transformed
        back into its original form:  Any 8-bit character from a non-encoded
        part of the original value is directly converted to Unicode
        (effectively handling it as if it was ISO-8859-1 encoded), and any
        8-bit character from an encoded part of the original value is mapped
        to the range U+F800..U+F8FF at the top of the Corporate Use Subarea
        within Unicode's Private Use Area (effectively adding 0xF800 to the
        character's numeric value).
     */
    const rtl::OUString m_sValue;

    /** This is true if the value is successfuly converted to Unicode, and
        false if the value is a special mixture of ISO-LATIN-1 characters and
        characters from Unicode's Private Use Area.
     */
    const bool m_bConverted;

    INetContentTypeParameter(const rtl::OString& rTheAttribute,
        const rtl::OString& rTheCharset, const rtl::OString& rTheLanguage,
        const rtl::OUString& rTheValue, bool bTheConverted)
    : m_sAttribute(rTheAttribute)
    , m_sCharset(rTheCharset)
    , m_sLanguage(rTheLanguage)
    , m_sValue(rTheValue)
    , m_bConverted(bTheConverted)
    {
    }
};

//============================================================================
class TOOLS_DLLPUBLIC INetContentTypeParameterList
{
public:

    void Clear();

    void Insert(INetContentTypeParameter * pParameter, sal_uIntPtr nIndex)
    {
        maEntries.insert(maEntries.begin()+nIndex,pParameter);
    }

    void Append(INetContentTypeParameter *pParameter)
    {
        maEntries.push_back(pParameter);
    }

    inline const INetContentTypeParameter * GetObject(sal_uIntPtr nIndex) const
    {
        return &(maEntries[nIndex]);
    }

    const INetContentTypeParameter * find(const rtl::OString& rAttribute) const;

private:

    boost::ptr_vector<INetContentTypeParameter> maEntries;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
