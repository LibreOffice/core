/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fdumper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:25:30 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_FDUMPER_HXX
#include "fdumper.hxx"
#endif

#if SCF_INCL_DUMPER

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

namespace scf {
namespace dump {

const sal_Unicode SCF_UTF8_BOM          = 0xFEFF;
const xub_StrLen SCF_DUMP_MAXSTRLEN     = 80;
const xub_StrLen SCF_DUMP_INDENT        = 2;
const sal_Unicode SCF_DUMP_BINDOT       = '.';
const sal_Unicode SCF_DUMP_CFG_LISTSEP  = ',';
const sal_Unicode SCF_DUMP_CFG_QUOTE    = '\'';
const sal_Unicode SCF_DUMP_LISTSEP      = '\n';
const sal_Unicode SCF_DUMP_ITEMSEP      = '=';
const sal_Size SCF_DUMP_BYTESPERLINE    = 16;
const sal_Size SCF_DUMP_MAXARRAY        = 16;

// ============================================================================
// ============================================================================

ItemFormat::ItemFormat() :
    meDataType( DATATYPE_VOID ),
    meFmtType( FORMATTYPE_NONE )
{
}

void ItemFormat::Set( DataType eDataType, FormatType eFmtType, const String& rItemName )
{
    meDataType = eDataType;
    meFmtType = eFmtType;
    maItemName = rItemName;
    maItemNameUtf8 = StringHelper::ConvertToUtf8( rItemName );
    maListName.Erase();
}

void ItemFormat::Set( DataType eDataType, FormatType eFmtType, const String& rItemName, const String& rListName )
{
    Set( eDataType, eFmtType, rItemName );
    maListName = rListName;
}

void ItemFormat::Parse( const String& rFormatStr )
{
    ScfStringVec aFormatVec;
    StringHelper::ConvertStringToStringList( aFormatVec, rFormatStr, false );
    Parse( aFormatVec );
}

void ItemFormat::Parse( const ScfStringVec& rFormatVec )
{
    Set( DATATYPE_VOID, FORMATTYPE_NONE, String::EmptyString() );

    ScfStringVec::const_iterator aIt = rFormatVec.begin(), aEnd = rFormatVec.end();
    String aDataType, aFmtType;
    if( aIt != aEnd ) aDataType = *aIt++;
    if( aIt != aEnd ) aFmtType = *aIt++;
    if( aIt != aEnd ) maItemName = *aIt++;
    if( aIt != aEnd ) maListName = *aIt++;

    meDataType = StringHelper::ConvertToDataType( aDataType );
    meFmtType = StringHelper::ConvertToFormatType( aFmtType );
    maItemNameUtf8 = StringHelper::ConvertToUtf8( maItemName );

    if( meFmtType == FORMATTYPE_NONE )
    {
        if( aFmtType.EqualsAscii( "unused" ) )
            Set( meDataType, FORMATTYPE_HEX, CREATE_STRING( SCF_DUMP_UNUSED ) );
        else if( aFmtType.EqualsAscii( "unknown" ) )
            Set( meDataType, FORMATTYPE_HEX, CREATE_STRING( SCF_DUMP_UNKNOWN ) );
    }
}

// ============================================================================
// ============================================================================

// append string to string ----------------------------------------------------

void StringHelper::AppendString( String& rStr, const String& rData, xub_StrLen nWidth, sal_Unicode cFill )
{
    if( rData.Len() < nWidth )
        rStr.Expand( rStr.Len() + nWidth - rData.Len(), cFill );
    rStr.Append( rData );
}

// append decimal -------------------------------------------------------------

void StringHelper::AppendDec( String& rStr, sal_uInt8 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_Int8 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_uInt16 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_Int16 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_uInt32 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt64( nData ), nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_Int32 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_uInt64 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    /*  Values greater than biggest signed 64bit integer will change to
        negative when converting to sal_Int64. Therefore, the trailing digit
        will be written separately. */
    String aData;
    if( nData > 9 )
        aData.Append( String::CreateFromInt64( static_cast< sal_Int64 >( nData / 10 ) ) );
    aData.Append( static_cast< sal_Unicode >( '0' + (nData % 10) ) );
    AppendString( rStr, aData, nWidth, cFill );
}

void StringHelper::AppendDec( String& rStr, sal_Int64 nData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String::CreateFromInt64( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, double fData, xub_StrLen nWidth, sal_Unicode cFill )
{
    AppendString( rStr, String( ::rtl::math::doubleToUString( fData, rtl_math_StringFormat_G, 15, '.', true ) ), nWidth, cFill );
}

// append hexadecimal ---------------------------------------------------------

void StringHelper::AppendHex( String& rStr, sal_uInt8 nData, bool bPrefix )
{
    static const sal_Char spcHexDigits[] = "0123456789ABCDEF";
    static const String saPrefix = CREATE_STRING( "0x" );
    if( bPrefix )
        rStr.Append( saPrefix );
    rStr.Append( spcHexDigits[ (nData >> 4) & 0x0F ] ).Append( spcHexDigits[ nData & 0x0F ] );
}

void StringHelper::AppendHex( String& rStr, sal_Int8 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::AppendHex( String& rStr, sal_uInt16 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt8 >( nData >> 8 ), bPrefix );
    AppendHex( rStr, static_cast< sal_uInt8 >( nData ), false );
}

void StringHelper::AppendHex( String& rStr, sal_Int16 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::AppendHex( String& rStr, sal_uInt32 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt16 >( nData >> 16 ), bPrefix );
    AppendHex( rStr, static_cast< sal_uInt16 >( nData ), false );
}

void StringHelper::AppendHex( String& rStr, sal_Int32 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::AppendHex( String& rStr, sal_uInt64 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt32 >( nData >> 32 ), bPrefix );
    AppendHex( rStr, static_cast< sal_uInt32 >( nData ), false );
}

void StringHelper::AppendHex( String& rStr, sal_Int64 nData, bool bPrefix )
{
    AppendHex( rStr, static_cast< sal_uInt64 >( nData ), bPrefix );
}

void StringHelper::AppendHex( String& rStr, double fData, bool bPrefix )
{
    const sal_uInt32* pnData = reinterpret_cast< const sal_uInt32* >( &fData );
    AppendHex( rStr, pnData[ 0 ], bPrefix );
    AppendHex( rStr, pnData[ 1 ], false );
}

// append shortened hexadecimal -----------------------------------------------

void StringHelper::AppendShortHex( String& rStr, sal_uInt8 nData, bool bPrefix )
{
    if( nData != 0 )
        AppendHex( rStr, nData, bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_Int8 nData, bool bPrefix )
{
    AppendShortHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_uInt16 nData, bool bPrefix )
{
    if( nData > 0xFF )
        AppendHex( rStr, nData, bPrefix );
    else
        AppendShortHex( rStr, static_cast< sal_uInt8 >( nData ), bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_Int16 nData, bool bPrefix )
{
    AppendShortHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_uInt32 nData, bool bPrefix )
{
    if( nData > 0xFFFF )
        AppendHex( rStr, nData, bPrefix );
    else
        AppendShortHex( rStr, static_cast< sal_uInt16 >( nData ), bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_Int32 nData, bool bPrefix )
{
    AppendShortHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_uInt64 nData, bool bPrefix )
{
    if( nData > 0xFFFFFFFF )
        AppendHex( rStr, nData, bPrefix );
    else
        AppendShortHex( rStr, static_cast< sal_uInt32 >( nData ), bPrefix );
}

void StringHelper::AppendShortHex( String& rStr, sal_Int64 nData, bool bPrefix )
{
    AppendShortHex( rStr, static_cast< sal_uInt64 >( nData ), bPrefix );
}

// append binary --------------------------------------------------------------

void StringHelper::AppendBin( String& rStr, sal_uInt8 nData, bool bDots )
{
    for( sal_uInt8 nMask = 0x80; nMask != 0; (nMask >>= 1) &= 0x7F )
    {
        rStr.Append( (nData & nMask) ? '1' : '0' );
        if( bDots && (nMask == 0x10) )
            rStr.Append( SCF_DUMP_BINDOT );
    }
}

void StringHelper::AppendBin( String& rStr, sal_Int8 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt8 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, sal_uInt16 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt8 >( nData >> 8 ), bDots );
    if( bDots )
        rStr.Append( SCF_DUMP_BINDOT );
    AppendBin( rStr, static_cast< sal_uInt8 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, sal_Int16 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt16 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, sal_uInt32 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt16 >( nData >> 16 ), bDots );
    if( bDots )
        rStr.Append( SCF_DUMP_BINDOT );
    AppendBin( rStr, static_cast< sal_uInt16 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, sal_Int32 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt32 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, sal_uInt64 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt32 >( nData >> 32 ), bDots );
    if( bDots )
        rStr.Append( SCF_DUMP_BINDOT );
    AppendBin( rStr, static_cast< sal_uInt32 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, sal_Int64 nData, bool bDots )
{
    AppendBin( rStr, static_cast< sal_uInt64 >( nData ), bDots );
}

void StringHelper::AppendBin( String& rStr, double fData, bool bDots )
{
    const sal_uInt32* pnData = reinterpret_cast< const sal_uInt32* >( &fData );
    AppendBin( rStr, pnData[ 0 ], bDots );
    if( bDots )
        rStr.Append( SCF_DUMP_BINDOT );
    AppendBin( rStr, pnData[ 1 ], bDots );
}

// append formatted value -----------------------------------------------------

void StringHelper::AppendBool( String& rStr, bool bData )
{
    rStr.AppendAscii( bData ? "true" : "false" );
}

// encoded text output --------------------------------------------------------

void StringHelper::AppendCChar( String& rStr, sal_Unicode cChar, bool bPrefix )
{
    static const String saXPrefix = CREATE_STRING( "\\x" );
    static const String saUPrefix = CREATE_STRING( "\\u" );
    if( cChar > 0x00FF )
    {
        if( bPrefix )
            rStr.Append( saUPrefix );
        AppendHex( rStr, static_cast< sal_uInt16 >( cChar ), false );
    }
    else
    {
        if( bPrefix )
            rStr.Append( saXPrefix );
        AppendHex( rStr, static_cast< sal_uInt8 >( cChar ), false );
    }
}

void StringHelper::AppendEncChar( String& rStr, sal_Unicode cChar, xub_StrLen nCount, bool bPrefix )
{
    if( cChar < 0x0020 )
    {
        // C-style hex code
        String aCode;
        AppendCChar( aCode, cChar, bPrefix );
        for( xub_StrLen nIdx = 0; nIdx < nCount; ++nIdx )
            rStr.Append( aCode );
    }
    else if( nCount == 1 )
        rStr.Append( cChar );
    else
        rStr.Expand( rStr.Len() + nCount, cChar );
}

void StringHelper::AppendEncString( String& rStr, const String& rData, bool bPrefix )
{
    xub_StrLen nBeg = 0;
    xub_StrLen nIdx = 0;
    xub_StrLen nEnd = rData.Len();
    while( nIdx < nEnd )
    {
        // find next character that needs encoding
        while( (nIdx < nEnd) && (rData.GetChar( nIdx ) >= 0x20) ) ++nIdx;
        // append portion
        if( nBeg < nIdx )
        {
            if( (nBeg == 0) && (nIdx == nEnd) )
                rStr.Append( rData );
            else
                rStr.Append( String( rData, nBeg, nIdx - nBeg ) );
        }
        // append characters to be encoded
        while( (nIdx < nEnd) && (rData.GetChar( nIdx ) < 0x20) )
        {
            AppendCChar( rStr, rData.GetChar( nIdx ), bPrefix );
            ++nIdx;
        }
        // adjust limits
        nBeg = nIdx;
    }
}

// token list -----------------------------------------------------------------

void StringHelper::AppendToken( String& rStr, const String& rToken, sal_Unicode cSep )
{
    if( (rStr.Len() > 0) && (rToken.Len() > 0) )
        rStr.Append( cSep );
    rStr.Append( rToken );
}

void StringHelper::AppendToken( String& rStr, sal_Int64 nToken, sal_Unicode cSep )
{
    String aToken;
    AppendDec( aToken, nToken );
    AppendToken( rStr, aToken, cSep );
}

void StringHelper::PrependToken( String& rStr, const String& rToken, sal_Unicode cSep )
{
    String aRet = rToken;
    AppendToken( aRet, rStr, cSep );
    rStr = aRet;
}

void StringHelper::PrependToken( String& rStr, sal_Int64 nToken, sal_Unicode cSep )
{
    String aToken;
    AppendDec( aToken, nToken );
    PrependToken( rStr, aToken, cSep );
}

void StringHelper::AppendIndex( String& rStr, const String& rIdx )
{
    rStr.Append( '[' ).Append( rIdx ).Append( ']' );
}

void StringHelper::AppendIndex( String& rStr, sal_Int64 nIdx )
{
    String aToken;
    AppendDec( aToken, nIdx );
    AppendIndex( rStr, aToken );
}

void StringHelper::AppendIndexedText( String& rStr, const String& rData, const String& rIdx )
{
    rStr.Append( rData );
    AppendIndex( rStr, rIdx );
}

void StringHelper::AppendIndexedText( String& rStr, const String& rData, sal_Int64 nIdx )
{
    rStr.Append( rData );
    AppendIndex( rStr, nIdx );
}

String StringHelper::GetToken( const String& rData, xub_StrLen& rnPos, sal_Unicode cSep )
{
    String aToken = rData.GetToken( 0, cSep, rnPos );
    TrimSpaces( aToken );
    return aToken;
}

// quoting and trimming -------------------------------------------------------

void StringHelper::Enclose( String& rStr, sal_Unicode cOpen, sal_Unicode cClose )
{
    rStr.Insert( cOpen, 0 ).Append( cClose ? cClose : cOpen );
}

void StringHelper::TrimSpaces( String& rStr )
{
    xub_StrLen nPos = 0;
    while( (nPos < rStr.Len()) && ((rStr.GetChar( nPos ) == ' ') || (rStr.GetChar( nPos ) == '\t')) )
        ++nPos;
    rStr.Erase( 0, nPos );
    nPos = rStr.Len();
    while( (nPos > 0) && ((rStr.GetChar( nPos - 1 ) == ' ') || (rStr.GetChar( nPos - 1 ) == '\t')) )
        --nPos;
    rStr.Erase( nPos + 1 );
}

void StringHelper::TrimQuotes( String& rStr )
{
    xub_StrLen nPos = 0;
    while( nPos < rStr.Len() )
    {
        // trim quotes
        if( (nPos < rStr.Len()) && (rStr.GetChar( nPos ) == SCF_DUMP_CFG_QUOTE) )
        {
            rStr.Erase( nPos, 1 );
            while( (nPos < rStr.Len()) && ((rStr.GetChar( nPos ) != SCF_DUMP_CFG_QUOTE) ||
                    ((nPos + 1 < rStr.Len()) && (rStr.GetChar( nPos + 1 ) == SCF_DUMP_CFG_QUOTE))) )
                ++nPos;
            if( (nPos < rStr.Len()) && (rStr.GetChar( nPos ) == SCF_DUMP_CFG_QUOTE) )
                rStr.Erase( nPos, 1 );
        }
        // look for separator
        while( (nPos < rStr.Len()) && (rStr.GetChar( nPos ) != SCF_DUMP_CFG_LISTSEP) )
            ++nPos;
        if( (nPos < rStr.Len()) && (rStr.GetChar( nPos ) == SCF_DUMP_CFG_LISTSEP) )
            rStr.SetChar( nPos, SCF_DUMP_LISTSEP );
        ++nPos;
    }
}

// string conversion ----------------------------------------------------------

ByteString StringHelper::ConvertToUtf8( const String& rStr )
{
    return ByteString( rStr, RTL_TEXTENCODING_UTF8 );
}

DataType StringHelper::ConvertToDataType( const String& rStr )
{
    DataType eType = DATATYPE_VOID;
    if( rStr.EqualsAscii( "int8" ) )
        eType = DATATYPE_INT8;
    else if( rStr.EqualsAscii( "uint8" ) )
        eType = DATATYPE_UINT8;
    else if( rStr.EqualsAscii( "int16" ) )
        eType = DATATYPE_INT16;
    else if( rStr.EqualsAscii( "uint16" ) )
        eType = DATATYPE_UINT16;
    else if( rStr.EqualsAscii( "int32" ) )
        eType = DATATYPE_INT32;
    else if( rStr.EqualsAscii( "uint32" ) )
        eType = DATATYPE_UINT32;
    else if( rStr.EqualsAscii( "int64" ) )
        eType = DATATYPE_INT64;
    else if( rStr.EqualsAscii( "uint64" ) )
        eType = DATATYPE_UINT64;
    else if( rStr.EqualsAscii( "float" ) )
        eType = DATATYPE_FLOAT;
    else if( rStr.EqualsAscii( "double" ) )
        eType = DATATYPE_DOUBLE;
    return eType;
}

FormatType StringHelper::ConvertToFormatType( const String& rStr )
{
    FormatType eType = FORMATTYPE_NONE;
    if( rStr.EqualsAscii( "dec" ) )
        eType = FORMATTYPE_DEC;
    else if( rStr.EqualsAscii( "hex" ) )
        eType = FORMATTYPE_HEX;
    else if( rStr.EqualsAscii( "bin" ) )
        eType = FORMATTYPE_BIN;
    else if( rStr.EqualsAscii( "fix" ) )
        eType = FORMATTYPE_FIX;
    else if( rStr.EqualsAscii( "bool" ) )
        eType = FORMATTYPE_BOOL;
    return eType;
}

bool StringHelper::ConvertFromDec( sal_Int64& rnData, const String& rData )
{
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rData.Len();
    bool bNeg = false;
    if( (nLen > 0) && (rData.GetChar( 0 ) == '-') )
    {
        bNeg = true;
        ++nPos;
    }
    rnData = 0;
    for( ; nPos < nLen; ++nPos )
    {
        sal_Unicode cChar = rData.GetChar( nPos );
        if( (cChar < '0') || (cChar > '9') )
            return false;
        (rnData *= 10) += (cChar - '0');
    }
    if( bNeg )
        rnData *= -1;
    return true;
}

bool StringHelper::ConvertFromHex( sal_Int64& rnData, const String& rData )
{
    rnData = 0;
    for( xub_StrLen nPos = 0, nLen = rData.Len(); nPos < nLen; ++nPos )
    {
        sal_Unicode cChar = rData.GetChar( nPos );
        if( ('0' <= cChar) && (cChar <= '9') )
            cChar -= '0';
        else if( ('A' <= cChar) && (cChar <= 'F') )
            cChar -= ('A' - 10);
        else if( ('a' <= cChar) && (cChar <= 'f') )
            cChar -= ('a' - 10);
        else
            return false;
        (rnData <<= 4) += cChar;
    }
    return true;
}

bool StringHelper::ConvertStringToInt( sal_Int64& rnData, const String& rData )
{
    if( (rData.Len() > 2) && (rData.GetChar( 0 ) == '0') && ((rData.GetChar( 1 ) == 'X') || (rData.GetChar( 1 ) == 'x')) )
        return ConvertFromHex( rnData, rData.Copy( 2 ) );
    return ConvertFromDec( rnData, rData );
}

bool StringHelper::ConvertStringToDouble( double& rfData, const String& rData )
{
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nSize = 0;
    rfData = rtl::math::stringToDouble( rData, '.', '\0', &eStatus, &nSize );
    return (eStatus == rtl_math_ConversionStatus_Ok) && (nSize == static_cast< sal_Int32 >( rData.Len() ));
}

bool StringHelper::ConvertStringToBool( const String& rData )
{
    if( rData.EqualsIgnoreCaseAscii( "true" ) )
        return true;
    if( rData.EqualsIgnoreCaseAscii( "false" ) )
        return false;
    sal_Int64 nData;
    return ConvertStringToInt( nData, rData ) && (nData != 0);
}

void StringHelper::ConvertStringToStringList( ScfStringVec& rVec, const String& rData, bool bIgnoreEmpty )
{
    rVec.clear();
    xub_StrLen nTokenCnt = rData.GetTokenCount( SCF_DUMP_LISTSEP );
    xub_StrLen nPos = 0;
    for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
    {
        String aToken = GetToken( rData, nPos, SCF_DUMP_LISTSEP );
        if( !bIgnoreEmpty || (aToken.Len() > 0) )
            rVec.push_back( aToken );
    }
}

void StringHelper::ConvertStringToIntList( ScfInt64Vec& rVec, const String& rData, bool bIgnoreEmpty )
{
    rVec.clear();
    xub_StrLen nTokenCnt = rData.GetTokenCount( SCF_DUMP_LISTSEP );
    xub_StrLen nPos = 0;
    sal_Int64 nData;
    for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
    {
        bool bOk = ConvertStringToInt( nData, GetToken( rData, nPos, SCF_DUMP_LISTSEP ) );
        if( !bIgnoreEmpty || bOk )
            rVec.push_back( bOk ? nData : 0 );
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

void ConfigItemBase::ReadConfigBlock( SvStream& rStrm )
{
    // first line must be "begin" item
    sal_Size nOldPos = rStrm.Tell();
    if( ReadConfigLine( rStrm ) == LINETYPE_BEGIN )
        ReadConfigBlockContents( rStrm );
    else
        rStrm.Seek( nOldPos );
}

void ConfigItemBase::ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData )
{
}

void ConfigItemBase::ImplProcessConfigItemInt( SvStream& rStrm, sal_Int64 nKey, const String& rData )
{
}

void ConfigItemBase::ReadConfigBlockContents( SvStream& rStrm )
{
    bool bLoop = true;
    while( bLoop && !rStrm.IsEof() )
    {
        String aKey, aData;
        switch( ReadConfigLine( rStrm, aKey, aData ) )
        {
            case LINETYPE_BEGIN:
                IgnoreConfigBlockContents( rStrm );     // ignore unknown embedded block
            break;
            case LINETYPE_END:
                bLoop = false;
            break;
            case LINETYPE_DATA:
                ProcessConfigItem( rStrm, aKey, aData );
            break;
            default:
                DBG_ERRORFILE( "scf::dump::ConfigItemBase::ReadConfigBlockContents - unknown config line type" );
        }
    }
}

void ConfigItemBase::IgnoreConfigBlockContents( SvStream& rStrm )
{
    bool bLoop = true;
    while( bLoop && !rStrm.IsEof() )
    {
        switch( ReadConfigLine( rStrm ) )
        {
            case LINETYPE_BEGIN:
                IgnoreConfigBlockContents( rStrm );
            break;
            case LINETYPE_END:
                bLoop = false;
            break;
            default:;
        }
    }
}

ConfigItemBase::LineType ConfigItemBase::ReadConfigLine( SvStream& rStrm, String& rKey, String& rData ) const
{
    String aLine;
    while( !rStrm.IsEof() && (aLine.Len() == 0) )
    {
        rStrm.ReadByteStringLine( aLine, RTL_TEXTENCODING_UTF8 );
        aLine.EraseLeadingChars( SCF_UTF8_BOM );
        StringHelper::TrimSpaces( aLine );
        if( aLine.Len() > 0 )
        {
            // ignore comments (starting with hash or semicolon)
            sal_Unicode cChar = aLine.GetChar( 0 );
            if( (cChar == '#') || (cChar == ';') )
                aLine.Erase();
        }
    }

    LineType eResult = LINETYPE_END;
    if( aLine.Len() > 0 )
    {
        xub_StrLen nEqPos = aLine.Search( '=' );
        if( nEqPos == STRING_NOTFOUND )
        {
            rKey = aLine;
        }
        else
        {
            rKey = aLine.Copy( 0, nEqPos );
            StringHelper::TrimSpaces( rKey );
            rData = aLine.Copy( nEqPos + 1 );
            StringHelper::TrimSpaces( rData );
            StringHelper::TrimQuotes( rData );
        }

        if( rKey.Len() > 0 )
        {
            eResult = LINETYPE_DATA;
            if( rData.Len() == 0 )
            {
                if( rKey.EqualsAscii( "begin" ) )
                    eResult = LINETYPE_BEGIN;
                else if( rKey.EqualsAscii( "end" ) )
                    eResult = LINETYPE_END;
            }
        }
    }

    return eResult;
}

ConfigItemBase::LineType ConfigItemBase::ReadConfigLine( SvStream& rStrm ) const
{
    String aKey, aData;
    return ReadConfigLine( rStrm, aKey, aData );
}

void ConfigItemBase::ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData )
{
    sal_Int64 nKey;
    if( StringHelper::ConvertStringToInt( nKey, rKey ) )
        ImplProcessConfigItemInt( rStrm, nKey, rData );
    else
        ImplProcessConfigItemStr( rStrm, rKey, rData );
}

// ============================================================================

NameListBase::~NameListBase()
{
}

void NameListBase::SetName( sal_Int64 nKey, const StringWrapper& rNameWrp )
{
    String aName = rNameWrp.GetString();
    StringHelper::TrimQuotes( aName );
    ImplSetName( nKey, aName );
}

void NameListBase::IncludeList( NameListRef xList )
{
    if( xList.is() )
    {
        for( const_iterator aIt = xList->begin(), aEnd = xList->end(); aIt != aEnd; ++aIt )
            maMap[ aIt->first ] = aIt->second;
        ImplIncludeList( *xList );
    }
}

bool NameListBase::ImplIsValid() const
{
    return true;
}

void NameListBase::ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "include" ) )
        Include( rData );
    else if( rKey.EqualsAscii( "exclude" ) )
        Exclude( rData );
    else
        ConfigItemBase::ImplProcessConfigItemStr( rStrm, rKey, rData );
}

void NameListBase::ImplProcessConfigItemInt( SvStream& rStrm, sal_Int64 nKey, const String& rData )
{
    ImplSetName( nKey, rData );
}

void NameListBase::InsertRawName( sal_Int64 nKey, const String& rName )
{
    maMap[ nKey ] = rName;
}

const String* NameListBase::FindRawName( sal_Int64 nKey ) const
{
    const_iterator aIt = maMap.find( nKey );
    return (aIt == end()) ? 0 : &aIt->second;
}

void NameListBase::Include( const String& rListKeys )
{
    ScfStringVec aVec;
    StringHelper::ConvertStringToStringList( aVec, rListKeys, true );
    for( ScfStringVec::const_iterator aIt = aVec.begin(), aEnd = aVec.end(); aIt != aEnd; ++aIt )
        IncludeList( mrCoreData.GetNameList( *aIt ) );
}

void NameListBase::Exclude( const String& rKeys )
{
    ScfInt64Vec aVec;
    StringHelper::ConvertStringToIntList( aVec, rKeys, true );
    for( ScfInt64Vec::const_iterator aIt = aVec.begin(), aEnd = aVec.end(); aIt != aEnd; ++aIt )
        maMap.erase( *aIt );
}

// ============================================================================

ConstList::ConstList( const ConfigCoreData& rCoreData ) :
    NameListBase( rCoreData ),
    maDefName( SCF_DUMP_ERR_NONAME ),
    mbQuoteNames( false )
{
}

void ConstList::ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "default" ) )
        SetDefaultName( rData );
    else if( rKey.EqualsAscii( "quote-names" ) )
        SetQuoteNames( StringHelper::ConvertStringToBool( rData ) );
    else
        NameListBase::ImplProcessConfigItemStr( rStrm, rKey, rData );
}

void ConstList::ImplSetName( sal_Int64 nKey, const String& rName )
{
    InsertRawName( nKey, rName );
}

String ConstList::ImplGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    const String* pName = FindRawName( nKey );
    String aName = pName ? *pName : maDefName;
    if( mbQuoteNames )
        StringHelper::Enclose( aName, '\'' );
    return aName;
}

String ConstList::ImplGetName( const Config& rCfg, double fValue ) const
{
    return String();
}

void ConstList::ImplIncludeList( const NameListBase& rList )
{
    if( const ConstList* pConstList = dynamic_cast< const ConstList* >( &rList ) )
    {
        maDefName = pConstList->maDefName;
        mbQuoteNames = pConstList->mbQuoteNames;
    }
}

// ============================================================================

MultiList::MultiList( const ConfigCoreData& rCoreData ) :
    ConstList( rCoreData ),
    mbIgnoreEmpty( true )
{
}

void MultiList::SetNamesFromVec( sal_Int64 nStartKey, const ScfStringVec& rNames )
{
    sal_Int64 nKey = nStartKey;
    for( ScfStringVec::const_iterator aIt = rNames.begin(), aEnd = rNames.end(); aIt != aEnd; ++aIt, ++nKey )
        if( !mbIgnoreEmpty || (aIt->Len() > 0) )
            InsertRawName( nKey, *aIt );
}

void MultiList::ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "ignore-empty" ) )
        mbIgnoreEmpty = StringHelper::ConvertStringToBool( rData );
    else
        ConstList::ImplProcessConfigItemStr( rStrm, rKey, rData );
}

void MultiList::ImplSetName( sal_Int64 nKey, const String& rName )
{
    ScfStringVec aNames;
    StringHelper::ConvertStringToStringList( aNames, rName, false );
    SetNamesFromVec( nKey, aNames );
}

// ============================================================================

FlagsList::FlagsList( const ConfigCoreData& rCoreData ) :
    NameListBase( rCoreData ),
    mnIgnore( 0 )
{
}

void FlagsList::ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "ignore" ) )
    {
        sal_Int64 nIgnore;
        if( StringHelper::ConvertStringToInt( nIgnore, rData ) )
            SetIgnoreFlags( nIgnore );
    }
    else
        NameListBase::ImplProcessConfigItemStr( rStrm, rKey, rData );
}

void FlagsList::ImplSetName( sal_Int64 nKey, const String& rName )
{
    InsertRawName( nKey, rName );
}

String FlagsList::ImplGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    sal_Int64 nFlags = nKey;
    ::set_flag( nFlags, mnIgnore, false );
    sal_Int64 nFound = 0;
    String aName;
    // add known flags
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        sal_Int64 nMask = aIt->first;
        if( ::get_flag( nFlags, nMask ) )
            StringHelper::AppendToken( aName, aIt->second );
        ::set_flag( nFound, nMask );
    }
    // add unknown flags
    ::set_flag( nFlags, nFound, false );
    if( nFlags != 0 )
    {
        String aUnknown = CREATE_STRING( SCF_DUMP_UNKNOWN );
        aUnknown.Append( SCF_DUMP_ITEMSEP );
        StringHelper::AppendShortHex( aUnknown, nFlags, true );
        StringHelper::Enclose( aUnknown, '(', ')' );
        StringHelper::AppendToken( aName, aUnknown );
    }
    return aName;
}

String FlagsList::ImplGetName( const Config& rCfg, double fValue ) const
{
    return String();
}

void FlagsList::ImplIncludeList( const NameListBase& rList )
{
    if( const FlagsList* pFlagsList = dynamic_cast< const FlagsList* >( &rList ) )
        mnIgnore = pFlagsList->mnIgnore;
}

// ============================================================================

CombiList::CombiList( const ConfigCoreData& rCoreData ) :
    FlagsList( rCoreData )
{
}

void CombiList::ImplSetName( sal_Int64 nKey, const String& rName )
{
    if( (nKey & (nKey - 1)) != 0 )  // more than a single bit set?
        maFmtMap[ nKey ].Parse( rName );
    else
        FlagsList::ImplSetName( nKey, rName );
}

String CombiList::ImplGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    sal_Int64 nFlags = nKey;
    sal_Int64 nFound = 0;
    String aName;
    // add known flag fields
    for( ItemFormatMap::const_iterator aIt = maFmtMap.begin(), aEnd = maFmtMap.end(); aIt != aEnd; ++aIt )
    {
        sal_Int64 nMask = aIt->first;
        if( nMask != 0 )
        {
            const ItemFormat& rItemFmt = aIt->second;

            sal_uInt64 nUFlags = static_cast< sal_uInt64 >( nFlags );
            sal_uInt64 nUMask = static_cast< sal_uInt64 >( nMask );
            while( (nUMask & 1) == 0 ) { nUFlags >>= 1; nUMask >>= 1; }

            String aItem = rItemFmt.maItemName;

            if( (nUMask & (nUMask + 1)) == 0 )  // all bits in one block
            {
                sal_uInt64 nUValue = nUFlags & nUMask;
                sal_Int64 nSValue = static_cast< sal_Int64 >( nUValue );
                if( ::get_flag< sal_uInt64 >( nUValue, (nUMask + 1) >> 1 ) )
                    ::set_flag( nSValue, static_cast< sal_Int64 >( ~nUMask ) );

                String aValue;
                switch( rItemFmt.meDataType )
                {
                    case DATATYPE_INT8:     StringHelper::AppendValue( aValue, static_cast< sal_Int8 >( nSValue ), rItemFmt.meFmtType );    break;
                    case DATATYPE_UINT8:    StringHelper::AppendValue( aValue, static_cast< sal_uInt8 >( nUValue ), rItemFmt.meFmtType );   break;
                    case DATATYPE_INT16:    StringHelper::AppendValue( aValue, static_cast< sal_Int16 >( nSValue ), rItemFmt.meFmtType );   break;
                    case DATATYPE_UINT16:   StringHelper::AppendValue( aValue, static_cast< sal_uInt16 >( nUValue ), rItemFmt.meFmtType );  break;
                    case DATATYPE_INT32:    StringHelper::AppendValue( aValue, static_cast< sal_Int32 >( nSValue ), rItemFmt.meFmtType );   break;
                    case DATATYPE_UINT32:   StringHelper::AppendValue( aValue, static_cast< sal_uInt32 >( nUValue ), rItemFmt.meFmtType );  break;
                    case DATATYPE_INT64:    StringHelper::AppendValue( aValue, nSValue, rItemFmt.meFmtType );                               break;
                    case DATATYPE_UINT64:   StringHelper::AppendValue( aValue, nUValue, rItemFmt.meFmtType );                               break;
                    case DATATYPE_FLOAT:    StringHelper::AppendValue( aValue, static_cast< float >( nSValue ), rItemFmt.meFmtType );       break;
                    case DATATYPE_DOUBLE:   StringHelper::AppendValue( aValue, static_cast< double >( nSValue ), rItemFmt.meFmtType );      break;
                    default:;
                }
                StringHelper::AppendToken( aItem, aValue, SCF_DUMP_ITEMSEP );
                if( rItemFmt.maListName.Len() > 0 )
                {
                    String aName = rCfg.GetName( rItemFmt.maListName, static_cast< sal_Int64 >( nUValue ) );
                    StringHelper::AppendToken( aItem, aName, SCF_DUMP_ITEMSEP );
                }
            }
            else
            {
                String aValue;
                StringHelper::AppendShortHex( aValue, nFlags & nMask );
                StringHelper::AppendToken( aItem, aValue, SCF_DUMP_ITEMSEP );
            }
            StringHelper::Enclose( aItem, '(', ')' );
            StringHelper::AppendToken( aName, aItem );
            ::set_flag( nFound, nMask );
        }
    }
    ::set_flag( nFlags, nFound, false );
    StringHelper::AppendToken( aName, FlagsList::ImplGetName( rCfg, nFlags ) );
    return aName;
}

void CombiList::ImplIncludeList( const NameListBase& rList )
{
    if( const CombiList* pCombiList = dynamic_cast< const CombiList* >( &rList ) )
        maFmtMap = pCombiList->maFmtMap;
    FlagsList::ImplIncludeList( rList );
}

// ============================================================================

UnitConverter::UnitConverter( const ConfigCoreData& rCoreData ) :
    NameListBase( rCoreData ),
    mfFactor( 1.0 )
{
}

void UnitConverter::ImplSetName( sal_Int64 nKey, const String& rName )
{
    // nothing to do
}

String UnitConverter::ImplGetName( const Config& rCfg, sal_Int64 nKey ) const
{
    return ImplGetName( rCfg, static_cast< double >( nKey ) );
}

String UnitConverter::ImplGetName( const Config& rCfg, double fValue ) const
{
    String aValue;
    StringHelper::AppendDec( aValue, mfFactor * fValue );
    aValue.Append( maUnitName );
    return aValue;
}

void UnitConverter::ImplIncludeList( const NameListBase& rList )
{
}

// ============================================================================

NameListRef NameListWrapper::GetNameList( const Config& rCfg ) const
{
    return mxList.is() ? mxList : (mxList = rCfg.GetNameList( maNameWrp.GetString() ));
}

// ============================================================================
// ============================================================================

ConfigCoreData::ConfigCoreData( const String& rFileName )
{
    Construct( rFileName );
}

ConfigCoreData::~ConfigCoreData()
{
}

void ConfigCoreData::Construct( const String& rFileName )
{
    SvFileStream aStrm( rFileName, STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE );
    mbLoaded = aStrm.IsOpen();
    if( mbLoaded )
        ReadConfigBlockContents( aStrm );
}

void ConfigCoreData::SetOption( const String& rKey, const String& rData )
{
    maConfigData[ rKey ] = rData;
}

const String* ConfigCoreData::GetOption( const String& rKey ) const
{
    ConfigDataMap::const_iterator aIt = maConfigData.find( rKey );
    return (aIt == maConfigData.end()) ? 0 : &aIt->second;
}

void ConfigCoreData::SetNameList( const String& rListName, NameListRef xList )
{
    if( rListName.Len() > 0 )
        maNameLists[ rListName ] = xList;
}

void ConfigCoreData::EraseNameList( const String& rListName )
{
    maNameLists.erase( rListName );
}

NameListRef ConfigCoreData::GetNameList( const String& rListName ) const
{
    NameListRef xList;
    NameListMap::const_iterator aIt = maNameLists.find( rListName );
    if( aIt != maNameLists.end() )
        xList = aIt->second;
    return xList;
}

bool ConfigCoreData::ImplIsValid() const
{
    return mbLoaded;
}

void ConfigCoreData::ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "constlist" ) )
        ReadNameList< ConstList >( rStrm, rData );
    else if( rKey.EqualsAscii( "multilist" ) )
        ReadNameList< MultiList >( rStrm, rData );
    else if( rKey.EqualsAscii( "flagslist" ) )
        ReadNameList< FlagsList >( rStrm, rData );
    else if( rKey.EqualsAscii( "combilist" ) )
        ReadNameList< CombiList >( rStrm, rData );
    else if( rKey.EqualsAscii( "shortlist" ) )
        CreateShortList( rData );
    else if( rKey.EqualsAscii( "unitconverter" ) )
        CreateUnitConverter( rData );
    else
        SetOption( rKey, rData );
}

void ConfigCoreData::CreateShortList( const String& rData )
{
    ScfStringVec aDataVec;
    StringHelper::ConvertStringToStringList( aDataVec, rData, false );
    if( aDataVec.size() >= 3 )
    {
        sal_Int64 nStartKey;
        if( StringHelper::ConvertStringToInt( nStartKey, aDataVec[ 1 ] ) )
        {
            ScfRef< MultiList > xList = CreateNameList< MultiList >( aDataVec[ 0 ] );
            if( xList.is() )
            {
                aDataVec.erase( aDataVec.begin(), aDataVec.begin() + 2 );
                xList->SetNamesFromVec( nStartKey, aDataVec );
            }
        }
    }
}

void ConfigCoreData::CreateUnitConverter( const String& rData )
{
    ScfStringVec aDataVec;
    StringHelper::ConvertStringToStringList( aDataVec, rData, false );
    if( aDataVec.size() >= 3 )
    {
        String aFactor = aDataVec[ 1 ];
        bool bRecip = (aFactor.Len() > 0) && (aFactor.GetChar( 0 ) == '/');
        if( bRecip )
            aFactor.Erase( 0, 1 );
        double fFactor;
        if( StringHelper::ConvertStringToDouble( fFactor, aFactor ) && (fFactor != 0.0) )
        {
            ScfRef< UnitConverter > xList = CreateNameList< UnitConverter >( aDataVec[ 0 ] );
            if( xList.is() )
            {
                xList->SetFactor( bRecip ? (1.0 / fFactor) : fFactor );
                xList->SetUnitName( aDataVec[ 2 ] );
            }
        }
    }
}

// ============================================================================

Config::Config( const Config& rParent )
{
    Construct( rParent );
}

Config::Config( const String& rFileName )
{
    Construct( rFileName );
}

Config::Config( const sal_Char* pcEnvVar )
{
    Construct( pcEnvVar );
}

Config::~Config()
{
}

void Config::Construct( const Config& rParent )
{
    *this = rParent;
}

void Config::Construct( const String& rFileName )
{
    mxCoreData.reset( new ConfigCoreData( rFileName ) );
}

void Config::Construct( const sal_Char* pcEnvVar )
{
    if( pcEnvVar )
        if( const sal_Char* pcFileName = ::getenv( pcEnvVar ) )
            Construct( String::CreateFromAscii( pcFileName ) );
}

void Config::SetStringOption( const StringWrapper& rKey, const StringWrapper& rData )
{
    mxCoreData->SetOption( rKey.GetString(), rData.GetString() );
}

const String& Config::GetStringOption( const StringWrapper& rKey, const String& rDefault ) const
{
    const String* pData = ImplGetOption( rKey.GetString() );
    return pData ? *pData : rDefault;
}

bool Config::GetBoolOption( const StringWrapper& rKey, bool bDefault ) const
{
    const String* pData = ImplGetOption( rKey.GetString() );
    return pData ? StringHelper::ConvertStringToBool( *pData ) : bDefault;
}

bool Config::IsDumperEnabled() const
{
    return GetBoolOption( "enable-dumper", false );
}

bool Config::IsImportEnabled() const
{
    return GetBoolOption( "enable-import", true );
}

void Config::SetNameList( const StringWrapper& rListName, NameListRef xList )
{
    mxCoreData->SetNameList( rListName.GetString(), xList );
}

void Config::EraseNameList( const StringWrapper& rListName )
{
    mxCoreData->EraseNameList( rListName.GetString() );
}

NameListRef Config::GetNameList( const StringWrapper& rListName ) const
{
    return ImplGetNameList( rListName.GetString() );
}

bool Config::ImplIsValid() const
{
    return IsValid( mxCoreData );
}

const String* Config::ImplGetOption( const String& rKey ) const
{
    return mxCoreData->GetOption( rKey );
}

NameListRef Config::ImplGetNameList( const String& rListName ) const
{
    return mxCoreData->GetNameList( rListName );
}

// ============================================================================
// ============================================================================

CoreData::CoreData( SfxMedium& rMedium, SfxObjectShell* pDocShell ) :
    mrMedium( rMedium ),
    mpDocShell( pDocShell )
{
}

SvStream& CoreData::GetCoreStream()
{
    return *mrMedium.GetInStream();
}

bool CoreData::ImplIsValid() const
{
    return mrMedium.GetInStream() != 0;
}

// ============================================================================
// ============================================================================

bool Input::ImplIsValid() const
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

SvStreamInput::SvStreamInput( SvStream& rStrm ) :
    mrStrm( rStrm )
{
}

SvStreamInput::~SvStreamInput()
{
}

sal_Size SvStreamInput::Size() const
{
    sal_Size nSize = 0;
    if( mrStrm.GetErrorCode() == SVSTREAM_OK )
    {
        sal_Size nPos = mrStrm.Tell();
        mrStrm.Seek( STREAM_SEEK_TO_END );
        nSize = mrStrm.Tell();
        mrStrm.Seek( nPos );
    }
    return nSize;
}

sal_Size SvStreamInput::Tell() const
{
    return mrStrm.Tell();
}

void SvStreamInput::Seek( sal_Size nPos )
{
    mrStrm.Seek( nPos );
}

void SvStreamInput::SeekRel( sal_sSize nRelPos )
{
    mrStrm.SeekRel( nRelPos );
}

sal_Size SvStreamInput::Read( void* pBuffer, sal_Size nSize )
{
    return mrStrm.Read( pBuffer, nSize );
}

void SvStreamInput::ReadLine( String& rLine, rtl_TextEncoding eEnc )
{
    mrStrm.ReadByteStringLine( rLine, eEnc );
}

SvStreamInput& SvStreamInput::operator>>( sal_Int8& rnData )   { mrStrm >> rnData; return *this; }
SvStreamInput& SvStreamInput::operator>>( sal_uInt8& rnData )  { mrStrm >> rnData; return *this; }
SvStreamInput& SvStreamInput::operator>>( sal_Int16& rnData )  { mrStrm >> rnData; return *this; }
SvStreamInput& SvStreamInput::operator>>( sal_uInt16& rnData ) { mrStrm >> rnData; return *this; }
SvStreamInput& SvStreamInput::operator>>( sal_Int32& rnData )  { mrStrm >> rnData; return *this; }
SvStreamInput& SvStreamInput::operator>>( sal_uInt32& rnData ) { mrStrm >> rnData; return *this; }
SvStreamInput& SvStreamInput::operator>>( float& rfData )      { mrStrm >> rfData; return *this; }
SvStreamInput& SvStreamInput::operator>>( double& rfData )     { mrStrm >> rfData; return *this; }

// ============================================================================
// ============================================================================

Output::Output( SvStream& rStrm ) :
    mrStrm( rStrm ),
    mnCol( 0 ),
    mnItemLevel( 0 ),
    mnMultiLevel( 0 ),
    mnItemIdx( 0 ),
    mnLastItem( 0 )
{
    WriteChar( SCF_UTF8_BOM );
    WriteAscii( "OpenOffice.org generic file dumper v1.0" );
    NewLine();
    EmptyLine();
}

// ----------------------------------------------------------------------------

void Output::NewLine()
{
    if( maLine.Len() > 0 )
    {
        mrStrm << maPrefixUtf8.GetBuffer() << maIndent.GetBuffer();
        mrStrm << StringHelper::ConvertToUtf8( maLine ).GetBuffer() << '\n';
        maLine.Erase();
        mnCol = 0;
        mnLastItem = 0;
    }
}

void Output::EmptyLine( size_t nCount )
{
    for( size_t nIdx = 0; nIdx < nCount; ++nIdx )
        mrStrm << maPrefixUtf8.GetBuffer() << '\n';
}

void Output::SetPrefix( const String& rPrefix )
{
    maPrefix = rPrefix;
    maPrefixUtf8 = StringHelper::ConvertToUtf8( rPrefix );
}

void Output::IncIndent()
{
    maIndent.Expand( maIndent.Len() + SCF_DUMP_INDENT, ' ' );
}

void Output::DecIndent()
{
    if( maIndent.Len() >= SCF_DUMP_INDENT )
        maIndent.Erase( 0, SCF_DUMP_INDENT );
}

void Output::ResetIndent()
{
    maIndent.Erase();
}

void Output::StartTable( xub_StrLen nW1 )
{
    StartTable( 1, &nW1 );
}

void Output::StartTable( xub_StrLen nW1, xub_StrLen nW2 )
{
    xub_StrLen pnColWidths[ 2 ];
    pnColWidths[ 0 ] = nW1;
    pnColWidths[ 1 ] = nW2;
    StartTable( 2, pnColWidths );
}

void Output::StartTable( xub_StrLen nW1, xub_StrLen nW2, xub_StrLen nW3 )
{
    xub_StrLen pnColWidths[ 3 ];
    pnColWidths[ 0 ] = nW1;
    pnColWidths[ 1 ] = nW2;
    pnColWidths[ 2 ] = nW3;
    StartTable( 3, pnColWidths );
}

void Output::StartTable( xub_StrLen nW1, xub_StrLen nW2, xub_StrLen nW3, xub_StrLen nW4 )
{
    xub_StrLen pnColWidths[ 4 ];
    pnColWidths[ 0 ] = nW1;
    pnColWidths[ 1 ] = nW2;
    pnColWidths[ 2 ] = nW3;
    pnColWidths[ 3 ] = nW4;
    StartTable( 4, pnColWidths );
}

void Output::StartTable( size_t nColCount, const xub_StrLen* pnColWidths )
{
    maColPos.clear();
    maColPos.push_back( 0 );
    xub_StrLen nColPos = 0;
    for( size_t nCol = 0; nCol < nColCount; ++nCol )
        maColPos.push_back( nColPos += pnColWidths[ nCol ] );
}

void Output::Tab()
{
    Tab( mnCol + 1 );
}

void Output::Tab( size_t nCol )
{
    mnCol = nCol;
    if( mnCol < maColPos.size() )
    {
        xub_StrLen nColPos = maColPos[ mnCol ];
        maLine.Erase( (nColPos == 0) ? 0 : (nColPos - 1) );
        maLine.Expand( nColPos, ' ' );
    }
    else
        WriteChar( ' ', 2 );
}

void Output::EndTable()
{
    maColPos.clear();
}

void Output::ResetItemIndex( sal_Int64 nIdx )
{
    mnItemIdx = nIdx;
}

void Output::StartItem( const sal_Char* pcName )
{
    if( mnItemLevel == 0 )
    {
        if( (mnMultiLevel > 0) && (maLine.Len() > 0) )
            Tab();
        if( pcName )
        {
            WriteItemName( pcName );
            WriteChar( SCF_DUMP_ITEMSEP );
        }
    }
    ++mnItemLevel;
    mnLastItem = maLine.Len();
}

void Output::ContItem()
{
    if( mnItemLevel > 0 )
    {
        WriteChar( SCF_DUMP_ITEMSEP );
        mnLastItem = maLine.Len();
    }
}

void Output::EndItem()
{
    if( mnItemLevel > 0 )
    {
        maLastItem = maLine.Copy( mnLastItem );
        if( (maLastItem.Len() == 0) && (mnLastItem > 0) && (maLine.GetChar( mnLastItem - 1 ) == SCF_DUMP_ITEMSEP) )
            maLine.Erase( mnLastItem - 1 );
        --mnItemLevel;
    }
    if( mnItemLevel == 0 )
    {
        if( mnMultiLevel == 0 )
            NewLine();
    }
    else
        ContItem();
}

void Output::StartMultiItems()
{
    ++mnMultiLevel;
}

void Output::EndMultiItems()
{
    if( mnMultiLevel > 0 )
        --mnMultiLevel;
    if( mnMultiLevel == 0 )
        NewLine();
}

// ----------------------------------------------------------------------------

void Output::WriteChar( sal_Unicode cChar, xub_StrLen nCount )
{
    StringHelper::AppendEncChar( maLine, cChar, nCount );
}

void Output::WriteAscii( const sal_Char* pcStr )
{
    if( pcStr )
        maLine.AppendAscii( pcStr );
}

void Output::WriteString( const String& rStr )
{
    StringHelper::AppendEncString( maLine, rStr );
}

void Output::WriteArray( const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep )
{
    const sal_uInt8* pnEnd = pnData ? (pnData + nSize) : 0;
    for( const sal_uInt8* pnByte = pnData; pnByte < pnEnd; ++pnByte )
    {
        if( pnByte > pnData )
            WriteChar( cSep );
        WriteHex( *pnByte, false );
    }
}

void Output::WriteBool( bool bData )
{
    StringHelper::AppendBool( maLine, bData );
}

void Output::WriteColor( const Color& rColor )
{
    WriteChar( 'a' );
    WriteDec( rColor.GetTransparency() );
    WriteAscii( ",r" );
    WriteDec( rColor.GetRed() );
    WriteAscii( ",g" );
    WriteDec( rColor.GetGreen() );
    WriteAscii( ",b" );
    WriteDec( rColor.GetBlue() );
}

void Output::WriteDateTime( const DateTime& rDateTime )
{
    WriteDec( rDateTime.GetYear(), 4, '0' );
    WriteChar( '-' );
    WriteDec( rDateTime.GetMonth(), 2, '0' );
    WriteChar( '-' );
    WriteDec( rDateTime.GetDay(), 2, '0' );
    WriteChar( 'T' );
    WriteDec( rDateTime.GetHour(), 2, '0' );
    WriteChar( ':' );
    WriteDec( rDateTime.GetMin(), 2, '0' );
    WriteChar( ':' );
    WriteDec( rDateTime.GetSec(), 2, '0' );
}

// ----------------------------------------------------------------------------

bool Output::ImplIsValid() const
{
    return true;
}

void Output::WriteItemName( const sal_Char* pcName )
{
    if( pcName && (*pcName == '#') )
    {
        WriteAscii( pcName + 1 );
        StringHelper::AppendIndex( maLine, mnItemIdx++ );
    }
    else
        WriteAscii( pcName );
}

// ============================================================================
// ============================================================================

ObjectBase::~ObjectBase()
{
}

void ObjectBase::Construct( ConfigRef xConfig, CoreDataRef xCore, OutputRef xOut )
{
    mxConfig = xConfig;
    mxCore = xCore;
    mxOut = xOut;
}

void ObjectBase::Construct( const ObjectBase& rParent )
{
    Construct( rParent.mxConfig, rParent.mxCore, rParent.mxOut );
}

SfxMedium& ObjectBase::GetCoreMedium() const
{
    return Core().GetMedium();
}

SvStream& ObjectBase::GetCoreStream() const
{
    return *Core().GetMedium().GetInStream();
}

void ObjectBase::Dump()
{
    if( IsValid() )
    {
        ImplDumpHeader();
        ImplDumpBody();
        ImplDumpFooter();
    }
}

bool ObjectBase::ImplIsValid() const
{
    return IsValid( mxConfig ) && IsValid( mxCore ) && IsValid( mxOut );
}

ConfigRef ObjectBase::ImplReconstructConfig()
{
    return mxConfig;
}

OutputRef ObjectBase::ImplReconstructOutput()
{
    return mxOut;
}

void ObjectBase::ImplDumpHeader()
{
}

void ObjectBase::ImplDumpBody()
{
}

void ObjectBase::ImplDumpFooter()
{
}

void ObjectBase::ReconstructConfig()
{
    mxConfig = ImplReconstructConfig();
}

void ObjectBase::ReconstructOutput()
{
    mxOut = ImplReconstructOutput();
}

void ObjectBase::WriteEmptyItem( const sal_Char* pcName )
{
    ItemGuard aItem( *mxOut, pcName );
}

void ObjectBase::WriteInfoItem( const sal_Char* pcName, const StringWrapper& rData )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteString( rData.GetString() );
}

void ObjectBase::WriteStringItem( const sal_Char* pcName, const String& rData )
{
    ItemGuard aItem( *mxOut, pcName );
    String aValue = rData.Copy( 0, SCF_DUMP_MAXSTRLEN );
    StringHelper::Enclose( aValue, '\'' );
    mxOut->WriteString( aValue );
    if( rData.Len() > SCF_DUMP_MAXSTRLEN )
    {
        mxOut->WriteAscii( " (cut,len=" );
        mxOut->WriteDec( static_cast< sal_Int32 >( rData.Len() ) );
        mxOut->WriteChar( ')' );
    }
}

void ObjectBase::WriteArrayItem( const sal_Char* pcName, const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteArray( pnData, nSize, cSep );
}

void ObjectBase::WriteBoolItem( const sal_Char* pcName, bool bData )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteBool( bData );
}

void ObjectBase::WriteColorItem( const sal_Char* pcName, const Color& rColor )
{
    ItemGuard aItem( *mxOut, pcName );
    WriteHexItem( pcName, rColor.GetColor() );
    mxOut->WriteColor( rColor );
}

void ObjectBase::WriteDateTimeItem( const sal_Char* pcName, const DateTime& rDateTime )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteDateTime( rDateTime );
}

void ObjectBase::WriteGuidItem( const sal_Char* pcName, const String& rGuid )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteString( rGuid );
    aItem.Cont();
    mxOut->WriteString( Cfg().GetStringOption( rGuid, String::EmptyString() ) );
}

// ============================================================================

InputObjectBase::~InputObjectBase()
{
}

void InputObjectBase::Construct( const ObjectBase& rParent, InputRef xIn )
{
    ObjectBase::Construct( rParent );
    mxIn = xIn;
}

void InputObjectBase::Construct( InputObjectBase& rParent )
{
    Construct( rParent, rParent.mxIn );
}

bool InputObjectBase::ImplIsValid() const
{
    return IsValid( mxIn ) && ObjectBase::ImplIsValid();
}

InputRef InputObjectBase::ImplReconstructInput()
{
    return mxIn;
}

void InputObjectBase::ImplDumpBody()
{
    DumpBinaryStream();
}

void InputObjectBase::ReconstructInput()
{
    mxIn = ImplReconstructInput();
}

void InputObjectBase::SkipBlock( sal_Size nSize, bool bShowSize )
{
    sal_Size nEndPos = ::std::min( mxIn->Tell() + nSize, mxIn->Size() );
    if( mxIn->Tell() < nEndPos )
    {
        if( bShowSize )
            WriteDecItem( "skipped-data-size", static_cast< sal_uInt64 >( nEndPos - mxIn->Tell() ) );
        mxIn->Seek( nEndPos );
    }
}

void InputObjectBase::DumpRawBinary( sal_Size nSize, bool bShowOffset, bool bStream )
{
    Output& rOut = Out();
    TableGuard aTabGuard( rOut,
        bShowOffset ? 12 : 0,
        3 * SCF_DUMP_BYTESPERLINE / 2 + 1,
        3 * SCF_DUMP_BYTESPERLINE / 2 + 1,
        SCF_DUMP_BYTESPERLINE / 2 + 1 );

    sal_Size nMaxShowSize = Cfg().GetIntOption< sal_Size >(
        bStream ? "max-binary-stream-size" : "max-binary-data-size", STREAM_SEEK_TO_END );

    sal_Size nEndPos = ::std::min( mxIn->Tell() + nSize, mxIn->Size() );
    sal_Size nDumpEnd = ::std::min( mxIn->Tell() + nMaxShowSize, nEndPos );

    while( mxIn->Tell() < nDumpEnd )
    {
        rOut.WriteHex( static_cast< sal_uInt32 >( mxIn->Tell() ) );
        rOut.Tab();

        sal_uInt8 pnLineData[ SCF_DUMP_BYTESPERLINE ];
        sal_Size nLineSize = ::std::min( nDumpEnd - mxIn->Tell(), SCF_DUMP_BYTESPERLINE );
        mxIn->Read( pnLineData, nLineSize );

        const sal_uInt8* pnByte = 0;
        const sal_uInt8* pnEnd = 0;
        for( pnByte = pnLineData, pnEnd = pnLineData + nLineSize; pnByte != pnEnd; ++pnByte )
        {
            if( (pnByte - pnLineData) == (SCF_DUMP_BYTESPERLINE / 2) ) rOut.Tab();
            rOut.WriteHex( *pnByte, false );
            rOut.WriteChar( ' ' );
        }

        aTabGuard.Tab( 3 );
        for( pnByte = pnLineData, pnEnd = pnLineData + nLineSize; pnByte != pnEnd; ++pnByte )
        {
            if( (pnByte - pnLineData) == (SCF_DUMP_BYTESPERLINE / 2) ) rOut.Tab();
            rOut.WriteChar( static_cast< sal_Unicode >( (*pnByte < 0x20) ? '.' : *pnByte ) );
        }
        rOut.NewLine();
    }

    // skip undumped data
    SkipBlock( nEndPos - mxIn->Tell() );
}

void InputObjectBase::DumpBinary( const sal_Char* pcName, sal_Size nSize, bool bShowOffset )
{
    WriteEmptyItem( pcName );
    IndentGuard aIndGuard( Out() );
    DumpRawBinary( nSize, bShowOffset );
}

void InputObjectBase::DumpArray( const sal_Char* pcName, sal_Size nSize, sal_Unicode cSep )
{
    sal_Size nDumpSize = ::std::min( nSize, mxIn->Size() - mxIn->Tell() );
    if( nDumpSize > SCF_DUMP_MAXARRAY )
    {
        DumpBinary( pcName, nSize, false );
    }
    else if( nDumpSize > 1 )
    {
        sal_uInt8 pnData[ SCF_DUMP_MAXARRAY ];
        mxIn->Read( pnData, nDumpSize );
        WriteArrayItem( pcName, pnData, nDumpSize, cSep );
    }
    else if( nDumpSize == 1 )
        DumpHex< sal_uInt8 >( pcName );
}

void InputObjectBase::DumpRemaining( sal_Size nSize )
{
    if( nSize > 0 )
    {
        if( Cfg().GetBoolOption( "show-trailing-unknown", true ) )
            DumpBinary( "remaining-data", nSize, false );
        else
            SkipBlock( nSize );
    }
}

void InputObjectBase::DumpBinaryStream( bool bShowOffset )
{
    mxIn->Seek( STREAM_SEEK_TO_BEGIN );
    DumpRawBinary( mxIn->Size(), bShowOffset, true );
    Out().EmptyLine();
}

void InputObjectBase::DumpTextStream( rtl_TextEncoding eEnc, bool bShowLines )
{
    Output& rOut = Out();
    TableGuard aTabGuard( rOut, bShowLines ? 8 : 0 );

    mxIn->Seek( STREAM_SEEK_TO_BEGIN );

    sal_uInt32 nLine = 0;
    while( mxIn->IsValidPos() )
    {
        String aLine;
        mxIn->ReadLine( aLine, eEnc );
        rOut.WriteDec( ++nLine, 6 );
        rOut.Tab();
        rOut.WriteString( aLine );
        rOut.NewLine();
    }
    rOut.EmptyLine();
}

String InputObjectBase::DumpGuid( const sal_Char* pcName )
{
    sal_uInt8 pnData[ 16 ];
    mxIn->Read( pnData, sizeof( pnData ) );
    String aGuid;
    StringHelper::AppendHex( aGuid, SVBT32ToUInt32( pnData ), false );
    aGuid.Append( '-' );
    StringHelper::AppendHex( aGuid, SVBT16ToShort( pnData + 4 ), false );
    aGuid.Append( '-' );
    StringHelper::AppendHex( aGuid, SVBT16ToShort( pnData + 6 ), false );
    aGuid.Append( '-' );
    StringHelper::AppendHex( aGuid, pnData[ 8 ], false );
    StringHelper::AppendHex( aGuid, pnData[ 9 ], false );
    aGuid.Append( '-' );
    StringHelper::AppendHex( aGuid, pnData[ 10 ], false );
    StringHelper::AppendHex( aGuid, pnData[ 11 ], false );
    StringHelper::AppendHex( aGuid, pnData[ 12 ], false );
    StringHelper::AppendHex( aGuid, pnData[ 13 ], false );
    StringHelper::AppendHex( aGuid, pnData[ 14 ], false );
    StringHelper::AppendHex( aGuid, pnData[ 15 ], false );
    WriteGuidItem( pcName, aGuid );
    return aGuid;
}

void InputObjectBase::DumpItem( const ItemFormat& rItemFmt )
{
    switch( rItemFmt.meDataType )
    {
        case DATATYPE_INT8:    DumpValue< sal_Int8 >( rItemFmt );   break;
        case DATATYPE_UINT8:   DumpValue< sal_uInt8 >( rItemFmt );  break;
        case DATATYPE_INT16:   DumpValue< sal_Int16 >( rItemFmt );  break;
        case DATATYPE_UINT16:  DumpValue< sal_uInt16 >( rItemFmt ); break;
        case DATATYPE_INT32:   DumpValue< sal_Int32 >( rItemFmt );  break;
        case DATATYPE_UINT32:  DumpValue< sal_uInt32 >( rItemFmt ); break;
        case DATATYPE_INT64:   DumpValue< sal_Int64 >( rItemFmt );  break;
        case DATATYPE_UINT64:  DumpValue< sal_uInt64 >( rItemFmt ); break;
        case DATATYPE_FLOAT:   DumpValue< float >( rItemFmt );      break;
        case DATATYPE_DOUBLE:  DumpValue< double >( rItemFmt );     break;
    }
}

// ============================================================================
// ============================================================================

StreamObjectBase::~StreamObjectBase()
{
}

void StreamObjectBase::Construct( const ObjectBase& rParent, SvStream& rStrm,
        const String& rPath, const String& rStrmName, InputRef xIn )
{
    mpStrm = &rStrm;
    maPath = rPath;
    maName = rStrmName;
    InputObjectBase::Construct( rParent, xIn );
}

void StreamObjectBase::Construct( const ObjectBase& rParent,
        SvStream& rStrm, const String& rPath, const String& rStrmName )
{
    InputRef xIn( new SvStreamInput( rStrm ) );
    Construct( rParent, rStrm, rPath, rStrmName, xIn );
}

void StreamObjectBase::Construct( const ObjectBase& rParent, SvStream& rStrm )
{
    Construct( rParent, rStrm, String::EmptyString(), String::EmptyString() );
}

String StreamObjectBase::GetFullName() const
{
    return String( maPath ).Append( '/' ).Append( maName );
}

sal_Size StreamObjectBase::GetStreamSize() const
{
    sal_Size nPos = mpStrm->Tell();
    mpStrm->Seek( STREAM_SEEK_TO_END );
    sal_Size nSize = mpStrm->Tell();
    mpStrm->Seek( nPos );
    return nSize;
}

bool StreamObjectBase::ImplIsValid() const
{
    return mpStrm && InputObjectBase::ImplIsValid();
}

void StreamObjectBase::ImplDumpHeader()
{
    Output& rOut = Out();
    rOut.ResetIndent();
    rOut.WriteChar( '+' );
    rOut.WriteChar( '-', 77 );
    rOut.NewLine();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "|" ) );
        WriteEmptyItem( "STREAM-BEGIN" );
        DumpStreamInfo( true );
        rOut.EmptyLine();
    }
    rOut.EmptyLine();
}

void StreamObjectBase::ImplDumpFooter()
{
    Output& rOut = Out();
    rOut.ResetIndent();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "|" ) );
        rOut.EmptyLine();
        DumpStreamInfo( false );
        WriteEmptyItem( "STREAM-END" );
    }
    rOut.WriteChar( '+' );
    rOut.WriteChar( '-', 77 );
    rOut.NewLine();
    rOut.EmptyLine();
}

void StreamObjectBase::ImplDumpExtendedHeader()
{
    WriteDecItem( "stream-size", static_cast< sal_uInt64 >( GetStreamSize() ) );
}

void StreamObjectBase::DumpStreamInfo( bool bExtended )
{
    IndentGuard aIndGuard( Out() );
    WriteStringItem( "stream-name", maName );
    WriteStringItem( "full-path", GetFullName() );
    if( bExtended )
        ImplDumpExtendedHeader();
}

// ============================================================================

SvStreamObject::SvStreamObject( const ObjectBase& rParent, SvStream& rStrm )
{
    Construct( rParent, rStrm );
}

SvStreamObject::~SvStreamObject()
{
}

void SvStreamObject::Construct( const ObjectBase& rParent, SvStream& rStrm )
{
    StreamObjectBase::Construct( rParent, rStrm );
}

// ============================================================================

WrappedStreamObject::WrappedStreamObject( const ObjectBase& rParent, StreamObjectRef xStrmObj )
{
    Construct( rParent, xStrmObj );
}

WrappedStreamObject::~WrappedStreamObject()
{
}

void WrappedStreamObject::Construct( const ObjectBase& rParent, StreamObjectRef xStrmObj )
{
    mxStrmObj = xStrmObj;
    if( IsValid( mxStrmObj ) )
        StreamObjectBase::Construct( rParent, mxStrmObj->GetStream(),
            mxStrmObj->GetStreamPath(), mxStrmObj->GetStreamName() );
}

bool WrappedStreamObject::ImplIsValid() const
{
    return IsValid( mxStrmObj ) && StreamObjectBase::ImplIsValid();
}

// ============================================================================
// ============================================================================

DumperBase::~DumperBase()
{
}

void DumperBase::Construct( ConfigRef xConfig, CoreDataRef xCore )
{
    if( IsValid( xConfig ) && IsValid( xCore ) && xConfig->IsDumperEnabled() )
    {
        String aOutName = xCore->GetMedium().GetPhysicalName();
        aOutName.AppendAscii( ".txt" );
        mxOutStrm.reset( new SvFileStream( aOutName, STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC ) );
        if( mxOutStrm->GetError() == SVSTREAM_OK )
            ObjectBase::Construct( xConfig, xCore, OutputRef( new Output( *mxOutStrm ) ) );
    }
}

void DumperBase::Construct( ConfigRef xConfig, SfxMedium& rMedium, SfxObjectShell* pDocShell )
{
    CoreDataRef xCore( new CoreData( rMedium, pDocShell ) );
    Construct( xConfig, xCore );
}

bool DumperBase::IsImportEnabled() const
{
    return !IsValid() || Cfg().IsImportEnabled();
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif

