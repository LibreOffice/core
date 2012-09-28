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

#include "oox/dump/dumperbase.hxx"

#include <algorithm>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <osl/file.hxx>
#include <rtl/math.hxx>
#include <rtl/tencinfo.h>
#include "oox/core/filterbase.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/textinputstream.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

using ::comphelper::MediaDescriptor;
using ::oox::core::FilterBase;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;

// ============================================================================

namespace {

const sal_Unicode OOX_DUMP_BOM          = 0xFEFF;
const sal_Int32 OOX_DUMP_MAXSTRLEN      = 80;
const sal_Int32 OOX_DUMP_INDENT         = 2;
const sal_Unicode OOX_DUMP_BINDOT       = '.';
const sal_Unicode OOX_DUMP_CFG_LISTSEP  = ',';
const sal_Unicode OOX_DUMP_CFG_QUOTE    = '\'';
const sal_Unicode OOX_DUMP_LF           = '\n';
const sal_Unicode OOX_DUMP_ITEMSEP      = '=';
const sal_Int32 OOX_DUMP_BYTESPERLINE   = 16;
const sal_Int64 OOX_DUMP_MAXARRAY       = 16;

} // namespace

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

// input streams --------------------------------------------------------------

Reference< XInputStream > InputOutputHelper::openInputStream(
        const Reference< XComponentContext >& rxContext, const OUString& rFileName )
{
    Reference< XInputStream > xInStrm;
    if( rxContext.is() ) try
    {
        Reference<XSimpleFileAccess2> xFileAccess(SimpleFileAccess::create(rxContext));
        xInStrm = xFileAccess->openFileRead( rFileName );
    }
    catch( Exception& )
    {
    }
    return xInStrm;
}

// output streams -------------------------------------------------------------

Reference< XOutputStream > InputOutputHelper::openOutputStream(
        const Reference< XComponentContext >& rxContext, const OUString& rFileName )
{
    Reference< XOutputStream > xOutStrm;
    if( rxContext.is() ) try
    {
        Reference<XSimpleFileAccess2> xFileAccess(SimpleFileAccess::create(rxContext));
        xOutStrm = xFileAccess->openFileWrite( rFileName );
    }
    catch( Exception& )
    {
    }
    return xOutStrm;
}

Reference< XTextOutputStream > InputOutputHelper::openTextOutputStream(
        const Reference< XComponentContext >& rxContext, const Reference< XOutputStream >& rxOutStrm, rtl_TextEncoding eTextEnc )
{
    Reference< XTextOutputStream > xTextOutStrm;
    const char* pcCharset = rtl_getMimeCharsetFromTextEncoding( eTextEnc );
    if( rxContext.is() && rxOutStrm.is() && pcCharset ) try
    {
        Reference< XMultiServiceFactory > xFactory( rxContext->getServiceManager(), UNO_QUERY_THROW );
        Reference< XActiveDataSource > xDataSource( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TextOutputStream" ) ), UNO_QUERY_THROW );
        xDataSource->setOutputStream( rxOutStrm );
        xTextOutStrm.set( xDataSource, UNO_QUERY_THROW );
        xTextOutStrm->setEncoding( OUString::createFromAscii( pcCharset ) );
    }
    catch( Exception& )
    {
    }
    return xTextOutStrm;
}

Reference< XTextOutputStream > InputOutputHelper::openTextOutputStream(
        const Reference< XComponentContext >& rxContext, const OUString& rFileName, rtl_TextEncoding eTextEnc )
{
    return openTextOutputStream( rxContext, openOutputStream( rxContext, rFileName ), eTextEnc );
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
        if ( aFmtType == "unused" )
            set( meDataType, FORMATTYPE_HEX, CREATE_OUSTRING( OOX_DUMP_UNUSED ) );
        else if ( aFmtType == "unknown" )
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

void StringHelper::appendChar( OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount )
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

static sal_uInt64
lcl_ConvertDouble(double const f)
{
    sal_uInt64 i = sal_uInt64();
    for (size_t j = 0; j < sizeof(double); ++j)
    {   // hopefully both endian independent and strict aliasing safe
        reinterpret_cast<char *>(&i)[j] = reinterpret_cast<char const *>(&f)[j];
    }
    return i;
}

void StringHelper::appendHex( OUStringBuffer& rStr, double fData, bool bPrefix )
{
    appendHex( rStr, lcl_ConvertDouble(fData), bPrefix );
}

// append shortened hexadecimal -----------------------------------------------

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt8 nData, bool bPrefix )
{
    appendHex( rStr, nData, bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int8 nData, bool bPrefix )
{
    appendHex( rStr, nData, bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix )
{
    if( nData > SAL_MAX_UINT8 )
        appendHex( rStr, nData, bPrefix );
    else
        appendHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int16 nData, bool bPrefix )
{
    appendShortHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix )
{
    if( nData > SAL_MAX_UINT16 )
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
    if( nData > SAL_MAX_UINT32 )
        appendHex( rStr, nData, bPrefix );
    else
        appendShortHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, sal_Int64 nData, bool bPrefix )
{
    appendShortHex( rStr, static_cast< sal_uInt64 >( nData ), bPrefix );
}

void StringHelper::appendShortHex( OUStringBuffer& rStr, double fData, bool bPrefix )
{
    appendHex( rStr, fData, bPrefix );
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
    appendBin( rStr, lcl_ConvertDouble(fData), bDots );
}

// append formatted value -----------------------------------------------------

void StringHelper::appendBool( OUStringBuffer& rStr, bool bData )
{
    rStr.appendAscii( bData ? "true" : "false" );
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
        OUString aCodeStr = aCode.makeStringAndClear();
        for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
            rStr.append( aCodeStr );
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
    if( (rStr.getLength() > 0) && (!rToken.isEmpty()) )
        rStr.append( cSep );
    rStr.append( rToken );
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

OUString StringHelper::trimTrailingNul( const OUString& rStr )
{
    sal_Int32 nLastPos = rStr.getLength() - 1;
    if( (nLastPos >= 0) && (rStr[ nLastPos ] == 0) )
        return rStr.copy( 0, nLastPos );
    return rStr;
}

OString StringHelper::convertToUtf8( const OUString& rStr )
{
    return OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 );
}

DataType StringHelper::convertToDataType( const OUString& rStr )
{
    DataType eType = DATATYPE_VOID;
    if ( rStr == "int8" )
        eType = DATATYPE_INT8;
    else if ( rStr == "uint8" )
        eType = DATATYPE_UINT8;
    else if ( rStr == "int16" )
        eType = DATATYPE_INT16;
    else if ( rStr == "uint16" )
        eType = DATATYPE_UINT16;
    else if ( rStr == "int32" )
        eType = DATATYPE_INT32;
    else if ( rStr == "uint32" )
        eType = DATATYPE_UINT32;
    else if ( rStr == "int64" )
        eType = DATATYPE_INT64;
    else if ( rStr == "uint64" )
        eType = DATATYPE_UINT64;
    else if ( rStr == "float" )
        eType = DATATYPE_FLOAT;
    else if ( rStr == "double" )
        eType = DATATYPE_DOUBLE;
    return eType;
}

FormatType StringHelper::convertToFormatType( const OUString& rStr )
{
    FormatType eType = FORMATTYPE_NONE;
    if ( rStr == "dec" )
        eType = FORMATTYPE_DEC;
    else if ( rStr == "hex" )
        eType = FORMATTYPE_HEX;
    else if ( rStr == "shorthex" )
        eType = FORMATTYPE_SHORTHEX;
    else if ( rStr == "bin" )
        eType = FORMATTYPE_BIN;
    else if ( rStr == "fix" )
        eType = FORMATTYPE_FIX;
    else if ( rStr == "bool" )
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
    if ( rData == "true" )
        return true;
    if ( rData == "false" )
        return false;
    sal_Int64 nData;
    return convertStringToInt( nData, rData ) && (nData != 0);
}

OUStringPair StringHelper::convertStringToPair( const OUString& rString, sal_Unicode cSep )
{
    OUStringPair aPair;
    if( !rString.isEmpty() )
    {
        sal_Int32 nEqPos = rString.indexOf( cSep );
        if( nEqPos < 0 )
        {
            aPair.first = rString;
        }
        else
        {
            aPair.first = StringHelper::trimSpaces( rString.copy( 0, nEqPos ) );
            aPair.second = StringHelper::trimSpaces( rString.copy( nEqPos + 1 ) );
        }
    }
    return aPair;
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
        if( !bIgnoreEmpty || !aToken.isEmpty() )
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

Base::~Base()
{
}

// ============================================================================
// ============================================================================

ConfigItemBase::~ConfigItemBase()
{
}

void ConfigItemBase::readConfigBlock( TextInputStream& rStrm )
{
    readConfigBlockContents( rStrm );
}

void ConfigItemBase::implProcessConfigItemStr(
        TextInputStream& /*rStrm*/, const OUString& /*rKey*/, const OUString& /*rData*/ )
{
}

void ConfigItemBase::implProcessConfigItemInt(
        TextInputStream& /*rStrm*/, sal_Int64 /*nKey*/, const OUString& /*rData*/ )
{
}

void ConfigItemBase::readConfigBlockContents( TextInputStream& rStrm )
{
    bool bLoop = true;
    while( bLoop && !rStrm.isEof() )
    {
        OUString aKey, aData;
        switch( readConfigLine( rStrm, aKey, aData ) )
        {
            case LINETYPE_DATA:
                processConfigItem( rStrm, aKey, aData );
            break;
            case LINETYPE_END:
                bLoop = false;
            break;
        }
    }
}

ConfigItemBase::LineType ConfigItemBase::readConfigLine(
        TextInputStream& rStrm, OUString& orKey, OUString& orData ) const
{
    OUString aLine;
    while( !rStrm.isEof() && aLine.isEmpty() )
    {
        aLine = rStrm.readLine();
        if( !aLine.isEmpty() && (aLine[ 0 ] == OOX_DUMP_BOM) )
            aLine = aLine.copy( 1 );
        aLine = StringHelper::trimSpaces( aLine );
        if( !aLine.isEmpty() )
        {
            // ignore comments (starting with hash or semicolon)
            sal_Unicode cChar = aLine[ 0 ];
            if( (cChar == '#') || (cChar == ';') )
                aLine = OUString();
        }
    }

    OUStringPair aPair = StringHelper::convertStringToPair( aLine );
    orKey = aPair.first;
    orData = aPair.second;
    return ( !orKey.isEmpty() && (!orData.isEmpty() || orKey != "end" )) ?
        LINETYPE_DATA : LINETYPE_END;
}

void ConfigItemBase::processConfigItem(
        TextInputStream& rStrm, const OUString& rKey, const OUString& rData )
{
    sal_Int64 nKey;
    if( StringHelper::convertStringToInt( nKey, rKey ) )
        implProcessConfigItemInt( rStrm, nKey, rData );
    else
        implProcessConfigItemStr( rStrm, rKey, rData );
}

// ============================================================================

NameListBase::~NameListBase()
{
}

void NameListBase::setName( sal_Int64 nKey, const String& rName )
{
    implSetName( nKey, rName );
}

void NameListBase::includeList( const NameListRef& rxList )
{
    if( rxList.get() )
    {
        for( const_iterator aIt = rxList->begin(), aEnd = rxList->end(); aIt != aEnd; ++aIt )
            maMap[ aIt->first ] = aIt->second;
        implIncludeList( *rxList );
    }
}

bool NameListBase::implIsValid() const
{
    return true;
}

void NameListBase::implProcessConfigItemStr(
        TextInputStream& rStrm, const OUString& rKey, const OUString& rData )
{
    if ( rKey == "include" )
        include( rData );
    else if ( rKey == "exclude" )
        exclude( rData );
    else
        ConfigItemBase::implProcessConfigItemStr( rStrm, rKey, rData );
}

void NameListBase::implProcessConfigItemInt(
        TextInputStream& /*rStrm*/, sal_Int64 nKey, const OUString& rData )
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

void ItemFormatMap::insertFormats( const NameListRef& rxNameList )
{
    if( Base::isValid( rxNameList ) )
        for( NameListBase::const_iterator aIt = rxNameList->begin(), aEnd = rxNameList->end(); aIt != aEnd; ++aIt )
            (*this)[ aIt->first ].parse( aIt->second );
}

// ============================================================================

ConstList::ConstList( const SharedConfigData& rCfgData ) :
    NameListBase( rCfgData ),
    maDefName( OOX_DUMP_ERR_NONAME ),
    mbQuoteNames( false )
{
}

void ConstList::implProcessConfigItemStr(
        TextInputStream& rStrm, const OUString& rKey, const OUString& rData )
{
    if ( rKey == "default" )
        setDefaultName( rData );
    else if ( rKey == "quote-names" )
        setQuoteNames( StringHelper::convertStringToBool( rData ) );
    else
        NameListBase::implProcessConfigItemStr( rStrm, rKey, rData );
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
        if( !mbIgnoreEmpty || !aIt->isEmpty() )
            insertRawName( nKey, *aIt );
}

void MultiList::implProcessConfigItemStr(
        TextInputStream& rStrm, const OUString& rKey, const OUString& rData )
{
    if ( rKey == "ignore-empty" )
        mbIgnoreEmpty = StringHelper::convertStringToBool( rData );
    else
        ConstList::implProcessConfigItemStr( rStrm, rKey, rData );
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
        TextInputStream& rStrm, const OUString& rKey, const OUString& rData )
{
    if ( rKey == "ignore" )
    {
        sal_Int64 nIgnore;
        if( StringHelper::convertStringToInt( nIgnore, rData ) )
            setIgnoreFlags( nIgnore );
    }
    else
    {
        NameListBase::implProcessConfigItemStr( rStrm, rKey, rData );
    }
}

void FlagsList::implSetName( sal_Int64 nKey, const OUString& rName )
{
    if( (nKey != 0) && ((nKey & (nKey - 1)) == 0) )  // only a single bit set?
        insertRawName( nKey, rName );
}

OUString FlagsList::implGetName( const Config& /*rCfg*/, sal_Int64 nKey ) const
{
    sal_Int64 nFound = mnIgnore;
    OUStringBuffer aName;
    // add known flags
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        sal_Int64 nMask = aIt->first;
        setFlag( nFound, nMask );
        if( !getFlag( mnIgnore, nMask ) )
        {
            const OUString& rFlagName = aIt->second;
            bool bOnOff = !rFlagName.isEmpty() && rFlagName[ 0 ] == ':';
            bool bFlag = getFlag( nKey, nMask );
            if( bOnOff )
            {
                StringHelper::appendToken( aName, rFlagName.copy( 1 ) );
                aName.appendAscii( bFlag ? ":on" : ":off" );
            }
            else
            {
                bool bNegated = !rFlagName.isEmpty() && rFlagName[ 0 ] == '!';
                sal_Int32 nBothSep = bNegated ? rFlagName.indexOf( '!', 1 ) : -1;
                if( bFlag )
                {
                    if( !bNegated )
                        StringHelper::appendToken( aName, rFlagName );
                    else if( nBothSep > 0 )
                        StringHelper::appendToken( aName, rFlagName.copy( nBothSep + 1 ) );
                }
                else if( bNegated )
                {
                    if( nBothSep > 0 )
                        StringHelper::appendToken( aName, rFlagName.copy( 1, nBothSep - 1 ) );
                    else
                        StringHelper::appendToken( aName, rFlagName.copy( 1 ) );
                }
            }
        }
    }
    // add unknown flags
    setFlag( nKey, nFound, false );
    if( nKey != 0 )
    {
        OUStringBuffer aUnknown( CREATE_OUSTRING( OOX_DUMP_UNKNOWN ) );
        aUnknown.append( OOX_DUMP_ITEMSEP );
        StringHelper::appendShortHex( aUnknown, nKey, true );
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

bool CombiList::ExtItemFormatKey::operator<( const ExtItemFormatKey& rRight ) const
{
    return (mnKey < rRight.mnKey) || ((mnKey == rRight.mnKey) && (maFilter < rRight.maFilter));
}

CombiList::CombiList( const SharedConfigData& rCfgData ) :
    FlagsList( rCfgData )
{
}

void CombiList::implSetName( sal_Int64 nKey, const OUString& rName )
{
    if( (nKey & (nKey - 1)) != 0 )  // more than a single bit set?
    {
        typedef ::std::set< ExtItemFormatKey > ExtItemFormatKeySet;
        ::std::set< ExtItemFormatKey > aItemKeys;
        ExtItemFormat aItemFmt;
        OUStringVector aRemain = aItemFmt.parse( rName );
        for( OUStringVector::iterator aIt = aRemain.begin(), aEnd = aRemain.end(); aIt != aEnd; ++aIt )
        {
            OUStringPair aPair = StringHelper::convertStringToPair( *aIt );
            if ( aPair.first == "noshift" )
            {
                aItemFmt.mbShiftValue = StringHelper::convertStringToBool( aPair.second );
            }
            else if ( aPair.first == "filter" )
            {
                OUStringPair aFilter = StringHelper::convertStringToPair( aPair.second, '~' );
                ExtItemFormatKey aKey( nKey );
                if( !aFilter.first.isEmpty() && StringHelper::convertStringToInt( aKey.maFilter.first, aFilter.first ) &&
                    !aFilter.second.isEmpty() && StringHelper::convertStringToInt( aKey.maFilter.second, aFilter.second ) )
                {
                    if( aKey.maFilter.first == 0 )
                        aKey.maFilter.second = 0;
                    aItemKeys.insert( aKey );
                }
            }
        }
        if( aItemKeys.empty() )
            aItemKeys.insert( ExtItemFormatKey( nKey ) );
        for( ExtItemFormatKeySet::iterator aIt = aItemKeys.begin(), aEnd = aItemKeys.end(); aIt != aEnd; ++aIt )
            maFmtMap[ *aIt ] = aItemFmt;
    }
    else
    {
        FlagsList::implSetName( nKey, rName );
    }
}

OUString CombiList::implGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    sal_Int64 nFound = 0;
    OUStringBuffer aName;
    // add known flag fields
    for( ExtItemFormatMap::const_iterator aIt = maFmtMap.begin(), aEnd = maFmtMap.end(); aIt != aEnd; ++aIt )
    {
        const ExtItemFormatKey& rMapKey = aIt->first;
        sal_Int64 nMask = rMapKey.mnKey;
        if( (nMask != 0) && ((nKey & rMapKey.maFilter.first) == rMapKey.maFilter.second) )
        {
            const ExtItemFormat& rItemFmt = aIt->second;

            sal_uInt64 nUFlags = static_cast< sal_uInt64 >( nKey );
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
            if( !rItemFmt.maListName.isEmpty() )
            {
                OUString aValueName = rCfg.getName( rItemFmt.maListName, static_cast< sal_Int64 >( nUValue ) );
                StringHelper::appendToken( aItem, aValueName, OOX_DUMP_ITEMSEP );
            }
            StringHelper::enclose( aItem, '(', ')' );
            StringHelper::appendToken( aName, aItem.makeStringAndClear() );
            setFlag( nFound, nMask );
        }
    }
    setFlag( nKey, nFound, false );
    StringHelper::appendToken( aName, FlagsList::implGetName( rCfg, nKey ) );
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
    return mxList.get() ? mxList : (mxList = rCfg.getNameList( maName ));
}

// ============================================================================
// ============================================================================

SharedConfigData::SharedConfigData( const OUString& rFileName,
        const Reference< XComponentContext >& rxContext, const StorageRef& rxRootStrg,
        const OUString& rSysFileName ) :
    mxContext( rxContext ),
    mxRootStrg( rxRootStrg ),
    maSysFileName( rSysFileName ),
    mbLoaded( false ),
    mbPwCancelled( false )
{
    OUString aFileUrl = InputOutputHelper::convertFileNameToUrl( rFileName );
    if( !aFileUrl.isEmpty() )
    {
        sal_Int32 nNamePos = InputOutputHelper::getFileNamePos( aFileUrl );
        maConfigPath = aFileUrl.copy( 0, nNamePos );
        mbLoaded = readConfigFile( aFileUrl );
    }
}

SharedConfigData::~SharedConfigData()
{
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

void SharedConfigData::setNameList( const OUString& rListName, const NameListRef& rxList )
{
    if( !rListName.isEmpty() )
        maNameLists[ rListName ] = rxList;
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
    return mbLoaded && mxContext.is() && mxRootStrg.get() && !maSysFileName.isEmpty();
}

void SharedConfigData::implProcessConfigItemStr(
        TextInputStream& rStrm, const OUString& rKey, const OUString& rData )
{
    if ( rKey == "include-config-file" )
        readConfigFile( maConfigPath + rData );
    else if ( rKey == "constlist" )
        readNameList< ConstList >( rStrm, rData );
    else if ( rKey == "multilist" )
        readNameList< MultiList >( rStrm, rData );
    else if ( rKey == "flagslist" )
        readNameList< FlagsList >( rStrm, rData );
    else if ( rKey == "combilist" )
        readNameList< CombiList >( rStrm, rData );
    else if ( rKey == "shortlist" )
        createShortList( rData );
    else if ( rKey == "unitconverter" )
        createUnitConverter( rData );
    else
        setOption( rKey, rData );
}

bool SharedConfigData::readConfigFile( const OUString& rFileUrl )
{
    bool bLoaded = maConfigFiles.count( rFileUrl ) > 0;
    if( !bLoaded )
    {
        Reference< XInputStream > xInStrm = InputOutputHelper::openInputStream( mxContext, rFileUrl );
        TextInputStream aTxtStrm( mxContext, xInStrm, RTL_TEXTENCODING_UTF8 );
        if( !aTxtStrm.isEof() )
        {
            maConfigFiles.insert( rFileUrl );
            readConfigBlockContents( aTxtStrm );
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
        bool bRecip = !aFactor.isEmpty() && aFactor[ 0 ] == '/';
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

Config::Config( const sal_Char* pcEnvVar, const FilterBase& rFilter )
{
    construct( pcEnvVar, rFilter );
}

Config::Config( const sal_Char* pcEnvVar, const Reference< XComponentContext >& rxContext, const StorageRef& rxRootStrg, const OUString& rSysFileName )
{
    construct( pcEnvVar, rxContext, rxRootStrg, rSysFileName );
}

Config::~Config()
{
}

void Config::construct( const Config& rParent )
{
    *this = rParent;
}

void Config::construct( const sal_Char* pcEnvVar, const FilterBase& rFilter )
{
    if( !rFilter.getFileUrl().isEmpty() )
        construct( pcEnvVar, rFilter.getComponentContext(), rFilter.getStorage(), rFilter.getFileUrl() );
}

void Config::construct( const sal_Char* pcEnvVar, const Reference< XComponentContext >& rxContext, const StorageRef& rxRootStrg, const OUString& rSysFileName )
{
    if( pcEnvVar && rxRootStrg.get() && !rSysFileName.isEmpty() )
        if( const sal_Char* pcFileName = ::getenv( pcEnvVar ) )
            mxCfgData.reset( new SharedConfigData( OUString::createFromAscii( pcFileName ), rxContext, rxRootStrg, rSysFileName ) );
}

const OUString& Config::getStringOption( const String& rKey, const OUString& rDefault ) const
{
    const OUString* pData = implGetOption( rKey );
    return pData ? *pData : rDefault;
}

bool Config::getBoolOption( const String& rKey, bool bDefault ) const
{
    const OUString* pData = implGetOption( rKey );
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

void Config::eraseNameList( const String& rListName )
{
    mxCfgData->eraseNameList( rListName );
}

NameListRef Config::getNameList( const String& rListName ) const
{
    return implGetNameList( rListName );
}

bool Config::isPasswordCancelled() const
{
    return mxCfgData->isPasswordCancelled();
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

Output::Output( const Reference< XComponentContext >& rxContext, const OUString& rFileName ) :
    mxStrm( InputOutputHelper::openTextOutputStream( rxContext, rFileName, RTL_TEXTENCODING_UTF8 ) ),
    mnCol( 0 ),
    mnItemLevel( 0 ),
    mnMultiLevel( 0 ),
    mnItemIdx( 0 ),
    mnLastItem( 0 )
{
    if( mxStrm.is() )
        mxStrm->writeString( OUString( OOX_DUMP_BOM ) );
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

void Output::startItem( const String& rItemName )
{
    if( mnItemLevel == 0 )
    {
        if( (mnMultiLevel > 0) && (maLine.getLength() > 0) )
            tab();
        if( rItemName.has() )
        {
            writeItemName( rItemName );
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
        if( maLastItem.isEmpty() && mnLastItem > 0 && maLine[ mnLastItem - 1 ] == OOX_DUMP_ITEMSEP )
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

// ----------------------------------------------------------------------------

bool Output::implIsValid() const
{
    return mxStrm.is();
}

void Output::writeItemName( const String& rItemName )
{
    if( rItemName.has() && (rItemName[ 0 ] == '#') )
    {
        writeString( rItemName.copy( 1 ) );
        StringHelper::appendIndex( maLine, mnItemIdx++ );
    }
    else
        writeString( rItemName );
}

// ============================================================================

StorageIterator::StorageIterator( const StorageRef& rxStrg ) :
    mxStrg( rxStrg )
{
    if( mxStrg.get() )
        mxStrg->getElementNames( maNames );
    maIt = maNames.begin();
}

StorageIterator::~StorageIterator()
{
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

ObjectBase::~ObjectBase()
{
}

void ObjectBase::construct( const ConfigRef& rxConfig )
{
    mxConfig = rxConfig;
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
    return isValid( mxConfig );
}

void ObjectBase::implDump()
{
}

// ============================================================================
// ============================================================================

void StorageObjectBase::construct( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath )
{
    ObjectBase::construct( rParent );
    mxStrg = rxStrg;
    maSysPath = rSysPath;
}

void StorageObjectBase::construct( const ObjectBase& rParent )
{
    ObjectBase::construct( rParent );
    if( ObjectBase::implIsValid() )
    {
        mxStrg = cfg().getRootStorage();
        maSysPath = cfg().getSysFileName();
    }
}

bool StorageObjectBase::implIsValid() const
{
    return mxStrg.get() && !maSysPath.isEmpty() && ObjectBase::implIsValid();
}

void StorageObjectBase::implDump()
{
    bool bIsStrg = mxStrg->isStorage();
    bool bIsRoot = mxStrg->isRootStorage();
    Reference< XInputStream > xBaseStrm;
    if( !bIsStrg )
        xBaseStrm = mxStrg->openInputStream( OUString() );

    OUString aSysOutPath = maSysPath;
    if( bIsRoot ) try
    {
        aSysOutPath += OOX_DUMP_DUMPEXT;
        Reference<XSimpleFileAccess2> xFileAccess(SimpleFileAccess::create(getContext()));
        xFileAccess->kill( aSysOutPath );
    }
    catch( Exception& )
    {
    }

    if( bIsStrg )
    {
        extractStorage( mxStrg, OUString(), aSysOutPath );
    }
    else if( xBaseStrm.is() )
    {
        BinaryInputStreamRef xInStrm( new BinaryXInputStream( xBaseStrm, false ) );
        xInStrm->seekToStart();
        implDumpBaseStream( xInStrm, aSysOutPath );
    }
}

void StorageObjectBase::implDumpStream( const Reference< XInputStream >&, const OUString&, const OUString&, const OUString& )
{
}

void StorageObjectBase::implDumpStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    extractStorage( rxStrg, rStrgPath, rSysPath );
}

void StorageObjectBase::implDumpBaseStream( const BinaryInputStreamRef&, const OUString& )
{
}

void StorageObjectBase::addPreferredStream( const String& rStrmName )
{
    if( rStrmName.has() )
        maPreferred.push_back( PreferredItem( rStrmName, false ) );
}

void StorageObjectBase::addPreferredStorage( const String& rStrgPath )
{
    if( rStrgPath.has() )
        maPreferred.push_back( PreferredItem( rStrgPath, true ) );
}

OUString StorageObjectBase::getSysFileName( const OUString& rStrmName, const OUString& rSysOutPath )
{
    // encode all characters < 0x20
    OUStringBuffer aBuffer;
    StringHelper::appendEncString( aBuffer, rStrmName, false );

    // replace all characters reserved in file system
    OUString aFileName = aBuffer.makeStringAndClear();
    static const sal_Unicode spcReserved[] = { '/', '\\', ':', '*', '?', '<', '>', '|' };
    for( const sal_Unicode* pcChar = spcReserved; pcChar < STATIC_ARRAY_END( spcReserved ); ++pcChar )
        aFileName = aFileName.replace( *pcChar, '_' );

    // build full path
    return rSysOutPath + OUString( sal_Unicode( '/' ) ) + aFileName;
}

void StorageObjectBase::extractStream( StorageBase& rStrg, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    BinaryXInputStream aInStrm( rStrg.openInputStream( rStrmName ), true );
    if( !aInStrm.isEof() )
    {
        BinaryXOutputStream aOutStrm( InputOutputHelper::openOutputStream( getContext(), rSysFileName ), true );
        if( !aOutStrm.isEof() )
            aInStrm.copyToStream( aOutStrm );
    }
    Reference< XInputStream > xDumpStrm = InputOutputHelper::openInputStream( getContext(), rSysFileName );
    if( xDumpStrm.is() )
        implDumpStream( xDumpStrm, rStrgPath, rStrmName, rSysFileName );
}

void StorageObjectBase::extractStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    // create directory in file system
    ::osl::FileBase::RC eRes = ::osl::Directory::create( rSysPath );
    if( (eRes != ::osl::FileBase::E_None) && (eRes != ::osl::FileBase::E_EXIST) )
        return;

    // process preferred storages and streams in root storage first
    if( rStrgPath.isEmpty() )
        for( PreferredItemVector::iterator aIt = maPreferred.begin(), aEnd = maPreferred.end(); aIt != aEnd; ++aIt )
            extractItem( rxStrg, rStrgPath, aIt->maName, rSysPath, aIt->mbStorage, !aIt->mbStorage );

    // process children of the storage
    for( StorageIterator aIt( rxStrg ); aIt.isValid(); ++aIt )
    {
        // skip processed preferred items
        OUString aItemName = aIt.getName();
        bool bFound = false;
        if( rStrgPath.isEmpty() )
            for( PreferredItemVector::iterator aIIt = maPreferred.begin(), aIEnd = maPreferred.end(); !bFound && (aIIt != aIEnd); ++aIIt )
                bFound = aIIt->maName == aItemName;
        if( !bFound )
            extractItem( rxStrg, rStrgPath, aItemName, rSysPath, aIt.isStorage(), aIt.isStream() );
    }
}

void StorageObjectBase::extractItem( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rItemName, const OUString& rSysPath, bool bIsStrg, bool bIsStrm )
{
    OUString aSysFileName = getSysFileName( rItemName, rSysPath );
    if( bIsStrg )
    {
        OUStringBuffer aStrgPath( rStrgPath );
        StringHelper::appendToken( aStrgPath, rItemName, '/' );
        implDumpStorage( rxStrg->openSubStorage( rItemName, false ), aStrgPath.makeStringAndClear(), aSysFileName );
    }
    else if( bIsStrm )
    {
        extractStream( *rxStrg, rStrgPath, rItemName, aSysFileName );
    }
}

// ============================================================================
// ============================================================================

OutputObjectBase::~OutputObjectBase()
{
}

void OutputObjectBase::construct( const ObjectBase& rParent, const OUString& rSysFileName )
{
    ObjectBase::construct( rParent );
    if( ObjectBase::implIsValid() )
    {
        maSysFileName = rSysFileName;
        mxOut.reset( new Output( getContext(), rSysFileName + OOX_DUMP_DUMPEXT ) );
    }
}

void OutputObjectBase::construct( const OutputObjectBase& rParent )
{
    *this = rParent;
}

bool OutputObjectBase::implIsValid() const
{
    return isValid( mxOut ) && ObjectBase::implIsValid();
}

void OutputObjectBase::writeEmptyItem( const String& rName )
{
    ItemGuard aItem( mxOut, rName );
}

void OutputObjectBase::writeInfoItem( const String& rName, const String& rData )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeString( rData );
}

void OutputObjectBase::writeCharItem( const String& rName, sal_Unicode cData )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeChar( OOX_DUMP_STRQUOTE );
    mxOut->writeChar( cData );
    mxOut->writeChar( OOX_DUMP_STRQUOTE );
}

void OutputObjectBase::writeStringItem( const String& rName, const OUString& rData )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeAscii( "(len=" );
    mxOut->writeDec( rData.getLength() );
    mxOut->writeAscii( ")," );
    OUStringBuffer aValue( rData.copy( 0, ::std::min( rData.getLength(), OOX_DUMP_MAXSTRLEN ) ) );
    StringHelper::enclose( aValue, OOX_DUMP_STRQUOTE );
    mxOut->writeString( aValue.makeStringAndClear() );
    if( rData.getLength() > OOX_DUMP_MAXSTRLEN )
        mxOut->writeAscii( ",cut" );
}

void OutputObjectBase::writeArrayItem( const String& rName, const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeArray( pnData, nSize, cSep );
}

void OutputObjectBase::writeDateTimeItem( const String& rName, const DateTime& rDateTime )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeDateTime( rDateTime );
}

void OutputObjectBase::writeGuidItem( const String& rName, const OUString& rGuid )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeString( rGuid );
    aItem.cont();
    mxOut->writeString( cfg().getStringOption( rGuid, OUString() ) );
}

// ============================================================================
// ============================================================================

InputObjectBase::~InputObjectBase()
{
}

void InputObjectBase::construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    OutputObjectBase::construct( rParent, rSysFileName );
    mxStrm = rxStrm;
}

void InputObjectBase::construct( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm )
{
    OutputObjectBase::construct( rParent );
    mxStrm = rxStrm;
}

void InputObjectBase::construct( const InputObjectBase& rParent )
{
    *this = rParent;
}

bool InputObjectBase::implIsValid() const
{
    return mxStrm.get() && OutputObjectBase::implIsValid();
}

void InputObjectBase::skipBlock( sal_Int64 nBytes, bool bShowSize )
{
    sal_Int64 nEndPos = ::std::min< sal_Int64 >( mxStrm->tell() + nBytes, mxStrm->size() );
    if( mxStrm->tell() < nEndPos )
    {
        if( bShowSize )
            writeDecItem( "skipped-data-size", static_cast< sal_uInt64 >( nEndPos - mxStrm->tell() ) );
        mxStrm->seek( nEndPos );
    }
}

void InputObjectBase::dumpRawBinary( sal_Int64 nBytes, bool bShowOffset, bool bStream )
{
    TableGuard aTabGuard( mxOut,
        bShowOffset ? 12 : 0,
        3 * OOX_DUMP_BYTESPERLINE / 2 + 1,
        3 * OOX_DUMP_BYTESPERLINE / 2 + 1,
        OOX_DUMP_BYTESPERLINE / 2 + 1 );

    sal_Int64 nMaxShowSize = cfg().getIntOption< sal_Int64 >(
        bStream ? "max-binary-stream-size" : "max-binary-data-size", SAL_MAX_INT64 );

    bool bSeekable = mxStrm->size() >= 0;
    sal_Int64 nEndPos = bSeekable ? ::std::min< sal_Int64 >( mxStrm->tell() + nBytes, mxStrm->size() ) : 0;
    sal_Int64 nDumpEnd = bSeekable ? ::std::min< sal_Int64 >( mxStrm->tell() + nMaxShowSize, nEndPos ) : nMaxShowSize;
    sal_Int64 nPos = bSeekable ? mxStrm->tell() : 0;
    bool bLoop = true;

    while( bLoop && (nPos < nDumpEnd) )
    {
        mxOut->writeHex( static_cast< sal_uInt32 >( nPos ) );
        mxOut->tab();

        sal_uInt8 pnLineData[ OOX_DUMP_BYTESPERLINE ];
        sal_Int32 nLineSize = bSeekable ? ::std::min( static_cast< sal_Int32 >( nDumpEnd - mxStrm->tell() ), OOX_DUMP_BYTESPERLINE ) : OOX_DUMP_BYTESPERLINE;
        sal_Int32 nReadSize = mxStrm->readMemory( pnLineData, nLineSize );
        bLoop = nReadSize == nLineSize;
        nPos += nReadSize;

        if( nReadSize > 0 )
        {
            const sal_uInt8* pnByte = 0;
            const sal_uInt8* pnEnd = 0;
            for( pnByte = pnLineData, pnEnd = pnLineData + nReadSize; pnByte != pnEnd; ++pnByte )
            {
                if( (pnByte - pnLineData) == (OOX_DUMP_BYTESPERLINE / 2) ) mxOut->tab();
                mxOut->writeHex( *pnByte, false );
                mxOut->writeChar( ' ' );
            }

            aTabGuard.tab( 3 );
            for( pnByte = pnLineData, pnEnd = pnLineData + nReadSize; pnByte != pnEnd; ++pnByte )
            {
                if( (pnByte - pnLineData) == (OOX_DUMP_BYTESPERLINE / 2) ) mxOut->tab();
                mxOut->writeChar( static_cast< sal_Unicode >( (*pnByte < 0x20) ? '.' : *pnByte ) );
            }
            mxOut->newLine();
        }
    }

    // skip undumped data
    if( bSeekable )
        skipBlock( nEndPos - mxStrm->tell() );
}

void InputObjectBase::dumpBinary( const String& rName, sal_Int64 nBytes, bool bShowOffset )
{
    {
        MultiItemsGuard aMultiGuard( mxOut );
        writeEmptyItem( rName );
        writeDecItem( "size", nBytes );
    }
    IndentGuard aIndGuard( mxOut );
    dumpRawBinary( nBytes, bShowOffset );
}

void InputObjectBase::dumpRemaining( sal_Int64 nBytes )
{
    if( nBytes > 0 )
    {
        if( cfg().getBoolOption( "show-trailing-unknown", true ) )
            dumpBinary( "remaining-data", nBytes, false );
        else
            skipBlock( nBytes );
    }
}

void InputObjectBase::dumpRemainingTo( sal_Int64 nPos )
{
    if( mxStrm->isEof() || (mxStrm->tell() > nPos) )
        writeInfoItem( "stream-state", OOX_DUMP_ERR_STREAM );
    else
        dumpRemaining( nPos - mxStrm->tell() );
    mxStrm->seek( nPos );
}

void InputObjectBase::dumpRemainingStream()
{
    dumpRemainingTo( mxStrm->size() );
}

void InputObjectBase::dumpArray( const String& rName, sal_Int32 nBytes, sal_Unicode cSep )
{
    sal_Int32 nDumpSize = getLimitedValue< sal_Int32, sal_Int64 >( mxStrm->size() - mxStrm->tell(), 0, nBytes );
    if( nDumpSize > OOX_DUMP_MAXARRAY )
    {
        dumpBinary( rName, nBytes, false );
    }
    else if( nDumpSize > 1 )
    {
        sal_uInt8 pnData[ OOX_DUMP_MAXARRAY ];
        mxStrm->readMemory( pnData, nDumpSize );
        writeArrayItem( rName, pnData, nDumpSize, cSep );
    }
    else if( nDumpSize == 1 )
        dumpHex< sal_uInt8 >( rName );
}

sal_Unicode InputObjectBase::dumpUnicode( const String& rName )
{
    sal_uInt16 nChar;
    *mxStrm >> nChar;
    sal_Unicode cChar = static_cast< sal_Unicode >( nChar );
    writeCharItem( rName( "char" ), cChar );
    return cChar;
}

OUString InputObjectBase::dumpCharArray( const String& rName, sal_Int32 nLen, rtl_TextEncoding eTextEnc, bool bHideTrailingNul )
{
    sal_Int32 nDumpSize = getLimitedValue< sal_Int32, sal_Int64 >( mxStrm->size() - mxStrm->tell(), 0, nLen );
    OUString aString;
    if( nDumpSize > 0 )
    {
        ::std::vector< sal_Char > aBuffer( static_cast< sal_Size >( nLen ) + 1 );
        sal_Int32 nCharsRead = mxStrm->readMemory( &aBuffer.front(), nLen );
        aBuffer[ nCharsRead ] = 0;
        aString = OStringToOUString( OString( &aBuffer.front() ), eTextEnc );
    }
    if( bHideTrailingNul )
        aString = StringHelper::trimTrailingNul( aString );
    writeStringItem( rName( "text" ), aString );
    return aString;
}

OUString InputObjectBase::dumpUnicodeArray( const String& rName, sal_Int32 nLen, bool bHideTrailingNul )
{
    OUStringBuffer aBuffer;
    for( sal_Int32 nIndex = 0; !mxStrm->isEof() && (nIndex < nLen); ++nIndex )
        aBuffer.append( static_cast< sal_Unicode >( mxStrm->readuInt16() ) );
    OUString aString = aBuffer.makeStringAndClear();
    if( bHideTrailingNul )
        aString = StringHelper::trimTrailingNul( aString );
    writeStringItem( rName( "text" ), aString );
    return aString;
}

DateTime InputObjectBase::dumpFileTime( const String& rName )
{
    DateTime aDateTime;

    ItemGuard aItem( mxOut, rName( "file-time" ) );
    sal_Int64 nFileTime = dumpDec< sal_Int64 >( EMPTY_STRING );
    // file time is in 10^-7 seconds (100 nanoseconds), convert to 1/100 seconds
    nFileTime /= 100000;
    // entire days
    sal_Int64 nDays = nFileTime / sal_Int64( 360000 * 24 );
    // number of entire years
    sal_Int64 nYears = (nDays - (nDays / (4 * 365)) + (nDays / (100 * 365)) - (nDays / (400 * 365))) / 365;
    // remaining days in the year
    sal_Int64 nDaysInYear = nDays - (nYears * 365 + nYears / 4 - nYears / 100 + nYears / 400);
    // the year (file dates start from 1601-01-01)
    aDateTime.Year = static_cast< sal_uInt16 >( 1601 + nYears );
    // leap year?
    bool bLeap = ((aDateTime.Year % 4 == 0) && (aDateTime.Year % 100 != 0)) || (aDateTime.Year % 400 == 0);
    // static arrays with number of days in month
    static const sal_Int64 spnDaysInMonth[]  = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static const sal_Int64 spnDaysInMonthL[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    const sal_Int64* pnDaysInMonth = bLeap ? spnDaysInMonthL : spnDaysInMonth;
    // the month
    aDateTime.Month = 1;
    while( nDaysInYear >= *pnDaysInMonth )
    {
        nDaysInYear -= *pnDaysInMonth++;
        ++aDateTime.Month;
    }
    // the day
    aDateTime.Day = static_cast< sal_uInt16 >( nDaysInYear + 1 );
    // number of 1/100 seconds in the day
    sal_Int64 nTimeInDay = nFileTime % sal_Int64( 360000 * 24 );
    // 1/100 seconds
    aDateTime.HundredthSeconds = static_cast< sal_uInt16 >( nTimeInDay % 100 );
    nTimeInDay /= 100;
    // seconds
    aDateTime.Seconds = static_cast< sal_uInt16 >( nTimeInDay % 60 );
    nTimeInDay /= 60;
    // minutes
    aDateTime.Minutes = static_cast< sal_uInt16 >( nTimeInDay % 60 );
    nTimeInDay /= 60;
    // hours
    aDateTime.Hours = static_cast< sal_uInt16 >( nTimeInDay );

    writeDateTimeItem( EMPTY_STRING, aDateTime );
    return aDateTime;
}

OUString InputObjectBase::dumpGuid( const String& rName )
{
    OUStringBuffer aBuffer;
    sal_uInt32 nData32;
    sal_uInt16 nData16;
    sal_uInt8 nData8;

    *mxStrm >> nData32;
    StringHelper::appendHex( aBuffer, nData32, false );
    aBuffer.append( sal_Unicode( '-' ) );
    *mxStrm >> nData16;
    StringHelper::appendHex( aBuffer, nData16, false );
    aBuffer.append( sal_Unicode( '-' ) );
    *mxStrm >> nData16;
    StringHelper::appendHex( aBuffer, nData16, false );
    aBuffer.append( sal_Unicode( '-' ) );
    *mxStrm >> nData8;
    StringHelper::appendHex( aBuffer, nData8, false );
    *mxStrm >> nData8;
    StringHelper::appendHex( aBuffer, nData8, false );
    aBuffer.append( sal_Unicode( '-' ) );
    for( int nIndex = 0; nIndex < 6; ++nIndex )
    {
        *mxStrm >> nData8;
        StringHelper::appendHex( aBuffer, nData8, false );
    }
    StringHelper::enclose( aBuffer, '{', '}' );
    OUString aGuid = aBuffer.makeStringAndClear();
    writeGuidItem( rName( "guid" ), aGuid );
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
// ============================================================================

BinaryStreamObject::BinaryStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    InputObjectBase::construct( rParent, rxStrm, rSysFileName );
}

void BinaryStreamObject::dumpBinaryStream( bool bShowOffset )
{
    mxStrm->seekToStart();
    dumpRawBinary( mxStrm->size(), bShowOffset, true );
    mxOut->emptyLine();
}

void BinaryStreamObject::implDump()
{
    dumpBinaryStream();
}

// ============================================================================
// ============================================================================

void TextStreamObjectBase::construct( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, rtl_TextEncoding eTextEnc, const OUString& rSysFileName )
{
    InputObjectBase::construct( rParent, rxStrm, rSysFileName );
    constructTextStrmObj( eTextEnc );
}

void TextStreamObjectBase::construct( const OutputObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, rtl_TextEncoding eTextEnc )
{
    InputObjectBase::construct( rParent, rxStrm );
    constructTextStrmObj( eTextEnc );
}

bool TextStreamObjectBase::implIsValid() const
{
    return InputObjectBase::implIsValid() && mxTextStrm.get();
}

void TextStreamObjectBase::implDump()
{
    implDumpText( *mxTextStrm );
}

void TextStreamObjectBase::constructTextStrmObj( rtl_TextEncoding eTextEnc )
{
    if( mxStrm.get() )
        mxTextStrm.reset( new TextInputStream( getContext(), *mxStrm, eTextEnc ) );
}

// ============================================================================

TextLineStreamObject::TextLineStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, rtl_TextEncoding eTextEnc, const OUString& rSysFileName )
{
    TextStreamObjectBase::construct( rParent, rxStrm, eTextEnc, rSysFileName );
}

TextLineStreamObject::TextLineStreamObject( const OutputObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, rtl_TextEncoding eTextEnc )
{
    TextStreamObjectBase::construct( rParent, rxStrm, eTextEnc );
}

void TextLineStreamObject::implDumpText( TextInputStream& rTextStrm )
{
    sal_uInt32 nLine = 0;
    while( !rTextStrm.isEof() )
    {
        OUString aLine = rTextStrm.readLine();
        if( !rTextStrm.isEof() || !aLine.isEmpty() )
            implDumpLine( aLine, ++nLine );
    }
}

void TextLineStreamObject::implDumpLine( const OUString& rLine, sal_uInt32 nLine )
{
    TableGuard aTabGuard( mxOut, 8 );
    mxOut->writeDec( nLine, 6 );
    mxOut->tab();
    mxOut->writeString( rLine );
    mxOut->newLine();
}

// ============================================================================

XmlStreamObject::XmlStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    TextStreamObjectBase::construct( rParent, rxStrm, RTL_TEXTENCODING_UTF8, rSysFileName );
}

void XmlStreamObject::implDumpText( TextInputStream& rTextStrm )
{
    /*  Buffers a start element and the following element text. Needed to dump
        matching start/end elements and the element text on the same line. */
    OUStringBuffer aOldStartElem;
    // special handling for VML
    bool bIsVml = InputOutputHelper::getFileNameExtension( maSysFileName ).equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("vml"));

    while( !rTextStrm.isEof() )
    {
        // get the next element and the following element text from text stream
        OUString aElem = rTextStrm.readToChar( '>', true ).trim();
        OUString aText = rTextStrm.readToChar( '<', false );

        // remove multiple whitespace from element
        sal_Int32 nPos = 0;
        while( nPos < aElem.getLength() )
        {
            while( (nPos < aElem.getLength()) && (aElem[ nPos ] >= 32) ) ++nPos;
            if( nPos < aElem.getLength() )
                aElem = OUStringBuffer( aElem.copy( 0, nPos ) ).append( sal_Unicode( ' ' ) ).append( aElem.copy( nPos ).trim() ).makeStringAndClear();
            ++nPos;
        }

        sal_Int32 nElemLen = aElem.getLength();
        if( (nElemLen >= 2) && (aElem[ 0 ] == '<') && (aElem[ nElemLen - 1 ] == '>') )
        {
            // determine type of the element
            bool bSimpleElem = (aElem[ 1 ] == '!') || (aElem[ 1 ] == '?') || (aElem[ nElemLen - 2 ] == '/') ||
                (bIsVml && (nElemLen == 4) && (aElem[ 1 ] == 'b') && (aElem[ 2 ] == 'r'));
            bool bStartElem = !bSimpleElem && (aElem[ 1 ] != '/');
            bool bEndElem = !bSimpleElem && !bStartElem;

            /*  Start element or simple element: flush old start element and
                its text from previous iteration, and start a new indentation
                level for the new element. Trim whitespace and line breaks from
                the text of the old start element. */
            if( (bSimpleElem || bStartElem) && (aOldStartElem.getLength() > 0) )
            {
                mxOut->writeString( aOldStartElem.makeStringAndClear().trim() );
                mxOut->newLine();
                mxOut->incIndent();
            }

            /*  Start element: remember it and its text, to be able to print the
                matching end element on the same line in the next iteration. */
            if( bStartElem )
            {
                aOldStartElem.append( aElem ).append( aText );
            }
            else
            {
                /*  End element: if a start element has been remembered in the
                    previous iteration, write it out here untrimmed, to show
                    all whitespace in the element text, and without trailing
                    line break. Code below will add the end element right after
                    it. Otherwise, return to previous indentation level. */
                if( bEndElem )
                {
                    if( aOldStartElem.getLength() == 0 )
                        mxOut->decIndent();
                    else
                        mxOut->writeString( aOldStartElem.makeStringAndClear() );
                }

                /*  Write the element. Write following element text in a new
                    line, but only, if it does not contain of white space
                    entirely. */
                mxOut->writeString( aElem );
                mxOut->newLine();
                if( !aText.trim().isEmpty() )
                {
                    mxOut->writeString( aText );
                    mxOut->newLine();
                }
            }
        }
    }
}

// ============================================================================
// ============================================================================

void RecordObjectBase::construct( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxBaseStrm, const OUString& rSysFileName,
        const BinaryInputStreamRef& rxRecStrm, const String& rRecNames, const String& rSimpleRecs )
{
    InputObjectBase::construct( rParent, rxRecStrm, rSysFileName );
    constructRecObjBase( rxBaseStrm, rRecNames, rSimpleRecs );
}

bool RecordObjectBase::implIsValid() const
{
    return mxBaseStrm.get() && InputObjectBase::implIsValid();
}

void RecordObjectBase::implDump()
{
    NameListRef xRecNames = getRecNames();
    ItemFormatMap aSimpleRecs( maSimpleRecs.getNameList( cfg() ) );

    while( implStartRecord( *mxBaseStrm, mnRecPos, mnRecId, mnRecSize ) )
    {
        // record header
        mxOut->emptyLine();
        writeHeader();
        implWriteExtHeader();
        IndentGuard aIndGuard( mxOut );
        sal_Int64 nRecPos = mxStrm->tell();

        // record body
        if( !mbBinaryOnly && cfg().hasName( xRecNames, mnRecId ) )
        {
            ItemFormatMap::const_iterator aIt = aSimpleRecs.find( mnRecId );
            if( aIt != aSimpleRecs.end() )
                dumpItem( aIt->second );
            else
                implDumpRecordBody();
        }

        // remaining undumped data
        if( !mxStrm->isEof() && (mxStrm->tell() == nRecPos) )
            dumpRawBinary( mnRecSize, false );
        else
            dumpRemainingTo( nRecPos + mnRecSize );
    }
}

void RecordObjectBase::implWriteExtHeader()
{
}

void RecordObjectBase::implDumpRecordBody()
{
}

void RecordObjectBase::constructRecObjBase( const BinaryInputStreamRef& rxBaseStrm, const String& rRecNames, const String& rSimpleRecs )
{
    mxBaseStrm = rxBaseStrm;
    maRecNames = rRecNames;
    maSimpleRecs = rSimpleRecs;
    mnRecPos = mnRecId = mnRecSize = 0;
    mbBinaryOnly = false;
    if( InputObjectBase::implIsValid() )
        mbShowRecPos = cfg().getBoolOption( "show-record-position", true );
}

void RecordObjectBase::writeHeader()
{
    MultiItemsGuard aMultiGuard( mxOut );
    writeEmptyItem( "REC" );
    if( mbShowRecPos && mxBaseStrm->isSeekable() )
        writeShortHexItem( "pos", mnRecPos, "CONV-DEC" );
    writeShortHexItem( "size", mnRecSize, "CONV-DEC" );
    ItemGuard aItem( mxOut, "id" );
    mxOut->writeShortHex( mnRecId );
    addNameToItem( mnRecId, "CONV-DEC" );
    addNameToItem( mnRecId, maRecNames );
}

// ============================================================================

void SequenceRecordObjectBase::construct( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxBaseStrm, const ::rtl::OUString& rSysFileName,
        const String& rRecNames, const String& rSimpleRecs )
{
    BinaryInputStreamRef xRecStrm( new SequenceInputStream( *mxRecData ) );
    RecordObjectBase::construct( rParent, rxBaseStrm, rSysFileName, xRecStrm, rRecNames, rSimpleRecs );
}

bool SequenceRecordObjectBase::implStartRecord( BinaryInputStream& rBaseStrm, sal_Int64& ornRecPos, sal_Int64& ornRecId, sal_Int64& ornRecSize )
{
    bool bValid = true;
    if( rBaseStrm.isSeekable() )
    {
        ornRecPos = rBaseStrm.tell();
        // do not try to overread seekable streams, may cause assertions
        bValid = ornRecPos < rBaseStrm.size();
    }

    // read the record header
    if( bValid )
        bValid = implReadRecordHeader( rBaseStrm, ornRecId, ornRecSize ) && !rBaseStrm.isEof() && (0 <= ornRecSize) && (ornRecSize <= 0x00100000);

    // read record contents into data sequence
    if( bValid )
    {
        sal_Int32 nRecSize = static_cast< sal_Int32 >( ornRecSize );
        mxRecData->realloc( nRecSize );
        bValid = (nRecSize == 0) || (rBaseStrm.readData( *mxRecData, nRecSize ) == nRecSize);
        mxStrm->seekToStart();
    }
    return bValid;
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

bool DumperBase::isImportCancelled() const
{
    return isValid() && cfg().isPasswordCancelled();
}

void DumperBase::construct( const ConfigRef& rxConfig )
{
    if( isValid( rxConfig ) && rxConfig->isDumperEnabled() )
        ObjectBase::construct( rxConfig );
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
