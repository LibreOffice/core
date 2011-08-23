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

#include "oox/xls/worksheethelper.hxx"
#include <algorithm>
#include <utility>
#include <list>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include "properties.hxx"
#include "tokens.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/commentsbuffer.hxx"
#include "oox/xls/condformatbuffer.hxx"
#include "oox/xls/drawingfragment.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/sharedformulabuffer.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/unitconverter.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"
#include "oox/xls/worksheetsettings.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XDrawPageSupplier;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::lang::XMultiServiceFactory;
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
using ::com::sun::star::table::BorderLine;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::XCell;
using ::com::sun::star::table::XCellRange;
using ::com::sun::star::table::XColumnRowRange;
using ::com::sun::star::table::XTableColumns;
using ::com::sun::star::table::XTableRows;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::util::XMergeable;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::util::XNumberFormatTypes;

namespace oox {
namespace xls {

// ============================================================================

namespace {

void lclUpdateProgressBar( ISegmentProgressBarRef xProgressBar, const CellRangeAddress& rUsedArea, sal_Int32 nRow )
{
    if( xProgressBar.get() && (rUsedArea.StartRow <= nRow) && (nRow <= rUsedArea.EndRow) )
    {
        double fPosition = static_cast< double >( nRow - rUsedArea.StartRow + 1 ) / (rUsedArea.EndRow - rUsedArea.StartRow + 1);
        if( xProgressBar->getPosition() < fPosition )
            xProgressBar->setPosition( fPosition );
    }
}

void lclUpdateProgressBar( ISegmentProgressBarRef xProgressBar, double fPosition )
{
    if( xProgressBar.get() )
        xProgressBar->setPosition( fPosition );
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

void CellModel::reset()
{
    mxCell.clear();
    maValueStr = maFormulaRef = OUString();
    mnCellType = mnFormulaType = XML_TOKEN_INVALID;
    mnSharedId = mnXfId = mnNumFmtId = -1;
    mbHasValueStr = mbShowPhonetic = false;
}

// ----------------------------------------------------------------------------

DataTableModel::DataTableModel() :
    mb2dTable( false ),
    mbRowTable( false ),
    mbRef1Deleted( false ),
    mbRef2Deleted( false )
{
}

// ----------------------------------------------------------------------------

ColumnModel::ColumnModel() :
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

bool ColumnModel::tryExpand( const ColumnModel& rModel )
{
    bool bExpandable =
        (mnFirstCol        <= rModel.mnFirstCol) &&
        (rModel.mnFirstCol <= mnLastCol + 1) &&
        (mfWidth           == rModel.mfWidth) &&
        // ignore mnXfId, cell formatting is always set directly
        (mnLevel           == rModel.mnLevel) &&
        (mbHidden          == rModel.mbHidden) &&
        (mbCollapsed       == rModel.mbCollapsed);

    if( bExpandable )
        mnLastCol = rModel.mnLastCol;
    return bExpandable;
}

// ----------------------------------------------------------------------------

RowModel::RowModel() :
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

bool RowModel::tryExpand( const RowModel& rModel )
{
    bool bExpandable =
        (mnFirstRow        <= rModel.mnFirstRow) &&
        (rModel.mnFirstRow <= mnLastRow + 1) &&
        (mfHeight          == rModel.mfHeight) &&
        // ignore mnXfId, mbCustomFormat, mbShowPhonetic - cell formatting is always set directly
        (mnLevel           == rModel.mnLevel) &&
        (mbCustomHeight    == rModel.mbCustomHeight) &&
        (mbHidden          == rModel.mbHidden) &&
        (mbCollapsed       == rModel.mbCollapsed);

    if( bExpandable )
        mnLastRow = rModel.mnLastRow;
    return bExpandable;
}

// ----------------------------------------------------------------------------

PageBreakModel::PageBreakModel() :
    mnColRow( 0 ),
    mbManual( false )
{
}

// ----------------------------------------------------------------------------

HyperlinkModel::HyperlinkModel()
{
}

// ----------------------------------------------------------------------------

ValidationModel::ValidationModel() :
    mnType( XML_none ),
    mnOperator( XML_between ),
    mnErrorStyle( XML_stop ),
    mbShowInputMsg( false ),
    mbShowErrorMsg( false ),
    mbNoDropDown( false ),
    mbAllowBlank( false )
{
}

void ValidationModel::setBinType( sal_uInt8 nType )
{
    static const sal_Int32 spnTypeIds[] = {
        XML_none, XML_whole, XML_decimal, XML_list, XML_date, XML_time, XML_textLength, XML_custom };
    mnType = STATIC_ARRAY_SELECT( spnTypeIds, nType, XML_none );
}

void ValidationModel::setBinOperator( sal_uInt8 nOperator )
{
    static const sal_Int32 spnOperators[] = {
        XML_between, XML_notBetween, XML_equal, XML_notEqual,
        XML_greaterThan, XML_lessThan, XML_greaterThanOrEqual, XML_lessThanOrEqual };
    mnOperator = STATIC_ARRAY_SELECT( spnOperators, nOperator, XML_TOKEN_INVALID );
}

void ValidationModel::setBinErrorStyle( sal_uInt8 nErrorStyle )
{
    static const sal_Int32 spnErrorStyles[] = { XML_stop, XML_warning, XML_information };
    mnErrorStyle = STATIC_ARRAY_SELECT( spnErrorStyles, nErrorStyle, XML_stop );
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
                            sal_Int16 nSheet );

    /** Returns true, if this helper refers to an existing Calc sheet. */
    inline bool         isValidSheet() const { return mxSheet.is(); }

    /** Returns a cell formula simulating the passed boolean value. */
    const OUString&     getBooleanFormula( bool bValue ) const;

    /** Returns the type of this sheet. */
    inline WorksheetType getSheetType() const { return meSheetType; }
    /** Returns the index of the current sheet. */
    inline sal_Int16    getSheetIndex() const { return maUsedArea.Sheet; }
    /** Returns the XSpreadsheet interface of the current sheet. */
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >&
                        getSheet() const { return mxSheet; }

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

    /** Returns the XDrawPage interface of the draw page of the current sheet. */
    Reference< XDrawPage > getDrawPage() const;
    /** Returns the size of the entire drawing page in 1/100 mm. */
    Size                getDrawPageSize() const;

    /** Returns the absolute position of the top-left corner of the cell in 1/100 mm. */
    Point               getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the size of the cell in 1/100 mm. */
    Size                getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const;

    /** Returns the address of the cell that contains the passed point in 1/100 mm. */
    CellAddress         getCellAddressFromPosition( const Point& rPosition, const CellAddress* pStartAddr = 0 ) const;
    /** Returns the cell range address that contains the passed rectangle in 1/100 mm. */
    CellRangeAddress    getCellRangeFromRectangle( const Rectangle& rRect ) const;

    /** Returns the worksheet settings object. */
    inline WorksheetSettings& getWorksheetSettings() { return maSheetSett; }
    /** Returns the buffer containing all shared formulas in this sheet. */
    inline SharedFormulaBuffer& getSharedFormulas() { return maSharedFmlas; }
    /** Returns the conditional formattings in this sheet. */
    inline CondFormatBuffer& getCondFormats() { return maCondFormats; }
    /** Returns the buffer for all cell comments in this sheet. */
    inline CommentsBuffer& getComments() { return maComments; }
    /** Returns the page/print settings for this sheet. */
    inline PageSettings& getPageSettings() { return maPageSett; }
    /** Returns the view settings for this sheet. */
    inline SheetViewSettings& getSheetViewSettings() { return maSheetViewSett; }
    /** Returns the VML drawing page for this sheet (OOX only!). */
    inline VmlDrawing&  getVmlDrawing() { return *mxVmlDrawing; }

    /** Changes the current sheet type. */
    inline void         setSheetType( WorksheetType eSheetType ) { meSheetType = eSheetType; }
    /** Stores the cell format at the passed address. */
    void                setCellFormat( const CellModel& rModel );
    /** Merges the cells in the passed cell range. */
    void                setMergedRange( const CellRangeAddress& rRange );
    /** Sets a column or row page break described in the passed struct. */
    void                setPageBreak( const PageBreakModel& rModel, bool bRowBreak );
    /** Inserts the hyperlink URL into the spreadsheet. */
    void                setHyperlink( const HyperlinkModel& rModel );
    /** Inserts the data validation settings into the spreadsheet. */
    void                setValidation( const ValidationModel& rModel );
    /** Sets the path to the DrawingML fragment of this sheet. */
    void                setDrawingPath( const OUString& rDrawingPath );
    /** Sets the path to the legacy VML drawing fragment of this sheet. */
    void                setVmlDrawingPath( const OUString& rVmlDrawingPath );

    /** Extends the used area of this sheet by the passed cell position. */
    void                extendUsedArea( const CellAddress& rAddress );
    /** Extends the used area of this sheet by the passed cell range. */
    void                extendUsedArea( const CellRangeAddress& rRange );
    /** Extends the shape bounding box by the position and size of the passed rectangle. */
    void                extendShapeBoundingBox( const Rectangle& rShapeRect );

    /** Sets base width for all columns (without padding pixels). This value
        is only used, if base width has not been set with setDefaultColumnWidth(). */
    void                setBaseColumnWidth( sal_Int32 nWidth );
    /** Sets default width for all columns. This function overrides the base
        width set with the setBaseColumnWidth() function. */
    void                setDefaultColumnWidth( double fWidth );
    /** Sets column settings for a specific column range.
        @descr  Column default formatting is converted directly, other settings
        are cached and converted in the finalizeImport() call. */
    void                setColumnModel( const ColumnModel& rModel );

    /** Sets default height and hidden state for all unused rows in the sheet. */
    void                setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom );
    /** Sets row settings for a specific row.
        @descr  Row default formatting is converted directly, other settings
        are cached and converted in the finalizeImport() call. */
    void                setRowModel( const RowModel& rModel );

    /** Converts column default cell formatting. */
    void                convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId ) const;
    /** Converts row default cell formatting. */
    void                convertRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId ) const;

    /** Initial conversion before importing the worksheet. */
    void                initializeWorksheetImport();
    /** Final conversion after importing the worksheet. */
    void                finalizeWorksheetImport();

private:
    typedef ::std::vector< sal_Int32 >                  OutlineLevelVec;
    typedef ::std::map< sal_Int32, ColumnModel >        ColumnModelMap;
    typedef ::std::map< sal_Int32, RowModel >           RowModelMap;
    typedef ::std::list< HyperlinkModel >               HyperlinkModelList;
    typedef ::std::list< ValidationModel >              ValidationModelList;

    struct XfIdRowRange
    {
        sal_Int32           mnFirstRow;         /// Index of first row.
        sal_Int32           mnLastRow;          /// Index of last row.
        sal_Int32           mnXfId;             /// XF identifier for the row range.

        explicit            XfIdRowRange();
        bool                intersects( const CellRangeAddress& rRange ) const;
        void                set( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId );
        bool                tryExpand( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId );
    };

    struct XfIdRange
    {
        CellRangeAddress    maRange;            /// The formatted cell range.
        sal_Int32           mnXfId;             /// XF identifier for the range.
        sal_Int32           mnNumFmtId;         /// Number format id overriding the XF.

        void                set( const CellModel& rModel );
        bool                tryExpand( const CellModel& rModel );
        bool                tryMerge( const XfIdRange& rXfIdRange );
    };

    struct MergedRange
    {
        CellRangeAddress    maRange;            /// The formatted cell range.
        sal_Int32           mnHorAlign;         /// Horizontal alignment in the range.

        explicit            MergedRange( const CellRangeAddress& rRange );
        explicit            MergedRange( const CellAddress& rAddress, sal_Int32 nHorAlign );
        bool                tryExpand( const CellAddress& rAddress, sal_Int32 nHorAlign );
    };

    typedef ::std::pair< sal_Int32, sal_Int32 > RowColKey;
    typedef ::std::map< RowColKey, XfIdRange >  XfIdRangeMap;
    typedef ::std::list< MergedRange >          MergedRangeList;

    /** Writes all cell formatting attributes to the passed row range. */
    void                writeXfIdRowRangeProperties( const XfIdRowRange& rXfIdRowRange ) const;
    /** Writes all cell formatting attributes to the passed cell range. */
    void                writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const;
    /** Tries to merge the ranges last inserted in maXfIdRanges with existing ranges. */
    void                mergeXfIdRanges();
    /** Finalizes the remaining ranges in maXfIdRanges. */
    void                finalizeXfIdRanges();

    /** Inserts all imported hyperlinks into their cell ranges. */
    void                finalizeHyperlinkRanges() const;
    /** Generates the final URL for the passed hyperlink. */
    OUString            getHyperlinkUrl( const HyperlinkModel& rHyperlink ) const;
    /** Inserts a hyperlinks into the specified cell. */
    void                insertHyperlink( const CellAddress& rAddress, const OUString& rUrl ) const;

    /** Inserts all imported data validations into their cell ranges. */
    void                finalizeValidationRanges() const;

    /** Merges all cached merged ranges and updates right/bottom cell borders. */
    void                finalizeMergedRanges();
    /** Merges the passed merged range and updates right/bottom cell borders. */
    void                finalizeMergedRange( const CellRangeAddress& rRange );

    /** Imports the drawing layer of the sheet (DrawingML part). */
    void                finalizeDrawing();
    /** Imports the drawing layer of the sheet (VML part). */
    void                finalizeVmlDrawing();
    /** Extends the used cell area with the area used by drawing objects. */
    void                finalizeUsedArea();

    /** Converts column properties for all columns in the sheet. */
    void                convertColumns();
    /** Converts column properties. */
    void                convertColumns( OutlineLevelVec& orColLevels, sal_Int32 nFirstCol, sal_Int32 nLastCol, const ColumnModel& rModel );

    /** Converts row properties for all rows in the sheet. */
    void                convertRows();
    /** Converts row properties. */
    void                convertRows( OutlineLevelVec& orRowLevels, sal_Int32 nFirstRow, sal_Int32 nLastRow, const RowModel& rModel, double fDefHeight = -1.0 );

    /** Converts outline grouping for the passed column or row. */
    void                convertOutlines( OutlineLevelVec& orLevels, sal_Int32 nColRow, sal_Int32 nLevel, bool bCollapsed, bool bRows );
    /** Groups columns or rows for the given range. */
    void                groupColumnsOrRows( sal_Int32 nFirstColRow, sal_Int32 nLastColRow, bool bCollapsed, bool bRows );

private:
    typedef ::std::auto_ptr< VmlDrawing > VmlDrawingPtr;

    const OUString      maTrueFormula;      /// Replacement formula for TRUE boolean cells.
    const OUString      maFalseFormula;     /// Replacement formula for FALSE boolean cells.
    const OUString      maSheetCellRanges;  /// Service name for a SheetCellRanges object.
    const OUString      maUrlTextField;     /// Service name for a URL text field.
    const CellAddress&  mrMaxApiPos;        /// Reference to maximum Calc cell address from address converter.
    CellRangeAddress    maUsedArea;         /// Used area of the sheet, and sheet index of the sheet.
    ColumnModel         maDefColModel;      /// Default column formatting.
    ColumnModelMap      maColModels;        /// Columns sorted by first column index.
    RowModel            maDefRowModel;      /// Default row formatting.
    RowModelMap         maRowModels;        /// Rows sorted by row index.
    HyperlinkModelList  maHyperlinks;       /// Cell ranges containing hyperlinks.
    ValidationModelList maValidations;      /// Cell ranges containing data validation settings.
    XfIdRowRange        maXfIdRowRange;     /// Cached XF identifier for a range of rows.
    XfIdRangeMap        maXfIdRanges;       /// Collected XF identifiers for cell ranges.
    MergedRangeList     maMergedRanges;     /// Merged cell ranges.
    MergedRangeList     maCenterFillRanges; /// Merged cell ranges from 'center across' or 'fill' alignment.
    ValueRangeSet       maManualRowHeights; /// Rows that need manual height independent from own settings.
    WorksheetSettings   maSheetSett;        /// Global settings for this sheet.
    SharedFormulaBuffer maSharedFmlas;      /// Buffer for shared formulas in this sheet.
    CondFormatBuffer    maCondFormats;      /// Buffer for conditional formattings.
    CommentsBuffer      maComments;         /// Buffer for all cell comments in this sheet.
    PageSettings        maPageSett;         /// Page/print settings for this sheet.
    SheetViewSettings   maSheetViewSett;    /// View settings for this sheet.
    VmlDrawingPtr       mxVmlDrawing;       /// Collection of all VML shapes.
    OUString            maDrawingPath;      /// Path to DrawingML fragment.
    OUString            maVmlDrawingPath;   /// Path to legacy VML drawing fragment.
    Rectangle           maShapeBoundingBox; /// Bounding box for all shapes from all drawings.
    ISegmentProgressBarRef mxProgressBar;   /// Sheet progress bar.
    ISegmentProgressBarRef mxRowProgress;   /// Progress bar for row/cell processing.
    ISegmentProgressBarRef mxFinalProgress; /// Progress bar for finalization.
    WorksheetType       meSheetType;        /// Type of this sheet.
    Reference< XSpreadsheet > mxSheet;      /// Reference to the current sheet.
    bool                mbHasDefWidth;      /// True = default column width is set from defaultColWidth attribute.
};

// ----------------------------------------------------------------------------

WorksheetData::WorksheetData( const WorkbookHelper& rHelper, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    WorkbookHelper( rHelper ),
    maTrueFormula( CREATE_OUSTRING( "=TRUE()" ) ),
    maFalseFormula( CREATE_OUSTRING( "=FALSE()" ) ),
    maSheetCellRanges( CREATE_OUSTRING( "com.sun.star.sheet.SheetCellRanges" ) ),
    maUrlTextField( CREATE_OUSTRING( "com.sun.star.text.TextField.URL" ) ),
    mrMaxApiPos( rHelper.getAddressConverter().getMaxApiAddress() ),
    maUsedArea( nSheet, SAL_MAX_INT32, SAL_MAX_INT32, -1, -1 ),
    maSheetSett( *this ),
    maSharedFmlas( *this ),
    maCondFormats( *this ),
    maComments( *this ),
    maPageSett( *this ),
    maSheetViewSett( *this ),
    mxProgressBar( xProgressBar ),
    meSheetType( eSheetType ),
    mbHasDefWidth( false )
{
    mxSheet = getSheetFromDoc( nSheet );
    if( !mxSheet.is() )
        maUsedArea.Sheet = -1;

    // default column settings (width and hidden state may be updated later)
    maDefColModel.mfWidth = 8.5;
    maDefColModel.mnXfId = -1;
    maDefColModel.mnLevel = 0;
    maDefColModel.mbHidden = false;
    maDefColModel.mbCollapsed = false;

    // default row settings (height and hidden state may be updated later)
    maDefRowModel.mfHeight = 0.0;
    maDefRowModel.mnXfId = -1;
    maDefRowModel.mnLevel = 0;
    maDefRowModel.mbCustomHeight = false;
    maDefRowModel.mbCustomFormat = false;
    maDefRowModel.mbShowPhonetic = false;
    maDefRowModel.mbHidden = false;
    maDefRowModel.mbCollapsed = false;

    // buffers
    if( getFilterType() == FILTER_OOX )
        mxVmlDrawing.reset( new VmlDrawing( *this ) );

    // prepare progress bars
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
        xRanges.set( getDocumentFactory()->createInstance( maSheetCellRanges ), UNO_QUERY_THROW );
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
        Reference< XTableColumns > xColumns( xColRowRange->getColumns(), UNO_SET_THROW );
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
        Reference< XTableRows > xRows( xColRowRange->getRows(), UNO_SET_THROW );
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
        Reference< XColumnRowRange > xRange( getCellRange( CellRangeAddress( getSheetIndex(), nFirstCol, 0, nLastCol, 0 ) ), UNO_QUERY );
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
        Reference< XColumnRowRange > xRange( getCellRange( CellRangeAddress( getSheetIndex(), 0, nFirstRow, 0, nLastRow ) ), UNO_QUERY );
        if( xRange.is() )
            xRows = xRange->getRows();
    }
    return xRows;
}

Reference< XDrawPage > WorksheetData::getDrawPage() const
{
    Reference< XDrawPage > xDrawPage;
    try
    {
        xDrawPage = Reference< XDrawPageSupplier >( mxSheet, UNO_QUERY_THROW )->getDrawPage();
    }
    catch( Exception& )
    {
    }
    return xDrawPage;
}

Size WorksheetData::getDrawPageSize() const
{
    Size aSize;
    PropertySet aRangeProp( getCellRange( CellRangeAddress( getSheetIndex(), 0, 0, mrMaxApiPos.Column, mrMaxApiPos.Row ) ) );
    aRangeProp.getProperty( aSize, PROP_Size );
    return aSize;
}

Point WorksheetData::getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const
{
    Point aPoint;
    PropertySet aCellProp( getCell( CellAddress( getSheetIndex(), nCol, nRow ) ) );
    aCellProp.getProperty( aPoint, PROP_Position );
    return aPoint;
}

Size WorksheetData::getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const
{
    Size aSize;
    PropertySet aCellProp( getCell( CellAddress( getSheetIndex(), nCol, nRow ) ) );
    aCellProp.getProperty( aSize, PROP_Size );
    return aSize;
}

CellAddress WorksheetData::getCellAddressFromPosition( const Point& rPosition, const CellAddress* pStartAddr ) const
{
    // prepare start address for search loop
    sal_Int32 nCol = pStartAddr ? ::std::min< sal_Int32 >( pStartAddr->Column + 1, mrMaxApiPos.Column ) : 1;
    sal_Int32 nRow = pStartAddr ? ::std::min< sal_Int32 >( pStartAddr->Row + 1,    mrMaxApiPos.Row )    : 1;

    /*  The loop will find the column/row index of the cell right of/below
        the cell containing the passed point, unless the point is located at
        the top or left border of the containing cell. */
    bool bNextCol = true;
    bool bNextRow = true;
    Point aCellPos;
    do
    {
        aCellPos = getCellPosition( nCol, nRow );
        if( bNextCol && ((bNextCol = (aCellPos.X < rPosition.X) && (nCol < mrMaxApiPos.Column)) == true) )
            ++nCol;
        if( bNextRow && ((bNextRow = (aCellPos.Y < rPosition.Y) && (nRow < mrMaxApiPos.Row)) == true) )
            ++nRow;
    }
    while( bNextCol || bNextRow );

    /*  The cell left of/above the current search position contains the passed
        point, unless the point is located on the top/left border of the cell,
        or the last column/row of the sheet has been reached. */
    if( aCellPos.X > rPosition.X ) --nCol;
    if( aCellPos.Y > rPosition.Y ) --nRow;
    return CellAddress( getSheetIndex(), nCol, nRow );
}

CellRangeAddress WorksheetData::getCellRangeFromRectangle( const Rectangle& rRect ) const
{
    CellAddress aStartAddr = getCellAddressFromPosition( Point( rRect.X, rRect.Y ) );
    Point aBotRight( rRect.X + rRect.Width, rRect.Y + rRect.Height );
    CellAddress aEndAddr = getCellAddressFromPosition( aBotRight );
    bool bMultiCols = aStartAddr.Column < aEndAddr.Column;
    bool bMultiRows = aStartAddr.Row < aEndAddr.Row;
    if( bMultiCols || bMultiRows )
    {
        /*  Reduce end position of the cell range to previous column or row, if
            the rectangle ends exactly between two columns or rows. */
        Point aEndPos = getCellPosition( aEndAddr.Column, aEndAddr.Row );
        if( bMultiCols && (aBotRight.X <= aEndPos.X) )
            --aEndAddr.Column;
        if( bMultiRows && (aBotRight.Y <= aEndPos.Y) )
            --aEndAddr.Row;
    }
    return CellRangeAddress( getSheetIndex(), aStartAddr.Column, aStartAddr.Row, aEndAddr.Column, aEndAddr.Row );
}

void WorksheetData::setCellFormat( const CellModel& rModel )
{
    if( rModel.mxCell.is() && ((rModel.mnXfId >= 0) || (rModel.mnNumFmtId >= 0)) )
    {
        // try to merge existing ranges and to write some formatting properties
        if( !maXfIdRanges.empty() )
        {
            // get row index of last inserted cell
            sal_Int32 nLastRow = maXfIdRanges.rbegin()->second.maRange.StartRow;
            // row changed - try to merge ranges of last row with existing ranges
            if( rModel.maAddress.Row != nLastRow )
            {
                mergeXfIdRanges();
                // write format properties of all ranges above last row and remove them
                XfIdRangeMap::iterator aIt = maXfIdRanges.begin(), aEnd = maXfIdRanges.end();
                while( aIt != aEnd )
                {
                    // check that range cannot be merged with current row, and that range is not in cached row range
                    if( (aIt->second.maRange.EndRow < nLastRow) && !maXfIdRowRange.intersects( aIt->second.maRange ) )
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
        if( maXfIdRanges.empty() || !maXfIdRanges.rbegin()->second.tryExpand( rModel ) )
            maXfIdRanges[ RowColKey( rModel.maAddress.Row, rModel.maAddress.Column ) ].set( rModel );

        // update merged ranges for 'center across selection' and 'fill'
        if( const Xf* pXf = getStyles().getCellXf( rModel.mnXfId ).get() )
        {
            sal_Int32 nHorAlign = pXf->getAlignment().getModel().mnHorAlign;
            if( (nHorAlign == XML_centerContinuous) || (nHorAlign == XML_fill) )
            {
                /*  start new merged range, if cell is not empty (#108781#),
                    or try to expand last range with empty cell */
                if( rModel.mnCellType != XML_TOKEN_INVALID )
                    maCenterFillRanges.push_back( MergedRange( rModel.maAddress, nHorAlign ) );
                else if( !maCenterFillRanges.empty() )
                    maCenterFillRanges.rbegin()->tryExpand( rModel.maAddress, nHorAlign );
            }
        }
    }
}

void WorksheetData::setMergedRange( const CellRangeAddress& rRange )
{
    maMergedRanges.push_back( MergedRange( rRange ) );
}

void WorksheetData::setPageBreak( const PageBreakModel& rModel, bool bRowBreak )
{
    if( rModel.mbManual && (rModel.mnColRow > 0) )
    {
        PropertySet aPropSet( bRowBreak ? getRow( rModel.mnColRow ) : getColumn( rModel.mnColRow ) );
        aPropSet.setProperty( PROP_IsStartOfNewPage, true );
    }
}

void WorksheetData::setHyperlink( const HyperlinkModel& rModel )
{
    maHyperlinks.push_back( rModel );
}

void WorksheetData::setValidation( const ValidationModel& rModel )
{
    maValidations.push_back( rModel );
}

void WorksheetData::setDrawingPath( const OUString& rDrawingPath )
{
    maDrawingPath = rDrawingPath;
}

void WorksheetData::setVmlDrawingPath( const OUString& rVmlDrawingPath )
{
    maVmlDrawingPath = rVmlDrawingPath;
}

void WorksheetData::extendUsedArea( const CellAddress& rAddress )
{
    maUsedArea.StartColumn = ::std::min( maUsedArea.StartColumn, rAddress.Column );
    maUsedArea.StartRow    = ::std::min( maUsedArea.StartRow,    rAddress.Row );
    maUsedArea.EndColumn   = ::std::max( maUsedArea.EndColumn,   rAddress.Column );
    maUsedArea.EndRow      = ::std::max( maUsedArea.EndRow,      rAddress.Row );
}

void WorksheetData::extendUsedArea( const CellRangeAddress& rRange )
{
    extendUsedArea( CellAddress( rRange.Sheet, rRange.StartColumn, rRange.StartRow ) );
    extendUsedArea( CellAddress( rRange.Sheet, rRange.EndColumn, rRange.EndRow ) );
}

void WorksheetData::extendShapeBoundingBox( const Rectangle& rShapeRect )
{
    if( (maShapeBoundingBox.Width == 0) && (maShapeBoundingBox.Height == 0) )
    {
        // width and height of maShapeBoundingBox are assumed to be zero on first cell
        maShapeBoundingBox = rShapeRect;
    }
    else
    {
        sal_Int32 nEndX = ::std::max( maShapeBoundingBox.X + maShapeBoundingBox.Width, rShapeRect.X + rShapeRect.Width );
        sal_Int32 nEndY = ::std::max( maShapeBoundingBox.Y + maShapeBoundingBox.Height, rShapeRect.Y + rShapeRect.Height );
        maShapeBoundingBox.X = ::std::min( maShapeBoundingBox.X, rShapeRect.X );
        maShapeBoundingBox.Y = ::std::min( maShapeBoundingBox.Y, rShapeRect.Y );
        maShapeBoundingBox.Width = nEndX - maShapeBoundingBox.X;
        maShapeBoundingBox.Height = nEndY - maShapeBoundingBox.Y;
    }
}

void WorksheetData::setBaseColumnWidth( sal_Int32 nWidth )
{
    // do not modify width, if setDefaultColumnWidth() has been used
    if( !mbHasDefWidth && (nWidth > 0) )
    {
        // #i3006# add 5 pixels padding to the width
        const UnitConverter& rUnitConv = getUnitConverter();
        maDefColModel.mfWidth = rUnitConv.scaleFromMm100(
            rUnitConv.scaleToMm100( nWidth, UNIT_DIGIT ) + rUnitConv.scaleToMm100( 5, UNIT_SCREENX ), UNIT_DIGIT );
    }
}

void WorksheetData::setDefaultColumnWidth( double fWidth )
{
    // overrides a width set with setBaseColumnWidth()
    if( fWidth > 0.0 )
    {
        maDefColModel.mfWidth = fWidth;
        mbHasDefWidth = true;
    }
}

void WorksheetData::setColumnModel( const ColumnModel& rModel )
{
    // convert 1-based OOX column indexes to 0-based API column indexes
    sal_Int32 nFirstCol = rModel.mnFirstCol - 1;
    sal_Int32 nLastCol = rModel.mnLastCol - 1;
    if( (0 <= nFirstCol) && (nFirstCol <= mrMaxApiPos.Column) )
    {
        // set column formatting directly, nLastCol is checked inside the function
        convertColumnFormat( nFirstCol, nLastCol, rModel.mnXfId );
        // expand last entry or add new entry
        if( maColModels.empty() || !maColModels.rbegin()->second.tryExpand( rModel ) )
            maColModels[ nFirstCol ] = rModel;
    }
}

void WorksheetData::setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom )
{
    maDefRowModel.mfHeight = fHeight;
    maDefRowModel.mbCustomHeight = bCustomHeight;
    maDefRowModel.mbHidden = bHidden;
    maDefRowModel.mbThickTop = bThickTop;
    maDefRowModel.mbThickBottom = bThickBottom;
}

void WorksheetData::setRowModel( const RowModel& rModel )
{
    // convert 1-based OOX row indexes to 0-based API row indexes
    sal_Int32 nFirstRow = rModel.mnFirstRow - 1;
    sal_Int32 nLastRow = rModel.mnLastRow - 1;
    if( (0 <= nFirstRow) && (nFirstRow <= mrMaxApiPos.Row) )
    {
        // set row formatting
        if( rModel.mbCustomFormat )
        {
            // try to expand cached row range, if formatting is equal
            if( (maXfIdRowRange.mnLastRow < 0) || !maXfIdRowRange.tryExpand( nFirstRow, nLastRow, rModel.mnXfId ) )
            {
                writeXfIdRowRangeProperties( maXfIdRowRange );
                maXfIdRowRange.set( nFirstRow, nLastRow, rModel.mnXfId );
            }
        }
        else if( maXfIdRowRange.mnLastRow >= 0 )
        {
            // finish last cached row range
            writeXfIdRowRangeProperties( maXfIdRowRange );
            maXfIdRowRange.set( -1, -1, -1 );
        }

        // expand last entry or add new entry
        if( maRowModels.empty() || !maRowModels.rbegin()->second.tryExpand( rModel ) )
            maRowModels[ nFirstRow ] = rModel;
    }
    lclUpdateProgressBar( mxRowProgress, maUsedArea, nLastRow );
}

void WorksheetData::convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId ) const
{
    CellRangeAddress aRange( getSheetIndex(), nFirstCol, 0, nLastCol, mrMaxApiPos.Row );
    if( getAddressConverter().validateCellRange( aRange, true, false ) )
    {
        PropertySet aPropSet( getCellRange( aRange ) );
        getStyles().writeCellXfToPropertySet( aPropSet, nXfId );
    }
}

void WorksheetData::convertRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId ) const
{
    CellRangeAddress aRange( getSheetIndex(), 0, nFirstRow, mrMaxApiPos.Column, nLastRow );
    if( getAddressConverter().validateCellRange( aRange, true, false ) )
    {
        PropertySet aPropSet( getCellRange( aRange ) );
        getStyles().writeCellXfToPropertySet( aPropSet, nXfId );
    }
}

void WorksheetData::initializeWorksheetImport()
{
    // set default cell style for unused cells
    PropertySet aPropSet( mxSheet );
    aPropSet.setProperty( PROP_CellStyle, getStyles().getDefaultStyleName() );

    /*  Remember current sheet index in global data, needed by some global
        objects, e.g. the chart converter. */
    setCurrentSheetIndex( getSheetIndex() );
}

void WorksheetData::finalizeWorksheetImport()
{
    lclUpdateProgressBar( mxRowProgress, 1.0 );
    finalizeXfIdRanges();
    lclUpdateProgressBar( mxFinalProgress, 0.25 );
    finalizeHyperlinkRanges();
    finalizeValidationRanges();
    finalizeMergedRanges();
    maSheetSett.finalizeImport();
    maCondFormats.finalizeImport();
    maPageSett.finalizeImport();
    maSheetViewSett.finalizeImport();
    maSheetSett.finalizeImport();

    lclUpdateProgressBar( mxFinalProgress, 0.5 );
    convertColumns();
    convertRows();
    lclUpdateProgressBar( mxFinalProgress, 0.75 );
    finalizeDrawing();
    finalizeVmlDrawing();
    maComments.finalizeImport();    // after VML drawing
    finalizeUsedArea();             // after DML and VML drawing
    lclUpdateProgressBar( mxFinalProgress, 1.0 );

    // reset current sheet index in global data
    setCurrentSheetIndex( -1 );
}

// private --------------------------------------------------------------------

WorksheetData::XfIdRowRange::XfIdRowRange() :
    mnFirstRow( -1 ),
    mnLastRow( -1 ),
    mnXfId( -1 )
{
}

bool WorksheetData::XfIdRowRange::intersects( const CellRangeAddress& rRange ) const
{
    return (rRange.StartRow <= mnLastRow) && (mnFirstRow <= rRange.EndRow);
}

void WorksheetData::XfIdRowRange::set( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId )
{
    mnFirstRow = nFirstRow;
    mnLastRow = nLastRow;
    mnXfId = nXfId;
}

bool WorksheetData::XfIdRowRange::tryExpand( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId )
{
    if( mnXfId == nXfId )
    {
        if( mnLastRow + 1 == nFirstRow )
        {
            mnLastRow = nLastRow;
            return true;
        }
        if( mnFirstRow == nLastRow + 1 )
        {
            mnFirstRow = nFirstRow;
            return true;
        }
    }
    return false;
}

void WorksheetData::XfIdRange::set( const CellModel& rModel )
{
    maRange.Sheet = rModel.maAddress.Sheet;
    maRange.StartColumn = maRange.EndColumn = rModel.maAddress.Column;
    maRange.StartRow = maRange.EndRow = rModel.maAddress.Row;
    mnXfId = rModel.mnXfId;
    mnNumFmtId = rModel.mnNumFmtId;
}

bool WorksheetData::XfIdRange::tryExpand( const CellModel& rModel )
{
    if( (mnXfId == rModel.mnXfId) && (mnNumFmtId == rModel.mnNumFmtId) &&
        (maRange.StartRow == rModel.maAddress.Row) &&
        (maRange.EndRow == rModel.maAddress.Row) &&
        (maRange.EndColumn + 1 == rModel.maAddress.Column) )
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

void WorksheetData::writeXfIdRowRangeProperties( const XfIdRowRange& rXfIdRowRange ) const
{
    if( (rXfIdRowRange.mnLastRow >= 0) && (rXfIdRowRange.mnXfId >= 0) )
        convertRowFormat( rXfIdRowRange.mnFirstRow, rXfIdRowRange.mnLastRow, rXfIdRowRange.mnXfId );
}

void WorksheetData::writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const
{
    StylesBuffer& rStyles = getStyles();
    PropertyMap aPropMap;
    if( rXfIdRange.mnXfId >= 0 )
        rStyles.writeCellXfToPropertyMap( aPropMap, rXfIdRange.mnXfId );
    if( rXfIdRange.mnNumFmtId >= 0 )
        rStyles.writeNumFmtToPropertyMap( aPropMap, rXfIdRange.mnNumFmtId );
    PropertySet aPropSet( getCellRange( rXfIdRange.maRange ) );
    aPropSet.setProperties( aPropMap );
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
    // write default formatting of remaining row range
    writeXfIdRowRangeProperties( maXfIdRowRange );
    // try to merge remaining inserted ranges
    mergeXfIdRanges();
    // write all formatting
    for( XfIdRangeMap::const_iterator aIt = maXfIdRanges.begin(), aEnd = maXfIdRanges.end(); aIt != aEnd; ++aIt )
        writeXfIdRangeProperties( aIt->second );
}

void WorksheetData::finalizeHyperlinkRanges() const
{
    for( HyperlinkModelList::const_iterator aIt = maHyperlinks.begin(), aEnd = maHyperlinks.end(); aIt != aEnd; ++aIt )
    {
        OUString aUrl = getHyperlinkUrl( *aIt );
        // try to insert URL into each cell of the range
        if( aUrl.getLength() > 0 )
            for( CellAddress aAddress( getSheetIndex(), aIt->maRange.StartColumn, aIt->maRange.StartRow ); aAddress.Row <= aIt->maRange.EndRow; ++aAddress.Row )
                for( aAddress.Column = aIt->maRange.StartColumn; aAddress.Column <= aIt->maRange.EndColumn; ++aAddress.Column )
                    insertHyperlink( aAddress, aUrl );
    }
}

OUString WorksheetData::getHyperlinkUrl( const HyperlinkModel& rHyperlink ) const
{
    OUStringBuffer aUrlBuffer;
    if( rHyperlink.maTarget.getLength() > 0 )
        aUrlBuffer.append( getBaseFilter().getAbsoluteUrl( rHyperlink.maTarget ) );
    if( rHyperlink.maLocation.getLength() > 0 )
        aUrlBuffer.append( sal_Unicode( '#' ) ).append( rHyperlink.maLocation );
    OUString aUrl = aUrlBuffer.makeStringAndClear();

    // convert '#SheetName!A1' to '#SheetName.A1'
    if( (aUrl.getLength() > 0) && (aUrl[ 0 ] == '#') )
    {
        sal_Int32 nSepPos = aUrl.lastIndexOf( '!' );
        if( nSepPos > 0 )
        {
            // replace the exclamation mark with a period
            aUrl = aUrl.replaceAt( nSepPos, 1, OUString( sal_Unicode( '.' ) ) );
            // #i66592# convert sheet names that have been renamed on import
            OUString aSheetName = aUrl.copy( 1, nSepPos - 1 );
            OUString aCalcName = getWorksheets().getCalcSheetName( aSheetName );
            if( aCalcName.getLength() > 0 )
                aUrl = aUrl.replaceAt( 1, nSepPos - 1, aCalcName );
        }
    }

    return aUrl;
}

void WorksheetData::insertHyperlink( const CellAddress& rAddress, const OUString& rUrl ) const
{
    Reference< XCell > xCell = getCell( rAddress );
    if( xCell.is() ) switch( xCell->getType() )
    {
        // #i54261# restrict creation of URL field to text cells
        case ::com::sun::star::table::CellContentType_TEXT:
        {
            Reference< XText > xText( xCell, UNO_QUERY );
            if( xText.is() )
            {
                // create a URL field object and set its properties
                Reference< XTextContent > xUrlField( getDocumentFactory()->createInstance( maUrlTextField ), UNO_QUERY );
                OSL_ENSURE( xUrlField.is(), "WorksheetData::insertHyperlink - cannot create text field" );
                if( xUrlField.is() )
                {
                    // properties of the URL field
                    PropertySet aPropSet( xUrlField );
                    aPropSet.setProperty( PROP_URL, rUrl );
                    aPropSet.setProperty( PROP_Representation, xText->getString() );
                    try
                    {
                        // insert the field into the cell
                        xText->setString( OUString() );
                        Reference< XTextRange > xRange( xText->createTextCursor(), UNO_QUERY_THROW );
                        xText->insertTextContent( xRange, xUrlField, sal_False );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( false, "WorksheetData::insertHyperlink - cannot insert text field" );
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
            OSL_ENSURE( xTokens.is(), "WorksheetHelper::insertHyperlink - missing formula interface" );
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
    for( ValidationModelList::const_iterator aIt = maValidations.begin(), aEnd = maValidations.end(); aIt != aEnd; ++aIt )
    {
        PropertySet aPropSet( getCellRangeList( aIt->maRanges ) );

        Reference< XPropertySet > xValidation;
        if( aPropSet.getProperty( xValidation, PROP_Validation ) && xValidation.is() )
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
            aValProps.setProperty( PROP_Type, eType );

            // convert error alert style to API enum
            ValidationAlertStyle eAlertStyle = csss::ValidationAlertStyle_STOP;
            switch( aIt->mnErrorStyle )
            {
                case XML_information:   eAlertStyle = csss::ValidationAlertStyle_INFO;      break;
                case XML_stop:          eAlertStyle = csss::ValidationAlertStyle_STOP;      break;
                case XML_warning:       eAlertStyle = csss::ValidationAlertStyle_WARNING;   break;
                default:    OSL_ENSURE( false, "WorksheetData::finalizeValidationRanges - unknown error style" );
            }
            aValProps.setProperty( PROP_ErrorAlertStyle, eAlertStyle );

            // convert dropdown style to API visibility constants
            sal_Int16 nVisibility = aIt->mbNoDropDown ? csss::TableValidationVisibility::INVISIBLE : csss::TableValidationVisibility::UNSORTED;
            aValProps.setProperty( PROP_ShowList, nVisibility );

            // messages
            aValProps.setProperty( PROP_ShowInputMessage, aIt->mbShowInputMsg );
            aValProps.setProperty( PROP_InputTitle, aIt->maInputTitle );
            aValProps.setProperty( PROP_InputMessage, aIt->maInputMessage );
            aValProps.setProperty( PROP_ShowErrorMessage, aIt->mbShowErrorMsg );
            aValProps.setProperty( PROP_ErrorTitle, aIt->maErrorTitle );
            aValProps.setProperty( PROP_ErrorMessage, aIt->maErrorMessage );

            // allow blank cells
            aValProps.setProperty( PROP_IgnoreBlankCells, aIt->mbAllowBlank );

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
            aPropSet.setProperty( PROP_Validation, xValidation );
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
        Reference< XCell > xTopLeft( getCell( CellAddress( getSheetIndex(), rRange.StartColumn, rRange.StartRow ) ), UNO_SET_THROW );
        PropertySet aTopLeftProp( xTopLeft );

        // copy right border of top-right cell to right border of top-left cell
        if( bMultiCol )
        {
            PropertySet aTopRightProp( getCell( CellAddress( getSheetIndex(), rRange.EndColumn, rRange.StartRow ) ) );
            BorderLine aLine;
            if( aTopRightProp.getProperty( aLine, PROP_RightBorder ) )
                aTopLeftProp.setProperty( PROP_RightBorder, aLine );
        }

        // copy bottom border of bottom-left cell to bottom border of top-left cell
        if( bMultiRow )
        {
            PropertySet aBottomLeftProp( getCell( CellAddress( getSheetIndex(), rRange.StartColumn, rRange.EndRow ) ) );
            BorderLine aLine;
            if( aBottomLeftProp.getProperty( aLine, PROP_BottomBorder ) )
                aTopLeftProp.setProperty( PROP_BottomBorder, aLine );
        }

        // #i93609# merged range in a single row: test if manual row height is needed
        if( !bMultiRow )
        {
            bool bTextWrap = aTopLeftProp.getBoolProperty( PROP_IsTextWrapped );
            if( !bTextWrap && (xTopLeft->getType() == ::com::sun::star::table::CellContentType_TEXT) )
            {
                Reference< XText > xText( xTopLeft, UNO_QUERY );
                bTextWrap = xText.is() && (xText->getString().indexOf( '\x0A' ) >= 0);
            }
            if( bTextWrap )
                maManualRowHeights.insert( rRange.StartRow );
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

void WorksheetData::finalizeVmlDrawing()
{
    OSL_ENSURE( (getFilterType() == FILTER_OOX) || (maVmlDrawingPath.getLength() == 0),
        "WorksheetData::finalizeVmlDrawing - unexpected VML path" );
    if( (getFilterType() == FILTER_OOX) && (maVmlDrawingPath.getLength() > 0) )
        importOoxFragment( new OoxVmlDrawingFragment( *this, maVmlDrawingPath ) );
}

void WorksheetData::finalizeUsedArea()
{
    /*  Extend used area of the sheet by cells covered with drawing objects.
        Needed if the imported document is inserted as "OLE object from file"
        and thus does not provide an OLE size property by itself. */
    if( (maShapeBoundingBox.Width > 0) || (maShapeBoundingBox.Height > 0) )
        extendUsedArea( getCellRangeFromRectangle( maShapeBoundingBox ) );

    // if no used area is set, default to A1
    if( maUsedArea.StartColumn > maUsedArea.EndColumn )
        maUsedArea.StartColumn = maUsedArea.EndColumn = 0;
    if( maUsedArea.StartRow > maUsedArea.EndRow )
        maUsedArea.StartRow = maUsedArea.EndRow = 0;
        
    /*  Register the used area of this sheet in global view settings. The
        global view settings will set the visible area if this document is an
        embedded OLE object. */
    getViewSettings().setSheetUsedArea( maUsedArea );
}

void WorksheetData::convertColumns()
{
    sal_Int32 nNextCol = 0;
    sal_Int32 nMaxCol = mrMaxApiPos.Column;
    // stores first grouped column index for each level
    OutlineLevelVec aColLevels;

    for( ColumnModelMap::const_iterator aIt = maColModels.begin(), aEnd = maColModels.end(); aIt != aEnd; ++aIt )
    {
        // convert 1-based OOX column indexes to 0-based API column indexes
        sal_Int32 nFirstCol = ::std::max( aIt->second.mnFirstCol - 1, nNextCol );
        sal_Int32 nLastCol = ::std::min( aIt->second.mnLastCol - 1, nMaxCol );

        // process gap between two column models, use default column model
        if( nNextCol < nFirstCol )
            convertColumns( aColLevels, nNextCol, nFirstCol - 1, maDefColModel );
        // process the column model
        convertColumns( aColLevels, nFirstCol, nLastCol, aIt->second );

        // cache next column to be processed
        nNextCol = nLastCol + 1;
    }

    // remaining default columns to end of sheet
    convertColumns( aColLevels, nNextCol, nMaxCol, maDefColModel );
    // close remaining column outlines spanning to end of sheet
    convertOutlines( aColLevels, nMaxCol + 1, 0, false, false );
}

void WorksheetData::convertColumns( OutlineLevelVec& orColLevels,
        sal_Int32 nFirstCol, sal_Int32 nLastCol, const ColumnModel& rModel )
{
    PropertySet aPropSet( getColumns( nFirstCol, nLastCol ) );

    // column width: convert 'number of characters' to column width in 1/100 mm
    sal_Int32 nWidth = getUnitConverter().scaleToMm100( rModel.mfWidth, UNIT_DIGIT );
    // macro sheets have double width
    if( meSheetType == SHEETTYPE_MACROSHEET )
        nWidth *= 2;
    if( nWidth > 0 )
        aPropSet.setProperty( PROP_Width, nWidth );

    // hidden columns: TODO: #108683# hide columns later?
    if( rModel.mbHidden )
        aPropSet.setProperty( PROP_IsVisible, false );

    // outline settings for this column range
    convertOutlines( orColLevels, nFirstCol, rModel.mnLevel, rModel.mbCollapsed, false );
}

void WorksheetData::convertRows()
{
    sal_Int32 nNextRow = 0;
    sal_Int32 nMaxRow = mrMaxApiPos.Row;
    // stores first grouped row index for each level
    OutlineLevelVec aRowLevels;

    for( RowModelMap::const_iterator aIt = maRowModels.begin(), aEnd = maRowModels.end(); aIt != aEnd; ++aIt )
    {
        // convert 1-based OOX row indexes to 0-based API row indexes
        sal_Int32 nFirstRow = ::std::max( aIt->second.mnFirstRow - 1, nNextRow );
        sal_Int32 nLastRow = ::std::min( aIt->second.mnLastRow - 1, nMaxRow );

        // process gap between two row models, use default row model
        if( nNextRow < nFirstRow )
            convertRows( aRowLevels, nNextRow, nFirstRow - 1, maDefRowModel );
        // process the row model
        convertRows( aRowLevels, nFirstRow, nLastRow, aIt->second, maDefRowModel.mfHeight );

        // cache next row to be processed
        nNextRow = nLastRow + 1;
    }

    // remaining default rows to end of sheet
    convertRows( aRowLevels, nNextRow, nMaxRow, maDefRowModel );
    // close remaining row outlines spanning to end of sheet
    convertOutlines( aRowLevels, nMaxRow + 1, 0, false, true );
}

void WorksheetData::convertRows( OutlineLevelVec& orRowLevels,
        sal_Int32 nFirstRow, sal_Int32 nLastRow, const RowModel& rModel, double fDefHeight )
{
    // row height: convert points to row height in 1/100 mm
    double fHeight = (rModel.mfHeight >= 0.0) ? rModel.mfHeight : fDefHeight;
    sal_Int32 nHeight = getUnitConverter().scaleToMm100( fHeight, UNIT_POINT );
    if( nHeight > 0 )
    {
        ValueRangeVector aManualRows;
        if( rModel.mbCustomHeight )
            aManualRows.push_back( ValueRange( nFirstRow, nLastRow ) );
        else
            maManualRowHeights.intersect( aManualRows, nFirstRow, nLastRow );
        for( ValueRangeVector::const_iterator aIt = aManualRows.begin(), aEnd = aManualRows.end(); aIt != aEnd; ++aIt )
        {
            PropertySet aPropSet( getRows( aIt->mnFirst, aIt->mnLast ) );
            aPropSet.setProperty( PROP_Height, nHeight );
        }
    }

    // hidden rows: TODO: #108683# hide rows later?
    if( rModel.mbHidden )
    {
        PropertySet aPropSet( getRows( nFirstRow, nLastRow ) );
        aPropSet.setProperty( PROP_IsVisible, false );
    }

    // outline settings for this row range
    convertOutlines( orRowLevels, nFirstRow, rModel.mnLevel, rModel.mbCollapsed, true );
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
            CellRangeAddress aRange( getSheetIndex(), 0, nFirstColRow, 0, nLastColRow );
            xOutline->group( aRange, ::com::sun::star::table::TableOrientation_ROWS );
            if( bCollapse )
                xOutline->hideDetail( aRange );
        }
        else
        {
            CellRangeAddress aRange( getSheetIndex(), nFirstColRow, 0, nLastColRow, 0 );
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

const Reference< XSpreadsheet >& WorksheetHelper::getSheet() const
{
    return mrSheetData.getSheet();
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
    if( getAddressConverter().convertToCellRange( aRange, rRangeStr, mrSheetData.getSheetIndex(), true, true ) )
    {
        if( opRange ) *opRange = aRange;
        return mrSheetData.getCellRange( aRange );
    }
    return Reference< XCellRange >();
}

Reference< XCellRange > WorksheetHelper::getCellRange( const BinRange& rBinRange, CellRangeAddress* opRange ) const
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rBinRange, mrSheetData.getSheetIndex(), true, true ) )
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

Reference< XDrawPage > WorksheetHelper::getDrawPage() const
{
    return mrSheetData.getDrawPage();
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

CommentsBuffer& WorksheetHelper::getComments() const
{
    return mrSheetData.getComments();
}

PageSettings& WorksheetHelper::getPageSettings() const
{
    return mrSheetData.getPageSettings();
}

SheetViewSettings& WorksheetHelper::getSheetViewSettings() const
{
    return mrSheetData.getSheetViewSettings();
}

VmlDrawing& WorksheetHelper::getVmlDrawing() const
{
    return mrSheetData.getVmlDrawing();
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

void WorksheetHelper::setDateTimeCell( const Reference< XCell >& rxCell, const DateTime& rDateTime ) const
{
    OSL_ENSURE( rxCell.is(), "WorksheetHelper::setDateTimeCell - missing cell interface" );
    // write serial date/time value into the cell
    double fSerial = getUnitConverter().calcSerialFromDateTime( rDateTime );
    rxCell->setValue( fSerial );
    // set appropriate number format
    using namespace ::com::sun::star::util::NumberFormat;
    sal_Int16 nStdFmt = (fSerial < 1.0) ? TIME : (((rDateTime.Hours > 0) || (rDateTime.Minutes > 0) || (rDateTime.Seconds > 0)) ? DATETIME : DATE);
    setStandardNumFmt( rxCell, nStdFmt );
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

void WorksheetHelper::setCell( CellModel& orModel ) const
{
    OSL_ENSURE( orModel.mxCell.is(), "WorksheetHelper::setCell - missing cell interface" );
    if( orModel.mbHasValueStr ) switch( orModel.mnCellType )
    {
        case XML_b:
            setBooleanCell( orModel.mxCell, orModel.maValueStr.toDouble() != 0.0 );
            // #108770# set 'Standard' number format for all Boolean cells
            orModel.mnNumFmtId = 0;
        break;
        case XML_n:
            orModel.mxCell->setValue( orModel.maValueStr.toDouble() );
        break;
        case XML_e:
            setErrorCell( orModel.mxCell, orModel.maValueStr );
        break;
        case XML_str:
            setStringCell( orModel.mxCell, orModel.maValueStr );
        break;
        case XML_s:
            setSharedStringCell( orModel.mxCell, orModel.maValueStr.toInt32(), orModel.mnXfId );
        break;
    }
}

void WorksheetHelper::setStandardNumFmt( const Reference< XCell >& rxCell, sal_Int16 nStdNumFmt ) const
{
    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( getDocument(), UNO_QUERY_THROW );
        Reference< XNumberFormatTypes > xNumFmtTypes( xNumFmtsSupp->getNumberFormats(), UNO_QUERY_THROW );
        sal_Int32 nIndex = xNumFmtTypes->getStandardFormat( nStdNumFmt, Locale() );
        PropertySet aPropSet( rxCell );
        aPropSet.setProperty( PROP_NumberFormat, nIndex );
    }
    catch( Exception& )
    {
    }
}

void WorksheetHelper::setSheetType( WorksheetType eSheetType )
{
    mrSheetData.setSheetType( eSheetType );
}

void WorksheetHelper::setCellFormat( const CellModel& rModel )
{
    mrSheetData.setCellFormat( rModel );
}

void WorksheetHelper::setMergedRange( const CellRangeAddress& rRange )
{
    mrSheetData.setMergedRange( rRange );
}

void WorksheetHelper::setPageBreak( const PageBreakModel& rModel, bool bRowBreak )
{
    mrSheetData.setPageBreak( rModel, bRowBreak );
}

void WorksheetHelper::setHyperlink( const HyperlinkModel& rModel )
{
    mrSheetData.setHyperlink( rModel );
}

void WorksheetHelper::setValidation( const ValidationModel& rModel )
{
    mrSheetData.setValidation( rModel );
}

void WorksheetHelper::setTableOperation( const CellRangeAddress& rRange, const DataTableModel& rModel ) const
{
    OSL_ENSURE( getAddressConverter().checkCellRange( rRange, true, false ), "WorksheetHelper::setTableOperation - invalid range" );
    bool bOk = false;
    if( !rModel.mbRef1Deleted && (rModel.maRef1.getLength() > 0) && (rRange.StartColumn > 0) && (rRange.StartRow > 0) )
    {
        CellRangeAddress aOpRange = rRange;
        CellAddress aRef1, aRef2;
        if( getAddressConverter().convertToCellAddress( aRef1, rModel.maRef1, mrSheetData.getSheetIndex(), true ) ) try
        {
            if( rModel.mb2dTable )
            {
                if( !rModel.mbRef2Deleted && getAddressConverter().convertToCellAddress( aRef2, rModel.maRef2, mrSheetData.getSheetIndex(), true ) )
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
            else if( rModel.mbRowTable )
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
    PropertySet aPropSet( getSheet() );

    if( !rColRanges.empty() && aPropSet.getProperty( xLabelRanges, PROP_ColumnLabelRanges ) && xLabelRanges.is() )
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

    if( !rRowRanges.empty() && aPropSet.getProperty( xLabelRanges, PROP_RowLabelRanges ) && xLabelRanges.is() )
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

void WorksheetHelper::extendUsedArea( const CellAddress& rAddress )
{
    mrSheetData.extendUsedArea( rAddress );
}

void WorksheetHelper::extendUsedArea( const CellRangeAddress& rRange )
{
    mrSheetData.extendUsedArea( rRange );
}

void WorksheetHelper::extendShapeBoundingBox( const Rectangle& rShapeRect )
{
    mrSheetData.extendShapeBoundingBox( rShapeRect );
}

void WorksheetHelper::setBaseColumnWidth( sal_Int32 nWidth )
{
    mrSheetData.setBaseColumnWidth( nWidth );
}

void WorksheetHelper::setDefaultColumnWidth( double fWidth )
{
    mrSheetData.setDefaultColumnWidth( fWidth );
}

void WorksheetHelper::setDefaultColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId )
{
    mrSheetData.convertColumnFormat( nFirstCol, nLastCol, nXfId );
}

void WorksheetHelper::setColumnModel( const ColumnModel& rModel )
{
    mrSheetData.setColumnModel( rModel );
}

void WorksheetHelper::setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom )
{
    mrSheetData.setDefaultRowSettings( fHeight, bCustomHeight, bHidden, bThickTop, bThickBottom );
}

void WorksheetHelper::setRowModel( const RowModel& rModel )
{
    mrSheetData.setRowModel( rModel );
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

WorksheetHelperRoot::WorksheetHelperRoot( const WorkbookHelper& rHelper, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
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

