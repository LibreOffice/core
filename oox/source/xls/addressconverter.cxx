/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: addressconverter.cxx,v $
 * $Revision: 1.4.20.1 $
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

#include "oox/xls/addressconverter.hxx"
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/biffoutputstream.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OStringBuffer;
using ::rtl::OUStringToOString;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::XCellRangeAddressable;

namespace oox {
namespace xls {

// ============================================================================

namespace {

//! TODO: this limit may be changed
const sal_Int16 API_MAXTAB          = 255;

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

const sal_Unicode BIFF_URL_DRIVE    = '\x01';       /// DOS drive letter or UNC path.
const sal_Unicode BIFF_URL_ROOT     = '\x02';       /// Root directory of current drive.
const sal_Unicode BIFF_URL_SUBDIR   = '\x03';       /// Subdirectory delimiter.
const sal_Unicode BIFF_URL_PARENT   = '\x04';       /// Parent directory.
const sal_Unicode BIFF_URL_RAW      = '\x05';       /// Unencoded URL.
const sal_Unicode BIFF_URL_INSTALL  = '\x06';       /// Application installation directory.
const sal_Unicode BIFF_URL_INSTALL2 = '\x07';       /// Alternative application installation directory.
const sal_Unicode BIFF_URL_LIBRARY  = '\x08';       /// Library directory in application installation.
const sal_Unicode BIFF4_URL_SHEET   = '\x09';       /// BIFF4 internal sheet.
const sal_Unicode BIFF_URL_UNC      = '@';          /// UNC path root.

const sal_Unicode BIFF_DCON_ENCODED = '\x01';       /// First character of an encoded path from DCON* records.
const sal_Unicode BIFF_DCON_INTERN  = '\x02';       /// First character of an encoded sheet name from DCON* records.


inline sal_uInt16 lclGetBiffAddressSize( bool bCol16Bit, bool bRow32Bit )
{
    return (bCol16Bit ? 2 : 1) + (bRow32Bit ? 4 : 2);
}

inline sal_uInt16 lclGetBiffRangeSize( bool bCol16Bit, bool bRow32Bit )
{
    return 2 * lclGetBiffAddressSize( bCol16Bit, bRow32Bit );
}

} // namespace

// ============================================================================
// ============================================================================

CellAddress ApiCellRangeList::getBaseAddress() const
{
    if( empty() )
        return CellAddress();
    return CellAddress( front().Sheet, front().StartColumn, front().StartRow );
}

// ============================================================================

void BinAddress::read( RecordInputStream& rStrm )
{
    rStrm >> mnRow >> mnCol;
}

void BinAddress::read( BiffInputStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    mnRow = bRow32Bit ? rStrm.readInt32() : rStrm.readuInt16();
    mnCol = bCol16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
}

void BinAddress::write( BiffOutputStream& rStrm, bool bCol16Bit, bool bRow32Bit ) const
{
    if( bRow32Bit )
        rStrm << mnRow;
    else
        rStrm << static_cast< sal_uInt16 >( mnRow );
    if( bCol16Bit )
        rStrm << static_cast< sal_uInt16 >( mnCol );
    else
        rStrm << static_cast< sal_uInt8 >( mnCol );
}

// ============================================================================

bool BinRange::contains( const BinAddress& rAddr ) const
{
    return  (maFirst.mnCol <= rAddr.mnCol) && (rAddr.mnCol <= maLast.mnCol) &&
            (maFirst.mnRow <= rAddr.mnRow) && (rAddr.mnRow <= maLast.mnRow);
}

void BinRange::read( RecordInputStream& rStrm )
{
    rStrm >> maFirst.mnRow >> maLast.mnRow >> maFirst.mnCol >> maLast.mnCol;
}

void BinRange::read( BiffInputStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    maFirst.mnRow = bRow32Bit ? rStrm.readInt32() : rStrm.readuInt16();
    maLast.mnRow = bRow32Bit ? rStrm.readInt32() : rStrm.readuInt16();
    maFirst.mnCol = bCol16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
    maLast.mnCol = bCol16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
}

void BinRange::write( BiffOutputStream& rStrm, bool bCol16Bit, bool bRow32Bit ) const
{
    if( bRow32Bit )
        rStrm << maFirst.mnRow << maLast.mnRow;
    else
        rStrm << static_cast< sal_uInt16 >( maFirst.mnRow ) << static_cast< sal_uInt16 >( maLast.mnRow );
    if( bCol16Bit )
        rStrm << static_cast< sal_uInt16 >( maFirst.mnCol ) << static_cast< sal_uInt16 >( maLast.mnCol );
    else
        rStrm << static_cast< sal_uInt8 >( maFirst.mnCol ) << static_cast< sal_uInt8 >( maLast.mnCol );
}

// ============================================================================

BinRange BinRangeList::getEnclosingRange() const
{
    BinRange aRange;
    if( !empty() )
    {
        const_iterator aIt = begin(), aEnd = end();
        aRange = *aIt;
        for( ++aIt; aIt != aEnd; ++aIt )
        {
            aRange.maFirst.mnCol = ::std::min( aRange.maFirst.mnCol, aIt->maFirst.mnCol );
            aRange.maFirst.mnRow = ::std::min( aRange.maFirst.mnRow, aIt->maFirst.mnRow );
            aRange.maLast.mnCol  = ::std::max( aRange.maLast.mnCol, aIt->maLast.mnCol );
            aRange.maLast.mnRow  = ::std::max( aRange.maLast.mnRow, aIt->maLast.mnRow );
        }
    }
    return aRange;
}

void BinRangeList::read( RecordInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    resize( getLimitedValue< size_t, sal_Int64 >( nCount, 0, rStrm.getRemaining() / 16 ) );
    for( iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        aIt->read( rStrm );
}

void BinRangeList::read( BiffInputStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    sal_uInt16 nCount = rStrm.readuInt16();
    resize( getLimitedValue< size_t, sal_Int64 >( nCount, 0, rStrm.getRemaining() / lclGetBiffRangeSize( bCol16Bit, bRow32Bit ) ) );
    for( iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        aIt->read( rStrm, bCol16Bit, bRow32Bit );
}

void BinRangeList::write( BiffOutputStream& rStrm, bool bCol16Bit, bool bRow32Bit ) const
{
    writeSubList( rStrm, 0, size(), bCol16Bit, bRow32Bit );
}

void BinRangeList::writeSubList( BiffOutputStream& rStrm, size_t nBegin, size_t nCount, bool bCol16Bit, bool bRow32Bit ) const
{
    OSL_ENSURE( nBegin <= size(), "BiffRangeList::writeSubList - invalid start position" );
    size_t nEnd = ::std::min< size_t >( nBegin + nCount, size() );
    sal_uInt16 nBiffCount = getLimitedValue< sal_uInt16, size_t >( nEnd - nBegin, 0, SAL_MAX_UINT16 );
    rStrm << nBiffCount;
    rStrm.setPortionSize( lclGetBiffRangeSize( bCol16Bit, bRow32Bit ) );
    for( const_iterator aIt = begin() + nBegin, aEnd = begin() + nEnd; aIt != aEnd; ++aIt )
        aIt->write( rStrm, bCol16Bit, bRow32Bit );
}

// ============================================================================
// ============================================================================

AddressConverter::AddressConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbColOverflow( false ),
    mbRowOverflow( false ),
    mbTabOverflow( false )
{
    maDConChars.set( 0xFFFF, '\x01', 0xFFFF, '\x02', 0xFFFF );
    switch( getFilterType() )
    {
        case FILTER_OOX:
            initializeMaxPos( OOX_MAXTAB, OOX_MAXCOL, OOX_MAXROW );
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
            case BIFF_UNKNOWN: break;
        }
        break;
        case FILTER_UNKNOWN: break;
    }
}

// ----------------------------------------------------------------------------

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

namespace {

bool lclAppendUrlChar( OUStringBuffer& orUrl, sal_Unicode cChar, bool bEncodeSpecial )
{
    // #126855# encode special characters
    if( bEncodeSpecial ) switch( cChar )
    {
        case '#':   orUrl.appendAscii( "%23" );  return true;
        case '%':   orUrl.appendAscii( "%25" );  return true;
    }
    orUrl.append( cChar );
    return cChar >= ' ';
}

} // namespace

BiffTargetType AddressConverter::parseBiffTargetUrl(
        OUString& orClassName, OUString& orTargetUrl, OUString& orSheetName,
        const OUString& rBiffTargetUrl, bool bFromDConRec )
{
    OUStringBuffer aTargetUrl;
    OUStringBuffer aSheetName;
    // default target type: some URL with/without sheet name, may be overridden below
    BiffTargetType eTargetType = BIFF_TARGETTYPE_URL;
    const ControlCharacters& rCChars = bFromDConRec ? maDConChars : maLinkChars;

    enum
    {
        STATE_START,
        STATE_ENCODED_PATH_START,       /// Start of encoded file path.
        STATE_ENCODED_PATH,             /// Inside encoded file path.
        STATE_ENCODED_DRIVE,            /// DOS drive letter or start of UNC path.
        STATE_ENCODED_URL,              /// Encoded URL, e.g. http links.
        STATE_UNENCODED,                /// Unencoded URL, could be DDE or OLE.
        STATE_DDE_OLE,                  /// Second part of DDE or OLE link.
        STATE_FILENAME,                 /// File name enclosed in brackets.
        STATE_SHEETNAME,                /// Sheet name following enclosed file name.
        STATE_UNSUPPORTED,              /// Unsupported special paths.
        STATE_ERROR
    }
    eState = STATE_START;

    const sal_Unicode* pcChar = rBiffTargetUrl.getStr();
    const sal_Unicode* pcEnd = pcChar + rBiffTargetUrl.getLength();
    for( ; (eState != STATE_ERROR) && (pcChar < pcEnd); ++pcChar )
    {
        sal_Unicode cChar = *pcChar;
        switch( eState )
        {
            case STATE_START:
                if( (cChar == rCChars.mcThisWorkbook) || (cChar == rCChars.mcThisSheet) || (cChar == rCChars.mcSameSheet) )
                {
                    if( pcChar + 1 < pcEnd )
                        eState = STATE_ERROR;
                    if( cChar == rCChars.mcSameSheet )
                        eTargetType = BIFF_TARGETTYPE_SAMESHEET;
                }
                else if( cChar == rCChars.mcExternal )
                    eState = (pcChar + 1 < pcEnd) ? STATE_ENCODED_PATH_START : STATE_ERROR;
                else if( cChar == rCChars.mcInternal )
                    eState = (pcChar + 1 < pcEnd) ? STATE_SHEETNAME : STATE_ERROR;
                else
                    eState = lclAppendUrlChar( aTargetUrl, cChar, true ) ? STATE_UNENCODED : STATE_ERROR;
            break;

            case STATE_ENCODED_PATH_START:
                if( cChar == BIFF_URL_DRIVE )
                    eState = STATE_ENCODED_DRIVE;
                else if( cChar == BIFF_URL_ROOT )
                {
                    aTargetUrl.append( sal_Unicode( '/' ) );
                    eState = STATE_ENCODED_PATH;
                }
                else if( cChar == BIFF_URL_PARENT )
                    aTargetUrl.appendAscii( "../" );
                else if( cChar == BIFF_URL_RAW )
                    eState = STATE_ENCODED_URL;
                else if( cChar == BIFF_URL_INSTALL )
                    eState = STATE_UNSUPPORTED;
                else if( cChar == BIFF_URL_INSTALL2 )
                    eState = STATE_UNSUPPORTED;
                else if( cChar == BIFF_URL_LIBRARY )
                {
                    eState = STATE_ENCODED_PATH;
                    eTargetType = BIFF_TARGETTYPE_LIBRARY;
                }
                else if( (getBiff() == BIFF4) && (cChar == BIFF4_URL_SHEET) )
                    eState = STATE_SHEETNAME;
                else if( cChar == '[' )
                    eState = STATE_FILENAME;
                else if( lclAppendUrlChar( aTargetUrl, cChar, true ) )
                    eState = STATE_ENCODED_PATH;
                else
                    eState = STATE_ERROR;
            break;

            case STATE_ENCODED_PATH:
                if( cChar == BIFF_URL_SUBDIR )
                    aTargetUrl.append( sal_Unicode( '/' ) );
                else if( cChar == '[' )
                    eState = STATE_FILENAME;
                else if( !lclAppendUrlChar( aTargetUrl, cChar, true ) )
                    eState = STATE_ERROR;
            break;

            case STATE_ENCODED_DRIVE:
                if( cChar == BIFF_URL_UNC )
                {
                    aTargetUrl.appendAscii( "file://" );
                    eState = STATE_ENCODED_PATH;
                }
                else
                {
                    aTargetUrl.appendAscii( "file:///" );
                    eState = lclAppendUrlChar( aTargetUrl, cChar, false ) ? STATE_ENCODED_PATH : STATE_ERROR;
                    aTargetUrl.appendAscii( ":/" );
                }
            break;

            case STATE_ENCODED_URL:
            {
                sal_Int32 nLength = cChar;
                if( nLength + 1 == pcEnd - pcChar )
                    aTargetUrl.append( pcChar + 1, nLength );
                else
                    eState = STATE_ERROR;
            }
            break;

            case STATE_UNENCODED:
                if( cChar == BIFF_URL_SUBDIR )
                {
                    orClassName = aTargetUrl.makeStringAndClear();
                    eState = bFromDConRec ? STATE_ERROR : STATE_DDE_OLE;
                    eTargetType = BIFF_TARGETTYPE_DDE_OLE;
                }
                else if( cChar == '[' )
                    eState = STATE_FILENAME;
                else if( !lclAppendUrlChar( aTargetUrl, cChar, true ) )
                    eState = STATE_ERROR;
            break;

            case STATE_DDE_OLE:
                if( !lclAppendUrlChar( aTargetUrl, cChar, true ) )
                    eState = STATE_ERROR;
            break;

            case STATE_FILENAME:
                if( cChar == ']' )
                    eState = STATE_SHEETNAME;
                else if( !lclAppendUrlChar( aTargetUrl, cChar, true ) )
                    eState = STATE_ERROR;
            break;

            case STATE_SHEETNAME:
                if( !lclAppendUrlChar( aSheetName, cChar, false ) )
                    eState = STATE_ERROR;
            break;

            case STATE_UNSUPPORTED:
                pcChar = pcEnd - 1;
            break;

            case STATE_ERROR:
            break;
        }
    }

    OSL_ENSURE( (eState != STATE_ERROR) && (pcChar == pcEnd),
        OStringBuffer( "AddressConverter::parseBiffTargetUrl - parser error in target \"" ).
        append( OUStringToOString( rBiffTargetUrl, RTL_TEXTENCODING_UTF8 ) ).append( '"' ).getStr() );
    bool bParserOk = (eState != STATE_ERROR) && (eState != STATE_UNSUPPORTED) && (pcChar == pcEnd);

    if( bParserOk )
    {
        orTargetUrl = aTargetUrl.makeStringAndClear();
        orSheetName = aSheetName.makeStringAndClear();
    }
    else
    {
        orClassName = orTargetUrl = orSheetName = OUString();
    }

    return bParserOk ? eTargetType : BIFF_TARGETTYPE_UNKNOWN;
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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

bool AddressConverter::convertToCellAddress( CellAddress& orAddress,
        const OUString& rString, sal_Int16 nSheet, bool bTrackOverflow )
{
    return
        convertToCellAddressUnchecked( orAddress, rString, nSheet ) &&
        checkCellAddress( orAddress, bTrackOverflow );
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

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

bool AddressConverter::checkCellRangeList( const ApiCellRangeList& rRanges, bool bAllowOverflow, bool bTrackOverflow )
{
    for( ApiCellRangeList::const_iterator aIt = rRanges.begin(), aEnd = rRanges.end(); aIt != aEnd; ++aIt )
        if( !checkCellRange( *aIt, bAllowOverflow, bTrackOverflow ) )
            return false;
    return true;
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
        if( (aToken.getLength() > 0) && convertToCellRange( aRange, aToken, nSheet, true, bTrackOverflow ) )
            orRanges.push_back( aRange );
    }
}

void AddressConverter::convertToCellRangeList( ApiCellRangeList& orRanges,
        const BinRangeList& rBinRanges, sal_Int16 nSheet, bool bTrackOverflow )
{
    CellRangeAddress aRange;
    for( BinRangeList::const_iterator aIt = rBinRanges.begin(), aEnd = rBinRanges.end(); aIt != aEnd; ++aIt )
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
        OSL_ENSURE( false, "AddressConverter::AddressConverter - cannot get sheet limits" );
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

