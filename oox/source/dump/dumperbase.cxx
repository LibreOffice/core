/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dumperbase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:40:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/dump/dumperbase.hxx"

#include <algorithm>
#include <rtl/math.hxx>
#include <rtl/tencinfo.h>
#include <osl/file.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <comphelper/processfactory.hxx>
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffhelper.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::ucb::XSimpleFileAccess;
using ::com::sun::star::io::XActiveDataSink;
using ::com::sun::star::io::XActiveDataSource;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XTextInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XTextOutputStream;
using ::oox::core::FilterBase;

namespace oox {
namespace dump {

const sal_Unicode OOX_DUMP_BOM          = 0xFEFF;
const sal_Int32 OOX_DUMP_MAXSTRLEN      = 80;
const sal_Int32 OOX_DUMP_INDENT         = 2;
const sal_Unicode OOX_DUMP_BINDOT       = '.';
const sal_Unicode OOX_DUMP_CFG_LISTSEP  = ',';
const sal_Unicode OOX_DUMP_CFG_QUOTE    = '\'';
const sal_Unicode OOX_DUMP_LF           = '\n';
const sal_Unicode OOX_DUMP_ITEMSEP      = '=';
const sal_Int32 OOX_DUMP_BYTESPERLINE   = 16;
const sal_Int32 OOX_DUMP_MAXARRAY       = 16;

// ============================================================================
// ============================================================================

// file names -----------------------------------------------------------------

OUString InputOutputHelper::convertFileNameToUrl( const OUString& rFileName )
{
    OUString aFileUrl;
    if( ::osl::FileBase::getFileURLFromSystemPath( rFileName, aFileUrl ) == ::osl::FileBase::E_None )
        return aFileUrl;
    return OUString();
}

sal_Int32 InputOutputHelper::getFileNamePos( const OUString& rFileUrl )
{
    sal_Int32 nSepPos = rFileUrl.lastIndexOf( '/' );
    return (nSepPos < 0) ? 0 : (nSepPos + 1);
}

OUString InputOutputHelper::getFileNameExtension( const OUString& rFileUrl )
{
    sal_Int32 nNamePos = getFileNamePos( rFileUrl );
    sal_Int32 nExtPos = rFileUrl.lastIndexOf( '.' );
    if( nExtPos >= nNamePos )
        return rFileUrl.copy( nExtPos + 1 );
    return OUString();
}

Reference< XInputStream > InputOutputHelper::openInputStream( const OUString& rFileName )
{
    Reference< XInputStream > xInStrm;
    try
    {
        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        Reference< XSimpleFileAccess > xFileAccess( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY_THROW );
        xInStrm = xFileAccess->openFileRead( rFileName );
    }
    catch( Exception& )
    {
    }
    return xInStrm;
}

Reference< XTextInputStream > InputOutputHelper::openTextInputStream( const Reference< XInputStream >& rxInStrm, const OUString& rEncoding )
{
    Reference< XTextInputStream > xTextInStrm;
    if( rxInStrm.is() ) try
    {
        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        Reference< XActiveDataSink > xDataSink( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TextInputStream" ) ), UNO_QUERY_THROW );
        xDataSink->setInputStream( rxInStrm );
        xTextInStrm.set( xDataSink, UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    if( xTextInStrm.is() )
        xTextInStrm->setEncoding( rEncoding );
    return xTextInStrm;
}

Reference< XTextInputStream > InputOutputHelper::openTextInputStream( const OUString& rFileName, const OUString& rEncoding )
{
    return openTextInputStream( openInputStream( rFileName ), rEncoding );
}

Reference< XOutputStream > InputOutputHelper::openOutputStream( const OUString& rFileName )
{
    Reference< XOutputStream > xOutStrm;
    try
    {
        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        Reference< XSimpleFileAccess > xFileAccess( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY_THROW );
        if( !xFileAccess->isFolder( rFileName ) )
        {
            try { xFileAccess->kill( rFileName ); } catch( Exception& ) {}
            xOutStrm = xFileAccess->openFileWrite( rFileName );
        }
    }
    catch( Exception& )
    {
    }
    return xOutStrm;
}

Reference< XTextOutputStream > InputOutputHelper::openTextOutputStream( const Reference< XOutputStream >& rxOutStrm, const OUString& rEncoding )
{
    Reference< XTextOutputStream > xTextOutStrm;
    if( rxOutStrm.is() ) try
    {
        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        Reference< XActiveDataSource > xDataSource( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TextOutputStream" ) ), UNO_QUERY_THROW );
        xDataSource->setOutputStream( rxOutStrm );
        xTextOutStrm.set( xDataSource, UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    if( xTextOutStrm.is() )
        xTextOutStrm->setEncoding( rEncoding );
    return xTextOutStrm;
}

Reference< XTextOutputStream > InputOutputHelper::openTextOutputStream( const OUString& rFileName, const OUString& rEncoding )
{
    return openTextOutputStream( openOutputStream( rFileName ), rEncoding );
}

// ============================================================================
// ============================================================================

ItemFormat::ItemFormat() :
    meDataType( DATATYPE_VOID ),
    meFmtType( FORMATTYPE_NONE )
{
}

void ItemFormat::set( DataType eDataType, FormatType eFmtType, const OUString& rItemName )
{
    meDataType = eDataType;
    meFmtType = eFmtType;
    maItemName = rItemName;
    maListName = OUString();
}

void ItemFormat::set( DataType eDataType, FormatType eFmtType, const OUString& rItemName, const OUString& rListName )
{
    set( eDataType, eFmtType, rItemName );
    maListName = rListName;
}

OUStringVector::const_iterator ItemFormat::parse( const OUStringVector& rFormatVec )
{
    set( DATATYPE_VOID, FORMATTYPE_NONE, OUString() );

    OUStringVector::const_iterator aIt = rFormatVec.begin(), aEnd = rFormatVec.end();
    OUString aDataType, aFmtType;
    if( aIt != aEnd ) aDataType = *aIt++;
    if( aIt != aEnd ) aFmtType = *aIt++;
    if( aIt != aEnd ) maItemName = *aIt++;
    if( aIt != aEnd ) maListName = *aIt++;

    meDataType = StringHelper::convertToDataType( aDataType );
    meFmtType = StringHelper::convertToFormatType( aFmtType );

    if( meFmtType == FORMATTYPE_NONE )
    {
        if( aFmtType.equalsAscii( "unused" ) )
            set( meDataType, FORMATTYPE_HEX, CREATE_OUSTRING( OOX_DUMP_UNUSED ) );
        else if( aFmtType.equalsAscii( "unknown" ) )
            set( meDataType, FORMATTYPE_HEX, CREATE_OUSTRING( OOX_DUMP_UNKNOWN ) );
    }

    return aIt;
}

OUStringVector ItemFormat::parse( const OUString& rFormatStr )
{
    OUStringVector aFormatVec;
    StringHelper::convertStringToStringList( aFormatVec, rFormatStr, false );
    OUStringVector::const_iterator aIt = parse( aFormatVec );
    return OUStringVector( aIt, const_cast< const OUStringVector& >( aFormatVec ).end() );
}

// ============================================================================
// ============================================================================

// append string to string ----------------------------------------------------

void StringHelper::appendChar( ::rtl::OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount )
{
    for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
        rStr.append( cChar );
}

void StringHelper::appendString( OUStringBuffer& rStr, const OUString& rData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendChar( rStr, cFill, nWidth - rData.getLength() );
    rStr.append( rData );
}

// append decimal -------------------------------------------------------------

void StringHelper::appendDec( OUStringBuffer& rStr, sal_uInt8 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( static_cast< sal_Int32 >( nData ) ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_Int8 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( static_cast< sal_Int32 >( nData ) ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_uInt16 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( static_cast< sal_Int32 >( nData ) ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_Int16 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( static_cast< sal_Int32 >( nData ) ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_uInt32 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( static_cast< sal_Int64 >( nData ) ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_Int32 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( nData ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_uInt64 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    /*  Values greater than biggest signed 64bit integer will change to
        negative when converting to sal_Int64. Therefore, the trailing digit
        will be written separately. */
    OUStringBuffer aBuffer;
    if( nData > 9 )
        aBuffer.append( OUString::valueOf( static_cast< sal_Int64 >( nData / 10 ) ) );
    aBuffer.append( static_cast< sal_Unicode >( '0' + (nData % 10) ) );
    appendString( rStr, aBuffer.makeStringAndClear(), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, sal_Int64 nData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, OUString::valueOf( nData ), nWidth, cFill );
}

void StringHelper::appendDec( OUStringBuffer& rStr, double fData, sal_Int32 nWidth, sal_Unicode cFill )
{
    appendString( rStr, ::rtl::math::doubleToUString( fData, rtl_math_StringFormat_G, 15, '.', true ), nWidth, cFill );
}

// append hexadecimal ---------------------------------------------------------

void StringHelper::appendHex( OUStringBuffer& rStr, sal_uInt8 nData, bool bPrefix )
{
    static const sal_Unicode spcHexDigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
    if( bPrefix )
        rStr.appendAscii( "0x" );
    rStr.append( spcHexDigits[ (nData >> 4) & 0x0F ] ).append( spcHexDigits[ nData & 0x0F ] );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_Int8 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt8 >( nData >> 8 ), bPrefix );
    appendHex( rStr, static_cast< sal_uInt8 >( nData ), false );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_Int16 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt16 >( nData >> 16 ), bPrefix );
    appendHex( rStr, static_cast< sal_uInt16 >( nData ), false );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_Int32 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_uInt64 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt32 >( nData >> 32 ), bPrefix );
    appendHex( rStr, static_cast< sal_uInt32 >( nData ), false );
}

void StringHelper::appendHex( OUStringBuffer& rStr, sal_Int64 nData, bool bPrefix )
{
    appendHex( rStr, static_cast< sal_uInt64 >( nData ), bPrefix );
}

void StringHelper::appendHex( OUStringBuffer& rStr, double fData, bool bPrefix )
{
    appendHex( rStr, *reinterpret_cast< const sal_uInt64* >( &fData ), bPrefix );
}

// append shortened hexadecimal -----------------------------------------------

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt8 nData, bool bPrefix )
{
    if( nData != 0 )
        appendHex( rStr, nData, bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int8 nData, bool bPrefix )
{
    appendShortHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix )
{
    if( nData > 0xFF )
        appendHex( rStr, nData, bPrefix );
    else
        appendShortHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int16 nData, bool bPrefix )
{
    appendShortHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix )
{
    if( nData > 0xFFFF )
        appendHex( rStr, nData, bPrefix );
    else
        appendShortHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int32 nData, bool bPrefix )
{
    appendShortHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt64 nData, bool bPrefix )
{
    if( nData > 0xFFFFFFFF )
        appendHex( rStr, nData, bPrefix );
    else
        appendShortHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int64 nData, bool bPrefix )
{
    appendShortHex( rStr, static_cast< sal_uInt64 >( nData ), bPrefix );
}

// append binary --------------------------------------------------------------

void StringHelper::appendBin( OUStringBuffer& rStr, sal_uInt8 nData, bool bDots )
{
    for( sal_uInt8 nMask = 0x80; nMask != 0; (nMask >>= 1) &= 0x7F )
    {
        rStr.append( static_cast< sal_Unicode >( (nData & nMask) ? '1' : '0' ) );
        if( bDots && (nMask == 0x10) )
            rStr.append( OOX_DUMP_BINDOT );
    }
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_Int8 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt8 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_uInt16 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt8 >( nData >> 8 ), bDots );
    if( bDots )
        rStr.append( OOX_DUMP_BINDOT );
    appendBin( rStr, static_cast< sal_uInt8 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_Int16 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt16 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_uInt32 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt16 >( nData >> 16 ), bDots );
    if( bDots )
        rStr.append( OOX_DUMP_BINDOT );
    appendBin( rStr, static_cast< sal_uInt16 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_Int32 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt32 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_uInt64 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt32 >( nData >> 32 ), bDots );
    if( bDots )
        rStr.append( OOX_DUMP_BINDOT );
    appendBin( rStr, static_cast< sal_uInt32 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, sal_Int64 nData, bool bDots )
{
    appendBin( rStr, static_cast< sal_uInt64 >( nData ), bDots );
}

void StringHelper::appendBin( OUStringBuffer& rStr, double fData, bool bDots )
{
    appendBin( rStr, *reinterpret_cast< const sal_uInt64* >( &fData ), bDots );
}

// append formatted value -----------------------------------------------------

void StringHelper::appendBool( OUStringBuffer& rStr, bool bData )
{
    rStr.appendAscii( bData ? "true" : "false" );
}

// append columns, rows, addresses --------------------------------------------

void StringHelper::appendAddrCol( OUStringBuffer& rStr, sal_Int32 nCol, bool bRel )
{
    if( !bRel ) rStr.append( OOX_DUMP_ADDRABS );
    sal_Int32 nPos = rStr.getLength();
    for( sal_Int32 nTemp = nCol; nTemp >= 0; (nTemp /= 26) -= 1 )
        rStr.insert( nPos, static_cast< sal_Unicode >( 'A' + (nTemp % 26) ) );
}

void StringHelper::appendAddrRow( OUStringBuffer& rStr, sal_Int32 nRow, bool bRel )
{
    if( !bRel ) rStr.append( OOX_DUMP_ADDRABS );
    appendDec( rStr, nRow + 1 );
}

void StringHelper::appendAddrName( OUStringBuffer& rStr, sal_Unicode cPrefix, sal_Int32 nColRow, bool bRel )
{
    rStr.append( cPrefix );
    if( bRel && (nColRow != 0) )
    {
        rStr.append( OOX_DUMP_R1C1OPEN );
        appendDec( rStr, nColRow );
        rStr.append( OOX_DUMP_R1C1CLOSE );
    }
    else if( !bRel )
        appendDec( rStr, nColRow + 1 );
}

void StringHelper::appendAddress( OUStringBuffer& rStr, const Address& rPos )
{
    appendAddrCol( rStr, rPos.mnCol, true );
    appendAddrRow( rStr, rPos.mnRow, true );
}

void StringHelper::appendRange( OUStringBuffer& rStr, const Range& rRange )
{
    appendAddress( rStr, rRange.maFirst );
    rStr.append( OOX_DUMP_RANGESEP );
    appendAddress( rStr, rRange.maLast );
}

void StringHelper::appendRangeList( OUStringBuffer& rStr, const RangeList& rRanges )
{
    OUStringBuffer aData;
    for( RangeList::const_iterator aIt = rRanges.begin(), aEnd = rRanges.end(); aIt != aEnd; ++aIt )
    {
        OUStringBuffer aRange;
        appendRange( aRange, *aIt );
        appendToken( aData, aRange.makeStringAndClear(), OOX_DUMP_LISTSEP );
    }
    rStr.append( aData.makeStringAndClear() );
}

void StringHelper::appendAddress( OUStringBuffer& rStr, const TokenAddress& rPos, bool bR1C1 )
{
    if( bR1C1 && (rPos.mbRelCol || rPos.mbRelRow) )
    {
        appendAddrName( rStr, OOX_DUMP_R1C1ROW, rPos.mnRow, rPos.mbRelRow );
        appendAddrName( rStr, OOX_DUMP_R1C1COL, rPos.mnCol, rPos.mbRelCol );
    }
    else
    {
        appendAddrCol( rStr, rPos.mnCol, rPos.mbRelCol );
        appendAddrRow( rStr, rPos.mnRow, rPos.mbRelRow );
    }
}

void StringHelper::appendRange( OUStringBuffer& rStr, const TokenRange& rRange, bool bR1C1 )
{
    appendAddress( rStr, rRange.maFirst, bR1C1 );
    rStr.append( OOX_DUMP_RANGESEP );
    appendAddress( rStr, rRange.maLast, bR1C1 );
}

// encoded text output --------------------------------------------------------

void StringHelper::appendCChar( OUStringBuffer& rStr, sal_Unicode cChar, bool bPrefix )
{
    if( cChar > 0x00FF )
    {
        if( bPrefix )
            rStr.appendAscii( "\\u" );
        appendHex( rStr, static_cast< sal_uInt16 >( cChar ), false );
    }
    else
    {
        if( bPrefix )
            rStr.appendAscii( "\\x" );
        appendHex( rStr, static_cast< sal_uInt8 >( cChar ), false );
    }
}

void StringHelper::appendEncChar( OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount, bool bPrefix )
{
    if( cChar < 0x0020 )
    {
        // C-style hex code
        OUStringBuffer aCode;
        appendCChar( aCode, cChar, bPrefix );
        for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
            rStr.append( aCode );
    }
    else
    {
        appendChar( rStr, cChar, nCount );
    }
}

void StringHelper::appendEncString( OUStringBuffer& rStr, const OUString& rData, bool bPrefix )
{
    sal_Int32 nBeg = 0;
    sal_Int32 nIdx = 0;
    sal_Int32 nEnd = rData.getLength();
    while( nIdx < nEnd )
    {
        // find next character that needs encoding
        while( (nIdx < nEnd) && (rData[ nIdx ] >= 0x20) ) ++nIdx;
        // append portion
        if( nBeg < nIdx )
        {
            if( (nBeg == 0) && (nIdx == nEnd) )
                rStr.append( rData );
            else
                rStr.append( rData.copy( nBeg, nIdx - nBeg ) );
        }
        // append characters to be encoded
        while( (nIdx < nEnd) && (rData[ nIdx ] < 0x20) )
        {
            appendCChar( rStr, rData[ nIdx ], bPrefix );
            ++nIdx;
        }
        // adjust limits
        nBeg = nIdx;
    }
}

// token list -----------------------------------------------------------------

void StringHelper::appendToken( OUStringBuffer& rStr, const OUString& rToken, sal_Unicode cSep )
{
    if( (rStr.getLength() > 0) && (rToken.getLength() > 0) )
        rStr.append( cSep );
    rStr.append( rToken );
}

void StringHelper::appendToken( OUStringBuffer& rStr, sal_Int64 nToken, sal_Unicode cSep )
{
    OUStringBuffer aToken;
    appendDec( aToken, nToken );
    appendToken( rStr, aToken.makeStringAndClear(), cSep );
}

void StringHelper::prependToken( OUStringBuffer& rStr, const OUString& rToken, sal_Unicode cSep )
{
    if( (rStr.getLength() > 0) && (rToken.getLength() > 0) )
        rStr.insert( 0, cSep );
    rStr.insert( 0, rToken );
}

void StringHelper::prependToken( OUStringBuffer& rStr, sal_Int64 nToken, sal_Unicode cSep )
{
    OUStringBuffer aToken;
    appendDec( aToken, nToken );
    prependToken( rStr, aToken.makeStringAndClear(), cSep );
}

void StringHelper::appendIndex( OUStringBuffer& rStr, const OUString& rIdx )
{
    rStr.append( sal_Unicode( '[' ) ).append( rIdx ).append( sal_Unicode( ']' ) );
}

void StringHelper::appendIndex( OUStringBuffer& rStr, sal_Int64 nIdx )
{
    OUStringBuffer aToken;
    appendDec( aToken, nIdx );
    appendIndex( rStr, aToken.makeStringAndClear() );
}

void StringHelper::appendIndexedText( OUStringBuffer& rStr, const OUString& rData, const OUString& rIdx )
{
    rStr.append( rData );
    appendIndex( rStr, rIdx );
}

void StringHelper::appendIndexedText( OUStringBuffer& rStr, const OUString& rData, sal_Int64 nIdx )
{
    rStr.append( rData );
    appendIndex( rStr, nIdx );
}

OUString StringHelper::getToken( const OUString& rData, sal_Int32& rnPos, sal_Unicode cSep )
{
    return trimSpaces( rData.getToken( 0, cSep, rnPos ) );
}

void StringHelper::enclose( OUStringBuffer& rStr, sal_Unicode cOpen, sal_Unicode cClose )
{
    rStr.insert( 0, cOpen ).append( cClose ? cClose : cOpen );
}

// string conversion ----------------------------------------------------------

namespace {

sal_Int32 lclIndexOf( const OUString& rStr, sal_Unicode cChar, sal_Int32 nStartPos )
{
    sal_Int32 nIndex = rStr.indexOf( cChar, nStartPos );
    return (nIndex < 0) ? rStr.getLength() : nIndex;
}

OUString lclTrimQuotedStringList( const OUString& rStr )
{
    OUStringBuffer aBuffer;
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rStr.getLength();
    while( nPos < nLen )
    {
        if( rStr[ nPos ] == OOX_DUMP_CFG_QUOTE )
        {
            // quoted string, skip leading quote character
            ++nPos;
            // process quoted text and ambedded literal quote characters
            OUStringBuffer aToken;
            do
            {
                // seek to next quote character and add text portion to token buffer
                sal_Int32 nEnd = lclIndexOf( rStr, OOX_DUMP_CFG_QUOTE, nPos );
                aToken.append( rStr.copy( nPos, nEnd - nPos ) );
                // process literal quotes
                while( (nEnd + 1 < nLen) && (rStr[ nEnd ] == OOX_DUMP_CFG_QUOTE) && (rStr[ nEnd + 1 ] == OOX_DUMP_CFG_QUOTE) )
                {
                    aToken.append( OOX_DUMP_CFG_QUOTE );
                    nEnd += 2;
                }
                // nEnd is start of possible next text portion
                nPos = nEnd;
            }
            while( (nPos < nLen) && (rStr[ nPos ] != OOX_DUMP_CFG_QUOTE) );
            // add token, seek to list separator, ignore text following closing quote
            aBuffer.append( aToken.makeStringAndClear() );
            nPos = lclIndexOf( rStr, OOX_DUMP_CFG_LISTSEP, nPos );
            if( nPos < nLen )
                aBuffer.append( OOX_DUMP_LF );
            // set current position behind list separator
            ++nPos;
        }
        else
        {
            // find list separator, add token text to buffer
            sal_Int32 nEnd = lclIndexOf( rStr, OOX_DUMP_CFG_LISTSEP, nPos );
            aBuffer.append( rStr.copy( nPos, nEnd - nPos ) );
            if( nEnd < nLen )
                aBuffer.append( OOX_DUMP_LF );
            // set current position behind list separator
            nPos = nEnd + 1;
        }
    }

    return aBuffer.makeStringAndClear();
}

} // namespace

OUString StringHelper::trimSpaces( const OUString& rStr )
{
    sal_Int32 nBeg = 0;
    while( (nBeg < rStr.getLength()) && ((rStr[ nBeg ] == ' ') || (rStr[ nBeg ] == '\t')) )
        ++nBeg;
    sal_Int32 nEnd = rStr.getLength();
    while( (nEnd > nBeg) && ((rStr[ nEnd - 1 ] == ' ') || (rStr[ nEnd - 1 ] == '\t')) )
        --nEnd;
    return rStr.copy( nBeg, nEnd - nBeg );
}

OString StringHelper::convertToUtf8( const OUString& rStr )
{
    return ::rtl::OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 );
}

DataType StringHelper::convertToDataType( const OUString& rStr )
{
    DataType eType = DATATYPE_VOID;
    if( rStr.equalsAscii( "int8" ) )
        eType = DATATYPE_INT8;
    else if( rStr.equalsAscii( "uint8" ) )
        eType = DATATYPE_UINT8;
    else if( rStr.equalsAscii( "int16" ) )
        eType = DATATYPE_INT16;
    else if( rStr.equalsAscii( "uint16" ) )
        eType = DATATYPE_UINT16;
    else if( rStr.equalsAscii( "int32" ) )
        eType = DATATYPE_INT32;
    else if( rStr.equalsAscii( "uint32" ) )
        eType = DATATYPE_UINT32;
    else if( rStr.equalsAscii( "int64" ) )
        eType = DATATYPE_INT64;
    else if( rStr.equalsAscii( "uint64" ) )
        eType = DATATYPE_UINT64;
    else if( rStr.equalsAscii( "float" ) )
        eType = DATATYPE_FLOAT;
    else if( rStr.equalsAscii( "double" ) )
        eType = DATATYPE_DOUBLE;
    return eType;
}

FormatType StringHelper::convertToFormatType( const OUString& rStr )
{
    FormatType eType = FORMATTYPE_NONE;
    if( rStr.equalsAscii( "dec" ) )
        eType = FORMATTYPE_DEC;
    else if( rStr.equalsAscii( "hex" ) )
        eType = FORMATTYPE_HEX;
    else if( rStr.equalsAscii( "bin" ) )
        eType = FORMATTYPE_BIN;
    else if( rStr.equalsAscii( "fix" ) )
        eType = FORMATTYPE_FIX;
    else if( rStr.equalsAscii( "bool" ) )
        eType = FORMATTYPE_BOOL;
    return eType;
}

bool StringHelper::convertFromDec( sal_Int64& ornData, const OUString& rData )
{
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rData.getLength();
    bool bNeg = false;
    if( (nLen > 0) && (rData[ 0 ] == '-') )
    {
        bNeg = true;
        ++nPos;
    }
    ornData = 0;
    for( ; nPos < nLen; ++nPos )
    {
        sal_Unicode cChar = rData[ nPos ];
        if( (cChar < '0') || (cChar > '9') )
            return false;
        (ornData *= 10) += (cChar - '0');
    }
    if( bNeg )
        ornData *= -1;
    return true;
}

bool StringHelper::convertFromHex( sal_Int64& ornData, const OUString& rData )
{
    ornData = 0;
    for( sal_Int32 nPos = 0, nLen = rData.getLength(); nPos < nLen; ++nPos )
    {
        sal_Unicode cChar = rData[ nPos ];
        if( ('0' <= cChar) && (cChar <= '9') )
            cChar -= '0';
        else if( ('A' <= cChar) && (cChar <= 'F') )
            cChar -= ('A' - 10);
        else if( ('a' <= cChar) && (cChar <= 'f') )
            cChar -= ('a' - 10);
        else
            return false;
        (ornData <<= 4) += cChar;
    }
    return true;
}

bool StringHelper::convertStringToInt( sal_Int64& ornData, const OUString& rData )
{
    if( (rData.getLength() > 2) && (rData[ 0 ] == '0') && ((rData[ 1 ] == 'X') || (rData[ 1 ] == 'x')) )
        return convertFromHex( ornData, rData.copy( 2 ) );
    return convertFromDec( ornData, rData );
}

bool StringHelper::convertStringToDouble( double& orfData, const OUString& rData )
{
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nSize = 0;
    orfData = rtl::math::stringToDouble( rData, '.', '\0', &eStatus, &nSize );
    return (eStatus == rtl_math_ConversionStatus_Ok) && (nSize == rData.getLength());
}

bool StringHelper::convertStringToBool( const OUString& rData )
{
    if( rData.equalsAscii( "true" ) )
        return true;
    if( rData.equalsAscii( "false" ) )
        return false;
    sal_Int64 nData;
    return convertStringToInt( nData, rData ) && (nData != 0);
}

void StringHelper::convertStringToStringList( OUStringVector& orVec, const OUString& rData, bool bIgnoreEmpty )
{
    orVec.clear();
    OUString aUnquotedData = lclTrimQuotedStringList( rData );
    sal_Int32 nPos = 0;
    sal_Int32 nLen = aUnquotedData.getLength();
    while( (0 <= nPos) && (nPos < nLen) )
    {
        OUString aToken = getToken( aUnquotedData, nPos, OOX_DUMP_LF );
        if( !bIgnoreEmpty || (aToken.getLength() > 0) )
            orVec.push_back( aToken );
    }
}

void StringHelper::convertStringToIntList( Int64Vector& orVec, const OUString& rData, bool bIgnoreEmpty )
{
    orVec.clear();
    OUString aUnquotedData = lclTrimQuotedStringList( rData );
    sal_Int32 nPos = 0;
    sal_Int32 nLen = aUnquotedData.getLength();
    sal_Int64 nData;
    while( (0 <= nPos) && (nPos < nLen) )
    {
        bool bOk = convertStringToInt( nData, getToken( aUnquotedData, nPos, OOX_DUMP_LF ) );
        if( !bIgnoreEmpty || bOk )
            orVec.push_back( bOk ? nData : 0 );
    }
}

// ============================================================================
// ============================================================================

FormulaStack::FormulaStack() :
    mbError( false )
{
}

void FormulaStack::pushOperand( const StringWrapper& rOp, const OUString& rTokClass )
{
    maFmlaStack.push( rOp.getString() );
    maClassStack.push( rTokClass );
}

void FormulaStack::pushOperand( const StringWrapper& rOp )
{
    pushOperand( rOp, OUString( OOX_DUMP_BASECLASS ) );
}

void FormulaStack::pushUnaryOp( const StringWrapper& rLOp, const StringWrapper& rROp )
{
    pushUnaryOp( maFmlaStack, rLOp.getString(), rROp.getString() );
    pushUnaryOp( maClassStack, rLOp.getString(), rROp.getString() );
}

void FormulaStack::pushBinaryOp( const StringWrapper& rOp )
{
    pushBinaryOp( maFmlaStack, rOp.getString() );
    pushBinaryOp( maClassStack, rOp.getString() );
}

void FormulaStack::pushFuncOp( const StringWrapper& rFunc, const OUString& rTokClass, sal_uInt8 nParamCount )
{
    pushFuncOp( maFmlaStack, rFunc.getString(), nParamCount );
    pushFuncOp( maClassStack, rTokClass, nParamCount );
}

void FormulaStack::replaceOnTop( const OUString& rOld, const OUString& rNew )
{
    if( !maFmlaStack.empty() )
    {
        sal_Int32 nPos = maFmlaStack.top().indexOf( rOld );
        if( nPos >= 0 )
            maFmlaStack.top() = maFmlaStack.top().copy( 0, nPos ) + rNew + maFmlaStack.top().copy( nPos + rOld.getLength() );
    }
}

const OUString& FormulaStack::getString( const StringStack& rStack ) const
{
    static const OUString saStackError = OOX_DUMP_ERRSTRING( "stack" );
    return (mbError || rStack.empty()) ? saStackError : rStack.top();
}

void FormulaStack::pushUnaryOp( StringStack& rStack, const OUString& rLOp, const OUString& rROp )
{
    if( check( !rStack.empty() ) )
        rStack.top() = rLOp + rStack.top() + rROp;
}

void FormulaStack::pushBinaryOp( StringStack& rStack, const OUString& rOp )
{
    OUString aSecond;
    if( check( !rStack.empty() ) )
    {
        aSecond = rStack.top();
        rStack.pop();
    }
    if( check( !rStack.empty() ) )
        rStack.top() = rStack.top() + rOp + aSecond;
}

void FormulaStack::pushFuncOp( StringStack& rStack, const OUString& rOp, sal_uInt8 nParamCount )
{
    OUStringBuffer aFunc;
    for( sal_uInt8 nParam = 0; (nParam < nParamCount) && check( !rStack.empty() ); ++nParam )
    {
        StringHelper::prependToken( aFunc, rStack.top(), OOX_DUMP_FUNCSEP );
        rStack.pop();
    }
    StringHelper::enclose( aFunc, '(', ')' );
    aFunc.insert( 0, rOp );
    rStack.push( aFunc.makeStringAndClear() );
}

// ============================================================================
// ============================================================================

Base::~Base()
{
}

// ============================================================================
// ============================================================================

ConfigItemBase::~ConfigItemBase()
{
}

void ConfigItemBase::readConfigBlock( const ConfigInputStreamRef& rxStrm )
{
    readConfigBlockContents( rxStrm );
}

void ConfigItemBase::implProcessConfigItemStr(
        const ConfigInputStreamRef& /*rxStrm*/, const OUString& /*rKey*/, const OUString& /*rData*/ )
{
}

void ConfigItemBase::implProcessConfigItemInt(
        const ConfigInputStreamRef& /*rxStrm*/, sal_Int64 /*nKey*/, const OUString& /*rData*/ )
{
}

void ConfigItemBase::readConfigBlockContents( const ConfigInputStreamRef& rxStrm )
{
    bool bLoop = true;
    while( bLoop && !rxStrm->isEOF() )
    {
        OUString aKey, aData;
        switch( readConfigLine( rxStrm, aKey, aData ) )
        {
            case LINETYPE_DATA:
                processConfigItem( rxStrm, aKey, aData );
            break;
            case LINETYPE_END:
                bLoop = false;
            break;
        }
    }
}

ConfigItemBase::LineType ConfigItemBase::readConfigLine(
        const ConfigInputStreamRef& rxStrm, OUString& orKey, OUString& orData ) const
{
    OUString aLine;
    while( !rxStrm->isEOF() && (aLine.getLength() == 0) )
    {
        try { aLine = rxStrm->readLine(); } catch( Exception& ) { aLine = OUString(); }
        if( (aLine.getLength() > 0) && (aLine[ 0 ] == OOX_DUMP_BOM) )
            aLine = aLine.copy( 1 );
        aLine = StringHelper::trimSpaces( aLine );
        if( aLine.getLength() > 0 )
        {
            // ignore comments (starting with hash or semicolon)
            sal_Unicode cChar = aLine[ 0 ];
            if( (cChar == '#') || (cChar == ';') )
                aLine = OUString();
        }
    }

    LineType eResult = LINETYPE_END;
    if( aLine.getLength() > 0 )
    {
        sal_Int32 nEqPos = aLine.indexOf( '=' );
        if( nEqPos < 0 )
        {
            orKey = aLine;
        }
        else
        {
            orKey = StringHelper::trimSpaces( aLine.copy( 0, nEqPos ) );
            orData = StringHelper::trimSpaces( aLine.copy( nEqPos + 1 ) );
        }

        if( (orKey.getLength() > 0) && ((orData.getLength() > 0) || !orKey.equalsAscii( "end" )) )
            eResult = LINETYPE_DATA;
    }

    return eResult;
}

ConfigItemBase::LineType ConfigItemBase::readConfigLine( const ConfigInputStreamRef& rxStrm ) const
{
    OUString aKey, aData;
    return readConfigLine( rxStrm, aKey, aData );
}

void ConfigItemBase::processConfigItem(
        const ConfigInputStreamRef& rxStrm, const OUString& rKey, const OUString& rData )
{
    sal_Int64 nKey;
    if( StringHelper::convertStringToInt( nKey, rKey ) )
        implProcessConfigItemInt( rxStrm, nKey, rData );
    else
        implProcessConfigItemStr( rxStrm, rKey, rData );
}

// ============================================================================

NameListBase::~NameListBase()
{
}

void NameListBase::setName( sal_Int64 nKey, const StringWrapper& rNameWrp )
{
    implSetName( nKey, rNameWrp.getString() );
}

void NameListBase::includeList( NameListRef xList )
{
    if( xList.get() )
    {
        for( const_iterator aIt = xList->begin(), aEnd = xList->end(); aIt != aEnd; ++aIt )
            maMap[ aIt->first ] = aIt->second;
        implIncludeList( *xList );
    }
}

bool NameListBase::implIsValid() const
{
    return true;
}

void NameListBase::implProcessConfigItemStr(
        const ConfigInputStreamRef& rxStrm, const OUString& rKey, const OUString& rData )
{
    if( rKey.equalsAscii( "include" ) )
        include( rData );
    else if( rKey.equalsAscii( "exclude" ) )
        exclude( rData );
    else
        ConfigItemBase::implProcessConfigItemStr( rxStrm, rKey, rData );
}

void NameListBase::implProcessConfigItemInt(
        const ConfigInputStreamRef& /*rxStrm*/, sal_Int64 nKey, const OUString& rData )
{
    implSetName( nKey, rData );
}

void NameListBase::insertRawName( sal_Int64 nKey, const OUString& rName )
{
    maMap[ nKey ] = rName;
}

const OUString* NameListBase::findRawName( sal_Int64 nKey ) const
{
    const_iterator aIt = maMap.find( nKey );
    return (aIt == end()) ? 0 : &aIt->second;
}

void NameListBase::include( const OUString& rListKeys )
{
    OUStringVector aVec;
    StringHelper::convertStringToStringList( aVec, rListKeys, true );
    for( OUStringVector::const_iterator aIt = aVec.begin(), aEnd = aVec.end(); aIt != aEnd; ++aIt )
        includeList( mrCfgData.getNameList( *aIt ) );
}

void NameListBase::exclude( const OUString& rKeys )
{
    Int64Vector aVec;
    StringHelper::convertStringToIntList( aVec, rKeys, true );
    for( Int64Vector::const_iterator aIt = aVec.begin(), aEnd = aVec.end(); aIt != aEnd; ++aIt )
        maMap.erase( *aIt );
}

// ============================================================================

ConstList::ConstList( const SharedConfigData& rCfgData ) :
    NameListBase( rCfgData ),
    maDefName( OOX_DUMP_ERR_NONAME ),
    mbQuoteNames( false )
{
}

void ConstList::implProcessConfigItemStr(
        const ConfigInputStreamRef& rxStrm, const OUString& rKey, const OUString& rData )
{
    if( rKey.equalsAscii( "default" ) )
        setDefaultName( rData );
    else if( rKey.equalsAscii( "quote-names" ) )
        setQuoteNames( StringHelper::convertStringToBool( rData ) );
    else
        NameListBase::implProcessConfigItemStr( rxStrm, rKey, rData );
}

void ConstList::implSetName( sal_Int64 nKey, const OUString& rName )
{
    insertRawName( nKey, rName );
}

OUString ConstList::implGetName( const Config& /*rCfg*/, sal_Int64 nKey ) const
{
    const OUString* pName = findRawName( nKey );
    OUString aName = pName ? *pName : maDefName;
    if( mbQuoteNames )
    {
        OUStringBuffer aBuffer( aName );
        StringHelper::enclose( aBuffer, OOX_DUMP_STRQUOTE );
        aName = aBuffer.makeStringAndClear();
    }
    return aName;
}

OUString ConstList::implGetNameDbl( const Config& /*rCfg*/, double /*fValue*/ ) const
{
    return OUString();
}

void ConstList::implIncludeList( const NameListBase& rList )
{
    if( const ConstList* pConstList = dynamic_cast< const ConstList* >( &rList ) )
    {
        maDefName = pConstList->maDefName;
        mbQuoteNames = pConstList->mbQuoteNames;
    }
}

// ============================================================================

MultiList::MultiList( const SharedConfigData& rCfgData ) :
    ConstList( rCfgData ),
    mbIgnoreEmpty( true )
{
}

void MultiList::setNamesFromVec( sal_Int64 nStartKey, const OUStringVector& rNames )
{
    sal_Int64 nKey = nStartKey;
    for( OUStringVector::const_iterator aIt = rNames.begin(), aEnd = rNames.end(); aIt != aEnd; ++aIt, ++nKey )
        if( !mbIgnoreEmpty || (aIt->getLength() > 0) )
            insertRawName( nKey, *aIt );
}

void MultiList::implProcessConfigItemStr(
        const ConfigInputStreamRef& rxStrm, const OUString& rKey, const OUString& rData )
{
    if( rKey.equalsAscii( "ignore-empty" ) )
        mbIgnoreEmpty = StringHelper::convertStringToBool( rData );
    else
        ConstList::implProcessConfigItemStr( rxStrm, rKey, rData );
}

void MultiList::implSetName( sal_Int64 nKey, const OUString& rName )
{
    OUStringVector aNames;
    StringHelper::convertStringToStringList( aNames, rName, false );
    setNamesFromVec( nKey, aNames );
}

// ============================================================================

FlagsList::FlagsList( const SharedConfigData& rCfgData ) :
    NameListBase( rCfgData ),
    mnIgnore( 0 )
{
}

void FlagsList::implProcessConfigItemStr(
        const ConfigInputStreamRef& rxStrm, const OUString& rKey, const OUString& rData )
{
    if( rKey.equalsAscii( "ignore" ) )
    {
        sal_Int64 nIgnore;
        if( StringHelper::convertStringToInt( nIgnore, rData ) )
            setIgnoreFlags( nIgnore );
    }
    else
    {
        NameListBase::implProcessConfigItemStr( rxStrm, rKey, rData );
    }
}

void FlagsList::implSetName( sal_Int64 nKey, const OUString& rName )
{
    insertRawName( nKey, rName );
}

OUString FlagsList::implGetName( const Config& /*rCfg*/, sal_Int64 nKey ) const
{
    sal_Int64 nFlags = nKey;
    setFlag( nFlags, mnIgnore, false );
    sal_Int64 nFound = 0;
    OUStringBuffer aName;
    // add known flags
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        sal_Int64 nMask = aIt->first;
        const OUString& rFlagName = aIt->second;
        bool bNegated = (rFlagName.getLength() > 0) && (rFlagName[ 0 ] == '!');
        if( getFlag( nFlags, nMask ) != bNegated )
            StringHelper::appendToken( aName, bNegated ? rFlagName.copy( 1 ) : rFlagName );
        setFlag( nFound, nMask );
    }
    // add unknown flags
    setFlag( nFlags, nFound, false );
    if( nFlags != 0 )
    {
        OUStringBuffer aUnknown( CREATE_OUSTRING( OOX_DUMP_UNKNOWN ) );
        aUnknown.append( OOX_DUMP_ITEMSEP );
        StringHelper::appendShortHex( aUnknown, nFlags, true );
        StringHelper::enclose( aUnknown, '(', ')' );
        StringHelper::appendToken( aName, aUnknown.makeStringAndClear() );
    }
    return aName.makeStringAndClear();
}

OUString FlagsList::implGetNameDbl( const Config& /*rCfg*/, double /*fValue*/ ) const
{
    return OUString();
}

void FlagsList::implIncludeList( const NameListBase& rList )
{
    if( const FlagsList* pFlagsList = dynamic_cast< const FlagsList* >( &rList ) )
        mnIgnore = pFlagsList->mnIgnore;
}

// ============================================================================

CombiList::CombiList( const SharedConfigData& rCfgData ) :
    FlagsList( rCfgData )
{
}

void CombiList::implSetName( sal_Int64 nKey, const OUString& rName )
{
    if( (nKey & (nKey - 1)) != 0 )  // more than a single bit set?
    {
        ExtItemFormat& rItemFmt = maFmtMap[ nKey ];
        OUStringVector aRemain = rItemFmt.parse( rName );
        rItemFmt.mbShiftValue = aRemain.empty() || !aRemain.front().equalsAscii( "noshift" );
    }
    else
    {
        FlagsList::implSetName( nKey, rName );
    }
}

OUString CombiList::implGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    sal_Int64 nFlags = nKey;
    sal_Int64 nFound = 0;
    OUStringBuffer aName;
    // add known flag fields
    for( ExtItemFormatMap::const_iterator aIt = maFmtMap.begin(), aEnd = maFmtMap.end(); aIt != aEnd; ++aIt )
    {
        sal_Int64 nMask = aIt->first;
        if( nMask != 0 )
        {
            const ExtItemFormat& rItemFmt = aIt->second;

            sal_uInt64 nUFlags = static_cast< sal_uInt64 >( nFlags );
            sal_uInt64 nUMask = static_cast< sal_uInt64 >( nMask );
            if( rItemFmt.mbShiftValue )
                while( (nUMask & 1) == 0 ) { nUFlags >>= 1; nUMask >>= 1; }

            sal_uInt64 nUValue = nUFlags & nUMask;
            sal_Int64 nSValue = static_cast< sal_Int64 >( nUValue );
            if( getFlag< sal_uInt64 >( nUValue, (nUMask + 1) >> 1 ) )
                setFlag( nSValue, static_cast< sal_Int64 >( ~nUMask ) );

            OUStringBuffer aItem( rItemFmt.maItemName );
            OUStringBuffer aValue;
            switch( rItemFmt.meDataType )
            {
                case DATATYPE_INT8:     StringHelper::appendValue( aValue, static_cast< sal_Int8 >( nSValue ), rItemFmt.meFmtType );    break;
                case DATATYPE_UINT8:    StringHelper::appendValue( aValue, static_cast< sal_uInt8 >( nUValue ), rItemFmt.meFmtType );   break;
                case DATATYPE_INT16:    StringHelper::appendValue( aValue, static_cast< sal_Int16 >( nSValue ), rItemFmt.meFmtType );   break;
                case DATATYPE_UINT16:   StringHelper::appendValue( aValue, static_cast< sal_uInt16 >( nUValue ), rItemFmt.meFmtType );  break;
                case DATATYPE_INT32:    StringHelper::appendValue( aValue, static_cast< sal_Int32 >( nSValue ), rItemFmt.meFmtType );   break;
                case DATATYPE_UINT32:   StringHelper::appendValue( aValue, static_cast< sal_uInt32 >( nUValue ), rItemFmt.meFmtType );  break;
                case DATATYPE_INT64:    StringHelper::appendValue( aValue, nSValue, rItemFmt.meFmtType );                               break;
                case DATATYPE_UINT64:   StringHelper::appendValue( aValue, nUValue, rItemFmt.meFmtType );                               break;
                case DATATYPE_FLOAT:    StringHelper::appendValue( aValue, static_cast< float >( nSValue ), rItemFmt.meFmtType );       break;
                case DATATYPE_DOUBLE:   StringHelper::appendValue( aValue, static_cast< double >( nSValue ), rItemFmt.meFmtType );      break;
                default:;
            }
            StringHelper::appendToken( aItem, aValue.makeStringAndClear(), OOX_DUMP_ITEMSEP );
            if( rItemFmt.maListName.getLength() > 0 )
            {
                OUString aValueName = rCfg.getName( rItemFmt.maListName, static_cast< sal_Int64 >( nUValue ) );
                StringHelper::appendToken( aItem, aValueName, OOX_DUMP_ITEMSEP );
            }
            StringHelper::enclose( aItem, '(', ')' );
            StringHelper::appendToken( aName, aItem.makeStringAndClear() );
            setFlag( nFound, nMask );
        }
    }
    setFlag( nFlags, nFound, false );
    StringHelper::appendToken( aName, FlagsList::implGetName( rCfg, nFlags ) );
    return aName.makeStringAndClear();
}

void CombiList::implIncludeList( const NameListBase& rList )
{
    if( const CombiList* pCombiList = dynamic_cast< const CombiList* >( &rList ) )
        maFmtMap = pCombiList->maFmtMap;
    FlagsList::implIncludeList( rList );
}

// ============================================================================

UnitConverter::UnitConverter( const SharedConfigData& rCfgData ) :
    NameListBase( rCfgData ),
    mfFactor( 1.0 )
{
}

void UnitConverter::implSetName( sal_Int64 /*nKey*/, const OUString& /*rName*/ )
{
    // nothing to do
}

OUString UnitConverter::implGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    return implGetNameDbl( rCfg, static_cast< double >( nKey ) );
}

OUString UnitConverter::implGetNameDbl( const Config& /*rCfg*/, double fValue ) const
{
    OUStringBuffer aValue;
    StringHelper::appendDec( aValue, mfFactor * fValue );
    aValue.append( maUnitName );
    return aValue.makeStringAndClear();
}

void UnitConverter::implIncludeList( const NameListBase& /*rList*/ )
{
}

// ============================================================================

NameListRef NameListWrapper::getNameList( const Config& rCfg ) const
{
    return mxList.get() ? mxList : (mxList = rCfg.getNameList( maNameWrp.getString() ));
}

// ============================================================================
// ============================================================================

SharedConfigData::SharedConfigData( const OUString& rFileName )
{
    construct( rFileName );
}

SharedConfigData::~SharedConfigData()
{
}

void SharedConfigData::construct( const OUString& rFileName )
{
    mbLoaded = false;
    OUString aFileUrl = InputOutputHelper::convertFileNameToUrl( rFileName );
    if( aFileUrl.getLength() > 0 )
    {
        sal_Int32 nNamePos = InputOutputHelper::getFileNamePos( aFileUrl );
        maConfigPath = aFileUrl.copy( 0, nNamePos );
        mbLoaded = readConfigFile( aFileUrl );
    }
}

void SharedConfigData::setOption( const OUString& rKey, const OUString& rData )
{
    maConfigData[ rKey ] = rData;
}

const OUString* SharedConfigData::getOption( const OUString& rKey ) const
{
    ConfigDataMap::const_iterator aIt = maConfigData.find( rKey );
    return (aIt == maConfigData.end()) ? 0 : &aIt->second;
}

void SharedConfigData::setNameList( const OUString& rListName, NameListRef xList )
{
    if( rListName.getLength() > 0 )
        maNameLists[ rListName ] = xList;
}

void SharedConfigData::eraseNameList( const OUString& rListName )
{
    maNameLists.erase( rListName );
}

NameListRef SharedConfigData::getNameList( const OUString& rListName ) const
{
    NameListRef xList;
    NameListMap::const_iterator aIt = maNameLists.find( rListName );
    if( aIt != maNameLists.end() )
        xList = aIt->second;
    return xList;
}

bool SharedConfigData::implIsValid() const
{
    return mbLoaded;
}

void SharedConfigData::implProcessConfigItemStr(
        const ConfigInputStreamRef& rxStrm, const OUString& rKey, const OUString& rData )
{
    if( rKey.equalsAscii( "include-config-file" ) )
        readConfigFile( maConfigPath + rData );
    else if( rKey.equalsAscii( "constlist" ) )
        readNameList< ConstList >( rxStrm, rData );
    else if( rKey.equalsAscii( "multilist" ) )
        readNameList< MultiList >( rxStrm, rData );
    else if( rKey.equalsAscii( "flagslist" ) )
        readNameList< FlagsList >( rxStrm, rData );
    else if( rKey.equalsAscii( "combilist" ) )
        readNameList< CombiList >( rxStrm, rData );
    else if( rKey.equalsAscii( "shortlist" ) )
        createShortList( rData );
    else if( rKey.equalsAscii( "unitconverter" ) )
        createUnitConverter( rData );
    else
        setOption( rKey, rData );
}

bool SharedConfigData::readConfigFile( const OUString& rFileUrl )
{
    bool bLoaded = maConfigFiles.count( rFileUrl ) > 0;
    if( !bLoaded )
    {
        Reference< XTextInputStream > xTextInStrm =
            InputOutputHelper::openTextInputStream( rFileUrl, CREATE_OUSTRING( "UTF-8" ) );
        if( xTextInStrm.is() )
        {
            maConfigFiles.insert( rFileUrl );
            readConfigBlockContents( xTextInStrm );
            bLoaded = true;
        }
    }
    return bLoaded;
}

void SharedConfigData::createShortList( const OUString& rData )
{
    OUStringVector aDataVec;
    StringHelper::convertStringToStringList( aDataVec, rData, false );
    if( aDataVec.size() >= 3 )
    {
        sal_Int64 nStartKey;
        if( StringHelper::convertStringToInt( nStartKey, aDataVec[ 1 ] ) )
        {
            ::boost::shared_ptr< MultiList > xList = createNameList< MultiList >( aDataVec[ 0 ] );
            if( xList.get() )
            {
                aDataVec.erase( aDataVec.begin(), aDataVec.begin() + 2 );
                xList->setNamesFromVec( nStartKey, aDataVec );
            }
        }
    }
}

void SharedConfigData::createUnitConverter( const OUString& rData )
{
    OUStringVector aDataVec;
    StringHelper::convertStringToStringList( aDataVec, rData, false );
    if( aDataVec.size() >= 2 )
    {
        OUString aFactor = aDataVec[ 1 ];
        bool bRecip = (aFactor.getLength() > 0) && (aFactor[ 0 ] == '/');
        if( bRecip )
            aFactor = aFactor.copy( 1 );
        double fFactor;
        if( StringHelper::convertStringToDouble( fFactor, aFactor ) && (fFactor != 0.0) )
        {
            ::boost::shared_ptr< UnitConverter > xList = createNameList< UnitConverter >( aDataVec[ 0 ] );
            if( xList.get() )
            {
                xList->setFactor( bRecip ? (1.0 / fFactor) : fFactor );
                if( aDataVec.size() >= 3 )
                    xList->setUnitName( aDataVec[ 2 ] );
            }
        }
    }
}

// ============================================================================

Config::Config( const Config& rParent ) :
    Base()  // c'tor needs to be called explicitly to avoid compiler warning
{
    construct( rParent );
}

Config::Config( const OUString& rFileName )
{
    construct( rFileName );
}

Config::Config( const sal_Char* pcEnvVar )
{
    construct( pcEnvVar );
}

Config::~Config()
{
}

void Config::construct( const Config& rParent )
{
    *this = rParent;
}

void Config::construct( const OUString& rFileName )
{
    mxCfgData.reset( new SharedConfigData( rFileName ) );
}

void Config::construct( const sal_Char* pcEnvVar )
{
    if( pcEnvVar )
        if( const sal_Char* pcFileName = ::getenv( pcEnvVar ) )
            construct( OUString::createFromAscii( pcFileName ) );
}

void Config::setStringOption( const StringWrapper& rKey, const StringWrapper& rData )
{
    mxCfgData->setOption( rKey.getString(), rData.getString() );
}

const OUString& Config::getStringOption( const StringWrapper& rKey, const OUString& rDefault ) const
{
    const OUString* pData = implGetOption( rKey.getString() );
    return pData ? *pData : rDefault;
}

bool Config::getBoolOption( const StringWrapper& rKey, bool bDefault ) const
{
    const OUString* pData = implGetOption( rKey.getString() );
    return pData ? StringHelper::convertStringToBool( *pData ) : bDefault;
}

bool Config::isDumperEnabled() const
{
    return getBoolOption( "enable-dumper", false );
}

bool Config::isImportEnabled() const
{
    return getBoolOption( "enable-import", true );
}

void Config::setNameList( const StringWrapper& rListName, NameListRef xList )
{
    mxCfgData->setNameList( rListName.getString(), xList );
}

void Config::eraseNameList( const StringWrapper& rListName )
{
    mxCfgData->eraseNameList( rListName.getString() );
}

NameListRef Config::getNameList( const StringWrapper& rListName ) const
{
    return implGetNameList( rListName.getString() );
}

bool Config::implIsValid() const
{
    return isValid( mxCfgData );
}

const OUString* Config::implGetOption( const OUString& rKey ) const
{
    return mxCfgData->getOption( rKey );
}

NameListRef Config::implGetNameList( const OUString& rListName ) const
{
    return mxCfgData->getNameList( rListName );
}

// ============================================================================
// ============================================================================

bool Input::implIsValid() const
{
    return true;
}

Input& operator>>( Input& rIn, sal_Int64& rnData )
{
    return rIn >> *reinterpret_cast< double* >( &rnData );
}

Input& operator>>( Input& rIn, sal_uInt64& rnData )
{
    return rIn >> *reinterpret_cast< double* >( &rnData );
}

// ============================================================================

BinaryInput::BinaryInput( BinaryInputStream& rStrm ) :
    mrStrm( rStrm )
{
}

sal_Int64 BinaryInput::getSize() const
{
    return mrStrm.getLength();
}

sal_Int64 BinaryInput::tell() const
{
    return mrStrm.tell();
}

void BinaryInput::seek( sal_Int64 nPos )
{
    mrStrm.seek( nPos );
}

void BinaryInput::skip( sal_Int32 nBytes )
{
    mrStrm.skip( nBytes );
}

sal_Int32 BinaryInput::read( void* pBuffer, sal_Int32 nBytes )
{
    return mrStrm.read( pBuffer, nBytes );
}

bool BinaryInput::implIsValid() const
{
    return mrStrm.is() && Input::implIsValid();
}

BinaryInput& BinaryInput::operator>>( sal_Int8& rnData )   { mrStrm >> rnData; return *this; }
BinaryInput& BinaryInput::operator>>( sal_uInt8& rnData )  { mrStrm >> rnData; return *this; }
BinaryInput& BinaryInput::operator>>( sal_Int16& rnData )  { mrStrm >> rnData; return *this; }
BinaryInput& BinaryInput::operator>>( sal_uInt16& rnData ) { mrStrm >> rnData; return *this; }
BinaryInput& BinaryInput::operator>>( sal_Int32& rnData )  { mrStrm >> rnData; return *this; }
BinaryInput& BinaryInput::operator>>( sal_uInt32& rnData ) { mrStrm >> rnData; return *this; }
BinaryInput& BinaryInput::operator>>( float& rfData )      { mrStrm >> rfData; return *this; }
BinaryInput& BinaryInput::operator>>( double& rfData )     { mrStrm >> rfData; return *this; }

// ============================================================================
// ============================================================================

Output::Output( const Reference< XTextOutputStream >& rxStrm )
{
    construct( rxStrm );
}

Output::Output( const ::rtl::OUString& rFileName )
{
    construct( InputOutputHelper::openTextOutputStream( rFileName, CREATE_OUSTRING( "UTF-8" ) ) );
}

// ----------------------------------------------------------------------------

void Output::newLine()
{
    if( maLine.getLength() > 0 )
    {
        mxStrm->writeString( maIndent );
        maLine.append( sal_Unicode( '\n' ) );
        mxStrm->writeString( maLine.makeStringAndClear() );
        mnCol = 0;
        mnLastItem = 0;
    }
}

void Output::emptyLine( size_t nCount )
{
    for( size_t nIdx = 0; nIdx < nCount; ++nIdx )
        mxStrm->writeString( OUString( sal_Unicode( '\n' ) ) );
}

void Output::incIndent()
{
    OUStringBuffer aBuffer( maIndent );
    StringHelper::appendChar( aBuffer, ' ', OOX_DUMP_INDENT );
    maIndent = aBuffer.makeStringAndClear();
}

void Output::decIndent()
{
    if( maIndent.getLength() >= OOX_DUMP_INDENT )
        maIndent = maIndent.copy( OOX_DUMP_INDENT );
}

void Output::resetIndent()
{
    maIndent = OUString();
}

void Output::startTable( sal_Int32 nW1 )
{
    startTable( 1, &nW1 );
}

void Output::startTable( sal_Int32 nW1, sal_Int32 nW2 )
{
    sal_Int32 pnColWidths[ 2 ];
    pnColWidths[ 0 ] = nW1;
    pnColWidths[ 1 ] = nW2;
    startTable( 2, pnColWidths );
}

void Output::startTable( sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3 )
{
    sal_Int32 pnColWidths[ 3 ];
    pnColWidths[ 0 ] = nW1;
    pnColWidths[ 1 ] = nW2;
    pnColWidths[ 2 ] = nW3;
    startTable( 3, pnColWidths );
}

void Output::startTable( sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3, sal_Int32 nW4 )
{
    sal_Int32 pnColWidths[ 4 ];
    pnColWidths[ 0 ] = nW1;
    pnColWidths[ 1 ] = nW2;
    pnColWidths[ 2 ] = nW3;
    pnColWidths[ 3 ] = nW4;
    startTable( 4, pnColWidths );
}

void Output::startTable( size_t nColCount, const sal_Int32* pnColWidths )
{
    maColPos.clear();
    maColPos.push_back( 0 );
    sal_Int32 nColPos = 0;
    for( size_t nCol = 0; nCol < nColCount; ++nCol )
    {
        nColPos = nColPos + pnColWidths[ nCol ];
        maColPos.push_back( nColPos );
    }
}

void Output::tab()
{
    tab( mnCol + 1 );
}

void Output::tab( size_t nCol )
{
    mnCol = nCol;
    if( mnCol < maColPos.size() )
    {
        sal_Int32 nColPos = maColPos[ mnCol ];
        if( maLine.getLength() >= nColPos )
            maLine.setLength( ::std::max< sal_Int32 >( nColPos - 1, 0 ) );
        StringHelper::appendChar( maLine, ' ', nColPos - maLine.getLength() );
    }
    else
    {
        StringHelper::appendChar( maLine, ' ', 2 );
    }
}

void Output::endTable()
{
    maColPos.clear();
}

void Output::resetItemIndex( sal_Int64 nIdx )
{
    mnItemIdx = nIdx;
}

void Output::startItem( const sal_Char* pcName )
{
    if( mnItemLevel == 0 )
    {
        if( (mnMultiLevel > 0) && (maLine.getLength() > 0) )
            tab();
        if( pcName )
        {
            writeItemName( pcName );
            writeChar( OOX_DUMP_ITEMSEP );
        }
    }
    ++mnItemLevel;
    mnLastItem = maLine.getLength();
}

void Output::contItem()
{
    if( mnItemLevel > 0 )
    {
        if( (maLine.getLength() == 0) || (maLine[ maLine.getLength() - 1 ] != OOX_DUMP_ITEMSEP) )
            writeChar( OOX_DUMP_ITEMSEP );
        mnLastItem = maLine.getLength();
    }
}

void Output::endItem()
{
    if( mnItemLevel > 0 )
    {
        maLastItem = OUString( maLine.getStr() + mnLastItem );
        if( (maLastItem.getLength() == 0) && (mnLastItem > 0) && (maLine[ mnLastItem - 1 ] == OOX_DUMP_ITEMSEP) )
            maLine.setLength( mnLastItem - 1 );
        --mnItemLevel;
    }
    if( mnItemLevel == 0 )
    {
        if( mnMultiLevel == 0 )
            newLine();
    }
    else
        contItem();
}

void Output::startMultiItems()
{
    ++mnMultiLevel;
}

void Output::endMultiItems()
{
    if( mnMultiLevel > 0 )
        --mnMultiLevel;
    if( mnMultiLevel == 0 )
        newLine();
}

// ----------------------------------------------------------------------------

void Output::writeChar( sal_Unicode cChar, sal_Int32 nCount )
{
    StringHelper::appendEncChar( maLine, cChar, nCount );
}

void Output::writeAscii( const sal_Char* pcStr )
{
    if( pcStr )
        maLine.appendAscii( pcStr );
}

void Output::writeString( const OUString& rStr )
{
    StringHelper::appendEncString( maLine, rStr );
}

void Output::writeArray( const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep )
{
    const sal_uInt8* pnEnd = pnData ? (pnData + nSize) : 0;
    for( const sal_uInt8* pnByte = pnData; pnByte < pnEnd; ++pnByte )
    {
        if( pnByte > pnData )
            writeChar( cSep );
        writeHex( *pnByte, false );
    }
}

void Output::writeBool( bool bData )
{
    StringHelper::appendBool( maLine, bData );
}

void Output::writeColor( sal_Int32 nColor )
{
    writeChar( 'a' );
    writeDec( static_cast< sal_uInt8 >( nColor >> 24 ) );
    writeAscii( ",r" );
    writeDec( static_cast< sal_uInt8 >( nColor >> 16 ) );
    writeAscii( ",g" );
    writeDec( static_cast< sal_uInt8 >( nColor >> 8 ) );
    writeAscii( ",b" );
    writeDec( static_cast< sal_uInt8 >( nColor ) );
}

void Output::writeDateTime( const DateTime& rDateTime )
{
    writeDec( rDateTime.Year, 4, '0' );
    writeChar( '-' );
    writeDec( rDateTime.Month, 2, '0' );
    writeChar( '-' );
    writeDec( rDateTime.Day, 2, '0' );
    writeChar( 'T' );
    writeDec( rDateTime.Hours, 2, '0' );
    writeChar( ':' );
    writeDec( rDateTime.Minutes, 2, '0' );
    writeChar( ':' );
    writeDec( rDateTime.Seconds, 2, '0' );
}

void Output::writeColIndex( sal_Int32 nCol )
{
    StringHelper::appendAddrCol( maLine, nCol, true );
}

void Output::writeRowIndex( sal_Int32 nRow )
{
    StringHelper::appendAddrRow( maLine, nRow, true );
}

void Output::writeColRowRange( sal_Int32 nColRow1, sal_Int32 nColRow2 )
{
    writeDec( nColRow1 );
    writeChar( OOX_DUMP_RANGESEP );
    writeDec( nColRow2 );
}

void Output::writeColRange( sal_Int32 nCol1, sal_Int32 nCol2 )
{
    writeColIndex( nCol1 );
    writeChar( OOX_DUMP_RANGESEP );
    writeColIndex( nCol2 );
}

void Output::writeRowRange( sal_Int32 nRow1, sal_Int32 nRow2 )
{
    writeRowIndex( nRow1 );
    writeChar( OOX_DUMP_RANGESEP );
    writeRowIndex( nRow2 );
}

void Output::writeAddress( const Address& rPos )
{
    StringHelper::appendAddress( maLine, rPos );
}

void Output::writeRange( const Range& rRange )
{
    StringHelper::appendRange( maLine, rRange );
}

void Output::writeRangeList( const RangeList& rRanges )
{
    StringHelper::appendRangeList( maLine, rRanges );
}

// ----------------------------------------------------------------------------

void Output::construct( const Reference< XTextOutputStream >& rxStrm )
{
    mxStrm = rxStrm;
    mnCol = mnItemLevel = mnMultiLevel = 0;
    mnItemIdx = 0;
    mnLastItem = 0;
    if( mxStrm.is() )
    {
        writeChar( OOX_DUMP_BOM );
        writeAscii( "OpenOffice.org binary file dumper v2.0" );
        newLine();
        emptyLine();
    }
}

bool Output::implIsValid() const
{
    return mxStrm.is();
}

void Output::writeItemName( const sal_Char* pcName )
{
    if( pcName && (*pcName == '#') )
    {
        writeAscii( pcName + 1 );
        StringHelper::appendIndex( maLine, mnItemIdx++ );
    }
    else
        writeAscii( pcName );
}

// ============================================================================
// ============================================================================

ObjectBase::~ObjectBase()
{
}

void ObjectBase::construct( const FilterBase& rFilter, ConfigRef xConfig )
{
    mpFilter = &rFilter;
    mxConfig = xConfig;
}

void ObjectBase::construct( const ObjectBase& rParent )
{
    *this = rParent;
}

void ObjectBase::dump()
{
    if( isValid() )
        implDump();
}

bool ObjectBase::implIsValid() const
{
    return mpFilter && mpFilter->isImportFilter() && isValid( mxConfig );
}

ConfigRef ObjectBase::implReconstructConfig()
{
    return mxConfig;
}

void ObjectBase::implDump()
{
}

void ObjectBase::reconstructConfig()
{
    mxConfig = implReconstructConfig();
}

// ============================================================================
// ============================================================================

RootStorageObjectBase::~RootStorageObjectBase()
{
}

void RootStorageObjectBase::construct( const ObjectBase& rParent )
{
    ObjectBase::construct( rParent );
}

void RootStorageObjectBase::implDump()
{
    StorageRef xStrg = getFilter().getStorage();
    if( xStrg.get() && xStrg->isStorage() )
        extractStorage( xStrg, getFilter().getFileUrl() + CREATE_OUSTRING( ".dump" ) );
}

void RootStorageObjectBase::implDumpStream( BinaryInputStreamRef, const OUString&, const OUString&, const OUString& )
{
}

void RootStorageObjectBase::extractStream( StorageBase& rStrg, const OUString& rStrmName, const OUString& rSystemFileName )
{
    BinaryInputStream aInStrm( rStrg.openInputStream( rStrmName ), true );
    if( aInStrm.is() )
    {
        BinaryOutputStream aOutStrm( InputOutputHelper::openOutputStream( rSystemFileName ), true );
        if( aOutStrm.is() )
            aOutStrm.copy( aInStrm );
    }
    BinaryInputStreamRef xDumpStrm( new BinaryInputStream( InputOutputHelper::openInputStream( rSystemFileName ), true ) );
    if( xDumpStrm->is() )
        implDumpStream( xDumpStrm, rStrg.getPath(), rStrmName, rSystemFileName + CREATE_OUSTRING( ".dump" ) );
}

void RootStorageObjectBase::extractStorage( StorageRef xStrg, const OUString& rSystemPath )
{
    // create directory in file system
    ::osl::FileBase::RC eRes = ::osl::Directory::create( rSystemPath );
    if( (eRes == ::osl::FileBase::E_None) || (eRes == ::osl::FileBase::E_EXIST) )
    {
        // process children of the storage
        for( StorageIterator aIt( xStrg ); aIt.isValid(); ++aIt )
        {
            // encode all characters < 0x20
            OUStringBuffer aBuffer;
            StringHelper::appendEncString( aBuffer, aIt.getName(), false );

            // replace all characters reserved in file system
            OUString aSystemName = aBuffer.makeStringAndClear();
            static const sal_Unicode spcReserved[] = { '/', '\\', ':', '*', '?', '<', '>', '|' };
            for( const sal_Unicode* pcChar = spcReserved; pcChar < STATIC_ARRAY_END( spcReserved ); ++pcChar )
                aSystemName = aSystemName.replace( *pcChar, '_' );

            // build full path
            OUString aFullSystemName = rSystemPath + OUString( sal_Unicode( '/' ) ) + aSystemName;

            // handle storages and streams
            if( aIt.isStorage() )
                extractStorage( xStrg->openSubStorage( aIt.getName(), false ), aFullSystemName );
            else if( aIt.isStream() )
                extractStream( *xStrg, aIt.getName(), aFullSystemName );
        }
    }
}

// ============================================================================

StorageIterator::StorageIterator( StorageRef xStrg ) :
    mxStrg( xStrg )
{
    if( mxStrg.get() )
        mxStrg->getElementNames( maNames );
    maIt = maNames.begin();
}

StorageIterator::~StorageIterator()
{
}

size_t StorageIterator::getElementCount() const
{
    return maNames.size();
}

StorageIterator& StorageIterator::operator++()
{
    if( maIt != maNames.end() )
        ++maIt;
    return *this;
}

OUString StorageIterator::getName() const
{
    OUString aName;
    if( maIt != maNames.end() )
        aName = *maIt;
    return aName;
}

bool StorageIterator::isStream() const
{
    return isValid() && mxStrg->openInputStream( *maIt ).is();
}

bool StorageIterator::isStorage() const
{
    if( !isValid() )
        return false;
    StorageRef xStrg = mxStrg->openSubStorage( *maIt, false );
    return xStrg.get() && xStrg->isStorage();
}

bool StorageIterator::implIsValid() const
{
    return mxStrg.get() && mxStrg->isStorage() && (maIt != maNames.end());
}

// ============================================================================
// ============================================================================

OutputObjectBase::~OutputObjectBase()
{
}

void OutputObjectBase::construct( const ObjectBase& rParent, const OUString& rOutFileName )
{
    ObjectBase::construct( rParent );
    mxOut.reset( new Output( rOutFileName ) );
}

void OutputObjectBase::construct( const ObjectBase& rParent, OutputRef xOut )
{
    ObjectBase::construct( rParent );
    mxOut = xOut;
}

void OutputObjectBase::construct( const OutputObjectBase& rParent )
{
    *this = rParent;
}

bool OutputObjectBase::implIsValid() const
{
    return isValid( mxOut ) && ObjectBase::implIsValid();
}

OutputRef OutputObjectBase::implReconstructOutput()
{
    return mxOut;
}

void OutputObjectBase::reconstructOutput()
{
    mxOut = implReconstructOutput();
}

void OutputObjectBase::writeEmptyItem( const sal_Char* pcName )
{
    ItemGuard aItem( *mxOut, pcName );
}

void OutputObjectBase::writeInfoItem( const sal_Char* pcName, const StringWrapper& rData )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeString( rData.getString() );
}

void OutputObjectBase::writeStringItem( const sal_Char* pcName, const ::rtl::OUString& rData )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeAscii( "(len=" );
    mxOut->writeDec( rData.getLength() );
    mxOut->writeAscii( ")," );
    OUStringBuffer aValue( rData.copy( 0, ::std::min( rData.getLength(), OOX_DUMP_MAXSTRLEN ) ) );
    StringHelper::enclose( aValue, OOX_DUMP_STRQUOTE );
    mxOut->writeString( aValue.makeStringAndClear() );
    if( rData.getLength() > OOX_DUMP_MAXSTRLEN )
        mxOut->writeAscii( ",cut" );
}

void OutputObjectBase::writeArrayItem( const sal_Char* pcName, const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeArray( pnData, nSize, cSep );
}

void OutputObjectBase::writeBoolItem( const sal_Char* pcName, bool bData )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeBool( bData );
}

double OutputObjectBase::writeRkItem( const sal_Char* pcName, sal_Int32 nRk )
{
    MultiItemsGuard aMultiGuard( out() );
    writeHexItem( pcName, static_cast< sal_uInt32 >( nRk ), "RK-FLAGS" );
    double fValue = ::oox::xls::BiffHelper::calcDoubleFromRk( nRk );
    writeDecItem( "decoded", fValue );
    return fValue;
}

void OutputObjectBase::writeColorItem( const sal_Char* pcName, sal_Int32 nColor )
{
    ItemGuard aItem( *mxOut, pcName );
    writeHexItem( pcName, nColor );
    mxOut->writeColor( nColor );
}

void OutputObjectBase::writeDateTimeItem( const sal_Char* pcName, const DateTime& rDateTime )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeDateTime( rDateTime );
}

void OutputObjectBase::writeGuidItem( const sal_Char* pcName, const OUString& rGuid )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeString( rGuid );
    aItem.cont();
    mxOut->writeString( cfg().getStringOption( rGuid, OUString() ) );
}

void OutputObjectBase::writeColIndexItem( const sal_Char* pcName, sal_Int32 nCol )
{
    Output& rOut = out();
    ItemGuard aItem( rOut, pcName );
    rOut.writeDec( nCol );
    aItem.cont();
    rOut.writeColIndex( nCol );
}

void OutputObjectBase::writeRowIndexItem( const sal_Char* pcName, sal_Int32 nRow )
{
    Output& rOut = out();
    ItemGuard aItem( rOut, pcName );
    rOut.writeDec( nRow );
    aItem.cont();
    rOut.writeRowIndex( nRow );
}

void OutputObjectBase::writeColRangeItem( const sal_Char* pcName, sal_Int32 nCol1, sal_Int32 nCol2 )
{
    Output& rOut = out();
    ItemGuard aItem( rOut, pcName );
    rOut.writeColRowRange( nCol1, nCol2 );
    aItem.cont();
    rOut.writeColRange( nCol1, nCol2 );
}

void OutputObjectBase::writeRowRangeItem( const sal_Char* pcName, sal_Int32 nRow1, sal_Int32 nRow2 )
{
    Output& rOut = out();
    ItemGuard aItem( rOut, pcName );
    rOut.writeColRowRange( nRow1, nRow2 );
    aItem.cont();
    rOut.writeRowRange( nRow1, nRow2 );
}

void OutputObjectBase::writeAddressItem( const sal_Char* pcName, const Address& rPos )
{
    ItemGuard aItem( out(), pcName );
    StringHelper::appendAddress( out().getLine(), rPos );
}

void OutputObjectBase::writeRangeItem( const sal_Char* pcName, const Range& rRange )
{
    ItemGuard aItem( out(), pcName );
    StringHelper::appendRange( out().getLine(), rRange );
}

void OutputObjectBase::writeRangeListItem( const sal_Char* pcName, const RangeList& rRanges )
{
    MultiItemsGuard aMultiGuard( out() );
    writeEmptyItem( pcName );
    writeDecItem( "count", static_cast< sal_uInt16 >( rRanges.size() ) );
    ItemGuard aItem( out(), "ranges" );
    StringHelper::appendRangeList( out().getLine(), rRanges );
}

void OutputObjectBase::writeTokenAddressItem( const sal_Char* pcName, const TokenAddress& rPos, bool bNameMode )
{
    ItemGuard aItem( out(), pcName );
    StringHelper::appendAddress( out().getLine(), rPos, bNameMode );
}

void OutputObjectBase::writeTokenAddress3dItem( const sal_Char* pcName, const OUString& rRef, const TokenAddress& rPos, bool bNameMode )
{
    ItemGuard aItem( out(), pcName );
    out().writeString( rRef );
    StringHelper::appendAddress( out().getLine(), rPos, bNameMode );
}

void OutputObjectBase::writeTokenRangeItem( const sal_Char* pcName, const TokenRange& rRange, bool bNameMode )
{
    ItemGuard aItem( out(), pcName );
    StringHelper::appendRange( out().getLine(), rRange, bNameMode );
}

void OutputObjectBase::writeTokenRange3dItem( const sal_Char* pcName, const OUString& rRef, const TokenRange& rRange, bool bNameMode )
{
    ItemGuard aItem( out(), pcName );
    out().writeString( rRef );
    StringHelper::appendRange( out().getLine(), rRange, bNameMode );
}

// ============================================================================
// ============================================================================

InputObjectBase::~InputObjectBase()
{
}

void InputObjectBase::construct( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, InputRef xIn )
{
    OutputObjectBase::construct( rParent, rOutFileName );
    mxIn = xIn;
}

void InputObjectBase::construct( const ObjectBase& rParent, OutputRef xOut, InputRef xIn )
{
    OutputObjectBase::construct( rParent, xOut );
    mxIn = xIn;
}

void InputObjectBase::construct( const OutputObjectBase& rParent, InputRef xIn )
{
    OutputObjectBase::construct( rParent );
    mxIn = xIn;
}

void InputObjectBase::construct( const InputObjectBase& rParent )
{
    *this = rParent;
}

bool InputObjectBase::implIsValid() const
{
    return isValid( mxIn ) && OutputObjectBase::implIsValid();
}

InputRef InputObjectBase::implReconstructInput()
{
    return mxIn;
}

void InputObjectBase::reconstructInput()
{
    mxIn = implReconstructInput();
}

void InputObjectBase::skipBlock( sal_Int32 nBytes, bool bShowSize )
{
    sal_Int64 nEndPos = ::std::min< sal_Int64 >( mxIn->tell() + nBytes, mxIn->getSize() );
    if( mxIn->tell() < nEndPos )
    {
        if( bShowSize )
            writeDecItem( "skipped-data-size", static_cast< sal_uInt64 >( nEndPos - mxIn->tell() ) );
        mxIn->seek( nEndPos );
    }
}

void InputObjectBase::dumpRawBinary( sal_Int32 nBytes, bool bShowOffset, bool bStream )
{
    Output& rOut = out();
    TableGuard aTabGuard( rOut,
        bShowOffset ? 12 : 0,
        3 * OOX_DUMP_BYTESPERLINE / 2 + 1,
        3 * OOX_DUMP_BYTESPERLINE / 2 + 1,
        OOX_DUMP_BYTESPERLINE / 2 + 1 );

    sal_Int32 nMaxShowSize = cfg().getIntOption< sal_Int32 >(
        bStream ? "max-binary-stream-size" : "max-binary-data-size", SAL_MAX_INT32 );

    bool bSeekable = mxIn->getSize() >= 0;
    sal_Int64 nEndPos = bSeekable ? ::std::min< sal_Int64 >( mxIn->tell() + nBytes, mxIn->getSize() ) : 0;
    sal_Int64 nDumpEnd = bSeekable ? ::std::min< sal_Int64 >( mxIn->tell() + nMaxShowSize, nEndPos ) : nMaxShowSize;
    sal_Int64 nPos = bSeekable ? mxIn->tell() : 0;
    bool bLoop = true;

    while( bLoop && (nPos < nDumpEnd) )
    {
        rOut.writeHex( static_cast< sal_uInt32 >( nPos ) );
        rOut.tab();

        sal_uInt8 pnLineData[ OOX_DUMP_BYTESPERLINE ];
        sal_Int32 nLineSize = bSeekable ? ::std::min( static_cast< sal_Int32 >( nDumpEnd - mxIn->tell() ), OOX_DUMP_BYTESPERLINE ) : OOX_DUMP_BYTESPERLINE;
        sal_Int32 nReadSize = mxIn->read( pnLineData, nLineSize );
        bLoop = nReadSize == nLineSize;
        nPos += nReadSize;

        if( nReadSize > 0 )
        {
            const sal_uInt8* pnByte = 0;
            const sal_uInt8* pnEnd = 0;
            for( pnByte = pnLineData, pnEnd = pnLineData + nReadSize; pnByte != pnEnd; ++pnByte )
            {
                if( (pnByte - pnLineData) == (OOX_DUMP_BYTESPERLINE / 2) ) rOut.tab();
                rOut.writeHex( *pnByte, false );
                rOut.writeChar( ' ' );
            }

            aTabGuard.tab( 3 );
            for( pnByte = pnLineData, pnEnd = pnLineData + nLineSize; pnByte != pnEnd; ++pnByte )
            {
                if( (pnByte - pnLineData) == (OOX_DUMP_BYTESPERLINE / 2) ) rOut.tab();
                rOut.writeChar( static_cast< sal_Unicode >( (*pnByte < 0x20) ? '.' : *pnByte ) );
            }
            rOut.newLine();
        }
    }

    // skip undumped data
    if( bSeekable )
        skipBlock( static_cast< sal_Int32 >( nEndPos - mxIn->tell() ) );
}

void InputObjectBase::dumpBinary( const sal_Char* pcName, sal_Int32 nBytes, bool bShowOffset )
{
    {
        MultiItemsGuard aMultiGuard( out() );
        writeEmptyItem( pcName );
        writeDecItem( "size", nBytes );
    }
    IndentGuard aIndGuard( out() );
    dumpRawBinary( nBytes, bShowOffset );
}

void InputObjectBase::dumpArray( const sal_Char* pcName, sal_Int32 nBytes, sal_Unicode cSep )
{
    sal_Int32 nDumpSize = getLimitedValue< sal_Int32, sal_Int64 >( mxIn->getSize() - mxIn->tell(), 0, nBytes );
    if( nDumpSize > OOX_DUMP_MAXARRAY )
    {
        dumpBinary( pcName, nBytes, false );
    }
    else if( nDumpSize > 1 )
    {
        sal_uInt8 pnData[ OOX_DUMP_MAXARRAY ];
        mxIn->read( pnData, nDumpSize );
        writeArrayItem( pcName, pnData, nDumpSize, cSep );
    }
    else if( nDumpSize == 1 )
        dumpHex< sal_uInt8 >( pcName );
}

void InputObjectBase::dumpRemaining( sal_Int32 nBytes )
{
    if( nBytes > 0 )
    {
        if( cfg().getBoolOption( "show-trailing-unknown", true ) )
            dumpBinary( "remaining-data", nBytes, false );
        else
            skipBlock( nBytes );
    }
}

OUString InputObjectBase::dumpCharArray( const sal_Char* pcName, sal_Int32 nSize, rtl_TextEncoding eTextEnc )
{
    sal_Int32 nDumpSize = getLimitedValue< sal_Int32, sal_Int64 >( mxIn->getSize() - mxIn->tell(), 0, nSize );
    OUString aString;
    if( nDumpSize > 0 )
    {
        ::std::vector< sal_Char > aBuffer( static_cast< sal_Size >( nSize ) + 1 );
        sal_Int32 nCharsRead = mxIn->read( &aBuffer.front(), nSize );
        aBuffer[ nCharsRead ] = 0;
        aString = OStringToOUString( OString( &aBuffer.front() ), eTextEnc );
    }
    writeStringItem( pcName, aString );
    return aString;
}

OUString InputObjectBase::dumpUnicodeArray( const sal_Char* pcName, sal_Int32 nSize )
{
    OUStringBuffer aBuffer;
    for( sal_Int32 nIndex = 0; mxIn->isValidPos() && (nIndex < nSize); ++nIndex )
        aBuffer.append( static_cast< sal_Unicode >( mxIn->readValue< sal_uInt16 >() ) );
    OUString aString = aBuffer.makeStringAndClear();
    writeStringItem( pcName, aString );
    return aString;
}

double InputObjectBase::dumpRk( const sal_Char* pcName )
{
    sal_Int32 nRk;
    *mxIn >> nRk;
    return writeRkItem( pcName ? pcName : "rk-value", nRk );
}

OUString InputObjectBase::dumpGuid( const sal_Char* pcName )
{
    OUStringBuffer aBuffer;
    sal_uInt32 nData32;
    sal_uInt16 nData16;
    sal_uInt8 nData8;

    *mxIn >> nData32;
    StringHelper::appendHex( aBuffer, nData32, false );
    aBuffer.append( sal_Unicode( '-' ) );
    *mxIn >> nData16;
    StringHelper::appendHex( aBuffer, nData16, false );
    aBuffer.append( sal_Unicode( '-' ) );
    *mxIn >> nData16;
    StringHelper::appendHex( aBuffer, nData16, false );
    aBuffer.append( sal_Unicode( '-' ) );
    *mxIn >> nData8;
    StringHelper::appendHex( aBuffer, nData8, false );
    *mxIn >> nData8;
    StringHelper::appendHex( aBuffer, nData8, false );
    aBuffer.append( sal_Unicode( '-' ) );
    for( int nIndex = 0; nIndex < 6; ++nIndex )
    {
        *mxIn >> nData8;
        StringHelper::appendHex( aBuffer, nData8, false );
    }
    OUString aGuid = aBuffer.makeStringAndClear();
    writeGuidItem( pcName ? pcName : "guid", aGuid );
    return aGuid;
}

void InputObjectBase::dumpItem( const ItemFormat& rItemFmt )
{
    switch( rItemFmt.meDataType )
    {
        case DATATYPE_VOID:                                         break;
        case DATATYPE_INT8:    dumpValue< sal_Int8 >( rItemFmt );   break;
        case DATATYPE_UINT8:   dumpValue< sal_uInt8 >( rItemFmt );  break;
        case DATATYPE_INT16:   dumpValue< sal_Int16 >( rItemFmt );  break;
        case DATATYPE_UINT16:  dumpValue< sal_uInt16 >( rItemFmt ); break;
        case DATATYPE_INT32:   dumpValue< sal_Int32 >( rItemFmt );  break;
        case DATATYPE_UINT32:  dumpValue< sal_uInt32 >( rItemFmt ); break;
        case DATATYPE_INT64:   dumpValue< sal_Int64 >( rItemFmt );  break;
        case DATATYPE_UINT64:  dumpValue< sal_uInt64 >( rItemFmt ); break;
        case DATATYPE_FLOAT:   dumpValue< float >( rItemFmt );      break;
        case DATATYPE_DOUBLE:  dumpValue< double >( rItemFmt );     break;
        default:;
    }
}

// ============================================================================

InputStreamObject::InputStreamObject( const ObjectBase& rParent, const OUString& rOutFileName, BinaryInputStreamRef xStrm )
{
    construct( rParent, rOutFileName, xStrm );
}

InputStreamObject::~InputStreamObject()
{
}

void InputStreamObject::construct( const ObjectBase& rParent, const OUString& rOutFileName, BinaryInputStreamRef xStrm )
{
    mxStrm = xStrm;
    if( mxStrm.get() )
        InputObjectBase::construct( rParent, rOutFileName, InputRef( new BinaryInput( *mxStrm ) ) );
}

sal_Int64 InputStreamObject::getStreamSize() const
{
    return mxStrm.get() ? mxStrm->getLength() : -1;
}

void InputStreamObject::dumpBinaryStream( bool bShowOffset )
{
    in().seek( 0 );
    dumpRawBinary( getLimitedValue< sal_Int32, sal_Int64 >( in().getSize(), 0, SAL_MAX_INT32 ), bShowOffset, true );
    out().emptyLine();
}

bool InputStreamObject::implIsValid() const
{
    return mxStrm.get() && mxStrm->is() && InputObjectBase::implIsValid();
}

void InputStreamObject::implDump()
{
    dumpBinaryStream();
}

// ============================================================================
// ============================================================================

TextStreamObject::TextStreamObject( const ObjectBase& rParent,
        const OUString& rOutFileName, BinaryInputStreamRef xStrm, rtl_TextEncoding eTextEnc ) :
    InputStreamObject( rParent, rOutFileName, xStrm ),
    meTextEnc( eTextEnc )
{
}

void TextStreamObject::implDump()
{
    const sal_Char* pcTextEnc = rtl_getBestMimeCharsetFromTextEncoding( meTextEnc );
    OUString aEncoding = OUString::createFromAscii( pcTextEnc ? pcTextEnc : "UTF-8" );
    Reference< XTextInputStream > xTextStrm = InputOutputHelper::openTextInputStream( getStream().getXInputStream(), aEncoding );
    if( xTextStrm.is() ) try
    {
        sal_uInt32 nLine = 0;
        while( !xTextStrm->isEOF() )
            implDumpLine( xTextStrm->readLine(), ++nLine );
    }
    catch( Exception& )
    {
        writeInfoItem( "stream-state", OOX_DUMP_ERR_STREAM );
    }
    out().emptyLine();
}

void TextStreamObject::implDumpLine( const OUString& rLine, sal_uInt32 nLine )
{
    Output& rOut = out();
    TableGuard aTabGuard( rOut, 8 );
    rOut.writeDec( nLine, 6 );
    rOut.tab();
    rOut.writeString( rLine );
    rOut.newLine();
}

// ============================================================================

XmlStreamObject::XmlStreamObject( const ObjectBase& rParent, const OUString& rOutFileName, BinaryInputStreamRef xStrm ) :
    TextStreamObject( rParent, rOutFileName, xStrm, RTL_TEXTENCODING_UTF8 )
{
}

void XmlStreamObject::implDump()
{
    maIncompleteLine = OUString();
    TextStreamObject::implDump();
    if( maIncompleteLine.getLength() > 0 )
    {
        out().resetIndent();
        out().writeString( maIncompleteLine );
        out().emptyLine();
        writeInfoItem( "stream-state", OOX_DUMP_ERR_STREAM );
    }
}

void XmlStreamObject::implDumpLine( const OUString& rLine, sal_uInt32 )
{
    // build input line from cached incomplete element and new text data
    OUStringBuffer aLine;
    if( maIncompleteLine.getLength() > 0 )
        aLine.append( maIncompleteLine ).append( sal_Unicode( ' ' ) );
    aLine.append( rLine );
    maIncompleteLine = OUString();

    Output& rOut = out();
    if( aLine.getLength() == 0 )
    {
        rOut.newLine();
        return;
    }

    const sal_Unicode* pcPos = aLine.getStr();
    const sal_Unicode* pcEnd = pcPos + aLine.getLength();
    while( pcPos < pcEnd )
    {
        OUStringBuffer aOutLine;
        bool bIsStartElement = false;
        bool bIsComplElement = false;
        bool bIsEndElement = false;

        /*  check for start element at beginning of the line - pcEnd and thus (pcPos+1)
            are dereferenceable, because OUStringBuffer::getStr is null-terminated. */
        if( (*pcPos == '<') && (*(pcPos + 1) != '/') )
        {
            const sal_Unicode* pcElementEnd = ::std::find( pcPos, pcEnd, '>' );
            if( pcElementEnd == pcEnd )
            {
                // incomplete start element
                maIncompleteLine = OUString( pcPos, static_cast< sal_Int32 >( pcEnd - pcPos ) );
                pcPos = pcEnd;
            }
            else
            {
                bIsComplElement = (*(pcPos + 1) == '?') || (*(pcElementEnd - 1) == '/');
                bIsStartElement = !bIsComplElement;
                ++pcElementEnd;
                aOutLine.append( pcPos, static_cast< sal_Int32 >( pcElementEnd - pcPos ) );
                pcPos = pcElementEnd;
            }
        }

        // check for following element text
        if( !bIsComplElement && (pcPos < pcEnd) )
        {
            const sal_Unicode* pcElementStart = ::std::find( pcPos, pcEnd, '<' );
            // append text between elements
            if( pcPos < pcElementStart )
            {
                OUString aText( pcPos, static_cast< sal_Int32 >( pcElementStart - pcPos ) );
                if( aText.trim().getLength() > 0 )
                    aOutLine.append( aText );
                pcPos = pcElementStart;
            }
        }

        // check for stand-alone or following end element
        if( !bIsComplElement && (pcPos < pcEnd) && (*(pcPos + 1) == '/') )
        {
            const sal_Unicode* pcElementEnd = ::std::find( pcPos, pcEnd, '>' );
            if( pcElementEnd == pcEnd )
            {
                // incomplete end element
                aOutLine.append( pcPos, static_cast< sal_Int32 >( pcEnd - pcPos ) );
                maIncompleteLine = aOutLine.makeStringAndClear();
                pcPos = pcEnd;
            }
            else
            {
                bIsEndElement = true;
                ++pcElementEnd;
                aOutLine.append( pcPos, static_cast< sal_Int32 >( pcElementEnd - pcPos ) );
                pcPos = pcElementEnd;
            }
        }

        // flush output line
        if( maIncompleteLine.getLength() == 0 )
        {
            if( !bIsStartElement && bIsEndElement ) rOut.decIndent();
            rOut.writeString( aOutLine.makeStringAndClear() );
            rOut.newLine();
            if( bIsStartElement && !bIsEndElement ) rOut.incIndent();
        }
    }
}

// ============================================================================
// ============================================================================

RecordHeaderImplBase::~RecordHeaderImplBase()
{
}

void RecordHeaderImplBase::construct( const InputObjectBase& rParent, const RecordHeaderConfigInfo& rCfgInfo )
{
    InputObjectBase::construct( rParent );
    if( InputObjectBase::implIsValid() )
    {
        const Config& rCfg = cfg();
        mpcTitle      = rCfgInfo.mpcTitle;
        mxRecNames    = rCfg.getNameList( rCfgInfo.mpcRecNames );
        mbShowRecPos  = rCfg.getBoolOption( rCfgInfo.mpcShowRecPos,  true );
        mbShowRecSize = rCfg.getBoolOption( rCfgInfo.mpcShowRecSize, true );
        mbShowRecId   = rCfg.getBoolOption( rCfgInfo.mpcShowRecId,   true );
        mbShowRecName = rCfg.getBoolOption( rCfgInfo.mpcShowRecName, true );
        mbShowRecBody = rCfg.getBoolOption( rCfgInfo.mpcShowRecBody, true );
    }
}

bool RecordHeaderImplBase::implIsValid() const
{
    return isValid( mxRecNames ) && InputObjectBase::implIsValid();
}

// ============================================================================
// ============================================================================

DumperBase::~DumperBase()
{
}

bool DumperBase::isImportEnabled() const
{
    return !isValid() || cfg().isImportEnabled();
}

StorageRef DumperBase::getRootStorage() const
{
    return getFilter().getStorage();
}

BinaryInputStreamRef DumperBase::getRootStream() const
{
    BinaryInputStreamRef xStrm;
    if( StorageBase* pStrg = getRootStorage().get() )
        xStrm.reset( new BinaryInputStream( pStrg->openInputStream( OUString() ), false ) );
    return xStrm;
}

void DumperBase::construct( const FilterBase& rFilter, ConfigRef xConfig )
{
    if( rFilter.isImportFilter() && isValid( xConfig ) && xConfig->isDumperEnabled() )
        ObjectBase::construct( rFilter, xConfig );
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#endif

