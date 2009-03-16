/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: worksheethelper.cxx,v $
 * $Revision: 1.5.20.7 $
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

#include "oox/xls/worksheethelper.hxx"
#include <algorithm>
#include <utility>
#include <list>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XMultiFormulaTokens.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/sheet/XMultipleOperation.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/text/XText.hpp>
#include "tokens.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/condformatbuffer.hxx"
#include "oox/xls/drawingfragment.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/sharedformulabuffer.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/unitconverter.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"
#include "oox/xls/worksheetsettings.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::util::XMergeable;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::table::BorderLine;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::XCell;
using ::com::sun::star::table::XCellRange;
using ::com::sun::star::table::XColumnRowRange;
using ::com::sun::star::table::XTableColumns;
using ::com::sun::star::table::XTableRows;
using ::com::sun::star::sheet::ConditionOperator;
using ::com::sun::star::sheet::ValidationType;
using ::com::sun::star::sheet::ValidationAlertStyle;
using ::com::sun::star::sheet::XCellAddressable;
using ::com::sun::star::sheet::XCellRangeAddressable;
using ::com::sun::star::sheet::XFormulaTokens;
using ::com::sun::star::sheet::XLabelRanges;
using ::com::sun::star::sheet::XMultiFormulaTokens;
using ::com::sun::star::sheet::XMultipleOperation;
using ::com::sun::star::sheet::XSheetCellRangeContainer;
using ::com::sun::star::sheet::XSheetCellRanges;
using ::com::sun::star::sheet::XSheetCondition;
using ::com::sun::star::sheet::XSheetOutline;
using ::com::sun::star::sheet::XSpreadsheet;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextRange;

namespace oox {
namespace xls {

// ============================================================================

namespace {

void lclUpdateProgressBar( ISegmentProgressBarRef xProgressBar, const CellRangeAddress& rDimension, sal_Int32 nRow )
{
    if( xProgressBar.get() && (rDimension.StartRow <= nRow) && (nRow <= rDimension.EndRow) )
    {
        double fPosition = static_cast< double >( nRow - rDimension.StartRow + 1 ) / (rDimension.EndRow - rDimension.StartRow + 1);
        if( xProgressBar->getPosition() < fPosition )
            xProgressBar->setPosition( fPosition );
    }
}

// ----------------------------------------------------------------------------

struct ValueRange
{
    sal_Int32           mnFirst;
    sal_Int32           mnLast;

    inline explicit     ValueRange( sal_Int32 nValue ) : mnFirst( nValue ), mnLast( nValue ) {}
    inline explicit     ValueRange( sal_Int32 nFirst, sal_Int32 nLast ) : mnFirst( nFirst ), mnLast( nLast ) {}
};

typedef ::std::vector< ValueRange > ValueRangeVector;

// ----------------------------------------------------------------------------

struct ValueRangeComp
{
    inline bool         operator()( const ValueRange& rRange, sal_Int32 nValue ) const { return rRange.mnLast < nValue; }
};

typedef ::std::vector< ValueRange > ValueRangeVector;

// ----------------------------------------------------------------------------

class ValueRangeSet
{
public:
    inline explicit     ValueRangeSet() {}

    void                insert( sal_Int32 nValue );
    void                intersect( ValueRangeVector& orRanges, sal_Int32 nFirst, sal_Int32 nLast ) const;

private:
    ValueRangeVector    maData;
};

void ValueRangeSet::insert( sal_Int32 nValue )
{
    // find the first range that contains nValue or that follows nValue
    ValueRangeVector::iterator aBeg = maData.begin();
    ValueRangeVector::iterator aEnd = maData.end();
    ValueRangeVector::iterator aNext = ::std::lower_bound( aBeg, aEnd, nValue, ValueRangeComp() );

    // nothing to do if found range contains nValue
    if( (aNext == aEnd) || (nValue < aNext->mnFirst) )
    {
        ValueRangeVector::iterator aPrev = (aNext == aBeg) ? aEnd : (aNext - 1);
        bool bJoinPrev = (aPrev != aEnd) && (aPrev->mnLast + 1 == nValue);
        bool bJoinNext = (aNext != aEnd) && (aNext->mnFirst - 1 == nValue);
        if( bJoinPrev && bJoinNext )
        {
            aPrev->mnLast = aNext->mnLast;
            maData.erase( aNext );
        }
        else if( bJoinPrev )
            ++aPrev->mnLast;
        else if( bJoinNext )
            --aNext->mnFirst;
        else
            maData.insert( aNext, ValueRange( nValue ) );
    }
}

void ValueRangeSet::intersect( ValueRangeVector& orRanges, sal_Int32 nFirst, sal_Int32 nLast ) const
{
    orRanges.clear();
    // find the range that contains nFirst or the first range that follows nFirst
    ValueRangeVector::const_iterator aIt = ::std::lower_bound( maData.begin(), maData.end(), nFirst, ValueRangeComp() );
    for( ValueRangeVector::const_iterator aEnd = maData.end(); (aIt != aEnd) && (aIt->mnFirst <= nLast); ++aIt )
        orRanges.push_back( ValueRange( ::std::max( aIt->mnFirst, nFirst ), ::std::min( aIt->mnLast, nLast ) ) );
}

} // namespace

// ============================================================================
// ============================================================================

void OoxCellData::reset()
{
    mxCell.clear();
    maValueStr = maFormulaRef = OUString();
    mnCellType = mnFormulaType = XML_TOKEN_INVALID;
    mnSharedId = mnXfId = mnNumFmtId = -1;
    mbHasValueStr = mbShowPhonetic = false;
}

// ----------------------------------------------------------------------------

OoxDataTableData::OoxDataTableData() :
    mb2dTable( false ),
    mbRowTable( false ),
    mbRef1Deleted( false ),
    mbRef2Deleted( false )
{
}

// ----------------------------------------------------------------------------

OoxColumnData::OoxColumnData() :
    mnFirstCol( -1 ),
    mnLastCol( -1 ),
    mfWidth( 0.0 ),
    mnXfId( -1 ),
    mnLevel( 0 ),
    mbShowPhonetic( false ),
    mbHidden( false ),
    mbCollapsed( false )
{
}

bool OoxColumnData::tryExpand( const OoxColumnData& rNewData )
{
    bool bExpandable =
        (mnFirstCol <= rNewData.mnFirstCol) &&
        (rNewData.mnFirstCol <= mnLastCol + 1) &&
        (mfWidth == rNewData.mfWidth) &&
        // ignore mnXfId, cell formatting is always set directly
        (mnLevel == rNewData.mnLevel) &&
        (mbHidden == rNewData.mbHidden) &&
        (mbCollapsed == rNewData.mbCollapsed);
    if( bExpandable )
        mnLastCol = rNewData.mnLastCol;
    return bExpandable;
}

// ----------------------------------------------------------------------------

OoxRowData::OoxRowData() :
    mnFirstRow( -1 ),
    mnLastRow( -1 ),
    mfHeight( 0.0 ),
    mnXfId( -1 ),
    mnLevel( 0 ),
    mbCustomHeight( false ),
    mbCustomFormat( false ),
    mbShowPhonetic( false ),
    mbHidden( false ),
    mbCollapsed( false ),
    mbThickTop( false ),
    mbThickBottom( false )
{
}

bool OoxRowData::tryExpand( const OoxRowData& rNewData )
{
    bool bExpandable =
        (mnFirstRow <= rNewData.mnFirstRow) &&
        (rNewData.mnFirstRow <= mnLastRow + 1) &&
        (mfHeight == rNewData.mfHeight) &&
        // ignore mnXfId, mbCustomFormat, mbShowPhonetic - cell formatting is always set directly
        (mnLevel == rNewData.mnLevel) &&
        (mbCustomHeight == rNewData.mbCustomHeight) &&
        (mbHidden == rNewData.mbHidden) &&
        (mbCollapsed == rNewData.mbCollapsed);
    if( bExpandable )
        mnLastRow = rNewData.mnLastRow;
    return bExpandable;
}

// ----------------------------------------------------------------------------

OoxPageBreakData::OoxPageBreakData() :
    mnColRow( 0 ),
    mbManual( false )
{
}

// ----------------------------------------------------------------------------

OoxHyperlinkData::OoxHyperlinkData()
{
}

// ----------------------------------------------------------------------------

OoxValidationData::OoxValidationData() :
    mnType( XML_none ),
    mnOperator( XML_between ),
    mnErrorStyle( XML_stop ),
    mbShowInputMsg( false ),
    mbShowErrorMsg( false ),
    mbNoDropDown( false ),
    mbAllowBlank( false )
{
}

void OoxValidationData::setBinType( sal_uInt8 nType )
{
    static const sal_Int32 spnTypeIds[] = {
        XML_none, XML_whole, XML_decimal, XML_list, XML_date, XML_time, XML_textLength, XML_custom };
    mnType = STATIC_ARRAY_SELECT( spnTypeIds, nType, XML_none );
}

void OoxValidationData::setBinOperator( sal_uInt8 nOperator )
{
    static const sal_Int32 spnOperators[] = {
        XML_between, XML_notBetween, XML_equal, XML_notEqual,
        XML_greaterThan, XML_lessThan, XML_greaterThanOrEqual, XML_lessThanOrEqual };
    mnOperator = STATIC_ARRAY_SELECT( spnOperators, nOperator, XML_TOKEN_INVALID );
}

void OoxValidationData::setBinErrorStyle( sal_uInt8 nErrorStyle )
{
    static const sal_Int32 spnErrorStyles[] = { XML_stop, XML_warning, XML_information };
    mnErrorStyle = STATIC_ARRAY_SELECT( spnErrorStyles, nErrorStyle, XML_stop );
}

// ----------------------------------------------------------------------------

OoxOleObjectData::OoxOleObjectData() :
    mnAspect( XML_DVASPECT_CONTENT ),
    mnUpdateMode( XML_OLEUPDATE_ALWAYS ),
    mnShapeId( 0 ),
    mbAutoLoad( false )
{
}

// ----------------------------------------------------------------------------

OoxFormControlData::OoxFormControlData() :
    mnShapeId( 0 )
{
}

// ============================================================================
// ============================================================================

class WorksheetData : public WorkbookHelper
{
public:
    explicit            WorksheetData(
                            const WorkbookHelper& rHelper,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

    /** Returns true, if this helper refers to an existing Calc sheet. */
    inline bool         isValidSheet() const { return mxSheet.is(); }

    /** Returns a cell formula simulating the passed boolean value. */
    const OUString&     getBooleanFormula( bool bValue ) const;

    /** Returns the type of this sheet. */
    inline WorksheetType getSheetType() const { return meSheetType; }
    /** Returns the index of the current sheet. */
    inline sal_Int16    getSheetIndex() const { return mnSheet; }
    /** Returns the XSpreadsheet interface of the current sheet. */
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >&
                        getXSpreadsheet() const { return mxSheet; }

    /** Returns the XCell interface for the passed cell address. */
    Reference< XCell >  getCell( const CellAddress& rAddress ) const;
    /** Returns the XCellRange interface for the passed cell range address. */
    Reference< XCellRange > getCellRange( const CellRangeAddress& rRange ) const;
    /** Returns the XSheetCellRanges interface for the passed cell range addresses. */
    Reference< XSheetCellRanges > getCellRangeList( const ApiCellRangeList& rRanges ) const;

    /** Returns the XCellRange interface for a column. */
    Reference< XCellRange > getColumn( sal_Int32 nCol ) const;
    /** Returns the XCellRange interface for a row. */
    Reference< XCellRange > getRow( sal_Int32 nRow ) const;

    /** Returns the XTableColumns interface for a range of columns. */
    Reference< XTableColumns > getColumns( sal_Int32 nFirstCol, sal_Int32 nLastCol ) const;
    /** Returns the XTableRows interface for a range of rows. */
    Reference< XTableRows > getRows( sal_Int32 nFirstRow, sal_Int32 nLastRow ) const;

    /** Returns the absolute cell position in 1/100 mm. */
    Point               getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the cell size in 1/100 mm. */
    Size                getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the size of the entire drawing page in 1/100 mm. */
    Size                getDrawPageSize() const;

    /** Returns the worksheet settings object. */
    inline WorksheetSettings& getWorksheetSettings() { return maSheetSett; }
    /** Returns the buffer containing all shared formulas in this sheet. */
    inline SharedFormulaBuffer& getSharedFormulas() { return maSharedFmlas; }
    /** Returns the conditional formattings in this sheet. */
    inline CondFormatBuffer& getCondFormats() { return maCondFormats; }
    /** Returns the page/print settings for this sheet. */
    inline PageSettings& getPageSettings() { return maPageSett; }
    /** Returns the view settings for this sheet. */
    inline SheetViewSettings& getSheetViewSettings() { return maSheetViewSett; }

    /** Changes the current sheet type. */
    inline void         setSheetType( WorksheetType eSheetType ) { meSheetType = eSheetType; }
    /** Sets the dimension (used area) of the sheet. */
    void                setDimension( const CellRangeAddress& rRange );
    /** Stores the cell format at the passed address. */
    void                setCellFormat( const OoxCellData& rCellData );
    /** Merges the cells in the passed cell range. */
    void                setMergedRange( const CellRangeAddress& rRange );
    /** Sets a column or row page break described in the passed struct. */
    void                setPageBreak( const OoxPageBreakData& rData, bool bRowBreak );
    /** Inserts the hyperlink URL into the spreadsheet. */
    void                setHyperlink( const OoxHyperlinkData& rHyperlink );
    /** Inserts the data validation settings into the spreadsheet. */
    void                setValidation( const OoxValidationData& rValData );
    /** Sets the path to the DrawingML fragment of this sheet. */
    void                setDrawingPath( const OUString& rDrawingPath );
    /** Sets the path to the legacy VML drawing fragment of this sheet. */
    void                setVmlDrawingPath( const OUString& rVmlDrawingPath );
    /** Sets additional data for an OLE object. */
    void                setOleObject( const OoxOleObjectData& rOleObjectData );
    /** Sets additional data for an OCX form control. */
    void                setFormControl( const OoxFormControlData& rFormControlData );

    /** Sets base width for all columns (without padding pixels). This value
        is only used, if base width has not been set with setDefaultColumnWidth(). */
    void                setBaseColumnWidth( sal_Int32 nWidth );
    /** Sets default width for all columns. This function overrides the base
        width set with the setBaseColumnWidth() function. */
    void                setDefaultColumnWidth( double fWidth );
    /** Sets column settings for a specific column range.
        @descr  Column default formatting is converted directly, other settings
        are cached and converted in the finalizeImport() call. */
    void                setColumnData( const OoxColumnData& rData );

    /** Sets default height and hidden state for all unused rows in the sheet. */
    void                setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom );
    /** Sets row settings for a specific row.
        @descr  Row default formatting is converted directly, other settings
        are cached and converted in the finalizeImport() call. */
    void                setRowData( const OoxRowData& rData );

    /** Converts column default cell formatting. */
    void                convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId );
    /** Converts row default cell formatting. */
    void                convertRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId );

    /** Initial conversion before importing the worksheet. */
    void                initializeWorksheetImport();
    /** Final conversion after importing the worksheet. */
    void                finalizeWorksheetImport();

private:
    typedef ::std::vector< sal_Int32 >                  OutlineLevelVec;
    typedef ::std::map< sal_Int32, OoxColumnData >      OoxColumnDataMap;
    typedef ::std::map< sal_Int32, OoxRowData >         OoxRowDataMap;
    typedef ::std::list< OoxHyperlinkData >             OoxHyperlinkList;
    typedef ::std::list< OoxValidationData >            OoxValidationList;
    typedef ::std::map< sal_Int32, OoxOleObjectData >   OoxOleObjectDataMap;
    typedef ::std::map< sal_Int32, OoxFormControlData > OoxFormControlDataMap;

    struct XfIdRange
    {
        CellRangeAddress    maRange;            /// The formatted cell range.
        sal_Int32           mnXfId;             /// XF identifier for the range.
        sal_Int32           mnNumFmtId;         /// Number format id overriding the XF.

        void                set( const OoxCellData& rCellData );
        bool                tryExpand( const OoxCellData& rCellData );
        bool                tryMerge( const XfIdRange& rXfIdRange );
    };

    struct MergedRange
    {
        CellRangeAddress    maRange;            /// The formatted cell range.
        sal_Int32           mnHorAlign;         /// Horizontal alignment in the range.

        explicit            MergedRange( const CellRangeAddress& rAddress );
        explicit            MergedRange( const CellAddress& rAddress, sal_Int32 nHorAlign );
        bool                tryExpand( const CellAddress& rAddress, sal_Int32 nHorAlign );
    };

    typedef ::std::pair< sal_Int32, sal_Int32 > RowColKey;
    typedef ::std::map< RowColKey, XfIdRange >  XfIdRangeMap;
    typedef ::std::list< MergedRange >          MergedRangeList;

    /** Writes all cell formatting attributes to the passed cell range. */
    void                writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const;
    /** Tries to merge the ranges last inserted in maXfIdRanges with existing ranges. */
    void                mergeXfIdRanges();
    /** Finalizes the remaining ranges in maXfIdRanges. */
    void                finalizeXfIdRanges();

    /** Inserts all imported hyperlinks into their cell ranges. */
    void                finalizeHyperlinkRanges() const;
    /** Inserts a hyperlinks into the specified cell. */
    void                finalizeHyperlink( const CellAddress& rAddress, const OUString& rUrl ) const;

    /** Inserts all imported data validations into their cell ranges. */
    void                finalizeValidationRanges() const;

    /** Merges all cached merged ranges and updates right/bottom cell borders. */
    void                finalizeMergedRanges();
    /** Merges the passed merged range and updates right/bottom cell borders. */
    void                finalizeMergedRange( const CellRangeAddress& rRange );

    /** Imports the drawing layer of the sheet. */
    void                finalizeDrawing();

    /** Converts column properties for all columns in the sheet. */
    void                convertColumns();
    /** Converts column properties. */
    void                convertColumns( OutlineLevelVec& orColLevels, sal_Int32 nFirstCol, sal_Int32 nLastCol, const OoxColumnData& rData );

    /** Converts row properties for all rows in the sheet. */
    void                convertRows();
    /** Converts row properties. */
    void                convertRows( OutlineLevelVec& orRowLevels, sal_Int32 nFirstRow, sal_Int32 nLastRow, const OoxRowData& rData, double fDefHeight = -1.0 );

    /** Converts outline grouping for the passed column or row. */
    void                convertOutlines( OutlineLevelVec& orLevels, sal_Int32 nColRow, sal_Int32 nLevel, bool bCollapsed, bool bRows );
    /** Groups columns or rows for the given range. */
    void                groupColumnsOrRows( sal_Int32 nFirstColRow, sal_Int32 nLastColRow, bool bCollapsed, bool bRows );

private:
    const OUString      maTrueFormula;      /// Replacement formula for TRUE boolean cells.
    const OUString      maFalseFormula;     /// Replacement formula for FALSE boolean cells.
    const OUString      maSheetCellRanges;  /// Service name for a SheetCellRanges object.
    const OUString      maRightBorderProp;  /// Property name of the right border of a cell.
    const OUString      maBottomBorderProp; /// Property name of the bottom border of a cell.
    const OUString      maWidthProp;        /// Property name for column width.
    const OUString      maHeightProp;       /// Property name for row height.
    const OUString      maPositionProp;     /// Property name for cell position.
    const OUString      maSizeProp;         /// Property name for cell size.
    const OUString      maVisibleProp;      /// Property name for column/row visibility.
    const OUString      maTextWrapProp;     /// Property name for automatic text wrap in cells.
    const OUString      maPageBreakProp;    /// Property name of a page break.
    const OUString      maUrlTextField;     /// Service name for a URL text field.
    const OUString      maUrlProp;          /// Property name for the URL string in a URL text field.
    const OUString      maReprProp;         /// Property name for the URL representation in a URL text field.
    const OUString      maValidationProp;   /// Property name for data validation settings.
    const CellAddress&  mrMaxApiPos;        /// Reference to maximum Calc cell address from address converter.
    CellRangeAddress    maDimension;        /// Dimension (used) area of the sheet.
    OoxColumnData       maDefColData;       /// Default column formatting.
    OoxColumnDataMap    maColDatas;         /// Column data sorted by first column index.
    OoxRowData          maDefRowData;       /// Default row formatting.
    OoxRowDataMap       maRowDatas;         /// Row data sorted by row index.
    OoxHyperlinkList    maHyperlinks;       /// Cell ranges containing hyperlinks.
    OoxValidationList   maValidations;      /// Cell ranges containing data validation settings.
    OoxOleObjectDataMap maOleObjects;       /// OLE object data, mapped to VML shape identifier.
    OoxFormControlDataMap maFormControls;   /// OCX form control data, mapped to VML shape identifier.
    XfIdRangeMap        maXfIdRanges;       /// Collected XF identifiers for cell ranges.
    MergedRangeList     maMergedRanges;     /// Merged cell ranges.
    MergedRangeList     maCenterFillRanges; /// Merged cell ranges from 'center across' or 'fill' alignment.
    ValueRangeSet       maManualRowHeights; /// Rows that need manual height independent from own settings.
    WorksheetSettings   maSheetSett;        /// Global settings for this sheet.
    SharedFormulaBuffer maSharedFmlas;      /// Buffer for shared formulas in this sheet.
    CondFormatBuffer    maCondFormats;      /// Buffer for conditional formattings.
    PageSettings        maPageSett;         /// Page/print settings for this sheet.
    SheetViewSettings   maSheetViewSett;    /// View settings for this sheet.
    OUString            maDrawingPath;      /// Path to DrawingML fragment.
    OUString            maVmlDrawingPath;   /// Path to legacy VML drawing fragment.
    ISegmentProgressBarRef mxProgressBar;   /// Sheet progress bar.
    ISegmentProgressBarRef mxRowProgress;   /// Progress bar for row/cell processing.
    ISegmentProgressBarRef mxFinalProgress; /// Progress bar for finalization.
    WorksheetType       meSheetType;        /// Type of this sheet.
    Reference< XSpreadsheet > mxSheet;      /// Reference to the current sheet.
    sal_Int16           mnSheet;            /// Index of the current sheet.
    bool                mbHasDefWidth;      /// True = default column width is set from defaultColWidth attribute.
};

// ----------------------------------------------------------------------------

WorksheetData::WorksheetData( const WorkbookHelper& rHelper, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    WorkbookHelper( rHelper ),
    maTrueFormula( CREATE_OUSTRING( "=TRUE()" ) ),
    maFalseFormula( CREATE_OUSTRING( "=FALSE()" ) ),
    maSheetCellRanges( CREATE_OUSTRING( "com.sun.star.sheet.SheetCellRanges" ) ),
    maRightBorderProp( CREATE_OUSTRING( "RightBorder" ) ),
    maBottomBorderProp( CREATE_OUSTRING( "BottomBorder" ) ),
    maWidthProp( CREATE_OUSTRING( "Width" ) ),
    maHeightProp( CREATE_OUSTRING( "Height" ) ),
    maPositionProp( CREATE_OUSTRING( "Position" ) ),
    maSizeProp( CREATE_OUSTRING( "Size" ) ),
    maVisibleProp( CREATE_OUSTRING( "IsVisible" ) ),
    maTextWrapProp( CREATE_OUSTRING( "IsTextWrapped" ) ),
    maPageBreakProp( CREATE_OUSTRING( "IsStartOfNewPage" ) ),
    maUrlTextField( CREATE_OUSTRING( "com.sun.star.text.TextField.URL" ) ),
    maUrlProp( CREATE_OUSTRING( "URL" ) ),
    maReprProp( CREATE_OUSTRING( "Representation" ) ),
    maValidationProp( CREATE_OUSTRING( "Validation" ) ),
    mrMaxApiPos( rHelper.getAddressConverter().getMaxApiAddress() ),
    maSheetSett( *this ),
    maSharedFmlas( *this ),
    maCondFormats( *this ),
    maPageSett( *this ),
    maSheetViewSett( *this ),
    mxProgressBar( xProgressBar ),
    meSheetType( eSheetType ),
    mnSheet( static_cast< sal_Int16 >( nSheet ) ),
    mbHasDefWidth( false )
{
    OSL_ENSURE( nSheet <= SAL_MAX_INT16, "WorksheetData::WorksheetData - invalid sheet index" );
    mxSheet = getSheet( nSheet );
    if( !mxSheet.is() )
        mnSheet = -1;

    maDimension.Sheet = mnSheet;

    // default column settings (width and hidden state may be updated later)
    maDefColData.mfWidth = 8.5;
    maDefColData.mnXfId = -1;
    maDefColData.mnLevel = 0;
    maDefColData.mbHidden = false;
    maDefColData.mbCollapsed = false;

    // default row settings (height and hidden state may be updated later)
    maDefRowData.mfHeight = 0.0;
    maDefRowData.mnXfId = -1;
    maDefRowData.mnLevel = 0;
    maDefRowData.mbCustomHeight = false;
    maDefRowData.mbCustomFormat = false;
    maDefRowData.mbShowPhonetic = false;
    maDefRowData.mbHidden = false;
    maDefRowData.mbCollapsed = false;

    if( mxProgressBar.get() )
    {
        mxRowProgress = mxProgressBar->createSegment( 0.5 );
        mxFinalProgress = mxProgressBar->createSegment( 0.5 );
    }
}

const OUString& WorksheetData::getBooleanFormula( bool bValue ) const
{
    return bValue ? maTrueFormula : maFalseFormula;
}

Reference< XCell > WorksheetData::getCell( const CellAddress& rAddress ) const
{
    Reference< XCell > xCell;
    if( mxSheet.is() ) try
    {
        xCell = mxSheet->getCellByPosition( rAddress.Column, rAddress.Row );
    }
    catch( Exception& )
    {
    }
    return xCell;
}

Reference< XCellRange > WorksheetData::getCellRange( const CellRangeAddress& rRange ) const
{
    Reference< XCellRange > xRange;
    if( mxSheet.is() ) try
    {
        xRange = mxSheet->getCellRangeByPosition( rRange.StartColumn, rRange.StartRow, rRange.EndColumn, rRange.EndRow );
    }
    catch( Exception& )
    {
    }
    return xRange;
}

Reference< XSheetCellRanges > WorksheetData::getCellRangeList( const ApiCellRangeList& rRanges ) const
{
    Reference< XSheetCellRanges > xRanges;
    if( mxSheet.is() && !rRanges.empty() ) try
    {
        Reference< XMultiServiceFactory > xFactory( getDocument(), UNO_QUERY_THROW );
        xRanges.set( xFactory->createInstance( maSheetCellRanges ), UNO_QUERY_THROW );
        Reference< XSheetCellRangeContainer > xRangeCont( xRanges, UNO_QUERY_THROW );
        xRangeCont->addRangeAddresses( ContainerHelper::vectorToSequence( rRanges ), sal_False );
    }
    catch( Exception& )
    {
    }
    return xRanges;
}

Reference< XCellRange > WorksheetData::getColumn( sal_Int32 nCol ) const
{
    Reference< XCellRange > xColumn;
    try
    {
        Reference< XColumnRowRange > xColRowRange( mxSheet, UNO_QUERY_THROW );
        Reference< XTableColumns > xColumns = xColRowRange->getColumns();
        if( xColumns.is() )
            xColumn.set( xColumns->getByIndex( nCol ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xColumn;
}

Reference< XCellRange > WorksheetData::getRow( sal_Int32 nRow ) const
{
    Reference< XCellRange > xRow;
    try
    {
        Reference< XColumnRowRange > xColRowRange( mxSheet, UNO_QUERY_THROW );
        Reference< XTableRows > xRows = xColRowRange->getRows();
            xRow.set( xRows->getByIndex( nRow ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xRow;
}

Reference< XTableColumns > WorksheetData::getColumns( sal_Int32 nFirstCol, sal_Int32 nLastCol ) const
{
    Reference< XTableColumns > xColumns;
    nLastCol = ::std::min( nLastCol, mrMaxApiPos.Column );
    if( (0 <= nFirstCol) && (nFirstCol <= nLastCol) )
    {
        Reference< XColumnRowRange > xRange( getCellRange( CellRangeAddress( mnSheet, nFirstCol, 0, nLastCol, 0 ) ), UNO_QUERY );
        if( xRange.is() )
            xColumns = xRange->getColumns();
    }
    return xColumns;
}

Reference< XTableRows > WorksheetData::getRows( sal_Int32 nFirstRow, sal_Int32 nLastRow ) const
{
    Reference< XTableRows > xRows;
    nLastRow = ::std::min( nLastRow, mrMaxApiPos.Row );
    if( (0 <= nFirstRow) && (nFirstRow <= nLastRow) )
    {
        Reference< XColumnRowRange > xRange( getCellRange( CellRangeAddress( mnSheet, 0, nFirstRow, 0, nLastRow ) ), UNO_QUERY );
        if( xRange.is() )
            xRows = xRange->getRows();
    }
    return xRows;
}

Point WorksheetData::getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const
{
    Point aPoint;
    PropertySet aCellProp( getCell( CellAddress( mnSheet, nCol, nRow ) ) );
    aCellProp.getProperty( aPoint, maPositionProp );
    return aPoint;
}

Size WorksheetData::getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const
{
    Size aSize;
    PropertySet aCellProp( getCell( CellAddress( mnSheet, nCol, nRow ) ) );
    aCellProp.getProperty( aSize, maSizeProp );
    return aSize;
}

Size WorksheetData::getDrawPageSize() const
{
    Size aSize;
    PropertySet aRangeProp( getCellRange( CellRangeAddress( mnSheet, 0, 0, mrMaxApiPos.Column, mrMaxApiPos.Row ) ) );
    aRangeProp.getProperty( aSize, maSizeProp );
    return aSize;
}

void WorksheetData::setDimension( const CellRangeAddress& rRange )
{
    maDimension = rRange;
}

void WorksheetData::setCellFormat( const OoxCellData& rCellData )
{
    if( rCellData.mxCell.is() && ((rCellData.mnXfId >= 0) || (rCellData.mnNumFmtId >= 0)) )
    {
        // try to merge existing ranges and to write some formatting properties
        if( !maXfIdRanges.empty() )
        {
            // get row index of last inserted cell
            sal_Int32 nLastRow = maXfIdRanges.rbegin()->second.maRange.StartRow;
            // row changed - try to merge ranges of last row with existing ranges
            if( rCellData.maAddress.Row != nLastRow )
            {
                mergeXfIdRanges();
                // write format properties of all ranges above last row and remove them
                XfIdRangeMap::iterator aIt = maXfIdRanges.begin(), aEnd = maXfIdRanges.end();
                while( aIt != aEnd )
                {
                    if( aIt->second.maRange.EndRow < nLastRow )
                    {
                        writeXfIdRangeProperties( aIt->second );
                        maXfIdRanges.erase( aIt++ );
                    }
                    else
                        ++aIt;
                }
            }
        }

        // try to expand last existing range, or create new range entry
        if( maXfIdRanges.empty() || !maXfIdRanges.rbegin()->second.tryExpand( rCellData ) )
            maXfIdRanges[ RowColKey( rCellData.maAddress.Row, rCellData.maAddress.Column ) ].set( rCellData );

        // update merged ranges for 'center across selection' and 'fill'
        if( const Xf* pXf = getStyles().getCellXf( rCellData.mnXfId ).get() )
        {
            sal_Int32 nHorAlign = pXf->getAlignment().getOoxData().mnHorAlign;
            if( (nHorAlign == XML_centerContinuous) || (nHorAlign == XML_fill) )
            {
                /*  start new merged range, if cell is not empty (#108781#),
                    or try to expand last range with empty cell */
                if( rCellData.mnCellType != XML_TOKEN_INVALID )
                    maCenterFillRanges.push_back( MergedRange( rCellData.maAddress, nHorAlign ) );
                else if( !maCenterFillRanges.empty() )
                    maCenterFillRanges.rbegin()->tryExpand( rCellData.maAddress, nHorAlign );
            }
        }
    }
}

void WorksheetData::setMergedRange( const CellRangeAddress& rRange )
{
    maMergedRanges.push_back( MergedRange( rRange ) );
}

void WorksheetData::setPageBreak( const OoxPageBreakData& rData, bool bRowBreak )
{
    if( rData.mbManual && (rData.mnColRow > 0) )
    {
        PropertySet aPropSet( bRowBreak ? getRow( rData.mnColRow ) : getColumn( rData.mnColRow ) );
        aPropSet.setProperty( maPageBreakProp, true );
    }
}

void WorksheetData::setHyperlink( const OoxHyperlinkData& rHyperlink )
{
    maHyperlinks.push_back( rHyperlink );
}

void WorksheetData::setValidation( const OoxValidationData& rValData )
{
    maValidations.push_back( rValData );
}

void WorksheetData::setDrawingPath( const OUString& rDrawingPath )
{
    maDrawingPath = rDrawingPath;
}

void WorksheetData::setVmlDrawingPath( const OUString& rVmlDrawingPath )
{
    maVmlDrawingPath = rVmlDrawingPath;
}

void WorksheetData::setOleObject( const OoxOleObjectData& rOleObjectData )
{
    if( rOleObjectData.mnShapeId > 0 )
        maOleObjects[ rOleObjectData.mnShapeId ] = rOleObjectData;
}

void WorksheetData::setFormControl( const OoxFormControlData& rFormControlData )
{
    if( rFormControlData.mnShapeId > 0 )
        maFormControls[ rFormControlData.mnShapeId ] = rFormControlData;
}

void WorksheetData::setBaseColumnWidth( sal_Int32 nWidth )
{
    // do not modify width, if setDefaultColumnWidth() has been used
    if( !mbHasDefWidth && (nWidth > 0) )
    {
        /*  #i3006# add 5 pixels padding to the width, assuming 1 pixel =
            1/96 inch. => 5/96 inch == 1.32 mm. */
        const UnitConverter& rUnitConv = getUnitConverter();
        maDefColData.mfWidth = rUnitConv.scaleFromMm100( rUnitConv.scaleToMm100( nWidth, UNIT_DIGIT ) + 132, UNIT_DIGIT );
    }
}

void WorksheetData::setDefaultColumnWidth( double fWidth )
{
    // overrides a width set with setBaseColumnWidth()
    if( fWidth > 0.0 )
    {
        maDefColData.mfWidth = fWidth;
        mbHasDefWidth = true;
    }
}

void WorksheetData::setColumnData( const OoxColumnData& rData )
{
    // convert 1-based OOX column indexes to 0-based API column indexes
    sal_Int32 nFirstCol = rData.mnFirstCol - 1;
    sal_Int32 nLastCol = rData.mnLastCol - 1;
    if( (0 <= nFirstCol) && (nFirstCol <= mrMaxApiPos.Column) )
    {
        // set column formatting directly, nLastCol is checked inside the function
        convertColumnFormat( nFirstCol, nLastCol, rData.mnXfId );
        // expand last entry or add new entry
        if( maColDatas.empty() || !maColDatas.rbegin()->second.tryExpand( rData ) )
            maColDatas[ nFirstCol ] = rData;
    }
}

void WorksheetData::setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom )
{
    maDefRowData.mfHeight = fHeight;
    maDefRowData.mbCustomHeight = bCustomHeight;
    maDefRowData.mbHidden = bHidden;
    maDefRowData.mbThickTop = bThickTop;
    maDefRowData.mbThickBottom = bThickBottom;
}

void WorksheetData::setRowData( const OoxRowData& rData )
{
    // convert 1-based OOX row indexes to 0-based API row indexes
    sal_Int32 nFirstRow = rData.mnFirstRow - 1;
    sal_Int32 nLastRow = rData.mnLastRow - 1;
    if( (0 <= nFirstRow) && (nFirstRow <= mrMaxApiPos.Row) )
    {
        // set row formatting directly
        if( rData.mbCustomFormat )
            convertRowFormat( nFirstRow, nLastRow, rData.mnXfId );
        // expand last entry or add new entry
        if( maRowDatas.empty() || !maRowDatas.rbegin()->second.tryExpand( rData ) )
            maRowDatas[ nFirstRow ] = rData;
    }
    lclUpdateProgressBar( mxRowProgress, maDimension, nLastRow );
}

void WorksheetData::convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId )
{
    CellRangeAddress aRange( mnSheet, nFirstCol, 0, nLastCol, mrMaxApiPos.Row );
    if( getAddressConverter().validateCellRange( aRange, false ) )
    {
        PropertySet aPropSet( getCellRange( aRange ) );
        getStyles().writeCellXfToPropertySet( aPropSet, nXfId );
    }
}

void WorksheetData::convertRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId )
{
    CellRangeAddress aRange( mnSheet, 0, nFirstRow, mrMaxApiPos.Column, nLastRow );
    if( getAddressConverter().validateCellRange( aRange, false ) )
    {
        PropertySet aPropSet( getCellRange( aRange ) );
        getStyles().writeCellXfToPropertySet( aPropSet, nXfId );
    }
}

void WorksheetData::initializeWorksheetImport()
{
#if OOX_XLS_USE_DEFAULT_STYLE
#else
    // set default cell style for unused cells
    PropertySet aPropSet( mxSheet );
    aPropSet.setProperty( CREATE_OUSTRING( "CellStyle" ), getStyles().getDefaultStyleName() );
#endif

    /*  remember current sheet index in global data, needed by some global
        objects, e.g. the chart converter. */
    setCurrentSheetIndex( mnSheet );
}

void WorksheetData::finalizeWorksheetImport()
{
    if( mxRowProgress.get() )
        mxRowProgress->setPosition( 1.0 );
    finalizeXfIdRanges();
    finalizeHyperlinkRanges();
    finalizeValidationRanges();
    finalizeMergedRanges();
    maCondFormats.finalizeImport();
    maPageSett.finalizeImport();
    maSheetViewSett.finalizeImport();
    convertColumns();
    convertRows();
    finalizeDrawing();
    if( mxFinalProgress.get() )
        mxFinalProgress->setPosition( 1.0 );

    // reset current sheet index in global data
    setCurrentSheetIndex( -1 );
}

// private --------------------------------------------------------------------

void WorksheetData::XfIdRange::set( const OoxCellData& rCellData )
{
    maRange.Sheet = rCellData.maAddress.Sheet;
    maRange.StartColumn = maRange.EndColumn = rCellData.maAddress.Column;
    maRange.StartRow = maRange.EndRow = rCellData.maAddress.Row;
    mnXfId = rCellData.mnXfId;
    mnNumFmtId = rCellData.mnNumFmtId;
}

bool WorksheetData::XfIdRange::tryExpand( const OoxCellData& rCellData )
{
    if( (mnXfId == rCellData.mnXfId) && (mnNumFmtId == rCellData.mnNumFmtId) &&
        (maRange.StartRow == rCellData.maAddress.Row) &&
        (maRange.EndRow == rCellData.maAddress.Row) &&
        (maRange.EndColumn + 1 == rCellData.maAddress.Column) )
    {
        ++maRange.EndColumn;
        return true;
    }
    return false;
}

bool WorksheetData::XfIdRange::tryMerge( const XfIdRange& rXfIdRange )
{
    if( (mnXfId == rXfIdRange.mnXfId) &&
        (mnNumFmtId == rXfIdRange.mnNumFmtId) &&
        (maRange.EndRow + 1 == rXfIdRange.maRange.StartRow) &&
        (maRange.StartColumn == rXfIdRange.maRange.StartColumn) &&
        (maRange.EndColumn == rXfIdRange.maRange.EndColumn) )
    {
        maRange.EndRow = rXfIdRange.maRange.EndRow;
        return true;
    }
    return false;
}


WorksheetData::MergedRange::MergedRange( const CellRangeAddress& rRange ) :
    maRange( rRange ),
    mnHorAlign( XML_TOKEN_INVALID )
{
}

WorksheetData::MergedRange::MergedRange( const CellAddress& rAddress, sal_Int32 nHorAlign ) :
    maRange( rAddress.Sheet, rAddress.Column, rAddress.Row, rAddress.Column, rAddress.Row ),
    mnHorAlign( nHorAlign )
{
}

bool WorksheetData::MergedRange::tryExpand( const CellAddress& rAddress, sal_Int32 nHorAlign )
{
    if( (mnHorAlign == nHorAlign) && (maRange.StartRow == rAddress.Row) &&
        (maRange.EndRow == rAddress.Row) && (maRange.EndColumn + 1 == rAddress.Column) )
    {
        ++maRange.EndColumn;
        return true;
    }
    return false;
}

void WorksheetData::writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const
{
    StylesBuffer& rStyles = getStyles();
    PropertySet aPropSet( getCellRange( rXfIdRange.maRange ) );
    if( rXfIdRange.mnXfId >= 0 )
        rStyles.writeCellXfToPropertySet( aPropSet, rXfIdRange.mnXfId );
    if( rXfIdRange.mnNumFmtId >= 0 )
        rStyles.writeNumFmtToPropertySet( aPropSet, rXfIdRange.mnNumFmtId );
}

void WorksheetData::mergeXfIdRanges()
{
    if( !maXfIdRanges.empty() )
    {
        // get row index of last range
        sal_Int32 nLastRow = maXfIdRanges.rbegin()->second.maRange.StartRow;
        // process all ranges located in the same row of the last range
        XfIdRangeMap::iterator aMergeIt = maXfIdRanges.end();
        while( (aMergeIt != maXfIdRanges.begin()) && ((--aMergeIt)->second.maRange.StartRow == nLastRow) )
        {
            const XfIdRange& rMergeXfIdRange = aMergeIt->second;
            // try to find a range that can be merged with rMergeRange
            bool bFound = false;
            for( XfIdRangeMap::iterator aIt = maXfIdRanges.begin(); !bFound && (aIt != aMergeIt); ++aIt )
                if( (bFound = aIt->second.tryMerge( rMergeXfIdRange )) == true )
                    maXfIdRanges.erase( aMergeIt++ );
        }
    }
}

void WorksheetData::finalizeXfIdRanges()
{
    // try to merge remaining inserted ranges
    mergeXfIdRanges();
    // write all formatting
    sal_Int32 nLastRow = -1;
    for( XfIdRangeMap::const_iterator aIt = maXfIdRanges.begin(), aEnd = maXfIdRanges.end(); aIt != aEnd; ++aIt )
    {
        writeXfIdRangeProperties( aIt->second );
        if( aIt->first.first > nLastRow )
            lclUpdateProgressBar( mxFinalProgress, maDimension, nLastRow = aIt->first.first );
    }
}

void WorksheetData::finalizeHyperlinkRanges() const
{
    for( OoxHyperlinkList::const_iterator aIt = maHyperlinks.begin(), aEnd = maHyperlinks.end(); aIt != aEnd; ++aIt )
    {
        OUStringBuffer aUrlBuffer;
        if( aIt->maTarget.getLength() > 0 )
            aUrlBuffer.append( getBaseFilter().getAbsoluteUrl( aIt->maTarget ) );
        if( aIt->maLocation.getLength() > 0 )
            aUrlBuffer.append( sal_Unicode( '#' ) ).append( aIt->maLocation );
        OUString aUrl = aUrlBuffer.makeStringAndClear();
        if( aUrl.getLength() > 0 )
        {
            // convert '#SheetName!A1' to '#SheetName.A1'
            if( aUrl[ 0 ] == '#' )
            {
                sal_Int32 nSepPos = aUrl.lastIndexOf( '!' );
                if( nSepPos > 0 )
                {
                    // replace the exclamation mark with a period
                    aUrl = aUrl.replaceAt( nSepPos, 1, OUString( sal_Unicode( '.' ) ) );
                    // #i66592# convert sheet names that have been renamed on import
                    bool bQuotedName = (nSepPos > 3) && (aUrl[ 1 ] == '\'') && (aUrl[ nSepPos - 1 ] == '\'');
                    sal_Int32 nNamePos = bQuotedName ? 2 : 1;
                    sal_Int32 nNameLen = nSepPos - (bQuotedName ? 3 : 1);
                    OUString aSheetName = aUrl.copy( nNamePos, nNameLen );
                    OUString aFinalName = getWorksheets().getFinalSheetName( aSheetName );
                    if( aFinalName.getLength() > 0 )
                        aUrl = aUrl.replaceAt( nNamePos, nNameLen, aFinalName );
                }
            }

            // try to insert URL into each cell of the range
            for( CellAddress aAddress( mnSheet, aIt->maRange.StartColumn, aIt->maRange.StartRow ); aAddress.Row <= aIt->maRange.EndRow; ++aAddress.Row )
                for( aAddress.Column = aIt->maRange.StartColumn; aAddress.Column <= aIt->maRange.EndColumn; ++aAddress.Column )
                    finalizeHyperlink( aAddress, aUrl );
        }
    }
}

void WorksheetData::finalizeHyperlink( const CellAddress& rAddress, const OUString& rUrl ) const
{
    Reference< XCell > xCell = getCell( rAddress );
    if( xCell.is() ) switch( xCell->getType() )
    {
        // #i54261# restrict creation of URL field to text cells
        case ::com::sun::star::table::CellContentType_TEXT:
        {
            Reference< XMultiServiceFactory > xFactory( getDocument(), UNO_QUERY );
            Reference< XText > xText( xCell, UNO_QUERY );
            if( xFactory.is() && xText.is() )
            {
                // create a URL field object and set its properties
                Reference< XTextContent > xUrlField( xFactory->createInstance( maUrlTextField ), UNO_QUERY );
                OSL_ENSURE( xUrlField.is(), "WorksheetData::finalizeHyperlink - cannot create text field" );
                if( xUrlField.is() )
                {
                    // properties of the URL field
                    PropertySet aPropSet( xUrlField );
                    aPropSet.setProperty( maUrlProp, rUrl );
                    aPropSet.setProperty( maReprProp, xText->getString() );
                    try
                    {
                        // insert the field into the cell
                        xText->setString( OUString() );
                        Reference< XTextRange > xRange( xText->createTextCursor(), UNO_QUERY_THROW );
                        xText->insertTextContent( xRange, xUrlField, sal_False );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( false, "WorksheetData::finalizeHyperlink - cannot insert text field" );
                    }
                }
            }
        }
        break;

        // fix for #i31050# disabled, HYPERLINK is not able to return numeric value (#i91351#)
#if 0
        // #i31050# replace number with HYPERLINK function
        case ::com::sun::star::table::CellContentType_VALUE:
        {
            Reference< XFormulaTokens > xTokens( xCell, UNO_QUERY );
            OSL_ENSURE( xTokens.is(), "WorksheetHelper::finalizeHyperlink - missing formula interface" );
            if( xTokens.is() )
            {
                SimpleFormulaContext aContext( xTokens, false, false );
                getFormulaParser().convertNumberToHyperlink( aContext, rUrl, xCell->getValue() );
            }
        }
        break;
#endif

        default:;
    }
}

void WorksheetData::finalizeValidationRanges() const
{
    for( OoxValidationList::const_iterator aIt = maValidations.begin(), aEnd = maValidations.end(); aIt != aEnd; ++aIt )
    {
        PropertySet aPropSet( getCellRangeList( aIt->maRanges ) );

        Reference< XPropertySet > xValidation;
        if( aPropSet.getProperty( xValidation, maValidationProp ) && xValidation.is() )
        {
            PropertySet aValProps( xValidation );
            namespace csss = ::com::sun::star::sheet;

            // convert validation type to API enum
            ValidationType eType = csss::ValidationType_ANY;
            switch( aIt->mnType )
            {
                case XML_custom:        eType = csss::ValidationType_CUSTOM;    break;
                case XML_date:          eType = csss::ValidationType_DATE;      break;
                case XML_decimal:       eType = csss::ValidationType_DECIMAL;   break;
                case XML_list:          eType = csss::ValidationType_LIST;      break;
                case XML_none:          eType = csss::ValidationType_ANY;       break;
                case XML_textLength:    eType = csss::ValidationType_TEXT_LEN;  break;
                case XML_time:          eType = csss::ValidationType_TIME;      break;
                case XML_whole:         eType = csss::ValidationType_WHOLE;     break;
                default:    OSL_ENSURE( false, "WorksheetData::finalizeValidationRanges - unknown validation type" );
            }
            aValProps.setProperty( CREATE_OUSTRING( "Type" ), eType );

            // convert error alert style to API enum
            ValidationAlertStyle eAlertStyle = csss::ValidationAlertStyle_STOP;
            switch( aIt->mnErrorStyle )
            {
                case XML_information:   eAlertStyle = csss::ValidationAlertStyle_INFO;      break;
                case XML_stop:          eAlertStyle = csss::ValidationAlertStyle_STOP;      break;
                case XML_warning:       eAlertStyle = csss::ValidationAlertStyle_WARNING;   break;
                default:    OSL_ENSURE( false, "WorksheetData::finalizeValidationRanges - unknown error style" );
            }
            aValProps.setProperty( CREATE_OUSTRING( "ErrorAlertStyle" ), eAlertStyle );

            // convert dropdown style to API visibility constants
            sal_Int16 nVisibility = aIt->mbNoDropDown ? csss::TableValidationVisibility::INVISIBLE : csss::TableValidationVisibility::UNSORTED;
            aValProps.setProperty( CREATE_OUSTRING( "ShowList" ), nVisibility );

            // messages
            aValProps.setProperty( CREATE_OUSTRING( "ShowInputMessage" ), aIt->mbShowInputMsg );
            aValProps.setProperty( CREATE_OUSTRING( "InputTitle" ), aIt->maInputTitle );
            aValProps.setProperty( CREATE_OUSTRING( "InputMessage" ), aIt->maInputMessage );
            aValProps.setProperty( CREATE_OUSTRING( "ShowErrorMessage" ), aIt->mbShowErrorMsg );
            aValProps.setProperty( CREATE_OUSTRING( "ErrorTitle" ), aIt->maErrorTitle );
            aValProps.setProperty( CREATE_OUSTRING( "ErrorMessage" ), aIt->maErrorMessage );

            // allow blank cells
            aValProps.setProperty( CREATE_OUSTRING( "IgnoreBlankCells" ), aIt->mbAllowBlank );

            try
            {
                // condition operator
                Reference< XSheetCondition > xSheetCond( xValidation, UNO_QUERY_THROW );
                xSheetCond->setOperator( CondFormatBuffer::convertToApiOperator( aIt->mnOperator ) );

                // condition formulas
                Reference< XMultiFormulaTokens > xTokens( xValidation, UNO_QUERY_THROW );
                xTokens->setTokens( 0, aIt->maTokens1 );
                xTokens->setTokens( 1, aIt->maTokens2 );
            }
            catch( Exception& )
            {
            }

            // write back validation settings to cell range(s)
            aPropSet.setProperty( maValidationProp, xValidation );
        }
    }
}

void WorksheetData::finalizeMergedRanges()
{
    MergedRangeList::const_iterator aIt, aEnd;
    for( aIt = maMergedRanges.begin(), aEnd = maMergedRanges.end(); aIt != aEnd; ++aIt )
        finalizeMergedRange( aIt->maRange );
    for( aIt = maCenterFillRanges.begin(), aEnd = maCenterFillRanges.end(); aIt != aEnd; ++aIt )
        finalizeMergedRange( aIt->maRange );
}

void WorksheetData::finalizeMergedRange( const CellRangeAddress& rRange )
{
    bool bMultiCol = rRange.StartColumn < rRange.EndColumn;
    bool bMultiRow = rRange.StartRow < rRange.EndRow;

    if( bMultiCol || bMultiRow ) try
    {
        // merge the cell range
        Reference< XMergeable > xMerge( getCellRange( rRange ), UNO_QUERY_THROW );
        xMerge->merge( sal_True );

        // if merging this range worked (no overlapping merged ranges), update cell borders
        Reference< XCell > xTopLeft = getCell( CellAddress( mnSheet, rRange.StartColumn, rRange.StartRow ) );
        if( xTopLeft.is() )
        {
            PropertySet aTopLeftProp( xTopLeft );

            // copy right border of top-right cell to right border of top-left cell
            if( bMultiCol )
            {
                PropertySet aTopRightProp( getCell( CellAddress( mnSheet, rRange.EndColumn, rRange.StartRow ) ) );
                BorderLine aLine;
                if( aTopRightProp.getProperty( aLine, maRightBorderProp ) )
                    aTopLeftProp.setProperty( maRightBorderProp, aLine );
            }

            // copy bottom border of bottom-left cell to bottom border of top-left cell
            if( bMultiRow )
            {
                PropertySet aBottomLeftProp( getCell( CellAddress( mnSheet, rRange.StartColumn, rRange.EndRow ) ) );
                BorderLine aLine;
                if( aBottomLeftProp.getProperty( aLine, maBottomBorderProp ) )
                    aTopLeftProp.setProperty( maBottomBorderProp, aLine );
            }

            // #i93609# merged range in a single row: test if manual row height is needed
            if( !bMultiRow )
            {
                bool bTextWrap = aTopLeftProp.getBoolProperty( maTextWrapProp );
                if( !bTextWrap && (xTopLeft->getType() == ::com::sun::star::table::CellContentType_TEXT) )
                {
                    Reference< XText > xText( xTopLeft, UNO_QUERY );
                    bTextWrap = xText.is() && (xText->getString().indexOf( '\x0A' ) >= 0);
                }
                if( bTextWrap )
                    maManualRowHeights.insert( rRange.StartRow );
            }
        }
    }
    catch( Exception& )
    {
    }
}

void WorksheetData::finalizeDrawing()
{
    OSL_ENSURE( (getFilterType() == FILTER_OOX) || (maDrawingPath.getLength() == 0),
        "WorksheetData::finalizeDrawing - unexpected DrawingML path" );
    if( (getFilterType() == FILTER_OOX) && (maDrawingPath.getLength() > 0) )
        importOoxFragment( new OoxDrawingFragment( *this, maDrawingPath ) );
}

void WorksheetData::convertColumns()
{
    sal_Int32 nNextCol = 0;
    sal_Int32 nMaxCol = mrMaxApiPos.Column;
    // stores first grouped column index for each level
    OutlineLevelVec aColLevels;

    for( OoxColumnDataMap::const_iterator aIt = maColDatas.begin(), aEnd = maColDatas.end(); aIt != aEnd; ++aIt )
    {
        // convert 1-based OOX column indexes to 0-based API column indexes
        sal_Int32 nFirstCol = ::std::max( aIt->second.mnFirstCol - 1, nNextCol );
        sal_Int32 nLastCol = ::std::min( aIt->second.mnLastCol - 1, nMaxCol );

        // process gap between two column datas, use default column data
        if( nNextCol < nFirstCol )
            convertColumns( aColLevels, nNextCol, nFirstCol - 1, maDefColData );
        // process the column data
        convertColumns( aColLevels, nFirstCol, nLastCol, aIt->second );

        // cache next column to be processed
        nNextCol = nLastCol + 1;
    }

    // remaining default columns to end of sheet
    convertColumns( aColLevels, nNextCol, nMaxCol, maDefColData );
    // close remaining column outlines spanning to end of sheet
    convertOutlines( aColLevels, nMaxCol + 1, 0, false, false );
}

void WorksheetData::convertColumns( OutlineLevelVec& orColLevels,
        sal_Int32 nFirstCol, sal_Int32 nLastCol, const OoxColumnData& rData )
{
    PropertySet aPropSet( getColumns( nFirstCol, nLastCol ) );

    // column width: convert 'number of characters' to column width in 1/100 mm
    sal_Int32 nWidth = getUnitConverter().scaleToMm100( rData.mfWidth, UNIT_DIGIT );
    // macro sheets have double width
    if( meSheetType == SHEETTYPE_MACROSHEET )
        nWidth *= 2;
    if( nWidth > 0 )
        aPropSet.setProperty( maWidthProp, nWidth );

    // hidden columns: TODO: #108683# hide columns later?
    if( rData.mbHidden )
        aPropSet.setProperty( maVisibleProp, false );

    // outline settings for this column range
    convertOutlines( orColLevels, nFirstCol, rData.mnLevel, rData.mbCollapsed, false );
}

void WorksheetData::convertRows()
{
    sal_Int32 nNextRow = 0;
    sal_Int32 nMaxRow = mrMaxApiPos.Row;
    // stores first grouped row index for each level
    OutlineLevelVec aRowLevels;

    for( OoxRowDataMap::const_iterator aIt = maRowDatas.begin(), aEnd = maRowDatas.end(); aIt != aEnd; ++aIt )
    {
        // convert 1-based OOX row indexes to 0-based API row indexes
        sal_Int32 nFirstRow = ::std::max( aIt->second.mnFirstRow - 1, nNextRow );
        sal_Int32 nLastRow = ::std::min( aIt->second.mnLastRow - 1, nMaxRow );

        // process gap between two row datas, use default row data
        if( nNextRow < nFirstRow )
            convertRows( aRowLevels, nNextRow, nFirstRow - 1, maDefRowData );
        // process the row data
        convertRows( aRowLevels, nFirstRow, nLastRow, aIt->second, maDefRowData.mfHeight );

        // cache next row to be processed
        nNextRow = nLastRow + 1;
    }

    // remaining default rows to end of sheet
    convertRows( aRowLevels, nNextRow, nMaxRow, maDefRowData );
    // close remaining row outlines spanning to end of sheet
    convertOutlines( aRowLevels, nMaxRow + 1, 0, false, true );
}

void WorksheetData::convertRows( OutlineLevelVec& orRowLevels,
        sal_Int32 nFirstRow, sal_Int32 nLastRow, const OoxRowData& rData, double fDefHeight )
{
    // row height: convert points to row height in 1/100 mm
    double fHeight = (rData.mfHeight >= 0.0) ? rData.mfHeight : fDefHeight;
    sal_Int32 nHeight = getUnitConverter().scaleToMm100( fHeight, UNIT_POINT );
    if( nHeight > 0 )
    {
        ValueRangeVector aManualRows;
        if( rData.mbCustomHeight )
            aManualRows.push_back( ValueRange( nFirstRow, nLastRow ) );
        else
            maManualRowHeights.intersect( aManualRows, nFirstRow, nLastRow );
        for( ValueRangeVector::const_iterator aIt = aManualRows.begin(), aEnd = aManualRows.end(); aIt != aEnd; ++aIt )
        {
            PropertySet aPropSet( getRows( aIt->mnFirst, aIt->mnLast ) );
            aPropSet.setProperty( maHeightProp, nHeight );
        }
    }

    // hidden rows: TODO: #108683# hide rows later?
    if( rData.mbHidden )
    {
        PropertySet aPropSet( getRows( nFirstRow, nLastRow ) );
        aPropSet.setProperty( maVisibleProp, false );
    }

    // outline settings for this row range
    convertOutlines( orRowLevels, nFirstRow, rData.mnLevel, rData.mbCollapsed, true );
}

void WorksheetData::convertOutlines( OutlineLevelVec& orLevels,
        sal_Int32 nColRow, sal_Int32 nLevel, bool bCollapsed, bool bRows )
{
    /*  It is ensured from caller functions, that this function is called
        without any gaps between the processed column or row ranges. */

    OSL_ENSURE( nLevel >= 0, "WorksheetData::convertOutlines - negative outline level" );
    nLevel = ::std::max< sal_Int32 >( nLevel, 0 );

    sal_Int32 nSize = orLevels.size();
    if( nSize < nLevel )
    {
        // Outline level increased. Push the begin column position.
        for( sal_Int32 nIndex = nSize; nIndex < nLevel; ++nIndex )
            orLevels.push_back( nColRow );
    }
    else if( nLevel < nSize )
    {
        // Outline level decreased. Pop them all out.
        for( sal_Int32 nIndex = nLevel; nIndex < nSize; ++nIndex )
        {
            sal_Int32 nFirstInLevel = orLevels.back();
            orLevels.pop_back();
            groupColumnsOrRows( nFirstInLevel, nColRow - 1, bCollapsed, bRows );
            bCollapsed = false; // collapse only once
        }
    }
}

void WorksheetData::groupColumnsOrRows( sal_Int32 nFirstColRow, sal_Int32 nLastColRow, bool bCollapse, bool bRows )
{
    try
    {
        Reference< XSheetOutline > xOutline( mxSheet, UNO_QUERY_THROW );
        if( bRows )
        {
            CellRangeAddress aRange( mnSheet, 0, nFirstColRow, 0, nLastColRow );
            xOutline->group( aRange, ::com::sun::star::table::TableOrientation_ROWS );
            if( bCollapse )
                xOutline->hideDetail( aRange );
        }
        else
        {
            CellRangeAddress aRange( mnSheet, nFirstColRow, 0, nLastColRow, 0 );
            xOutline->group( aRange, ::com::sun::star::table::TableOrientation_COLUMNS );
            if( bCollapse )
                xOutline->hideDetail( aRange );
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================
// ============================================================================

WorksheetHelper::WorksheetHelper( WorksheetData& rSheetData ) :
    WorkbookHelper( rSheetData ),
    mrSheetData( rSheetData )
{
}

WorksheetType WorksheetHelper::getSheetType() const
{
    return mrSheetData.getSheetType();
}

sal_Int16 WorksheetHelper::getSheetIndex() const
{
    return mrSheetData.getSheetIndex();
}

const Reference< XSpreadsheet >& WorksheetHelper::getXSpreadsheet() const
{
    return mrSheetData.getXSpreadsheet();
}

Reference< XCell > WorksheetHelper::getCell( const CellAddress& rAddress ) const
{
    return mrSheetData.getCell( rAddress );
}

Reference< XCell > WorksheetHelper::getCell( const OUString& rAddressStr, CellAddress* opAddress ) const
{
    CellAddress aAddress;
    if( getAddressConverter().convertToCellAddress( aAddress, rAddressStr, mrSheetData.getSheetIndex(), true ) )
    {
        if( opAddress ) *opAddress = aAddress;
        return mrSheetData.getCell( aAddress );
    }
    return Reference< XCell >();
}

Reference< XCell > WorksheetHelper::getCell( const BinAddress& rBinAddress, CellAddress* opAddress ) const
{
    CellAddress aAddress;
    if( getAddressConverter().convertToCellAddress( aAddress, rBinAddress, mrSheetData.getSheetIndex(), true ) )
    {
        if( opAddress ) *opAddress = aAddress;
        return mrSheetData.getCell( aAddress );
    }
    return Reference< XCell >();
}

Reference< XCellRange > WorksheetHelper::getCellRange( const CellRangeAddress& rRange ) const
{
    return mrSheetData.getCellRange( rRange );
}

Reference< XCellRange > WorksheetHelper::getCellRange( const OUString& rRangeStr, CellRangeAddress* opRange ) const
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rRangeStr, mrSheetData.getSheetIndex(), true ) )
    {
        if( opRange ) *opRange = aRange;
        return mrSheetData.getCellRange( aRange );
    }
    return Reference< XCellRange >();
}

Reference< XCellRange > WorksheetHelper::getCellRange( const BinRange& rBinRange, CellRangeAddress* opRange ) const
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rBinRange, mrSheetData.getSheetIndex(), true ) )
    {
        if( opRange ) *opRange = aRange;
        return mrSheetData.getCellRange( aRange );
    }
    return Reference< XCellRange >();
}

Reference< XSheetCellRanges > WorksheetHelper::getCellRangeList( const ApiCellRangeList& rRanges ) const
{
    return mrSheetData.getCellRangeList( rRanges );
}

Reference< XSheetCellRanges > WorksheetHelper::getCellRangeList(
        const OUString& rRangesStr, ApiCellRangeList* opRanges ) const
{
    ApiCellRangeList aRanges;
    getAddressConverter().convertToCellRangeList( aRanges, rRangesStr, mrSheetData.getSheetIndex(), true );
    if( opRanges ) *opRanges = aRanges;
    return mrSheetData.getCellRangeList( aRanges );
}

Reference< XSheetCellRanges > WorksheetHelper::getCellRangeList(
        const BinRangeList& rBinRanges, ApiCellRangeList* opRanges ) const
{
    ApiCellRangeList aRanges;
    getAddressConverter().convertToCellRangeList( aRanges, rBinRanges, mrSheetData.getSheetIndex(), true );
    if( opRanges ) *opRanges = aRanges;
    return mrSheetData.getCellRangeList( aRanges );
}

CellAddress WorksheetHelper::getCellAddress( const Reference< XCell >& rxCell )
{
    CellAddress aAddress;
    Reference< XCellAddressable > xAddressable( rxCell, UNO_QUERY );
    OSL_ENSURE( xAddressable.is(), "WorksheetHelper::getCellAddress - cell reference not addressable" );
    if( xAddressable.is() )
        aAddress = xAddressable->getCellAddress();
    return aAddress;
}

CellRangeAddress WorksheetHelper::getRangeAddress( const Reference< XCellRange >& rxRange )
{
    CellRangeAddress aRange;
    Reference< XCellRangeAddressable > xAddressable( rxRange, UNO_QUERY );
    OSL_ENSURE( xAddressable.is(), "WorksheetHelper::getRangeAddress - cell range reference not addressable" );
    if( xAddressable.is() )
        aRange = xAddressable->getRangeAddress();
    return aRange;
}

Reference< XCellRange > WorksheetHelper::getColumn( sal_Int32 nCol ) const
{
    return mrSheetData.getColumn( nCol );
}

Reference< XCellRange > WorksheetHelper::getRow( sal_Int32 nRow ) const
{
    return mrSheetData.getRow( nRow );
}

Reference< XTableColumns > WorksheetHelper::getColumns( sal_Int32 nFirstCol, sal_Int32 nLastCol ) const
{
    return mrSheetData.getColumns( nFirstCol, nLastCol );
}

Reference< XTableRows > WorksheetHelper::getRows( sal_Int32 nFirstRow, sal_Int32 nLastRow ) const
{
    return mrSheetData.getRows( nFirstRow, nLastRow );
}

Point WorksheetHelper::getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return mrSheetData.getCellPosition( nCol, nRow );
}

Size WorksheetHelper::getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return mrSheetData.getCellSize( nCol, nRow );
}

Size WorksheetHelper::getDrawPageSize() const
{
    return mrSheetData.getDrawPageSize();
}

WorksheetSettings& WorksheetHelper::getWorksheetSettings() const
{
    return mrSheetData.getWorksheetSettings();
}

SharedFormulaBuffer& WorksheetHelper::getSharedFormulas() const
{
    return mrSheetData.getSharedFormulas();
}

CondFormatBuffer& WorksheetHelper::getCondFormats() const
{
    return mrSheetData.getCondFormats();
}

PageSettings& WorksheetHelper::getPageSettings() const
{
    return mrSheetData.getPageSettings();
}

SheetViewSettings& WorksheetHelper::getSheetViewSettings() const
{
    return mrSheetData.getSheetViewSettings();
}

void WorksheetHelper::setStringCell( const Reference< XCell >& rxCell, const OUString& rText ) const
{
    OSL_ENSURE( rxCell.is(), "WorksheetHelper::setStringCell - missing cell interface" );
    Reference< XText > xText( rxCell, UNO_QUERY );
    if( xText.is() )
        xText->setString( rText );
}

void WorksheetHelper::setSharedStringCell( const Reference< XCell >& rxCell, sal_Int32 nStringId, sal_Int32 nXfId ) const
{
    OSL_ENSURE( rxCell.is(), "WorksheetHelper::setSharedStringCell - missing cell interface" );
    getSharedStrings().convertString( Reference< XText >( rxCell, UNO_QUERY ), nStringId, nXfId );
}

void WorksheetHelper::setBooleanCell( const Reference< XCell >& rxCell, bool bValue ) const
{
    OSL_ENSURE( rxCell.is(), "WorksheetHelper::setBooleanCell - missing cell interface" );
    rxCell->setFormula( mrSheetData.getBooleanFormula( bValue ) );
}

void WorksheetHelper::setErrorCell( const Reference< XCell >& rxCell, const OUString& rErrorCode ) const
{
    setErrorCell( rxCell, getUnitConverter().calcBiffErrorCode( rErrorCode ) );
}

void WorksheetHelper::setErrorCell( const Reference< XCell >& rxCell, sal_uInt8 nErrorCode ) const
{
    Reference< XFormulaTokens > xTokens( rxCell, UNO_QUERY );
    OSL_ENSURE( xTokens.is(), "WorksheetHelper::setErrorCell - missing formula interface" );
    if( xTokens.is() )
    {
        SimpleFormulaContext aContext( xTokens, false, false );
        getFormulaParser().convertErrorToFormula( aContext, nErrorCode );
    }
}

void WorksheetHelper::setOoxCell( OoxCellData& orCellData ) const
{
    OSL_ENSURE( orCellData.mxCell.is(), "WorksheetHelper::setCell - missing cell interface" );
    if( orCellData.mbHasValueStr ) switch( orCellData.mnCellType )
    {
        case XML_b:
            setBooleanCell( orCellData.mxCell, orCellData.maValueStr.toDouble() != 0.0 );
            // #108770# set 'Standard' number format for all Boolean cells
            orCellData.mnNumFmtId = 0;
        break;
        case XML_n:
            orCellData.mxCell->setValue( orCellData.maValueStr.toDouble() );
        break;
        case XML_e:
            setErrorCell( orCellData.mxCell, orCellData.maValueStr );
        break;
        case XML_str:
            setStringCell( orCellData.mxCell, orCellData.maValueStr );
        break;
        case XML_s:
            setSharedStringCell( orCellData.mxCell, orCellData.maValueStr.toInt32(), orCellData.mnXfId );
        break;
    }
}

void WorksheetHelper::setSheetType( WorksheetType eSheetType )
{
    mrSheetData.setSheetType( eSheetType );
}

void WorksheetHelper::setDimension( const CellRangeAddress& rRange )
{
    mrSheetData.setDimension( rRange );
}

void WorksheetHelper::setCellFormat( const OoxCellData& rCellData )
{
    mrSheetData.setCellFormat( rCellData );
}

void WorksheetHelper::setMergedRange( const CellRangeAddress& rRange )
{
    mrSheetData.setMergedRange( rRange );
}

void WorksheetHelper::setPageBreak( const OoxPageBreakData& rData, bool bRowBreak )
{
    mrSheetData.setPageBreak( rData, bRowBreak );
}

void WorksheetHelper::setHyperlink( const OoxHyperlinkData& rHyperlink )
{
    mrSheetData.setHyperlink( rHyperlink );
}

void WorksheetHelper::setValidation( const OoxValidationData& rValData )
{
    mrSheetData.setValidation( rValData );
}

void WorksheetHelper::setTableOperation( const CellRangeAddress& rRange, const OoxDataTableData& rTableData ) const
{
    OSL_ENSURE( getAddressConverter().checkCellRange( rRange, false ), "WorksheetHelper::setTableOperation - invalid range" );
    bool bOk = false;
    if( !rTableData.mbRef1Deleted && (rTableData.maRef1.getLength() > 0) && (rRange.StartColumn > 0) && (rRange.StartRow > 0) )
    {
        CellRangeAddress aOpRange = rRange;
        CellAddress aRef1, aRef2;
        if( getAddressConverter().convertToCellAddress( aRef1, rTableData.maRef1, mrSheetData.getSheetIndex(), true ) ) try
        {
            if( rTableData.mb2dTable )
            {
                if( !rTableData.mbRef2Deleted && getAddressConverter().convertToCellAddress( aRef2, rTableData.maRef2, mrSheetData.getSheetIndex(), true ) )
                {
                    // API call expects input values inside operation range
                    --aOpRange.StartColumn;
                    --aOpRange.StartRow;
                    // formula range is top-left cell of operation range
                    CellRangeAddress aFormulaRange( mrSheetData.getSheetIndex(), aOpRange.StartColumn, aOpRange.StartRow, aOpRange.StartColumn, aOpRange.StartRow );
                    // set multiple operation
                    Reference< XMultipleOperation > xMultOp( mrSheetData.getCellRange( aOpRange ), UNO_QUERY_THROW );
                    xMultOp->setTableOperation( aFormulaRange, ::com::sun::star::sheet::TableOperationMode_BOTH, aRef2, aRef1 );
                    bOk = true;
                }
            }
            else if( rTableData.mbRowTable )
            {
                // formula range is column to the left of operation range
                CellRangeAddress aFormulaRange( mrSheetData.getSheetIndex(), aOpRange.StartColumn - 1, aOpRange.StartRow, aOpRange.StartColumn - 1, aOpRange.EndRow );
                // API call expects input values (top row) inside operation range
                --aOpRange.StartRow;
                // set multiple operation
                Reference< XMultipleOperation > xMultOp( mrSheetData.getCellRange( aOpRange ), UNO_QUERY_THROW );
                xMultOp->setTableOperation( aFormulaRange, ::com::sun::star::sheet::TableOperationMode_ROW, aRef1, aRef1 );
                bOk = true;
            }
            else
            {
                // formula range is row above operation range
                CellRangeAddress aFormulaRange( mrSheetData.getSheetIndex(), aOpRange.StartColumn, aOpRange.StartRow - 1, aOpRange.EndColumn, aOpRange.StartRow - 1 );
                // API call expects input values (left column) inside operation range
                --aOpRange.StartColumn;
                // set multiple operation
                Reference< XMultipleOperation > xMultOp( mrSheetData.getCellRange( aOpRange ), UNO_QUERY_THROW );
                xMultOp->setTableOperation( aFormulaRange, ::com::sun::star::sheet::TableOperationMode_COLUMN, aRef1, aRef1 );
                bOk = true;
            }
        }
        catch( Exception& )
        {
        }
    }

    // on error: fill cell range with error codes
    if( !bOk )
    {
        for( CellAddress aPos( mrSheetData.getSheetIndex(), rRange.StartColumn, rRange.StartRow ); aPos.Row <= rRange.EndRow; ++aPos.Row )
            for( aPos.Column = rRange.StartColumn; aPos.Column <= rRange.EndColumn; ++aPos.Column )
                setErrorCell( mrSheetData.getCell( aPos ), BIFF_ERR_REF );
    }
}

void WorksheetHelper::setLabelRanges( const ApiCellRangeList& rColRanges, const ApiCellRangeList& rRowRanges )
{
    const CellAddress& rMaxPos = getAddressConverter().getMaxApiAddress();
    Reference< XLabelRanges > xLabelRanges;
    PropertySet aPropSet( getXSpreadsheet() );

    if( !rColRanges.empty() && aPropSet.getProperty( xLabelRanges, CREATE_OUSTRING( "ColumnLabelRanges" ) ) && xLabelRanges.is() )
    {
        for( ApiCellRangeList::const_iterator aIt = rColRanges.begin(), aEnd = rColRanges.end(); aIt != aEnd; ++aIt )
        {
            CellRangeAddress aDataRange = *aIt;
            if( aDataRange.EndRow < rMaxPos.Row )
            {
                aDataRange.StartRow = aDataRange.EndRow + 1;
                aDataRange.EndRow = rMaxPos.Row;
            }
            else if( aDataRange.StartRow > 0 )
            {
                aDataRange.EndRow = aDataRange.StartRow - 1;
                aDataRange.StartRow = 0;
            }
            xLabelRanges->addNew( *aIt, aDataRange );
        }
    }

    if( !rRowRanges.empty() && aPropSet.getProperty( xLabelRanges, CREATE_OUSTRING( "RowLabelRanges" ) ) && xLabelRanges.is() )
    {
        for( ApiCellRangeList::const_iterator aIt = rRowRanges.begin(), aEnd = rRowRanges.end(); aIt != aEnd; ++aIt )
        {
            CellRangeAddress aDataRange = *aIt;
            if( aDataRange.EndColumn < rMaxPos.Column )
            {
                aDataRange.StartColumn = aDataRange.EndColumn + 1;
                aDataRange.EndColumn = rMaxPos.Column;
            }
            else if( aDataRange.StartColumn > 0 )
            {
                aDataRange.EndColumn = aDataRange.StartColumn - 1;
                aDataRange.StartColumn = 0;
            }
            xLabelRanges->addNew( *aIt, aDataRange );
        }
    }
}

void WorksheetHelper::setDrawingPath( const OUString& rDrawingPath )
{
    mrSheetData.setDrawingPath( rDrawingPath );
}

void WorksheetHelper::setVmlDrawingPath( const OUString& rVmlDrawingPath )
{
    mrSheetData.setVmlDrawingPath( rVmlDrawingPath );
}

void WorksheetHelper::setOleObject( const OoxOleObjectData& rOleObjectData )
{
    mrSheetData.setOleObject( rOleObjectData );
}

void WorksheetHelper::setFormControl( const OoxFormControlData& rFormControlData )
{
    mrSheetData.setFormControl( rFormControlData );
}

void WorksheetHelper::setBaseColumnWidth( sal_Int32 nWidth )
{
    mrSheetData.setBaseColumnWidth( nWidth );
}

void WorksheetHelper::setDefaultColumnWidth( double fWidth )
{
    mrSheetData.setDefaultColumnWidth( fWidth );
}

void WorksheetHelper::setColumnData( const OoxColumnData& rData )
{
    mrSheetData.setColumnData( rData );
}

void WorksheetHelper::setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom )
{
    mrSheetData.setDefaultRowSettings( fHeight, bCustomHeight, bHidden, bThickTop, bThickBottom );
}

void WorksheetHelper::setRowData( const OoxRowData& rData )
{
    mrSheetData.setRowData( rData );
}

void WorksheetHelper::convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId )
{
    mrSheetData.convertColumnFormat( nFirstCol, nLastCol, nXfId );
}

void WorksheetHelper::convertRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId )
{
    mrSheetData.convertRowFormat( nFirstRow, nLastRow, nXfId );
}

void WorksheetHelper::initializeWorksheetImport()
{
    mrSheetData.initializeWorksheetImport();
}

void WorksheetHelper::finalizeWorksheetImport()
{
    mrSheetData.finalizeWorksheetImport();
}

// ============================================================================

namespace prv {

WorksheetDataOwner::WorksheetDataOwner( WorksheetDataRef xSheetData ) :
    mxSheetData( xSheetData )
{
}

WorksheetDataOwner::~WorksheetDataOwner()
{
}

} // namespace prv

// ----------------------------------------------------------------------------

WorksheetHelperRoot::WorksheetHelperRoot( const WorkbookHelper& rHelper, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    prv::WorksheetDataOwner( prv::WorksheetDataRef( new WorksheetData( rHelper, xProgressBar, eSheetType, nSheet ) ) ),
    WorksheetHelper( *mxSheetData )
{
}

WorksheetHelperRoot::WorksheetHelperRoot( const WorksheetHelper& rHelper ) :
    prv::WorksheetDataOwner( prv::WorksheetDataRef() ),
    WorksheetHelper( rHelper )
{
}

WorksheetHelperRoot::WorksheetHelperRoot( const WorksheetHelperRoot& rHelper ) :
    prv::WorksheetDataOwner( rHelper.mxSheetData ),
    WorksheetHelper( rHelper )
{
}

bool WorksheetHelperRoot::isValidSheet() const
{
    return mxSheetData->isValidSheet();
}

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox

