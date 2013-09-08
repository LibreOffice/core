/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulabuffer.hxx"
#include "formulaparser.hxx"
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCell2.hpp>
#include "formulacell.hxx"
#include "document.hxx"
#include "convuno.hxx"

#include "rangelst.hxx"
#include "autonamecache.hxx"
#include "tokenuno.hxx"
#include "tokenarray.hxx"
#include "sharedformulagroups.hxx"
#include "oox/token/tokens.hxx"

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::container;

namespace oox { namespace xls {

FormulaBuffer::SharedFormulaEntry::SharedFormulaEntry(
    const table::CellAddress& rAddr, const table::CellRangeAddress& rRange,
    const OUString& rTokenStr, sal_Int32 nSharedId ) :
    maAddress(rAddr), maRange(rRange), maTokenStr(rTokenStr), mnSharedId(nSharedId) {}

FormulaBuffer::SharedFormulaDesc::SharedFormulaDesc(
    const com::sun::star::table::CellAddress& rAddr, sal_Int32 nSharedId,
    const OUString& rCellValue, sal_Int32 nValueType ) :
    maAddress(rAddr), mnSharedId(nSharedId), maCellValue(rCellValue), mnValueType(nValueType) {}

FormulaBuffer::FormulaBuffer( const WorkbookHelper& rHelper ) : WorkbookHelper( rHelper )
{
}

Reference<XCellRange> FormulaBuffer::getRange( const CellRangeAddress& rRange )
{
    Reference<XCellRange> xRange;
    try
    {
        xRange = mxCurrSheet->getCellRangeByPosition(
            rRange.StartColumn, rRange.StartRow, rRange.EndColumn, rRange.EndRow);
    }
    catch( Exception& )
    {
    }
    return xRange;
}

void FormulaBuffer::finalizeImport()
{
    ISegmentProgressBarRef xFormulaBar = getProgressBar().createSegment( getProgressBar().getFreeLength() );

    ScDocument& rDoc = getScDocument();
    Reference< XIndexAccess > xSheets( getDocument()->getSheets(), UNO_QUERY_THROW );
    rDoc.SetAutoNameCache( new ScAutoNameCache( &rDoc ) );
    for ( sal_Int32 nTab = 0, nElem = xSheets->getCount(); nTab < nElem; ++nTab )
    {
        double fPosition = static_cast< double> (nTab + 1) /static_cast<double>(nElem);
        xFormulaBar->setPosition( fPosition );
        mxCurrSheet = getSheetFromDoc( nTab );

        applySharedFormulas(nTab);

        FormulaDataMap::iterator cellIt = maCellFormulas.find( nTab );
        if ( cellIt != maCellFormulas.end() )
        {
            applyCellFormulas( cellIt->second );
        }

        ArrayFormulaDataMap::iterator itArray = maCellArrayFormulas.find( nTab );
        if ( itArray != maCellArrayFormulas.end() )
        {
            applyArrayFormulas( itArray->second );
        }

        FormulaValueMap::iterator itValues = maCellFormulaValues.find( nTab );
        if ( itValues != maCellFormulaValues.end() )
        {
            std::vector< ValueAddressPair > & rVector = itValues->second;
            applyCellFormulaValues( rVector );
        }
    }
    rDoc.SetAutoNameCache( NULL );
    xFormulaBar->setPosition( 1.0 );
}

void FormulaBuffer::applyCellFormula( ScDocument& rDoc, const ApiTokenSequence& rTokens, const ::com::sun::star::table::CellAddress& rAddress )
{
    ScTokenArray aTokenArray;
    ScAddress aCellPos;
    ScUnoConversion::FillScAddress( aCellPos, rAddress );
    ScTokenConversion::ConvertToTokenArray( rDoc, aTokenArray, rTokens );
    ScFormulaCell* pNewCell = new ScFormulaCell( &rDoc, aCellPos, &aTokenArray );
    pNewCell->StartListeningTo( &rDoc );
    rDoc.EnsureTable(aCellPos.Tab());
    rDoc.SetFormulaCell(aCellPos, pNewCell);
}

void FormulaBuffer::applyCellFormulas( const std::vector< TokenAddressItem >& rVector )
{
    ScDocument& rDoc = getScDocument();
    for ( std::vector< TokenAddressItem >::const_iterator it = rVector.begin(), it_end = rVector.end(); it != it_end; ++it )
    {
        const ::com::sun::star::table::CellAddress& rAddress = it->maCellAddress;
        ApiTokenSequence aTokens = getFormulaParser().importFormula( rAddress, it->maTokenStr );
        applyCellFormula( rDoc, aTokens, rAddress );
    }
}

void FormulaBuffer::applyCellFormulaValues( const std::vector< ValueAddressPair >& rVector )
{
    ScDocument& rDoc = getScDocument();
    for ( std::vector< ValueAddressPair >::const_iterator it = rVector.begin(), it_end = rVector.end(); it != it_end; ++it )
    {
        ScAddress aCellPos;
        ScUnoConversion::FillScAddress( aCellPos, it->first );
        ScFormulaCell* pCell = rDoc.GetFormulaCell(aCellPos);
        if (pCell)
        {
            pCell->SetHybridDouble( it->second );
            pCell->ResetDirty();
            pCell->SetChanged(false);
        }
    }
}

void FormulaBuffer::applySharedFormulas( sal_Int32 nTab )
{
    SheetToFormulaEntryMap::const_iterator itShared = maSharedFormulas.find(nTab);
    if (itShared == maSharedFormulas.end())
        // There is no shared formulas for this sheet.
        return;

    SheetToSharedFormulaid::const_iterator itCells = maSharedFormulaIds.find(nTab);
    if (itCells == maSharedFormulaIds.end())
        // There is no formula cells that use shared formulas for this sheet.
        return;

    const std::vector<SharedFormulaEntry>& rSharedFormulas = itShared->second;
    const std::vector<SharedFormulaDesc>& rCells = itCells->second;

    ScDocument& rDoc = getScDocument();

    sc::SharedFormulaGroups aGroups;
    {
        // Process shared formulas first.
        std::vector<SharedFormulaEntry>::const_iterator it = rSharedFormulas.begin(), itEnd = rSharedFormulas.end();
        for (; it != itEnd; ++it)
        {
            const table::CellAddress& rAddr = it->maAddress;
            const table::CellRangeAddress& rRange = it->maRange;
            sal_Int32 nId = it->mnSharedId;
            const OUString& rTokenStr = it->maTokenStr;

            ScAddress aPos;
            ScUnoConversion::FillScAddress(aPos, rAddr);
            ScCompiler aComp(&rDoc, aPos);
            aComp.SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);
            ScTokenArray* pArray = aComp.CompileString(rTokenStr);
            if (pArray)
            {
                for (sal_Int32 nCol = rRange.StartColumn; nCol <= rRange.EndColumn; ++nCol)
                {
                    // Create one group per column, since Calc doesn't support
                    // shared formulas across multiple columns.
                    ScFormulaCellGroupRef xNewGroup(new ScFormulaCellGroup);
                    xNewGroup->mnStart = rRange.StartRow;
                    xNewGroup->mnLength = rRange.EndRow - rRange.StartRow + 1;
                    xNewGroup->setCode(*pArray);
                    aGroups.set(nId, nCol, xNewGroup);
                }
            }
        }
    }

    {
        // Process formulas that use shared formulas.
        std::vector<SharedFormulaDesc>::const_iterator it = rCells.begin(), itEnd = rCells.end();
        for (; it != itEnd; ++it)
        {
            const table::CellAddress& rAddr = it->maAddress;

            ScFormulaCellGroupRef xGroup = aGroups.get(it->mnSharedId, rAddr.Column);
            if (!xGroup)
                continue;

            ScAddress aPos;
            ScUnoConversion::FillScAddress(aPos, rAddr);
            if (xGroup->mnStart == aPos.Row())
                // Generate code for the top cell only.
                xGroup->compileCode(rDoc, aPos, formula::FormulaGrammar::GRAM_DEFAULT);
            ScFormulaCell* pCell = new ScFormulaCell(&rDoc, aPos, xGroup);

            bool bInserted = rDoc.SetGroupFormulaCell(aPos, pCell);
            if (!bInserted)
            {
                // Insertion failed.
                delete pCell;
                continue;
            }

            pCell->StartListeningTo(&rDoc);

            if (it->maCellValue.isEmpty())
            {
                // No cached cell value. Mark it for re-calculation.
                pCell->SetDirty(true);
                continue;
            }

            // Set cached formula results. For now, we only use numeric
            // results. Find out how to utilize cached results of other types.
            switch (it->mnValueType)
            {
                case XML_n:
                    // numeric value.
                    pCell->SetResultDouble(it->maCellValue.toDouble());
                break;
                default:
                    // Mark it for re-calculation.
                    pCell->SetDirty(true);
            }
        }
    }
}

// bound to need this somewhere else, if so probably need to move it to
// worksheethelper or somewhere else more suitable
void StartCellListening( sal_Int16 nSheet, sal_Int32 nRow, sal_Int32 nCol, ScDocument& rDoc )
{
    ScAddress aCellPos;
    CellAddress aAddress;
    aAddress.Sheet = nSheet;
    aAddress.Row = nRow;
    aAddress.Column = nCol;
    ScUnoConversion::FillScAddress( aCellPos, aAddress );
    ScFormulaCell* pFCell = rDoc.GetFormulaCell( aCellPos );
    if ( pFCell )
        pFCell->StartListeningTo( &rDoc );
}

void FormulaBuffer::applyArrayFormulas( const std::vector< TokenRangeAddressItem >& rVector )
{
    ScDocument& rDoc = getScDocument();
    for ( std::vector< TokenRangeAddressItem >::const_iterator it = rVector.begin(), it_end = rVector.end(); it != it_end; ++it )
    {
        Reference< XArrayFormulaTokens > xTokens( getRange( it->maCellRangeAddress ), UNO_QUERY );
        OSL_ENSURE( xTokens.is(), "SheetDataBuffer::finalizeArrayFormula - missing formula token interface" );
        ApiTokenSequence aTokens = getFormulaParser().importFormula( it->maTokenAndAddress.maCellAddress, it->maTokenAndAddress.maTokenStr );
        if( xTokens.is() )
        {
            xTokens->setArrayTokens( aTokens );
            // set dependencies, add listeners on the cells in array
            for ( sal_Int32 nCol = it->maCellRangeAddress.StartColumn; nCol <=  it->maCellRangeAddress.EndColumn; ++nCol )
            {
                for ( sal_Int32 nRow = it->maCellRangeAddress.StartRow; nRow <=  it->maCellRangeAddress.EndRow; ++nRow )
                {
                    StartCellListening( it->maCellRangeAddress.Sheet, nRow, nCol, rDoc );
                }
            }
        }
    }
}

void FormulaBuffer::createSharedFormulaMapEntry(
    const table::CellAddress& rAddress, const table::CellRangeAddress& rRange,
    sal_Int32 nSharedId, const OUString& rTokens )
{
    std::vector<SharedFormulaEntry>& rSharedFormulas = maSharedFormulas[ rAddress.Sheet ];
    SharedFormulaEntry aEntry(rAddress, rRange, rTokens, nSharedId);
    rSharedFormulas.push_back( aEntry );
}

void FormulaBuffer::setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, const OUString& rTokenStr )
{
    maCellFormulas[ rAddress.Sheet ].push_back( TokenAddressItem( rTokenStr, rAddress ) );
}

void FormulaBuffer::setCellFormula(
    const table::CellAddress& rAddress, sal_Int32 nSharedId, const OUString& rCellValue, sal_Int32 nValueType )
{
    maSharedFormulaIds[rAddress.Sheet].push_back(
        SharedFormulaDesc(rAddress, nSharedId, rCellValue, nValueType));
}

void FormulaBuffer::setCellArrayFormula( const ::com::sun::star::table::CellRangeAddress& rRangeAddress, const ::com::sun::star::table::CellAddress& rTokenAddress, const OUString& rTokenStr )
{

    TokenAddressItem tokenPair( rTokenStr, rTokenAddress );
    maCellArrayFormulas[ rRangeAddress.Sheet ].push_back( TokenRangeAddressItem( tokenPair, rRangeAddress ) );
}

void FormulaBuffer::setCellFormulaValue( const ::com::sun::star::table::CellAddress& rAddress, double fValue )
{
    maCellFormulaValues[ rAddress.Sheet ].push_back( ValueAddressPair( rAddress, fValue ) );
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
