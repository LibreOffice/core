/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "tokens.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/graphichelper.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace oox {
namespace ole {

// ============================================================================

namespace {

const sal_uInt32 OLE_COLORTYPE_MASK         = 0xFF000000;
const sal_uInt32 OLE_COLORTYPE_CLIENT       = 0x00000000;
const sal_uInt32 OLE_COLORTYPE_PALETTE      = 0x01000000;
const sal_uInt32 OLE_COLORTYPE_BGR          = 0x02000000;
const sal_uInt32 OLE_COLORTYPE_SYSCOLOR     = 0x80000000;

const sal_uInt32 OLE_PALETTECOLOR_MASK      = 0x0000FFFF;
const sal_uInt32 OLE_BGRCOLOR_MASK          = 0x00FFFFFF;
const sal_uInt32 OLE_SYSTEMCOLOR_MASK       = 0x0000FFFF;

/** Returns the UNO RGB color from the passed encoded OLE BGR color. */
inline sal_Int32 lclDecodeBgrColor( sal_uInt32 nOleColor )
{
    return static_cast< sal_Int32 >( ((nOleColor & 0x0000FF) << 16) | (nOleColor & 0x00FF00) | ((nOleColor & 0xFF0000) >> 16) );
}

// ----------------------------------------------------------------------------

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

OUString lclReadStdHlinkString( BinaryInputStream& rInStrm, bool bUnicode )
{
    OUString aRet;
    sal_Int32 nChars = rInStrm.readInt32();
    if( nChars > 0 )
    {
        sal_Int32 nReadChars = getLimitedValue< sal_Int32, sal_Int32 >( nChars, 0, SAL_MAX_UINT16 );
        // byte strings are always in ANSI (Windows 1252) encoding
        aRet = bUnicode ? rInStrm.readUnicodeArray( nReadChars, true ) : rInStrm.readCharArrayUC( nReadChars, RTL_TEXTENCODING_MS_1252, true );
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

StdFontInfo::StdFontInfo() :
    mnHeight( 0 ),
    mnWeight( OLE_STDFONT_NORMAL ),
    mnCharSet( WINDOWS_CHARSET_ANSI ),
    mnFlags( 0 )
{
}

StdFontInfo::StdFontInfo( const ::rtl::OUString& rName, sal_uInt32 nHeight,
        sal_uInt16 nWeight, sal_uInt16 nCharSet, sal_uInt8 nFlags ) :
    maName( rName ),
    mnHeight( nHeight ),
    mnWeight( nWeight ),
    mnCharSet( nCharSet ),
    mnFlags( nFlags )
{
}

// ============================================================================

/*static*/ sal_Int32 OleHelper::decodeOleColor(
        const GraphicHelper& rGraphicHelper, sal_uInt32 nOleColor, bool bDefaultColorBgr )
{
    static const sal_Int32 spnSystemColors[] =
    {
        XML_scrollBar,      XML_background,     XML_activeCaption,  XML_inactiveCaption,
        XML_menu,           XML_window,         XML_windowFrame,    XML_menuText,
        XML_windowText,     XML_captionText,    XML_activeBorder,   XML_inactiveBorder,
        XML_appWorkspace,   XML_highlight,      XML_highlightText,  XML_btnFace,
        XML_btnShadow,      XML_grayText,       XML_btnText,        XML_inactiveCaptionText,
        XML_btnHighlight,   XML_3dDkShadow,     XML_3dLight,        XML_infoText,
        XML_infoBk
    };

    switch( nOleColor & OLE_COLORTYPE_MASK )
    {
        case OLE_COLORTYPE_CLIENT:
            return bDefaultColorBgr ? lclDecodeBgrColor( nOleColor ) : rGraphicHelper.getPaletteColor( nOleColor & OLE_PALETTECOLOR_MASK );

        case OLE_COLORTYPE_PALETTE:
            return rGraphicHelper.getPaletteColor( nOleColor & OLE_PALETTECOLOR_MASK );

        case OLE_COLORTYPE_BGR:
            return lclDecodeBgrColor( nOleColor );

        case OLE_COLORTYPE_SYSCOLOR:
            return rGraphicHelper.getSystemColor( STATIC_ARRAY_SELECT( spnSystemColors, nOleColor & OLE_SYSTEMCOLOR_MASK, XML_TOKEN_INVALID ), API_RGB_WHITE );
    }
    OSL_ENSURE( false, "OleHelper::decodeOleColor - unknown color type" );
    return API_RGB_BLACK;
}

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

/*static*/ bool OleHelper::importStdFont( StdFontInfo& orFontInfo, BinaryInputStream& rInStrm, bool bWithGuid )
{
    if( bWithGuid )
    {
        bool bIsStdFont = importGuid( rInStrm ).equalsAscii( OLE_GUID_STDFONT );
        OSL_ENSURE( bIsStdFont, "OleHelper::importStdFont - unexpected header GUID, expected StdFont" );
        if( !bIsStdFont )
            return false;
    }

    sal_uInt8 nVersion, nNameLen;
    rInStrm >> nVersion >> orFontInfo.mnCharSet >> orFontInfo.mnFlags >> orFontInfo.mnWeight >> orFontInfo.mnHeight >> nNameLen;
    // according to spec the name is ASCII
    orFontInfo.maName = rInStrm.readCharArrayUC( nNameLen, RTL_TEXTENCODING_ASCII_US );
    OSL_ENSURE( nVersion <= 1, "OleHelper::importStdFont - wrong version" );
    return !rInStrm.isEof() && (nVersion <= 1);
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

/*static*/ bool OleHelper::importStdHlink( StdHlinkInfo& orHlinkInfo, BinaryInputStream& rInStrm, bool bWithGuid )
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
        orHlinkInfo.maDisplay = lclReadStdHlinkString( rInStrm, true );
    // frame string
    if( getFlag( nFlags, OLE_STDHLINK_HASFRAME ) )
        orHlinkInfo.maFrame = lclReadStdHlinkString( rInStrm, true );

    // target
    if( getFlag( nFlags, OLE_STDHLINK_HASTARGET ) )
    {
        if( getFlag( nFlags, OLE_STDHLINK_ASSTRING ) )
        {
            OSL_ENSURE( getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ), "OleHelper::importStdHlink - link not absolute" );
            orHlinkInfo.maTarget = lclReadStdHlinkString( rInStrm, true );
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
                orHlinkInfo.maTarget = lclReadStdHlinkString( rInStrm, false );
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
        orHlinkInfo.maLocation = lclReadStdHlinkString( rInStrm, true );

    return !rInStrm.isEof();
}

// ============================================================================

} // namespace ole
} // namespace oox

