/*************************************************************************
 *
 *  $RCSfile: xehelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:40:06 $
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
#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif


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
        @return  The new string object (shared pointer). */
    static XclExpStringPtr      CreateString(
                                    const XclExpRoot& rRoot,
                                    const ScEditCell& rEditCell,
                                    const ScPatternAttr* pCellAttr,
                                    XclStrFlags nFlags = EXC_STR_DEFAULT,
                                    sal_uInt16 nMaxLen = 0xFFFF );

    /** Creates a new formatted string from a drawing text box.
        @param rTextObj  The text box object.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object (shared pointer). */
    static XclExpStringPtr      CreateString(
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
    &X                      superscript on/off
    &Y                      subscript on/off
    &"fontname,fontstyle"   use font with name 'fontname' and style 'fontstyle'
    &fontheight             set font height in points ('fontheight' is a decimal value)

    Known but unsupported control sequences:
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

