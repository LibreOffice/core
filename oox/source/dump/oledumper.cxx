/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oledumper.cxx,v $
 * $Revision: 1.1.2.13 $
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

#include "oox/dump/oledumper.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <rtl/tencinfo.h>
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/ole/vbainputstream.hxx"
#include "oox/xls/biffhelper.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;

namespace oox {
namespace dump {

// ============================================================================
// ============================================================================

StdFontObject::StdFontObject( const InputObjectBase& rParent ) :
    InputObjectBase( rParent )
{
}

void StdFontObject::implDump()
{
    dumpDec< sal_uInt8 >( "version" );
    dumpDec< sal_uInt16 >( "charset", "CHARSET" );
    dumpHex< sal_uInt8 >( "flags", "STDFONT-FLAGS" );
    dumpDec< sal_uInt16 >( "weight", "FONT-WEIGHT" );
    dumpDec< sal_uInt32 >( "height", "STDFONT-HEIGHT" );
    dumpCharArray( "name", in().readuInt8(), RTL_TEXTENCODING_ASCII_US );
}

// ============================================================================

StdPicObject::StdPicObject( const InputObjectBase& rParent ) :
    InputObjectBase( rParent )
{
}

void StdPicObject::implDump()
{
    dumpHex< sal_uInt32 >( "identifier" );
    sal_uInt32 nSize = dumpHex< sal_uInt32 >( "image-size", "CONV-DEC" );
    dumpBinary( "image-data", nSize );
}

// ============================================================================

namespace {

const sal_uInt32 STDHLINK_HASTARGET         = 0x00000001;   /// Has hyperlink moniker.
const sal_uInt32 STDHLINK_ABSOLUTE          = 0x00000002;   /// Absolute path.
const sal_uInt32 STDHLINK_HASLOCATION       = 0x00000008;   /// Has target location.
const sal_uInt32 STDHLINK_HASDISPLAY        = 0x00000010;   /// Has display string.
const sal_uInt32 STDHLINK_HASGUID           = 0x00000020;   /// Has identification GUID.
const sal_uInt32 STDHLINK_HASTIME           = 0x00000040;   /// Has creation time.
const sal_uInt32 STDHLINK_HASFRAME          = 0x00000080;   /// Has frame.
const sal_uInt32 STDHLINK_ASSTRING          = 0x00000100;   /// Hyperlink as simple string.

} // namespace

StdHlinkObject::StdHlinkObject( const InputObjectBase& rParent ) :
    InputObjectBase( rParent )
{
}

void StdHlinkObject::implDump()
{
    dumpDec< sal_uInt32 >( "stream-version" );
    sal_uInt32 nFlags = dumpHex< sal_uInt32 >( "flags", "STDHLINK-FLAGS" );
    if( getFlag( nFlags, STDHLINK_HASDISPLAY ) )
        dumpHyperlinkString( "display", true );
    if( getFlag( nFlags, STDHLINK_HASFRAME ) )
        dumpHyperlinkString( "frame", true );
    if( getFlag( nFlags, STDHLINK_HASTARGET ) )
    {
        if( getFlag( nFlags, STDHLINK_ASSTRING ) )
            dumpHyperlinkString( "filename", true );
        else if( !dumpGuidAndMoniker() )
            return;
    }
    if( getFlag( nFlags, STDHLINK_HASLOCATION ) )
        dumpHyperlinkString( "location", true );
    if( getFlag( nFlags, STDHLINK_HASGUID ) )
        dumpGuid( "id-guid" );
    if( getFlag( nFlags, STDHLINK_HASTIME ) )
        dumpFileTime( "creation-time" );
}

OUString StdHlinkObject::dumpHyperlinkString( const String& rName, bool bUnicode )
{
    sal_Int32 nLen = in().readInt32();
    return bUnicode ? dumpUnicodeArray( rName, nLen ) : dumpCharArray( rName, nLen, osl_getThreadTextEncoding() );
}

bool StdHlinkObject::dumpGuidAndMoniker()
{
    bool bValidMoniker = true;
    OUString aGuid = cfg().getStringOption( dumpGuid( "moniker" ), OUString() );
    IndentGuard aIndGuard( out() );
    if( aGuid.equalsAscii( "URLMoniker" ) )
        dumpUrlMoniker();
    else if( aGuid.equalsAscii( "FileMoniker" ) )
        dumpFileMoniker();
    else if( aGuid.equalsAscii( "ItemMoniker" ) )
        dumpItemMoniker();
    else if( aGuid.equalsAscii( "AntiMoniker" ) )
        dumpAntiMoniker();
    else if( aGuid.equalsAscii( "CompositeMoniker" ) )
        dumpCompositeMoniker();
    else
        bValidMoniker = false;
    return bValidMoniker;
}

void StdHlinkObject::dumpUrlMoniker()
{
    sal_Int32 nBytes = dumpDec< sal_Int32 >( "url-bytes" );
    sal_Int64 nEndPos = in().tell() + ::std::max< sal_Int32 >( nBytes, 0 );
    dumpNullUnicodeArray( "url" );
    if( in().tell() + 24 == nEndPos )
    {
        dumpGuid( "implementation-id" );
        dumpDec< sal_uInt32 >( "version" );
        dumpHex< sal_uInt32 >( "flags", "STDHLINK-URL-FLAGS" );
    }
    dumpRemainingTo( nEndPos );
}

void StdHlinkObject::dumpFileMoniker()
{
    dumpDec< sal_Int16 >( "up-levels" );
    dumpHyperlinkString( "ansi-filename", false );
    dumpDec< sal_Int16 >( "server-path-len" );
    dumpHex< sal_uInt16 >( "version" );
    dumpUnused( 20 );
    sal_Int32 nBytes = dumpDec< sal_Int32 >( "total-bytes" );
    sal_Int64 nEndPos = in().tell() + ::std::max< sal_Int32 >( nBytes, 0 );
    if( nBytes > 0 )
    {
        sal_Int32 nFileBytes = dumpDec< sal_Int32 >( "uni-filename-bytes" );
        dumpDec< sal_uInt16 >( "key-value" );
        dumpUnicodeArray( "unicode-filename", nFileBytes / 2 );
    }
    dumpRemainingTo( nEndPos );
}

void StdHlinkObject::dumpItemMoniker()
{
    sal_Int32 nBytes = dumpDec< sal_Int32 >( "delimiter-bytes" );
    sal_Int64 nEndPos = in().tell() + ::std::max< sal_Int32 >( nBytes, 0 );
    dumpNullCharArray( "ansi-delimiter", osl_getThreadTextEncoding() );
    if( in().tell() < nEndPos )
        dumpUnicodeArray( "unicode-delimiter", (nEndPos - in().tell()) / 2 );
    in().seek( nEndPos );

    nBytes = dumpDec< sal_Int32 >( "item-bytes" );
    nEndPos = in().tell() + ::std::max< sal_Int32 >( nBytes, 0 );
    dumpNullCharArray( "ansi-item", osl_getThreadTextEncoding() );
    if( in().tell() < nEndPos )
        dumpUnicodeArray( "unicode-item", (nEndPos - in().tell()) / 2 );
    in().seek( nEndPos );
}

void StdHlinkObject::dumpAntiMoniker()
{
    dumpDec< sal_Int32 >( "count" );
}

void StdHlinkObject::dumpCompositeMoniker()
{
    sal_Int32 nCount = dumpDec< sal_Int32 >( "moniker-count" );
    for( sal_Int32 nIndex = 0; !in().isEof() && (nIndex < nCount); ++nIndex )
        dumpGuidAndMoniker();
}

// ============================================================================
// ============================================================================

namespace {

const sal_Int32 OLEPROP_ID_DICTIONARY   = 0;
const sal_Int32 OLEPROP_ID_CODEPAGE     = 1;

const sal_Int32 OLEPROP_TYPE_INT16      = 2;
const sal_Int32 OLEPROP_TYPE_INT32      = 3;
const sal_Int32 OLEPROP_TYPE_FLOAT      = 4;
const sal_Int32 OLEPROP_TYPE_DOUBLE     = 5;
const sal_Int32 OLEPROP_TYPE_DATE       = 7;
const sal_Int32 OLEPROP_TYPE_STRING     = 8;
const sal_Int32 OLEPROP_TYPE_STATUS     = 10;
const sal_Int32 OLEPROP_TYPE_BOOL       = 11;
const sal_Int32 OLEPROP_TYPE_VARIANT    = 12;
const sal_Int32 OLEPROP_TYPE_INT8       = 16;
const sal_Int32 OLEPROP_TYPE_UINT8      = 17;
const sal_Int32 OLEPROP_TYPE_UINT16     = 18;
const sal_Int32 OLEPROP_TYPE_UINT32     = 19;
const sal_Int32 OLEPROP_TYPE_INT64      = 20;
const sal_Int32 OLEPROP_TYPE_UINT64     = 21;
const sal_Int32 OLEPROP_TYPE_STRING8    = 30;
const sal_Int32 OLEPROP_TYPE_STRING16   = 31;
const sal_Int32 OLEPROP_TYPE_FILETIME   = 64;
const sal_Int32 OLEPROP_TYPE_BLOB       = 65;
const sal_Int32 OLEPROP_TYPE_STREAM     = 66;
const sal_Int32 OLEPROP_TYPE_STORAGE    = 67;
const sal_Int32 OLEPROP_TYPE_CLIPFMT    = 71;

const sal_uInt16 CODEPAGE_UNICODE       = 1200;

const sal_uInt32 OCX_STRING_COMPRESSED  = 0x80000000;

} // namespace

// ============================================================================

OlePropertyStreamObject::OlePropertyStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName ) :
    BinaryStreamObject( rParent, rxStrm, rSysFileName )
{
}

void OlePropertyStreamObject::implDump()
{
    BinaryInputStream& rStrm = in();
    Output& rOut = out();

    OUStringVector aGuidVec;
    ::std::vector< sal_uInt32 > aStartPosVec;

    // dump header
    writeEmptyItem( "HEADER" );
    {
        IndentGuard aIndGuard( rOut );
        dumpHex< sal_uInt16 >( "byte-order", "OLEPROP-BYTE-ORDER" );
        dumpDec< sal_uInt16 >( "version" );
        dumpDec< sal_uInt16 >( "os-minor" );
        dumpDec< sal_uInt16 >( "os-type", "OLEPROP-OSTYPE" );
        dumpGuid( "guid" );
        sal_Int32 nSectCount = dumpDec< sal_Int32 >( "section-count" );

        // dump table of section positions
        {
            TableGuard aTabGuard( rOut, 15, 60 );
            rOut.resetItemIndex();
            for( sal_Int32 nSectIdx = 0; !rStrm.isEof() && (nSectIdx < nSectCount); ++nSectIdx )
            {
                MultiItemsGuard aMultiGuard( rOut );
                writeEmptyItem( "#section" );
                aGuidVec.push_back( dumpGuid( "guid" ) );
                aStartPosVec.push_back( dumpHex< sal_uInt32 >( "start-pos", "CONV-DEC" ) );
            }
        }
    }
    rOut.emptyLine();

    // dump sections
    for( size_t nSectIdx = 0; !rStrm.isEof() && (nSectIdx < aStartPosVec.size()); ++nSectIdx )
        dumpSection( aGuidVec[ nSectIdx ], aStartPosVec[ nSectIdx ] );
}

void OlePropertyStreamObject::dumpSection( const OUString& rGuid, sal_uInt32 nStartPos )
{
    BinaryInputStream& rStrm = in();
    Output& rOut = out();

    // property ID names
    mxPropIds = cfg().createNameList< ConstList >( "OLEPROP-IDS" );
    OUString aGuidName = cfg().getStringOption( rGuid, OUString() );
    if( aGuidName.equalsAscii( "GlobalDocProp" ) )
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-GLOBALIDS" ) );
    else if( aGuidName.equalsAscii( "BuiltinDocProp" ) )
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-BUILTINIDS" ) );
    else
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-BASEIDS" ) );

    // property ID/position map
    typedef ::std::map< sal_Int32, sal_uInt32 > PropertyPosMap;
    PropertyPosMap aPropMap;

    // dump section header line
    writeSectionHeader( rGuid, nStartPos );

    // seek to section
    IndentGuard aIndGuard( rOut );
    if( startElement( nStartPos ) )
    {
        // dump section header
        dumpDec< sal_Int32 >( "size" );
        sal_Int32 nPropCount = dumpDec< sal_Int32 >( "property-count" );

        // dump table of property positions
        {
            TableGuard aTabGuard( rOut, 15, 25 );
            rOut.resetItemIndex();
            for( sal_Int32 nPropIdx = 0; !rStrm.isEof() && (nPropIdx < nPropCount); ++nPropIdx )
            {
                MultiItemsGuard aMultiGuard( rOut );
                writeEmptyItem( "#property" );
                sal_Int32 nPropId = dumpDec< sal_Int32 >( "id", mxPropIds );
                sal_uInt32 nPropPos = nStartPos + dumpHex< sal_uInt32 >( "start-pos", "CONV-DEC" );
                aPropMap[ nPropId ] = nPropPos;
            }
        }
    }
    rOut.emptyLine();

    // code page property
    meTextEnc = osl_getThreadTextEncoding();
    mbIsUnicode = false;
    PropertyPosMap::iterator aCodePageIt = aPropMap.find( OLEPROP_ID_CODEPAGE );
    if( aCodePageIt != aPropMap.end() )
    {
        dumpCodePageProperty( aCodePageIt->second );
        aPropMap.erase( aCodePageIt );
    }

    // dictionary property
    PropertyPosMap::iterator aDictIt = aPropMap.find( OLEPROP_ID_DICTIONARY );
    if( aDictIt != aPropMap.end() )
    {
        dumpDictionaryProperty( aDictIt->second );
        aPropMap.erase( aDictIt );
    }

    // other properties
    for( PropertyPosMap::const_iterator aIt = aPropMap.begin(), aEnd = aPropMap.end(); aIt != aEnd; ++aIt )
        dumpProperty( aIt->first, aIt->second );

    // remove the user defined list of property ID names
    cfg().eraseNameList( "OLEPROP-IDS" );
}

void OlePropertyStreamObject::dumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    writePropertyHeader( nPropId, nStartPos );
    IndentGuard aIndGuard( out() );
    if( startElement( nStartPos ) )
        dumpPropertyContents( nPropId );
    out().emptyLine();
}

void OlePropertyStreamObject::dumpCodePageProperty( sal_uInt32 nStartPos )
{
    writePropertyHeader( OLEPROP_ID_CODEPAGE, nStartPos );
    IndentGuard aIndGuard( out() );
    if( startElement( nStartPos ) )
    {
        sal_Int32 nType = dumpPropertyType();
        if( nType == OLEPROP_TYPE_INT16 )
        {
            sal_uInt16 nCodePage = dumpDec< sal_uInt16 >( "codepage", "CODEPAGES" );
            rtl_TextEncoding nNewTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
            if( nNewTextEnc != RTL_TEXTENCODING_DONTKNOW )
                meTextEnc = nNewTextEnc;
            mbIsUnicode = nCodePage == CODEPAGE_UNICODE;
        }
        else
            dumpPropertyContents( OLEPROP_ID_CODEPAGE );
    }
    out().emptyLine();
}

void OlePropertyStreamObject::dumpDictionaryProperty( sal_uInt32 nStartPos )
{
    writePropertyHeader( OLEPROP_ID_DICTIONARY, nStartPos );
    IndentGuard aIndGuard( out() );
    if( startElement( nStartPos ) )
    {
        sal_Int32 nCount = dumpDec< sal_Int32 >( "count" );
        for( sal_Int32 nIdx = 0; !in().isEof() && (nIdx < nCount); ++nIdx )
        {
            MultiItemsGuard aMultiGuard( out() );
            TableGuard aTabGuard( out(), 10, 20 );
            sal_Int32 nId = dumpDec< sal_Int32 >( "id" );
            OUString aName = dumpString8( "name" );
            if( mxPropIds.get() )
                mxPropIds->setName( nId, aName );
        }
    }
    out().emptyLine();
}

void OlePropertyStreamObject::dumpPropertyContents( sal_Int32 nPropId )
{
    sal_Int32 nType = dumpPropertyType();
    if( getFlag< sal_Int32 >( nType, 0x1000 ) ) // vector
    {
        sal_Int32 nBaseType = nType & 0x0FFF;
        sal_Int32 nElemCount = dumpDec< sal_Int32 >( "element-count" );
        for( sal_Int32 nElemIdx = 0; !in().isEof() && (nElemIdx < nElemCount); ++nElemIdx )
        {
            out().resetItemIndex( nElemIdx );
            writeEmptyItem( "#element" );
            IndentGuard aIndGuard( out() );
            dumpPropertyValue( nPropId, nBaseType );
        }
    }
    else if( !getFlag< sal_Int32 >( nType, 0x7000 ) )
    {
        dumpPropertyValue( nPropId, nType );
    }
}

void OlePropertyStreamObject::dumpPropertyValue( sal_Int32 nPropId, sal_Int32 nBaseType )
{
    switch( nBaseType )
    {
        case OLEPROP_TYPE_INT16:        dumpDec< sal_Int16 >( "value" );        break;
        case OLEPROP_TYPE_INT32:        dumpDec< sal_Int32 >( "value" );        break;
        case OLEPROP_TYPE_FLOAT:        dumpDec< float >( "value" );            break;
        case OLEPROP_TYPE_DOUBLE:       dumpDec< double >( "value" );           break;
        case OLEPROP_TYPE_DATE:         dumpDec< double >( "date" );            break;
        case OLEPROP_TYPE_STRING:       dumpString8( "value" );                 break;
        case OLEPROP_TYPE_STATUS:       dumpHex< sal_Int32 >( "status" );       break;
        case OLEPROP_TYPE_BOOL:         dumpBool< sal_Int16 >( "value" );       break;
        case OLEPROP_TYPE_VARIANT:      dumpPropertyContents( nPropId );        break;
        case OLEPROP_TYPE_INT8:         dumpDec< sal_Int8 >( "value" );         break;
        case OLEPROP_TYPE_UINT8:        dumpDec< sal_uInt8 >( "value" );        break;
        case OLEPROP_TYPE_UINT16:       dumpDec< sal_uInt16 >( "value" );       break;
        case OLEPROP_TYPE_UINT32:       dumpDec< sal_uInt32 >( "value" );       break;
        case OLEPROP_TYPE_INT64:        dumpDec< sal_Int64 >( "value" );        break;
        case OLEPROP_TYPE_UINT64:       dumpDec< sal_uInt64 >( "value" );       break;
        case OLEPROP_TYPE_STRING8:      dumpString8( "value" );                 break;
        case OLEPROP_TYPE_STRING16:     dumpString16( "value" );                break;
        case OLEPROP_TYPE_FILETIME:     dumpFileTime( "file-time" );            break;
        case OLEPROP_TYPE_BLOB:         dumpBlob( "data" );                     break;
        case OLEPROP_TYPE_STREAM:       dumpString8( "stream-name" );           break;
        case OLEPROP_TYPE_STORAGE:      dumpString8( "storage-name" );          break;
        case OLEPROP_TYPE_CLIPFMT:      dumpBlob( "clip-data" );                break;
    }
}

sal_Int32 OlePropertyStreamObject::dumpPropertyType()
{
    return dumpHex< sal_Int32 >( "type", "OLEPROP-TYPE" );
}

void OlePropertyStreamObject::dumpBlob( const String& rName )
{
    sal_Int32 nSize = dumpDec< sal_Int32 >( "data-size" );
    if( nSize > 0 )
        dumpBinary( rName, nSize );
}

OUString OlePropertyStreamObject::dumpString8( const String& rName )
{
    sal_Int32 nLen = dumpDec< sal_Int32 >( "string-len" );
    return mbIsUnicode ? dumpCharArray16( rName, nLen ) : dumpCharArray8( rName, nLen );
}

OUString OlePropertyStreamObject::dumpCharArray8( const String& rName, sal_Int32 nLen )
{
    OUString aData;
    size_t nNewLen = getLimitedValue< size_t, sal_Int32 >( nLen, 0, 1024 );
    if( nNewLen > 0 )
    {
        ::std::vector< sal_Char > aBuffer( nNewLen + 1 );
        in().readMemory( &aBuffer.front(), nNewLen );
        aBuffer[ nNewLen ] = 0;
        aData = OStringToOUString( OString( &aBuffer.front() ), meTextEnc );
    }
    writeStringItem( rName, aData );
    return aData;
}

OUString OlePropertyStreamObject::dumpString16( const String& rName )
{
    sal_Int32 nLen = dumpDec< sal_Int32 >( "string-len" );
    return dumpCharArray16( rName, nLen );
}

OUString OlePropertyStreamObject::dumpCharArray16( const String& rName, sal_Int32 nLen )
{
    size_t nNewLen = getLimitedValue< size_t, sal_Int32 >( nLen, 0, 1024 );
    ::std::vector< sal_Unicode > aBuffer;
    aBuffer.reserve( nNewLen + 1 );
    for( size_t nIdx = 0; nIdx < nNewLen; ++nIdx )
        aBuffer.push_back( static_cast< sal_Unicode >( in().readuInt16() ) );
    aBuffer.push_back( 0 );
    OUString aData( &aBuffer.front() );
    writeStringItem( rName, aData );
    if( nNewLen & 1 ) dumpUnused( 2 ); // always padding to 32bit
    return aData;
}

bool OlePropertyStreamObject::startElement( sal_uInt32 nStartPos )
{
    in().seek( nStartPos );
    if( in().isEof() )
        writeInfoItem( "stream-state", OOX_DUMP_ERR_STREAM );
    return !in().isEof();
}

void OlePropertyStreamObject::writeSectionHeader( const OUString& rGuid, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( out() );
    writeEmptyItem( "SECTION" );
    writeHexItem( "pos", nStartPos, "CONV-DEC" );
    writeGuidItem( "guid", rGuid );
}

void OlePropertyStreamObject::writePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( out() );
    writeEmptyItem( "PROPERTY" );
    writeHexItem( "pos", nStartPos, "CONV-DEC" );
    writeDecItem( "id", nPropId, mxPropIds );
}

// ============================================================================

OleStorageObject::OleStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath )
{
    construct( rParent, rxStrg, rSysPath );
}

void OleStorageObject::construct( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath )
{
    StorageObjectBase::construct( rParent, rxStrg, rSysPath );
}

void OleStorageObject::construct( const ObjectBase& rParent )
{
    StorageObjectBase::construct( rParent );
}

void OleStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& /*rStrgPath*/, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( rStrmName.equalsAscii( "\005SummaryInformation" ) || rStrmName.equalsAscii( "\005DocumentSummaryInformation" ) )
        OlePropertyStreamObject( *this, rxStrm, rSysFileName ).dump();
    else
        BinaryStreamObject( *this, rxStrm, rSysFileName ).dump();
}

// ============================================================================
// ============================================================================

void OcxPropertyObjectBase::construct( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, const String& rPropNameList, bool b64BitPropFlags )
{
    InputObjectBase::construct( rParent, rxStrm, rSysFileName );
    constructOcxPropObj( rPropNameList, b64BitPropFlags );
}

void OcxPropertyObjectBase::construct( const OutputObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const String& rPropNameList, bool b64BitPropFlags )
{
    InputObjectBase::construct( rParent, rxStrm );
    constructOcxPropObj( rPropNameList, b64BitPropFlags );
}

void OcxPropertyObjectBase::construct( const InputObjectBase& rParent,
        const String& rPropNameList, bool b64BitPropFlags )
{
    InputObjectBase::construct( rParent );
    constructOcxPropObj( rPropNameList, b64BitPropFlags );
}

bool OcxPropertyObjectBase::implIsValid() const
{
    return InputObjectBase::implIsValid() && in().isSeekable();
}

void OcxPropertyObjectBase::implDump()
{
    mbValid = true;
    // header
    setAlignAnchor();
    dumpVersion();
    sal_uInt16 nSize = dumpDec< sal_uInt16 >( "size" );
    mnPropertiesEnd = in().tell() + nSize;
    // property flags
    maLargeProps.clear();
    maStreamProps.clear();
    mnPropFlags = dumpHex< sal_Int64, sal_uInt32 >( mb64BitPropFlags, "properties", mxPropNames );
    mnCurrProp = 0;
    // properties
    dumpShortProperties();
    dumpLargeProperties();
    setAlignAnchor();
    if( ensureValid() )
        implDumpExtended();
}

void OcxPropertyObjectBase::implDumpShortProperties()
{
}

void OcxPropertyObjectBase::implDumpExtended()
{
}

bool OcxPropertyObjectBase::ensureValid( bool bCondition )
{
    if( mbValid && (!bCondition || in().isEof()) )
    {
        if( !bCondition )
            writeInfoItem( "state", OOX_DUMP_ERRASCII( "format-error" ) );
        mbValid = false;
    }
    return mbValid;
}

void OcxPropertyObjectBase::setAlignAnchor()
{
    mnPropertiesStart = in().tell();
}

bool OcxPropertyObjectBase::startNextProperty()
{
    if( mnCurrProp == 0 ) mnCurrProp = 1; else mnCurrProp <<= 1;
    bool bHasProp = getFlag( mnPropFlags, mnCurrProp );
    setFlag( mnPropFlags, mnCurrProp, false );
    return ensureValid() && bHasProp;
}

OUString OcxPropertyObjectBase::getPropertyName() const
{
    return cfg().getName( mxPropNames, mnCurrProp );
}

sal_uInt32 OcxPropertyObjectBase::dumpFlagsProperty( sal_uInt32 nDefault, const sal_Char* pcNameList )
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        return dumpHex< sal_uInt32 >( getPropertyName(), pcNameList );
    }
    return nDefault;
}

sal_uInt32 OcxPropertyObjectBase::dumpColorProperty( sal_uInt32 nDefault )
{
    if( startNextProperty() )
    {
        MultiItemsGuard aMultiGuard( out() );
        alignInput< sal_uInt32 >();
        sal_uInt32 nColor = dumpHex< sal_uInt32 >( getPropertyName(), "OCX-COLOR" );
        switch( extractValue< sal_uInt8 >( nColor, 24, 8 ) )
        {
            case 0x00:  writeColorABGRItem( "rgb", extractValue< sal_Int32 >( nColor, 0, 24 ) );                        break;
            case 0x01:  writeDecItem( "palette-index", extractValue< sal_uInt16 >( nColor, 0, 16 ) );                   break;
            case 0x02:  writeColorABGRItem( "rgb", extractValue< sal_Int32 >( nColor, 0, 24 ) );                        break;
            case 0x80:  writeDecItem( "sys-color", extractValue< sal_uInt16 >( nColor, 0, 16 ), "OCX-SYSTEMCOLOR" );    break;
        }
        return nColor;
    }
    return nDefault;
}

sal_Unicode OcxPropertyObjectBase::dumpUnicodeProperty()
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt16 >();
        return dumpUnicode( getPropertyName() );
    }
    return '\0';
}

void OcxPropertyObjectBase::dumpUnknownProperty()
{
    if( startNextProperty() )
        ensureValid( false );
}

void OcxPropertyObjectBase::dumpPosProperty()
{
    if( startNextProperty() )
        maLargeProps.push_back( LargeProperty( LargeProperty::PROPTYPE_POS, getPropertyName(), 8 ) );
}

void OcxPropertyObjectBase::dumpSizeProperty()
{
    if( startNextProperty() )
        maLargeProps.push_back( LargeProperty( LargeProperty::PROPTYPE_SIZE, getPropertyName(), 8 ) );
}

void OcxPropertyObjectBase::dumpGuidProperty( OUString* pValue )
{
    if( startNextProperty() )
        maLargeProps.push_back( LargeProperty( LargeProperty::PROPTYPE_GUID, getPropertyName(), 16, pValue ) );
}

void OcxPropertyObjectBase::dumpStringProperty( OUString* pValue )
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        sal_uInt32 nLen = dumpHex< sal_uInt32 >( getPropertyName(), "OCX-STRINGLEN" );
        maLargeProps.push_back( LargeProperty( LargeProperty::PROPTYPE_STRING, getPropertyName(), nLen, pValue ) );
    }
}

void OcxPropertyObjectBase::dumpStringArrayProperty()
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        sal_uInt32 nLen = dumpHex< sal_uInt32 >( getPropertyName(), "CONV-DEC" );
        maLargeProps.push_back( LargeProperty( LargeProperty::PROPTYPE_STRINGARRAY, getPropertyName(), nLen ) );
    }
}

void OcxPropertyObjectBase::dumpStreamProperty()
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt16 >();
        sal_uInt16 nData = dumpHex< sal_uInt16 >( getPropertyName() );
        maStreamProps.push_back( StreamProperty( getPropertyName(), nData ) );
    }
}

void OcxPropertyObjectBase::dumpEmbeddedFont()
{
    if( ensureValid() )
    {
        writeEmptyItem( "embedded-fontdata" );
        IndentGuard aIndGuard( out() );
        OcxCFontNewObject( *this ).dump();
    }
}

void OcxPropertyObjectBase::dumpToPosition( sal_Int64 nPos )
{
    dumpRemainingTo( nPos );
    mbValid = true;
    ensureValid();
}

void OcxPropertyObjectBase::constructOcxPropObj( const String& rPropNameList, bool b64BitPropFlags )
{
    if( InputObjectBase::implIsValid() )
    {
        mxPropNames = cfg().getNameList( rPropNameList );
        mb64BitPropFlags = b64BitPropFlags;
        mbValid = true;
    }
}

void OcxPropertyObjectBase::dumpVersion()
{
    ItemGuard aItem( out(), "version" );
    sal_uInt8 nMinor, nMajor;
    in() >> nMinor >> nMajor;
    out().writeDec( nMajor );
    out().writeChar( '.' );
    out().writeDec( nMinor );
}

OUString OcxPropertyObjectBase::dumpString( const String& rName, sal_uInt32 nSize, bool bArray )
{
    bool bCompressed = getFlag( nSize, OCX_STRING_COMPRESSED );
    sal_uInt32 nBufSize = extractValue< sal_uInt32 >( nSize, 0, 31 );
    OUString aString = bCompressed ?
        dumpCharArray( rName, nBufSize, RTL_TEXTENCODING_ISO_8859_1 ) :
        dumpUnicodeArray( rName, bArray ? nBufSize : (nBufSize / 2) );
    alignInput< sal_Int32 >();
    return aString;
}

void OcxPropertyObjectBase::dumpShortProperties()
{
    if( ensureValid() )
    {
        writeEmptyItem( "short-properties" );
        IndentGuard aIndGuard( out() );
        implDumpShortProperties();
        alignInput< sal_uInt32 >();
    }
}

void OcxPropertyObjectBase::dumpLargeProperties()
{
    if( ensureValid( mnPropFlags == 0 ) && !maLargeProps.empty() )
    {
        writeEmptyItem( "large-properties" );
        IndentGuard aIndGuard( out() );
        for( LargePropertyVector::iterator aIt = maLargeProps.begin(), aEnd = maLargeProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
        {
            switch( aIt->mePropType )
            {
                case LargeProperty::PROPTYPE_POS:
                {
                    MultiItemsGuard aMultiGuard( out() );
                    writeEmptyItem( aIt->maItemName );
                    dumpDec< sal_Int32 >( "top", "CONV-HMM-TO-CM" );
                    dumpDec< sal_Int32 >( "left", "CONV-HMM-TO-CM" );
                }
                break;
                case LargeProperty::PROPTYPE_SIZE:
                {
                    MultiItemsGuard aMultiGuard( out() );
                    writeEmptyItem( aIt->maItemName );
                    dumpDec< sal_Int32 >( "width", "CONV-HMM-TO-CM" );
                    dumpDec< sal_Int32 >( "height", "CONV-HMM-TO-CM" );
                }
                break;
                case LargeProperty::PROPTYPE_GUID:
                {
                    OUString aGuid = dumpGuid( aIt->maItemName );
                    if( aIt->mpItemValue )
                        *aIt->mpItemValue = cfg().getStringOption( aGuid, OUString() );
                }
                break;
                case LargeProperty::PROPTYPE_STRING:
                {
                    OUString aString = dumpString( aIt->maItemName, aIt->mnDataSize, false );
                    if( aIt->mpItemValue )
                        *aIt->mpItemValue = aString;
                }
                break;
                case LargeProperty::PROPTYPE_STRINGARRAY:
                {
                    writeEmptyItem( aIt->maItemName );
                    IndentGuard aIndGuard2( out() );
                    out().resetItemIndex();
                    sal_Int64 nEndPos = in().tell() + aIt->mnDataSize;
                    while( in().tell() < nEndPos )
                    {
                        MultiItemsGuard aMultiGuard( out() );
                        sal_uInt32 nDataSize = dumpHex< sal_uInt32 >( "#flags", "OCX-ARRAYSTRINGLEN" );
                        dumpString( "string", nDataSize, true );
                    }
                    dumpToPosition( nEndPos );
                }
                break;
            }
        }
    }
    dumpToPosition( mnPropertiesEnd );

    if( ensureValid() && !maStreamProps.empty() )
    {
        writeEmptyItem( "stream-properties" );
        IndentGuard aIndGuard( out() );
        for( StreamPropertyVector::iterator aIt = maStreamProps.begin(), aEnd = maStreamProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
        {
            writeEmptyItem( aIt->maItemName );
            if( ensureValid( aIt->mnData == 0xFFFF ) )
            {
                IndentGuard aIndGuard2( out() );
                OUString aClassName = cfg().getStringOption( dumpGuid(), OUString() );
                if( aClassName.equalsAscii( "StdFont" ) )
                    StdFontObject( *this ).dump();
                else if( aClassName.equalsAscii( "StdPic" ) )
                    StdPicObject( *this ).dump();
                else if( aClassName.equalsAscii( "CFontNew" ) )
                    OcxCFontNewObject( *this ).dump();
                else
                    ensureValid( false );
            }
        }
    }
}

// ============================================================================

OcxCFontNewObject::OcxCFontNewObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-CFONTNEW-PROPERTIES" );
}

void OcxCFontNewObject::implDumpShortProperties()
{
    dumpStringProperty();
    dumpFlagsProperty( 0, "OCX-CFONTNEW-FLAGS" );
    dumpDecProperty< sal_Int32 >( 160 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_uInt8 >( 1, "CHARSET" );
    dumpDecProperty< sal_uInt8 >( 0, "FONT-PITCHFAMILY" );
    dumpDecProperty< sal_uInt8 >( 1, "OCX-CFONTNEW-ALIGNMENT" );
    dumpDecProperty< sal_uInt16 >( 400, "FONT-WEIGHT" );
}

// ============================================================================

OcxColumnInfoObject::OcxColumnInfoObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-COLUMNINFO-PROPERTIES" );
}

void OcxColumnInfoObject::implDumpShortProperties()
{
    dumpDecProperty< sal_Int32 >( -1, "CONV-HMM-TO-CM" );
}

// ============================================================================

OcxCommandButtonObject::OcxCommandButtonObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-COMMANDBUTTON-PROPERTIES" );
}

void OcxCommandButtonObject::implDumpShortProperties()
{
    dumpColorProperty( 0x80000012 );
    dumpColorProperty( 0x80000008 );
    dumpFlagsProperty( 0x0000001B );
    dumpStringProperty();
    dumpImagePosProperty();
    dumpSizeProperty();
    dumpMousePtrProperty();
    dumpStreamProperty();
    dumpUnicodeProperty();
    dumpBoolProperty();
    dumpStreamProperty();
}

void OcxCommandButtonObject::implDumpExtended()
{
    dumpEmbeddedFont();
}

// ============================================================================

OcxMorphControlObject::OcxMorphControlObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-MORPH-PROPERTIES", true );
}

void OcxMorphControlObject::implDumpShortProperties()
{
    dumpFlagsProperty( 0x2C80081B );
    dumpColorProperty( 0x80000005 );
    dumpColorProperty( 0x80000008 );
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpBorderStyleProperty< sal_uInt8 >( 0 );
    dumpDecProperty< sal_uInt8 >( 0, "OCX-MORPH-SCROLLBARS" );
    mnCtrlType = dumpDecProperty< sal_uInt8 >( 1, "OCX-MORPH-CONTROLTYPE" );
    dumpMousePtrProperty();
    dumpSizeProperty();
    dumpUnicodeProperty();
    dumpDecProperty< sal_uInt32 >( 0, "CONV-HMM-TO-CM" );
    dumpDecProperty< sal_uInt16 >( 1, "OCX-MORPH-BOUNDCOLUMN" );
    dumpDecProperty< sal_Int16 >( -1, "OCX-MORPH-TEXTCOLUMN" );
    dumpDecProperty< sal_Int16 >( 1, "OCX-MORPH-COLUMNCOUNT" );
    dumpDecProperty< sal_uInt16 >( 8 );
    mnColInfoCount = dumpDecProperty< sal_uInt16 >( 1 );
    dumpDecProperty< sal_uInt8 >( 2, "OCX-MORPH-MATCHENTRYTYPE" );
    dumpDecProperty< sal_uInt8 >( 0, "OCX-MORPH-LISTSTYLE" );
    dumpDecProperty< sal_uInt8 >( 0, "OCX-MORPH-SHOWDROPDOWNMODE" );
    dumpUnknownProperty();
    dumpDecProperty< sal_uInt8 >( 1, "OCX-MORPH-DROPDOWNSTYLE" );
    dumpDecProperty< sal_uInt8 >( 0, "OCX-MORPH-SELECTIONTYPE" );
    dumpStringProperty();
    dumpStringProperty();
    dumpImagePosProperty();
    dumpColorProperty( 0x80000006 );
    dumpSpecialEffectProperty< sal_uInt32 >( 2 );
    dumpStreamProperty();
    dumpStreamProperty();
    dumpUnicodeProperty();
    dumpUnknownProperty();
    dumpBoolProperty();
    dumpStringProperty();
}

void OcxMorphControlObject::implDumpExtended()
{
    dumpEmbeddedFont();
    dumpColumnInfos();
}

void OcxMorphControlObject::dumpColumnInfos()
{
    if( ensureValid() && (mnColInfoCount > 0) && ((mnCtrlType == 2) || (mnCtrlType == 3)) )
    {
        out().resetItemIndex();
        for( sal_uInt16 nIdx = 0; ensureValid() && (nIdx < mnColInfoCount); ++nIdx )
        {
            writeEmptyItem( "#column-info" );
            IndentGuard aIndGuard( out() );
            OcxColumnInfoObject( *this ).dump();
        }
    }
}

// ============================================================================

OcxLabelObject::OcxLabelObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-LABEL-PROPERTIES" );
}

void OcxLabelObject::implDumpShortProperties()
{
    dumpColorProperty( 0x80000012 );
    dumpColorProperty( 0x8000000F );
    dumpFlagsProperty( 0x0080001B );
    dumpStringProperty();
    dumpImagePosProperty();
    dumpSizeProperty();
    dumpMousePtrProperty();
    dumpColorProperty( 0x80000006 );
    dumpBorderStyleProperty< sal_uInt16 >( 0 );
    dumpSpecialEffectProperty< sal_uInt16 >( 0 );
    dumpStreamProperty();
    dumpUnicodeProperty();
    dumpBoolProperty();
    dumpStreamProperty();
}

void OcxLabelObject::implDumpExtended()
{
    dumpEmbeddedFont();
}

// ============================================================================

OcxImageObject::OcxImageObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-IMAGE-PROPERTIES" );
}

void OcxImageObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    dumpUnknownProperty();
    dumpBoolProperty();
    dumpColorProperty( 0x80000006 );
    dumpColorProperty( 0x8000000F );
    dumpBorderStyleProperty< sal_uInt8 >( 1 );
    dumpMousePtrProperty();
    dumpImageSizeModeProperty();
    dumpSpecialEffectProperty< sal_uInt8 >( 0 );
    dumpSizeProperty();
    dumpStreamProperty();
    dumpImageAlignProperty();
    dumpBoolProperty();
    dumpFlagsProperty( 0x0000001B );
    dumpStreamProperty();
}

// ============================================================================

OcxScrollBarObject::OcxScrollBarObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-SCROLLBAR-PROPERTIES" );
}

void OcxScrollBarObject::implDumpShortProperties()
{
    dumpColorProperty( 0x80000012 );
    dumpColorProperty( 0x8000000F );
    dumpFlagsProperty( 0x0000001B );
    dumpSizeProperty();
    dumpMousePtrProperty();
    dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_Int32 >( 32767 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpHexProperty< sal_uInt32 >( 0 );
    dumpEnabledProperty();
    dumpEnabledProperty();
    dumpDecProperty< sal_Int32 >( 1 );
    dumpDecProperty< sal_Int32 >( 1 );
    dumpOrientationProperty();
    dumpDecProperty< sal_Int16 >( -1, "OCX-SCROLLBAR-PROPTHUMB" );
    dumpDelayProperty();
    dumpStreamProperty();
}

// ============================================================================

OcxSpinButtonObject::OcxSpinButtonObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-SPINBUTTON-PROPERTIES" );
}

void OcxSpinButtonObject::implDumpShortProperties()
{
    dumpColorProperty( 0x80000012 );
    dumpColorProperty( 0x8000000F );
    dumpFlagsProperty( 0x0000001B );
    dumpSizeProperty();
    dumpHexProperty< sal_uInt32 >( 0 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_Int32 >( 100 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpEnabledProperty();
    dumpEnabledProperty();
    dumpDecProperty< sal_Int32 >( 1 );
    dumpOrientationProperty();
    dumpDelayProperty();
    dumpStreamProperty();
    dumpMousePtrProperty();
}

// ============================================================================

OcxTabStripObject::OcxTabStripObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-TABSTRIP-PROPERTIES" );
}

void OcxTabStripObject::implDumpShortProperties()
{
    dumpDecProperty< sal_Int32 >( -1 );
    dumpColorProperty( 0x8000000F );
    dumpColorProperty( 0x80000012 );
    dumpUnknownProperty();
    dumpSizeProperty();
    dumpStringArrayProperty();
    dumpMousePtrProperty();
    dumpUnknownProperty();
    dumpDecProperty< sal_uInt32 >( 0, "OCX-TABSTRIP-ORIENTATION" );
    dumpDecProperty< sal_uInt32 >( 0, "OCX-TABSTRIP-TABSTYLE" );
    dumpBoolProperty();
    dumpHmmProperty();
    dumpHmmProperty();
    dumpBoolProperty();
    dumpUnknownProperty();
    dumpStringArrayProperty();
    dumpUnknownProperty();
    dumpStringArrayProperty();
    dumpFlagsProperty( 0x0000001B );
    dumpBoolProperty();
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpStringArrayProperty();
    mnTabFlagCount = dumpDecProperty< sal_Int32 >( 0 );
    dumpStringArrayProperty();
    dumpStreamProperty();
}

void OcxTabStripObject::implDumpExtended()
{
    dumpEmbeddedFont();
    if( mnTabFlagCount > 0 )
    {
        writeEmptyItem( "tab-flags" );
        IndentGuard aIndGuard( out() );
        out().resetItemIndex();
        for( sal_Int32 nIdx = 0; ensureValid() && (nIdx < mnTabFlagCount); ++nIdx )
            dumpHex< sal_uInt32 >( "#flags", "OCX-TABSTRIP-FLAGS" );
    }
}

// ============================================================================

OcxControlObject::OcxControlObject( const InputObjectBase& rParent, const OUString& rProgId, sal_Int64 nLength ) :
    maProgId( rProgId ),
    mnLength( nLength )
{
    InputObjectBase::construct( rParent );
}

void OcxControlObject::implDump()
{
    sal_Int64 nEndPos = in().tell() + ::std::max< sal_Int64 >( mnLength, 0 );
    if( (maProgId.getLength() > 0) && (mnLength > 0) && !in().isEof() )
    {
        if( maProgId.equalsAscii( "Forms.CommandButton.1" ) )
            OcxCommandButtonObject( *this ).dump();
        else if( maProgId.equalsAscii( "Forms.TextBox.1" ) ||
                 maProgId.equalsAscii( "Forms.ListBox.1" ) ||
                 maProgId.equalsAscii( "Forms.ComboBox.1" ) ||
                 maProgId.equalsAscii( "Forms.CheckBox.1" ) ||
                 maProgId.equalsAscii( "Forms.OptionButton.1" ) ||
                 maProgId.equalsAscii( "Forms.ToggleButton.1" ) ||
                 maProgId.equalsAscii( "RefEdit.Ctrl" ) )
            OcxMorphControlObject( *this ).dump();
        else if( maProgId.equalsAscii( "Forms.Label.1" ) )
            OcxLabelObject( *this ).dump();
        else if( maProgId.equalsAscii( "Forms.Image.1" ) )
            OcxImageObject( *this ).dump();
        else if( maProgId.equalsAscii( "Forms.ScrollBar.1" ) )
            OcxScrollBarObject( *this ).dump();
        else if( maProgId.equalsAscii( "Forms.SpinButton.1" ) )
            OcxSpinButtonObject( *this ).dump();
        else if( maProgId.equalsAscii( "Forms.TabStrip.1" ) )
            OcxTabStripObject( *this ).dump();
        else if( maProgId.equalsAscii( "StdPic" ) )
            StdPicObject( *this ).dump();
    }
    dumpRemainingTo( nEndPos );
}

// ============================================================================

OcxGuidControlObject::OcxGuidControlObject( const InputObjectBase& rParent, sal_Int64 nLength ) :
    mnLength( nLength )
{
    InputObjectBase::construct( rParent );
}

OcxGuidControlObject::OcxGuidControlObject( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm )
{
    mnLength = rxStrm.get() ? rxStrm->getLength() : -1;
    InputObjectBase::construct( rParent, rxStrm );
}

OcxGuidControlObject::OcxGuidControlObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    mnLength = rxStrm.get() ? rxStrm->getLength() : -1;
    InputObjectBase::construct( rParent, rxStrm, rSysFileName );
}

void OcxGuidControlObject::implDump()
{
    OUString aProgId = cfg().getStringOption( dumpGuid(), OUString() );
    OcxControlObject( *this, aProgId, mnLength - 16 ).dump();
}

// ============================================================================

OcxControlsStreamObject::OcxControlsStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, OcxFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    InputObjectBase::construct( rParent, rxStrm, rSysFileName );
}

void OcxControlsStreamObject::implDump()
{
    for( OcxFormSiteInfoVector::iterator aIt = mrFormData.maSiteInfos.begin(), aEnd = mrFormData.maSiteInfos.end(); !in().isEof() && (aIt != aEnd); ++aIt )
    {
        if( (aIt->mbInStream) && (aIt->mnLength > 0) )
        {
            out().emptyLine();
            writeDecItem( "control-id", aIt->mnId );
            writeInfoItem( "prog-id", aIt->maProgId );
            IndentGuard aIndGuard( out() );
            OcxControlObject( *this, aIt->maProgId, aIt->mnLength ).dump();
        }
    }
    dumpRemainingStream();
}

// ============================================================================
// ============================================================================

OcxPageObject::OcxPageObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-PAGE-PROPERTIES" );
}

void OcxPageObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    dumpDecProperty< sal_uInt32 >( 0, "OCX-PAGE-TRANSITIONEFFECT" );
    dumpDecProperty< sal_uInt32 >( 0, "OCX-CONV-MS" );
}

// ============================================================================

OcxMultiPageObject::OcxMultiPageObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-MULTIPAGE-PROPERTIES" );
}

void OcxMultiPageObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    mnPageCount = dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpBoolProperty();
}

void OcxMultiPageObject::implDumpExtended()
{
    if( ensureValid() && (mnPageCount > 0) )
    {
        writeEmptyItem( "page-ids" );
        IndentGuard aIndGuard( out() );
        out().resetItemIndex();
        for( sal_Int32 nIdx = 0; ensureValid() && (nIdx < mnPageCount); ++nIdx )
            dumpDec< sal_Int32 >( "#id" );
    }
}

// ============================================================================

OcxMultiPageStreamObject::OcxMultiPageStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, OcxFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    InputObjectBase::construct( rParent, rxStrm,  rSysFileName );
}

void OcxMultiPageStreamObject::implDump()
{
    for( size_t nIdx = 0, nCount = mrFormData.maSiteInfos.size(); !in().isEof() && (nIdx < nCount); ++nIdx )
    {
        out().emptyLine();
        writeEmptyItem( "page" );
        IndentGuard aIndGuard( out() );
        OcxPageObject( *this ).dump();
    }
    if( !in().isEof() )
    {
        out().emptyLine();
        writeEmptyItem( "multi-page" );
        IndentGuard aIndGuard( out() );
        OcxMultiPageObject( *this ).dump();
    }
    dumpRemainingStream();
}

// ============================================================================
// ============================================================================

OcxFormClassInfoObject::OcxFormClassInfoObject( const InputObjectBase& rParent, OcxFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-CLASSINFO-PROPERTIES" );
}

void OcxFormClassInfoObject::implDumpShortProperties()
{
    mrFormData.maClassInfoProgIds.push_back( OUString() );
    dumpGuidProperty( &mrFormData.maClassInfoProgIds.back() );
    dumpGuidProperty();
    dumpUnknownProperty();
    dumpGuidProperty();
    dumpFlagsProperty( 0, "OCX-CLASSINFO-FLAGS" );
    dumpDecProperty< sal_Int32 >( -1 );
    dumpDecProperty< sal_uInt16 >( 0 );
    dumpDecProperty< sal_uInt16 >( 0 );
    dumpDecProperty< sal_uInt16 >( 0, "OLEPROP-TYPE" );
    dumpDecProperty< sal_uInt16 >( 0 );
    dumpDecProperty< sal_uInt16 >( 0 );
    dumpDecProperty< sal_uInt16 >( 0, "OLEPROP-TYPE" );
    dumpDecProperty< sal_Int32 >( -1 );
    dumpDecProperty< sal_uInt16 >( 0 );
}

// ============================================================================

namespace {

const sal_uInt32 OCX_FORMSITE_OBJSTREAM         = 0x0010;

const sal_uInt16 OCX_FORMSITE_CLASSTABLEINDEX   = 0x8000;
const sal_uInt16 OCX_FORMSITE_CLASSTABLEMASK    = 0x7FFF;

} // namespace

// ----------------------------------------------------------------------------

OcxFormSiteObject::OcxFormSiteObject( const InputObjectBase& rParent, OcxFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-FORMSITE-PROPERTIES" );
}

void OcxFormSiteObject::implDumpShortProperties()
{
    OcxFormSiteInfo aSiteInfo;
    dumpStringProperty();
    dumpStringProperty();
    sal_Int32 nId = dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_Int32 >( 0 );
    sal_uInt32 nFlags = dumpFlagsProperty( 0x00000033, "OCX-FORMSITE-FLAGS" );
    sal_uInt32 nLength = dumpDecProperty< sal_uInt32 >( 0 );
    dumpDecProperty< sal_Int16 >( -1 );
    sal_uInt16 nClassId = dumpHexProperty< sal_uInt16 >( 0x7FFF, "OCX-FORMSITE-CLASSIDCACHE" );
    dumpPosProperty();
    dumpDecProperty< sal_uInt16 >( 0 );
    dumpUnknownProperty();
    dumpStringProperty();
    dumpStringProperty();
    dumpStringProperty();
    dumpStringProperty();

    sal_uInt16 nIndex = nClassId & OCX_FORMSITE_CLASSTABLEMASK;
    if( getFlag( nClassId, OCX_FORMSITE_CLASSTABLEINDEX ) )
    {
        if( nIndex < mrFormData.maClassInfoProgIds.size() )
            aSiteInfo.maProgId = mrFormData.maClassInfoProgIds[ nIndex ];
    }
    else
    {
        if( cfg().hasName( "OCX-FORMSITE-CLASSNAMES", nIndex ) )
            aSiteInfo.maProgId = cfg().getName( "OCX-FORMSITE-CLASSNAMES", nIndex );
    }
    aSiteInfo.mnId = nId;
    aSiteInfo.mnLength = nLength;
    aSiteInfo.mbInStream = getFlag( nFlags, OCX_FORMSITE_OBJSTREAM );

    mrFormData.maSiteInfos.push_back( aSiteInfo );
}

// ============================================================================

OcxFormDesignExtObject::OcxFormDesignExtObject( const InputObjectBase& rParent )
{
    OcxPropertyObjectBase::construct( rParent, "OCX-FORMDESIGNEXT-PROPERTIES" );
}

void OcxFormDesignExtObject::implDumpShortProperties()
{
    dumpFlagsProperty( 0x00015F55, "OCX-FORMDESIGNEXT-FLAGS" );
    dumpHmmProperty();
    dumpHmmProperty();
    dumpDecProperty< sal_Int8 >( 0, "OCX-FORMDESIGNEXT-CLICKCTRLMODE" );
    dumpDecProperty< sal_Int8 >( 0, "OCX-FORMDESIGNEXT-DBLCLICKCTRLMODE" );
}

// ============================================================================

namespace {

const sal_uInt32 OCX_FORM_HASDESIGNEXTENDER     = 0x00004000;
const sal_uInt32 OCX_FORM_SKIPCLASSTABLE        = 0x00008000;

const sal_uInt8 OCX_FORM_SITECOUNTTYPE_COUNT    = 0x80;
const sal_uInt8 OCX_FORM_SITECOUNTTYPE_MASK     = 0x7F;

} // namespace

// ----------------------------------------------------------------------------

OcxFormObject::OcxFormObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, OcxFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    OcxPropertyObjectBase::construct( rParent, rxStrm, rSysFileName, "OCX-FORM-PROPERTIES" );
}

void OcxFormObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    dumpColorProperty( 0x8000000F );
    dumpColorProperty( 0x80000012 );
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpUnknownProperty();
    dumpUnknownProperty();
    mnFlags = dumpFlagsProperty( 0x00000004, "OCX-FORM-FLAGS" );
    dumpBorderStyleProperty< sal_uInt8 >( 0 );
    dumpMousePtrProperty();
    dumpHexProperty< sal_uInt8 >( 0x0C, "OCX-FORM-SCROLLBARS" );
    dumpSizeProperty();
    dumpSizeProperty();
    dumpPosProperty();
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpUnknownProperty();
    dumpStreamProperty();
    dumpDecProperty< sal_uInt8 >( 0, "OCX-FORM-CYCLE" );
    dumpSpecialEffectProperty< sal_uInt8 >( 0 );
    dumpColorProperty( 0x80000012 );
    dumpStringProperty();
    dumpStreamProperty();
    dumpStreamProperty();
    dumpDecProperty< sal_Int32 >( 100, "CONV-PERCENT" );
    dumpImageAlignProperty();
    dumpBoolProperty();
    dumpImageSizeModeProperty();
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpDecProperty< sal_uInt32 >( 0 );
}

void OcxFormObject::implDumpExtended()
{
    dumpClassInfos();
    dumpSiteData();
    dumpDesignExtender();
    dumpRemainingStream();
}

void OcxFormObject::dumpClassInfos()
{
    if( ensureValid() && !getFlag( mnFlags, OCX_FORM_SKIPCLASSTABLE ) )
    {
        out().emptyLine();
        sal_uInt16 nCount = dumpDec< sal_uInt16 >( "class-info-count" );
        out().resetItemIndex();
        for( sal_uInt16 nIdx = 0; ensureValid() && (nIdx < nCount); ++nIdx )
        {
            writeEmptyItem( "#class-info" );
            IndentGuard aIndGuard( out() );
            OcxFormClassInfoObject( *this, mrFormData ).dump();
        }
    }
}

void OcxFormObject::dumpFormSites( sal_uInt32 nCount )
{
    out().resetItemIndex();
    for( sal_uInt32 nIdx = 0; ensureValid() && (nIdx < nCount); ++nIdx )
    {
        out().emptyLine();
        writeEmptyItem( "#form-site" );
        IndentGuard aIndGuard( out() );
        OcxFormSiteObject( *this, mrFormData ).dump();
    }
}

void OcxFormObject::dumpSiteData()
{
    if( ensureValid() )
    {
        out().emptyLine();
        setAlignAnchor();
        sal_uInt32 nSiteCount = dumpDec< sal_uInt32 >( "site-count" );
        sal_uInt32 nSiteLength = dumpDec< sal_uInt32 >( "site-data-size" );
        sal_Int64 nEndPos = in().tell() + nSiteLength;
        if( ensureValid( nEndPos <= in().getLength() ) )
        {
            out().resetItemIndex();
            sal_uInt32 nSiteIdx = 0;
            while( ensureValid() && (nSiteIdx < nSiteCount) )
            {
                out().emptyLine();
                writeEmptyItem( "#site-info" );
                IndentGuard aIndGuard( out() );
                dumpDec< sal_uInt8 >( "depth" );
                sal_uInt8 nTypeCount = dumpHex< sal_uInt8 >( "type-count", "OCX-FORM-SITE-TYPECOUNT" );
                if( getFlag( nTypeCount, OCX_FORM_SITECOUNTTYPE_COUNT ) )
                {
                    dumpDec< sal_uInt8 >( "repeated-type" );
                    nSiteIdx += (nTypeCount & OCX_FORM_SITECOUNTTYPE_MASK);
                }
                else
                {
                    ++nSiteIdx;
                }
            }
            alignInput< sal_uInt32 >();
            dumpFormSites( nSiteCount );
            dumpToPosition( nEndPos );
        }
    }
}

void OcxFormObject::dumpDesignExtender()
{
    if( ensureValid() && getFlag( mnFlags, OCX_FORM_HASDESIGNEXTENDER ) )
    {
        out().emptyLine();
        writeEmptyItem( "design-extender" );
        IndentGuard aIndGuard( out() );
        OcxFormDesignExtObject( *this ).dump();
    }
}

// ============================================================================

OcxFormStorageObject::OcxFormStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath ) :
    OleStorageObject( rParent, rxStrg, rSysPath )
{
    addPreferredStream( "f" );
}

void OcxFormStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( rStrmName.equalsAscii( "f" ) )
        OcxFormObject( *this, rxStrm, rSysFileName, maFormData ).dump();
    else if( rStrmName.equalsAscii( "o" ) )
        OcxControlsStreamObject( *this, rxStrm, rSysFileName, maFormData ).dump();
    else if( rStrmName.equalsAscii( "x" ) )
        OcxMultiPageStreamObject( *this, rxStrm, rSysFileName, maFormData ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

void OcxFormStorageObject::implDumpStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    if( isFormStorage( rStrgPath ) )
        OcxFormStorageObject( *this, rxStrg, rSysPath ).dump();
    else
        OleStorageObject( *this, rxStrg, rSysPath ).dump();
}

bool OcxFormStorageObject::isFormStorage( const OUString& rStrgPath ) const
{
    if( (rStrgPath.getLength() >= 3) && (rStrgPath[ 0 ] == 'i') )
    {
        OUString aId = rStrgPath.copy( 1 );
        if( (aId.getLength() == 2) && (aId[ 0 ] == '0') )
            aId = aId.copy( 1 );
        sal_Int32 nId = aId.toInt32();
        if( (nId > 0) && (OUString::valueOf( nId ) == aId) )
            for( OcxFormSiteInfoVector::const_iterator aIt = maFormData.maSiteInfos.begin(), aEnd = maFormData.maSiteInfos.end(); aIt != aEnd; ++aIt )
                if( aIt->mnId == nId )
                    return true;
    }
    return false;
}

// ============================================================================
// ============================================================================

VbaSharedData::VbaSharedData() :
    meTextEnc( osl_getThreadTextEncoding() )
{
}

bool VbaSharedData::isModuleStream( const ::rtl::OUString& rStrmName ) const
{
    return maStrmOffsets.count( rStrmName ) > 0;
}

sal_Int32 VbaSharedData::getStreamOffset( const OUString& rStrmName ) const
{
    StreamOffsetMap::const_iterator aIt = maStrmOffsets.find( rStrmName );
    return (aIt == maStrmOffsets.end()) ? 0 : aIt->second;
}

// ============================================================================

VbaDirStreamObject::VbaDirStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, VbaSharedData& rVbaData ) :
    mrVbaData( rVbaData )
{
    mxInStrm = rxStrm;
    if( mxInStrm.get() )
    {
        BinaryInputStreamRef xVbaStrm( new ::oox::ole::VbaInputStream( *mxInStrm ) );
        SequenceRecordObjectBase::construct( rParent, xVbaStrm, rSysFileName, "VBA-DIR-RECORD-NAMES", "VBA-DIR-SIMPLE-RECORDS" );
    }
}

bool VbaDirStreamObject::implIsValid() const
{
    return mxInStrm.get() && SequenceRecordObjectBase::implIsValid();
}

bool VbaDirStreamObject::implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize )
{
    ornRecId = rBaseStrm.readuInt16();
    ornRecSize = rBaseStrm.readInt32();

    // for no obvious reason, PROJECTVERSION record contains size field of 4, but is 6 bytes long
    if( ornRecId == 9 )
        ornRecSize = 6;

    return !rBaseStrm.isEof();
}

void VbaDirStreamObject::implDumpRecordBody()
{
    switch( getRecId() )
    {
        case 0x0003:
            mrVbaData.meTextEnc = ::oox::xls::BiffHelper::calcTextEncodingFromCodePage( dumpDec< sal_uInt16 >( "codepage", "CODEPAGES" ) );
        break;
        case 0x0004:
            dumpByteString( "name" );
        break;
        case 0x0005:
            dumpByteString( "description" );
        break;
        case 0x0006:
            dumpByteString( "helpfile-path" );
        break;
        case 0x0009:
            dumpDec< sal_uInt32 >( "major" );
            dumpDec< sal_uInt16 >( "minor" );
        break;
        case 0x000C:
            dumpByteString( "constants" );
        break;
        case 0x000D:
            dumpByteStringWithLength( "lib-id" );
            dumpUnused( 6 );
        break;
        case 0x000E:
            dumpByteStringWithLength( "lib-id-absolute" );
            dumpByteStringWithLength( "lib-id-relative" );
            dumpDec< sal_uInt32 >( "major" );
            dumpDec< sal_uInt16 >( "minor" );
        break;
        case 0x0016:
            dumpByteString( "name" );
        break;
        case 0x0019:
            dumpByteString( "name" );
            maCurrStream = OUString();
            mnCurrOffset = 0;
        break;
        case 0x001A:
            maCurrStream = dumpByteString( "stream-name" );
        break;
        case 0x001C:
            dumpByteString( "description" );
        break;
        case 0x002B:
            if( maCurrStream.getLength() > 0 )
                mrVbaData.maStrmOffsets[ maCurrStream ] = mnCurrOffset;
            maCurrStream = OUString();
            mnCurrOffset = 0;
        break;
        case 0x002F:
            dumpByteStringWithLength( "lib-id-twiddled" );
            dumpUnused( 6 );
        break;
        case 0x0030:
            dumpByteStringWithLength( "lib-id-extended" );
            dumpUnused( 6 );
            dumpGuid( "original-typelib" );
            dumpDec< sal_uInt32 >( "cookie" );
        break;
        case 0x0031:
            mnCurrOffset = dumpHex< sal_Int32 >( "stream-offset", "CONV-DEC" );
        break;
        case 0x0032:
            dumpUniString( "stream-name" );
        break;
        case 0x0033:
            dumpByteString( "lib-id-original" );
        break;
        case 0x003C:
            dumpUniString( "constants" );
        break;
        case 0x003D:
            dumpUniString( "helpfile-path" );
        break;
        case 0x003E:
            dumpUniString( "name" );
        break;
        case 0x0040:
            dumpUniString( "description" );
        break;
        case 0x0047:
            dumpUniString( "name" );
        break;
        case 0x0048:
            dumpUniString( "description" );
        break;
    }
}

OUString VbaDirStreamObject::dumpByteString( const String& rName )
{
    return dumpCharArray( rName, static_cast< sal_Int32 >( getRecSize() ), mrVbaData.meTextEnc );
}

OUString VbaDirStreamObject::dumpUniString( const String& rName )
{
    return dumpUnicodeArray( rName, static_cast< sal_Int32 >( getRecSize() / 2 ) );
}

OUString VbaDirStreamObject::dumpByteStringWithLength( const String& rName )
{
    return dumpCharArray( rName, in().readInt32(), mrVbaData.meTextEnc );
}

// ============================================================================

VbaModuleStreamObject::VbaModuleStreamObject(
        const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm,
        const OUString& rSysFileName, VbaSharedData& rVbaData, sal_Int32 nStrmOffset ) :
    mrVbaData( rVbaData ),
    mnStrmOffset( nStrmOffset )
{
    InputObjectBase::construct( rParent, rxStrm, rSysFileName );
}

void VbaModuleStreamObject::implDump()
{
    dumpBinary( "perf-cache", mnStrmOffset );
    out().emptyLine();
    writeEmptyItem( "source-code" );
    IndentGuard aIndGuard( out() );
    BinaryInputStreamRef xVbaStrm( new ::oox::ole::VbaInputStream( in() ) );
    TextStreamObject( *this, xVbaStrm, mrVbaData.meTextEnc ).dump();
}

// ============================================================================

VbaStorageObject::VbaStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath, VbaSharedData& rVbaData ) :
    OleStorageObject( rParent, rxStrg, rSysPath ),
    mrVbaData( rVbaData )
{
    addPreferredStream( "dir" );
}

void VbaStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( (rStrgPath.getLength() == 0) && rStrmName.equalsAscii( "dir" ) )
        VbaDirStreamObject( *this, rxStrm, rSysFileName, mrVbaData ).dump();
    else if( mrVbaData.isModuleStream( rStrmName ) )
        VbaModuleStreamObject( *this, rxStrm, rSysFileName, mrVbaData, mrVbaData.getStreamOffset( rStrmName ) ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

// ============================================================================

VbaFormStorageObject::VbaFormStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath, VbaSharedData& rVbaData ) :
    OcxFormStorageObject( rParent, rxStrg, rSysPath ),
    mrVbaData( rVbaData )
{
}

void VbaFormStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( rStrmName.equalsAscii( "\003VBFrame" ) )
        TextStreamObject( *this, rxStrm, mrVbaData.meTextEnc, rSysFileName ).dump();
    else
        OcxFormStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

// ============================================================================

VbaProjectStorageObject::VbaProjectStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath ) :
    OleStorageObject( rParent, rxStrg, rSysPath )
{
    addPreferredStorage( "VBA" );
}

void VbaProjectStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( (rStrgPath.getLength() == 0) && rStrmName.equalsAscii( "PROJECT" ) )
        TextStreamObject( *this, rxStrm, maVbaData.meTextEnc, rSysFileName ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

void VbaProjectStorageObject::implDumpStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    if( rStrgPath.equalsAscii( "VBA" ) )
        VbaStorageObject( *this, rxStrg, rSysPath, maVbaData ).dump();
    else
        VbaFormStorageObject( *this, rxStrg, rSysPath, maVbaData ).dump();
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#endif

