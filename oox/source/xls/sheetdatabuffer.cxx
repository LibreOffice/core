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

#include "oox/xls/sheetdatabuffer.hxx"

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
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/tokens.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/unitconverter.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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

SheetDataBuffer::SheetDataBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    mbPendingSharedFmla( false )
{
}

void SheetDataBuffer::setValueCell( const CellAddress& rCellAddr, double fValue )
{
    Reference< XCell > xCell = getCell( rCellAddr );
    OSL_ENSURE( xCell.is(), "SheetDataBuffer::setValueCell - missing cell interface" );
    if( xCell.is() )
        xCell->setValue( fValue );
}

void SheetDataBuffer::setStringCell( const CellAddress& rCellAddr, const OUString& rText )
{
    Reference< XText > xText( getCell( rCellAddr ), UNO_QUERY );
    OSL_ENSURE( xText.is(), "SheetDataBuffer::setStringCell - missing text interface" );
    if( xText.is() )
        xText->setString( rText );
}

void SheetDataBuffer::setStringCell( const CellAddress& rCellAddr, const RichString& rString, sal_Int32 nXfId )
{
    Reference< XText > xText( getCell( rCellAddr ), UNO_QUERY );
    OSL_ENSURE( xText.is(), "SheetDataBuffer::setStringCell - missing text interface" );
    rString.convert( xText, nXfId );
    (void)rString;
    (void)nXfId;
}

void SheetDataBuffer::setStringCell( const CellAddress& rCellAddr, sal_Int32 nStringId, sal_Int32 nXfId )
{
    Reference< XText > xText( getCell( rCellAddr ), UNO_QUERY );
    OSL_ENSURE( xText.is(), "SheetDataBuffer::setStringCell - missing text interface" );
    getSharedStrings().convertString( xText, nStringId, nXfId );
    (void)nStringId;
    (void)nXfId;
}

void SheetDataBuffer::setDateTimeCell( const CellAddress& rCellAddr, const DateTime& rDateTime )
{
    // write serial date/time value into the cell
    double fSerial = getUnitConverter().calcSerialFromDateTime( rDateTime );
    setValueCell( rCellAddr, fSerial );
    // set appropriate number format
    using namespace ::com::sun::star::util::NumberFormat;
    sal_Int16 nStdFmt = (fSerial < 1.0) ? TIME : (((rDateTime.Hours > 0) || (rDateTime.Minutes > 0) || (rDateTime.Seconds > 0)) ? DATETIME : DATE);
    setStandardNumFmt( rCellAddr, nStdFmt );
}

void SheetDataBuffer::setBooleanCell( const CellAddress& rCellAddr, bool bValue )
{
    setFormulaCell( rCellAddr, getFormulaParser().convertBoolToFormula( bValue ) );
}

void SheetDataBuffer::setErrorCell( const CellAddress& rCellAddr, const OUString& rErrorCode )
{
    setErrorCell( rCellAddr, getUnitConverter().calcBiffErrorCode( rErrorCode ) );
}

void SheetDataBuffer::setErrorCell( const CellAddress& rCellAddr, sal_uInt8 nErrorCode )
{
    setFormulaCell( rCellAddr, getFormulaParser().convertErrorToFormula( nErrorCode ) );
}

void SheetDataBuffer::setFormulaCell( const CellAddress& rCellAddr, const ApiTokenSequence& rTokens )
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
                maSharedFmlaAddr = rCellAddr;
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

    if( aTokens.hasElements() )
    {
        Reference< XFormulaTokens > xTokens( getCell( rCellAddr ), UNO_QUERY );
        OSL_ENSURE( xTokens.is(), "SheetDataBuffer::setFormulaCell - missing formula interface" );
        if( xTokens.is() )
            xTokens->setTokens( aTokens );
    }
}

void SheetDataBuffer::setFormulaCell( const CellAddress& rCellAddr, sal_Int32 nSharedId )
{
    setFormulaCell( rCellAddr, resolveSharedFormula( BinAddress( nSharedId, 0 ) ) );
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

void SheetDataBuffer::createSharedFormula( sal_Int32 nSharedId, const ApiTokenSequence& rTokens )
{
    createSharedFormula( BinAddress( nSharedId, 0 ), rTokens );
}

void SheetDataBuffer::createSharedFormula( const CellAddress& rCellAddr, const ApiTokenSequence& rTokens )
{
    createSharedFormula( BinAddress( rCellAddr ), rTokens );
}

void SheetDataBuffer::setRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId, bool bCustomFormat )
{
    // set row formatting
    if( bCustomFormat )
    {
        // try to expand cached row range, if formatting is equal
        if( (maXfIdRowRange.mnLastRow < 0) || !maXfIdRowRange.tryExpand( nFirstRow, nLastRow, nXfId ) )
        {
            writeXfIdRowRangeProperties( maXfIdRowRange );
            maXfIdRowRange.set( nFirstRow, nLastRow, nXfId );
        }
    }
    else if( maXfIdRowRange.mnLastRow >= 0 )
    {
        // finish last cached row range
        writeXfIdRowRangeProperties( maXfIdRowRange );
        maXfIdRowRange.set( -1, -1, -1 );
    }
}

void SheetDataBuffer::setCellFormat( const CellModel& rModel, sal_Int32 nNumFmtId )
{
    if( (rModel.mnXfId >= 0) || (nNumFmtId >= 0) )
    {
        // try to merge existing ranges and to write some formatting properties
        if( !maXfIdRanges.empty() )
        {
            // get row index of last inserted cell
            sal_Int32 nLastRow = maXfIdRanges.rbegin()->second.maRange.StartRow;
            // row changed - try to merge ranges of last row with existing ranges
            if( rModel.maCellAddr.Row != nLastRow )
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
        if( maXfIdRanges.empty() || !maXfIdRanges.rbegin()->second.tryExpand( rModel, nNumFmtId ) )
            maXfIdRanges[ BinAddress( rModel.maCellAddr ) ].set( rModel, nNumFmtId );

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

void SheetDataBuffer::finalizeImport()
{
    // create all array formulas
    for( ArrayFormulaList::iterator aIt = maArrayFormulas.begin(), aEnd = maArrayFormulas.end(); aIt != aEnd; ++aIt )
        finalizeArrayFormula( aIt->first, aIt->second );

    // create all table operations
    for( TableOperationList::iterator aIt = maTableOperations.begin(), aEnd = maTableOperations.end(); aIt != aEnd; ++aIt )
        finalizeTableOperation( aIt->first, aIt->second );

    // write default formatting of remaining row range
    writeXfIdRowRangeProperties( maXfIdRowRange );

    // try to merge remaining inserted ranges
    mergeXfIdRanges();
    // write all formatting
    for( XfIdRangeMap::const_iterator aIt = maXfIdRanges.begin(), aEnd = maXfIdRanges.end(); aIt != aEnd; ++aIt )
        writeXfIdRangeProperties( aIt->second );

    // merge all cached merged ranges and update right/bottom cell borders
    for( MergedRangeList::iterator aIt = maMergedRanges.begin(), aEnd = maMergedRanges.end(); aIt != aEnd; ++aIt )
        finalizeMergedRange( aIt->maRange );
    for( MergedRangeList::iterator aIt = maCenterFillRanges.begin(), aEnd = maCenterFillRanges.end(); aIt != aEnd; ++aIt )
        finalizeMergedRange( aIt->maRange );
}

// private --------------------------------------------------------------------

SheetDataBuffer::XfIdRowRange::XfIdRowRange() :
    mnFirstRow( -1 ),
    mnLastRow( -1 ),
    mnXfId( -1 )
{
}

bool SheetDataBuffer::XfIdRowRange::intersects( const CellRangeAddress& rRange ) const
{
    return (rRange.StartRow <= mnLastRow) && (mnFirstRow <= rRange.EndRow);
}

void SheetDataBuffer::XfIdRowRange::set( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId )
{
    mnFirstRow = nFirstRow;
    mnLastRow = nLastRow;
    mnXfId = nXfId;
}

bool SheetDataBuffer::XfIdRowRange::tryExpand( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId )
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

void SheetDataBuffer::XfIdRange::set( const CellModel& rModel, sal_Int32 nNumFmtId )
{
    maRange.Sheet = rModel.maCellAddr.Sheet;
    maRange.StartColumn = maRange.EndColumn = rModel.maCellAddr.Column;
    maRange.StartRow = maRange.EndRow = rModel.maCellAddr.Row;
    mnXfId = rModel.mnXfId;
    mnNumFmtId = nNumFmtId;
}

bool SheetDataBuffer::XfIdRange::tryExpand( const CellModel& rModel, sal_Int32 nNumFmtId )
{
    if( (mnXfId == rModel.mnXfId) && (mnNumFmtId == nNumFmtId) &&
        (maRange.StartRow == rModel.maCellAddr.Row) &&
        (maRange.EndRow == rModel.maCellAddr.Row) &&
        (maRange.EndColumn + 1 == rModel.maCellAddr.Column) )
    {
        ++maRange.EndColumn;
        return true;
    }
    return false;
}

bool SheetDataBuffer::XfIdRange::tryMerge( const XfIdRange& rXfIdRange )
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

void SheetDataBuffer::finalizeArrayFormula( const CellRangeAddress& rRange, const ApiTokenSequence& rTokens )
{
    Reference< XArrayFormulaTokens > xTokens( getCellRange( rRange ), UNO_QUERY );
    OSL_ENSURE( xTokens.is(), "SheetDataBuffer::finalizeArrayFormula - missing formula token interface" );
    if( xTokens.is() )
        xTokens->setArrayTokens( rTokens );
}

void SheetDataBuffer::finalizeTableOperation( const CellRangeAddress& rRange, const DataTableModel& rModel )
{
    sal_Int16 nSheet = getSheetIndex();
    bool bOk = false;
    if( !rModel.mbRef1Deleted && (rModel.maRef1.getLength() > 0) && (rRange.StartColumn > 0) && (rRange.StartRow > 0) )
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

void SheetDataBuffer::createSharedFormula( const BinAddress& rMapKey, const ApiTokenSequence& rTokens )
{
    // create the defined name that will represent the shared formula
    OUString aName = OUStringBuffer().appendAscii( RTL_CONSTASCII_STRINGPARAM( "__shared_" ) ).
        append( static_cast< sal_Int32 >( getSheetIndex() + 1 ) ).
        append( sal_Unicode( '_' ) ).append( rMapKey.mnRow ).
        append( sal_Unicode( '_' ) ).append( rMapKey.mnCol ).makeStringAndClear();
    Reference< XNamedRange > xNamedRange = createNamedRangeObject( aName );
    OSL_ENSURE( xNamedRange.is(), "SheetDataBuffer::createSharedFormula - cannot create shared formula" );
    PropertySet aNameProps( xNamedRange );
    aNameProps.setProperty( PROP_IsSharedFormula, true );

    // get and store the token index of the defined name
    OSL_ENSURE( maSharedFormulas.count( rMapKey ) == 0, "SheetDataBuffer::createSharedFormula - shared formula exists already" );
    sal_Int32 nTokenIndex = 0;
    if( aNameProps.getProperty( nTokenIndex, PROP_TokenIndex ) && (nTokenIndex >= 0) ) try
    {
        // store the token index in the map
        maSharedFormulas[ rMapKey ] = nTokenIndex;
        // set the formula definition
        Reference< XFormulaTokens > xTokens( xNamedRange, UNO_QUERY_THROW );
        xTokens->setTokens( rTokens );
        // retry to insert a pending shared formula cell
        if( mbPendingSharedFmla )
        {
            ApiTokenSequence aTokens = resolveSharedFormula( maSharedBaseAddr );
            setFormulaCell( maSharedFmlaAddr, aTokens );
        }
    }
    catch( Exception& )
    {
    }
    mbPendingSharedFmla = false;
}

ApiTokenSequence SheetDataBuffer::resolveSharedFormula( const BinAddress& rMapKey ) const
{
    sal_Int32 nTokenIndex = ContainerHelper::getMapElement( maSharedFormulas, rMapKey, -1 );
    return (nTokenIndex >= 0) ? getFormulaParser().convertNameToFormula( nTokenIndex ) : ApiTokenSequence();
}

void SheetDataBuffer::writeXfIdRowRangeProperties( const XfIdRowRange& rXfIdRowRange ) const
{
    if( (rXfIdRowRange.mnLastRow >= 0) && (rXfIdRowRange.mnXfId >= 0) )
    {
        AddressConverter& rAddrConv = getAddressConverter();
        CellRangeAddress aRange( getSheetIndex(), 0, rXfIdRowRange.mnFirstRow, rAddrConv.getMaxApiAddress().Column, rXfIdRowRange.mnLastRow );
        if( rAddrConv.validateCellRange( aRange, true, false ) )
        {
            PropertySet aPropSet( getCellRange( aRange ) );
            getStyles().writeCellXfToPropertySet( aPropSet, rXfIdRowRange.mnXfId );
        }
    }
}

void SheetDataBuffer::writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const
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

void SheetDataBuffer::mergeXfIdRanges()
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

void SheetDataBuffer::finalizeMergedRange( const CellRangeAddress& rRange )
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
            if( !bTextWrap && (xTopLeft->getType() == CellContentType_TEXT) )
            {
                Reference< XText > xText( xTopLeft, UNO_QUERY );
                bTextWrap = xText.is() && (xText->getString().indexOf( '\x0A' ) >= 0);
            }
            if( bTextWrap )
                setManualRowHeight( rRange.StartRow );
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace xls
} // namespace oox
