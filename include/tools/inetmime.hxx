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
#include <rtl/character.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/tencinfo.h>
#include <tools/debug.hxx>

#include <unordered_map>

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
    static bool equalIgnoreCase(const sal_Unicode * pBegin1,
                                const sal_Unicode * pEnd1,
                                const sal_Char * pString2);

    static bool scanUnsigned(const sal_Unicode *& rBegin,
                             const sal_Unicode * pEnd, bool bLeadingZeroes,
                             sal_uInt32 & rValue);

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
        OUString * pType = nullptr, OUString * pSubType = nullptr,
        INetContentTypeParameterList * pParameters = nullptr);

    static void writeHeaderFieldBody(INetMIMEOutputSink & rSink,
                                     const OUString& rBody,
                                     rtl_TextEncoding ePreferredEncoding);

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
};

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
