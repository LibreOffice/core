/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
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
#include "cell.hxx"
#include "document.hxx"
#include "convuno.hxx"

#include "rangelst.hxx"
#include "autonamecache.hxx"
#include "tokenuno.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::container;

FormulaBuffer::FormulaBuffer( const WorkbookHelper& rHelper ) : WorkbookHelper( rHelper )
{
}

Reference< XCellRange > FormulaBuffer::getRange( const CellRangeAddress& rRange)
{
    Reference< XCellRange > xRange;
    try
    {
        xRange = mxCurrSheet->getCellRangeByPosition( rRange.StartColumn, rRange.StartRow, rRange.EndColumn, rRange.EndRow );
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
    for ( sal_Int16 nTab = 0, nElem = xSheets->getCount(); nTab < nElem; ++nTab )
    {
        double fPosition = static_cast< double> (nTab + 1) /static_cast<double>(nElem);
        xFormulaBar->setPosition( fPosition );
        mxCurrSheet = getSheetFromDoc( nTab );
        // process shared Formula
        SheetToFormulaEntryMap::iterator sharedIt = sharedFormulas.find( nTab );
        if ( sharedIt != sharedFormulas.end() )
        {
            // shared id ( to create the special shared names from )
            std::vector<SharedFormulaEntry>& rSharedFormulas = sharedIt->second;
            for ( std::vector<SharedFormulaEntry>::iterator it = rSharedFormulas.begin(), it_end = rSharedFormulas.end(); it != it_end; ++it )
            {
                 createSharedFormula( it->maAddress, it->mnSharedId, it->maTokenStr );
            }
        }
        // now process any defined shared formulae
        SheetToSharedFormulaid::iterator formulDescIt = sharedFormulaIds.find( nTab );
        SheetToSharedIdToTokenIndex::iterator tokensIt = tokenIndexes.find( nTab );
        if ( formulDescIt != sharedFormulaIds.end() && tokensIt !=  tokenIndexes.end() )
        {
            SharedIdToTokenIndex& rTokenIdMap = tokensIt->second;
            std::vector< SharedFormulaDesc >& rVector = formulDescIt->second;
            for ( std::vector< SharedFormulaDesc >::iterator it = rVector.begin(), it_end = rVector.end(); it != it_end; ++it )
            {
                // see if we have a
                // resolved tokenId
                CellAddress& rAddress = it->first;
                sal_Int32& rnSharedId = it->second;
                SharedIdToTokenIndex::iterator itTokenId =  rTokenIdMap.find( rnSharedId );
                if ( itTokenId != rTokenIdMap.end() )
                {
                    ApiTokenSequence aTokens =  getFormulaParser().convertNameToFormula( itTokenId->second );
                    applyCellFormula( rDoc, aTokens, rAddress );
                }
            }
        }

        FormulaDataMap::iterator cellIt = cellFormulas.find( nTab );
        if ( cellIt != cellFormulas.end() )
        {
            applyCellFormulas( cellIt->second );
        }

        ArrayFormulaDataMap::iterator itArray = cellArrayFormulas.find( nTab );
        if ( itArray != cellArrayFormulas.end() )
        {
            applyArrayFormulas( itArray->second );
        }

        FormulaValueMap::iterator itValues = cellFormulaValues.find( nTab );
        if ( itValues != cellFormulaValues.end() )
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
        ScBaseCell* pNewCell = new ScFormulaCell( &rDoc, aCellPos, &aTokenArray );
        rDoc.PutCell( aCellPos, pNewCell, sal_True );
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
        ScBaseCell* pBaseCell = rDoc.GetCell( aCellPos );
        SAL_WARN_IF( !pBaseCell, "sc", "why is the formula not imported? bug?");
        if ( pBaseCell && pBaseCell->GetCellType() == CELLTYPE_FORMULA )
        {
            ScFormulaCell* pCell = static_cast< ScFormulaCell* >( pBaseCell );
            pCell->SetHybridDouble( it->second );
            pCell->ResetDirty();
            pCell->ResetChanged();
        }
    }
}

void FormulaBuffer::applyArrayFormulas( const std::vector< TokenRangeAddressItem >& rVector )
{
    for ( std::vector< TokenRangeAddressItem >::const_iterator it = rVector.begin(), it_end = rVector.end(); it != it_end; ++it )
    {
        Reference< XArrayFormulaTokens > xTokens( getRange( it->maCellRangeAddress ), UNO_QUERY );
        OSL_ENSURE( xTokens.is(), "SheetDataBuffer::finalizeArrayFormula - missing formula token interface" );
        ApiTokenSequence aTokens = getFormulaParser().importFormula( it->maTokenAndAddress.maCellAddress, it->maTokenAndAddress.maTokenStr );
        if( xTokens.is() )
            xTokens->setArrayTokens( aTokens );
    }
}

void FormulaBuffer::createSharedFormulaMapEntry( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nSharedId, const rtl::OUString& rTokens )
{
     std::vector<SharedFormulaEntry>& rSharedFormulas = sharedFormulas[ rAddress.Sheet ];
    SharedFormulaEntry aEntry( rAddress, rTokens, nSharedId );
    rSharedFormulas.push_back( aEntry );
}

void FormulaBuffer::setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, const rtl::OUString& rTokenStr )
{
    cellFormulas[ rAddress.Sheet ].push_back( TokenAddressItem( rTokenStr, rAddress ) );
}

void FormulaBuffer::setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nSharedId )
{
    sharedFormulaIds[ rAddress.Sheet ].push_back( SharedFormulaDesc( rAddress, nSharedId ) );
}

void FormulaBuffer::setCellArrayFormula( const ::com::sun::star::table::CellRangeAddress& rRangeAddress, const ::com::sun::star::table::CellAddress& rTokenAddress, const rtl::OUString& rTokenStr )
{

    TokenAddressItem tokenPair( rTokenStr, rTokenAddress );
    cellArrayFormulas[ rRangeAddress.Sheet ].push_back( TokenRangeAddressItem( tokenPair, rRangeAddress ) );
}

void FormulaBuffer::setCellFormulaValue( const ::com::sun::star::table::CellAddress& rAddress, double fValue )
{
    cellFormulaValues[ rAddress.Sheet ].push_back( ValueAddressPair( rAddress, fValue ) );
}

void  FormulaBuffer::createSharedFormula( const ::com::sun::star::table::CellAddress& rAddress,  sal_Int32 nSharedId, const rtl::OUString& rTokenStr )
{
    ApiTokenSequence aTokens = getFormulaParser().importFormula( rAddress, rTokenStr );
    rtl::OUString aName = rtl::OUStringBuffer().appendAscii( RTL_CONSTASCII_STRINGPARAM( "__shared_" ) ).
        append( static_cast< sal_Int32 >( rAddress.Sheet + 1 ) ).
        append( sal_Unicode( '_' ) ).append( nSharedId ).
        append( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_0") ) ).makeStringAndClear();
    ScRangeData* pScRangeData  = createNamedRangeObject( aName, aTokens, 0  );

    pScRangeData->SetType(RT_SHARED);
    sal_Int32 nTokenIndex = static_cast< sal_Int32 >( pScRangeData->GetIndex() );

        // store the token index in the map
   tokenIndexes[  rAddress.Sheet ][ nSharedId ] = nTokenIndex;
}
} // namespace xls
} // namespace oox
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
