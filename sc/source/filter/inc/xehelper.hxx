/*************************************************************************
 *
 *  $RCSfile: xehelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:37:50 $
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

#ifndef SC_XEHELPER_HXX
#define SC_XEHELPER_HXX

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif

// Byte/Unicode Strings =======================================================

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
    /** Removes rormatting runs at the end, if the string contains too much. */
    void                        LimitFormatCount( sal_uInt16 nMaxCount );

    // get data ---------------------------------------------------------------

    /** Returns the character count of the string. */
    inline sal_uInt16           Len() const { return mnLen; }
    /** Returns true, if the string is empty. */
    inline bool                 IsEmpty() const { return mnLen == 0; }
    /** Returns true, if the string contains line breaks. */
    inline bool                 IsWrapped() const { return mbWrapped; }

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

    // streaming --------------------------------------------------------------

    /** Writes the string flags field (1 byte). */
    void                        WriteFlagField( XclExpStream& rStrm ) const;
    /** Writes 8-bit or 16-bit length field and string flags field. */
    void                        WriteHeader( XclExpStream& rStrm ) const;
    /** Writes the raw character buffer. */
    void                        WriteBuffer( XclExpStream& rStrm ) const;
    /** Writes the complete Unicode string. */
    void                        Write( XclExpStream& rStrm ) const;

    /** Writes the raw character buffer to memory (8-bit or 16-bit little-endian) */
    void                        WriteBuffer( void* pDest ) const;

private:
    /** Returns true, if the flag field should be written. */
    bool                        IsWriteFlags() const;

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

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclExpString& rString )
{
    rString.Write( rStrm );
    return rStrm;
}

//! for compatibility with old code
typedef XclExpString XclExpUniString;


// EditEngine->String conversion ==============================================

class EditEngine;
class ScEditCell;
class ScPatternAttr;
class SdrTextObj;

/** This class provides methods to create an XclExpString.
    @descr  The string can be created from an edit engine text object or
    directly from a Calc edit cell. */
class XclExpStringHelper : ScfNoInstance
{
public:
    /** Creates a new formatted string from a Calc edit cell.
        @param rEditCell  The Calc edit cell object.
        @param pCellAttr  The set item containing the cell formatting.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object. */
    static XclExpString*        CreateString(
                                    const XclExpRoot& rRoot,
                                    const ScEditCell& rEditCell,
                                    const ScPatternAttr* pCellAttr,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

    /** Creates a new formatted string from a drawing text box.
        @param rTextObj  The text box object.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object. */
    static XclExpString*        CreateString(
                                    const XclExpRoot& rRoot,
                                    const SdrTextObj& rTextObj,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
};


// Header/footer conversion ===================================================

class EditEngine;

/** Converts edit engine text objects to an Excel header/footer string.
    @descr  Header/footer content is divided into three parts: Left, center and
    right portion. All formatting information will be encoded in the Excel string
    using special character seuences. A control sequence starts with the ampersand
    character.

    Supported control sequences:
    &L                      start of left portion
    &C                      start of center portion
    &R                      start of right portion
    &P                      current page number
    &N                      page count
    &D                      current date
    &T                      current time
    &A                      table name
    &F                      file name without path
    &Z                      file path without file name
    &Z&F                    file path and name
    &U                      underlining on/off
    &E                      double underlining on/off
    &S                      strikeout characters on/off
    &"fontname,fontstyle"   use font with name 'fontname' and style 'fontstyle'
    &fontheight             set font height in points ('fontheight' is a decimal value)

    Known but unsupported control sequences:
    &X                      superscript on/off
    &Y                      subscript on/off
    &G                      picture
 */
class XclExpHFConverter : protected XclExpRoot, ScfNoCopy
{
public:
    explicit                    XclExpHFConverter( const XclExpRoot& rRoot );

    /** Generates the header/footer string from the passed edit engine text objects. */
    String                      GenerateString(
                                    const EditTextObject* pLeftObj,
                                    const EditTextObject* pCenterObj,
                                    const EditTextObject* pRightObj );

private:
    /** Converts the text object contents and stores it in the passed string. */
    void                        AppendPortion(
                                    String& rHFString,
                                    const EditTextObject* pTextObj,
                                    sal_Unicode cPortionCode );

private:
    EditEngine&                 mrEE;           /// The header/footer edit engine.
};


// URL conversion =============================================================

/** This class contains static methods to encode a file URL.
    @descr  Excel stores URLs in a format that contains special control characters,
    i.e. for directory separators or volume names. */
class XclExpUrlHelper : ScfNoInstance
{
public:
    /** Encodes and returns the URL passed in rAbsUrl to an Excel like URL.
        @param pTableName  Optional pointer to a table name to be encoded in this URL. */
    static String               EncodeUrl( const XclExpRoot& rRoot, const String& rAbsUrl, const String* pTableName = NULL );
    /** Encodes and returns the passed DDE link to an Excel like DDE link. */
    static String               EncodeDde( const String& rApplic, const String rTopic );

private:
    /** Converts the URL passed in rUrl to a URL in MS-DOS mode.
        @descr  Converts to a relative URL, if enabled in rRootData.
        @return  True = rUrl is a file URL (local or UNC). */
    static bool                 ConvertToDos( String& rUrl, const XclExpRoot& rRoot );
    /** Encodes special parts of the URL, i.e. directory separators and volume names.
        @param pTableName  Pointer to a table name to be encoded in this URL, or NULL. */
    static void                 EncodeUrl_Impl( String& rUrl, const String* pTableName );
};


// Cached Value Lists =========================================================

class XclExpStream;

/** The base class for cached values.
    @descr  Cached values are used to store a list or a 2D array of double,
    string and Boolean values and error codes, for instannce in the records
    CRN and EXTERNNAME or in the token tArray. */
class XclExpCachedValue
{
public:
    virtual                     ~XclExpCachedValue();
    virtual sal_uInt32          GetSize() const = 0;
    virtual void                Save( XclExpStream& rStrm ) const = 0;
};

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclExpCachedValue& rValue )
{
    rValue.Save( rStrm );
    return rStrm;
}


// ----------------------------------------------------------------------------

/** A cached value that stores a double. */
class XclExpCachedDouble : public XclExpCachedValue
{
public:
    explicit inline             XclExpCachedDouble( double fVal ) : mfVal( fVal ) {}
    /** Returns size of this value. */
    virtual sal_uInt32          GetSize() const;
    /** Writes the double value to stream. */
    virtual void                Save( XclExpStream& rStrm ) const;

private:
    double                      mfVal;          /// The double value.
};


// ----------------------------------------------------------------------------

/** A cached value that stores a string. */
class XclExpCachedString : public XclExpCachedValue
{
public:
    explicit                    XclExpCachedString( const String& rStr, XclStrFlags nFlags = EXC_STR_DEFAULT );
    /** Returns size of this value. */
    virtual sal_uInt32          GetSize() const;
    /** Writes the string to stream. */
    virtual void                Save( XclExpStream& rStrm ) const;

private:
    XclExpString                maStr;
};


// ----------------------------------------------------------------------------

class ScDocument;
class ScMatrix;

/** 2-dimensional matrix of cached values (for EXTERNNAME, tArray, ...).
    @descr  The file format is as follows:
    (1 byte) BIFF2-BIFF7: column count(*) / BIFF8: column count - 1
    (2 byte) BIFF2-BIFF7: row count / BIFF8: row count - 1
    (x byte) list of values (doubles, strings)

    (*) In BIFF2-BIFF7 256 columns are stored as 0 columns.

    - Structure of a double value: (XclExpCachedValueDbl)
        (1 byte) 0x01 (identifier)
        (8 byte) double

    - structure of a string value: (XclExpCachedValueStr)
        (1 byte) 0x02 (identifier)
        (x byte) byte string or Unicode string (always write flag field)
*/
class XclExpCachedMatrix
{
public:
    /** Constructs and fills a new matrix.
        @param nCols  The column count of the value matrix.
        @param nRows  The row count of the value matrix.
        @param pMatrix  The Calc value matrix.
        @param nFlags  Flags for writing strings. */
                                XclExpCachedMatrix(
                                    ScDocument& rDoc,
                                    sal_uInt16 nCols, sal_uInt16 nRows,
                                    const ScMatrix* pMatrix,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT );

    /** Returns the byte count of all contained data. */
    sal_uInt32                  GetSize() const;
    /** Writes the complete matrix to stream. */
    void                        Save( XclExpStream& rStrm ) const;

private:
    /** Appends a double value to the value list. */
    void                        Append( double fVal );
    /** Appends a string to the value list. */
    void                        Append( const String& rStr, XclStrFlags nFlags );

private:
    typedef ScfDelList< XclExpCachedValue > XclExpCachedValueList;

    XclExpCachedValueList       maValueList;    /// The list containing the cached values.
    sal_uInt16                  mnCols;         /// Column count of the value matrix.
    sal_uInt16                  mnRows;         /// Row count of the value matrix.
};


// ============================================================================

#endif

