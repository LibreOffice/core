/*************************************************************************
 *
 *  $RCSfile: xestring.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:41:07 $
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

// ============================================================================

#ifndef SC_XESTRING_HXX
#define SC_XESTRING_HXX

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif


// ============================================================================

class ScEditCell;
class ScPatternAttr;
class EditTextObject;
class XclExpStream;

/** This class stores an unformatted or formatted string for Excel export.
    @descr  The class supports two completely different types of Excel strings:
    1)  BIFF2-BIFF7 byte strings: The text is encoded as a 8-bit character array.
        The strings cannot contain any character formatting.
    2)  BIFF8 Unicode strings: The text may be stored as UCS-2 character array,
        or compressed to an 8-bit array, if all characters are less than U+0100.
        Unicode strings may contain a formatting array, that specifies the used
        FONT record for different ranges of characters.
    The class fully supports NUL characters in strings. On construction or
    assignment the passed flags specify the behaviour of the string while it is
    written to a stream (the 'Write' functions and 'operator<<'). */
class XclExpString
{
public:
    // constructors -----------------------------------------------------------

    /** Constructs an empty BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

    /** Constructs an unformatted BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const String& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Constructs a formatted BIFF8 Unicode string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const String& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Constructs an unformatted BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const ::rtl::OUString& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Constructs a formatted BIFF8 Unicode string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const ::rtl::OUString& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

    // assign -----------------------------------------------------------------

    /** Assigns an unformatted string, converts this object to a BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const String& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a formatted string, converts this object to a BIFF8 Unicode string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const String& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns an unformatted string, converts this object to a BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const ::rtl::OUString& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a formatted string, converts this object to a BIFF8 Unicode string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const ::rtl::OUString& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a Unicode character, converts this object to a BIFF8 Unicode string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string (for appending). */
    void                        Assign(
                                    sal_Unicode cChar,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

    /** Assigns an unformatted string, converts this object to a BIFF2-BIFF7 byte string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        AssignByte(
                                    const String& rString,
                                    CharSet eCharSet,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a character, converts this object to a BIFF2-BIFF7 byte string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string (for appending). */
    void                        AssignByte(
                                    sal_Unicode cChar,
                                    CharSet eCharSet,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

    // append -----------------------------------------------------------------

    /** Appends a string. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF8 Unicode string. */
    void                        Append( const String& rString );
    /** Appends a string. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF8 Unicode string. */
    void                        Append( const ::rtl::OUString& rString );
    /** Appends a character. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF8 Unicode string. */
    void                        Append( sal_Unicode cChar );

    /** Appends a string. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF2-BIFF7 byte string. */
    void                        AppendByte( const String& rString, CharSet eCharSet );
    /** Appends a character. Uses the string flags used in constructor or last Assign().
        @descr  This object must be a BIFF2-BIFF7 byte string. */
    void                        AppendByte( sal_Unicode cChar, CharSet eCharSet );

    // formatting runs --------------------------------------------------------

    /** Sets new formatting runs for the current text. */
    void                        SetFormats( const XclFormatRunVec& rFormats );
    /** Appends a formatting run. nChar must be greater than last contained character index. */
    void                        AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIx );
    /** Removes formatting runs at the end, if the string contains too much. */
    void                        LimitFormatCount( sal_uInt16 nMaxCount );
    /** returns and removes the Font Index for a char index otherwise EXC_FONT_NOTFOUND from the formatting runs. */
    sal_uInt16                  RemoveFontOfChar(sal_uInt16 nCharIdx);

    // get data ---------------------------------------------------------------

    /** Returns the character count of the string. */
    inline sal_uInt16           Len() const { return mnLen; }
    /** Returns true, if the string is empty. */
    inline bool                 IsEmpty() const { return mnLen == 0; }
    /** Returns true, if the string contains line breaks. */
    inline bool                 IsWrapped() const { return mbWrapped; }
    /** Returns true, if this string is equal to the passed string. */
    bool                        IsEqual( const XclExpString& rCmp ) const;
    /** Returns true, if this string is less than the passed string. */
    bool                        IsLessThan( const XclExpString& rCmp ) const;

    /** Returns true, if the string contains formatting information. */
    inline bool                 IsRich() const { return mbIsBiff8 && !maFormats.empty(); }
    /** Returns the current count of formatting runs for rich strings. */
    sal_uInt16                  GetFormatsCount() const;
    /** Returns the vector with all formatting runs. */
    inline const XclFormatRunVec& GetFormats() const { return maFormats; }

    /** Returns the current string flags field to export. */
    sal_uInt8                   GetFlagField() const;
    /** Returns the byte count the character buffer will take on export. */
    sal_uInt32                  GetBufferSize() const;
    /** Returns the byte count the whole string will take on export. */
    sal_uInt32                  GetSize() const;

    /** Returns a hash value for the string. */
    sal_uInt16                  GetHash() const;

    // streaming --------------------------------------------------------------

    /** Writes the string flags field (1 byte). */
    void                        WriteFlagField( XclExpStream& rStrm ) const;
    /** Writes 8-bit or 16-bit length field and string flags field. */
    void                        WriteHeader( XclExpStream& rStrm ) const;
    /** Writes the raw character buffer. */
    void                        WriteBuffer( XclExpStream& rStrm ) const;
    /** Writes the raw formatting run buffer. */
    void                        WriteFormats( XclExpStream& rStrm ) const;
    /** Writes the complete Unicode string. */
    void                        Write( XclExpStream& rStrm ) const;

    /** Writes the raw character buffer to memory (8-bit or 16-bit little-endian) */
    void                        WriteBuffer( void* pDest ) const;

    // ------------------------------------------------------------------------
private:
    /** Returns true, if the flag field should be written. */
    bool                        IsWriteFlags() const;
    /** Returns true, if the formatting run vector should be written. */
    bool                        IsWriteFormats() const;

    /** Sets the string length but regards the limit given in mnMaxLen. */
    void                        SetStrLen( sal_Int32 nNewLen );
    /** Inserts the passed character array into the internal character buffer.
        @param nBegin  First index in internal buffer to fill.
        @param nLen  Number of characters to insert. */
    void                        CharsToBuffer( const sal_Unicode* pcSource, sal_Int32 nBegin, sal_Int32 nLen );
    /** Inserts the passed character array into the internal character buffer.
        @param nBegin  First index in internal buffer to fill.
        @param nLen  Number of characters to insert. */
    void                        CharsToBuffer( const sal_Char* pcSource, sal_Int32 nBegin, sal_Int32 nLen );

    /** Initializes flags, string length, and resizes character buffer.
        @param nFlags  Modifiers for string export.
        @param nCurrLen  The requested number of characters for the string.
        @param nMaxLen  The maximum length allowed of the resulting string.
        @param bBiff8  true = BIFF8 Unicode string; false = BIFF2-BIFF7 byte string. */
    void                        Init( sal_Int32 nCurrLen, XclStrFlags nFlags, sal_uInt16 nMaxLen, bool bBiff8 );
    /** Creates the character buffer from the given Unicode array.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nFlags  Modifiers for string export.
        @param nCurrLen  The real count of characters contained in the passed buffer.
        @param nMaxLen  The maximum length allowed of the resulting string. */
    void                        Build(
                                    const sal_Unicode* pcSource, sal_Int32 nCurrLen,
                                    XclStrFlags nFlags, sal_uInt16 nMaxLen );
    /** Creates the character buffer from the given character array.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nFlags  Modifiers for string export.
        @param nCurrLen  The real count of characters contained in the passed buffer.
        @param nMaxLen  The maximum length allowed of the resulting string. */
    void                        Build(
                                    const sal_Char* pcSource, sal_Int32 nCurrLen,
                                    XclStrFlags nFlags, sal_uInt16 nMaxLen );

    /** Initializes string length and resizes character buffers for appending operation.
        @param nAddLen  The number of characters to be appended. */
    void                        InitAppend( sal_Int32 nAddLen );
    /** Appends the given Unicode array to the character buffer.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nAddLen  The real count of characters contained in the passed buffer. */
    void                        BuildAppend( const sal_Unicode* pcSource, sal_Int32 nAddLen );
    /** Appends the given character array to the character buffer.
        @param pcSource  The source character buffer. Trailing NUL character is not necessary.
        @param nAddLen  The real count of characters contained in the passed buffer. */
    void                        BuildAppend( const sal_Char* pcSource, sal_Int32 nAddLen );

    /** Initializes write process on stream. */
    void                        PrepareWrite( XclExpStream& rStrm, sal_uInt32 nBytes ) const;

private:
    ScfUInt16Vec                maUniBuffer;    /// The Unicode character buffer.
    ScfUInt8Vec                 maCharBuffer;   /// The byte character buffer.
    XclFormatRunVec             maFormats;      /// All formatting runs.
    sal_uInt16                  mnLen;          /// Character count to export.
    sal_uInt16                  mnMaxLen;       /// Maximum allowed number of characters.
    bool                        mbIsBiff8;      /// true = BIFF8 Unicode string, false = BIFF2-7 bytestring.
    bool                        mbIsUnicode;    /// true, if at least one character is >0xFF.
    bool                        mb8BitLen;      /// true = write 8-bit string length; false = 16-bit.
    bool                        mbSmartFlags;   /// true = omit flags on empty string; false = always write flags.
    bool                        mbWrapped;      /// true = text contains several paragraphs.
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


// ----------------------------------------------------------------------------

typedef ::boost::shared_ptr< XclExpString > XclExpStringPtr;

//! for compatibility with old code
typedef XclExpString XclExpUniString;


// ============================================================================

#endif

