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

#include "sheetdatabuffer.hxx"

#include <algorithm>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XMultipleOperation.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <rtl/ustrbuf.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/editobj.hxx>
#include <svl/eitem.hxx>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/tokens.hxx"
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "formulaparser.hxx"
#include "sharedstringsbuffer.hxx"
#include "unitconverter.hxx"
#include "convuno.hxx"
#include "markdata.hxx"
#include "rangelst.hxx"
#include "document.hxx"
#include "scitems.hxx"
#include "formulacell.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;


// ============================================================================

CellModel::CellModel() :
    mnCellType( XML_TOKEN_INVALID ),
    mnXfId( -1 ),
    mbShowPhonetic( false )
{
}

// ----------------------------------------------------------------------------

CellFormulaModel::CellFormulaModel() :
    mnFormulaType( XML_TOKEN_INVALID ),
    mnSharedId( -1 )
{
}

bool CellFormulaModel::isValidArrayRef( const CellAddress& rCellAddr )
{
    return
        (maFormulaRef.Sheet == rCellAddr.Sheet) &&
        (maFormulaRef.StartColumn == rCellAddr.Column) &&
        (maFormulaRef.StartRow == rCellAddr.Row);
}

bool CellFormulaModel::isValidSharedRef( const CellAddress& rCellAddr )
{
    return
        (maFormulaRef.Sheet == rCellAddr.Sheet) &&
        (maFormulaRef.StartColumn <= rCellAddr.Column) && (rCellAddr.Column <= maFormulaRef.EndColumn) &&
        (maFormulaRef.StartRow <= rCellAddr.Row) && (rCellAddr.Row <= maFormulaRef.EndRow);
}

// ----------------------------------------------------------------------------

DataTableModel::DataTableModel() :
    mb2dTable( false ),
    mbRowTable( false ),
    mbRef1Deleted( false ),
    mbRef2Deleted( false )
{
}

// ============================================================================

CellBlockBuffer::CellBlockBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    mnCurrRow( -1 )
{
}

void CellBlockBuffer::setColSpans( sal_Int32 nRow, const ValueRangeSet& rColSpans )
{
    OSL_ENSURE( maColSpans.count( nRow ) == 0, "CellBlockBuffer::setColSpans - multiple column spans for the same row" );
    OSL_ENSURE( (mnCurrRow < nRow) && (maColSpans.empty() || (maColSpans.rbegin()->first < nRow)), "CellBlockBuffer::setColSpans - rows are unsorted" );
    if( (mnCurrRow < nRow) && (maColSpans.count( nRow ) == 0) )
        maColSpans[ nRow ] = rColSpans.getRanges();
}

void CellBlockBuffer::finalizeImport()
{
}

// ============================================================================

SheetDataBuffer::SheetDataBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    maCellBlocks( rHelper ),
    mbPendingSharedFmla( false )
{
}

void SheetDataBuffer::setColSpans( sal_Int32 nRow, const ValueRangeSet& rColSpans )
{
    maCellBlocks.setColSpans( nRow, rColSpans );
}

void SheetDataBuffer::setBlankCell( const CellModel& rModel )
{
    setCellFormat( rModel );
}

void SheetDataBuffer::setValueCell( const CellModel& rModel, double fValue )
{
    putValue( rModel.maCellAddr, fValue );
    setCellFormat( rModel );
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, const OUString& rText )
{
    putString( rModel.maCellAddr, rText );
    setCellFormat( rModel );
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, const RichStringRef& rxString )
{
    OSL_ENSURE( rxString.get(), "SheetDataBuffer::setStringCell - missing rich string object" );
    const Font* pFirstPortionFont = getStyles().getFontFromCellXf( rModel.mnXfId ).get();
    OUString aText;
    if( rxString->extractPlainString( aText, pFirstPortionFont ) )
    {
        setStringCell( rModel, aText );
    }
    else
    {
        putRichString( rModel.maCellAddr, *rxString, pFirstPortionFont );
        setCellFormat( rModel );
    }
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, sal_Int32 nStringId )
{
    RichStringRef xString = getSharedStrings().getString( nStringId );
    if( xString.get() )
        setStringCell( rModel, xString );
    else
        setBlankCell( rModel );
}

void SheetDataBuffer::setDateTimeCell( const CellModel& rModel, const ::com::sun::star::util::DateTime& rDateTime )
{
    // write serial date/time value into the cell
    double fSerial = getUnitConverter().calcSerialFromDateTime( rDateTime );
    setValueCell( rModel, fSerial );
    // set appropriate number format
    using namespace ::com::sun::star::util::NumberFormat;
    sal_Int16 nStdFmt = (fSerial < 1.0) ? TIME : (((rDateTime.Hours > 0) || (rDateTime.Minutes > 0) || (rDateTime.Seconds > 0)) ? DATETIME : DATE);
    setStandardNumFmt( rModel.maCellAddr, nStdFmt );
}

void SheetDataBuffer::setBooleanCell( const CellModel& rModel, bool bValue )
{
    setCellFormula( rModel.maCellAddr, getFormulaParser().convertBoolToFormula( bValue ) );
    // #108770# set 'Standard' number format for all Boolean cells
    setCellFormat( rModel, 0 );
}

void SheetDataBuffer::setErrorCell( const CellModel& rModel, const OUString& rErrorCode )
{
    setErrorCell( rModel, getUnitConverter().calcBiffErrorCode( rErrorCode ) );
}

void SheetDataBuffer::setErrorCell( const CellModel& rModel, sal_uInt8 nErrorCode )
{
    setCellFormula( rModel.maCellAddr, getFormulaParser().convertErrorToFormula( nErrorCode ) );
    setCellFormat( rModel );
}

void SheetDataBuffer::setFormulaCell( const CellModel& rModel, const ApiTokenSequence& rTokens )
{
    mbPendingSharedFmla = false;
    ApiTokenSequence aTokens;

    /*  Detect special token passed as placeholder for array formulas, shared
        formulas, and table operations. In BIFF, these formulas are represented
        by a single tExp resp. tTbl token. If the formula parser finds these
        tokens, it puts a single OPCODE_BAD token with the base address and
        formula type into the token sequence. This information will be
        extracted here, and in case of a shared formula, the shared formula
        buffer will generate the resulting formula token array. */
    ApiSpecialTokenInfo aTokenInfo;
    if( rTokens.hasElements() && getFormulaParser().extractSpecialTokenInfo( aTokenInfo, rTokens ) )
    {
        /*  The second member of the token info is set to true, if the formula
            represents a table operation, which will be skipped. In BIFF12 it
            is not possible to distinguish array and shared formulas
            (BIFF5/BIFF8 provide this information with a special flag in the
            FORMULA record). */
        if( !aTokenInfo.Second )
        {
            /*  Construct the token array representing the shared formula. If
                the returned sequence is empty, the definition of the shared
                formula has not been loaded yet, or the cell is part of an
                array formula. In this case, the cell will be remembered. After
                reading the formula definition it will be retried to insert the
                formula via retryPendingSharedFormulaCell(). */
            BinAddress aBaseAddr( aTokenInfo.First );
            aTokens = resolveSharedFormula( aBaseAddr );
            if( !aTokens.hasElements() )
            {
                maSharedFmlaAddr = rModel.maCellAddr;
                maSharedBaseAddr = aBaseAddr;
                mbPendingSharedFmla = true;
            }
        }
    }
    else
    {
        // simple formula, use the passed token array
        aTokens = rTokens;
    }

    setCellFormula( rModel.maCellAddr, aTokens );
    setCellFormat( rModel );
}

void SheetDataBuffer::createArrayFormula( const CellRangeAddress& rRange, const ApiTokenSequence& rTokens )
{
    /*  Array formulas will be inserted later in finalizeImport(). This is
        needed to not disturb collecting all the cells, which will be put into
        the sheet in large blocks to increase performance. */
    maArrayFormulas.push_back( ArrayFormula( rRange, rTokens ) );
}

void SheetDataBuffer::createTableOperation( const CellRangeAddress& rRange, const DataTableModel& rModel )
{
    /*  Table operations will be inserted later in finalizeImport(). This is
        needed to not disturb collecting all the cells, which will be put into
        the sheet in large blocks to increase performance. */
    maTableOperations.push_back( TableOperation( rRange, rModel ) );
}

void SheetDataBuffer::setRowFormat( sal_Int32 nRow, sal_Int32 nXfId, bool bCustomFormat )
{
    // set row formatting
    if( bCustomFormat )
    {
        // try to expand cached row range, if formatting is equal
        if( (maXfIdRowRange.maRowRange.mnLast < 0) || !maXfIdRowRange.tryExpand( nRow, nXfId ) )
        {

            maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );
            maXfIdRowRange.set( nRow, nXfId );
        }
    }
    else if( maXfIdRowRange.maRowRange.mnLast >= 0 )
    {
        // finish last cached row range
        maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );
        maXfIdRowRange.set( -1, -1 );
    }
}

void SheetDataBuffer::setMergedRange( const CellRangeAddress& rRange )
{
    maMergedRanges.push_back( MergedRange( rRange ) );
}

void SheetDataBuffer::setStandardNumFmt( const CellAddress& rCellAddr, sal_Int16 nStdNumFmt )
{
    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( getDocument(), UNO_QUERY_THROW );
        Reference< XNumberFormatTypes > xNumFmtTypes( xNumFmtsSupp->getNumberFormats(), UNO_QUERY_THROW );
        sal_Int32 nIndex = xNumFmtTypes->getStandardFormat( nStdNumFmt, Locale() );
        PropertySet aPropSet( getCell( rCellAddr ) );
        aPropSet.setProperty( PROP_NumberFormat, nIndex );
    }
    catch( Exception& )
    {
    }
}

void addIfNotInMyMap( StylesBuffer& rStyles, std::map< std::pair< sal_Int32, sal_Int32 >, ApiCellRangeList >& rMap, sal_Int32 nXfId, sal_Int32 nFormatId, const ApiCellRangeList& rRangeList )
{
    Xf* pXf1 = rStyles.getCellXf( nXfId ).get();
    if ( pXf1 )
    {
        for ( std::map< std::pair< sal_Int32, sal_Int32 >, ApiCellRangeList >::iterator it = rMap.begin(), it_end = rMap.end(); it != it_end; ++it )
        {
            if ( it->first.second == nFormatId )
            {
                Xf* pXf2 = rStyles.getCellXf( it->first.first ).get();
                if ( *pXf1 == *pXf2 ) // already exists
                {
                    // add ranges from the rangelist to the existing rangelist for the
                    // matching style ( should we check if they overlap ? )
                    for ( ApiCellRangeList::const_iterator iter = rRangeList.begin(), iter_end =  rRangeList.end(); iter != iter_end; ++iter )
                       it->second.push_back( *iter );
                    return;
                }
            }
        }
        rMap[ std::pair<sal_Int32, sal_Int32>( nXfId, nFormatId ) ] = rRangeList;
    }
}

void SheetDataBuffer::addColXfStyle( sal_Int32 nXfId, sal_Int32 nFormatId, const ::com::sun::star::table::CellRangeAddress& rAddress, bool bProcessRowRange )
{
    RowRangeStyle aStyleRows;
    aStyleRows.mnNumFmt.first = nXfId;
    aStyleRows.mnNumFmt.second = nFormatId;
    aStyleRows.mnStartRow = rAddress.StartRow;
    aStyleRows.mnEndRow = rAddress.EndRow;
    for ( sal_Int32 nCol = rAddress.StartColumn; nCol <= rAddress.EndColumn; ++nCol )
    {
        if ( !bProcessRowRange )
            maStylesPerColumn[ nCol ].insert( aStyleRows );
        else
        {
            RowStyles& rRowStyles =  maStylesPerColumn[ nCol ];
            // If the rowrange style includes rows already
            // allocated to a style then we need to split
            // the range style Rows into sections ( to
            // occupy only rows that have no style definition )

            // We dont want to set any rowstyle 'rows'
            // for rows where there is an existing 'style' )
            std::vector< RowRangeStyle > aRangeRowsSplits;

            RowStyles::iterator rows_it = rRowStyles.begin();
            RowStyles::iterator rows_end = rRowStyles.end();
            bool bAddRange = true;
            for ( ; rows_it != rows_end; ++rows_it )
            {
                const RowRangeStyle& r = *rows_it;
                // if row is completely within existing style, discard it
                if ( aStyleRows.mnStartRow >= r.mnStartRow && aStyleRows.mnEndRow <= r.mnEndRow )
                    bAddRange = false;
                else if ( aStyleRows.mnStartRow <= r.mnStartRow )
                {
                    // not intersecting at all?, if so finish as none left
                    // to check ( row ranges are in ascending order
                    if ( aStyleRows.mnEndRow < r.mnStartRow )
                        break;
                    else if ( aStyleRows.mnEndRow <= r.mnEndRow )
                    {
                        aStyleRows.mnEndRow = r.mnStartRow - 1;
                        break;
                    }
                    if ( aStyleRows.mnStartRow < r.mnStartRow )
                    {
                        RowRangeStyle aSplit = aStyleRows;
                        aSplit.mnEndRow = r.mnStartRow - 1;
                        aRangeRowsSplits.push_back( aSplit );
                    }
                }
            }
            std::vector< RowRangeStyle >::iterator splits_it = aRangeRowsSplits.begin();
            std::vector< RowRangeStyle >::iterator splits_end = aRangeRowsSplits.end();
            for ( ; splits_it != splits_end; ++splits_it )
                rRowStyles.insert( *splits_it );
            if ( bAddRange )
                rRowStyles.insert( aStyleRows );
        }
    }
}
void SheetDataBuffer::finalizeImport()
{
    // insert all cells of all open cell blocks
    maCellBlocks.finalizeImport();

    // create all array formulas
    for( ArrayFormulaList::iterator aIt = maArrayFormulas.begin(), aEnd = maArrayFormulas.end(); aIt != aEnd; ++aIt )
        finalizeArrayFormula( aIt->first, aIt->second );

    // create all table operations
    for( TableOperationList::iterator aIt = maTableOperations.begin(), aEnd = maTableOperations.end(); aIt != aEnd; ++aIt )
        finalizeTableOperation( aIt->first, aIt->second );

    // write default formatting of remaining row range
    maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );

    std::map< std::pair< sal_Int32, sal_Int32 >, ApiCellRangeList > rangeStyleListMap;
    for( XfIdRangeListMap::const_iterator aIt = maXfIdRangeLists.begin(), aEnd = maXfIdRangeLists.end(); aIt != aEnd; ++aIt )
    {
        addIfNotInMyMap( getStyles(), rangeStyleListMap, aIt->first.first, aIt->first.second, aIt->second );
    }
    // gather all ranges that have the same style and apply them in bulk
    for (  std::map< std::pair< sal_Int32, sal_Int32 >, ApiCellRangeList >::iterator it = rangeStyleListMap.begin(), it_end = rangeStyleListMap.end(); it != it_end; ++it )
    {
        const ApiCellRangeList& rRanges( it->second );
        for ( ApiCellRangeList::const_iterator it_range = rRanges.begin(), it_rangeend = rRanges.end(); it_range!=it_rangeend; ++it_range )
            addColXfStyle( it->first.first, it->first.second, *it_range );
    }

    for ( std::map< sal_Int32, std::vector< ValueRange > >::iterator it = maXfIdRowRangeList.begin(), it_end =  maXfIdRowRangeList.end(); it != it_end; ++it )
    {
        ApiCellRangeList rangeList;
        AddressConverter& rAddrConv = getAddressConverter();
        // get all row ranges for id
        for ( std::vector< ValueRange >::iterator rangeIter = it->second.begin(), rangeIter_end = it->second.end(); rangeIter != rangeIter_end; ++rangeIter )
        {
            if ( it->first == -1 ) // it's a dud skip it
                continue;
            CellRangeAddress aRange( getSheetIndex(), 0, rangeIter->mnFirst, rAddrConv.getMaxApiAddress().Column, rangeIter->mnLast );

            addColXfStyle( it->first, -1, aRange, true );
        }
    }

    ScDocument& rDoc = getScDocument();
    StylesBuffer& rStyles = getStyles();
    for ( ColStyles::iterator col = maStylesPerColumn.begin(), col_end = maStylesPerColumn.end(); col != col_end; ++col )
    {
        RowStyles& rRowStyles = col->second;
        std::list<ScAttrEntry> aAttrs;
        SCCOL nScCol = static_cast< SCCOL >( col->first );
        for ( RowStyles::iterator rRows = rRowStyles.begin(), rRows_end = rRowStyles.end(); rRows != rRows_end; ++rRows )
        {
             Xf* pXf = rStyles.getCellXf( rRows->mnNumFmt.first ).get();

             if ( pXf )
                 pXf->applyPatternToAttrList( aAttrs,  rRows->mnStartRow,  rRows->mnEndRow,  rRows->mnNumFmt.second );
        }
        if (aAttrs.empty() || aAttrs.back().nRow != MAXROW)
        {
            ScAttrEntry aEntry;
            aEntry.nRow = MAXROW;
            aEntry.pPattern = rDoc.GetDefPattern();
            aAttrs.push_back(aEntry);
        }

        size_t nAttrSize = aAttrs.size();
        ScAttrEntry* pData = new ScAttrEntry[nAttrSize];
        std::list<ScAttrEntry>::const_iterator itr = aAttrs.begin(), itrEnd = aAttrs.end();
        for (size_t i = 0; itr != itrEnd; ++itr, ++i)
            pData[i] = *itr;

        rDoc.SetAttrEntries(nScCol, getSheetIndex(), pData, static_cast<SCSIZE>(nAttrSize));
    }

    // merge all cached merged ranges and update right/bottom cell borders
    for( MergedRangeList::iterator aIt = maMergedRanges.begin(), aEnd = maMergedRanges.end(); aIt != aEnd; ++aIt )
        applyCellMerging( aIt->maRange );
    for( MergedRangeList::iterator aIt = maCenterFillRanges.begin(), aEnd = maCenterFillRanges.end(); aIt != aEnd; ++aIt )
        applyCellMerging( aIt->maRange );
}

// private --------------------------------------------------------------------

SheetDataBuffer::XfIdRowRange::XfIdRowRange() :
    maRowRange( -1 ),
    mnXfId( -1 )
{
}

void SheetDataBuffer::XfIdRowRange::set( sal_Int32 nRow, sal_Int32 nXfId )
{
    maRowRange = ValueRange( nRow );
    mnXfId = nXfId;
}

bool SheetDataBuffer::XfIdRowRange::tryExpand( sal_Int32 nRow, sal_Int32 nXfId )
{
    if( mnXfId == nXfId )
    {
        if( maRowRange.mnLast + 1 == nRow )
        {
            ++maRowRange.mnLast;
            return true;
        }
        if( maRowRange.mnFirst == nRow + 1 )
        {
            --maRowRange.mnFirst;
            return true;
        }
    }
    return false;
}


SheetDataBuffer::MergedRange::MergedRange( const CellRangeAddress& rRange ) :
    maRange( rRange ),
    mnHorAlign( XML_TOKEN_INVALID )
{
}

SheetDataBuffer::MergedRange::MergedRange( const CellAddress& rAddress, sal_Int32 nHorAlign ) :
    maRange( rAddress.Sheet, rAddress.Column, rAddress.Row, rAddress.Column, rAddress.Row ),
    mnHorAlign( nHorAlign )
{
}

bool SheetDataBuffer::MergedRange::tryExpand( const CellAddress& rAddress, sal_Int32 nHorAlign )
{
    if( (mnHorAlign == nHorAlign) && (maRange.StartRow == rAddress.Row) &&
        (maRange.EndRow == rAddress.Row) && (maRange.EndColumn + 1 == rAddress.Column) )
    {
        ++maRange.EndColumn;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

void SheetDataBuffer::setCellFormula( const CellAddress& rCellAddr, const ApiTokenSequence& rTokens )
{
    if( rTokens.hasElements() )
    {
        putFormulaTokens( rCellAddr, rTokens );
    }
}

ApiTokenSequence SheetDataBuffer::resolveSharedFormula( const BinAddress& rMapKey ) const
{
    sal_Int32 nTokenIndex = ContainerHelper::getMapElement( maSharedFormulas, rMapKey, -1 );
    return (nTokenIndex >= 0) ? getFormulaParser().convertNameToFormula( nTokenIndex ) : ApiTokenSequence();
}

void SheetDataBuffer::finalizeArrayFormula( const CellRangeAddress& rRange, const ApiTokenSequence& rTokens ) const
{
    Reference< XArrayFormulaTokens > xTokens( getCellRange( rRange ), UNO_QUERY );
    OSL_ENSURE( xTokens.is(), "SheetDataBuffer::finalizeArrayFormula - missing formula token interface" );
    if( xTokens.is() )
        xTokens->setArrayTokens( rTokens );
}

void SheetDataBuffer::finalizeTableOperation( const CellRangeAddress& rRange, const DataTableModel& rModel ) const
{
    sal_Int16 nSheet = getSheetIndex();
    bool bOk = false;
    if( !rModel.mbRef1Deleted && !rModel.maRef1.isEmpty() && (rRange.StartColumn > 0) && (rRange.StartRow > 0) )
    {
        CellRangeAddress aOpRange = rRange;
        CellAddress aRef1;
        if( getAddressConverter().convertToCellAddress( aRef1, rModel.maRef1, nSheet, true ) ) try
        {
            if( rModel.mb2dTable )
            {
                CellAddress aRef2;
                if( !rModel.mbRef2Deleted && getAddressConverter().convertToCellAddress( aRef2, rModel.maRef2, nSheet, true ) )
                {
                    // API call expects input values inside operation range
                    --aOpRange.StartColumn;
                    --aOpRange.StartRow;
                    // formula range is top-left cell of operation range
                    CellRangeAddress aFormulaRange( nSheet, aOpRange.StartColumn, aOpRange.StartRow, aOpRange.StartColumn, aOpRange.StartRow );
                    // set multiple operation
                    Reference< XMultipleOperation > xMultOp( getCellRange( aOpRange ), UNO_QUERY_THROW );
                    xMultOp->setTableOperation( aFormulaRange, TableOperationMode_BOTH, aRef2, aRef1 );
                    bOk = true;
                }
            }
            else if( rModel.mbRowTable )
            {
                // formula range is column to the left of operation range
                CellRangeAddress aFormulaRange( nSheet, aOpRange.StartColumn - 1, aOpRange.StartRow, aOpRange.StartColumn - 1, aOpRange.EndRow );
                // API call expects input values (top row) inside operation range
                --aOpRange.StartRow;
                // set multiple operation
                Reference< XMultipleOperation > xMultOp( getCellRange( aOpRange ), UNO_QUERY_THROW );
                xMultOp->setTableOperation( aFormulaRange, TableOperationMode_ROW, aRef1, aRef1 );
                bOk = true;
            }
            else
            {
                // formula range is row above operation range
                CellRangeAddress aFormulaRange( nSheet, aOpRange.StartColumn, aOpRange.StartRow - 1, aOpRange.EndColumn, aOpRange.StartRow - 1 );
                // API call expects input values (left column) inside operation range
                --aOpRange.StartColumn;
                // set multiple operation
                Reference< XMultipleOperation > xMultOp( getCellRange( aOpRange ), UNO_QUERY_THROW );
                xMultOp->setTableOperation( aFormulaRange, TableOperationMode_COLUMN, aRef1, aRef1 );
                bOk = true;
            }
        }
        catch( Exception& )
        {
        }
    }

    // on error: fill cell range with #REF! error codes
    if( !bOk ) try
    {
        Reference< XCellRangeData > xCellRangeData( getCellRange( rRange ), UNO_QUERY_THROW );
        size_t nWidth = static_cast< size_t >( rRange.EndColumn - rRange.StartColumn + 1 );
        size_t nHeight = static_cast< size_t >( rRange.EndRow - rRange.StartRow + 1 );
        Matrix< Any > aErrorCells( nWidth, nHeight, Any( getFormulaParser().convertErrorToFormula( BIFF_ERR_REF ) ) );
        xCellRangeData->setDataArray( ContainerHelper::matrixToSequenceSequence( aErrorCells ) );
    }
    catch( Exception& )
    {
    }
}

void SheetDataBuffer::setCellFormat( const CellModel& rModel, sal_Int32 nNumFmtId )
{
    if( (rModel.mnXfId >= 0) || (nNumFmtId >= 0) )
    {
        ApiCellRangeList::reverse_iterator aIt = maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, nNumFmtId ) ].rbegin();
        ApiCellRangeList::reverse_iterator aItEnd = maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, nNumFmtId ) ].rend();
        /* The xlsx sheet data contains row wise information.
         * It is sufficient to check if the row range size is one
         */
        if(     aIt                 != aItEnd &&
                aIt->Sheet          == rModel.maCellAddr.Sheet &&
                aIt->StartRow       == aIt->EndRow &&
                aIt->StartRow       == rModel.maCellAddr.Row &&
                (aIt->EndColumn+1)  == rModel.maCellAddr.Column )
        {
            aIt->EndColumn++;       // Expand Column
        }
        else
        {
            maXfIdRangeLists[ XfIdNumFmtKey (rModel.mnXfId, nNumFmtId ) ].push_back(
                              CellRangeAddress( rModel.maCellAddr.Sheet, rModel.maCellAddr.Column, rModel.maCellAddr.Row,
                              rModel.maCellAddr.Column, rModel.maCellAddr.Row ) );
        }

        aIt = maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, nNumFmtId ) ].rbegin();
        aItEnd = maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, nNumFmtId ) ].rend();
        ApiCellRangeList::reverse_iterator aItM = aIt+1;
        while( aItM != aItEnd )
        {
            if( aIt->Sheet == aItM->Sheet )
            {
                /* Try to merge this with the previous range */
                if( aIt->StartRow == (aItM->EndRow + 1) &&
                        aIt->StartColumn == aItM->StartColumn &&
                        aIt->EndColumn == aItM->EndColumn)
                {
                    aItM->EndRow = aIt->EndRow;
                    maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, nNumFmtId ) ].pop_back();
                    break;
                }
                else if( aIt->StartRow > aItM->EndRow + 1 )
                    break; // Un-necessary to check with any other rows
            }
            else
                break;
            ++aItM;
        }

        // update merged ranges for 'center across selection' and 'fill'
        if( const Xf* pXf = getStyles().getCellXf( rModel.mnXfId ).get() )
        {
            sal_Int32 nHorAlign = pXf->getAlignment().getModel().mnHorAlign;
            if( (nHorAlign == XML_centerContinuous) || (nHorAlign == XML_fill) )
            {
                /*  start new merged range, if cell is not empty (#108781#),
                    or try to expand last range with empty cell */
                if( rModel.mnCellType != XML_TOKEN_INVALID )
                    maCenterFillRanges.push_back( MergedRange( rModel.maCellAddr, nHorAlign ) );
                else if( !maCenterFillRanges.empty() )
                    maCenterFillRanges.rbegin()->tryExpand( rModel.maCellAddr, nHorAlign );
            }
        }
    }
}

void lcl_SetBorderLine( ScDocument& rDoc, ScRange& rRange, SCTAB nScTab, sal_uInt16 nLine )
{
    SCCOL nFromScCol = (nLine == BOX_LINE_RIGHT) ? rRange.aEnd.Col() : rRange.aStart.Col();
    SCROW nFromScRow = (nLine == BOX_LINE_BOTTOM) ? rRange.aEnd.Row() : rRange.aStart.Row();

    const SvxBoxItem* pFromItem = static_cast< const SvxBoxItem* >(
        rDoc.GetAttr( nFromScCol, nFromScRow, nScTab, ATTR_BORDER ) );
    const SvxBoxItem* pToItem = static_cast< const SvxBoxItem* >(
        rDoc.GetAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, ATTR_BORDER ) );

    SvxBoxItem aNewItem( *pToItem );
    aNewItem.SetLine( pFromItem->GetLine( nLine ), nLine );
    rDoc.ApplyAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, aNewItem );
}

void SheetDataBuffer::applyCellMerging( const CellRangeAddress& rRange )
{
    bool bMultiCol = rRange.StartColumn < rRange.EndColumn;
    bool bMultiRow = rRange.StartRow < rRange.EndRow;

    ScRange aRange;
    ScUnoConversion::FillScRange( aRange, rRange );
    const ScAddress& rStart = aRange.aStart;
    const ScAddress& rEnd = aRange.aEnd;
    ScDocument& rDoc = getScDocument();
    // set correct right border
    if( bMultiCol )
        lcl_SetBorderLine( rDoc, aRange, getSheetIndex(), BOX_LINE_RIGHT );
        // set correct lower border
    if( bMultiRow )
        lcl_SetBorderLine( rDoc, aRange, getSheetIndex(), BOX_LINE_BOTTOM );
    // do merge
    if( bMultiCol || bMultiRow )
        rDoc.DoMerge( getSheetIndex(), rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row() );
    // #i93609# merged range in a single row: test if manual row height is needed
    if( !bMultiRow )
    {
        bool bTextWrap = static_cast< const SfxBoolItem* >( rDoc.GetAttr( rStart.Col(), rStart.Row(), rStart.Tab(), ATTR_LINEBREAK ) )->GetValue();
        if( !bTextWrap && (rDoc.GetCellType( rStart ) == CELLTYPE_EDIT) )
        {
            if (const EditTextObject* pEditObj = rDoc.GetEditText(rStart))
                bTextWrap = pEditObj->GetParagraphCount() > 1;
        }
    }
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
