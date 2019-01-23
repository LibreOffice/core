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
#include <osl/diagnose.h>
#include <oox/core/filterbase.hxx>
#include <oox/helper/binaryinputstream.hxx>

namespace oox {
namespace xls {

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

bool AddressConverter::parseOoxAddress2d(
        sal_Int32& ornColumn, sal_Int32& ornRow,
        const OUString& rString, sal_Int32 nStart, sal_Int32 nLength )
{
    ornColumn = ornRow = 0;
    if( (nStart < 0) || (nStart >= rString.getLength()) || (nLength < 2) )
        return false;

    const sal_Unicode* pcChar = rString.getStr() + nStart;
    const sal_Unicode* pcEndChar = pcChar + ::std::min( nLength, rString.getLength() - nStart );

    enum { STATE_COL, STATE_ROW } eState = STATE_COL;
    while( pcChar < pcEndChar )
    {
        sal_Unicode cChar = *pcChar;
        switch( eState )
        {
            case STATE_COL:
            {
                if( ('a' <= cChar) && (cChar <= 'z') )
                    cChar = (cChar - 'a') + 'A';
                if( ('A' <= cChar) && (cChar <= 'Z') )
                {
                    /*  Return, if 1-based column index is already 6 characters
                        long (12356631 is column index for column AAAAAA). */
                    if( ornColumn >= 12356631 )
                        return false;
                    ornColumn = (ornColumn * 26) + (cChar - 'A' + 1);
                }
                else if( ornColumn > 0 )
                {
                    --pcChar;
                    eState = STATE_ROW;
                }
                else
                    return false;
            }
            break;

            case STATE_ROW:
            {
                if( ('0' <= cChar) && (cChar <= '9') )
                {
                    // return, if 1-based row is already 9 digits long
                    if( ornRow >= 100000000 )
                        return false;
                    ornRow = (ornRow * 10) + (cChar - '0');
                }
                else
                    return false;
            }
            break;
        }
        ++pcChar;
    }

    --ornColumn;
    --ornRow;
    return (ornColumn >= 0) && (ornRow >= 0);
}

bool AddressConverter::parseOoxAddress2d( sal_Int32& ornColumn, sal_Int32& ornRow, const char* pStr )
{
    ornColumn = ornRow = 0;

    enum { STATE_COL, STATE_ROW } eState = STATE_COL;

    while (*pStr)
    {
        char cChar = *pStr;
        switch( eState )
        {
            case STATE_COL:
            {
                if( ('a' <= cChar) && (cChar <= 'z') )
                    cChar = (cChar - 'a') + 'A';
                if( ('A' <= cChar) && (cChar <= 'Z') )
                {
                    /*  Return, if 1-based column index is already 6 characters
                        long (12356631 is column index for column AAAAAA). */
                    if( ornColumn >= 12356631 )
                        return false;
                    ornColumn = (ornColumn * 26) + (cChar - 'A' + 1);
                }
                else if( ornColumn > 0 )
                {
                    --pStr;
                    eState = STATE_ROW;
                }
                else
                    return false;
            }
            break;

            case STATE_ROW:
            {
                if( ('0' <= cChar) && (cChar <= '9') )
                {
                    // return, if 1-based row is already 9 digits long
                    if( ornRow >= 100000000 )
                        return false;
                    ornRow = (ornRow * 10) + (cChar - '0');
                }
                else
                    return false;
            }
            break;
        }
        ++pStr;
    }

    --ornColumn;
    --ornRow;
    return (ornColumn >= 0) && (ornRow >= 0);
}

bool AddressConverter::parseOoxRange2d(
        sal_Int32& ornStartColumn, sal_Int32& ornStartRow,
        sal_Int32& ornEndColumn, sal_Int32& ornEndRow,
        const OUString& rString, sal_Int32 nStart )
{
    ornStartColumn = ornStartRow = ornEndColumn = ornEndRow = 0;
    if( (nStart < 0) || (nStart >= rString.getLength()) )
        return false;

    sal_Int32 nEnd = nStart + ( rString.getLength() - nStart );
    sal_Int32 nColonPos = rString.indexOf( ':', nStart );
    if( (nStart < nColonPos) && (nColonPos + 1 < nEnd) )
    {
        return
            parseOoxAddress2d( ornStartColumn, ornStartRow, rString, nStart, nColonPos - nStart ) &&
            parseOoxAddress2d( ornEndColumn, ornEndRow, rString, nColonPos + 1, SAL_MAX_INT32 - nColonPos - 1 );
    }

    if( parseOoxAddress2d( ornStartColumn, ornStartRow, rString, nStart ) )
    {
        ornEndColumn = ornStartColumn;
        ornEndRow = ornStartRow;
        return true;
    }

    return false;
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
        const OUString& rString, sal_Int16 nSheet )
{
    orAddress.SetTab(nSheet);
    sal_Int32 nCol = 0;
    sal_Int32 nRow = 0;
    bool bRes = parseOoxAddress2d( nCol, nRow, rString );
    orAddress.SetRow(nRow);
    orAddress.SetCol(nCol);

    return bRes;
}

bool AddressConverter::convertToCellAddressUnchecked(
        ScAddress& orAddress, const char* pStr, sal_Int16 nSheet )
{
    orAddress.SetTab(nSheet);
    sal_Int32 nCol = 0;
    sal_Int32 nRow = 0;
    bool bRes = parseOoxAddress2d(nCol, nRow, pStr);
    orAddress.SetRow(nRow);
    orAddress.SetCol(nCol);

    return bRes;
}

bool AddressConverter::convertToCellAddress( ScAddress& orAddress,
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    return
        convertToCellAddressUnchecked( orAddress, rString, nSheet ) &&
        checkCellAddress( orAddress, bTrackOverflow );
}

bool AddressConverter::convertToCellAddress(
    ScAddress& rAddress,
    const char* pStr, sal_Int16 nSheet, bool bTrackOverflow )
{
    if (!convertToCellAddressUnchecked(rAddress, pStr, nSheet))
        return false;

    return checkCellAddress(rAddress, bTrackOverflow);
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

bool AddressConverter::convertToCellRangeUnchecked( ScRange& orRange,
        const OUString& rString, sal_Int16 nSheet )
{
    orRange.aStart.SetTab( nSheet );
    orRange.aEnd.SetTab( nSheet );
    sal_Int32 aStartCol = orRange.aStart.Col();
    sal_Int32 aStartRow = orRange.aStart.Row();
    sal_Int32 aEndCol = orRange.aEnd.Col();
    sal_Int32 aEndRow = orRange.aEnd.Row();
    bool bReturnValue = parseOoxRange2d( aStartCol, aStartRow, aEndCol, aEndRow, rString );
    orRange.aStart.SetCol( aStartCol );
    orRange.aStart.SetRow( aStartRow );
    orRange.aEnd.SetCol( aEndCol );
    orRange.aEnd.SetRow( aEndRow );
    return bReturnValue;
}

bool AddressConverter::convertToCellRange( ScRange& orRange,
        const OUString& rString, sal_Int16 nSheet, bool bAllowOverflow, bool bTrackOverflow )
{
    return
        convertToCellRangeUnchecked( orRange, rString, nSheet ) &&
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
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rString.getLength();
    ScRange aRange;
    while( (0 <= nPos) && (nPos < nLen) )
    {
        OUString aToken = rString.getToken( 0, ' ', nPos );
        if( !aToken.isEmpty() && convertToCellRange( aRange, aToken, nSheet, true, bTrackOverflow ) )
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

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
