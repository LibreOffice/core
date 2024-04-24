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

#include <addressconverter.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <convuno.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <oox/core/filterbase.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <docuno.hxx>
#include <rangeutl.hxx>

namespace oox::xls {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

namespace {

//! TODO: this limit may change, is there a way to obtain it via API?
const sal_Int16 API_MAXTAB          = MAXTAB;

const sal_Int32 OOX_MAXCOL          = static_cast< sal_Int32 >( (1 << 14) - 1 );
const sal_Int32 OOX_MAXROW          = static_cast< sal_Int32 >( (1 << 20) - 1 );
const sal_Int16 OOX_MAXTAB          = static_cast< sal_Int16 >( (1 << 15) - 1 );

} // namespace


void BinAddress::read( SequenceInputStream& rStrm )
{
    mnRow = rStrm.readInt32();
    mnCol = rStrm.readInt32();
}

void BinRange::read( SequenceInputStream& rStrm )
{
    maFirst.mnRow = rStrm.readInt32();
    maLast.mnRow = rStrm.readInt32();
    maFirst.mnCol = rStrm.readInt32();
    maLast.mnCol = rStrm.readInt32();
}

void BinRangeList::read( SequenceInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    mvRanges.resize( getLimitedValue< size_t, sal_Int64 >( nCount, 0, rStrm.getRemaining() / 16 ) );
    for( auto& rRange : mvRanges )
        rRange.read( rStrm );
}

AddressConverter::AddressConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbColOverflow( false ),
    mbRowOverflow( false ),
    mbTabOverflow( false )
{
    initializeMaxPos( OOX_MAXTAB, OOX_MAXCOL, OOX_MAXROW );
}

bool AddressConverter::checkCol( sal_Int32 nCol, bool bTrackOverflow )
{
    bool bValid = (0 <= nCol) && ( nCol <= maMaxPos.Col() );
    if( !bValid && bTrackOverflow )
        mbColOverflow = true;
    return bValid;
}

bool AddressConverter::checkRow( sal_Int32 nRow, bool bTrackOverflow )
{
    bool bValid = (0 <= nRow) && ( nRow <= maMaxPos.Row() );
    if( !bValid && bTrackOverflow )
        mbRowOverflow = true;
    return bValid;
}

bool AddressConverter::checkTab( sal_Int16 nSheet, bool bTrackOverflow )
{
    bool bValid = (0 <= nSheet) && ( nSheet <= maMaxPos.Tab() );
    if( !bValid && bTrackOverflow )
        mbTabOverflow |= ( nSheet > maMaxPos.Tab() );  // do not warn for deleted refs (-1)
    return bValid;
}

bool AddressConverter::checkCellAddress( const ScAddress& rAddress, bool bTrackOverflow )
{
    return
        checkTab( rAddress.Tab(), bTrackOverflow ) &&
        checkCol( rAddress.Col(), bTrackOverflow ) &&
        checkRow( rAddress.Row(), bTrackOverflow );
}

bool AddressConverter::convertToCellAddressUnchecked( ScAddress& orAddress,
        const OUString& rString, sal_Int16 nSheet, const ScDocument& rDoc)
{
    orAddress.SetTab(nSheet);
    orAddress.SetRow(0);
    orAddress.SetCol(0);

    if (rString.isEmpty())
        return false;

    return (orAddress.Parse(rString, rDoc, formula::FormulaGrammar::CONV_XL_OOX)
            & ScRefFlags::VALID)
           == ScRefFlags::VALID;
}

bool AddressConverter::convertToCellAddress( ScAddress& orAddress,
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    return
        convertToCellAddressUnchecked(orAddress, rString, nSheet, getScDocument()) &&
        checkCellAddress( orAddress, bTrackOverflow );
}

ScAddress AddressConverter::createValidCellAddress(
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    ScAddress aAddress( 0, 0, 0 );
    if( !convertToCellAddress( aAddress, rString, nSheet, bTrackOverflow ) )
    {
        aAddress.SetTab( getLimitedValue< sal_Int16, sal_Int16 >( nSheet, 0, maMaxPos.Tab() ) );
        aAddress.SetCol( ::std::min( aAddress.Col(), maMaxPos.Col() ) );
        aAddress.SetRow( ::std::min( aAddress.Row(), maMaxPos.Row() ) );
    }
    return aAddress;
}

void AddressConverter::convertToCellAddressUnchecked( ScAddress& orAddress,
        const BinAddress& rBinAddress, sal_Int16 nSheet )
{
    orAddress.SetTab(nSheet);
    orAddress.SetCol(rBinAddress.mnCol);
    orAddress.SetRow(rBinAddress.mnRow);
}

bool AddressConverter::convertToCellAddress( ScAddress& orAddress,
        const BinAddress& rBinAddress, sal_Int16 nSheet, bool bTrackOverflow )
{
    convertToCellAddressUnchecked( orAddress, rBinAddress, nSheet );
    return checkCellAddress( orAddress, bTrackOverflow );
}

ScAddress AddressConverter::createValidCellAddress(
        const BinAddress& rBinAddress, sal_Int16 nSheet, bool bTrackOverflow )
{
    ScAddress aAddress ( 0, 0, 0 );
    if( !convertToCellAddress( aAddress, rBinAddress, nSheet, bTrackOverflow ) )
    {
        aAddress.SetTab( getLimitedValue< sal_Int16, sal_Int16 >( nSheet, 0, maMaxPos.Tab() ) );
        aAddress.SetCol( getLimitedValue< sal_Int32, sal_Int32 >( rBinAddress.mnCol, 0, sal_Int32( maMaxPos.Col() ) ) );
        aAddress.SetRow( getLimitedValue< sal_Int32, sal_Int32 >( rBinAddress.mnRow, 0, sal_Int32( maMaxPos.Row() ) ) );
    }
    return aAddress;
}

bool AddressConverter::checkCellRange( const ScRange& rRange, bool bAllowOverflow, bool bTrackOverflow )
{
    return
        (checkCol( rRange.aEnd.Col(), bTrackOverflow ) || bAllowOverflow) &&     // bAllowOverflow after checkCol to track overflow!
        (checkRow( rRange.aEnd.Row(), bTrackOverflow ) || bAllowOverflow) &&     // bAllowOverflow after checkRow to track overflow!
        checkTab( rRange.aStart.Tab(), bTrackOverflow ) &&
        checkCol( rRange.aStart.Col(), bTrackOverflow ) &&
        checkRow( rRange.aStart.Row(), bTrackOverflow );
}

bool AddressConverter::validateCellRange( ScRange& orRange, bool bAllowOverflow, bool bTrackOverflow )
{
    if( orRange.aStart.Col() > orRange.aEnd.Col() )
    {
        SCCOL nCol = orRange.aStart.Col();
        orRange.aStart.SetCol( orRange.aEnd.Col() );
        orRange.aEnd.SetCol( nCol );
    }
    if( orRange.aStart.Row() > orRange.aEnd.Row() )
    {
        SCROW nRow = orRange.aStart.Row();
        orRange.aStart.SetRow( orRange.aEnd.Row() );
        orRange.aEnd.SetRow( nRow );
    }
    if( !checkCellRange( orRange, bAllowOverflow, bTrackOverflow ) )
        return false;
    if( orRange.aEnd.Col() > maMaxPos.Col() )
        orRange.aEnd.SetCol( maMaxPos.Col() );
    if( orRange.aEnd.Row() > maMaxPos.Row() )
        orRange.aEnd.SetRow( maMaxPos.Row() );
    return true;
}

bool AddressConverter::convertToCellRangeUnchecked(ScRange& orRange, std::u16string_view aString,
                                                   sal_Int16 nSheet, const ScDocument& rDoc)
{
    orRange.aStart.SetTab(nSheet);
    orRange.aEnd.SetTab(nSheet);

    if (aString.empty())
        return false;

    sal_Int32 nOffset = 0;
    bool bRet = ScRangeStringConverter::GetRangeFromString(
        orRange, aString, rDoc, formula::FormulaGrammar::CONV_XL_OOX, nOffset);

    return bRet && nOffset > 0 && o3tl::make_unsigned(nOffset) == aString.length();
}

bool AddressConverter::convertToCellRange( ScRange& orRange,
        std::u16string_view aString, sal_Int16 nSheet, bool bAllowOverflow, bool bTrackOverflow )
{
    return
        convertToCellRangeUnchecked(orRange, aString, nSheet, getScDocument()) &&
        validateCellRange( orRange, bAllowOverflow, bTrackOverflow );
}

void AddressConverter::convertToCellRangeUnchecked( ScRange& orRange,
        const BinRange& rBinRange, sal_Int16 nSheet )
{
    orRange.aStart.SetTab( nSheet );
    orRange.aStart.SetCol( rBinRange.maFirst.mnCol );
    orRange.aStart.SetRow( rBinRange.maFirst.mnRow );
    orRange.aEnd.SetTab( nSheet );
    orRange.aEnd.SetCol( rBinRange.maLast.mnCol );
    orRange.aEnd.SetRow( rBinRange.maLast.mnRow );
}

bool AddressConverter::convertToCellRange( ScRange& orRange,
        const BinRange& rBinRange, sal_Int16 nSheet, bool bAllowOverflow, bool bTrackOverflow )
{
    convertToCellRangeUnchecked( orRange, rBinRange, nSheet );
    return validateCellRange( orRange, bAllowOverflow, bTrackOverflow );
}

void AddressConverter::validateCellRangeList( ScRangeList& orRanges, bool bTrackOverflow )
{
    for( size_t nIndex = orRanges.size(); nIndex > 0; --nIndex )
        if( !validateCellRange( orRanges[ nIndex - 1 ], true, bTrackOverflow ) )
            orRanges.Remove( nIndex - 1 );
}

void AddressConverter::convertToCellRangeList( ScRangeList& orRanges,
        std::u16string_view aString, sal_Int16 nSheet, bool bTrackOverflow )
{
    size_t nPos = 0;
    size_t nLen = aString.size();
    ScRange aRange;
    while( nPos != std::u16string_view::npos && (nPos < nLen) )
    {
        std::u16string_view aToken = o3tl::getToken(aString, u' ', nPos );
        if( !aToken.empty() && convertToCellRange( aRange, aToken, nSheet, true, bTrackOverflow ) )
            orRanges.push_back(aRange);
    }
}

void AddressConverter::convertToCellRangeList( ScRangeList& orRanges,
        const BinRangeList& rBinRanges, sal_Int16 nSheet, bool bTrackOverflow )
{
    ScRange aRange;
    for( const auto& rBinRange : rBinRanges )
        if( convertToCellRange( aRange, rBinRange, nSheet, true, bTrackOverflow ) )
            orRanges.push_back( aRange );
}

Sequence<CellRangeAddress> AddressConverter::toApiSequence(const ScRangeList& orRanges)
{
    const size_t nSize = orRanges.size();
    Sequence<CellRangeAddress> aRangeSequence(nSize);
    CellRangeAddress* pApiRanges = aRangeSequence.getArray();
    for (size_t i = 0; i < nSize; ++i)
    {
        ScUnoConversion::FillApiRange(pApiRanges[i], orRanges[i]);
    }
    return aRangeSequence;
}

// private --------------------------------------------------------------------

void AddressConverter::initializeMaxPos(
        sal_Int16 nMaxXlsTab, sal_Int32 nMaxXlsCol, sal_Int32 nMaxXlsRow )
{
    maMaxXlsPos.Set( nMaxXlsCol, nMaxXlsRow, nMaxXlsTab);

    // maximum cell position in Calc
    try
    {
        Reference< XIndexAccess > xSheetsIA( getDocument()->getSheets(), UNO_QUERY_THROW );
        Reference< XCellRangeAddressable > xAddressable( xSheetsIA->getByIndex( 0 ), UNO_QUERY_THROW );
        CellRangeAddress aRange = xAddressable->getRangeAddress();
        maMaxApiPos = ScAddress( aRange.EndColumn, aRange.EndRow, API_MAXTAB );
        maMaxPos = getBaseFilter().isImportFilter() ? maMaxApiPos : maMaxXlsPos;
    }
    catch( Exception& )
    {
        OSL_FAIL( "AddressConverter::AddressConverter - cannot get sheet limits" );
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
