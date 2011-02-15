/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XIHELPER_HXX
#define SC_XIHELPER_HXX

#include <editeng/editdata.hxx>
#include "scmatrix.hxx"
#include "xladdress.hxx"
#include "xiroot.hxx"
#include "xistring.hxx"

// Excel->Calc cell address/range conversion ==================================

/** Provides functions to convert Excel cell addresses to Calc cell addresses. */
class XclImpAddressConverter : public XclAddressConverterBase
{
public:
    explicit            XclImpAddressConverter( const XclImpRoot& rRoot );

    // cell address -----------------------------------------------------------

    /** Checks if the passed Excel cell address is valid.
        @param rXclPos  The Excel cell address to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell address is not valid.
        @return  true = Cell address in rXclPos is valid. */
    bool                CheckAddress( const XclAddress& rXclPos, bool bWarn );

    /** Converts the passed Excel cell address to a Calc cell address.
        @param rScPos  (Out) The converted Calc cell address, if valid.
        @param rXclPos  The Excel cell address to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell address is invalid.
        @return  true = Cell address returned in rScPos is valid. */
    bool                ConvertAddress( ScAddress& rScPos,
                            const XclAddress& rXclPos, SCTAB nScTab, bool bWarn );

    /** Returns a valid cell address by moving it into allowed dimensions.
        @param rXclPos  The Excel cell address to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell address is invalid.
        @return  The converted Calc cell address. */
    ScAddress           CreateValidAddress( const XclAddress& rXclPos,
                            SCTAB nScTab, bool bWarn );

    // cell range -------------------------------------------------------------

    /** Checks if the passed cell range is valid (checks start and end position).
        @param rXclRange  The Excel cell range to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell range is not valid.
        @return  true = Cell range in rXclRange is valid. */
    bool                CheckRange( const XclRange& rXclRange, bool bWarn );

    /** Converts the passed Excel cell range to a Calc cell range.
        @param rScRange  (Out) The converted Calc cell range, if valid.
        @param rXclRange  The Excel cell range to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell range contains invalid cells.
        @return  true = Cell range returned in rScRange is valid (original or cropped). */
    bool                ConvertRange( ScRange& rScRange, const XclRange& rXclRange,
                            SCTAB nScTab1, SCTAB nScTab2, bool bWarn );

//UNUSED2009-05 /** Returns a valid cell range by moving it into allowed dimensions.
//UNUSED2009-05     @descr  The start and/or end position of the range may be modified.
//UNUSED2009-05     @param rXclRange  The Excel cell range to convert.
//UNUSED2009-05     @param bWarn  true = Sets the internal flag that produces a warning box
//UNUSED2009-05         after loading/saving the file, if the cell range contains invalid cells.
//UNUSED2009-05     @return  The converted Calc cell range. */
//UNUSED2009-05 ScRange             CreateValidRange( const XclRange& rXclRange,
//UNUSED2009-05                         SCTAB nScTab1, SCTAB nScTab2, bool bWarn );

    // cell range list --------------------------------------------------------

//UNUSED2009-05 /** Checks if the passed cell range list is valid.
//UNUSED2009-05     @param rXclRanges  The Excel cell range list to check.
//UNUSED2009-05     @param bWarn  true = Sets the internal flag that produces a warning box
//UNUSED2009-05         after loading/saving the file, if the cell range list contains at
//UNUSED2009-05         least one invalid range.
//UNUSED2009-05     @return  true = Cell range list in rScRanges is completly valid. */
//UNUSED2009-05 bool                CheckRangeList( const XclRangeList& rXclRanges, bool bWarn );

    /** Converts the passed Excel cell range list to a Calc cell range list.
        @descr  The start position of the ranges will not be modified. Cell
            ranges that fit partly into valid dimensions are cropped
            accordingly. Cell ranges that do not fit at all, are not inserted
            into the Calc cell range list.
        @param rScRanges  (Out) The converted Calc cell range list.
        @param rXclRanges  The Excel cell range list to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if at least one of the cell ranges
            contains invalid cells. */
    void                ConvertRangeList( ScRangeList& rScRanges,
                            const XclRangeList& rXclRanges, SCTAB nScTab, bool bWarn );
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
    static EditTextObject* CreateTextObject(
                            const XclImpRoot& rRoot,
                            const XclImpString& rString );

//UNUSED2009-05 /** Returns a new edit engine text object for a cell note.
//UNUSED2009-05     @param nXFIndex  Index to XF for first text portion (for escapement). */
//UNUSED2009-05 static EditTextObject* CreateNoteObject(
//UNUSED2009-05                         const XclImpRoot& rRoot,
//UNUSED2009-05                         const XclImpString& rString );

    /** Creates a new text cell or edit cell for a Calc document.
        @param nXFIndex  Index to XF for first text portion (for escapement). */
    static ScBaseCell*  CreateCell(
                            const XclImpRoot& rRoot,
                            const XclImpString& rString,
                            sal_uInt16 nXFIndex = 0 );
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
        typedef ScfRef< EditTextObject > EditTextObjectRef;
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
    static void         DecodeUrl(
                            String& rUrl,
                            String& rTabName,
                            bool& rbSameWb,
                            const XclImpRoot& rRoot,
                            const String& rEncodedUrl );

    /** Decodes an encoded external document URL without sheet name.
        @param rUrl  Returns the decoded file name incl. path.
        @param rbSameWb  Returns true, if the URL is a reference to the own workbook.
        @param rEncodedUrl   An encoded URL from Excel. */
    static void         DecodeUrl(
                            String& rUrl,
                            bool& rbSameWb,
                            const XclImpRoot& rRoot,
                            const String& rEncodedUrl );

    /** Decodes the passed URL to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    static bool         DecodeLink( String& rApplic, String& rTopic, const String rEncUrl );
};

// Cached values ==============================================================

class ScTokenArray;

/** This class stores one cached value of a cached value list (used for instance in
    CRN, EXTERNNAME, tArray). */
class XclImpCachedValue : ScfNoCopy
{
public:
    /** Creates a cached value and reads contents from stream and stores it with its array address. */
    explicit            XclImpCachedValue( XclImpStream& rStrm );
    virtual             ~XclImpCachedValue();

    /** Returns the type of the cached value (EXC_CACHEDVAL_*). */
    inline sal_uInt8    GetType() const     { return mnType; }
    /** Returns the cached string value, if this value is a string, else an empty string. */
    inline const String& GetString() const  { return mxStr.get() ? *mxStr : EMPTY_STRING; }
    /** Returns the cached number, if this value has number type, else 0.0. */
    inline double       GetValue() const    { return mfValue; }
    /** Returns the cached Boolean value, if this value has Boolean type, else false. */
    inline bool         GetBool() const     { return (mnType == EXC_CACHEDVAL_BOOL) && (mnBoolErr != 0); }
    /** Returns the cached Calc error code, if this value has Error type, else 0. */
    inline sal_uInt8    GetXclError() const { return (mnType == EXC_CACHEDVAL_ERROR) ? mnBoolErr : EXC_ERR_NA; }
    /** Returns the cached Calc error code, if this value has Error type, else 0. */
    sal_uInt16              GetScError() const;
    /** Returns the token array if this is a Boolean value or error value, else 0. */
    inline const ScTokenArray* GetBoolErrFmla() const { return mxTokArr.get(); }

protected:
    typedef ::std::auto_ptr< String >               StringPtr;
    typedef ::std::auto_ptr< const ScTokenArray >   ScTokenArrayPtr;

    StringPtr           mxStr;      /// Cached value is a string.
    double              mfValue;    /// Cached value is a double.
    ScTokenArrayPtr     mxTokArr;   /// Cached value is a formula or error code or Boolean.
    sal_uInt8           mnBoolErr;  /// Boolean value or Excel error code.
    sal_uInt8           mnType;     /// The type of the cached value (EXC_CACHEDVAL_*).
};

// ----------------------------------------------------------------------------

/** Contains cached values in a 2-dimensional array. */
class XclImpCachedMatrix
{
public:
    explicit            XclImpCachedMatrix( XclImpStream& rStrm );
                        ~XclImpCachedMatrix();

    /** Creates a new ScMatrix object and fills it with the contained values. */
    ScMatrixRef         CreateScMatrix() const;

private:
    typedef ScfDelList< XclImpCachedValue > XclImpValueList;

    XclImpValueList     maValueList;    /// List of cached cell values.
    SCSIZE              mnScCols;       /// Number of cached columns.
    SCSIZE              mnScRows;       /// Number of cached rows.
};

// ============================================================================

#endif

