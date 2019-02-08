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

#include <oox/dump/oledumper.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>
#include <oox/core/filterbase.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/ole/vbainputstream.hxx>

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

OUString OleInputObjectBase::dumpAnsiString32( const String& rName )
{
    return dumpCharArray( rName, mxStrm->readInt32(), RTL_TEXTENCODING_MS_1252 );
}

OUString OleInputObjectBase::dumpUniString32( const String& rName )
{
    return dumpUnicodeArray( rName, mxStrm->readInt32() );
}

sal_Int32 OleInputObjectBase::dumpStdClipboardFormat( const String& rName )
{
    return dumpDec< sal_Int32 >( rName( "clipboard-format" ), "OLE-STD-CLIPBOARD-FORMAT" );
}

OUString OleInputObjectBase::dumpAnsiString32OrStdClip( const String& rName )
{
    sal_Int32 nLen = mxStrm->readInt32();
    return (nLen < 0) ? OUString::number( dumpStdClipboardFormat( rName ) ) : dumpCharArray( rName, nLen, RTL_TEXTENCODING_MS_1252 );
}

OUString OleInputObjectBase::dumpUniString32OrStdClip( const String& rName )
{
    sal_Int32 nLen = mxStrm->readInt32();
    return (nLen < 0) ? OUString::number( dumpStdClipboardFormat( rName ) ) : dumpUnicodeArray( rName, nLen );
}

void OleInputObjectBase::writeOleColorItem( const String& rName, sal_uInt32 nColor )
{
    MultiItemsGuard aMultiGuard( mxOut );
    writeHexItem( rName, nColor, "OLE-COLOR" );
}

sal_uInt32 OleInputObjectBase::dumpOleColor( const String& rName )
{
    sal_uInt32 nOleColor = mxStrm->readuInt32();
    writeOleColorItem( rName, nOleColor );
    return nOleColor;
}

StdFontObject::StdFontObject( const InputObjectBase& rParent )
{
    construct( rParent );
}

void StdFontObject::implDump()
{
    dumpDec< sal_uInt8 >( "version" );
    dumpDec< sal_uInt16 >( "charset", "CHARSET" );
    dumpHex< sal_uInt8 >( "flags", "STDFONT-FLAGS" );
    dumpDec< sal_uInt16 >( "weight", "FONT-WEIGHT" );
    dumpDec< sal_uInt32 >( "height", "STDFONT-HEIGHT" );
    dumpCharArray( "name", mxStrm->readuInt8(), RTL_TEXTENCODING_ASCII_US );
}

StdPicObject::StdPicObject( const InputObjectBase& rParent )
{
    construct( rParent );
}

void StdPicObject::implDump()
{
    dumpHex< sal_uInt32 >( "identifier", "STDPIC-ID" );
    sal_uInt32 nSize = dumpHex< sal_uInt32 >( "image-size", "CONV-DEC" );
    dumpBinary( "image-data", nSize );
}

OleStreamObject::OleStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    construct( rParent, rxStrm, rSysFileName );
}

OleCompObjObject::OleCompObjObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName ) :
    OleStreamObject( rParent, rxStrm, rSysFileName )
{
}

void OleCompObjObject::implDump()
{
    dumpUnused( 4 );
    dumpDec< sal_uInt32 >( "version" );
    dumpUnused( 20 );
    dumpAnsiString32( "ansi-display-name" );
    dumpAnsiString32OrStdClip( "ansi-clipboard-format" );
    if( mxStrm->getRemaining() >= 4 )
    {
        sal_Int32 nLen = mxStrm->readInt32();
        if( (0 <= nLen) && (nLen <= 40) )
        {
            dumpCharArray( "ansi-unused", nLen, RTL_TEXTENCODING_MS_1252 );
            if( (mxStrm->getRemaining() >= 4) && (dumpHex< sal_Int32 >( "unicode-marker" ) == 0x71B239F4) )
            {
                dumpUniString32( "unicode-display-name" );
                dumpUniString32OrStdClip( "unicode-clipboard-format" );
                dumpUniString32( "unicode-unused" );
            }
        }
        else
            writeDecItem( "length", nLen );
    }
    dumpRemainingStream();
}

namespace {

const sal_Int32 OLEPROP_ID_DICTIONARY   = 0;
const sal_Int32 OLEPROP_ID_CODEPAGE     = 1;

const sal_uInt16 OLEPROP_TYPE_INT16     = 2;
const sal_uInt16 OLEPROP_TYPE_INT32     = 3;
const sal_uInt16 OLEPROP_TYPE_FLOAT     = 4;
const sal_uInt16 OLEPROP_TYPE_DOUBLE    = 5;
const sal_uInt16 OLEPROP_TYPE_DATE      = 7;
const sal_uInt16 OLEPROP_TYPE_STRING    = 8;
const sal_uInt16 OLEPROP_TYPE_STATUS    = 10;
const sal_uInt16 OLEPROP_TYPE_BOOL      = 11;
const sal_uInt16 OLEPROP_TYPE_VARIANT   = 12;
const sal_uInt16 OLEPROP_TYPE_INT8      = 16;
const sal_uInt16 OLEPROP_TYPE_UINT8     = 17;
const sal_uInt16 OLEPROP_TYPE_UINT16    = 18;
const sal_uInt16 OLEPROP_TYPE_UINT32    = 19;
const sal_uInt16 OLEPROP_TYPE_INT64     = 20;
const sal_uInt16 OLEPROP_TYPE_UINT64    = 21;
const sal_uInt16 OLEPROP_TYPE_STRING8   = 30;
const sal_uInt16 OLEPROP_TYPE_STRING16  = 31;
const sal_uInt16 OLEPROP_TYPE_FILETIME  = 64;
const sal_uInt16 OLEPROP_TYPE_BLOB      = 65;
const sal_uInt16 OLEPROP_TYPE_STREAM    = 66;
const sal_uInt16 OLEPROP_TYPE_STORAGE   = 67;
const sal_uInt16 OLEPROP_TYPE_CLIPFMT   = 71;

const sal_uInt16 OLEPROP_TYPE_SIMPLE    = 0x0000;
const sal_uInt16 OLEPROP_TYPE_VECTOR    = 0x1000;
const sal_uInt16 OLEPROP_TYPE_ARRAY     = 0x2000;

const sal_uInt16 CODEPAGE_UNICODE       = 1200;

const sal_uInt32 AX_STRING_COMPRESSED   = 0x80000000;

} // namespace

OlePropertyStreamObject::OlePropertyStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    construct( rParent, rxStrm, rSysFileName );
}

void OlePropertyStreamObject::implDump()
{
    OUStringVector aGuidVec;
    ::std::vector< sal_uInt32 > aStartPosVec;

    // dump header
    writeEmptyItem( "HEADER" );
    {
        IndentGuard aIndGuard( mxOut );
        dumpHex< sal_uInt16 >( "byte-order", "OLEPROP-BYTE-ORDER" );
        dumpDec< sal_uInt16 >( "version" );
        dumpDec< sal_uInt16 >( "os-minor" );
        dumpDec< sal_uInt16 >( "os-type", "OLEPROP-OSTYPE" );
        dumpGuid( "guid" );
        sal_Int32 nSectCount = dumpDec< sal_Int32 >( "section-count" );

        // dump table of section positions
        {
            TableGuard aTabGuard( mxOut, 15, 60 );
            mxOut->resetItemIndex();
            for( sal_Int32 nSectIdx = 0; !mxStrm->isEof() && (nSectIdx < nSectCount); ++nSectIdx )
            {
                MultiItemsGuard aMultiGuard( mxOut );
                writeEmptyItem( "#section" );
                aGuidVec.push_back( dumpGuid( "guid" ) );
                aStartPosVec.push_back( dumpHex< sal_uInt32 >( "start-pos", "CONV-DEC" ) );
            }
        }
    }
    mxOut->emptyLine();

    // dump sections
    for( size_t nSectIdx = 0; !mxStrm->isEof() && (nSectIdx < aStartPosVec.size()); ++nSectIdx )
        dumpSection( aGuidVec[ nSectIdx ], aStartPosVec[ nSectIdx ] );
}

void OlePropertyStreamObject::dumpSection( const OUString& rGuid, sal_uInt32 nStartPos )
{
    // property ID names
    mxPropIds = cfg().createNameList< ConstList >( "OLEPROP-IDS" );
    OUString aGuidName = cfg().getStringOption( rGuid, OUString() );
    if ( aGuidName == "GlobalDocProp" )
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-GLOBALIDS" ) );
    else if ( aGuidName == "BuiltinDocProp" )
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-BUILTINIDS" ) );
    else
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-BASEIDS" ) );

    // property ID/position map
    typedef ::std::map< sal_Int32, sal_uInt32 > PropertyPosMap;
    PropertyPosMap aPropMap;

    // dump section header line
    writeSectionHeader( rGuid, nStartPos );

    // seek to section
    IndentGuard aIndGuard( mxOut );
    if( startElement( nStartPos ) )
    {
        // dump section header
        dumpDec< sal_Int32 >( "size" );
        sal_Int32 nPropCount = dumpDec< sal_Int32 >( "property-count" );

        // dump table of property positions
        {
            TableGuard aTabGuard( mxOut, 15, 25 );
            mxOut->resetItemIndex();
            for( sal_Int32 nPropIdx = 0; !mxStrm->isEof() && (nPropIdx < nPropCount); ++nPropIdx )
            {
                MultiItemsGuard aMultiGuard( mxOut );
                writeEmptyItem( "#property" );
                sal_Int32 nPropId = dumpDec< sal_Int32 >( "id", mxPropIds );
                sal_uInt32 nPropPos = nStartPos + dumpHex< sal_uInt32 >( "start-pos", "CONV-DEC" );
                aPropMap[ nPropId ] = nPropPos;
            }
        }
    }
    mxOut->emptyLine();

    // code page property
    meTextEnc = RTL_TEXTENCODING_MS_1252;
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
    for (auto const& elem : aPropMap)
        dumpProperty( elem.first, elem.second );

    // remove the user defined list of property ID names
    cfg().eraseNameList( "OLEPROP-IDS" );
}

void OlePropertyStreamObject::dumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    writePropertyHeader( nPropId, nStartPos );
    IndentGuard aIndGuard( mxOut );
    if( startElement( nStartPos ) )
        dumpPropertyContents( nPropId );
    mxOut->emptyLine();
}

void OlePropertyStreamObject::dumpCodePageProperty( sal_uInt32 nStartPos )
{
    writePropertyHeader( OLEPROP_ID_CODEPAGE, nStartPos );
    IndentGuard aIndGuard( mxOut );
    if( startElement( nStartPos ) )
    {
        sal_uInt16 nType = dumpPropertyType();
        if( nType == OLEPROP_TYPE_INT16 )
        {
            sal_uInt16 nCodePage = dumpDec< sal_uInt16 >( "codepage", "CODEPAGES" );
            rtl_TextEncoding eNewTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
            if( eNewTextEnc != RTL_TEXTENCODING_DONTKNOW )
                meTextEnc = eNewTextEnc;
            mbIsUnicode = nCodePage == CODEPAGE_UNICODE;
        }
        else
            dumpPropertyContents( OLEPROP_ID_CODEPAGE );
    }
    mxOut->emptyLine();
}

void OlePropertyStreamObject::dumpDictionaryProperty( sal_uInt32 nStartPos )
{
    writePropertyHeader( OLEPROP_ID_DICTIONARY, nStartPos );
    IndentGuard aIndGuard( mxOut );
    if( startElement( nStartPos ) )
    {
        sal_Int32 nCount = dumpDec< sal_Int32 >( "count" );
        for( sal_Int32 nIdx = 0; !mxStrm->isEof() && (nIdx < nCount); ++nIdx )
        {
            MultiItemsGuard aMultiGuard( mxOut );
            TableGuard aTabGuard( mxOut, 10, 20 );
            sal_Int32 nId = dumpDec< sal_Int32 >( "id" );
            OUString aName = dumpString8( "name" );
            if( mxPropIds.get() )
                mxPropIds->setName( nId, aName );
        }
    }
    mxOut->emptyLine();
}

sal_uInt16 OlePropertyStreamObject::dumpPropertyContents( sal_Int32 nPropId )
{
    sal_uInt16 nType = dumpPropertyType();
    sal_uInt16 nBaseType = static_cast< sal_uInt16 >( nType & 0x0FFF );
    sal_uInt16 nArrayType = static_cast< sal_uInt16 >( nType & 0xF000 );
    switch( nArrayType )
    {
        case OLEPROP_TYPE_SIMPLE:   dumpPropertyValue( nPropId, nBaseType );    break;
        case OLEPROP_TYPE_VECTOR:   dumpPropertyVector( nPropId, nBaseType );   break;
        case OLEPROP_TYPE_ARRAY:    /*TODO*/;    break;
    }
    return nType;
}

void OlePropertyStreamObject::dumpPropertyValue( sal_Int32 nPropId, sal_uInt16 nBaseType )
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
        case OLEPROP_TYPE_BLOB:         dumpBlob( nPropId, "data" );            break;
        case OLEPROP_TYPE_STREAM:       dumpString8( "stream-name" );           break;
        case OLEPROP_TYPE_STORAGE:      dumpString8( "storage-name" );          break;
        case OLEPROP_TYPE_CLIPFMT:      dumpBlob( nPropId, "clip-data" );       break;
    }
}

void OlePropertyStreamObject::dumpPropertyVector( sal_Int32 nPropId, sal_uInt16 nBaseType )
{
    sal_Int32 nElemCount = dumpDec< sal_Int32 >( "element-count" );
    for( sal_Int32 nElemIdx = 0; !mxStrm->isEof() && (nElemIdx < nElemCount); ++nElemIdx )
    {
        mxOut->resetItemIndex( nElemIdx );
        writeEmptyItem( "#element" );
        IndentGuard aIndGuard( mxOut );
        dumpPropertyValue( nPropId, nBaseType );
    }
}

sal_uInt16 OlePropertyStreamObject::dumpPropertyType()
{
    return static_cast< sal_uInt16 >( dumpHex< sal_Int32 >( "type", "OLEPROP-TYPE" ) & 0xFFFF );
}

void OlePropertyStreamObject::dumpBlob( sal_Int32 nPropId, const String& rName )
{
    sal_Int32 nSize = dumpDec< sal_Int32 >( "data-size" );
    if( nSize > 0 )
    {
        OUString aPropName = mxPropIds->getName( cfg(), nPropId );
        if( aPropName == "'_PID_HLINKS'" )
            dumpHlinks( nSize );
        else
            dumpBinary( rName, nSize );
    }
}

OUString OlePropertyStreamObject::dumpString8( const String& rName )
{
    sal_Int32 nLen = dumpDec< sal_Int32 >( "string-len" );
    return mbIsUnicode ? dumpCharArray16( rName, nLen ) : dumpCharArray8( rName, nLen );
}

OUString OlePropertyStreamObject::dumpCharArray8( const String& rName, sal_Int32 nLen )
{
    sal_Int32 nNewLen = getLimitedValue< sal_Int32, sal_Int32 >( nLen, 0, 1024 );
    OUString aData = mxStrm->readCharArrayUC( nNewLen, meTextEnc );
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
    sal_Int32 nNewLen = getLimitedValue< sal_Int32, sal_Int32 >( nLen, 0, 1024 );
    OUString aData = mxStrm->readUnicodeArray( nNewLen );
    writeStringItem( rName, aData );
    if( nNewLen & 1 ) dumpUnused( 2 ); // always padding to 32bit
    return aData;
}

bool OlePropertyStreamObject::dumpTypedProperty( const String& rName, sal_uInt16 nExpectedType )
{
    writeEmptyItem( rName );
    IndentGuard aIndGuard( mxOut );
    return (dumpPropertyContents( -1 ) == nExpectedType) && !mxStrm->isEof();
}

void OlePropertyStreamObject::dumpHlinks( sal_Int32 nSize )
{
    sal_Int64 nEndPos = mxStrm->tell() + nSize;
    sal_Int32 nCount = dumpDec< sal_Int32 >( "property-count" );
    bool bValid = true;
    for( sal_Int32 nHlinkIndex = 0, nHlinkCount = nCount / 6; bValid && !mxStrm->isEof() && (nHlinkIndex < nHlinkCount); ++nHlinkIndex )
    {
        writeEmptyItem( "HYPERLINK" );
        IndentGuard aIndGuard( mxOut );
        bValid =
            dumpTypedProperty( "hash", OLEPROP_TYPE_INT32 ) &&
            dumpTypedProperty( "app", OLEPROP_TYPE_INT32 ) &&
            dumpTypedProperty( "shape-id", OLEPROP_TYPE_INT32 ) &&
            dumpTypedProperty( "info", OLEPROP_TYPE_INT32 ) &&
            dumpTypedProperty( "target", OLEPROP_TYPE_STRING16 ) &&
            dumpTypedProperty( "location", OLEPROP_TYPE_STRING16 );
    }
    dumpRemainingTo( nEndPos );
}

bool OlePropertyStreamObject::startElement( sal_uInt32 nStartPos )
{
    mxStrm->seek( nStartPos );
    if( mxStrm->isEof() )
        writeInfoItem( "stream-state", OOX_DUMP_ERR_STREAM );
    return !mxStrm->isEof();
}

void OlePropertyStreamObject::writeSectionHeader( const OUString& rGuid, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( mxOut );
    writeEmptyItem( "SECTION" );
    writeHexItem( "pos", nStartPos, "CONV-DEC" );
    writeGuidItem( "guid", rGuid );
}

void OlePropertyStreamObject::writePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( mxOut );
    writeEmptyItem( "PROPERTY" );
    writeHexItem( "pos", nStartPos, "CONV-DEC" );
    writeDecItem( "id", nPropId, mxPropIds );
}

OleStorageObject::OleStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath )
{
    construct( rParent, rxStrg, rSysPath );
}

void OleStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& /*rStrgPath*/, const OUString& rStrmName, const OUString& rSysFileName )
{
    if ( rStrmName == "\001CompObj" )
        OleCompObjObject( *this, rxStrm, rSysFileName ).dump();
    else if( rStrmName ==  "\005SummaryInformation"  || rStrmName == "\005DocumentSummaryInformation" )
        OlePropertyStreamObject( *this, rxStrm, rSysFileName ).dump();
    else
        BinaryStreamObject( *this, rxStrm, rSysFileName ).dump();
}

ComCtlObjectBase::ComCtlObjectBase( const InputObjectBase& rParent,
        sal_uInt32 nDataId5, sal_uInt32 nDataId6, sal_uInt16 nVersion, bool bCommonPart, bool bComplexPart ) :
    mnDataId5( nDataId5 ),
    mnDataId6( nDataId6 ),
    mnVersion( nVersion ),
    mbCommonPart( bCommonPart ),
    mbComplexPart( bComplexPart )
{
    construct( rParent );
}

void ComCtlObjectBase::implDump()
{
    sal_uInt32 nCommonSize = 0;
    dumpComCtlSize() && dumpComCtlData( nCommonSize ) && (!mbCommonPart || dumpComCtlCommon( nCommonSize )) && (!mbComplexPart || dumpComCtlComplex());
}

void ComCtlObjectBase::implDumpCommonExtra( sal_Int64 /*nEndPos*/ )
{
}

void ComCtlObjectBase::implDumpCommonTrailing()
{
}

bool ComCtlObjectBase::dumpComCtlHeader( sal_uInt32 nExpId, sal_uInt16 nExpMajor, sal_uInt16 nExpMinor )
{
    // no idea if all this is correct...
    sal_uInt32 nId = dumpHex< sal_uInt32 >( "header-id", "COMCTL-HEADER-IDS" );
    ItemGuard aItem( mxOut, "version" );
    sal_uInt16 nMinor = mxStrm->readuInt16();
    sal_uInt16 nMajor = mxStrm->readuInt16();
    mxOut->writeDec( nMajor );
    mxOut->writeChar( '.' );
    mxOut->writeDec( nMinor );
    return !mxStrm->isEof() && (nId == nExpId) && ((nExpMajor == SAL_MAX_UINT16) || (nExpMajor == nMajor)) && ((nExpMinor == SAL_MAX_UINT16) || (nExpMinor == nMinor));
}

bool ComCtlObjectBase::dumpComCtlSize()
{
    if( dumpComCtlHeader( 0x12344321, 0, 8 ) )
    {
        IndentGuard aIndGuard( mxOut );
        dumpDec< sal_Int32 >( "width", "CONV-HMM-TO-CM" );
        dumpDec< sal_Int32 >( "height", "CONV-HMM-TO-CM" );
        return !mxStrm->isEof();
    }
    return false;
}

bool ComCtlObjectBase::dumpComCtlData( sal_uInt32& ornCommonPartSize )
{
    if( dumpComCtlHeader( (mnVersion == 5) ? mnDataId5 : mnDataId6, mnVersion ) )
    {
        IndentGuard aIndGuard( mxOut );
        if( mbCommonPart )
            ornCommonPartSize = dumpDec< sal_uInt32 >( "common-part-size" );
        implDumpProperties();
        return !mxStrm->isEof();
    }
    return false;
}

bool ComCtlObjectBase::dumpComCtlCommon( sal_uInt32 nPartSize )
{
    sal_Int64 nEndPos = mxStrm->tell() + nPartSize;
    if( (nPartSize >= 16) && dumpComCtlHeader( 0xABCDEF01, 5, 0 ) )
    {
        IndentGuard aIndGuard( mxOut );
        dumpUnknown( 4 );
        dumpHex< sal_uInt32 >( "common-flags", "COMCTL-COMMON-FLAGS" );
        implDumpCommonExtra( nEndPos );
        dumpRemainingTo( nEndPos );
        implDumpCommonTrailing();
        return !mxStrm->isEof();
    }
    return false;
}

bool ComCtlObjectBase::dumpComCtlComplex()
{
    if( dumpComCtlHeader( 0xBDECDE1F, 5, 1 ) )
    {
        IndentGuard aIndGuard( mxOut );
        sal_uInt32 nFlags = dumpHex< sal_uInt32 >( "comctl-complex-flags", "COMCTL-COMPLEX-FLAGS" );
        if( !mxStrm->isEof() && (nFlags & 0x01) )
        {
            writeEmptyItem( "font" );
            IndentGuard aIndGuard2( mxOut );
            OUString aClassName = cfg().getStringOption( dumpGuid(), OUString() );
            if ( aClassName == "StdFont" )
                StdFontObject( *this ).dump();
        }
        if( !mxStrm->isEof() && (nFlags & 0x02) )
        {
            writeEmptyItem( "mouse-icon" );
            IndentGuard aIndGuard2( mxOut );
            OUString aClassName = cfg().getStringOption( dumpGuid(), OUString() );
            if ( aClassName == "StdPic" )
                StdPicObject( *this ).dump();
        }
        return !mxStrm->isEof();
    }
    return false;
}

ComCtlScrollBarObject::ComCtlScrollBarObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, SAL_MAX_UINT32, 0x99470A83, nVersion, true, true )
{
}

void ComCtlScrollBarObject::implDumpProperties()
{
    dumpHex< sal_uInt32 >( "flags", "COMCTL-SCROLLBAR-FLAGS" );
    dumpDec< sal_Int32 >( "large-change" );
    dumpDec< sal_Int32 >( "small-change" );
    dumpDec< sal_Int32 >( "min" );
    dumpDec< sal_Int32 >( "max" );
    dumpDec< sal_Int32 >( "value" );
}

ComCtlProgressBarObject::ComCtlProgressBarObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xE6E17E84, 0x97AB8A01, nVersion, true, true )
{
}

void ComCtlProgressBarObject::implDumpProperties()
{
    dumpDec< float >( "min" );
    dumpDec< float >( "max" );
    if( mnVersion == 6 )
    {
        dumpBool< sal_uInt16 >( "vertical" );
        dumpBool< sal_uInt16 >( "smooth-scroll" );
    }
}

ComCtlSliderObject::ComCtlSliderObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xE6E17E86, 0x0A2BAE11, nVersion, true, true )
{
}

void ComCtlSliderObject::implDumpProperties()
{
    dumpBool< sal_Int32 >( "vertical" );
    dumpDec< sal_Int32 >( "large-change" );
    dumpDec< sal_Int32 >( "small-change" );
    dumpDec< sal_Int32 >( "min" );
    dumpDec< sal_Int32 >( "max" );
    dumpDec< sal_Int16 >( "select-range", "COMCTL-SLIDER-SELECTRANGE" );
    dumpUnused( 2 );
    dumpDec< sal_Int32 >( "select-start" );
    dumpDec< sal_Int32 >( "select-length" );
    dumpDec< sal_Int32 >( "tick-style", "COMCTL-SLIDER-TICKSTYLE" );
    dumpDec< sal_Int32 >( "tick-frequency" );
    dumpDec< sal_Int32 >( "value" );
    if( mnVersion == 6 )
        dumpBool< sal_Int32 >( "tooltip-below" );
}

ComCtlUpDownObject::ComCtlUpDownObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xFF3626A0, 0xFF3626A0, nVersion, false, false )
{
}

void ComCtlUpDownObject::implDumpProperties()
{
    dumpUnknown( 16 ); // buddy-property, somehow
    dumpDec< sal_Int32 >( "buddy-control" );
    dumpUnknown( 8 );
    dumpDec< sal_Int32 >( "value" );
    dumpUnknown( 4 );
    dumpDec< sal_Int32 >( "increment" );
    dumpDec< sal_Int32 >( "max" );
    dumpDec< sal_Int32 >( "min" );
    dumpHex< sal_uInt32 >( "flags-1", "COMCTL-UPDOWN-FLAGS1" );
    dumpHex< sal_uInt32 >( "flags-2", "COMCTL-UPDOWN-FLAGS2" );
    dumpUnknown( 4 );
}

ComCtlImageListObject::ComCtlImageListObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xE6E17E80, 0xE6E17E80, nVersion, true, false )
{
}

void ComCtlImageListObject::implDumpProperties()
{
    dumpDec< sal_uInt16 >( "image-width" );
    dumpDec< sal_uInt16 >( "image-height" );
    dumpOleColor( "mask-color" );
    dumpBool< sal_Int16 >( "use-mask-color" );
    dumpUnknown( 2 );
}

void ComCtlImageListObject::implDumpCommonExtra( sal_Int64 /*nEndPos*/ )
{
    dumpUnknown( 4 );
    dumpOleColor( "back-color" );
    dumpUnknown( 4 );
    sal_Int32 nImageCount = dumpDec< sal_Int32 >( "image-count" );
    mxOut->resetItemIndex();
    for( sal_Int32 nImageIndex = 0; (nImageIndex < nImageCount) && !mxStrm->isEof(); ++nImageIndex )
    {
        writeEmptyItem( "#image" );
        IndentGuard aIndGuard( mxOut );
        sal_uInt8 nFlags = dumpHex< sal_uInt8 >( "text-flags", "COMCTL-IMAGELIST-TEXTFLAGS" );
        if( nFlags & 0x01 ) dumpUniString32( "caption" );
        if( nFlags & 0x02 ) dumpUniString32( "key" );
    }
}

void ComCtlImageListObject::implDumpCommonTrailing()
{
    sal_Int32 nImageCount = dumpDec< sal_Int32 >( "image-count" );
    mxOut->resetItemIndex();
    for( sal_Int32 nImageIndex = 0; (nImageIndex < nImageCount) && !mxStrm->isEof(); ++nImageIndex )
    {
        writeEmptyItem( "#image" );
        IndentGuard aIndGuard( mxOut );
        dumpDec< sal_Int32 >( "index" );
        StdPicObject( *this ).dump();
    }
}

ComCtlTabStripObject::ComCtlTabStripObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xE6E17E8A, 0xD12A7AC1, nVersion, true, true )
{
}

void ComCtlTabStripObject::implDumpProperties()
{
    dumpHex< sal_uInt32 >( "flags-1", "COMCTL-TABSTRIP-FLAGS1" );
    dumpDec< sal_uInt16 >( "tab-fixed-width", "CONV-HMM-TO-CM" );
    dumpDec< sal_uInt16 >( "tab-fixed-height", "CONV-HMM-TO-CM" );
    if( mnVersion == 6 )
    {
        dumpHex< sal_uInt32 >( "flags-2", "COMCTL-TABSTRIP-FLAGS2" );
        dumpDec< sal_uInt16 >( "tab-min-width", "CONV-HMM-TO-CM" );
        dumpUnknown( 2 );
        dumpHex< sal_uInt32 >( "flags-3", "COMCTL-TABSTRIP-FLAGS3" );
    }
}

void ComCtlTabStripObject::implDumpCommonExtra( sal_Int64 /*nEndPos*/ )
{
    dumpUnknown( 12 );
    dumpUniString32( "image-list" );
    sal_Int32 nTabCount = dumpDec< sal_Int32 >( "tab-count" );
    mxOut->resetItemIndex();
    for( sal_Int32 nTabIndex = 0; (nTabIndex < nTabCount) && !mxStrm->isEof(); ++nTabIndex )
    {
        writeEmptyItem( "#tab" );
        IndentGuard aIndGuard( mxOut );
        dumpUnknown( 4 );
        sal_uInt32 nTabFlags = dumpHex< sal_uInt32 >( "tab-flags", "COMCTL-TABSTRIP-TABFLAGS" );
        if( nTabFlags & 0x01 ) dumpUniString32( "caption" );
        if( nTabFlags & 0x02 ) dumpUniString32( "key" );
        if( nTabFlags & 0x04 ) dumpUniString32( "tag" );
        if( nTabFlags & 0x08 ) dumpUniString32( "tooltip" );
        dumpDec< sal_uInt16 >( "image-id" );
    }
}

ComCtlTreeViewObject::ComCtlTreeViewObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xE6E17E8E, 0x6AC13CB1, nVersion, true, true ),
    mnStringFlags( 0 )
{
}

void ComCtlTreeViewObject::implDumpProperties()
{
    dumpHex< sal_uInt32 >( "flags", "COMCTL-TREEVIEW-FLAGS" );
    dumpDec< sal_Int32 >( "indentation", "CONV-HMM-TO-CM" );
    if( mnVersion == 6 )
        dumpHex< sal_uInt32 >( "flags-2", "COMCTL-TREEVIEW-FLAGS2" );
    mnStringFlags = dumpHex< sal_uInt32 >( "string-flags", "COMCTL-TREEVIEW-STRINGFLAGS" );
}

void ComCtlTreeViewObject::implDumpCommonExtra( sal_Int64 /*nEndPos*/ )
{
    dumpOleColor( "text-color" );
    dumpOleColor( "back-color" );
    dumpUnknown( 4 );
    if( mnStringFlags & 0x02 )
        dumpUniString32( "image-list" );
    dumpUniString32( "path-separator" );
}

ComCtlStatusBarObject::ComCtlStatusBarObject( const InputObjectBase& rParent, sal_uInt16 nVersion ) :
    ComCtlObjectBase( rParent, 0xE6E17E88, SAL_MAX_UINT32, nVersion, true, true )
{
}

void ComCtlStatusBarObject::implDumpProperties()
{
    dumpBool< sal_Int32 >( "style-simple-text" );
    dumpBool< sal_Int16 >( "show-tips" );
    dumpUnknown( 2 );
}

void ComCtlStatusBarObject::implDumpCommonExtra( sal_Int64 /*nEndPos*/ )
{
    dumpUnknown( 12 );
    dumpUniString32( "simple-text" );
    sal_Int32 nPanelCount = dumpDec< sal_Int32 >( "panel-count" );
    mxOut->resetItemIndex();
    for( sal_Int32 nPanelIndex = 0; (nPanelIndex < nPanelCount) && !mxStrm->isEof(); ++nPanelIndex )
    {
        writeEmptyItem( "#panel" );
        IndentGuard aIndGuard( mxOut );
        dumpHex< sal_uInt32 >( "panel-flags", "COMCTL-STATUSBAR-PANELFLAGS" );
        dumpDec< sal_Int32 >( "current-width", "CONV-HMM-TO-CM" );
        dumpDec< sal_Int32 >( "minimal-width", "CONV-HMM-TO-CM" );
        sal_uInt32 nTextFlags = dumpHex< sal_uInt32 >( "text-flags", "COMCTL-STATUSBAR-TEXTFLAGS" );
        if( nTextFlags & 0x01 ) dumpUniString32( "text" );
        if( nTextFlags & 0x02 ) dumpUniString32( "vis-text" );
        if( nTextFlags & 0x04 ) dumpUniString32( "key" );
        if( nTextFlags & 0x08 ) dumpUniString32( "tag" );
        if( nTextFlags & 0x10 ) dumpUniString32( "tooltip" );
    }
}

void ComCtlStatusBarObject::implDumpCommonTrailing()
{
    sal_Int32 nImageCount = dumpDec< sal_Int32 >( "image-count" );
    mxOut->resetItemIndex();
    for( sal_Int32 nImageIndex = 0; (nImageIndex < nImageCount) && !mxStrm->isEof(); ++nImageIndex )
    {
        writeEmptyItem( "#image" );
        IndentGuard aIndGuard( mxOut );
        dumpDec< sal_Int32 >( "panel-index" );
        StdPicObject( *this ).dump();
    }
}

void AxPropertyObjectBase::construct( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, const String& rPropNameList, bool b64BitPropFlags )
{
    OleInputObjectBase::construct( rParent, rxStrm, rSysFileName );
    constructAxPropObj( rPropNameList, b64BitPropFlags );
}

void AxPropertyObjectBase::construct( const InputObjectBase& rParent,
        const String& rPropNameList, bool b64BitPropFlags )
{
    OleInputObjectBase::construct( rParent );
    constructAxPropObj( rPropNameList, b64BitPropFlags );
}

bool AxPropertyObjectBase::implIsValid() const
{
    return OleInputObjectBase::implIsValid() && mxStrm->isSeekable();
}

void AxPropertyObjectBase::implDump()
{
    mbValid = true;
    // header
    setAlignAnchor();
    dumpVersion();
    sal_uInt16 nSize = dumpDec< sal_uInt16 >( "size" );
    mnPropertiesEnd = mxStrm->tell() + nSize;
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

void AxPropertyObjectBase::implDumpShortProperties()
{
}

void AxPropertyObjectBase::implDumpExtended()
{
}

bool AxPropertyObjectBase::ensureValid( bool bCondition )
{
    if( mbValid && (!bCondition || mxStrm->isEof()) )
    {
        if( !bCondition )
            writeInfoItem( "state", OOX_DUMP_ERRASCII( "format-error" ) );
        mbValid = false;
    }
    return mbValid;
}

void AxPropertyObjectBase::setAlignAnchor()
{
    mnPropertiesStart = mxStrm->tell();
}

bool AxPropertyObjectBase::startNextProperty()
{
    if( mnCurrProp == 0 ) mnCurrProp = 1; else mnCurrProp <<= 1;
    bool bHasProp = getFlag( mnPropFlags, mnCurrProp );
    setFlag( mnPropFlags, mnCurrProp, false );
    return ensureValid() && bHasProp;
}

OUString AxPropertyObjectBase::getPropertyName() const
{
    return cfg().getName( mxPropNames, mnCurrProp );
}

sal_uInt32 AxPropertyObjectBase::dumpFlagsProperty( sal_uInt32 nDefault, const sal_Char* pcNameList )
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        return dumpHex< sal_uInt32 >( getPropertyName(), pcNameList );
    }
    return nDefault;
}

sal_uInt32 AxPropertyObjectBase::dumpColorProperty( sal_uInt32 nDefault )
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        return dumpOleColor( getPropertyName() );
    }
    return nDefault;
}

sal_Unicode AxPropertyObjectBase::dumpUnicodeProperty()
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt16 >();
        return dumpUnicode( getPropertyName() );
    }
    return '\0';
}

void AxPropertyObjectBase::dumpUnknownProperty()
{
    if( startNextProperty() )
        ensureValid( false );
}

void AxPropertyObjectBase::dumpPosProperty()
{
    if( startNextProperty() )
        maLargeProps.emplace_back( LargeProperty::PROPTYPE_POS, getPropertyName(), 8 );
}

void AxPropertyObjectBase::dumpSizeProperty()
{
    if( startNextProperty() )
        maLargeProps.emplace_back( LargeProperty::PROPTYPE_SIZE, getPropertyName(), 8 );
}

void AxPropertyObjectBase::dumpGuidProperty( OUString* pValue )
{
    if( startNextProperty() )
        maLargeProps.emplace_back( LargeProperty::PROPTYPE_GUID, getPropertyName(), 16, pValue );
}

void AxPropertyObjectBase::dumpStringProperty( OUString* pValue )
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        sal_uInt32 nLen = dumpHex< sal_uInt32 >( getPropertyName(), "AX-STRINGLEN" );
        maLargeProps.emplace_back( LargeProperty::PROPTYPE_STRING, getPropertyName(), nLen, pValue );
    }
}

void AxPropertyObjectBase::dumpStringArrayProperty()
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt32 >();
        sal_uInt32 nLen = dumpHex< sal_uInt32 >( getPropertyName(), "CONV-DEC" );
        maLargeProps.emplace_back( LargeProperty::PROPTYPE_STRINGARRAY, getPropertyName(), nLen );
    }
}

void AxPropertyObjectBase::dumpStreamProperty()
{
    if( startNextProperty() )
    {
        alignInput< sal_uInt16 >();
        sal_uInt16 nData = dumpHex< sal_uInt16 >( getPropertyName() );
        maStreamProps.emplace_back( getPropertyName(), nData );
    }
}

void AxPropertyObjectBase::dumpEmbeddedFont()
{
    if( ensureValid() )
    {
        writeEmptyItem( "embedded-fontdata" );
        IndentGuard aIndGuard( mxOut );
        AxCFontNewObject( *this ).dump();
    }
}

void AxPropertyObjectBase::dumpToPosition( sal_Int64 nPos )
{
    dumpRemainingTo( nPos );
    mbValid = true;
    ensureValid();
}

void AxPropertyObjectBase::constructAxPropObj( const String& rPropNameList, bool b64BitPropFlags )
{
    if( OleInputObjectBase::implIsValid() )
    {
        mxPropNames = cfg().getNameList( rPropNameList );
        mb64BitPropFlags = b64BitPropFlags;
        mbValid = true;
    }
}

void AxPropertyObjectBase::dumpVersion()
{
    ItemGuard aItem( mxOut, "version" );
    sal_uInt8 nMinor = mxStrm->readuChar();
    sal_uInt8 nMajor = mxStrm->readuChar();
    mxOut->writeDec( nMajor );
    mxOut->writeChar( '.' );
    mxOut->writeDec( nMinor );
}

OUString AxPropertyObjectBase::dumpString( const String& rName, sal_uInt32 nSize, bool bArray )
{
    bool bCompressed = getFlag( nSize, AX_STRING_COMPRESSED );
    sal_uInt32 nBufSize = extractValue< sal_uInt32 >( nSize, 0, 31 );
    OUString aString = bCompressed ?
        dumpCharArray( rName, nBufSize, RTL_TEXTENCODING_ISO_8859_1 ) :
        dumpUnicodeArray( rName, bArray ? nBufSize : (nBufSize / 2) );
    alignInput< sal_Int32 >();
    return aString;
}

void AxPropertyObjectBase::dumpShortProperties()
{
    if( ensureValid() )
    {
        writeEmptyItem( "short-properties" );
        IndentGuard aIndGuard( mxOut );
        implDumpShortProperties();
        alignInput< sal_uInt32 >();
    }
}

void AxPropertyObjectBase::dumpLargeProperties()
{
    if( ensureValid( mnPropFlags == 0 ) && !maLargeProps.empty() )
    {
        writeEmptyItem( "large-properties" );
        IndentGuard aIndGuard( mxOut );
        for (auto const& largeProp : maLargeProps)
        {
            if (!ensureValid())
                break;
            switch( largeProp.mePropType )
            {
                case LargeProperty::PROPTYPE_POS:
                {
                    MultiItemsGuard aMultiGuard( mxOut );
                    writeEmptyItem( largeProp.maItemName );
                    dumpDec< sal_Int32 >( "top", "CONV-HMM-TO-CM" );
                    dumpDec< sal_Int32 >( "left", "CONV-HMM-TO-CM" );
                }
                break;
                case LargeProperty::PROPTYPE_SIZE:
                {
                    MultiItemsGuard aMultiGuard( mxOut );
                    writeEmptyItem( largeProp.maItemName );
                    dumpDec< sal_Int32 >( "width", "CONV-HMM-TO-CM" );
                    dumpDec< sal_Int32 >( "height", "CONV-HMM-TO-CM" );
                }
                break;
                case LargeProperty::PROPTYPE_GUID:
                {
                    OUString aGuid = dumpGuid( largeProp.maItemName );
                    if( largeProp.mpItemValue )
                        *largeProp.mpItemValue = cfg().getStringOption( aGuid, OUString() );
                }
                break;
                case LargeProperty::PROPTYPE_STRING:
                {
                    OUString aString = dumpString( largeProp.maItemName, largeProp.mnDataSize, false );
                    if( largeProp.mpItemValue )
                        *largeProp.mpItemValue = aString;
                }
                break;
                case LargeProperty::PROPTYPE_STRINGARRAY:
                {
                    writeEmptyItem( largeProp.maItemName );
                    IndentGuard aIndGuard2( mxOut );
                    mxOut->resetItemIndex();
                    sal_Int64 nEndPos = mxStrm->tell() + largeProp.mnDataSize;
                    while( mxStrm->tell() < nEndPos )
                    {
                        MultiItemsGuard aMultiGuard( mxOut );
                        sal_uInt32 nDataSize = dumpHex< sal_uInt32 >( "#flags", "AX-ARRAYSTRINGLEN" );
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
        IndentGuard aIndGuard( mxOut );
        for (auto const& streamProp : maStreamProps)
        {
            if (!ensureValid())
                break;
            writeEmptyItem( streamProp.maItemName );
            if( ensureValid( streamProp.mnData == 0xFFFF ) )
            {
                IndentGuard aIndGuard2( mxOut );
                OUString aClassName = cfg().getStringOption( dumpGuid(), OUString() );
                if ( aClassName == "StdFont" )
                    StdFontObject( *this ).dump();
                else if ( aClassName == "StdPic" )
                    StdPicObject( *this ).dump();
                else if ( aClassName == "CFontNew" )
                    AxCFontNewObject( *this ).dump();
                else
                    ensureValid( false );
            }
        }
    }
}

AxCFontNewObject::AxCFontNewObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-CFONTNEW-PROPERTIES" );
}

void AxCFontNewObject::implDumpShortProperties()
{
    dumpStringProperty();
    dumpFlagsProperty( 0, "AX-CFONTNEW-FLAGS" );
    dumpDecProperty< sal_Int32 >( 160 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_uInt8 >( WINDOWS_CHARSET_DEFAULT, "CHARSET" );
    dumpDecProperty< sal_uInt8 >( 0, "FONT-PITCHFAMILY" );
    dumpDecProperty< sal_uInt8 >( 1, "AX-CFONTNEW-ALIGNMENT" );
    dumpDecProperty< sal_uInt16 >( 400, "FONT-WEIGHT" );
}

AxColumnInfoObject::AxColumnInfoObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-COLUMNINFO-PROPERTIES" );
}

void AxColumnInfoObject::implDumpShortProperties()
{
    dumpDecProperty< sal_Int32 >( -1, "CONV-HMM-TO-CM" );
}

AxCommandButtonObject::AxCommandButtonObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-COMMANDBUTTON-PROPERTIES" );
}

void AxCommandButtonObject::implDumpShortProperties()
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

void AxCommandButtonObject::implDumpExtended()
{
    dumpEmbeddedFont();
}

AxMorphControlObject::AxMorphControlObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-MORPH-PROPERTIES", true );
}

void AxMorphControlObject::implDumpShortProperties()
{
    dumpFlagsProperty( 0x2C80081B );
    dumpColorProperty( 0x80000005 );
    dumpColorProperty( 0x80000008 );
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpBorderStyleProperty< sal_uInt8 >( 0 );
    dumpDecProperty< sal_uInt8 >( 0, "AX-MORPH-SCROLLBARS" );
    mnCtrlType = dumpDecProperty< sal_uInt8 >( 1, "AX-MORPH-CONTROLTYPE" );
    dumpMousePtrProperty();
    dumpSizeProperty();
    dumpUnicodeProperty();
    dumpDecProperty< sal_uInt32 >( 0, "CONV-HMM-TO-CM" );
    dumpDecProperty< sal_uInt16 >( 1, "AX-MORPH-BOUNDCOLUMN" );
    dumpDecProperty< sal_Int16 >( -1, "AX-MORPH-TEXTCOLUMN" );
    dumpDecProperty< sal_Int16 >( 1, "AX-MORPH-COLUMNCOUNT" );
    dumpDecProperty< sal_uInt16 >( 8 );
    mnColInfoCount = dumpDecProperty< sal_uInt16 >( 1 );
    dumpDecProperty< sal_uInt8 >( 2, "AX-MORPH-MATCHENTRYTYPE" );
    dumpDecProperty< sal_uInt8 >( 0, "AX-MORPH-LISTSTYLE" );
    dumpDecProperty< sal_uInt8 >( 0, "AX-MORPH-SHOWDROPDOWNMODE" );
    dumpUnknownProperty();
    dumpDecProperty< sal_uInt8 >( 1, "AX-MORPH-DROPDOWNSTYLE" );
    dumpDecProperty< sal_uInt8 >( 0, "AX-MORPH-SELECTIONTYPE" );
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

void AxMorphControlObject::implDumpExtended()
{
    dumpEmbeddedFont();
    dumpColumnInfos();
}

void AxMorphControlObject::dumpColumnInfos()
{
    if( ensureValid() && (mnColInfoCount > 0) && ((mnCtrlType == 2) || (mnCtrlType == 3)) )
    {
        mxOut->resetItemIndex();
        for( sal_uInt16 nIdx = 0; ensureValid() && (nIdx < mnColInfoCount); ++nIdx )
        {
            writeEmptyItem( "#column-info" );
            IndentGuard aIndGuard( mxOut );
            AxColumnInfoObject( *this ).dump();
        }
    }
}

AxLabelObject::AxLabelObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-LABEL-PROPERTIES" );
}

void AxLabelObject::implDumpShortProperties()
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
    dumpStreamProperty();
}

void AxLabelObject::implDumpExtended()
{
    dumpEmbeddedFont();
}

AxImageObject::AxImageObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-IMAGE-PROPERTIES" );
}

void AxImageObject::implDumpShortProperties()
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

AxScrollBarObject::AxScrollBarObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-SCROLLBAR-PROPERTIES" );
}

void AxScrollBarObject::implDumpShortProperties()
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
    dumpDecProperty< sal_Int16 >( -1, "AX-SCROLLBAR-PROPTHUMB" );
    dumpDelayProperty();
    dumpStreamProperty();
}

AxSpinButtonObject::AxSpinButtonObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-SPINBUTTON-PROPERTIES" );
}

void AxSpinButtonObject::implDumpShortProperties()
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

AxTabStripObject::AxTabStripObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "AX-TABSTRIP-PROPERTIES" );
}

void AxTabStripObject::implDumpShortProperties()
{
    dumpDecProperty< sal_Int32 >( -1 );
    dumpColorProperty( 0x8000000F );
    dumpColorProperty( 0x80000012 );
    dumpUnknownProperty();
    dumpSizeProperty();
    dumpStringArrayProperty();
    dumpMousePtrProperty();
    dumpUnknownProperty();
    dumpDecProperty< sal_uInt32 >( 0, "AX-TABSTRIP-ORIENTATION" );
    dumpDecProperty< sal_uInt32 >( 0, "AX-TABSTRIP-TABSTYLE" );
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

void AxTabStripObject::implDumpExtended()
{
    dumpEmbeddedFont();
    if( mnTabFlagCount > 0 )
    {
        writeEmptyItem( "tab-flags" );
        IndentGuard aIndGuard( mxOut );
        mxOut->resetItemIndex();
        for( sal_Int32 nIdx = 0; ensureValid() && (nIdx < mnTabFlagCount); ++nIdx )
            dumpHex< sal_uInt32 >( "#flags", "AX-TABSTRIP-FLAGS" );
    }
}

FormControlStreamObject::FormControlStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, const OUString* pProgId )
{
    construct( rParent, rxStrm, rSysFileName );
    constructFormCtrlStrmObj( pProgId );
}

FormControlStreamObject::FormControlStreamObject( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString* pProgId )
{
    construct( rParent, rxStrm );
    constructFormCtrlStrmObj( pProgId );
}

void FormControlStreamObject::implDump()
{
    if( mbReadGuid )
        maProgId = cfg().getStringOption( dumpGuid(), OUString() );

    if( !maProgId.isEmpty() && !mxStrm->isEof() )
    {
        if ( maProgId == "Forms.CommandButton.1" )
            AxCommandButtonObject( *this ).dump();
        else if( maProgId ==  "Forms.TextBox.1" ||
                 maProgId ==  "Forms.ListBox.1" ||
                 maProgId ==  "Forms.ComboBox.1" ||
                 maProgId ==  "Forms.CheckBox.1" ||
                 maProgId ==  "Forms.OptionButton.1" ||
                 maProgId ==  "Forms.ToggleButton.1" ||
                 maProgId ==  "RefEdit.Ctrl" )
            AxMorphControlObject( *this ).dump();
        else if ( maProgId == "Forms.Label.1" )
            AxLabelObject( *this ).dump();
        else if ( maProgId == "Forms.Image.1" )
            AxImageObject( *this ).dump();
        else if ( maProgId == "Forms.ScrollBar.1" )
            AxScrollBarObject( *this ).dump();
        else if ( maProgId == "Forms.SpinButton.1" )
            AxSpinButtonObject( *this ).dump();
        else if ( maProgId == "Forms.TabStrip.1" )
            AxTabStripObject( *this ).dump();
        else if ( maProgId == "MSComCtl2.FlatScrollBar.2" )
            ComCtlScrollBarObject( *this, 6 ).dump();
        else if ( maProgId == "COMCTL.ProgCtrl.1" )
            ComCtlProgressBarObject( *this, 5 ).dump();
        else if ( maProgId == "MSComctlLib.ProgCtrl.2" )
            ComCtlProgressBarObject( *this, 6 ).dump();
        else if ( maProgId == "COMCTL.Slider.1" )
            ComCtlSliderObject( *this, 5 ).dump();
        else if ( maProgId == "MSComctlLib.Slider.2" )
            ComCtlSliderObject( *this, 6 ).dump();
        else if ( maProgId == "ComCtl2.UpDown.1" )
            ComCtlUpDownObject( *this, 5 ).dump();
        else if ( maProgId == "MSComCtl2.UpDown.2" )
            ComCtlUpDownObject( *this, 6 ).dump();
        else if ( maProgId == "COMCTL.ImageListCtrl.1" )
            ComCtlImageListObject( *this, 5 ).dump();
        else if ( maProgId == "MSComctlLib.ImageListCtrl.2" )
            ComCtlImageListObject( *this, 6 ).dump();
        else if ( maProgId == "COMCTL.TabStrip.1" )
            ComCtlTabStripObject( *this, 5 ).dump();
        else if ( maProgId == "MSComctlLib.TabStrip.2" )
            ComCtlTabStripObject( *this, 6 ).dump();
        else if ( maProgId == "COMCTL.TreeCtrl.1" )
            ComCtlTreeViewObject( *this, 5 ).dump();
        else if ( maProgId == "MSComctlLib.TreeCtrl.2" )
            ComCtlTreeViewObject( *this, 6 ).dump();
        else if ( maProgId == "COMCTL.SBarCtrl.1" )
            ComCtlStatusBarObject( *this, 5 ).dump();
        else if ( maProgId == "StdPic" )
            StdPicObject( *this ).dump();
    }
    dumpRemainingStream();
}

void FormControlStreamObject::constructFormCtrlStrmObj( const OUString* pProgId )
{
    mbReadGuid = pProgId == nullptr;
    if( pProgId )
        maProgId = *pProgId;
}

VbaFormClassInfoObject::VbaFormClassInfoObject( const InputObjectBase& rParent, VbaFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    AxPropertyObjectBase::construct( rParent, "VBA-CLASSINFO-PROPERTIES" );
}

void VbaFormClassInfoObject::implDumpShortProperties()
{
    mrFormData.maClassInfoProgIds.emplace_back( );
    dumpGuidProperty( &mrFormData.maClassInfoProgIds.back() );
    dumpGuidProperty();
    dumpUnknownProperty();
    dumpGuidProperty();
    dumpFlagsProperty( 0, "VBA-CLASSINFO-FLAGS" );
    dumpDecProperty< sal_uInt32 >( 0 );
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

namespace {

const sal_uInt32 VBA_FORMSITE_OBJSTREAM         = 0x0010;

const sal_uInt16 VBA_FORMSITE_CLASSTABLEINDEX   = 0x8000;
const sal_uInt16 VBA_FORMSITE_CLASSTABLEMASK    = 0x7FFF;

} // namespace

VbaFormSiteObject::VbaFormSiteObject( const InputObjectBase& rParent, VbaFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    AxPropertyObjectBase::construct( rParent, "VBA-FORMSITE-PROPERTIES" );
}

void VbaFormSiteObject::implDumpShortProperties()
{
    VbaFormSiteInfo aSiteInfo;
    dumpStringProperty();
    dumpStringProperty();
    sal_Int32 nId = dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_Int32 >( 0 );
    sal_uInt32 nFlags = dumpFlagsProperty( 0x00000033, "VBA-FORMSITE-FLAGS" );
    sal_uInt32 nLength = dumpDecProperty< sal_uInt32 >( 0 );
    dumpDecProperty< sal_Int16 >( -1 );
    sal_uInt16 nClassId = dumpHexProperty< sal_uInt16 >( 0x7FFF, "VBA-FORMSITE-CLASSIDCACHE" );
    dumpPosProperty();
    dumpDecProperty< sal_uInt16 >( 0 );
    dumpUnknownProperty();
    dumpStringProperty();
    dumpStringProperty();
    dumpStringProperty();
    dumpStringProperty();

    sal_uInt16 nIndex = nClassId & VBA_FORMSITE_CLASSTABLEMASK;
    if( getFlag( nClassId, VBA_FORMSITE_CLASSTABLEINDEX ) )
    {
        if( nIndex < mrFormData.maClassInfoProgIds.size() )
            aSiteInfo.maProgId = mrFormData.maClassInfoProgIds[ nIndex ];
    }
    else
    {
        if( cfg().hasName( "VBA-FORMSITE-CLASSNAMES", nIndex ) )
            aSiteInfo.maProgId = cfg().getName( "VBA-FORMSITE-CLASSNAMES", nIndex );
    }
    aSiteInfo.mnId = nId;
    aSiteInfo.mnLength = nLength;
    aSiteInfo.mbInStream = getFlag( nFlags, VBA_FORMSITE_OBJSTREAM );

    mrFormData.maSiteInfos.push_back( aSiteInfo );
}

VbaFormDesignExtObject::VbaFormDesignExtObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "VBA-FORMDESIGNEXT-PROPERTIES" );
}

void VbaFormDesignExtObject::implDumpShortProperties()
{
    dumpFlagsProperty( 0x00015F55, "VBA-FORMDESIGNEXT-FLAGS" );
    dumpHmmProperty();
    dumpHmmProperty();
    dumpDecProperty< sal_Int8 >( 0, "VBA-FORMDESIGNEXT-CLICKCTRLMODE" );
    dumpDecProperty< sal_Int8 >( 0, "VBA-FORMDESIGNEXT-DBLCLICKCTRLMODE" );
}

namespace {

const sal_uInt32 AX_FORM_HASDESIGNEXTENDER      = 0x00004000;
const sal_uInt32 AX_FORM_SKIPCLASSTABLE         = 0x00008000;

const sal_uInt8 AX_FORM_SITECOUNTTYPE_COUNT     = 0x80;
const sal_uInt8 AX_FORM_SITECOUNTTYPE_MASK      = 0x7F;

} // namespace

VbaFStreamObject::VbaFStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, VbaFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    AxPropertyObjectBase::construct( rParent, rxStrm, rSysFileName, "VBA-FORM-PROPERTIES" );
}

void VbaFStreamObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    dumpColorProperty( 0x8000000F );
    dumpColorProperty( 0x80000012 );
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpUnknownProperty();
    dumpUnknownProperty();
    mnFlags = dumpFlagsProperty( 0x00000004, "VBA-FORM-FLAGS" );
    dumpBorderStyleProperty< sal_uInt8 >( 0 );
    dumpMousePtrProperty();
    dumpHexProperty< sal_uInt8 >( 0x0C, "VBA-FORM-SCROLLBARS" );
    dumpSizeProperty();
    dumpSizeProperty();
    dumpPosProperty();
    dumpDecProperty< sal_uInt32 >( 0 );
    dumpUnknownProperty();
    dumpStreamProperty();
    dumpDecProperty< sal_uInt8 >( 0, "VBA-FORM-CYCLE" );
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

void VbaFStreamObject::implDumpExtended()
{
    dumpClassInfos();
    dumpSiteData();
    dumpDesignExtender();
    dumpRemainingStream();
}

void VbaFStreamObject::dumpClassInfos()
{
    if( ensureValid() && !getFlag( mnFlags, AX_FORM_SKIPCLASSTABLE ) )
    {
        mxOut->emptyLine();
        sal_uInt16 nCount = dumpDec< sal_uInt16 >( "class-info-count" );
        mxOut->resetItemIndex();
        for( sal_uInt16 nIdx = 0; ensureValid() && (nIdx < nCount); ++nIdx )
        {
            writeEmptyItem( "#class-info" );
            IndentGuard aIndGuard( mxOut );
            VbaFormClassInfoObject( *this, mrFormData ).dump();
        }
    }
}

void VbaFStreamObject::dumpFormSites( sal_uInt32 nCount )
{
    mxOut->resetItemIndex();
    for( sal_uInt32 nIdx = 0; ensureValid() && (nIdx < nCount); ++nIdx )
    {
        mxOut->emptyLine();
        writeEmptyItem( "#form-site" );
        IndentGuard aIndGuard( mxOut );
        VbaFormSiteObject( *this, mrFormData ).dump();
    }
}

void VbaFStreamObject::dumpSiteData()
{
    if( ensureValid() )
    {
        mxOut->emptyLine();
        setAlignAnchor();
        sal_uInt32 nSiteCount = dumpDec< sal_uInt32 >( "site-count" );
        sal_uInt32 nSiteLength = dumpDec< sal_uInt32 >( "site-data-size" );
        sal_Int64 nEndPos = mxStrm->tell() + nSiteLength;
        if( ensureValid( nEndPos <= mxStrm->size() ) )
        {
            mxOut->resetItemIndex();
            sal_uInt32 nSiteIdx = 0;
            while( ensureValid() && (nSiteIdx < nSiteCount) )
            {
                mxOut->emptyLine();
                writeEmptyItem( "#site-info" );
                IndentGuard aIndGuard( mxOut );
                dumpDec< sal_uInt8 >( "depth" );
                sal_uInt8 nTypeCount = dumpHex< sal_uInt8 >( "type-count", "VBA-FORM-SITE-TYPECOUNT" );
                if( getFlag( nTypeCount, AX_FORM_SITECOUNTTYPE_COUNT ) )
                {
                    dumpDec< sal_uInt8 >( "repeated-type" );
                    nSiteIdx += (nTypeCount & AX_FORM_SITECOUNTTYPE_MASK);
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

void VbaFStreamObject::dumpDesignExtender()
{
    if( ensureValid() && getFlag( mnFlags, AX_FORM_HASDESIGNEXTENDER ) )
    {
        mxOut->emptyLine();
        writeEmptyItem( "design-extender" );
        IndentGuard aIndGuard( mxOut );
        VbaFormDesignExtObject( *this ).dump();
    }
}

VbaOStreamObject::VbaOStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, VbaFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    OleInputObjectBase::construct( rParent, rxStrm, rSysFileName );
}

void VbaOStreamObject::implDump()
{
    for (auto const& siteInfo : mrFormData.maSiteInfos)
    {
        if (mxStrm->isEof())
            break;
        if( (siteInfo.mbInStream) && (siteInfo.mnLength > 0) )
        {
            mxOut->emptyLine();
            writeDecItem( "control-id", siteInfo.mnId );
            writeInfoItem( "prog-id", siteInfo.maProgId );
            IndentGuard aIndGuard( mxOut );
            BinaryInputStreamRef xRelStrm( new RelativeInputStream( *mxStrm, siteInfo.mnLength ) );
            FormControlStreamObject( *this, xRelStrm, &siteInfo.maProgId ).dump();
        }
    }
    dumpRemainingStream();
}

VbaPageObject::VbaPageObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "VBA-PAGE-PROPERTIES" );
}

void VbaPageObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    dumpDecProperty< sal_uInt32 >( 0, "VBA-PAGE-TRANSITIONEFFECT" );
    dumpDecProperty< sal_uInt32 >( 0, "AX-CONV-MS" );
}

VbaMultiPageObject::VbaMultiPageObject( const InputObjectBase& rParent )
{
    AxPropertyObjectBase::construct( rParent, "VBA-MULTIPAGE-PROPERTIES" );
}

void VbaMultiPageObject::implDumpShortProperties()
{
    dumpUnknownProperty();
    mnPageCount = dumpDecProperty< sal_Int32 >( 0 );
    dumpDecProperty< sal_Int32 >( 0 );
    dumpBoolProperty();
}

void VbaMultiPageObject::implDumpExtended()
{
    if( ensureValid() && (mnPageCount > 0) )
    {
        writeEmptyItem( "page-ids" );
        IndentGuard aIndGuard( mxOut );
        mxOut->resetItemIndex();
        for( sal_Int32 nIdx = 0; ensureValid() && (nIdx < mnPageCount); ++nIdx )
            dumpDec< sal_Int32 >( "#id" );
    }
}

VbaXStreamObject::VbaXStreamObject( const ObjectBase& rParent,
        const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName, VbaFormSharedData& rFormData ) :
    mrFormData( rFormData )
{
    InputObjectBase::construct( rParent, rxStrm,  rSysFileName );
}

void VbaXStreamObject::implDump()
{
    for( size_t nIdx = 0, nCount = mrFormData.maSiteInfos.size(); !mxStrm->isEof() && (nIdx < nCount); ++nIdx )
    {
        mxOut->emptyLine();
        writeEmptyItem( "page" );
        IndentGuard aIndGuard( mxOut );
        VbaPageObject( *this ).dump();
    }
    if( !mxStrm->isEof() )
    {
        mxOut->emptyLine();
        writeEmptyItem( "multi-page" );
        IndentGuard aIndGuard( mxOut );
        VbaMultiPageObject( *this ).dump();
    }
    dumpRemainingStream();
}

VbaContainerStorageObject::VbaContainerStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath ) :
    OleStorageObject( rParent, rxStrg, rSysPath )
{
    addPreferredStream( "f" );
}

void VbaContainerStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if ( rStrmName == "f" )
        VbaFStreamObject( *this, rxStrm, rSysFileName, maFormData ).dump();
    else if ( rStrmName == "o" )
        VbaOStreamObject( *this, rxStrm, rSysFileName, maFormData ).dump();
    else if ( rStrmName == "x" )
        VbaXStreamObject( *this, rxStrm, rSysFileName, maFormData ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

void VbaContainerStorageObject::implDumpStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    if( isFormStorage( rStrgPath ) )
        VbaContainerStorageObject( *this, rxStrg, rSysPath ).dump();
    else
        OleStorageObject( *this, rxStrg, rSysPath ).dump();
}

bool VbaContainerStorageObject::isFormStorage( const OUString& rStrgPath ) const
{
    if( (rStrgPath.getLength() >= 3) && (rStrgPath[ 0 ] == 'i') )
    {
        OUString aId = rStrgPath.copy( 1 );
        if( (aId.getLength() == 2) && (aId[ 0 ] == '0') )
            aId = aId.copy( 1 );
        sal_Int32 nId = aId.toInt32();
        if( (nId > 0) && (OUString::number( nId ) == aId) )
            for (auto const& siteInfo : maFormData.maSiteInfos)
                if( siteInfo.mnId == nId )
                    return true;
    }
    return false;
}

VbaSharedData::VbaSharedData() :
    meTextEnc( RTL_TEXTENCODING_MS_1252 )
{
}

bool VbaSharedData::isModuleStream( const OUString& rStrmName ) const
{
    return maStrmOffsets.count( rStrmName ) > 0;
}

sal_Int32 VbaSharedData::getStreamOffset( const OUString& rStrmName ) const
{
    StreamOffsetMap::const_iterator aIt = maStrmOffsets.find( rStrmName );
    return (aIt == maStrmOffsets.end()) ? 0 : aIt->second;
}

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
            mrVbaData.meTextEnc = rtl_getTextEncodingFromWindowsCodePage( dumpDec< sal_uInt16 >( "codepage", "CODEPAGES" ) );
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
            maCurrStream.clear();
            mnCurrOffset = 0;
        break;
        case 0x001A:
            maCurrStream = dumpByteString( "stream-name" );
        break;
        case 0x001C:
            dumpByteString( "description" );
        break;
        case 0x002B:
            if( !maCurrStream.isEmpty() )
                mrVbaData.maStrmOffsets[ maCurrStream ] = mnCurrOffset;
            maCurrStream.clear();
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
            dumpByteString( "helpfile-path" );
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
    return dumpCharArray( rName, mxStrm->readInt32(), mrVbaData.meTextEnc );
}

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
    mxOut->emptyLine();
    writeEmptyItem( "source-code" );
    IndentGuard aIndGuard( mxOut );
    BinaryInputStreamRef xVbaStrm( new ::oox::ole::VbaInputStream( *mxStrm ) );
    TextLineStreamObject( *this, xVbaStrm, mrVbaData.meTextEnc ).dump();
}

VbaStorageObject::VbaStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath, VbaSharedData& rVbaData ) :
    OleStorageObject( rParent, rxStrg, rSysPath ),
    mrVbaData( rVbaData )
{
    addPreferredStream( "dir" );
}

void VbaStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( rStrgPath.isEmpty() && rStrmName == "dir" )
        VbaDirStreamObject( *this, rxStrm, rSysFileName, mrVbaData ).dump();
    else if( mrVbaData.isModuleStream( rStrmName ) )
        VbaModuleStreamObject( *this, rxStrm, rSysFileName, mrVbaData, mrVbaData.getStreamOffset( rStrmName ) ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

VbaFormStorageObject::VbaFormStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath, VbaSharedData& rVbaData ) :
    VbaContainerStorageObject( rParent, rxStrg, rSysPath ),
    mrVbaData( rVbaData )
{
}

void VbaFormStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if ( rStrmName == "\003VBFrame" )
        TextLineStreamObject( *this, rxStrm, mrVbaData.meTextEnc, rSysFileName ).dump();
    else
        VbaContainerStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

VbaProjectStorageObject::VbaProjectStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath ) :
    OleStorageObject( rParent, rxStrg, rSysPath )
{
    addPreferredStorage( "VBA" );
}

void VbaProjectStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( rStrgPath.isEmpty() && rStrmName == "PROJECT" )
        TextLineStreamObject( *this, rxStrm, maVbaData.meTextEnc, rSysFileName ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

void VbaProjectStorageObject::implDumpStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    if ( rStrgPath == "VBA" )
        VbaStorageObject( *this, rxStrg, rSysPath, maVbaData ).dump();
    else
        VbaFormStorageObject( *this, rxStrg, rSysPath, maVbaData ).dump();
}

ActiveXStorageObject::ActiveXStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath ) :
    VbaContainerStorageObject( rParent, rxStrg, rSysPath )
{
}

void ActiveXStorageObject::implDumpBaseStream( const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    FormControlStreamObject( *this, rxStrm, rSysFileName ).dump();
}

} // namespace dump
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
