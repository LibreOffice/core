/*************************************************************************
 *
 *  $RCSfile: xihelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:19:35 $
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

/** Enumeration to specify a portion of a header or footer. */
enum XclHFPortion
{
    xlHFLeft = 0,               /// Left portion.
    xlHFCenter = 1,             /// Centered portion.
    xlHFRight = 2               /// Right portion.
};


// ----------------------------------------------------------------------------

class EditEngine;
class EditTextObject;
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
private:
    typedef ::std::auto_ptr< EditTextObject >   EditTextObjectPtr;
    typedef ::std::auto_ptr< XclFontData >      XclFontDataPtr;

    EditEngine&                 mrEE;           /// The header/footer edit engine.
    XclHFPortion                meCurrObj;      /// The current portion.
    EditTextObjectPtr           mppObjs[ 3 ];   /// Edit engine text objects for all portions.
    ESelection                  mpSels[ 3 ];    /// Edit engine selections for all portions.
    String                      maCurrText;     /// Current text to insert into edit engine.
    XclFontDataPtr              mpFontData;     /// Font data of current text.

public:
    explicit                    XclImpHFConverter( const XclImpRoot& rRoot );

    /** Parses the passed string and creates three new edit engine text objects. */
    void                        ParseString( const String& rHFString );

    /** Returns a pointer to the specified edit engine text object.
        @descr  This parser remains owner of the text object. */
    inline const EditTextObject* GetTextObject( XclHFPortion ePortion ) const
                                    { return mppObjs[ ePortion ].get(); }

private:
    /** Returns the current edit engine text object. */
    inline EditTextObjectPtr&   GetCurrObj() { return mppObjs[ meCurrObj ]; }
    /** Returns the current selection. */
    inline ESelection&          GetCurrSel() { return mpSels[ meCurrObj ]; }

    /** Sets the font attributes at the current selection.
        @descr  After that, the start position of the current selection object is
        adjusted to the end of the selection. */
    void                        SetAttribs();

    /** Inserts maCurrText into edit engine and adjusts the current selection object.
        @descr  The text shall not contain a newline character.
        The text will be cleared after insertion. */
    void                        InsertText();
    /** Inserts a line break and adjusts the current selection object. */
    void                        InsertLineBreak();
    /** Inserts the passed text field and adjusts the current selection object. */
    void                        InsertField( const SvxFieldItem& rFieldItem );

    /** Creates the edit engine text object of current portion from edit engine. */
    void                        CreateCurrObject();
    /** Changes current header/footer portion to eNew.
        @descr  Creates text object of current portion and reinitializes edit engine. */
    void                        SetNewPortion( XclHFPortion eNew );
};


// URL conversion =============================================================

/** This class contains static methods to decode an URL stored in an Excel file.
    @descr  Excel URLs can contain a sheet name, for instance: path\[test.xls]Sheet1
    This sheet name will be extracted automatically. */
class XclImpUrlHelper : ScfNoInstance
{
public:
    /** Reads and decodes an encoded external document URL.
        @param rUrl  Returns the decoded file name incl. path.
        @param rTable  Returns the decoded sheet name.
        @param rbSameWb  Returns true, if the URL is a reference to the own workbook.
        @param rEncodedUrl   An encoded URL from Excel. */
    static void                 DecodeUrl(
                                    String& rUrl,
                                    String& rTable,
                                    bool& rbSameWb,
                                    const XclImpRoot& rRoot,
                                    const String& rEncodedUrl );
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
    sal_uInt8                   mnType;     /// The type of the cached value (EXC_CACHEDVAL_*).
    sal_uInt16                  mnCol;      /// Column index of the cached cell.
    sal_uInt16                  mnRow;      /// Row index of the cached cell.

public:
    /** Creates a cached value and reads contents from stream and stores it with its array address. */
                                XclImpCachedValue( XclImpStream& rStrm,
                                    sal_uInt16 nCol,
                                    sal_uInt16 nRow);
    virtual                     ~XclImpCachedValue();

    /** Returns the type of the cached value (EXC_CACHEDVAL_*). */
    inline sal_uInt8            GetType() const     { return mnType; }
    /** Returns the cached string value, if this is a EXC_CACHEDVAL_STRING. */
    inline const String*        GetString() const   { return mpStr.get(); }
    /** Returns the cached double value if this is a EXC_CACHEDVAL_DOUBLE. */
    inline double               GetValue() const    { return mfValue; }
    /** Returns the token array if this is a Boolean value or error value. */
    inline const ScTokenArray*  GetTokArray() const { return mpTokArr.get(); }

    inline sal_uInt16           GetCol() const    { return mnCol; }
    inline sal_uInt16           GetRow() const    { return mnRow; }
};


class XclImpCachedMatrix
{
protected:
    typedef ScfDelList< XclImpCachedValue > XclImpValueList;

    XclImpValueList             maValueList;     /// List of cached cell values.
    sal_uInt16                  mnColumns;       /// Number of cached columns.
    sal_uInt16                  mnRows;          /// Number of cached rows.

public:
    explicit                    XclImpCachedMatrix( sal_uInt16 nCols, sal_uInt16 nRows);
                                ~XclImpCachedMatrix();

    /** Copies the contents of our cached matrix into the ScMatrix. */
    void                        FillMatrix(ScDocument &rDoc, ScMatrix *pMatrix) const ;

    /** Stores the contents of an external referenced cell in the Value list. */
    inline void                 AppendValue( XclImpCachedValue* pCachedValue ) { maValueList.Append( pCachedValue ); }

    inline sal_uInt16           GetColumns() const { return mnColumns; }
    inline sal_uInt16           GetRows() const    { return mnRows; }
};

// ============================================================================

#endif

