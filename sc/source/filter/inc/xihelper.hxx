/*************************************************************************
 *
 *  $RCSfile: xihelper.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:22:57 $
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

#ifndef SC_XIHELPER_HXX
#define SC_XIHELPER_HXX

#ifndef _MyEDITDATA_HXX
#include <svx/editdata.hxx>
#endif

#ifndef SC_MATRIX_HXX
#include "scmatrix.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

class ScMatrix;


// Byte/Unicode strings =======================================================

/** This class represents an unformatted or formatted string and provides importing from stream. */
class XclImpString
{
private:
    String                      maString;       /// The text data of the string.
    XclFormatRunVec             maFormats;      /// All formatting runs.

public:
    /** Constructs an unformatted string. */
    explicit                    XclImpString( const String& rString );
    /** Constructs a formatted string by reading completely from stream. */
    explicit                    XclImpString( XclImpStream& rStrm, XclStrFlags nFlags = EXC_STR_DEFAULT );

                                ~XclImpString();

    /** Insert a formatting run to the format buffer. */
    void                        AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIx );
    /** Reads and appends the formatting information (run count and runs) from stream. */
    void                        ReadFormats( XclImpStream& rStrm );
    /** Reads and appends nRunCount formatting runs from stream. */
    void                        ReadFormats( XclImpStream& rStrm, sal_uInt16 nRunCount );

    /** Returns the pure text data of the string. */
    inline const String&        GetText() const { return maString; }
    /** Returns true, if the string contains formatting information. */
    inline bool                 IsRich() const { return !maFormats.empty(); }
    /** Returns the formatting run vector. */
    inline const XclFormatRunVec& GetFormats() const { return maFormats; }
};


// String->EditEngine conversion ==============================================

class ScBaseCell;
class EditTextObject;

/** This class provides methods to convert an XclImpString.
    @The string can be converted to an edit engine text object or directly
    to a Calc edit cell. */
class XclImpStringHelper : ScfNoInstance
{
public:
    /** Returns a new edit engine text object.
        @param nXFIndex  Index to XF for first text portion (for escapement). */
    static EditTextObject*      CreateTextObject(
                                    const XclImpRoot& rRoot,
                                    const XclImpString& rString,
                                    sal_uInt32 nXFIndex = 0 );

    /** Creates a new text cell or edit cell for a Calc document.
        @param nXFIndex  Index to XF for first text portion (for escapement). */
    static ScBaseCell*          CreateCell(
                                    const XclImpRoot& rRoot,
                                    const XclImpString& rString,
                                    sal_uInt32 nXFIndex = 0 );
};


// Header/footer conversion ===================================================

class EditEngine;
class EditTextObject;
class SfxItemSet;
class SvxFieldItem;
struct XclFontData;

/** Converts an Excel header/footer string into three edit engine text objects.
    @descr  Header/footer content is divided into three parts: Left, center and
    right portion. All formatting information is encoded in the Excel string
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
    &F                      file name without path (see also &Z&F)
    &Z                      file path without file name (converted to full file name, see also &Z&F)
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
class XclImpHFConverter : protected XclImpRoot, ScfNoCopy
{
public:
    explicit            XclImpHFConverter( const XclImpRoot& rRoot );
                        ~XclImpHFConverter();

    /** Parses the passed string and creates three new edit engine text objects. */
    void                ParseString( const String& rHFString );

    /** Creates a ScPageHFItem and inserts it into the passed item set. */
    void                FillToItemSet( SfxItemSet& rItemSet, sal_uInt16 nWhichId ) const;
    /** Returns the total height of the converted header or footer in twips. */
    sal_Int32           GetTotalHeight() const;

private:    // types
    typedef ::std::auto_ptr< XclFontData > XclFontDataPtr;

    /** Enumerates the supported header/footer portions. */
    enum XclImpHFPortion { EXC_HF_LEFT, EXC_HF_CENTER, EXC_HF_RIGHT, EXC_HF_PORTION_COUNT };

    /** Contains all information about a header/footer portion. */
    struct XclImpHFPortionInfo
    {
        typedef ::boost::shared_ptr< EditTextObject > EditTextObjectRef;
        EditTextObjectRef   mxObj;          /// Edit engine text object.
        ESelection          maSel;          /// Edit engine selection.
        sal_Int32           mnHeight;       /// Height of previous lines in twips.
        sal_uInt16          mnMaxLineHt;    /// Maximum font height for the current text line.
        explicit            XclImpHFPortionInfo();
    };
    typedef ::std::vector< XclImpHFPortionInfo > XclImpHFPortionInfoVec;

private:
    /** Returns the current edit engine text object. */
    inline XclImpHFPortionInfo& GetCurrInfo() { return maInfos[ meCurrObj ]; }
    /** Returns the current edit engine text object. */
    inline XclImpHFPortionInfo::EditTextObjectRef& GetCurrObj() { return GetCurrInfo().mxObj; }
    /** Returns the current selection. */
    inline ESelection&  GetCurrSel() { return GetCurrInfo().maSel; }

    /** Returns the maximum line height of the specified portion. */
    sal_uInt16          GetMaxLineHeight( XclImpHFPortion ePortion ) const;
    /** Returns the current maximum line height. */
    sal_uInt16          GetCurrMaxLineHeight() const;

    /** Updates the maximum line height of the specified portion, using the current font size. */
    void                UpdateMaxLineHeight( XclImpHFPortion ePortion );
    /** Updates the current maximum line height, using the current font size. */
    void                UpdateCurrMaxLineHeight();

    /** Sets the font attributes at the current selection.
        @descr  After that, the start position of the current selection object is
        adjusted to the end of the selection. */
    void                SetAttribs();
    /** Resets font data to application default font. */
    void                ResetFontData();

    /** Inserts maCurrText into edit engine and adjusts the current selection object.
        @descr  The text shall not contain a newline character.
        The text will be cleared after insertion. */
    void                InsertText();
    /** Inserts the passed text field and adjusts the current selection object. */
    void                InsertField( const SvxFieldItem& rFieldItem );
    /** Inserts a line break and adjusts the current selection object. */
    void                InsertLineBreak();

    /** Creates the edit engine text object of current portion from edit engine. */
    void                CreateCurrObject();
    /** Changes current header/footer portion to eNew.
        @descr  Creates text object of current portion and reinitializes edit engine. */
    void                SetNewPortion( XclImpHFPortion eNew );

private:
    EditEngine&         mrEE;               /// The header/footer edit engine.
    XclImpHFPortionInfoVec maInfos;         /// Edit engine text objects for all portions.
    String              maCurrText;         /// Current text to insert into edit engine.
    XclFontDataPtr      mxFontData;         /// Font data of current text.
    XclImpHFPortion     meCurrObj;          /// The current portion.
};


// URL conversion =============================================================

/** This class contains static methods to decode an URL stored in an Excel file.
    @descr  Excel URLs can contain a sheet name, for instance: path\[test.xls]Sheet1
    This sheet name will be extracted automatically. */
class XclImpUrlHelper : ScfNoInstance
{
public:
    /** Decodes an encoded external document URL with optional sheet name.
        @param rUrl  Returns the decoded file name incl. path.
        @param rTabName  Returns the decoded sheet name.
        @param rbSameWb  Returns true, if the URL is a reference to the own workbook.
        @param rEncodedUrl   An encoded URL from Excel. */
    static void                 DecodeUrl(
                                    String& rUrl,
                                    String& rTabName,
                                    bool& rbSameWb,
                                    const XclImpRoot& rRoot,
                                    const String& rEncodedUrl );

    /** Decodes an encoded external document URL without sheet name.
        @param rUrl  Returns the decoded file name incl. path.
        @param rbSameWb  Returns true, if the URL is a reference to the own workbook.
        @param rEncodedUrl   An encoded URL from Excel. */
    static void                 DecodeUrl(
                                    String& rUrl,
                                    bool& rbSameWb,
                                    const XclImpRoot& rRoot,
                                    const String& rEncodedUrl );

    /** Decodes the passed URL to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    static bool                 DecodeLink( String& rApplic, String& rTopic, const String rEncUrl );
};


// Cached Values ==============================================================

class ScTokenArray;

/** This class stores one cached value of a cached value list (used for instance in
    CRN, EXTERNNAME, tArray). */
class XclImpCachedValue : ScfNoCopy
{
protected:
    typedef ::std::auto_ptr< String >               StringPtr;
    typedef ::std::auto_ptr< const ScTokenArray >   ScTokenArrayPtr;

    StringPtr                   mpStr;      /// Cached value is a string.
    double                      mfValue;    /// Cached value is a double.
    ScTokenArrayPtr             mpTokArr;   /// Cached value is a formula or error code or Boolean.
    sal_uInt8                   mnBoolErr;  /// Boolean value or Excel error code.
    sal_uInt8                   mnType;     /// The type of the cached value (EXC_CACHEDVAL_*).

public:
    /** Creates a cached value and reads contents from stream and stores it with its array address. */
    explicit                    XclImpCachedValue( XclImpStream& rStrm );
    virtual                     ~XclImpCachedValue();

    /** Returns the type of the cached value (EXC_CACHEDVAL_*). */
    inline sal_uInt8            GetType() const     { return mnType; }
    /** Returns the cached string value, if this value is a string, else an empty string. */
    inline const String&        GetString() const   { return mpStr.get() ? *mpStr : EMPTY_STRING; }
    /** Returns the cached number, if this value has number type, else 0.0. */
    inline double               GetValue() const    { return mfValue; }
    /** Returns the cached Boolean value, if this value has Boolean type, else false. */
    inline bool                 GetBool() const     { return (mnType == EXC_CACHEDVAL_BOOL) && (mnBoolErr != 0); }
    /** Returns the cached Calc error code, if this value has Error type, else 0. */
    USHORT                      GetError() const;
    /** Returns the token array if this is a Boolean value or error value, else 0. */
    inline const ScTokenArray*  GetBoolErrFmla() const { return mpTokArr.get(); }
};


// ----------------------------------------------------------------------------

/** Contains cached values in a 2-dimensional array. */
class XclImpCachedMatrix
{
public:
    explicit                    XclImpCachedMatrix( XclImpStream& rStrm );
                                ~XclImpCachedMatrix();

    /** Creates a new ScMatrix object and fills it with the contained valöues. */
    ScMatrixRef                 CreateScMatrix() const;

private:
    typedef ScfDelList< XclImpCachedValue > XclImpValueList;

    XclImpValueList             maValueList;    /// List of cached cell values.
    SCSIZE                      mnScCols;       /// Number of cached columns.
    SCSIZE                      mnScRows;       /// Number of cached rows.

};

// ============================================================================

#endif

