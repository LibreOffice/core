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

#include "addressconverter.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/helper/containerhelper.hxx>
#include "biffinputstream.hxx"

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

const sal_Int32 BIFF2_MAXCOL        = 255;
const sal_Int32 BIFF2_MAXROW        = 16383;
const sal_Int16 BIFF2_MAXTAB        = 0;

const sal_Int32 BIFF3_MAXCOL        = BIFF2_MAXCOL;
const sal_Int32 BIFF3_MAXROW        = BIFF2_MAXROW;
const sal_Int16 BIFF3_MAXTAB        = BIFF2_MAXTAB;

const sal_Int32 BIFF4_MAXCOL        = BIFF3_MAXCOL;
const sal_Int32 BIFF4_MAXROW        = BIFF3_MAXROW;
const sal_Int16 BIFF4_MAXTAB        = 32767;

const sal_Int32 BIFF5_MAXCOL        = BIFF4_MAXCOL;
const sal_Int32 BIFF5_MAXROW        = BIFF4_MAXROW;
const sal_Int16 BIFF5_MAXTAB        = BIFF4_MAXTAB;

const sal_Int32 BIFF8_MAXCOL        = BIFF5_MAXCOL;
const sal_Int32 BIFF8_MAXROW        = 65535;
const sal_Int16 BIFF8_MAXTAB        = BIFF5_MAXTAB;

} // namespace

CellAddress ApiCellRangeList::getBaseAddress() const
{
    if( mvAddresses.empty() )
        return CellAddress();
    return CellAddress( mvAddresses.front().Sheet, mvAddresses.front().StartColumn, mvAddresses.front().StartRow );
}

css::uno::Sequence< CellRangeAddress > ApiCellRangeList::toSequence() const
{
    return ContainerHelper::vectorToSequence( mvAddresses );
}

void BinAddress::read( SequenceInputStream& rStrm )
{
    mnRow = rStrm.readInt32();
    mnCol = rStrm.readInt32();
}

void BinAddress::read( BiffInputStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    mnRow = bRow32Bit ? rStrm.readInt32() : rStrm.readuInt16();
    mnCol = bCol16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
}

void BinRange::read( SequenceInputStream& rStrm )
{
    maFirst.mnRow = rStrm.readInt32();
    maLast.mnRow = rStrm.readInt32();
    maFirst.mnCol = rStrm.readInt32();
    maLast.mnCol = rStrm.readInt32();
}

void BinRange::read( BiffInputStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    maFirst.mnRow = bRow32Bit ? rStrm.readInt32() : rStrm.readuInt16();
    maLast.mnRow = bRow32Bit ? rStrm.readInt32() : rStrm.readuInt16();
    maFirst.mnCol = bCol16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
    maLast.mnCol = bCol16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
}

void BinRangeList::read( SequenceInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    mvRanges.resize( getLimitedValue< size_t, sal_Int64 >( nCount, 0, rStrm.getRemaining() / 16 ) );
    for( ::std::vector< BinRange >::iterator aIt = mvRanges.begin(), aEnd = mvRanges.end(); aIt != aEnd; ++aIt )
        aIt->read( rStrm );
}

AddressConverter::AddressConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbColOverflow( false ),
    mbRowOverflow( false ),
    mbTabOverflow( false )
{
    maDConChars.set( 0xFFFF, '\x01', 0xFFFF, '\x02', 0xFFFF );
    switch( getFilterType() )
    {
        case FILTER_OOXML:
            initializeMaxPos( OOX_MAXTAB, OOX_MAXCOL, OOX_MAXROW );
            maLinkChars.set( 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF );
        break;
        case FILTER_BIFF: switch( getBiff() )
        {
            case BIFF2:
                initializeMaxPos( BIFF2_MAXTAB, BIFF2_MAXCOL, BIFF2_MAXROW );
                maLinkChars.set( 0xFFFF, '\x01', '\x02', 0xFFFF, 0xFFFF );
            break;
            case BIFF3:
                initializeMaxPos( BIFF3_MAXTAB, BIFF3_MAXCOL, BIFF3_MAXROW );
                maLinkChars.set( 0xFFFF, '\x01', '\x02', 0xFFFF, 0xFFFF );
            break;
            case BIFF4:
                initializeMaxPos( BIFF4_MAXTAB, BIFF4_MAXCOL, BIFF4_MAXROW );
                maLinkChars.set( 0xFFFF, '\x01', '\x02', 0xFFFF, '\x00' );
            break;
            case BIFF5:
                initializeMaxPos( BIFF5_MAXTAB, BIFF5_MAXCOL, BIFF5_MAXROW );
                maLinkChars.set( '\x04', '\x01', '\x02', '\x03', '\x00' );
            break;
            case BIFF8:
                initializeMaxPos( BIFF8_MAXTAB, BIFF8_MAXCOL, BIFF8_MAXROW );
                maLinkChars.set( '\x04', '\x01', 0xFFFF, '\x02', '\x00' );
            break;
            case BIFF_UNKNOWN:
                initializeMaxPos( 0, 0, 0 );
                maLinkChars.set( 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF );
            break;
        }
        break;
        case FILTER_UNKNOWN:
            initializeMaxPos( 0, 0, 0 );
            maLinkChars.set( 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF );
        break;
    }
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
                    (cChar -= 'a') += 'A';
                if( ('A' <= cChar) && (cChar <= 'Z') )
                {
                    /*  Return, if 1-based column index is already 6 characters
                        long (12356631 is column index for column AAAAAA). */
                    if( ornColumn >= 12356631 )
                        return false;
                    (ornColumn *= 26) += (cChar - 'A' + 1);
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
                    (ornRow *= 10) += (cChar - '0');
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
                    (cChar -= 'a') += 'A';
                if( ('A' <= cChar) && (cChar <= 'Z') )
                {
                    /*  Return, if 1-based column index is already 6 characters
                        long (12356631 is column index for column AAAAAA). */
                    if( ornColumn >= 12356631 )
                        return false;
                    (ornColumn *= 26) += (cChar - 'A' + 1);
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
                    (ornRow *= 10) += (cChar - '0');
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
        const OUString& rString, sal_Int32 nStart, sal_Int32 nLength )
{
    ornStartColumn = ornStartRow = ornEndColumn = ornEndRow = 0;
    if( (nStart < 0) || (nStart >= rString.getLength()) || (nLength < 2) )
        return false;

    sal_Int32 nEnd = nStart + ::std::min( nLength, rString.getLength() - nStart );
    sal_Int32 nColonPos = rString.indexOf( ':', nStart );
    if( (nStart < nColonPos) && (nColonPos + 1 < nEnd) )
    {
        return
            parseOoxAddress2d( ornStartColumn, ornStartRow, rString, nStart, nColonPos - nStart ) &&
            parseOoxAddress2d( ornEndColumn, ornEndRow, rString, nColonPos + 1, nLength - nColonPos - 1 );
    }

    if( parseOoxAddress2d( ornStartColumn, ornStartRow, rString, nStart, nLength ) )
    {
        ornEndColumn = ornStartColumn;
        ornEndRow = ornStartRow;
        return true;
    }

    return false;
}

bool AddressConverter::checkCol( sal_Int32 nCol, bool bTrackOverflow )
{
    bool bValid = (0 <= nCol) && (nCol <= maMaxPos.Column);
    if( !bValid && bTrackOverflow )
        mbColOverflow = true;
    return bValid;
}

bool AddressConverter::checkRow( sal_Int32 nRow, bool bTrackOverflow )
{
    bool bValid = (0 <= nRow) && (nRow <= maMaxPos.Row);
    if( !bValid && bTrackOverflow )
        mbRowOverflow = true;
    return bValid;
}

bool AddressConverter::checkTab( sal_Int16 nSheet, bool bTrackOverflow )
{
    bool bValid = (0 <= nSheet) && (nSheet <= maMaxPos.Sheet);
    if( !bValid && bTrackOverflow )
        mbTabOverflow |= (nSheet > maMaxPos.Sheet);  // do not warn for deleted refs (-1)
    return bValid;
}

bool AddressConverter::checkCellAddress( const CellAddress& rAddress, bool bTrackOverflow )
{
    return
        checkTab( rAddress.Sheet, bTrackOverflow ) &&
        checkCol( rAddress.Column, bTrackOverflow ) &&
        checkRow( rAddress.Row, bTrackOverflow );
}

bool AddressConverter::convertToCellAddressUnchecked( CellAddress& orAddress,
        const OUString& rString, sal_Int16 nSheet )
{
    orAddress.Sheet = nSheet;
    return parseOoxAddress2d( orAddress.Column, orAddress.Row, rString );
}

bool AddressConverter::convertToCellAddressUnchecked(
        css::table::CellAddress& orAddress, const char* pStr, sal_Int16 nSheet )
{
    orAddress.Sheet = nSheet;
    return parseOoxAddress2d(orAddress.Column, orAddress.Row, pStr);
}

bool AddressConverter::convertToCellAddress( CellAddress& orAddress,
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    return
        convertToCellAddressUnchecked( orAddress, rString, nSheet ) &&
        checkCellAddress( orAddress, bTrackOverflow );
}

bool AddressConverter::convertToCellAddress(
    css::table::CellAddress& rAddress,
    const char* pStr, sal_Int16 nSheet, bool bTrackOverflow )
{
    if (!convertToCellAddressUnchecked(rAddress, pStr, nSheet))
        return false;

    return checkCellAddress(rAddress, bTrackOverflow);
}

CellAddress AddressConverter::createValidCellAddress(
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    CellAddress aAddress;
    if( !convertToCellAddress( aAddress, rString, nSheet, bTrackOverflow ) )
    {
        aAddress.Sheet  = getLimitedValue< sal_Int16, sal_Int16 >( nSheet, 0, maMaxPos.Sheet );
        aAddress.Column = ::std::min( aAddress.Column, maMaxPos.Column );
        aAddress.Row    = ::std::min( aAddress.Row, maMaxPos.Row );
    }
    return aAddress;
}

void AddressConverter::convertToCellAddressUnchecked( CellAddress& orAddress,
        const BinAddress& rBinAddress, sal_Int16 nSheet )
{
    orAddress.Sheet  = nSheet;
    orAddress.Column = rBinAddress.mnCol;
    orAddress.Row    = rBinAddress.mnRow;
}

bool AddressConverter::convertToCellAddress( CellAddress& orAddress,
        const BinAddress& rBinAddress, sal_Int16 nSheet, bool bTrackOverflow )
{
    convertToCellAddressUnchecked( orAddress, rBinAddress, nSheet );
    return checkCellAddress( orAddress, bTrackOverflow );
}

CellAddress AddressConverter::createValidCellAddress(
        const BinAddress& rBinAddress, sal_Int16 nSheet, bool bTrackOverflow )
{
    CellAddress aAddress;
    if( !convertToCellAddress( aAddress, rBinAddress, nSheet, bTrackOverflow ) )
    {
        aAddress.Sheet  = getLimitedValue< sal_Int16, sal_Int16 >( nSheet, 0, maMaxPos.Sheet );
        aAddress.Column = getLimitedValue< sal_Int32, sal_Int32 >( rBinAddress.mnCol, 0, maMaxPos.Column );
        aAddress.Row    = getLimitedValue< sal_Int32, sal_Int32 >( rBinAddress.mnRow, 0, maMaxPos.Row );
    }
    return aAddress;
}

bool AddressConverter::checkCellRange( const CellRangeAddress& rRange, bool bAllowOverflow, bool bTrackOverflow )
{
    return
        (checkCol( rRange.EndColumn, bTrackOverflow ) || bAllowOverflow) &&     // bAllowOverflow after checkCol to track overflow!
        (checkRow( rRange.EndRow, bTrackOverflow ) || bAllowOverflow) &&        // bAllowOverflow after checkRow to track overflow!
        checkTab( rRange.Sheet, bTrackOverflow ) &&
        checkCol( rRange.StartColumn, bTrackOverflow ) &&
        checkRow( rRange.StartRow, bTrackOverflow );
}

bool AddressConverter::validateCellRange( CellRangeAddress& orRange, bool bAllowOverflow, bool bTrackOverflow )
{
    if( orRange.StartColumn > orRange.EndColumn )
        ::std::swap( orRange.StartColumn, orRange.EndColumn );
    if( orRange.StartRow > orRange.EndRow )
        ::std::swap( orRange.StartRow, orRange.EndRow );
    if( !checkCellRange( orRange, bAllowOverflow, bTrackOverflow ) )
        return false;
    if( orRange.EndColumn > maMaxPos.Column )
        orRange.EndColumn = maMaxPos.Column;
    if( orRange.EndRow > maMaxPos.Row )
        orRange.EndRow = maMaxPos.Row;
    return true;
}

bool AddressConverter::convertToCellRangeUnchecked( CellRangeAddress& orRange,
        const OUString& rString, sal_Int16 nSheet )
{
    orRange.Sheet = nSheet;
    return parseOoxRange2d( orRange.StartColumn, orRange.StartRow, orRange.EndColumn, orRange.EndRow, rString );
}

bool AddressConverter::convertToCellRange( CellRangeAddress& orRange,
        const OUString& rString, sal_Int16 nSheet, bool bAllowOverflow, bool bTrackOverflow )
{
    return
        convertToCellRangeUnchecked( orRange, rString, nSheet ) &&
        validateCellRange( orRange, bAllowOverflow, bTrackOverflow );
}

void AddressConverter::convertToCellRangeUnchecked( CellRangeAddress& orRange,
        const BinRange& rBinRange, sal_Int16 nSheet )
{
    orRange.Sheet       = nSheet;
    orRange.StartColumn = rBinRange.maFirst.mnCol;
    orRange.StartRow    = rBinRange.maFirst.mnRow;
    orRange.EndColumn   = rBinRange.maLast.mnCol;
    orRange.EndRow      = rBinRange.maLast.mnRow;
}

bool AddressConverter::convertToCellRange( CellRangeAddress& orRange,
        const BinRange& rBinRange, sal_Int16 nSheet, bool bAllowOverflow, bool bTrackOverflow )
{
    convertToCellRangeUnchecked( orRange, rBinRange, nSheet );
    return validateCellRange( orRange, bAllowOverflow, bTrackOverflow );
}

void AddressConverter::validateCellRangeList( ApiCellRangeList& orRanges, bool bTrackOverflow )
{
    for( size_t nIndex = orRanges.size(); nIndex > 0; --nIndex )
        if( !validateCellRange( orRanges[ nIndex - 1 ], true, bTrackOverflow ) )
            orRanges.erase( orRanges.begin() + nIndex - 1 );
}

void AddressConverter::convertToCellRangeList( ApiCellRangeList& orRanges,
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rString.getLength();
    CellRangeAddress aRange;
    while( (0 <= nPos) && (nPos < nLen) )
    {
        OUString aToken = rString.getToken( 0, ' ', nPos );
        if( !aToken.isEmpty() && convertToCellRange( aRange, aToken, nSheet, true, bTrackOverflow ) )
            orRanges.push_back( aRange );
    }
}

void AddressConverter::convertToCellRangeList( ApiCellRangeList& orRanges,
        const BinRangeList& rBinRanges, sal_Int16 nSheet, bool bTrackOverflow )
{
    CellRangeAddress aRange;
    for( ::std::vector< BinRange >::const_iterator aIt = rBinRanges.begin(), aEnd = rBinRanges.end(); aIt != aEnd; ++aIt )
        if( convertToCellRange( aRange, *aIt, nSheet, true, bTrackOverflow ) )
            orRanges.push_back( aRange );
}

// private --------------------------------------------------------------------

void AddressConverter::ControlCharacters::set(
        sal_Unicode cThisWorkbook, sal_Unicode cExternal,
        sal_Unicode cThisSheet, sal_Unicode cInternal, sal_Unicode cSameSheet )
{
    mcThisWorkbook = cThisWorkbook;
    mcExternal     = cExternal;
    mcThisSheet    = cThisSheet;
    mcInternal     = cInternal;
    mcSameSheet    = cSameSheet;
}

void AddressConverter::initializeMaxPos(
        sal_Int16 nMaxXlsTab, sal_Int32 nMaxXlsCol, sal_Int32 nMaxXlsRow )
{
    maMaxXlsPos.Sheet  = nMaxXlsTab;
    maMaxXlsPos.Column = nMaxXlsCol;
    maMaxXlsPos.Row    = nMaxXlsRow;

    // maximum cell position in Calc
    try
    {
        Reference< XIndexAccess > xSheetsIA( getDocument()->getSheets(), UNO_QUERY_THROW );
        Reference< XCellRangeAddressable > xAddressable( xSheetsIA->getByIndex( 0 ), UNO_QUERY_THROW );
        CellRangeAddress aRange = xAddressable->getRangeAddress();
        maMaxApiPos = CellAddress( API_MAXTAB, aRange.EndColumn, aRange.EndRow );
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
