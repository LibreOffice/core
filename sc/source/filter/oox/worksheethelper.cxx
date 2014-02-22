/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "worksheethelper.hxx"

#include <algorithm>
#include <list>
#include <utility>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XMultiFormulaTokens.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSheetCondition2.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XText.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/propertyset.hxx"
#include "addressconverter.hxx"
#include "autofilterbuffer.hxx"
#include "commentsbuffer.hxx"
#include "condformatbuffer.hxx"
#include "convuno.hxx"
#include "document.hxx"
#include "drawingfragment.hxx"
#include "drawingmanager.hxx"
#include "formulaparser.hxx"
#include "pagesettings.hxx"
#include "querytablebuffer.hxx"
#include "sharedstringsbuffer.hxx"
#include "sheetdatabuffer.hxx"
#include "stylesbuffer.hxx"
#include "tokenuno.hxx"
#include "unitconverter.hxx"
#include "viewsettings.hxx"
#include "workbooksettings.hxx"
#include "worksheetbuffer.hxx"
#include "worksheetsettings.hxx"
#include "formulabuffer.hxx"
#include "scitems.hxx"
#include "editutil.hxx"
#include "tokenarray.hxx"
#include "tablebuffer.hxx"
#include "documentimport.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "cellvalue.hxx"

#include <svl/stritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>

namespace oox {
namespace xls {



using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;




namespace {

void lclUpdateProgressBar( const ISegmentProgressBarRef& rxProgressBar, double fPosition )
{
    if( rxProgressBar.get() )
        rxProgressBar->setPosition( fPosition );
}

} 




ColumnModel::ColumnModel() :
    maRange( -1 ),
    mfWidth( 0.0 ),
    mnXfId( -1 ),
    mnLevel( 0 ),
    mbShowPhonetic( false ),
    mbHidden( false ),
    mbCollapsed( false )
{
}

bool ColumnModel::isMergeable( const ColumnModel& rModel ) const
{
    return
        (maRange.mnFirst        <= rModel.maRange.mnFirst) &&
        (rModel.maRange.mnFirst <= maRange.mnLast + 1) &&
        (mfWidth                == rModel.mfWidth) &&
        
        (mnLevel                == rModel.mnLevel) &&
        (mbHidden               == rModel.mbHidden) &&
        (mbCollapsed            == rModel.mbCollapsed);
}



RowModel::RowModel() :
    mnRow( -1 ),
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

void RowModel::insertColSpan( const ValueRange& rColSpan )
{
    if( (0 <= rColSpan.mnFirst) && (rColSpan.mnFirst <= rColSpan.mnLast) )
        maColSpans.insert( rColSpan );
}

bool RowModel::isMergeable( const RowModel& rModel ) const
{
    return
        
        (mfHeight       == rModel.mfHeight) &&
        
        (mnLevel        == rModel.mnLevel) &&
        (mbCustomHeight == rModel.mbCustomHeight) &&
        (mbHidden       == rModel.mbHidden) &&
        (mbCollapsed    == rModel.mbCollapsed);
}



PageBreakModel::PageBreakModel() :
    mnColRow( 0 ),
    mbManual( false )
{
}



HyperlinkModel::HyperlinkModel()
{
}



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

void ValidationModel::setBiffType( sal_uInt8 nType )
{
    static const sal_Int32 spnTypeIds[] = {
        XML_none, XML_whole, XML_decimal, XML_list, XML_date, XML_time, XML_textLength, XML_custom };
    mnType = STATIC_ARRAY_SELECT( spnTypeIds, nType, XML_none );
}

void ValidationModel::setBiffOperator( sal_uInt8 nOperator )
{
    static const sal_Int32 spnOperators[] = {
        XML_between, XML_notBetween, XML_equal, XML_notEqual,
        XML_greaterThan, XML_lessThan, XML_greaterThanOrEqual, XML_lessThanOrEqual };
    mnOperator = STATIC_ARRAY_SELECT( spnOperators, nOperator, XML_TOKEN_INVALID );
}

void ValidationModel::setBiffErrorStyle( sal_uInt8 nErrorStyle )
{
    static const sal_Int32 spnErrorStyles[] = { XML_stop, XML_warning, XML_information };
    mnErrorStyle = STATIC_ARRAY_SELECT( spnErrorStyles, nErrorStyle, XML_stop );
}




class WorksheetGlobals : public WorkbookHelper, public IWorksheetProgress
{
public:
    explicit            WorksheetGlobals(
                            const WorkbookHelper& rHelper,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );
    virtual            ~WorksheetGlobals() {}

    /** Returns true, if this helper refers to an existing Calc sheet. */
    inline bool         isValidSheet() const { return mxSheet.is(); }

    /** Returns the type of this sheet. */
    inline WorksheetType getSheetType() const { return meSheetType; }
    /** Returns the index of the current sheet. */
    inline sal_Int16    getSheetIndex() const { return maUsedArea.Sheet; }
    /** Returns the XSpreadsheet interface of the current sheet. */
    inline const Reference< XSpreadsheet >& getSheet() const { return mxSheet; }

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
    Reference< XTableColumns > getColumns( const ValueRange& rColRange ) const;
    /** Returns the XTableRows interface for a range of rows. */
    Reference< XTableRows > getRows( const ValueRange& rRowRange ) const;

    /** Returns the XDrawPage interface of the draw page of the current sheet. */
    Reference< XDrawPage > getDrawPage() const;
    /** Returns the size of the entire drawing page in 1/100 mm. */
    const awt::Size&         getDrawPageSize() const;

    /** Returns the absolute position of the top-left corner of the cell in 1/100 mm. */
    awt::Point               getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the size of the cell in 1/100 mm. */
    awt::Size                getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const;

    /** Returns the address of the cell that contains the passed point in 1/100 mm. */
    CellAddress         getCellAddressFromPosition( const awt::Point& rPosition ) const;
    /** Returns the cell range address that contains the passed rectangle in 1/100 mm. */
    CellRangeAddress    getCellRangeFromRectangle( const awt::Rectangle& rRect ) const;

    /** Returns the buffer for cell contents and cell formatting. */
    inline SheetDataBuffer& getSheetData() { return maSheetData; }
    /** Returns the conditional formatting in this sheet. */
    inline CondFormatBuffer& getCondFormats() { return maCondFormats; }
    /** Returns the buffer for all cell comments in this sheet. */
    inline CommentsBuffer& getComments() { return maComments; }
    /** Returns the auto filters for the sheet. */
    inline AutoFilterBuffer& getAutoFilters() { return maAutoFilters; }
    /** Returns the buffer for all web query tables in this sheet. */
    inline QueryTableBuffer& getQueryTables() { return maQueryTables; }
    /** Returns the worksheet settings object. */
    inline WorksheetSettings& getWorksheetSettings() { return maSheetSett; }
    /** Returns the page/print settings for this sheet. */
    inline PageSettings& getPageSettings() { return maPageSett; }
    /** Returns the view settings for this sheet. */
    inline SheetViewSettings& getSheetViewSettings() { return maSheetViewSett; }
    /** Returns the VML drawing page for this sheet (OOXML/BIFF12 only). */
    inline VmlDrawing&  getVmlDrawing() { return *mxVmlDrawing; }
    /** returns the ExtLst entries that need to be filled */
    inline ExtLst&      getExtLst() { return maExtLst; }

    /** Returns the BIFF drawing page for this sheet (BIFF2-BIFF8 only). */
    inline BiffSheetDrawing& getBiffDrawing() const { return *mxBiffDrawing; }

    /** Changes the current sheet type. */
    inline void         setSheetType( WorksheetType eSheetType ) { meSheetType = eSheetType; }
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
    void                extendShapeBoundingBox( const awt::Rectangle& rShapeRect );

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
    /** Converts column default cell formatting. */
    void                convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId ) const;

    /** Sets default height and hidden state for all unused rows in the sheet. */
    void                setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom );
    /** Sets row settings for a specific row.
        @descr  Row default formatting is converted directly, other settings
        are cached and converted in the finalizeImport() call. */
    void                setRowModel( const RowModel& rModel );

    /** Initial conversion before importing the worksheet. */
    void                initializeWorksheetImport();
    /** Final conversion after importing the worksheet. */
    void                finalizeWorksheetImport();

    void finalizeDrawingImport();

    
    virtual ISegmentProgressBarRef getRowProgress()
    {
        return mxRowProgress;
    }
    virtual void setCustomRowProgress( const ISegmentProgressBarRef &rxRowProgress )
    {
        mxRowProgress = rxRowProgress;
        mbFastRowProgress = true;
    }

private:
    typedef ::std::vector< sal_Int32 >                  OutlineLevelVec;
    typedef ::std::pair< ColumnModel, sal_Int32 >       ColumnModelRange;
    typedef ::std::map< sal_Int32, ColumnModelRange >   ColumnModelRangeMap;
    typedef ::std::pair< RowModel, sal_Int32 >          RowModelRange;
    typedef ::std::map< sal_Int32, RowModelRange >      RowModelRangeMap;
    typedef ::std::list< HyperlinkModel >               HyperlinkModelList;
    typedef ::std::list< ValidationModel >              ValidationModelList;

    /** Inserts all imported hyperlinks into their cell ranges. */
    void finalizeHyperlinkRanges();
    /** Generates the final URL for the passed hyperlink. */
    OUString            getHyperlinkUrl( const HyperlinkModel& rHyperlink ) const;
    /** Inserts a hyperlinks into the specified cell. */
    void insertHyperlink( const CellAddress& rAddress, const OUString& rUrl );

    /** Inserts all imported data validations into their cell ranges. */
    void                finalizeValidationRanges() const;

    /** Converts column properties for all columns in the sheet. */
    void                convertColumns();
    /** Converts column properties. */
    void                convertColumns( OutlineLevelVec& orColLevels, const ValueRange& rColRange, const ColumnModel& rModel );

    /** Converts row properties for all rows in the sheet. */
    void                convertRows();
    /** Converts row properties. */
    void                convertRows( OutlineLevelVec& orRowLevels, const ValueRange& rRowRange, const RowModel& rModel, double fDefHeight = -1.0 );

    /** Converts outline grouping for the passed column or row. */
    void                convertOutlines( OutlineLevelVec& orLevels, sal_Int32 nColRow, sal_Int32 nLevel, bool bCollapsed, bool bRows );
    /** Groups columns or rows for the given range. */
    void                groupColumnsOrRows( sal_Int32 nFirstColRow, sal_Int32 nLastColRow, bool bCollapsed, bool bRows );

    /** Imports the drawings of the sheet (DML, VML, DFF) and updates the used area. */
    void                finalizeDrawings();

    /** Update the row import progress bar */
    void UpdateRowProgress( const CellRangeAddress& rUsedArea, sal_Int32 nRow );

private:
    typedef ::std::auto_ptr< VmlDrawing >       VmlDrawingPtr;
    typedef ::std::auto_ptr< BiffSheetDrawing > BiffSheetDrawingPtr;

    const OUString      maSheetCellRanges;  
    const OUString      maUrlTextField;     
    const CellAddress&  mrMaxApiPos;        
    CellRangeAddress    maUsedArea;         
    ColumnModel         maDefColModel;      
    ColumnModelRangeMap maColModels;        
    RowModel            maDefRowModel;      
    RowModelRangeMap    maRowModels;        
    HyperlinkModelList  maHyperlinks;       
    ValidationModelList maValidations;      
    SheetDataBuffer     maSheetData;        
    CondFormatBuffer    maCondFormats;      
    CommentsBuffer      maComments;         
    AutoFilterBuffer    maAutoFilters;      
    QueryTableBuffer    maQueryTables;      
    WorksheetSettings   maSheetSett;        
    PageSettings        maPageSett;         
    SheetViewSettings   maSheetViewSett;    
    VmlDrawingPtr       mxVmlDrawing;       
    ExtLst              maExtLst;           
    BiffSheetDrawingPtr mxBiffDrawing;      
    OUString            maDrawingPath;      
    OUString            maVmlDrawingPath;   
    awt::Size                maDrawPageSize;     
    awt::Rectangle           maShapeBoundingBox; 
    ISegmentProgressBarRef mxProgressBar;   
    bool                   mbFastRowProgress; 
    ISegmentProgressBarRef mxRowProgress;   
    ISegmentProgressBarRef mxFinalProgress; 
    WorksheetType       meSheetType;        
    Reference< XSpreadsheet > mxSheet;      
    bool                mbHasDefWidth;      
};



WorksheetGlobals::WorksheetGlobals( const WorkbookHelper& rHelper, const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    WorkbookHelper( rHelper ),
    maSheetCellRanges( "com.sun.star.sheet.SheetCellRanges" ),
    maUrlTextField( "com.sun.star.text.TextField.URL" ),
    mrMaxApiPos( rHelper.getAddressConverter().getMaxApiAddress() ),
    maUsedArea( nSheet, SAL_MAX_INT32, SAL_MAX_INT32, -1, -1 ),
    maSheetData( *this ),
    maCondFormats( *this ),
    maComments( *this ),
    maAutoFilters( *this ),
    maQueryTables( *this ),
    maSheetSett( *this ),
    maPageSett( *this ),
    maSheetViewSett( *this ),
    mxProgressBar( rxProgressBar ),
    mbFastRowProgress( false ),
    meSheetType( eSheetType ),
    mbHasDefWidth( false )
{
    mxSheet = getSheetFromDoc( nSheet );
    if( !mxSheet.is() )
        maUsedArea.Sheet = -1;

    
    maDefColModel.mfWidth = 8.5;
    maDefColModel.mnXfId = -1;
    maDefColModel.mnLevel = 0;
    maDefColModel.mbHidden = false;
    maDefColModel.mbCollapsed = false;

    
    maDefRowModel.mfHeight = 0.0;
    maDefRowModel.mnXfId = -1;
    maDefRowModel.mnLevel = 0;
    maDefRowModel.mbCustomHeight = false;
    maDefRowModel.mbCustomFormat = false;
    maDefRowModel.mbShowPhonetic = false;
    maDefRowModel.mbHidden = false;
    maDefRowModel.mbCollapsed = false;

    
    switch( getFilterType() )
    {
        case FILTER_OOXML:
            mxVmlDrawing.reset( new VmlDrawing( *this ) );
        break;
        case FILTER_BIFF:
            mxBiffDrawing.reset( new BiffSheetDrawing( *this ) );
        break;
        case FILTER_UNKNOWN:
        break;
    }

    
    if( mxProgressBar.get() )
    {
        mxRowProgress = mxProgressBar->createSegment( 0.5 );
        mxFinalProgress = mxProgressBar->createSegment( 0.5 );
    }
}

Reference< XCell > WorksheetGlobals::getCell( const CellAddress& rAddress ) const
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

Reference< XCellRange > WorksheetGlobals::getCellRange( const CellRangeAddress& rRange ) const
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

Reference< XSheetCellRanges > WorksheetGlobals::getCellRangeList( const ApiCellRangeList& rRanges ) const
{
    Reference< XSheetCellRanges > xRanges;
    if( mxSheet.is() && !rRanges.empty() ) try
    {
        xRanges.set( getBaseFilter().getModelFactory()->createInstance( maSheetCellRanges ), UNO_QUERY_THROW );
        Reference< XSheetCellRangeContainer > xRangeCont( xRanges, UNO_QUERY_THROW );
        xRangeCont->addRangeAddresses( ContainerHelper::vectorToSequence( rRanges ), sal_False );
    }
    catch( Exception& )
    {
    }
    return xRanges;
}

Reference< XCellRange > WorksheetGlobals::getColumn( sal_Int32 nCol ) const
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

Reference< XCellRange > WorksheetGlobals::getRow( sal_Int32 nRow ) const
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

Reference< XTableColumns > WorksheetGlobals::getColumns( const ValueRange& rColRange ) const
{
    Reference< XTableColumns > xColumns;
    sal_Int32 nLastCol = ::std::min( rColRange.mnLast, mrMaxApiPos.Column );
    if( (0 <= rColRange.mnFirst) && (rColRange.mnFirst <= nLastCol) )
    {
        Reference< XColumnRowRange > xRange( getCellRange( CellRangeAddress( getSheetIndex(), rColRange.mnFirst, 0, nLastCol, 0 ) ), UNO_QUERY );
        if( xRange.is() )
            xColumns = xRange->getColumns();
    }
    return xColumns;
}

Reference< XTableRows > WorksheetGlobals::getRows( const ValueRange& rRowRange ) const
{
    Reference< XTableRows > xRows;
    sal_Int32 nLastRow = ::std::min( rRowRange.mnLast, mrMaxApiPos.Row );
    if( (0 <= rRowRange.mnFirst) && (rRowRange.mnFirst <= nLastRow) )
    {
        Reference< XColumnRowRange > xRange( getCellRange( CellRangeAddress( getSheetIndex(), 0, rRowRange.mnFirst, 0, nLastRow ) ), UNO_QUERY );
        if( xRange.is() )
            xRows = xRange->getRows();
    }
    return xRows;
}

Reference< XDrawPage > WorksheetGlobals::getDrawPage() const
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

const awt::Size& WorksheetGlobals::getDrawPageSize() const
{
    OSL_ENSURE( (maDrawPageSize.Width > 0) && (maDrawPageSize.Height > 0), "WorksheetGlobals::getDrawPageSize - called too early, size invalid" );
    return maDrawPageSize;
}

awt::Point WorksheetGlobals::getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const
{
    awt::Point aPoint;
    PropertySet aCellProp( getCell( CellAddress( getSheetIndex(), nCol, nRow ) ) );
    aCellProp.getProperty( aPoint, PROP_Position );
    return aPoint;
}

awt::Size WorksheetGlobals::getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const
{
    awt::Size aSize;
    PropertySet aCellProp( getCell( CellAddress( getSheetIndex(), nCol, nRow ) ) );
    aCellProp.getProperty( aSize, PROP_Size );
    return aSize;
}

namespace {

inline sal_Int32 lclGetMidAddr( sal_Int32 nBegAddr, sal_Int32 nEndAddr, sal_Int32 nBegPos, sal_Int32 nEndPos, sal_Int32 nSearchPos )
{
    
    return nBegAddr + 1 + static_cast< sal_Int32 >( static_cast< sal_Int64 >( nEndAddr - nBegAddr - 2 ) * (nSearchPos - nBegPos) / (nEndPos - nBegPos) );
}

bool lclPrepareInterval( sal_Int32 nBegAddr, sal_Int32& rnMidAddr, sal_Int32 nEndAddr,
        sal_Int32 nBegPos, sal_Int32 nEndPos, sal_Int32 nSearchPos )
{
    
    if( nSearchPos <= nBegPos )
    {
        rnMidAddr = nBegAddr;
        return false;
    }

    
    if( (nSearchPos >= nEndPos) || (nBegAddr + 1 >= nEndAddr) )
    {
        rnMidAddr = nEndAddr;
        return false;
    }

    /*  Otherwise find mid address according to position. lclGetMidAddr() will
        return an address between nBegAddr and nEndAddr. */
    rnMidAddr = lclGetMidAddr( nBegAddr, nEndAddr, nBegPos, nEndPos, nSearchPos );
    return true;
}

bool lclUpdateInterval( sal_Int32& rnBegAddr, sal_Int32& rnMidAddr, sal_Int32& rnEndAddr,
        sal_Int32& rnBegPos, sal_Int32 nMidPos, sal_Int32& rnEndPos, sal_Int32 nSearchPos )
{
    
    if( nSearchPos < nMidPos )
    {
        
        if( rnBegAddr + 1 >= rnMidAddr )
            return false;
        
        rnEndPos = nMidPos;
        rnEndAddr = rnMidAddr;
        rnMidAddr = lclGetMidAddr( rnBegAddr, rnEndAddr, rnBegPos, rnEndPos, nSearchPos );
        return true;
    }

    
    if( nSearchPos > nMidPos )
    {
        
        if( rnMidAddr + 1 >= rnEndAddr )
        {
            rnMidAddr = rnEndAddr;
            return false;
        }
        
        rnBegPos = nMidPos;
        rnBegAddr = rnMidAddr;
        rnMidAddr = lclGetMidAddr( rnBegAddr, rnEndAddr, rnBegPos, rnEndPos, nSearchPos );
        return true;
    }

    
    return false;
}

} 

CellAddress WorksheetGlobals::getCellAddressFromPosition( const awt::Point& rPosition ) const
{
    
    sal_Int32 nBegCol = 0;
    sal_Int32 nBegRow = 0;
    awt::Point aBegPos( 0, 0 );

    
    sal_Int32 nEndCol = mrMaxApiPos.Column + 1;
    sal_Int32 nEndRow = mrMaxApiPos.Row + 1;
    awt::Point aEndPos( maDrawPageSize.Width, maDrawPageSize.Height );

    
    sal_Int32 nMidCol, nMidRow;
    bool bLoopCols = lclPrepareInterval( nBegCol, nMidCol, nEndCol, aBegPos.X, aEndPos.X, rPosition.X );
    bool bLoopRows = lclPrepareInterval( nBegRow, nMidRow, nEndRow, aBegPos.Y, aEndPos.Y, rPosition.Y );
    awt::Point aMidPos = getCellPosition( nMidCol, nMidRow );

    /*  The loop will find the column/row index of the cell right of/below
        the cell containing the passed point, unless the point is located at
        the top or left border of the containing cell. */
    while( bLoopCols || bLoopRows )
    {
        bLoopCols = bLoopCols && lclUpdateInterval( nBegCol, nMidCol, nEndCol, aBegPos.X, aMidPos.X, aEndPos.X, rPosition.X );
        bLoopRows = bLoopRows && lclUpdateInterval( nBegRow, nMidRow, nEndRow, aBegPos.Y, aMidPos.Y, aEndPos.Y, rPosition.Y );
        aMidPos = getCellPosition( nMidCol, nMidRow );
    }

    /*  The cell left of/above the current search position contains the passed
        point, unless the point is located on the top/left border of the cell,
        or the last column/row of the sheet has been reached. */
    if( aMidPos.X > rPosition.X ) --nMidCol;
    if( aMidPos.Y > rPosition.Y ) --nMidRow;
    return CellAddress( getSheetIndex(), nMidCol, nMidRow );
}

CellRangeAddress WorksheetGlobals::getCellRangeFromRectangle( const awt::Rectangle& rRect ) const
{
    CellAddress aStartAddr = getCellAddressFromPosition( awt::Point( rRect.X, rRect.Y ) );
    awt::Point aBotRight( rRect.X + rRect.Width, rRect.Y + rRect.Height );
    CellAddress aEndAddr = getCellAddressFromPosition( aBotRight );
    bool bMultiCols = aStartAddr.Column < aEndAddr.Column;
    bool bMultiRows = aStartAddr.Row < aEndAddr.Row;
    if( bMultiCols || bMultiRows )
    {
        /*  Reduce end position of the cell range to previous column or row, if
            the rectangle ends exactly between two columns or rows. */
        awt::Point aEndPos = getCellPosition( aEndAddr.Column, aEndAddr.Row );
        if( bMultiCols && (aBotRight.X <= aEndPos.X) )
            --aEndAddr.Column;
        if( bMultiRows && (aBotRight.Y <= aEndPos.Y) )
            --aEndAddr.Row;
    }
    return CellRangeAddress( getSheetIndex(), aStartAddr.Column, aStartAddr.Row, aEndAddr.Column, aEndAddr.Row );
}

void WorksheetGlobals::setPageBreak( const PageBreakModel& rModel, bool bRowBreak )
{
    if( rModel.mbManual && (rModel.mnColRow > 0) )
    {
        PropertySet aPropSet( bRowBreak ? getRow( rModel.mnColRow ) : getColumn( rModel.mnColRow ) );
        aPropSet.setProperty( PROP_IsStartOfNewPage, true );
    }
}

void WorksheetGlobals::setHyperlink( const HyperlinkModel& rModel )
{
    maHyperlinks.push_back( rModel );
}

void WorksheetGlobals::setValidation( const ValidationModel& rModel )
{
    maValidations.push_back( rModel );
}

void WorksheetGlobals::setDrawingPath( const OUString& rDrawingPath )
{
    maDrawingPath = rDrawingPath;
}

void WorksheetGlobals::setVmlDrawingPath( const OUString& rVmlDrawingPath )
{
    maVmlDrawingPath = rVmlDrawingPath;
}

void WorksheetGlobals::extendUsedArea( const CellAddress& rAddress )
{
    maUsedArea.StartColumn = ::std::min( maUsedArea.StartColumn, rAddress.Column );
    maUsedArea.StartRow    = ::std::min( maUsedArea.StartRow,    rAddress.Row );
    maUsedArea.EndColumn   = ::std::max( maUsedArea.EndColumn,   rAddress.Column );
    maUsedArea.EndRow      = ::std::max( maUsedArea.EndRow,      rAddress.Row );
}

void WorksheetGlobals::extendUsedArea( const CellRangeAddress& rRange )
{
    extendUsedArea( CellAddress( rRange.Sheet, rRange.StartColumn, rRange.StartRow ) );
    extendUsedArea( CellAddress( rRange.Sheet, rRange.EndColumn, rRange.EndRow ) );
}

void WorksheetGlobals::extendShapeBoundingBox( const awt::Rectangle& rShapeRect )
{
    if( (maShapeBoundingBox.Width == 0) && (maShapeBoundingBox.Height == 0) )
    {
        
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

void WorksheetGlobals::setBaseColumnWidth( sal_Int32 nWidth )
{
    
    if( !mbHasDefWidth && (nWidth > 0) )
    {
        
        const UnitConverter& rUnitConv = getUnitConverter();
        maDefColModel.mfWidth = rUnitConv.scaleFromMm100(
            rUnitConv.scaleToMm100( nWidth, UNIT_DIGIT ) + rUnitConv.scaleToMm100( 5, UNIT_SCREENX ), UNIT_DIGIT );
    }
}

void WorksheetGlobals::setDefaultColumnWidth( double fWidth )
{
    
    if( fWidth > 0.0 )
    {
        maDefColModel.mfWidth = fWidth;
        mbHasDefWidth = true;
    }
}

void WorksheetGlobals::setColumnModel( const ColumnModel& rModel )
{
    
    sal_Int32 nFirstCol = rModel.maRange.mnFirst - 1;
    sal_Int32 nLastCol = rModel.maRange.mnLast - 1;
    if( getAddressConverter().checkCol( nFirstCol, true ) && (nFirstCol <= nLastCol) )
    {
        
        if( !getAddressConverter().checkCol( nLastCol, true ) )
            nLastCol = mrMaxApiPos.Column;
        
        bool bInsertModel = true;
        if( !maColModels.empty() )
        {
            
            ColumnModelRangeMap::iterator aIt = maColModels.upper_bound( nFirstCol );
            OSL_ENSURE( aIt == maColModels.end(), "WorksheetGlobals::setColModel - columns are unsorted" );
            
            OSL_ENSURE( (aIt == maColModels.end()) || (nLastCol < aIt->first), "WorksheetGlobals::setColModel - multiple models of the same column" );
            if( aIt != maColModels.end() )
                nLastCol = ::std::min( nLastCol, aIt->first - 1 );
            if( aIt != maColModels.begin() )
            {
                
                --aIt;
                
                sal_Int32& rnLastMapCol = aIt->second.second;
                OSL_ENSURE( rnLastMapCol < nFirstCol, "WorksheetGlobals::setColModel - multiple models of the same column" );
                nFirstCol = ::std::max( rnLastMapCol + 1, nFirstCol );
                if( (rnLastMapCol + 1 == nFirstCol) && (nFirstCol <= nLastCol) && aIt->second.first.isMergeable( rModel ) )
                {
                    
                    rnLastMapCol = nLastCol;
                    bInsertModel = false;
                }
            }
        }
        if( nFirstCol <= nLastCol )
        {
            
            if( bInsertModel )
                maColModels[ nFirstCol ] = ColumnModelRange( rModel, nLastCol );
            
            convertColumnFormat( nFirstCol, nLastCol, rModel.mnXfId );
        }
    }
}

void WorksheetGlobals::convertColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId ) const
{
    CellRangeAddress aRange( getSheetIndex(), nFirstCol, 0, nLastCol, mrMaxApiPos.Row );
    if( getAddressConverter().validateCellRange( aRange, true, false ) )
    {
        PropertySet aPropSet( getCellRange( aRange ) );
        getStyles().writeCellXfToPropertySet( aPropSet, nXfId );
    }
}

void WorksheetGlobals::setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom )
{
    maDefRowModel.mfHeight = fHeight;
    maDefRowModel.mbCustomHeight = bCustomHeight;
    maDefRowModel.mbHidden = bHidden;
    maDefRowModel.mbThickTop = bThickTop;
    maDefRowModel.mbThickBottom = bThickBottom;
}

void WorksheetGlobals::setRowModel( const RowModel& rModel )
{
    
    sal_Int32 nRow = rModel.mnRow - 1;
    if( getAddressConverter().checkRow( nRow, true ) )
    {
        
        bool bInsertModel = true;
        bool bUnusedRow = true;
        if( !maRowModels.empty() )
        {
            
            RowModelRangeMap::iterator aIt = maRowModels.upper_bound( nRow );
            OSL_ENSURE( aIt == maRowModels.end(), "WorksheetGlobals::setRowModel - rows are unsorted" );
            if( aIt != maRowModels.begin() )
            {
                
                --aIt;
                
                sal_Int32& rnLastMapRow = aIt->second.second;
                bUnusedRow = rnLastMapRow < nRow;
                OSL_ENSURE( bUnusedRow, "WorksheetGlobals::setRowModel - multiple models of the same row" );
                if( (rnLastMapRow + 1 == nRow) && aIt->second.first.isMergeable( rModel ) )
                {
                    
                    ++rnLastMapRow;
                    bInsertModel = false;
                }
            }
        }
        if( bUnusedRow )
        {
            
            if( bInsertModel )
                maRowModels[ nRow ] = RowModelRange( rModel, nRow );
            
            maSheetData.setRowFormat( nRow, rModel.mnXfId, rModel.mbCustomFormat );
            
            maSheetData.setColSpans( nRow, rModel.maColSpans );
        }
    }

    UpdateRowProgress( maUsedArea, nRow );
}


void WorksheetGlobals::UpdateRowProgress( const CellRangeAddress& rUsedArea, sal_Int32 nRow )
{
    if (!mxRowProgress || nRow < rUsedArea.StartRow || rUsedArea.EndRow < nRow)
        return;

    double fNewPos = static_cast<double>(nRow - rUsedArea.StartRow + 1.0) / (rUsedArea.EndRow - rUsedArea.StartRow + 1.0);

    if (mbFastRowProgress)
        mxRowProgress->setPosition(fNewPos);
    else
    {
        double fCurPos = mxRowProgress->getPosition();
        if (fCurPos < fNewPos && (fNewPos - fCurPos) > 0.3)
            
            mxRowProgress->setPosition(fNewPos);
    }
}


void WorksheetGlobals::initializeWorksheetImport()
{
    
    ScDocumentImport& rDoc = getDocImport();

    ScStyleSheet* pStyleSheet =
        static_cast<ScStyleSheet*>(rDoc.getDoc().GetStyleSheetPool()->Find(
            getStyles().getDefaultStyleName(), SFX_STYLE_FAMILY_PARA));

    if (pStyleSheet)
        rDoc.setCellStyleToSheet(getSheetIndex(), *pStyleSheet);

    /*  Remember the current sheet index in global data, needed by global
        objects, e.g. the chart converter. */
    setCurrentSheetIndex( getSheetIndex() );
}

void WorksheetGlobals::finalizeWorksheetImport()
{
    lclUpdateProgressBar( mxRowProgress, 1.0 );
    maSheetData.finalizeImport();
    
    
    getTables().applyAutoFilters();

    getCondFormats().finalizeImport();
    lclUpdateProgressBar( mxFinalProgress, 0.25 );
    finalizeHyperlinkRanges();
    finalizeValidationRanges();
    maAutoFilters.finalizeImport( getSheetIndex() );
    maQueryTables.finalizeImport();
    maSheetSett.finalizeImport();
    maPageSett.finalizeImport();
    maSheetViewSett.finalizeImport();

    lclUpdateProgressBar( mxFinalProgress, 0.5 );
    convertColumns();
    convertRows();
    lclUpdateProgressBar( mxFinalProgress, 1.0 );
}

void WorksheetGlobals::finalizeDrawingImport()
{
    finalizeDrawings();

    
    setCurrentSheetIndex( -1 );
}



void WorksheetGlobals::finalizeHyperlinkRanges()
{
    for( HyperlinkModelList::const_iterator aIt = maHyperlinks.begin(), aEnd = maHyperlinks.end(); aIt != aEnd; ++aIt )
    {
        OUString aUrl = getHyperlinkUrl( *aIt );
        
        if( !aUrl.isEmpty() )
            for( CellAddress aAddress( getSheetIndex(), aIt->maRange.StartColumn, aIt->maRange.StartRow ); aAddress.Row <= aIt->maRange.EndRow; ++aAddress.Row )
                for( aAddress.Column = aIt->maRange.StartColumn; aAddress.Column <= aIt->maRange.EndColumn; ++aAddress.Column )
                    insertHyperlink( aAddress, aUrl );
    }
}

OUString WorksheetGlobals::getHyperlinkUrl( const HyperlinkModel& rHyperlink ) const
{
    OUStringBuffer aUrlBuffer;
    if( !rHyperlink.maTarget.isEmpty() )
        aUrlBuffer.append( getBaseFilter().getAbsoluteUrl( rHyperlink.maTarget ) );
    if( !rHyperlink.maLocation.isEmpty() )
        aUrlBuffer.append( '#' ).append( rHyperlink.maLocation );
    OUString aUrl = aUrlBuffer.makeStringAndClear();

    
    if( aUrl.startsWith("#") )
    {
        sal_Int32 nSepPos = aUrl.lastIndexOf( '!' );
        if( nSepPos > 0 )
        {
            
            aUrl = aUrl.replaceAt( nSepPos, 1, OUString( '.' ) );
            
            OUString aSheetName = aUrl.copy( 1, nSepPos - 1 );
            OUString aCalcName = getWorksheets().getCalcSheetName( aSheetName );
            if( !aCalcName.isEmpty() )
                aUrl = aUrl.replaceAt( 1, nSepPos - 1, aCalcName );
        }
    }

    return aUrl;
}

void WorksheetGlobals::insertHyperlink( const CellAddress& rAddress, const OUString& rUrl )
{
    ScDocumentImport& rDoc = getDocImport();
    ScAddress aPos(rAddress.Column, rAddress.Row, rAddress.Sheet);
    ScRefCellValue aCell;
    aCell.assign(rDoc.getDoc(), aPos);

    if (aCell.meType == CELLTYPE_STRING || aCell.meType == CELLTYPE_EDIT)
    {
        OUString aStr = aCell.getString(&rDoc.getDoc());
        ScFieldEditEngine& rEE = rDoc.getDoc().GetEditEngine();
        rEE.Clear();

        SvxURLField aURLField(rUrl, aStr, SVXURLFORMAT_REPR);
        SvxFieldItem aURLItem(aURLField, EE_FEATURE_FIELD);
        rEE.QuickInsertField(aURLItem, ESelection());

        rDoc.setEditCell(aPos, rEE.CreateTextObject());
    }
    else
    {
        
        
        
        
        
        
        
        
        

        SfxStringItem aItem(ATTR_HYPERLINK, rUrl);
        rDoc.getDoc().ApplyAttr(rAddress.Column, rAddress.Row, rAddress.Sheet, aItem);
    }
}

void WorksheetGlobals::finalizeValidationRanges() const
{
    for( ValidationModelList::const_iterator aIt = maValidations.begin(), aEnd = maValidations.end(); aIt != aEnd; ++aIt )
    {
        PropertySet aPropSet( getCellRangeList( aIt->maRanges ) );

        Reference< XPropertySet > xValidation( aPropSet.getAnyProperty( PROP_Validation ), UNO_QUERY );
        if( xValidation.is() )
        {
            PropertySet aValProps( xValidation );

            try
            {
                sal_Int32 nIndex = 0;
                OUString aToken = aIt->msRef.getToken( 0, ' ', nIndex );

                Reference<XSpreadsheet> xSheet = getSheetFromDoc( getCurrentSheetIndex() );
                Reference<XCellRange> xDBCellRange;
                Reference<XCell> xCell;
                xDBCellRange = xSheet->getCellRangeByName( aToken );

                xCell = xDBCellRange->getCellByPosition( 0, 0 );
                Reference<XCellAddressable> xCellAddressable( xCell, UNO_QUERY_THROW );
                CellAddress aFirstCell = xCellAddressable->getCellAddress();
                Reference<XSheetCondition> xCondition( xValidation, UNO_QUERY_THROW );
                xCondition->setSourcePosition( aFirstCell );
            }
            catch(const Exception&)
            {
            }

            
            ValidationType eType = ValidationType_ANY;
            switch( aIt->mnType )
            {
                case XML_custom:        eType = ValidationType_CUSTOM;      break;
                case XML_date:          eType = ValidationType_DATE;        break;
                case XML_decimal:       eType = ValidationType_DECIMAL;     break;
                case XML_list:          eType = ValidationType_LIST;        break;
                case XML_none:          eType = ValidationType_ANY;         break;
                case XML_textLength:    eType = ValidationType_TEXT_LEN;    break;
                case XML_time:          eType = ValidationType_TIME;        break;
                case XML_whole:         eType = ValidationType_WHOLE;       break;
                default:    OSL_FAIL( "WorksheetData::finalizeValidationRanges - unknown validation type" );
            }
            aValProps.setProperty( PROP_Type, eType );

            
            ValidationAlertStyle eAlertStyle = ValidationAlertStyle_STOP;
            switch( aIt->mnErrorStyle )
            {
                case XML_information:   eAlertStyle = ValidationAlertStyle_INFO;    break;
                case XML_stop:          eAlertStyle = ValidationAlertStyle_STOP;    break;
                case XML_warning:       eAlertStyle = ValidationAlertStyle_WARNING; break;
                default:    OSL_FAIL( "WorksheetData::finalizeValidationRanges - unknown error style" );
            }
            aValProps.setProperty( PROP_ErrorAlertStyle, eAlertStyle );

            
            sal_Int16 nVisibility = aIt->mbNoDropDown ? TableValidationVisibility::INVISIBLE : TableValidationVisibility::UNSORTED;
            aValProps.setProperty( PROP_ShowList, nVisibility );

            
            aValProps.setProperty( PROP_ShowInputMessage, aIt->mbShowInputMsg );
            aValProps.setProperty( PROP_InputTitle, aIt->maInputTitle );
            aValProps.setProperty( PROP_InputMessage, aIt->maInputMessage );
            aValProps.setProperty( PROP_ShowErrorMessage, aIt->mbShowErrorMsg );
            aValProps.setProperty( PROP_ErrorTitle, aIt->maErrorTitle );
            aValProps.setProperty( PROP_ErrorMessage, aIt->maErrorMessage );

            
            aValProps.setProperty( PROP_IgnoreBlankCells, aIt->mbAllowBlank );

            try
            {
                
                Reference< XSheetCondition2 > xSheetCond( xValidation, UNO_QUERY_THROW );
                xSheetCond->setConditionOperator( CondFormatBuffer::convertToApiOperator( aIt->mnOperator ) );

                
                Reference< XMultiFormulaTokens > xTokens( xValidation, UNO_QUERY_THROW );
                xTokens->setTokens( 0, aIt->maTokens1 );
                xTokens->setTokens( 1, aIt->maTokens2 );
            }
            catch( Exception& )
            {
            }

            
            aPropSet.setProperty( PROP_Validation, xValidation );
        }
    }
}

void WorksheetGlobals::convertColumns()
{
    sal_Int32 nNextCol = 0;
    sal_Int32 nMaxCol = mrMaxApiPos.Column;
    
    OutlineLevelVec aColLevels;

    for( ColumnModelRangeMap::iterator aIt = maColModels.begin(), aEnd = maColModels.end(); aIt != aEnd; ++aIt )
    {
        
        ValueRange aColRange( ::std::max( aIt->first, nNextCol ), ::std::min( aIt->second.second, nMaxCol ) );
        
        if( nNextCol < aColRange.mnFirst )
            convertColumns( aColLevels, ValueRange( nNextCol, aColRange.mnFirst - 1 ), maDefColModel );
        
        convertColumns( aColLevels, aColRange, aIt->second.first );
        
        nNextCol = aColRange.mnLast + 1;
    }

    
    convertColumns( aColLevels, ValueRange( nNextCol, nMaxCol ), maDefColModel );
    
    convertOutlines( aColLevels, nMaxCol + 1, 0, false, false );
}

void WorksheetGlobals::convertColumns( OutlineLevelVec& orColLevels,
        const ValueRange& rColRange, const ColumnModel& rModel )
{
    
    sal_Int32 nWidth = getUnitConverter().scaleToMm100( rModel.mfWidth, UNIT_DIGIT );
    
    if( meSheetType == SHEETTYPE_MACROSHEET )
        nWidth *= 2;

    SCTAB nTab = getSheetIndex();
    ScDocument& rDoc = getScDocument();
    SCCOL nStartCol = rColRange.mnFirst;
    SCCOL nEndCol = rColRange.mnLast;

    if( nWidth > 0 )
    {
        for( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
        {
            rDoc.SetColWidthOnly( nCol, nTab, (sal_uInt16)sc::HMMToTwips( nWidth ) );
        }
    }

    
    if( rModel.mbHidden )
    {
        rDoc.SetColHidden( nStartCol, nEndCol, nTab, true );
    }

    
    convertOutlines( orColLevels, rColRange.mnFirst, rModel.mnLevel, rModel.mbCollapsed, false );
}

void WorksheetGlobals::convertRows()
{
    sal_Int32 nNextRow = 0;
    sal_Int32 nMaxRow = mrMaxApiPos.Row;
    
    OutlineLevelVec aRowLevels;

    for( RowModelRangeMap::iterator aIt = maRowModels.begin(), aEnd = maRowModels.end(); aIt != aEnd; ++aIt )
    {
        
        ValueRange aRowRange( ::std::max( aIt->first, nNextRow ), ::std::min( aIt->second.second, nMaxRow ) );
        
        if( nNextRow < aRowRange.mnFirst )
            convertRows( aRowLevels, ValueRange( nNextRow, aRowRange.mnFirst - 1 ), maDefRowModel );
        
        convertRows( aRowLevels, aRowRange, aIt->second.first, maDefRowModel.mfHeight );
        
        nNextRow = aRowRange.mnLast + 1;
    }

    
    convertRows( aRowLevels, ValueRange( nNextRow, nMaxRow ), maDefRowModel );
    
    convertOutlines( aRowLevels, nMaxRow + 1, 0, false, true );
}

void WorksheetGlobals::convertRows( OutlineLevelVec& orRowLevels,
        const ValueRange& rRowRange, const RowModel& rModel, double fDefHeight )
{
    
    double fHeight = (rModel.mfHeight >= 0.0) ? rModel.mfHeight : fDefHeight;
    sal_Int32 nHeight = getUnitConverter().scaleToMm100( fHeight, UNIT_POINT );
    SCROW nStartRow = rRowRange.mnFirst;
    SCROW nEndRow = rRowRange.mnLast;
    SCTAB nTab = getSheetIndex();
    if( nHeight > 0 )
    {
        /* always import the row height, ensures better layout */
        ScDocument& rDoc = getScDocument();
        rDoc.SetRowHeightOnly( nStartRow, nEndRow, nTab, (sal_uInt16)sc::HMMToTwips(nHeight) );
        if(rModel.mbCustomHeight)
            rDoc.SetManualHeight( nStartRow, nEndRow, nTab, true );
    }

    
    if( rModel.mbHidden )
    {
        ScDocument& rDoc = getScDocument();
        rDoc.SetRowHidden( nStartRow, nEndRow, nTab, true );
    }

    
    convertOutlines( orRowLevels, rRowRange.mnFirst, rModel.mnLevel, rModel.mbCollapsed, true );
}

void WorksheetGlobals::convertOutlines( OutlineLevelVec& orLevels,
        sal_Int32 nColRow, sal_Int32 nLevel, bool bCollapsed, bool bRows )
{
    /*  It is ensured from caller functions, that this function is called
        without any gaps between the processed column or row ranges. */

    OSL_ENSURE( nLevel >= 0, "WorksheetGlobals::convertOutlines - negative outline level" );
    nLevel = ::std::max< sal_Int32 >( nLevel, 0 );

    sal_Int32 nSize = orLevels.size();
    if( nSize < nLevel )
    {
        
        for( sal_Int32 nIndex = nSize; nIndex < nLevel; ++nIndex )
            orLevels.push_back( nColRow );
    }
    else if( nLevel < nSize )
    {
        
        for( sal_Int32 nIndex = nLevel; nIndex < nSize; ++nIndex )
        {
            sal_Int32 nFirstInLevel = orLevels.back();
            orLevels.pop_back();
            groupColumnsOrRows( nFirstInLevel, nColRow - 1, bCollapsed, bRows );
            bCollapsed = false; 
        }
    }
}

void WorksheetGlobals::groupColumnsOrRows( sal_Int32 nFirstColRow, sal_Int32 nLastColRow, bool bCollapse, bool bRows )
{
    try
    {
        Reference< XSheetOutline > xOutline( mxSheet, UNO_QUERY_THROW );
        if( bRows )
        {
            CellRangeAddress aRange( getSheetIndex(), 0, nFirstColRow, 0, nLastColRow );
            xOutline->group( aRange, TableOrientation_ROWS );
            if( bCollapse )
                xOutline->hideDetail( aRange );
        }
        else
        {
            CellRangeAddress aRange( getSheetIndex(), nFirstColRow, 0, nLastColRow, 0 );
            xOutline->group( aRange, TableOrientation_COLUMNS );
            if( bCollapse )
                xOutline->hideDetail( aRange );
        }
    }
    catch( Exception& )
    {
    }
}

void WorksheetGlobals::finalizeDrawings()
{
    
    PropertySet aRangeProp( getCellRange( CellRangeAddress( getSheetIndex(), 0, 0, mrMaxApiPos.Column, mrMaxApiPos.Row ) ) );
    aRangeProp.getProperty( maDrawPageSize, PROP_Size );

    switch( getFilterType() )
    {
        case FILTER_OOXML:
            
            if( !maDrawingPath.isEmpty() )
                importOoxFragment( new DrawingFragment( *this, maDrawingPath ) );
            if( !maVmlDrawingPath.isEmpty() )
                importOoxFragment( new VmlDrawingFragment( *this, maVmlDrawingPath ) );
        break;

        case FILTER_BIFF:
            
            getBiffDrawing().finalizeImport();
        break;

        case FILTER_UNKNOWN:
        break;
    }

    
    maComments.finalizeImport();

    /*  Extend used area of the sheet by cells covered with drawing objects.
        Needed if the imported document is inserted as "OLE object from file"
        and thus does not provide an OLE size property by itself. */
    if( (maShapeBoundingBox.Width > 0) || (maShapeBoundingBox.Height > 0) )
        extendUsedArea( getCellRangeFromRectangle( maShapeBoundingBox ) );

    
    if( maUsedArea.StartColumn > maUsedArea.EndColumn )
        maUsedArea.StartColumn = maUsedArea.EndColumn = 0;
    if( maUsedArea.StartRow > maUsedArea.EndRow )
        maUsedArea.StartRow = maUsedArea.EndRow = 0;

    /*  Register the used area of this sheet in global view settings. The
        global view settings will set the visible area if this document is an
        embedded OLE object. */
    getViewSettings().setSheetUsedArea( maUsedArea );

    /*  #i103686# Set right-to-left sheet layout. Must be done after all
        drawing shapes to simplify calculation of shape coordinates. */
    if( maSheetViewSett.isSheetRightToLeft() )
    {
        PropertySet aPropSet( mxSheet );
        aPropSet.setProperty( PROP_TableLayout, WritingMode2::RL_TB );
    }
}




WorksheetHelper::WorksheetHelper( WorksheetGlobals& rSheetGlob ) :
    WorkbookHelper( rSheetGlob ),
    mrSheetGlob( rSheetGlob )
{
}

/*static*/ WorksheetGlobalsRef WorksheetHelper::constructGlobals( const WorkbookHelper& rHelper,
        const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet )
{
    WorksheetGlobalsRef xSheetGlob( new WorksheetGlobals( rHelper, rxProgressBar, eSheetType, nSheet ) );
    if( !xSheetGlob->isValidSheet() )
        xSheetGlob.reset();
    return xSheetGlob;
}

/* static */ IWorksheetProgress *WorksheetHelper::getWorksheetInterface( const WorksheetGlobalsRef &xRef )
{
    return static_cast< IWorksheetProgress *>( xRef.get() );
}

WorksheetType WorksheetHelper::getSheetType() const
{
    return mrSheetGlob.getSheetType();
}

sal_Int16 WorksheetHelper::getSheetIndex() const
{
    return mrSheetGlob.getSheetIndex();
}

const Reference< XSpreadsheet >& WorksheetHelper::getSheet() const
{
    return mrSheetGlob.getSheet();
}

Reference< XCell > WorksheetHelper::getCell( const CellAddress& rAddress ) const
{
    return mrSheetGlob.getCell( rAddress );
}

Reference< XCellRange > WorksheetHelper::getCellRange( const CellRangeAddress& rRange ) const
{
    return mrSheetGlob.getCellRange( rRange );
}

Reference< XDrawPage > WorksheetHelper::getDrawPage() const
{
    return mrSheetGlob.getDrawPage();
}

awt::Point WorksheetHelper::getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return mrSheetGlob.getCellPosition( nCol, nRow );
}

awt::Size WorksheetHelper::getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return mrSheetGlob.getCellSize( nCol, nRow );
}

awt::Size WorksheetHelper::getDrawPageSize() const
{
    return mrSheetGlob.getDrawPageSize();
}

SheetDataBuffer& WorksheetHelper::getSheetData() const
{
    return mrSheetGlob.getSheetData();
}

CondFormatBuffer& WorksheetHelper::getCondFormats() const
{
    return mrSheetGlob.getCondFormats();
}

CommentsBuffer& WorksheetHelper::getComments() const
{
    return mrSheetGlob.getComments();
}

AutoFilterBuffer& WorksheetHelper::getAutoFilters() const
{
    return mrSheetGlob.getAutoFilters();
}

QueryTableBuffer& WorksheetHelper::getQueryTables() const
{
    return mrSheetGlob.getQueryTables();
}

WorksheetSettings& WorksheetHelper::getWorksheetSettings() const
{
    return mrSheetGlob.getWorksheetSettings();
}

PageSettings& WorksheetHelper::getPageSettings() const
{
    return mrSheetGlob.getPageSettings();
}

SheetViewSettings& WorksheetHelper::getSheetViewSettings() const
{
    return mrSheetGlob.getSheetViewSettings();
}

VmlDrawing& WorksheetHelper::getVmlDrawing() const
{
    return mrSheetGlob.getVmlDrawing();
}

ExtLst& WorksheetHelper::getExtLst() const
{
    return mrSheetGlob.getExtLst();
}

void WorksheetHelper::setPageBreak( const PageBreakModel& rModel, bool bRowBreak )
{
    mrSheetGlob.setPageBreak( rModel, bRowBreak );
}

void WorksheetHelper::setHyperlink( const HyperlinkModel& rModel )
{
    mrSheetGlob.setHyperlink( rModel );
}

void WorksheetHelper::setValidation( const ValidationModel& rModel )
{
    mrSheetGlob.setValidation( rModel );
}

void WorksheetHelper::setDrawingPath( const OUString& rDrawingPath )
{
    mrSheetGlob.setDrawingPath( rDrawingPath );
}

void WorksheetHelper::setVmlDrawingPath( const OUString& rVmlDrawingPath )
{
    mrSheetGlob.setVmlDrawingPath( rVmlDrawingPath );
}

void WorksheetHelper::extendUsedArea( const CellAddress& rAddress )
{
    mrSheetGlob.extendUsedArea( rAddress );
}

void WorksheetHelper::extendUsedArea( const CellRangeAddress& rRange )
{
    mrSheetGlob.extendUsedArea( rRange );
}

void WorksheetHelper::extendShapeBoundingBox( const awt::Rectangle& rShapeRect )
{
    mrSheetGlob.extendShapeBoundingBox( rShapeRect );
}

void WorksheetHelper::setBaseColumnWidth( sal_Int32 nWidth )
{
    mrSheetGlob.setBaseColumnWidth( nWidth );
}

void WorksheetHelper::setDefaultColumnWidth( double fWidth )
{
    mrSheetGlob.setDefaultColumnWidth( fWidth );
}

void WorksheetHelper::setColumnModel( const ColumnModel& rModel )
{
    mrSheetGlob.setColumnModel( rModel );
}

void WorksheetHelper::setDefaultRowSettings( double fHeight, bool bCustomHeight, bool bHidden, bool bThickTop, bool bThickBottom )
{
    mrSheetGlob.setDefaultRowSettings( fHeight, bCustomHeight, bHidden, bThickTop, bThickBottom );
}

void WorksheetHelper::setRowModel( const RowModel& rModel )
{
    mrSheetGlob.setRowModel( rModel );
}

void WorksheetHelper::putValue( const CellAddress& rAddress, double fValue )
{
    ScAddress aAddress;
    ScUnoConversion::FillScAddress( aAddress, rAddress );
    getDocImport().setNumericCell(aAddress, fValue);
}

void WorksheetHelper::setCellFormulaValue( const ::com::sun::star::table::CellAddress& rAddress,
                            double fValue  )
{
    getFormulaBuffer().setCellFormulaValue( rAddress, fValue );
}

void WorksheetHelper::putString( const CellAddress& rAddress, const OUString& rText )
{
    ScAddress aAddress;
    ScUnoConversion::FillScAddress( aAddress, rAddress );
    if ( !rText.isEmpty() )
        getDocImport().setStringCell(aAddress, rText);
}

void WorksheetHelper::putRichString( const CellAddress& rAddress, const RichString& rString, const Font* pFirstPortionFont )
{
    ScEditEngineDefaulter& rEE = getEditEngine();

    
    ScAddress aAddress;
    ScUnoConversion::FillScAddress( aAddress, rAddress );
    getDocImport().setEditCell(aAddress, rString.convert(rEE, pFirstPortionFont));
}

void WorksheetHelper::putFormulaTokens( const CellAddress& rAddress, const ApiTokenSequence& rTokens )
{
    ScDocumentImport& rDoc = getDocImport();
    ScTokenArray aTokenArray;
    ScAddress aCellPos;
    ScUnoConversion::FillScAddress( aCellPos, rAddress );
    ScTokenConversion::ConvertToTokenArray(rDoc.getDoc(), aTokenArray, rTokens);
    rDoc.setFormulaCell(aCellPos, new ScTokenArray(aTokenArray));
}

void WorksheetHelper::initializeWorksheetImport()
{
    mrSheetGlob.initializeWorksheetImport();
}

void WorksheetHelper::finalizeWorksheetImport()
{
    mrSheetGlob.finalizeWorksheetImport();
}

void WorksheetHelper::finalizeDrawingImport()
{
    mrSheetGlob.finalizeDrawingImport();
}

void WorksheetHelper::setCellFormula( const ::com::sun::star::table::CellAddress& rTokenAddress, const OUString& rTokenStr )
{
    getFormulaBuffer().setCellFormula( rTokenAddress,  rTokenStr );
}

void WorksheetHelper::setCellFormula(
    const ::com::sun::star::table::CellAddress& rAddr, sal_Int32 nSharedId,
    const OUString& rCellValue, sal_Int32 nValueType )
{
    getFormulaBuffer().setCellFormula(rAddr, nSharedId, rCellValue, nValueType);
}

void WorksheetHelper::setCellArrayFormula( const ::com::sun::star::table::CellRangeAddress& rRangeAddress, const ::com::sun::star::table::CellAddress& rTokenAddress, const OUString& rTokenStr )
{
    getFormulaBuffer().setCellArrayFormula( rRangeAddress,  rTokenAddress, rTokenStr );
}

void WorksheetHelper::createSharedFormulaMapEntry(
    const table::CellAddress& rAddress, const table::CellRangeAddress& rRange, sal_Int32 nSharedId, const OUString& rTokens )
{
    getFormulaBuffer().createSharedFormulaMapEntry(rAddress, rRange, nSharedId, rTokens);
}




} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
