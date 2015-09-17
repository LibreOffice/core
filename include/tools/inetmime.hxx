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
#ifndef INCLUDED_TOOLS_INETMIME_HXX
#define INCLUDED_TOOLS_INETMIME_HXX

#include <tools/toolsdllapi.h>
#include <rtl/alloc.h>
#include <rtl/character.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/tencinfo.h>
#include <tools/debug.hxx>
#include <tools/errcode.hxx>

#include <unordered_map>

class DateTime;
class INetMIMECharsetList_Impl;
class INetMIMEOutputSink;

struct INetContentTypeParameter
{
    /** The name of the attribute, in US-ASCII encoding and converted to lower
        case.  If a parameter value is split as described in RFC 2231, there
        will only be one item for the complete parameter, with the attribute
        name lacking any section suffix.
     */
    OString m_sAttribute;

    /** The optional character set specification (see RFC 2231), in US-ASCII
        encoding and converted to lower case.
     */
    OString m_sCharset;

    /** The optional language specification (see RFC 2231), in US-ASCII
        encoding and converted to lower case.
     */
    OString m_sLanguage;

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
    OUString m_sValue;

    /** This is true if the value is successfully converted to Unicode, and
        false if the value is a special mixture of ISO-LATIN-1 characters and
        characters from Unicode's Private Use Area.
     */
    bool m_bConverted;
};

// the key is the m_sAttribute again; all keys are lower case:
typedef std::unordered_map<OString, INetContentTypeParameter, OStringHash>
    INetContentTypeParameterList;


class TOOLS_DLLPUBLIC INetMIME
{
public:
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

    static inline bool startsWithLineBreak(const sal_Unicode * pBegin,
                                           const sal_Unicode * pEnd);

    static inline bool startsWithLineFolding(const sal_Unicode * pBegin,
                                             const sal_Unicode * pEnd);

    static const sal_Unicode * skipLinearWhiteSpace(const sal_Unicode *
                                                        pBegin,
                                                    const sal_Unicode * pEnd);

    static const sal_Unicode * skipComment(const sal_Unicode * pBegin,
                                           const sal_Unicode * pEnd);

    static const sal_Unicode * skipLinearWhiteSpaceComment(const sal_Unicode *
                                                               pBegin,
                                                           const sal_Unicode *
                                                               pEnd);

    static inline bool needsQuotedStringEscape(sal_uInt32 nChar);

    static const sal_Char * skipQuotedString(const sal_Char * pBegin,
                                             const sal_Char * pEnd);

    static const sal_Unicode * skipQuotedString(const sal_Unicode * pBegin,
                                                const sal_Unicode * pEnd);

    static bool scanUnsigned(const sal_Unicode *& rBegin,
                             const sal_Unicode * pEnd, bool bLeadingZeroes,
                             sal_uInt32 & rValue);

    static const sal_Unicode * scanQuotedBlock(const sal_Unicode * pBegin,
                                               const sal_Unicode * pEnd,
                                               sal_uInt32 nOpening,
                                               sal_uInt32 nClosing,
                                               sal_Size & rLength,
                                               bool & rModify);

    static sal_Unicode const * scanParameters(sal_Unicode const * pBegin,
                                              sal_Unicode const * pEnd,
                                              INetContentTypeParameterList *
                                                  pParameters);

    /** Parse the body of an RFC 2045 Content-Type header field.

        @param pBegin  The range (that must be valid) from non-null pBegin,
        inclusive. to non-null pEnd, exclusive, forms the body of the
        Content-Type header field.  It must be of the form

          token "/" token *(";" token "=" (token / quoted-string))

        with intervening linear white space and comments (cf. RFCs 822, 2045).
        The RFC 2231 extension are supported.  The encoding of rMediaType
        should be US-ASCII, but any Unicode values in the range U+0080..U+FFFF
        are interpretet 'as appropriate.'

        @param pType  If not null, returns the type (the first of the above
        tokens), in US-ASCII encoding and converted to lower case.

        @param pSubType  If not null, returns the sub-type (the second of the
        above tokens), in US-ASCII encoding and converted to lower case.

        @param pParameters  If not null, returns the parameters as a list of
        INetContentTypeParameters (the attributes are in US-ASCII encoding and
        converted to lower case, the values are in Unicode encoding).  If
        null, only the syntax of the parameters is checked, but they are not
        returned.

        @return  Null if the syntax of the field body is incorrect (i.e., does
        not start with type and sub-type tokens).  Otherwise, a pointer past the
        longest valid input prefix.  If null is returned, none of the output
        parameters will be modified.
     */
    static sal_Unicode const * scanContentType(
        sal_Unicode const *pBegin, sal_Unicode const * pEnd,
        OUString * pType = 0, OUString * pSubType = 0,
        INetContentTypeParameterList * pParameters = 0);

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
                                     const OUString& rBody,
                                     rtl_TextEncoding ePreferredEncoding,
                                     bool bInitialSpace = true);

    static bool translateUTF8Char(const sal_Char *& rBegin,
                                  const sal_Char * pEnd,
                                  rtl_TextEncoding eEncoding,
                                  sal_uInt32 & rCharacter);

    static OUString decodeHeaderFieldBody(const OString& rBody);

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
inline int INetMIME::getWeight(sal_uInt32 nChar)
{
    return rtl::isAsciiDigit(nChar) ? int(nChar - '0') : -1;
}

// static
inline int INetMIME::getHexWeight(sal_uInt32 nChar)
{
    return rtl::isAsciiDigit(nChar) ? int(nChar - '0') :
           nChar >= 'A' && nChar <= 'F' ? int(nChar - 'A' + 10) :
           nChar >= 'a' && nChar <= 'f' ? int(nChar - 'a' + 10) : -1;
}

// static
inline int INetMIME::getBase64Weight(sal_uInt32 nChar)
{
    return rtl::isAsciiUpperCase(nChar) ? int(nChar - 'A') :
           rtl::isAsciiLowerCase(nChar) ? int(nChar - 'a' + 26) :
           rtl::isAsciiDigit(nChar) ? int(nChar - '0' + 52) :
           nChar == '+' ? 62 :
           nChar == '/' ? 63 :
           nChar == '=' ? -1 : -2;
}

// static


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
inline bool INetMIME::startsWithLineFolding(const sal_Unicode * pBegin,
                                            const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::startsWithLineFolding(): Bad sequence");

    return pEnd - pBegin >= 3 && pBegin[0] == 0x0D && pBegin[1] == 0x0A
           && isWhiteSpace(pBegin[2]); // CR, LF
}

// static


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
    return rtl_isOctetTextEncoding(eEncoding);
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

class INetMIMEOutputSink
{
private:
    OStringBuffer m_aBuffer;

    /** Write a sequence of octets.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    void writeSequence(const sal_Char * pBegin, const sal_Char * pEnd);

    /** Write a null terminated sequence of octets (without the terminating
        null).

        @param pOctets  A null terminated sequence of octets, must not be
        null.

        @return  The length of pOctets (without the terminating null).
     */
    sal_Size writeSequence(const sal_Char * pSequence);

    /** Write a sequence of octets.

        @descr  The supplied sequence of Unicode characters is interpreted as
        a sequence of octets.  It is an error if any of the elements of the
        sequence has a numerical value greater than 255.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    void writeSequence(const sal_Unicode * pBegin,
                               const sal_Unicode * pEnd);

public:
    /** Write a sequence of octets.

        @descr  The supplied sequence of Unicode characters is interpreted as
        a sequence of octets.  It is an error if any of the elements of the
        sequence has a numerical value greater than 255.

        @param pBegin  Points to the start of the sequence, must not be null.

        @param pEnd  Points past the end of the sequence, must be >= pBegin.
     */
    inline void write(const sal_Unicode * pBegin, const sal_Unicode * pEnd);

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

        @param rOctets  A OString, interpreted as a sequence of octets.

        @return  This instance.
     */
    INetMIMEOutputSink & operator <<(const OString& rOctets)
    {
        writeSequence(rOctets.getStr(), rOctets.getStr() + rOctets.getLength());
        return *this;
    }

    /** Call a manipulator function.

        @param  pManipulator  A manipulator function.

        @return  Whatever the manipulator function returns.
     */
    INetMIMEOutputSink &
    operator <<(INetMIMEOutputSink & (* pManipulator)(INetMIMEOutputSink &))
    { return pManipulator(*this); }

    OString takeBuffer()
    {
        return m_aBuffer.makeStringAndClear();
    }
};


inline void INetMIMEOutputSink::write(const sal_Unicode * pBegin,
                                      const sal_Unicode * pEnd)
{
    writeSequence(pBegin, pEnd);
}

inline INetMIMEOutputSink & INetMIMEOutputSink::operator <<(sal_Char nOctet)
{
    writeSequence(&nOctet, &nOctet + 1);
    return *this;
}

inline INetMIMEOutputSink & INetMIMEOutputSink::operator <<(const sal_Char *
                                                                pOctets)
{
    writeSequence(pOctets);
    return *this;
}

// static
inline void INetMIME::writeEscapeSequence(INetMIMEOutputSink & rSink,
                                          sal_uInt32 nChar)
{
    DBG_ASSERT(nChar <= 0xFF, "INetMIME::writeEscapeSequence(): Bad char");
    rSink << '=' << sal_uInt8(getHexDigit(nChar >> 4))
          << sal_uInt8(getHexDigit(nChar & 15));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
