/*************************************************************************
 *
 *  $RCSfile: xehelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-11-21 12:11:10 $
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

class ScDocument;
class ScMatrix;
class XclExpStream;


// Byte/Unicode Strings =======================================================

class ScEditCell;
class ScPatternAttr;
class EditTextObject;

/** This class stores an unformatted or formatted string for Excel export.
    @descr  The string may contain 8-bit characters or 16-bit Unicode characters.
    The class provides exporting the string in many different ways. */
class XclExpString
{
private:
    ScfUInt16Vec                maBuffer;       /// The character buffer.
    XclFormatRunVec             maFormats;      /// All formatting runs.
    sal_uInt16                  mnLen;          /// Character count to export.
    bool                        mbIsUnicode;    /// true, if at least one character is >0xFF.
    bool                        mb8BitLen;      /// true = write 8-bit string length, false = 16-bit.
    bool                        mbSmartFlags;   /// true = omit flags on empty string, false = always write flags.
    bool                        mbWrapped;      /// true = text contains several paragraphs.

public:
    /** Constructs an empty XclExpString. */
    explicit                    XclExpString( XclStrFlags nFlags = EXC_STR_DEFAULT );
    /** Constructs an unformatted string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const String& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Constructs a formatted string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const String& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Constructs an unformatted string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const ::rtl::OUString& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Constructs a formatted string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    explicit                    XclExpString(
                                    const ::rtl::OUString& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

                                ~XclExpString();

    /** Assigns a new unformatted string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const String& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a new formatted string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const String& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a new string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const ::rtl::OUString& rString,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a new formatted string.
        @param rFormats  The formatting runs.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string. */
    void                        Assign(
                                    const ::rtl::OUString& rString,
                                    const XclFormatRunVec& rFormats,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
    /** Assigns a Unicode character.
        @param nFlags  Modifiers for string export. */
    void                        Assign( sal_Unicode cChar, XclStrFlags nFlags = EXC_STR_DEFAULT );

    /** Sets new formatting runs for the current text. */
    void                        SetFormats( const XclFormatRunVec& rFormats );

    /** Returns the character count of the string. */
    inline sal_uInt16           GetLen() const { return mnLen; }
    /** Returns true, if the string contains formatting information. */
    inline bool                 IsRich() const { return !maFormats.empty(); }
    /** Returns true, if the string contains line breaks. */
    inline bool                 IsWrapped() const { return mbWrapped; }

    /** Returns the current string flags field to export. */
    sal_uInt8                   GetFlagField() const;
    /** Returns the byte count the character buffer will take on export. */
    sal_uInt32                  GetBufferByteCount() const;
    /** Returns the byte count the whole string will take on export. */
    sal_uInt32                  GetByteCount() const;

    /** Writes the string flags field (1 byte). */
    void                        WriteFlagField( XclExpStream& rStrm ) const;
    /** Writes 8-bit or 16-bit length field and string flags field. */
    virtual void                WriteHeader( XclExpStream& rStrm ) const;
    /** Writes the raw character buffer. */
    virtual void                WriteBuffer( XclExpStream& rStrm ) const;
    /** Writes the complete Unicode string. */
    virtual void                Write( XclExpStream& rStrm ) const;

    /** Writes the raw character buffer to memory (8-bit or 16-bit little-endian) */
    void                        WriteBuffer( void* pDest ) const;

private:
    /** Initializes all members. */
    void                        Init( XclStrFlags nFlags );

    /** Calculates the string length to be used from real length and given limit. */
    sal_uInt16                  CalcStrLen( const String& rString, sal_uInt16 nMaxLen );
    /** Calculates the string length to be used from real length and given limit. */
    sal_uInt16                  CalcStrLen( const ::rtl::OUString& rString, sal_uInt16 nMaxLen );
    /** Returns a valid maximum length value, according to current string state. */
    inline sal_uInt16           GetMaxLen( sal_uInt16 nMaxLen )
                                    { return (mb8BitLen && (nMaxLen > 255)) ? 255 : nMaxLen; }

    /** Creates the character buffer from the given Unicode array.
        @param pSource  The source character buffer. Trailing NULL character is not necessary.
        @param nFlags  Modifiers for string export.
        @param nCount  The count of characters contained in the buffer. */
    void                        Build( const sal_Unicode* pSource, XclStrFlags nFlags, sal_uInt16 nCount );

    /** Initializes write process on stream. */
    void                        PrepareWrite( XclExpStream& rStrm, sal_uInt32 nBytes ) const;
};

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclExpString& rString )
{
    rString.Write( rStrm );
    return rStrm;
}

//! for compatibility with old code
typedef XclExpString XclExpUniString;


// EditEngine->String conversion ==============================================

class ScEditCell;
class ScPatternAttr;
class EditTextObject;

/** This class provides methods to create an XclExpString.
    @The string can be created from an edit engine text object or directly
    from a Calc edit cell. */
class XclExpStringHelper : ScfNoInstance
{
public:
    /** Creates a new formatted string from an edit engine text object.
        @param rTextObj  The edit engine text object.
        @param pCellAttr  The set item containing the cell formatting.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object. */
    static XclExpString*        CreateString(
                                    const XclExpRoot& rRoot,
                                    const EditTextObject& rTextObj,
                                    const ScPatternAttr* pCellAttr,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );
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
    &F                      file name without path (converted to full file name)
    &A                      table name
    &U                      underlining on/off
    &E                      double underlining on/off
    &S                      strikeout characters on/off
    &"fontname,fontstyle"   use font with name 'fontname' and style 'fontstyle'
    &fontheight             set font height in points ('fontheight' is a decimal value)

    Known but unsupported control sequences:
    &X                      superscript on/off
    &Y                      subscript on/off
    &Z                      file path without file name
    &G                      picture
 */
class XclExpHFConverter : protected XclExpRoot, ScfNoCopy
{
private:
    EditEngine&                 mrEE;           /// The header/footer edit engine.

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
};


// URL conversion =============================================================

/** This class contains static methods to encode a file URL.
    @descr  Excel stores URLs in a format that contains special control characters,
    i.e. for directory separators or volume names. */
class XclExpUrlHelper : ScfNoInstance
{
public:
    /** Encodes and returns the URL passed in rAbsUrl to an Excel like URL. */
    static String               EncodeUrl(
                                    const XclExpRoot& rRoot,
                                    const String& rAbsUrl );

private:
    /** Converts the URL passed in rUrl to a URL in MS-DOS mode.
        @descr  Converts to a relative URL, if enabled in rRootData.
        @return  True = rUrl is a file URL (local or UNC). */
    static bool                 ConvertToDos( String& rUrl, const XclExpRoot& rRoot );
    /** Encodes special parts of the URL, i.e. directory separators and volume names.
        @param bBrackets  True = Adds brackets around the file name. */
    static void                 EncodeUrl_Impl( String& rUrl, bool bBrackets );
};


// Cached Value Lists =========================================================

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
private:
    double                      mfVal;          /// The double value.
public:
    explicit inline             XclExpCachedDouble( double fVal ) : mfVal( fVal ) {}
    /** Returns size of this value. */
    virtual sal_uInt32          GetSize() const;
    /** Writes the double value to stream. */
    virtual void                Save( XclExpStream& rStrm ) const;
};


// ----------------------------------------------------------------------------

/** A cached value that stores a string. */
class XclExpCachedString : public XclExpCachedValue
{
private:
    XclExpString                maStr;

public:
    explicit                    XclExpCachedString( const String& rStr, XclStrFlags nFlags = EXC_STR_DEFAULT );
    /** Returns size of this value. */
    virtual sal_uInt32          GetSize() const;
    /** Writes the string to stream. */
    virtual void                Save( XclExpStream& rStrm ) const;
};


// ----------------------------------------------------------------------------

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
private:
    ScfDelList< XclExpCachedValue > maValueList;    /// The list containing the cached values.
    sal_uInt16                  mnCols;             /// Column count of the value matrix.
    sal_uInt16                  mnRows;             /// Row count of the value matrix.

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
};


// ============================================================================

#endif

