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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEHELPER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEHELPER_HXX

#include <boost/noncopyable.hpp>
#include <memory>
#include "xladdress.hxx"
#include "xeroot.hxx"
#include "xestring.hxx"

// Export progress bar ========================================================

class ScfProgressBar;

/** The main progress bar for the export filter.

    This class encapsulates creation and initialization of sub progress
    segments. The Activate***Segment() functions activate a specific segement
    of the main progress bar. The implementation of these functions contain the
    calculation of the needed size of the segment. Following calls of the
    Progress() function increase the currently activated sub segment.
 */
class XclExpProgressBar : protected XclExpRoot
{
public:
    explicit            XclExpProgressBar( const XclExpRoot& rRoot );
    virtual             ~XclExpProgressBar();

    /** Initializes all segments and sub progress bars. */
    void                Initialize();

    /** Increases the number of existing ROW records by 1. */
    void                IncRowRecordCount();

    /** Activates the progress segment to create ROW records. */
    void                ActivateCreateRowsSegment();
    /** Activates the progress segment to finalize ROW records. */
    void                ActivateFinalRowsSegment();

    /** Increases the currently activated (sub) progress bar by 1 step. */
    void                Progress();

private:
    typedef std::unique_ptr< ScfProgressBar > ScfProgressBarPtr;

    ScfProgressBarPtr   mxProgress;         /// Progress bar implementation.
    ScfProgressBar*     mpSubProgress;      /// Current sub progress bar.

    ScfProgressBar*     mpSubRowCreate;     /// Sub progress bar for creating table rows.
    ScfInt32Vec         maSubSegRowCreate;  /// Segment ID's for all sheets in sub progress bar.

    ScfProgressBar*     mpSubRowFinal;      /// Sub progress bar for finalizing ROW records.
    sal_Int32           mnSegRowFinal;      /// Progress segment for finalizing ROW records.

    sal_Size            mnRowCount;         /// Number of created ROW records.
};

// Calc->Excel cell address/range conversion ==================================

/** Provides functions to convert Calc cell addresses to Excel cell addresses. */
class XclExpAddressConverter : public XclAddressConverterBase
{
public:
    explicit            XclExpAddressConverter( const XclExpRoot& rRoot );

    // cell address -----------------------------------------------------------

    /** Checks if the passed Calc cell address is valid.
        @param rScPos  The Calc cell address to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell address is not valid.
        @return  true = Cell address in rScPos is valid. */
    bool                CheckAddress( const ScAddress& rScPos, bool bWarn );

    /** Converts the passed Calc cell address to an Excel cell address.
        @param rXclPos  (Out) The converted Excel cell address, if valid.
        @param rScPos  The Calc cell address to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell address is not valid.
        @return  true = Cell address returned in rXclPos is valid. */
    bool                ConvertAddress( XclAddress& rXclPos,
                            const ScAddress& rScPos, bool bWarn );

    /** Returns a valid cell address by moving it into allowed dimensions.
        @param rScPos  The Calc cell address to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell address is invalid.
        @return  The converted Excel cell address. */
    XclAddress          CreateValidAddress( const ScAddress& rScPos, bool bWarn );

    // cell range -------------------------------------------------------------

    /** Checks if the passed cell range is valid (checks start and end position).
        @param rScRange  The Calc cell range to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell range is not valid.
        @return  true = Cell range in rScRange is valid. */
    bool                CheckRange( const ScRange& rScRange, bool bWarn );

    /** Checks and eventually crops the cell range to valid dimensions.
        @descr  The start position of the range will not be modified.
        @param rScRange  (In/out) The cell range to validate.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell range contains invalid
            cells. If the range is partly valid, this function sets the warning
            flag, corrects the range and returns true.
        @return  true = Cell range in rScRange is valid (original or cropped). */
    bool                ValidateRange( ScRange& rScRange, bool bWarn );

    /** Converts the passed Calc cell range to an Excel cell range.
        @param rXclRange  (Out) The converted Excel cell range, if valid.
        @param rScRange  The Calc cell range to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the cell range contains invalid cells.
        @return  true = Cell range returned in rXclRange is valid (original or cropped). */
    bool                ConvertRange( XclRange& rXclRange, const ScRange& rScRange, bool bWarn );

    // cell range list --------------------------------------------------------

    /** Checks and eventually crops the cell ranges to valid dimensions.
        @descr  The start position of the ranges will not be modified. Cell
            ranges that fit partly into valid dimensions are cropped
            accordingly. Cell ranges that do not fit at all, are removed from
            the cell range list.
        @param rScRanges  (In/out) The cell range list to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if at least one of the cell ranges
            contains invalid cells. */
    void                ValidateRangeList( ScRangeList& rScRanges, bool bWarn );

    /** Converts the passed Calc cell range list to an Excel cell range list.
        @descr  The start position of the ranges will not be modified. Cell
            ranges that fit partly into valid dimensions are cropped
            accordingly. Cell ranges that do not fit at all, are not inserted
            into the Excel cell range list.
        @param rXclRanges  (Out) The converted Excel cell range list.
        @param rScRanges  The Calc cell range list to convert.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if at least one of the cell ranges
            contains invalid cells. */
    void                ConvertRangeList( XclRangeList& rXclRanges,
                            const ScRangeList& rScRanges, bool bWarn );
};

// EditEngine->String conversion ==============================================

class SvxURLField;
class XclExpHyperlink;

/** Helper to create HLINK records during creation of formatted cell strings.

    In Excel it is not possible to have more than one hyperlink in a cell. This
    helper detects multiple occurrences of hyperlinks and fills a string which
    is used to create a cell note containing all URLs. Only cells containing
    one hyperlink are exported as hyperlink cells.
 */
class XclExpHyperlinkHelper : protected XclExpRoot
{
public:
    typedef std::shared_ptr< XclExpHyperlink > XclExpHyperlinkRef;

    explicit            XclExpHyperlinkHelper( const XclExpRoot& rRoot, const ScAddress& rScPos );
                        virtual ~XclExpHyperlinkHelper();

    /** Processes the passed URL field (tries to create a HLINK record).
        @return  The representation string of the URL field. */
    OUString ProcessUrlField( const SvxURLField& rUrlField );

    /** Returns true, if a single HLINK record has been created. */
    bool                HasLinkRecord() const;
    /** Returns the craeted single HLINk record, or an empty reference. */
    XclExpHyperlinkRef  GetLinkRecord();

    /** Returns true, if multiple URLs have been processed. */
    inline bool         HasMultipleUrls() const { return mbMultipleUrls; }
    /** Returns a string containing all processed URLs. */
    inline const OUString& GetUrlList() { return maUrlList; }

private:
    XclExpHyperlinkRef  mxLinkRec;          /// Created HLINK record.
    ScAddress           maScPos;            /// Cell position to set at the HLINK record.
    OUString            maUrlList;          /// List with all processed URLs.
    bool                mbMultipleUrls;     /// true = Multiple URL fields processed.
};

class EditEngine;
class EditTextObject;
class SdrTextObj;
class ScPatternAttr;

/** This class provides methods to create an XclExpString.
    @descr  The string can be created from an edit engine text object or
    directly from a Calc edit cell. */
class XclExpStringHelper : boost::noncopyable
{
public:
    /** Creates a new unformatted string from the passed string.
        @descr  Creates a Unicode string or a byte string, depending on the
                current BIFF version contained in the passed XclExpRoot object.
        @param rString  The source string.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object (shared pointer). */
    static XclExpStringRef CreateString(
                            const XclExpRoot& rRoot,
                            const OUString& rString,
                            XclStrFlags nFlags = EXC_STR_DEFAULT,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Creates a new unformatted string from the passed character.
        @descr  Creates a Unicode string or a byte string, depending on the
                current BIFF version contained in the passed XclExpRoot object.
        @param cChar  The source character. The NUL character is explicitly allowed.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object (shared pointer). */
    static XclExpStringRef CreateString(
                            const XclExpRoot& rRoot,
                            sal_Unicode cChar,
                            XclStrFlags nFlags = EXC_STR_DEFAULT,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Appends an unformatted string to an Excel string object.
        @descr  Selects the correct Append() function depending on the current
                BIFF version contained in the passed XclExpRoot object.
        @param rXclString  The Excel string object.
        @param rString  The source string. */
    static void         AppendString(
                            XclExpString& rXclString,
                            const XclExpRoot& rRoot,
                            const OUString& rString );

    /** Appends a character to an Excel string object.
        @descr  Selects the correct Append() function depending on the current
                BIFF version contained in the passed XclExpRoot object.
        @param rXclString  The Excel string object.
        @param rString  The source string. */
    static void         AppendChar(
                            XclExpString& rXclString,
                            const XclExpRoot& rRoot,
                            sal_Unicode cChar );

    /** Creates a new formatted string from a Calc string cell.
        @descr  Creates a Unicode string or a byte string, depending on the
                current BIFF version contained in the passed XclExpRoot object.
                May create a formatted string object, if the cell text contains
                different script types.
        @param rStringCell  The Calc string cell object.
        @param pCellAttr  The set item containing the cell formatting.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object (shared pointer). */
    static XclExpStringRef CreateCellString(
                            const XclExpRoot& rRoot,
                            const OUString& rString,
                            const ScPatternAttr* pCellAttr,
                            XclStrFlags nFlags = EXC_STR_DEFAULT,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Creates a new formatted string from a Calc edit cell.
        @descr  Creates a Unicode string or a byte string, depending on the
                current BIFF version contained in the passed XclExpRoot object.
        @param rEditCell  The Calc edit cell object.
        @param pCellAttr  The set item containing the cell formatting.
        @param rLinkHelper  Helper object for hyperlink conversion.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object (shared pointer). */
    static XclExpStringRef CreateCellString(
                            const XclExpRoot& rRoot,
                            const EditTextObject& rEditText,
                            const ScPatternAttr* pCellAttr,
                            XclExpHyperlinkHelper& rLinkHelper,
                            XclStrFlags nFlags = EXC_STR_DEFAULT,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Creates a new formatted string from a drawing text box.
        @descr  Creates a Unicode string or a byte string, depending on the
                current BIFF version contained in the passed XclExpRoot object.
        @param rTextObj  The text box object.
        @param nFlags  Modifiers for string export.
        @param nMaxLen  The maximum number of characters to store in this string.
        @return  The new string object (shared pointer). */
    static XclExpStringRef CreateString(
                            const XclExpRoot& rRoot,
                            const SdrTextObj& rTextObj,
                            XclStrFlags nFlags = EXC_STR_DEFAULT,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Creates a new formatted string from a edit text string.
        @param rEditObj  The edittext object.
        @param nFlags  Modifiers for string export.
        @param nMaxLen The maximum number of characters to store in this string.
        @return  The new string object. */
    static XclExpStringRef CreateString(
                            const XclExpRoot& rRoot,
                            const EditTextObject& rEditObj,
                            XclStrFlags nFlags = EXC_STR_DEFAULT,
                            sal_uInt16 nMaxLen = EXC_STR_MAXLEN );

    /** Returns the script type first text portion different to WEAK, or the system
        default script type, if there is only weak script in the passed string. */
    static sal_Int16    GetLeadingScriptType( const XclExpRoot& rRoot, const OUString& rString );

private:
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static methods. To enforce this, the default constructor
        is made private */
    XclExpStringHelper();
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
class XclExpHFConverter : protected XclExpRoot, private boost::noncopyable
{
public:
    explicit            XclExpHFConverter( const XclExpRoot& rRoot );

    /** Generates the header/footer string from the passed edit engine text objects. */
    void                GenerateString(
                            const EditTextObject* pLeftObj,
                            const EditTextObject* pCenterObj,
                            const EditTextObject* pRightObj );

    /** Returns the last generated header/footer string. */
    inline const OUString& GetHFString() const { return maHFString; }
    /** Returns the total height of the last generated header/footer in twips. */
    inline sal_Int32    GetTotalHeight() const { return mnTotalHeight; }

private:
    /** Converts the text object contents and stores it in the passed string. */
    void                AppendPortion(
                            const EditTextObject* pTextObj,
                            sal_Unicode cPortionCode );

private:
    EditEngine&         mrEE;           /// The header/footer edit engine.
    OUString            maHFString;     /// The last generated header/footer string.
    sal_Int32           mnTotalHeight;  /// Total height of the last header/footer (twips).
};

// URL conversion =============================================================

/** This class contains static methods to encode a file URL.
    @descr  Excel stores URLs in a format that contains special control characters,
    i.e. for directory separators or volume names. */
class XclExpUrlHelper : boost::noncopyable
{
public:
    /** Encodes and returns the URL passed in rAbsUrl to an Excel like URL.
        @param pTableName  Optional pointer to a table name to be encoded in this URL. */
    static OUString EncodeUrl( const XclExpRoot& rRoot, const OUString& rAbsUrl, const OUString* pTableName = nullptr );
    /** Encodes and returns the passed DDE link to an Excel like DDE link. */
    static OUString EncodeDde( const OUString& rApplic, const OUString& rTopic );

private:
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static methods. To enforce this, the default constructor
        is made private */
    XclExpUrlHelper();
};

class ScMatrix;

/** Contains cached values in a 2-dimensional array. */
class XclExpCachedMatrix
{
    void            GetDimensions( SCSIZE & nCols, SCSIZE & nRows ) const;
public:
    /** Constructs and fills a new matrix.
        @param rMatrix  The Calc value matrix. */
    explicit        XclExpCachedMatrix( const ScMatrix& rMatrix );
                   ~XclExpCachedMatrix();

    /** Returns the byte count of all contained data. */
    sal_Size        GetSize() const;
    /** Writes the complete matrix to stream. */
    void            Save( XclExpStream& rStrm ) const;

private:
    const ScMatrix& mrMatrix;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
