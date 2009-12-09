/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: olehelper.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/ole/olehelper.hxx"
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace oox {
namespace ole {

// ============================================================================

namespace {

const sal_Char* const OLE_GUID_STDFONT      = "{0BE35203-8F91-11CE-9DE3-00AA004BB851}";
const sal_Char* const OLE_GUID_STDPIC       = "{0BE35204-8F91-11CE-9DE3-00AA004BB851}";
const sal_Char* const OLE_GUID_STDHLINK     = "{79EAC9D0-BAF9-11CE-8C82-00AA004BA90B}";
const sal_Char* const OLE_GUID_URLMONIKER   = "{79EAC9E0-BAF9-11CE-8C82-00AA004BA90B}";
const sal_Char* const OLE_GUID_FILEMONIKER  = "{00000303-0000-0000-C000-000000000046}";

const sal_uInt32 OLE_STDPIC_ID              = 0x0000746C;

const sal_uInt32 OLE_STDHLINK_VERSION       = 2;
const sal_uInt32 OLE_STDHLINK_HASTARGET     = 0x00000001;   /// Has hyperlink moniker.
const sal_uInt32 OLE_STDHLINK_ABSOLUTE      = 0x00000002;   /// Absolute path.
const sal_uInt32 OLE_STDHLINK_HASLOCATION   = 0x00000008;   /// Has target location.
const sal_uInt32 OLE_STDHLINK_HASDISPLAY    = 0x00000010;   /// Has display string.
const sal_uInt32 OLE_STDHLINK_HASGUID       = 0x00000020;   /// Has identification GUID.
const sal_uInt32 OLE_STDHLINK_HASTIME       = 0x00000040;   /// Has creation time.
const sal_uInt32 OLE_STDHLINK_HASFRAME      = 0x00000080;   /// Has frame.
const sal_uInt32 OLE_STDHLINK_ASSTRING      = 0x00000100;   /// Hyperlink as simple string.

// ----------------------------------------------------------------------------

template< typename Type >
void lclAppendHex( OUStringBuffer& orBuffer, Type nValue )
{
    const sal_Int32 nWidth = 2 * sizeof( Type );
    static const sal_Unicode spcHexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    orBuffer.setLength( orBuffer.getLength() + nWidth );
    for( sal_Int32 nCharIdx = orBuffer.getLength() - 1, nCharEnd = nCharIdx - nWidth; nCharIdx > nCharEnd; --nCharIdx, nValue >>= 4 )
        orBuffer.setCharAt( nCharIdx, spcHexChars[ nValue & 0xF ] );
}

OUString lclReadStdHlinkString( BinaryInputStream& rInStrm, rtl_TextEncoding eTextEnc, bool bUnicode )
{
    OUString aRet;
    sal_Int32 nChars = rInStrm.readInt32();
    if( nChars > 0 )
    {
        sal_Int32 nReadChars = getLimitedValue< sal_Int32, sal_Int32 >( nChars, 0, SAL_MAX_UINT16 );
        aRet = bUnicode ? rInStrm.readUnicodeArray( nReadChars, true ) : rInStrm.readCharArrayUC( nReadChars, eTextEnc, true );
        // strings are NUL terminated, remove trailing NUL and possible other garbage
        sal_Int32 nNulPos = aRet.indexOf( '\0' );
        if( nNulPos >= 0 )
            aRet = aRet.copy( 0, nNulPos );
        // skip remaining chars
        rInStrm.skip( (bUnicode ? 2 : 1) * (nChars - nReadChars) );
    }
    return aRet;
}

} // namespace

// ============================================================================

/*static*/ OUString OleHelper::importGuid( BinaryInputStream& rInStrm )
{
    OUStringBuffer aBuffer;
    aBuffer.append( sal_Unicode( '{' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt32() );
    aBuffer.append( sal_Unicode( '-' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt16() );
    aBuffer.append( sal_Unicode( '-' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt16() );
    aBuffer.append( sal_Unicode( '-' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt8() );
    lclAppendHex( aBuffer, rInStrm.readuInt8() );
    aBuffer.append( sal_Unicode( '-' ) );
    for( int nIndex = 0; nIndex < 6; ++nIndex )
        lclAppendHex( aBuffer, rInStrm.readuInt8() );
    aBuffer.append( sal_Unicode( '}' ) );
    return aBuffer.makeStringAndClear();
}

/*static*/ bool OleHelper::importStdPic( StreamDataSequence& orGraphicData, BinaryInputStream& rInStrm, bool bWithGuid )
{
    if( bWithGuid )
    {
        bool bIsStdPic = importGuid( rInStrm ).equalsAscii( OLE_GUID_STDPIC );
        OSL_ENSURE( bIsStdPic, "OleHelper::importStdPic - unexpected header GUID, expected StdPic" );
        if( !bIsStdPic )
            return false;
    }

    sal_uInt32 nStdPicId;
    sal_Int32 nBytes;
    rInStrm >> nStdPicId >> nBytes;
    OSL_ENSURE( nStdPicId == OLE_STDPIC_ID, "OleHelper::importStdPic - unexpected header version" );
    return !rInStrm.isEof() && (nStdPicId == OLE_STDPIC_ID) && (nBytes > 0) && (rInStrm.readData( orGraphicData, nBytes ) == nBytes);
}

/*static*/ bool OleHelper::importStdHlink( StdHlinkInfo& orHlinkInfo, BinaryInputStream& rInStrm, rtl_TextEncoding eTextEnc, bool bWithGuid )
{
    if( bWithGuid )
    {
        bool bIsStdHlink = importGuid( rInStrm ).equalsAscii( OLE_GUID_STDHLINK );
        OSL_ENSURE( bIsStdHlink, "OleHelper::importStdHlink - unexpected header GUID, expected StdHlink" );
        if( !bIsStdHlink )
            return false;
    }

    sal_uInt32 nVersion, nFlags;
    rInStrm >> nVersion >> nFlags;
    OSL_ENSURE( nVersion == OLE_STDHLINK_VERSION, "OleHelper::importStdHlink - unexpected header version" );
    if( rInStrm.isEof() || (nVersion != OLE_STDHLINK_VERSION) )
        return false;

    // display string
    if( getFlag( nFlags, OLE_STDHLINK_HASDISPLAY ) )
        orHlinkInfo.maDisplay = lclReadStdHlinkString( rInStrm, eTextEnc, true );
    // frame string
    if( getFlag( nFlags, OLE_STDHLINK_HASFRAME ) )
        orHlinkInfo.maFrame = lclReadStdHlinkString( rInStrm, eTextEnc, true );

    // target
    if( getFlag( nFlags, OLE_STDHLINK_HASTARGET ) )
    {
        if( getFlag( nFlags, OLE_STDHLINK_ASSTRING ) )
        {
            OSL_ENSURE( getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ), "OleHelper::importStdHlink - link not absolute" );
            orHlinkInfo.maTarget = lclReadStdHlinkString( rInStrm, eTextEnc, true );
        }
        else // hyperlink moniker
        {
            OUString aGuid = importGuid( rInStrm );
            if( aGuid.equalsAscii( OLE_GUID_FILEMONIKER ) )
            {
                // file name, maybe relative and with directory up-count
                sal_Int16 nUpLevels;
                rInStrm >> nUpLevels;
                OSL_ENSURE( (nUpLevels == 0) || !getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ), "OleHelper::importStdHlink - absolute filename with upcount" );
                orHlinkInfo.maTarget = lclReadStdHlinkString( rInStrm, eTextEnc, false );
                rInStrm.skip( 24 );
                sal_Int32 nBytes = rInStrm.readInt32();
                if( nBytes > 0 )
                {
                    sal_Int64 nEndPos = rInStrm.tell() + ::std::max< sal_Int32 >( nBytes, 0 );
                    sal_uInt16 nChars = getLimitedValue< sal_uInt16, sal_Int32 >( rInStrm.readInt32() / 2, 0, SAL_MAX_UINT16 );
                    rInStrm.skip( 2 );  // key value
                    orHlinkInfo.maTarget = rInStrm.readUnicodeArray( nChars );  // NOT null terminated
                    rInStrm.seek( nEndPos );
                }
                if( !getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ) )
                    for( sal_Int16 nLevel = 0; nLevel < nUpLevels; ++nLevel )
                        orHlinkInfo.maTarget = CREATE_OUSTRING( "../" ) + orHlinkInfo.maTarget;
            }
            else if( aGuid.equalsAscii( OLE_GUID_URLMONIKER ) )
            {
                // URL, maybe relative and with leading '../'
                sal_Int32 nBytes = rInStrm.readInt32();
                sal_Int64 nEndPos = rInStrm.tell() + ::std::max< sal_Int32 >( nBytes, 0 );
                orHlinkInfo.maTarget = rInStrm.readNulUnicodeArray();
                rInStrm.seek( nEndPos );
            }
            else
            {
                OSL_ENSURE( false, "OleHelper::importStdHlink - unsupported hyperlink moniker" );
                return false;
            }
        }
    }

    // target location
    if( getFlag( nFlags, OLE_STDHLINK_HASLOCATION ) )
        orHlinkInfo.maLocation = lclReadStdHlinkString( rInStrm, eTextEnc, true );

    return true;
}

// ============================================================================

} // namespace ole
} // namespace oox

