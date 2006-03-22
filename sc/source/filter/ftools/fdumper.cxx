/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fdumper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-22 12:05:29 $
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
#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif

namespace scf {
namespace dump {

const sal_Unicode SCF_UTF8_BOM          = 0xFEFF;
const xub_StrLen SCF_DUMP_INDENT        = 2;
const sal_Unicode SCF_DUMP_BINDOT       = '.';
const sal_Size SCF_DUMP_BYTESPERLINE    = 16;

// ============================================================================
// ============================================================================

void StringHelper::AppendString( String& rStr, const String& rData, xub_StrLen nWidth )
{
    if( rData.Len() < nWidth )
        rStr.Expand( rStr.Len() + nWidth - rData.Len(), ' ' );
    rStr.Append( rData );
}

void StringHelper::AppendToken( String& rStr, const String& rToken, sal_Unicode cSep )
{
    if( (rStr.Len() > 0) && (rToken.Len() > 0) )
        rStr.Append( cSep );
    rStr.Append( rToken );
}

void StringHelper::TrimString( String& rStr )
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

ByteString StringHelper::ConvertToUtf8( const String& rStr )
{
    return ByteString( rStr, RTL_TEXTENCODING_UTF8 );
}

// append decimal -------------------------------------------------------------

void StringHelper::AppendDec( String& rStr, sal_uInt8 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_Int8 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_uInt16 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_Int16 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_uInt32 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt64( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_Int32 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt32( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_uInt64 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt64( static_cast< sal_Int64 >( nData ) ), nWidth );
}

void StringHelper::AppendDec( String& rStr, sal_Int64 nData, xub_StrLen nWidth )
{
    AppendString( rStr, String::CreateFromInt64( nData ), nWidth );
}

void StringHelper::AppendDec( String& rStr, double fData, xub_StrLen nWidth )
{
    AppendString( rStr, String( ::rtl::math::doubleToUString( fData, rtl_math_StringFormat_G, 15, '.', true ) ), nWidth );
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

void StringHelper::AppendBin( String& rStr, sal_uInt8 nData, bool bDots )
{
    for( sal_uInt8 nMask = 0x80; nMask != 0; (nMask >>= 1) &= 0x7F )
    {
        rStr.Append( (nData & nMask) ? '1' : '0' );
        if( bDots && (nMask == 0x10) )
            rStr.Append( SCF_DUMP_BINDOT );
    }
}

// append binary --------------------------------------------------------------

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

// encoded text output --------------------------------------------------------

void StringHelper::AppendCChar( String& rStr, sal_Unicode cChar )
{
    static const String saXPrefix = CREATE_STRING( "\\x" );
    static const String saUPrefix = CREATE_STRING( "\\u" );
    if( cChar > 0x00FF )
    {
        rStr.Append( saUPrefix );
        AppendHex( rStr, static_cast< sal_uInt16 >( cChar ), false );
    }
    else
    {
        rStr.Append( saXPrefix );
        AppendHex( rStr, static_cast< sal_uInt8 >( cChar ), false );
    }
}

void StringHelper::AppendEncChar( String& rStr, sal_Unicode cChar, xub_StrLen nCount )
{
    if( cChar < 0x0020 )
    {
        // C-style hex code
        String aCode;
        AppendCChar( aCode, cChar );
        for( xub_StrLen nIdx = 0; nIdx < nCount; ++nIdx )
            rStr.Append( aCode );
    }
    else if( nCount == 1 )
        rStr.Append( cChar );
    else
        rStr.Expand( rStr.Len() + nCount, cChar );
}

void StringHelper::AppendEncString( String& rStr, const String& rData )
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
            AppendCChar( rStr, rData.GetChar( nIdx ) );
            ++nIdx;
        }
        // adjust limits
        nBeg = nIdx;
    }
}

// string to value conversion -------------------------------------------------

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

bool StringHelper::ConvertStringToBool( const String& rData )
{
    if( rData.EqualsIgnoreCaseAscii( "true" ) )
        return true;
    if( rData.EqualsIgnoreCaseAscii( "false" ) )
        return false;
    sal_Int64 nData;
    return ConvertStringToInt( nData, rData ) && (nData != 0);
}

// ============================================================================
// ============================================================================

Base::~Base()
{
}

// ============================================================================
// ============================================================================

void ConfigBase::ReadConfigBlock( SvStream& rStrm )
{
    // first line must be "begin" item
    sal_Size nOldPos = rStrm.Tell();
    if( ReadConfigLine( rStrm ) == LINETYPE_BEGIN )
        ReadConfigBlockContents( rStrm );
    else
        rStrm.Seek( nOldPos );
}

void ConfigBase::ReadConfigBlockContents( SvStream& rStrm )
{
    bool bLoop = true;
    while( bLoop && !rStrm.IsEof() )
    {
        String aItem, aData;
        switch( ReadConfigLine( rStrm, aItem, aData ) )
        {
            case LINETYPE_BEGIN:
                IgnoreConfigBlockContents( rStrm );     // ignore unknown embedded block
            break;
            case LINETYPE_END:
                bLoop = false;
            break;
            case LINETYPE_DATA:
                ProcessConfigItem( rStrm, aItem, aData );
            break;
            default:
                DBG_ERRORFILE( "scf::dump::ConfigBase::ReadConfigBlockContents - unknown config line type" );
        }
    }
}

void ConfigBase::IgnoreConfigBlockContents( SvStream& rStrm )
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

ConfigBase::LineType ConfigBase::ReadConfigLine( SvStream& rStrm, String& rKey, String& rData ) const
{
    String aLine;
    while( !rStrm.IsEof() && (aLine.Len() == 0) )
    {
        rStrm.ReadByteStringLine( aLine, RTL_TEXTENCODING_UTF8 );
        aLine.EraseLeadingChars( SCF_UTF8_BOM );
        TrimString( aLine );
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
            TrimString( rKey );
            rData = aLine.Copy( nEqPos + 1 );
            TrimString( rData );
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

ConfigBase::LineType ConfigBase::ReadConfigLine( SvStream& rStrm ) const
{
    String aItem, aData;
    return ReadConfigLine( rStrm, aItem, aData );
}

// ============================================================================

NameListBase::NameListBase()
{
}

void NameListBase::SetName( NameListKey nKey, const String& rName )
{
    maMap[ nKey ] = rName;
}

void NameListBase::ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData )
{
    sal_Int64 nKey;
    if( ConvertStringToInt( nKey, rKey ) )
        SetName( static_cast< NameListKey >( nKey ), rData );
}

// ============================================================================

ConstList::ConstList() :
    maDefName( SCF_DUMP_ERR_NONAME )
{
}

void ConstList::ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "default" ) )
        SetDefaultName( rData );
    else
        NameListBase::ProcessConfigItem( rStrm, rKey, rData );
}

String ConstList::ImplGetName( NameListKey nKey ) const
{
    StringMap::const_iterator aIt = maMap.find( nKey );
    return (aIt == maMap.end()) ? maDefName : aIt->second;
}

// ============================================================================

FlagsList::FlagsList() :
    mnIgnore( 0 )
{
}

void FlagsList::ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "ignore" ) )
    {
        sal_Int64 nIgnore;
        if( ConvertStringToInt( nIgnore, rData ) )
            SetIgnoreFlags( nIgnore );
    }
    else
        NameListBase::ProcessConfigItem( rStrm, rKey, rData );
}

String FlagsList::ImplGetName( NameListKey nKey ) const
{
    NameListKey nFlags = nKey;
    ::set_flag( nFlags, mnIgnore, false );
    NameListKey nFound = 0;
    String aName;
    // add known flags
    for( StringMap::const_iterator aIt = maMap.begin(), aEnd = maMap.end(); aIt != aEnd; ++aIt )
    {
        if( ::get_flag( nFlags, aIt->first ) )
            AppendToken( aName, aIt->second );
        ::set_flag( nFound, aIt->first );
    }
    // add unknown flags
    ::set_flag( nFlags, nFound, false );
    if( nFlags != 0 )
    {
        String aUnknown( '?' );
        AppendShortHex( aUnknown, nFlags, true );
        AppendToken( aName, aUnknown );
    }
    return aName;
}

// ============================================================================

Config::Config( const String& rFullName )
{
    Construct( rFullName );
}

Config::Config( const sal_Char* pcEnvVar, const String& rFileName )
{
    Construct( pcEnvVar, rFileName );
}

bool Config::Construct( const String& rFullName )
{
    SvFileStream aStrm( rFullName, STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE );
    bool bLoaded = aStrm.IsOpen();
    if( bLoaded )
        ReadConfigBlockContents( aStrm );
    return bLoaded;
}

bool Config::Construct( const sal_Char* pcEnvVar, const String& rFileName )
{
    bool bLoaded = false;

    // first try environment variable
    if( pcEnvVar )
        if( const sal_Char* pcVarFileName = ::getenv( pcEnvVar ) )
            bLoaded = Construct( String::CreateFromAscii( pcVarFileName ) );

    // if no success, try HOME
    if( !bLoaded )
    {
        if( const sal_Char* pcHome = ::getenv( "HOME" ) )
        {
            String aFullName = String::CreateFromAscii( pcHome );
            aFullName.EraseTrailingChars( SAL_PATHDELIMITER );
            aFullName.Append( SAL_PATHDELIMITER ).Append( rFileName );
            bLoaded = Construct( aFullName );
        }
    }

    return bLoaded;
}

bool Config::GetBoolOption( const sal_Char* pcKey, bool bDefault ) const
{
    ConfigDataMap::const_iterator aIt = maConfigData.find( String::CreateFromAscii( pcKey ) );
    return (aIt == maConfigData.end()) ? bDefault : ConvertStringToBool( aIt->second );
}

bool Config::IsDumperEnabled() const
{
    return GetBoolOption( "enable-dumper", true );
}

NameListRef Config::GetNameList( const sal_Char* pcMapName ) const
{
    NameListRef xList;
    String aMapName = String::CreateFromAscii( pcMapName );
    NameListMap::const_iterator aIt = maNameLists.find( aMapName );
    if( aIt != maNameLists.end() )
        xList = aIt->second;
    return xList;
}

void Config::ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData )
{
    if( rKey.EqualsAscii( "constlist" ) )
        ReadNameList< ConstList >( rStrm, rData );
    else if( rKey.EqualsAscii( "flagslist" ) )
        ReadNameList< FlagsList >( rStrm, rData );
    else
        maConfigData[ rKey ] = rData;
}

// ============================================================================
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
    mnCol( 0 )
{
    WriteChar( SCF_UTF8_BOM );
    WriteAscii( "OpenOffice.org generic file dumper v1.0" );
    NewLine();
    EmptyLine();
}

// ----------------------------------------------------------------------------

void Output::SetPrefix( const String& rPrefix )
{
    maPrefix = rPrefix;
    maPrefixUtf8 = ConvertToUtf8( rPrefix );
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

void Output::StartTable( size_t nColCount,
        xub_StrLen nWidth1, xub_StrLen nWidth2, xub_StrLen nWidth3, xub_StrLen nWidth4 )
{
    xub_StrLen pnColWidths[ 4 ];
    pnColWidths[ 0 ] = nWidth1;
    pnColWidths[ 1 ] = nWidth2;
    pnColWidths[ 2 ] = nWidth3;
    pnColWidths[ 3 ] = nWidth4;
    StartTable( ::std::min< size_t >( nColCount, 4 ), pnColWidths );
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
}

void Output::EndTable()
{
    maColPos.clear();
}

void Output::NewLine()
{
    if( maLine.Len() > 0 )
    {
        mrStrm << maPrefixUtf8.GetBuffer() << maIndent.GetBuffer();
        mrStrm << ConvertToUtf8( maLine ).GetBuffer() << '\n';
        maLine.Erase();
        mnCol = 0;
    }
}

void Output::EmptyLine( sal_Int32 nCount )
{
    for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
        mrStrm << maPrefixUtf8.GetBuffer() << '\n';
}

// strings --------------------------------------------------------------------

void Output::WriteChar( sal_Unicode cChar, xub_StrLen nCount )
{
    AppendEncChar( maLine, cChar, nCount );
}

void Output::WriteAscii( const sal_Char* pcStr )
{
    if( pcStr )
        maLine.AppendAscii( pcStr );
}

void Output::WriteString( const String& rStr )
{
    AppendEncString( maLine, rStr );
}

// ============================================================================
// ============================================================================

void BaseObject::Construct( ConfigRef xConfig, OutputRef xOut )
{
    mxConfig = xConfig;
    mxOut = xOut;
    mnItemLevel = 0;
    mnMultiLevel = 0;
}

void BaseObject::Construct( const BaseObject& rParent )
{
    Construct( rParent.mxConfig, rParent.mxOut );
}

void BaseObject::Construct( ConfigRef xConfig, const String& rFileName )
{
    if( xConfig.is() && xConfig->IsDumperEnabled() )
    {
        mxOwnStrm.reset( new SvFileStream( rFileName, STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC ) );
        if( mxOwnStrm->GetError() == SVSTREAM_OK )
            Construct( xConfig, OutputRef( new Output( *mxOwnStrm ) ) );
    }
}

bool BaseObject::IsValid() const
{
    return ImplIsValid();
}

void BaseObject::Dump()
{
    if( IsValid() )
    {
        ImplDumpHeader();
        ImplDumpBody();
        ImplDumpFooter();
    }
}

void BaseObject::DumpHeader()
{
    if( IsValid() )
        ImplDumpHeader();
}

void BaseObject::DumpFooter()
{
    if( IsValid() )
        ImplDumpFooter();
}

void BaseObject::StartItem( const sal_Char* pcName )
{
    if( mnItemLevel == 0 )
    {
        if( (mnMultiLevel > 0) && !mxOut->IsEmpty() )
            mxOut->WriteChar( ' ', 2 );
        mxOut->WriteAscii( pcName );
        mxOut->WriteChar( '=' );
    }
    ++mnItemLevel;
}

void BaseObject::EndItem()
{
    if( mnItemLevel > 0 )
        --mnItemLevel;
    if( mnItemLevel == 0 )
    {
        if( mnMultiLevel == 0 )
            mxOut->NewLine();
    }
    else
        mxOut->WriteChar( '=' );
}

void BaseObject::StartMultiItems()
{
    ++mnMultiLevel;
}

void BaseObject::EndMultiItems()
{
    if( mnMultiLevel > 0 )
        --mnMultiLevel;
    if( mnMultiLevel == 0 )
        mxOut->NewLine();
}

bool BaseObject::ImplIsValid() const
{
    return mxConfig.is() && mxConfig->IsDumperEnabled() && mxOut.is();
}

void BaseObject::ImplDumpHeader()
{
}

void BaseObject::ImplDumpBody()
{
}

void BaseObject::ImplDumpFooter()
{
}

void BaseObject::WriteEmptyItem( const sal_Char* pcName )
{
    ItemGuard aItem( *this, pcName );
}

void BaseObject::WriteInfoItem( const sal_Char* pcName, const sal_Char* pcData )
{
    ItemGuard aItem( *this, pcName );
    mxOut->WriteAscii( pcData );
}

void BaseObject::WriteInfoItem( const sal_Char* pcName, const String& rData )
{
    ItemGuard aItem( *this, pcName );
    mxOut->WriteString( rData );
}

void BaseObject::WriteStringItem( const sal_Char* pcName, const String& rData )
{
    ItemGuard aItem( *this, pcName );
    bool bCutStr = rData.Len() > 80;
    mxOut->WriteChar( '\'' );
    if( bCutStr )
        mxOut->WriteString( String( rData, 0, 80 ) );
    else
        mxOut->WriteString( rData );
    mxOut->WriteChar( '\'' );
    if( bCutStr )
    {
        mxOut->WriteAscii( " (cut,len=" );
        mxOut->WriteDec( static_cast< sal_Int32 >( rData.Len() ) );
        mxOut->WriteChar( ')' );
    }
}

// ============================================================================

InputObject::InputObject( const BaseObject& rParent, InputRef xIn )
{
    Construct( rParent, xIn );
}

InputObject::~InputObject()
{
}

void InputObject::Construct( const BaseObject& rParent, InputRef xIn )
{
    BaseObject::Construct( rParent );
    mxIn = xIn;
}

bool InputObject::ImplIsValid() const
{
    return mxIn.is() && BaseObject::ImplIsValid();
}

void InputObject::ImplDumpBody()
{
    DumpBinaryStream();
}

void InputObject::DumpBinary( sal_Size nSize, bool bShowOffset )
{
    Output& rOut = Out();
    TableGuard aTabGuard( rOut, 4,
        bShowOffset ? 12 : 0,
        3 * SCF_DUMP_BYTESPERLINE / 2 + 1,
        3 * SCF_DUMP_BYTESPERLINE / 2 + 1,
        SCF_DUMP_BYTESPERLINE / 2 + 1 );

    sal_Size nEndPos = ::std::min( mxIn->Tell() + nSize, mxIn->Size() );
    while( mxIn->Tell() < nEndPos )
    {
        rOut.WriteHex( static_cast< sal_uInt32 >( mxIn->Tell() ) );
        rOut.Tab();

        sal_uInt8 pnLineData[ SCF_DUMP_BYTESPERLINE ];
        sal_Size nLineSize = ::std::min( nSize - mxIn->Tell(), SCF_DUMP_BYTESPERLINE );
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
}

void InputObject::DumpBinaryStream( bool bShowOffset )
{
    mxIn->Seek( STREAM_SEEK_TO_BEGIN );
    DumpBinary( mxIn->Size(), bShowOffset );
    Out().EmptyLine();
}

void InputObject::DumpTextStream( rtl_TextEncoding eEnc, bool bShowLines )
{
    Output& rOut = Out();
    TableGuard aTabGuard( rOut, 1, bShowLines ? 8 : 0 );

    mxIn->Seek( STREAM_SEEK_TO_BEGIN );

    sal_uInt32 nLine = 0;
    while( mxIn->Tell() < mxIn->Size() )
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

// ============================================================================
// ============================================================================

OleStorageObject::OleStorageObject( const OleStorageObject& rParentStrg, const String& rStrgName )
{
    Construct( rParentStrg, rStrgName );
}

OleStorageObject::OleStorageObject( const BaseObject& rParent, SotStorageRef xRootStrg )
{
    Construct( rParent, xRootStrg, String::EmptyString() );
}

OleStorageObject::OleStorageObject( const BaseObject& rParent, SvStream& rRootStrm )
{
    Construct( rParent, rRootStrm );
}

OleStorageObject::~OleStorageObject()
{
}

void OleStorageObject::Construct( const BaseObject& rParent, SotStorageRef xStrg, const String& rParentPath )
{
    BaseObject::Construct( rParent );

    if( xStrg.Is() && (xStrg->GetError() == ERRCODE_NONE) )
    {
        mxStrg = xStrg;
        maStrgName = xStrg->GetName();
        maFullPath.Assign( rParentPath ).Append( '/' ).Append( maStrgName );
    }
}

void OleStorageObject::Construct( const OleStorageObject& rParentStrg, const String& rStrgName )
{
    SotStorageRef xStrg = ScfTools::OpenStorageRead( rParentStrg.mxStrg, rStrgName );
    Construct( rParentStrg, xStrg, rParentStrg.maFullPath );
}

void OleStorageObject::Construct( const BaseObject& rParent, SvStream& rRootStrm )
{
    SotStorageRef xRootStrg( new SotStorage( rRootStrm ) );
    Construct( rParent, xRootStrg, String::EmptyString() );
}

bool OleStorageObject::ImplIsValid() const
{
    return mxStrg.Is() && BaseObject::ImplIsValid();
}

void OleStorageObject::ImplDumpHeader()
{
    Output& rOut = Out();
    rOut.WriteChar( '+', 2 );
    rOut.WriteChar( '=', 76 );
    rOut.NewLine();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "||" ) );
        rOut.WriteAscii( "STORAGE-BEGIN" );
        rOut.NewLine();

        IndentGuard aIndGuard( rOut );
        WriteStringItem( "storage-name", maStrgName );
        WriteStringItem( "full-path", maFullPath );

        // directory
        SvStorageInfoList aInfoList;
        mxStrg->FillInfoList( &aInfoList );
        WriteDecItem< sal_uInt32 >( "directory-size", aInfoList.Count() );

        IndentGuard aDirIndGuard( rOut );
        for( ULONG nInfo = 0; nInfo < aInfoList.Count(); ++nInfo )
        {
            MultiItemsGuard aMultiGuard( *this );
            SvStorageInfo& rInfo = aInfoList.GetObject( nInfo );
            const sal_Char* pcType = rInfo.IsStream() ? "stream " :
                (rInfo.IsStorage() ? "storage" : "unknown");
            WriteInfoItem( "type", pcType );
            WriteStringItem( "name", rInfo.GetName() );
        }

        rOut.EmptyLine();
    }
    rOut.EmptyLine();
}

void OleStorageObject::ImplDumpFooter()
{
    Output& rOut = Out();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "||" ) );
        rOut.EmptyLine();
        {
            IndentGuard aIndGuard( rOut );
            WriteStringItem( "storage-name", maStrgName );
            WriteStringItem( "full-path", maFullPath );
        }
        rOut.WriteAscii( "STORAGE-END" );
        rOut.NewLine();
    }
    rOut.WriteChar( '+', 2 );
    rOut.WriteChar( '=', 76 );
    rOut.NewLine();
    rOut.EmptyLine();
}

// ============================================================================

OleStreamObject::OleStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    Construct( rParentStrg, rStrmName );
}

OleStreamObject::~OleStreamObject()
{
}

void OleStreamObject::Construct( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    mxStrm = ScfTools::OpenStorageStreamRead( rParentStrg.GetStorage(), rStrmName );
    if( mxStrm.Is() )
    {
        InputRef xIn( new SvStreamInput( *mxStrm ) );
        InputObject::Construct( rParentStrg, xIn );

        maStrmName = rStrmName;
        maFullPath.Assign( rParentStrg.GetFullPath() ).Append( '/' ).Append( rStrmName );
    }
}

bool OleStreamObject::ImplIsValid() const
{
    return mxStrm.Is() && InputObject::ImplIsValid();
}

void OleStreamObject::ImplDumpHeader()
{
    Output& rOut = Out();
    rOut.WriteChar( '+' );
    rOut.WriteChar( '-', 77 );
    rOut.NewLine();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "|" ) );
        rOut.WriteAscii( "STREAM-BEGIN" );
        rOut.NewLine();

        IndentGuard aIndGuard( rOut );
        WriteStringItem( "stream-name", maStrmName );
        WriteStringItem( "full-path", maFullPath );
        WriteDecItem< sal_uInt32 >( "stream-size", In().Size() );

        rOut.EmptyLine();
    }
    rOut.EmptyLine();
}

void OleStreamObject::ImplDumpFooter()
{
    Output& rOut = Out();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "|" ) );
        rOut.EmptyLine();
        {
            IndentGuard aIndGuard( rOut );
            WriteStringItem( "stream-name", maStrmName );
            WriteStringItem( "full-path", maFullPath );
        }
        rOut.WriteAscii( "STREAM-END" );
        rOut.NewLine();
    }
    rOut.WriteChar( '+' );
    rOut.WriteChar( '-', 77 );
    rOut.NewLine();
    rOut.EmptyLine();
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif

