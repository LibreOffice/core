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

#include <sheetdatabuffer.hxx>

#include <algorithm>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <editeng/boxitem.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <addressconverter.hxx>
#include <formulaparser.hxx>
#include <sharedstringsbuffer.hxx>
#include <unitconverter.hxx>
#include <rangelst.hxx>
#include <document.hxx>
#include <scitems.hxx>
#include <docpool.hxx>
#include <paramisc.hxx>
#include <patattr.hxx>
#include <documentimport.hxx>
#include <formulabuffer.hxx>
#include <numformat.hxx>
#include <sax/tools/converter.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

CellModel::CellModel() :
    mnCellType( XML_TOKEN_INVALID ),
    mnXfId( -1 ),
    mbShowPhonetic( false )
{
}

CellFormulaModel::CellFormulaModel() :
    mnFormulaType( XML_TOKEN_INVALID ),
    mnSharedId( -1 )
{
}

bool CellFormulaModel::isValidArrayRef( const ScAddress& rCellAddr )
{
    return (maFormulaRef.aStart == rCellAddr );
}

bool CellFormulaModel::isValidSharedRef( const ScAddress& rCellAddr )
{
    return
        (maFormulaRef.aStart.Tab() == rCellAddr.Tab() ) &&
        (maFormulaRef.aStart.Col() <= rCellAddr.Col() ) && (rCellAddr.Col() <= maFormulaRef.aEnd.Col()) &&
        (maFormulaRef.aStart.Row() <= rCellAddr.Row() ) && (rCellAddr.Row() <= maFormulaRef.aEnd.Row());
}

DataTableModel::DataTableModel() :
    mb2dTable( false ),
    mbRowTable( false ),
    mbRef1Deleted( false ),
    mbRef2Deleted( false )
{
}

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
    {
        maColSpans[ nRow ] = rColSpans.getRanges();
        mnCurrRow = nRow;
    }
}

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
    getDocImport().setNumericCell(rModel.maCellAddr, fValue);
    setCellFormat( rModel );
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, const OUString& rText )
{
    if (!rText.isEmpty())
        getDocImport().setStringCell(rModel.maCellAddr, rText);

    setCellFormat( rModel );
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, const RichStringRef& rxString )
{
    OSL_ENSURE( rxString.get(), "SheetDataBuffer::setStringCell - missing rich string object" );
    const oox::xls::Font* pFirstPortionFont = getStyles().getFontFromCellXf( rModel.mnXfId ).get();
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

void SheetDataBuffer::setDateTimeCell( const CellModel& rModel, const css::util::DateTime& rDateTime )
{
    // write serial date/time value into the cell
    double fSerial = getUnitConverter().calcSerialFromDateTime( rDateTime );
    setValueCell( rModel, fSerial );
    // set appropriate number format
    using namespace ::com::sun::star::util::NumberFormat;
    sal_Int16 nStdFmt = (fSerial < 1.0) ? TIME : (((rDateTime.Hours > 0) || (rDateTime.Minutes > 0) || (rDateTime.Seconds > 0)) ? DATETIME : DATE);
    // set number format
    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( getDocument(), UNO_QUERY_THROW );
        Reference< XNumberFormatTypes > xNumFmtTypes( xNumFmtsSupp->getNumberFormats(), UNO_QUERY_THROW );
        sal_Int32 nIndex = xNumFmtTypes->getStandardFormat( nStdFmt, Locale() );
        PropertySet aPropSet( getCell( rModel.maCellAddr ) );
        aPropSet.setProperty( PROP_NumberFormat, nIndex );
    }
    catch( Exception& )
    {
    }
}

void SheetDataBuffer::setBooleanCell( const CellModel& rModel, bool bValue )
{
    getFormulaBuffer().setCellFormula(
        rModel.maCellAddr, bValue ? OUString("TRUE()") : OUString("FALSE()"));

    // #108770# set 'Standard' number format for all Boolean cells
    setCellFormat( rModel );
}

void SheetDataBuffer::setErrorCell( const CellModel& rModel, const OUString& rErrorCode )
{
    // Using the formula compiler now we can simply pass on the error string.
    getFormulaBuffer().setCellFormula( rModel.maCellAddr, rErrorCode);
    setCellFormat( rModel );
}

void SheetDataBuffer::setErrorCell( const CellModel& rModel, sal_uInt8 nErrorCode )
{
    setErrorCell( rModel, getUnitConverter().calcErrorString( nErrorCode));
}

void SheetDataBuffer::setDateCell( const CellModel& rModel, const OUString& rDateString )
{
    css::util::DateTime aDateTime;
    if (!sax::Converter::parseDateTime( aDateTime, rDateString))
    {
        SAL_WARN("sc.filter", "SheetDataBuffer::setDateCell - could not parse: " << rDateString);
        // At least don't lose data.
        setStringCell( rModel, rDateString);
        return;
    }

    double fSerial = getUnitConverter().calcSerialFromDateTime( aDateTime);
    setValueCell( rModel, fSerial);
}

void SheetDataBuffer::createSharedFormula(const ScAddress& rAddr, const ApiTokenSequence& rTokens)
{
    BinAddress aAddr(rAddr);
    maSharedFormulas[aAddr] = rTokens;
    if( mbPendingSharedFmla )
        setCellFormula( maSharedFmlaAddr, resolveSharedFormula( maSharedBaseAddr ) );
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
            ScAddress aTokenAddr = ScAddress ( aTokenInfo.First.Column, aTokenInfo.First.Row, aTokenInfo.First.Sheet );
            BinAddress aBaseAddr( aTokenAddr );
            aTokens = resolveSharedFormula( aTokenAddr );
            if( !aTokens.hasElements() )
            {
                maSharedFmlaAddr = rModel.maCellAddr;
                maSharedBaseAddr = aTokenAddr;
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

void SheetDataBuffer::createArrayFormula( const ScRange& rRange, const ApiTokenSequence& rTokens )
{
    /*  Array formulas will be inserted later in finalizeImport(). This is
        needed to not disturb collecting all the cells, which will be put into
        the sheet in large blocks to increase performance. */
    maArrayFormulas.emplace_back( rRange, rTokens );
}

void SheetDataBuffer::createTableOperation( const ScRange& rRange, const DataTableModel& rModel )
{
    /*  Table operations will be inserted later in finalizeImport(). This is
        needed to not disturb collecting all the cells, which will be put into
        the sheet in large blocks to increase performance. */
    maTableOperations.emplace_back( rRange, rModel );
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

void SheetDataBuffer::setMergedRange( const ScRange& rRange )
{
    maMergedRanges.emplace_back( rRange );
}

typedef std::pair<sal_Int32, sal_Int32> FormatKeyPair;

static void addIfNotInMyMap( const StylesBuffer& rStyles, std::map< FormatKeyPair, ScRangeList >& rMap, sal_Int32 nXfId, sal_Int32 nFormatId, const ScRangeList& rRangeList )
{
    Xf* pXf1 = rStyles.getCellXf( nXfId ).get();
    if ( pXf1 )
    {
        auto it = std::find_if(rMap.begin(), rMap.end(),
            [&nFormatId, &rStyles, &pXf1](const std::pair<FormatKeyPair, ScRangeList>& rEntry) {
                if (rEntry.first.second != nFormatId)
                    return false;
                Xf* pXf2 = rStyles.getCellXf( rEntry.first.first ).get();
                return *pXf1 == *pXf2;
            });
        if (it != rMap.end()) // already exists
        {
            // add ranges from the rangelist to the existing rangelist for the
            // matching style ( should we check if they overlap ? )
            for (size_t i = 0, nSize = rRangeList.size(); i < nSize; ++i)
                it->second.push_back(rRangeList[i]);
            return;
        }
        rMap[ FormatKeyPair( nXfId, nFormatId ) ] = rRangeList;
    }
}

void SheetDataBuffer::addColXfStyle( sal_Int32 nXfId, sal_Int32 nFormatId, const ScRange& rAddress, bool bProcessRowRange )
{
    RowRangeStyle aStyleRows;
    aStyleRows.mnNumFmt.first = nXfId;
    aStyleRows.mnNumFmt.second = nFormatId;
    aStyleRows.mnStartRow = rAddress.aStart.Row();
    aStyleRows.mnEndRow = rAddress.aEnd.Row();
    for ( sal_Int32 nCol = rAddress.aStart.Col(); nCol <= rAddress.aEnd.Col(); ++nCol )
    {
        if ( !bProcessRowRange )
            maStylesPerColumn[ nCol ].insert( aStyleRows );
        else
        {
            RowStyles& rRowStyles = maStylesPerColumn[ nCol ];
            // Reset row range for each column
            aStyleRows.mnStartRow = rAddress.aStart.Row();
            aStyleRows.mnEndRow = rAddress.aEnd.Row();

            // If aStyleRows includes rows already allocated to a style
            // in rRowStyles, then we need to split it into parts.
            // ( to occupy only rows that have no style definition)

            // Start iterating at the first element that is not completely before aStyleRows
            RowStyles::iterator rows_it = rRowStyles.lower_bound(aStyleRows);
            RowStyles::iterator rows_end = rRowStyles.end();
            bool bAddRange = true;
            for ( ; rows_it != rows_end; ++rows_it )
            {
                const RowRangeStyle& r = *rows_it;

                // Add the part of aStyleRows that does not overlap with r
                if ( aStyleRows.mnStartRow < r.mnStartRow )
                {
                    RowRangeStyle aSplit = aStyleRows;
                    aSplit.mnEndRow = std::min(aStyleRows.mnEndRow, r.mnStartRow - 1);
                    // Insert with hint that aSplit comes directly before the current position
                    rRowStyles.insert( rows_it, aSplit );
                }

                // Done if no part of aStyleRows extends beyond r
                if ( aStyleRows.mnEndRow <= r.mnEndRow )
                {
                    bAddRange = false;
                    break;
                }

                // Cut off the part aStyleRows that was handled above
                aStyleRows.mnStartRow = r.mnEndRow + 1;
            }
            if ( bAddRange )
                rRowStyles.insert( aStyleRows );
        }
    }
}

void SheetDataBuffer::finalizeImport()
{
    // create all array formulas
    for( const auto& [rRange, rTokens] : maArrayFormulas )
        finalizeArrayFormula( rRange, rTokens );

    // create all table operations
    for( const auto& [rRange, rModel] : maTableOperations )
        finalizeTableOperation( rRange, rModel );

    // write default formatting of remaining row range
    maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );

    std::map< FormatKeyPair, ScRangeList > rangeStyleListMap;
    for( const auto& [rFormatKeyPair, rRangeList] : maXfIdRangeLists )
    {
        addIfNotInMyMap( getStyles(), rangeStyleListMap, rFormatKeyPair.first, rFormatKeyPair.second, rRangeList );
    }
    // gather all ranges that have the same style and apply them in bulk
    for ( const auto& [rFormatKeyPair, rRanges] : rangeStyleListMap )
    {
        for (size_t i = 0, nSize = rRanges.size(); i < nSize; ++i)
            addColXfStyle( rFormatKeyPair.first, rFormatKeyPair.second, rRanges[i]);
    }

    for ( const auto& [rXfId, rRowRangeList] : maXfIdRowRangeList )
    {
        if ( rXfId == -1 ) // it's a dud skip it
            continue;
        AddressConverter& rAddrConv = getAddressConverter();
        // get all row ranges for id
        for ( const auto& rRange : rRowRangeList )
        {
            ScRange aRange( 0, rRange.mnFirst, getSheetIndex(),
                            rAddrConv.getMaxApiAddress().Col(), rRange.mnLast, getSheetIndex() );

            addColXfStyle( rXfId, -1, aRange, true );
        }
    }

    ScDocumentImport& rDoc = getDocImport();
    StylesBuffer& rStyles = getStyles();
    for ( const auto& [rCol, rRowStyles] : maStylesPerColumn )
    {
        SCCOL nScCol = static_cast< SCCOL >( rCol );

        // tdf#91567 Get pattern from the first row without AutoFilter
        const ScPatternAttr* pDefPattern = nullptr;
        bool bAutoFilter = true;
        SCROW nScRow = 0;
        while ( bAutoFilter && nScRow < MAXROW )
        {
            pDefPattern = rDoc.getDoc().GetPattern( nScCol, nScRow, getSheetIndex() );
            if ( pDefPattern )
            {
                const ScMergeFlagAttr* pAttr = pDefPattern->GetItemSet().GetItem( ATTR_MERGE_FLAG );
                bAutoFilter = pAttr->HasAutoFilter();
            }
            else
                break;
            nScRow++;
        }
        if ( !pDefPattern || nScRow == MAXROW )
            pDefPattern = rDoc.getDoc().GetDefPattern();

        Xf::AttrList aAttrs(pDefPattern);
        for ( const auto& rRowStyle : rRowStyles )
        {
             Xf* pXf = rStyles.getCellXf( rRowStyle.mnNumFmt.first ).get();

             if ( pXf )
                 pXf->applyPatternToAttrList( aAttrs,  rRowStyle.mnStartRow,  rRowStyle.mnEndRow,  rRowStyle.mnNumFmt.second );
        }
        if (aAttrs.maAttrs.empty() || aAttrs.maAttrs.back().nEndRow != MAXROW)
        {
            ScAttrEntry aEntry;
            aEntry.nEndRow = MAXROW;
            aEntry.pPattern = pDefPattern;
            rDoc.getDoc().GetPool()->Put(*aEntry.pPattern);
            aAttrs.maAttrs.push_back(aEntry);

            if (!sc::NumFmtUtil::isLatinScript(*aEntry.pPattern, rDoc.getDoc()))
                aAttrs.mbLatinNumFmtOnly = false;
        }

        ScDocumentImport::Attrs aAttrParam;
        aAttrParam.mvData.swap(aAttrs.maAttrs);
        aAttrParam.mbLatinNumFmtOnly = aAttrs.mbLatinNumFmtOnly;

        rDoc.setAttrEntries(getSheetIndex(), nScCol, std::move(aAttrParam));
    }

    // merge all cached merged ranges and update right/bottom cell borders
    for( const auto& rMergedRange : maMergedRanges )
        applyCellMerging( rMergedRange.maRange );
    for( const auto& rCenterFillRange : maCenterFillRanges )
        applyCellMerging( rCenterFillRange.maRange );
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

SheetDataBuffer::MergedRange::MergedRange( const ScRange& rRange ) :
    maRange( rRange ),
    mnHorAlign( XML_TOKEN_INVALID )
{
}

SheetDataBuffer::MergedRange::MergedRange( const ScAddress& rAddress, sal_Int32 nHorAlign ) :
    maRange( rAddress, rAddress ),
    mnHorAlign( nHorAlign )
{
}

bool SheetDataBuffer::MergedRange::tryExpand( const ScAddress& rAddress, sal_Int32 nHorAlign )
{
    if( (mnHorAlign == nHorAlign) && (maRange.aStart.Row() == rAddress.Row() ) &&
        (maRange.aEnd.Row() == rAddress.Row() ) && (maRange.aEnd.Col() + 1 == rAddress.Col() ) )
    {
        maRange.aEnd.IncCol();
        return true;
    }
    return false;
}

void SheetDataBuffer::setCellFormula( const ScAddress& rCellAddr, const ApiTokenSequence& rTokens )
{
    if( rTokens.hasElements() )
    {
        putFormulaTokens( rCellAddr, rTokens );
    }
}


ApiTokenSequence SheetDataBuffer::resolveSharedFormula( const ScAddress& rAddr ) const
{
    BinAddress aAddr(rAddr);
    ApiTokenSequence aTokens = ContainerHelper::getMapElement( maSharedFormulas, aAddr, ApiTokenSequence() );
    return aTokens;
}

void SheetDataBuffer::finalizeArrayFormula( const ScRange& rRange, const ApiTokenSequence& rTokens ) const
{
    Reference< XArrayFormulaTokens > xTokens( getCellRange( rRange ), UNO_QUERY );
    OSL_ENSURE( xTokens.is(), "SheetDataBuffer::finalizeArrayFormula - missing formula token interface" );
    if( xTokens.is() )
        xTokens->setArrayTokens( rTokens );
}

void SheetDataBuffer::finalizeTableOperation( const ScRange& rRange, const DataTableModel& rModel )
{
    if (rModel.mbRef1Deleted)
        return;

    if (rModel.maRef1.isEmpty())
        return;

    if (rRange.aStart.Col() <= 0 || rRange.aStart.Row() <= 0)
        return;

    sal_Int16 nSheet = getSheetIndex();

    ScAddress aRef1( 0, 0, 0 );
    if (!getAddressConverter().convertToCellAddress(aRef1, rModel.maRef1, nSheet, true))
        return;

    ScDocumentImport& rDoc = getDocImport();
    ScTabOpParam aParam;

    ScRange aScRange(rRange);

    if (rModel.mb2dTable)
    {
        // Two-variable data table.
        if (rModel.mbRef2Deleted)
            return;

        if (rModel.maRef2.isEmpty())
            return;

        ScAddress aRef2( 0, 0, 0 );
        if (!getAddressConverter().convertToCellAddress(aRef2, rModel.maRef2, nSheet, true))
            return;

        aParam.meMode = ScTabOpParam::Both;

        aScRange.aStart.IncCol(-1);
        aScRange.aStart.IncRow(-1);

        aParam.aRefFormulaCell.Set(aScRange.aStart.Col(), aScRange.aStart.Row(), nSheet, false, false, false);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;

        // Ref1 is row input cell and Ref2 is column input cell.
        aParam.aRefRowCell.Set(aRef1.Col(), aRef1.Row(), aRef1.Tab(), false, false, false);
        aParam.aRefColCell.Set(aRef2.Col(), aRef2.Row(), aRef2.Tab(), false, false, false);
        rDoc.setTableOpCells(aScRange, aParam);

        return;
    }

    // One-variable data table.

    if (rModel.mbRowTable)
    {
        // One-variable row input cell (horizontal).
        aParam.meMode = ScTabOpParam::Row;
        aParam.aRefRowCell.Set(aRef1.Col(), aRef1.Row(), aRef1.Tab(), false, false, false);
        aParam.aRefFormulaCell.Set(rRange.aStart.Col()-1, rRange.aStart.Row(), nSheet, false, true, false);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
        aScRange.aStart.IncRow(-1);
        rDoc.setTableOpCells(aScRange, aParam);
    }
    else
    {
        // One-variable column input cell (vertical).
        aParam.meMode = ScTabOpParam::Column;
        aParam.aRefColCell.Set(aRef1.Col(), aRef1.Row(), aRef1.Tab(), false, false, false);
        aParam.aRefFormulaCell.Set(rRange.aStart.Col(), rRange.aStart.Row()-1, nSheet, true, false, false);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
        aScRange.aStart.IncCol(-1);
        rDoc.setTableOpCells(aScRange, aParam);
    }
}

void SheetDataBuffer::setCellFormat( const CellModel& rModel )
{
    if( rModel.mnXfId >= 0 )
    {
        ScRangeList& rRangeList = maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, -1 ) ];
        ScRange* pLastRange = rRangeList.empty() ? nullptr : &rRangeList.back();
        /* The xlsx sheet data contains row wise information.
         * It is sufficient to check if the row range size is one
         */
        if (!rRangeList.empty() &&
            pLastRange->aStart.Tab() == rModel.maCellAddr.Tab() &&
            pLastRange->aStart.Row() == pLastRange->aEnd.Row() &&
            pLastRange->aStart.Row() == rModel.maCellAddr.Row() &&
            pLastRange->aEnd.Col() + 1 == rModel.maCellAddr.Col())
        {
            pLastRange->aEnd.IncCol();       // Expand Column
        }
        else
        {
            rRangeList.push_back(ScRange(rModel.maCellAddr));
            pLastRange = &rRangeList.back();
        }

        if (rRangeList.size() > 1)
        {
            for (size_t i = rRangeList.size() - 1; i != 0; --i)
            {
                ScRange& rMergeRange = rRangeList[i - 1];
                if (pLastRange->aStart.Tab() != rMergeRange.aStart.Tab())
                    break;

                /* Try to merge this with the previous range */
                if (pLastRange->aStart.Row() == (rMergeRange.aEnd.Row() + 1) &&
                    pLastRange->aStart.Col() == rMergeRange.aStart.Col() &&
                    pLastRange->aEnd.Col() == rMergeRange.aEnd.Col())
                {
                    rMergeRange.aEnd.SetRow(pLastRange->aEnd.Row());
                    rRangeList.Remove(rRangeList.size() - 1);
                    break;
                }
                else if (pLastRange->aStart.Row() > (rMergeRange.aEnd.Row() + 1))
                    break; // Un-necessary to check with any other rows
            }
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
                    maCenterFillRanges.emplace_back( rModel.maCellAddr, nHorAlign );
                else if( !maCenterFillRanges.empty() )
                    maCenterFillRanges.rbegin()->tryExpand( rModel.maCellAddr, nHorAlign );
            }
        }
    }
}

static void lcl_SetBorderLine( ScDocument& rDoc, const ScRange& rRange, SCTAB nScTab, SvxBoxItemLine nLine )
{
    SCCOL nFromScCol = (nLine == SvxBoxItemLine::RIGHT) ? rRange.aEnd.Col() : rRange.aStart.Col();
    SCROW nFromScRow = (nLine == SvxBoxItemLine::BOTTOM) ? rRange.aEnd.Row() : rRange.aStart.Row();

    const SvxBoxItem* pFromItem =
        rDoc.GetAttr( nFromScCol, nFromScRow, nScTab, ATTR_BORDER );
    const SvxBoxItem* pToItem =
        rDoc.GetAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, ATTR_BORDER );

    SvxBoxItem aNewItem( *pToItem );
    aNewItem.SetLine( pFromItem->GetLine( nLine ), nLine );
    rDoc.ApplyAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, aNewItem );
}

void SheetDataBuffer::applyCellMerging( const ScRange& rRange )
{
    bool bMultiCol = rRange.aStart.Col() < rRange.aEnd.Col();
    bool bMultiRow = rRange.aStart.Row() < rRange.aEnd.Row();

    const ScAddress& rStart = rRange.aStart;
    const ScAddress& rEnd = rRange.aEnd;
    ScDocument& rDoc = getScDocument();
    // set correct right border
    if( bMultiCol )
        lcl_SetBorderLine( rDoc, rRange, getSheetIndex(), SvxBoxItemLine::RIGHT );
        // set correct lower border
    if( bMultiRow )
        lcl_SetBorderLine( rDoc, rRange, getSheetIndex(), SvxBoxItemLine::BOTTOM );
    // do merge
    if( bMultiCol || bMultiRow )
        rDoc.DoMerge( getSheetIndex(), rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row() );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
