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

#pragma once

#include <sal/config.h>

#include <string_view>

#include "xlstring.hxx"
#include "ftools.hxx"

class XclExpStream;
class XclExpXmlStream;

/** This class stores an unformatted or formatted string for Excel export.

    The class supports two completely different types of Excel strings:
    1)  BIFF2-BIFF7 byte strings: The text is encoded as a 8-bit character
        array. The strings cannot contain any character formatting.
    2)  BIFF8 Unicode strings: The text may be stored as UCS-2 character array,
        or compressed to an 8-bit array, if all characters are less than
        U+0100. Unicode strings may contain a formatting array, that specifies
        the used FONT record for different ranges of characters.

    The class provides full support for NUL characters in strings. On
    construction or assignment the passed flags specify the behaviour of the
    string while it is written to a stream (the 'Write' functions and
    'operator<<').
 */
class XclExpString
{
public:
    // constructors -----------------------------------------------------------

    /** Constructs an empty BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit            XclExpString(
                            XclStrFlags nFlags = XclStrFlags::NONE,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Constructs an unformatted BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit            XclExpString(
                            const OUString& rString,
                            XclStrFlags nFlags = XclStrFlags::NONE,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    // assign -----------------------------------------------------------------

    /** Assigns an unformatted string, converts this object to a BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                Assign(
                            const OUString& rString,
                            XclStrFlags nFlags = XclStrFlags::NONE,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Assigns a Unicode character, converts this object to a BIFF8 Unicode string. */
    void                Assign( sal_Unicode cChar );

    /** Assigns an unformatted string, converts this object to a BIFF2-BIFF7 byte string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                AssignByte(
                            std::u16string_view rString,
                            rtl_TextEncoding eTextEnc,
                            XclStrFlags nFlags = XclStrFlags::NONE,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    // append -----------------------------------------------------------------

    /** Appends a string. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF8 Unicode string. */
    void                Append( const OUString& rString );

    /** Appends a string. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF2-BIFF7 byte string. */
    void                AppendByte( std::u16string_view rString, rtl_TextEncoding eTextEnc );
    /** Appends a character. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF2-BIFF7 byte string. */
    void                AppendByte( sal_Unicode cChar, rtl_TextEncoding eTextEnc );

    // formatting runs --------------------------------------------------------

    /** Appends a formatting run. nChar must be greater than last contained character index. */
    void                AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIdx, bool bDropDuplicate = true );
    /** Appends a trailing formatting run with the passed font index. */
    void                AppendTrailingFormat( sal_uInt16 nFontIdx );
    /** Removes formatting runs at the end, if the string contains too much. */
    void                LimitFormatCount( sal_uInt16 nMaxCount );
    /** Returns the font index of the first char in the formatting run, or EXC_FONT_NOTFOUND. */
    sal_uInt16          GetLeadingFont();
    /** The same as above + additionally remove the given font from the formatting run*/
    sal_uInt16          RemoveLeadingFont();

    // get data ---------------------------------------------------------------

    /** Returns the character count of the string. */
    sal_uInt16   Len() const { return mnLen; }
    /** Returns true, if the string is empty. */
    bool         IsEmpty() const { return mnLen == 0; }
    /** Returns true, if the string contains line breaks. */
    bool         IsWrapped() const { return mbWrapped; }
    /** Returns true, if this string is equal to the passed string. */
    bool                IsEqual( const XclExpString& rCmp ) const;
    /** Returns true, if this string is less than the passed string. */
    bool                IsLessThan( const XclExpString& rCmp ) const;

    /** Returns true, if the string contains formatting information. */
    bool         IsRich() const { return !maFormats.empty(); }
    /** Returns the current count of formatting runs for rich strings. */
    sal_uInt16          GetFormatsCount() const;
    /** Returns the vector with all formatting runs. */
    const XclFormatRunVec& GetFormats() const { return maFormats; }

    /** Returns the current string flags field to export. */
    sal_uInt8           GetFlagField() const;
    /** Returns the byte count the header will take on export. */
    sal_uInt16          GetHeaderSize() const;
    /** Returns the byte count the character buffer will take on export. */
    std::size_t         GetBufferSize() const;
    /** Returns the byte count the whole string will take on export. */
    std::size_t         GetSize() const;

    /** Returns the specified character from the (already encoded) string. */
    sal_uInt16          GetChar( sal_uInt16 nCharIdx ) const;
    /** Returns a hash value for the string. */
    sal_uInt16          GetHash() const;

    const ScfUInt16Vec& GetUnicodeBuffer() const { return maUniBuffer; }

    // streaming --------------------------------------------------------------

    /** Writes the string length field (1 byte or 2 bytes). */
    void                WriteLenField( XclExpStream& rStrm ) const;
    /** Writes the string flags field (1 byte). */
    void                WriteFlagField( XclExpStream& rStrm ) const;
    /** Writes 8-bit or 16-bit length field and string flags field. */
    void                WriteHeader( XclExpStream& rStrm ) const;
    /** Writes the raw character buffer. */
    void                WriteBuffer( XclExpStream& rStrm ) const;
    /** Writes the raw formatting run buffer. */
    void                WriteFormats( XclExpStream& rStrm, bool bWriteSize = false ) const;
    /** Writes the complete Unicode string. */
    void                Write( XclExpStream& rStrm ) const;

    /** Writes the string header to memory. */
    void                WriteHeaderToMem( sal_uInt8* pnMem ) const;
    /** Writes the raw character buffer to memory (8-bit or 16-bit little-endian). */
    void                WriteBufferToMem( sal_uInt8* pnMem ) const;
    /** Writes the entire string to memory. */
    void                WriteToMem( sal_uInt8* pnMem ) const;

    void                WriteXml( XclExpXmlStream& rStrm ) const;

private:
    /** Returns true, if the flag field should be written. */
    bool                IsWriteFlags() const;
    /** Returns true, if the formatting run vector should be written. */
    bool                IsWriteFormats() const;

    /** Sets the string length but regards the limit given in mnMaxLen. */
    void                SetStrLen( sal_Int32 nNewLen );
    /** Inserts the passed character array into the internal character buffer.
        @param nBegin  First index in internal buffer to fill.
        @param nLen  Number of characters to insert. */
    void                CharsToBuffer( const sal_Unicode* pcSource, sal_Int32 nBegin, sal_Int32 nLen );
    /** Inserts the passed character array into the internal character buffer.
        @param nBegin  First index in internal buffer to fill.
        @param nLen  Number of characters to insert. */
    void                CharsToBuffer( const char* pcSource, sal_Int32 nBegin, sal_Int32 nLen );

    /** Initializes flags, string length, and resizes character buffer.
        @param nFlags  Modifiers for string export.
        @param nCurrLen  The requested number of characters for the string.
        @param nMaxLen  The maximum length allowed of the resulting string.
        @param bBiff8  true = BIFF8 Unicode string; false = BIFF2-BIFF7 byte string. */
    void                Init( sal_Int32 nCurrLen, XclStrFlags nFlags, sal_uInt16 nMaxLen, bool bBiff8 );
    /** Creates the character buffer from the given Unicode array.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nFlags  Modifiers for string export.
        @param nCurrLen  The real count of characters contained in the passed buffer.
        @param nMaxLen  The maximum length allowed of the resulting string. */
    void                Build(
                            const sal_Unicode* pcSource, sal_Int32 nCurrLen,
                            XclStrFlags nFlags, sal_uInt16 nMaxLen );
    /** Creates the character buffer from the given character array.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nFlags  Modifiers for string export.
        @param nCurrLen  The real count of characters contained in the passed buffer.
        @param nMaxLen  The maximum length allowed of the resulting string. */
    void                Build(
                            const char* pcSource, sal_Int32 nCurrLen,
                            XclStrFlags nFlags, sal_uInt16 nMaxLen );

    /** Initializes string length and resizes character buffers for appending operation.
        @param nAddLen  The number of characters to be appended. */
    void                InitAppend( sal_Int32 nAddLen );
    /** Appends the given Unicode array to the character buffer.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nAddLen  The real count of characters contained in the passed buffer. */
    void                BuildAppend( const sal_Unicode* pcSource, sal_Int32 nAddLen );
    /** Appends the given character array to the character buffer.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nAddLen  The real count of characters contained in the passed buffer. */
    void                BuildAppend( const char* pcSource, sal_Int32 nAddLen );

    /** Initializes write process on stream. */
    void                PrepareWrite( XclExpStream& rStrm, sal_uInt16 nBytes ) const;

private:
    ScfUInt16Vec        maUniBuffer;    /// The Unicode character buffer.
    ScfUInt8Vec         maCharBuffer;   /// The byte character buffer.
    XclFormatRunVec     maFormats;      /// All formatting runs.
    sal_uInt16          mnLen;          /// Character count to export.
    sal_uInt16          mnMaxLen;       /// Maximum allowed number of characters.
    bool                mbIsBiff8;      /// true = BIFF8 Unicode string, false = BIFF2-7 bytestring.
    bool                mbIsUnicode;    /// true, if at least one character is >0xFF.
    bool                mb8BitLen;      /// true = write 8-bit string length; false = 16-bit.
    bool                mbSmartFlags;   /// true = omit flags on empty string; false = always write flags.
    bool                mbSkipFormats;  /// true = skip formats on export; false = write complete formatted string.
    bool                mbWrapped;      /// true = text contains several paragraphs.
    bool                mbSkipHeader;   /// true = skip length and flags when writing string bytes.
};

inline bool operator==( const XclExpString& rLeft, const XclExpString& rRight )
{
    return rLeft.IsEqual( rRight );
}

inline bool operator!=( const XclExpString& rLeft, const XclExpString& rRight )
{
    return !(rLeft == rRight);
}

inline bool operator<( const XclExpString& rLeft, const XclExpString& rRight )
{
    return rLeft.IsLessThan( rRight );
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclExpString& rString )
{
    rString.Write( rStrm );
    return rStrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
