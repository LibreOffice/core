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

#include "oox/dump/dffdumper.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;

namespace oox {
namespace dump {

// ============================================================================

namespace {

const sal_uInt16 DFF_ID_BSE                 = 0xF007;   /// BLIP store entry.
const sal_uInt16 DFF_ID_BSTORECONTAINER     = 0xF001;   /// BLIP store container.
const sal_uInt16 DFF_ID_CHILDANCHOR         = 0xF00F;   /// Child anchor (in groups).
const sal_uInt16 DFF_ID_CLIENTANCHOR        = 0xF010;   /// Client anchor.
const sal_uInt16 DFF_ID_DG                  = 0xF008;   /// Drawing.
const sal_uInt16 DFF_ID_DGG                 = 0xF006;   /// Drawing group.
const sal_uInt16 DFF_ID_OPT                 = 0xF00B;   /// Property set.
const sal_uInt16 DFF_ID_OPT2                = 0xF121;   /// Secondary property set.
const sal_uInt16 DFF_ID_OPT3                = 0xF122;   /// Ternary property set.
const sal_uInt16 DFF_ID_SP                  = 0xF00A;   /// Shape.
const sal_uInt16 DFF_ID_SPGR                = 0xF009;   /// Shape group.
const sal_uInt16 DFF_ID_SPLITMENUCOLORS     = 0xF11E;   /// Current toolbar colors.

const sal_uInt16 DFF_OPT_IDMASK             = 0x3FFF;
const sal_uInt16 DFF_OPT_PICTURE            = 0x4000;
const sal_uInt16 DFF_OPT_COMPLEX            = 0x8000;
const sal_uInt16 DFF_OPT_FLAGSMASK          = 0x003F;

} // namespace

// ============================================================================

void DffStreamObject::construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    SequenceRecordObjectBase::construct( rParent, rxStrm, rSysFileName, "DFF-RECORD-NAMES" );
    constructDffObj();
}

void DffStreamObject::construct( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm )
{
    SequenceRecordObjectBase::construct( rParent, rxStrm, "DFF-RECORD-NAMES" );
    constructDffObj();
}

bool DffStreamObject::implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize )
{
    sal_uInt16 nRecId;
    rBaseStrm >> mnInstVer >> nRecId >> mnRealSize;
    ornRecId = nRecId;
    ornRecSize = isContainer() ? 0 : mnRealSize;
    return !rBaseStrm.isEof();
}

void DffStreamObject::implWriteExtHeader()
{
    const sal_Char* pcListName = "DFF-RECORD-INST";
    switch( getRecId() )
    {
        case DFF_ID_BSE:                pcListName = "DFFBSE-RECORD-INST";          break;  // BLIP type
        case DFF_ID_BSTORECONTAINER:    pcListName = "DFFBSTORECONT-RECORD-INST";   break;  // BLIP count
        case DFF_ID_DG:                 pcListName = "DFFDG-RECORD-INST";           break;  // drawing ID
        case DFF_ID_OPT:                pcListName = "DFFOPT-RECORD-INST";          break;  // property count
        case DFF_ID_SP:                 pcListName = "DFFSP-RECORD-INST";           break;  // shape type
        case DFF_ID_SPLITMENUCOLORS:    pcListName = "DFFSPLITMENUC-RECORD-INST";   break;  // number of colors
    }
    MultiItemsGuard aMultiGuard( mxOut );
    writeHexItem( "instance", mnInstVer, pcListName );
    if( isContainer() ) writeDecItem( "container-size", mnRealSize );
}

void DffStreamObject::implDumpRecordBody()
{
    switch( getRecId() )
    {
        case DFF_ID_BSE:
            dumpDec< sal_uInt8 >( "win-type", "DFFBSE-TYPE" );
            dumpDec< sal_uInt8 >( "mac-type", "DFFBSE-TYPE" );
            dumpGuid( "guid" );
            dumpDec< sal_uInt16 >( "tag" );
            dumpDec< sal_uInt32 >( "blip-size" );
            dumpDec< sal_uInt32 >( "blip-refcount" );
            dumpDec< sal_uInt32 >( "blip-streampos" );
            dumpDec< sal_uInt8 >( "blip-usage", "DFFBSE-USAGE" );
            dumpDec< sal_uInt8 >( "blip-name-len" );
            dumpUnused( 2 );
        break;

        case DFF_ID_CHILDANCHOR:
            dumpDec< sal_uInt32 >( "left" );
            dumpDec< sal_uInt32 >( "top" );
            dumpDec< sal_uInt32 >( "right" );
            dumpDec< sal_uInt32 >( "bottom" );
        break;

        case DFF_ID_CLIENTANCHOR:
            implDumpClientAnchor();
        break;

        case DFF_ID_DG:
            dumpDec< sal_uInt32 >( "shape-count" );
            dumpHex< sal_uInt32 >( "max-shape-id", "CONV-DEC" );
        break;

        case DFF_ID_DGG:
        {
            dumpHex< sal_uInt32 >( "max-shape-id", "CONV-DEC" );
            sal_uInt32 nClusters = dumpDec< sal_uInt32 >( "id-cluster-count" );
            dumpDec< sal_uInt32 >( "shape-count" );
            dumpDec< sal_uInt32 >( "drawing-count" );
            mxOut->resetItemIndex( 1 );
            TableGuard aTabGuard( mxOut, 15, 16 );
            for( sal_uInt32 nCluster = 1; !mxStrm->isEof() && (nCluster < nClusters); ++nCluster )
            {
                MultiItemsGuard aMultiGuard( mxOut );
                writeEmptyItem( "#cluster" );
                dumpDec< sal_uInt32 >( "drawing-id" );
                dumpHex< sal_uInt32 >( "next-free-id", "CONV-DEC" );
            }
        }
        break;

        case DFF_ID_OPT:
        case DFF_ID_OPT2:
        case DFF_ID_OPT3:
            dumpDffOpt();
        break;

        case DFF_ID_SP:
            dumpHex< sal_uInt32 >( "shape-id", "CONV-DEC" );
            dumpHex< sal_uInt32 >( "shape-flags", "DFFSP-FLAGS" );
        break;

        case DFF_ID_SPGR:
            dumpDec< sal_uInt32 >( "left" );
            dumpDec< sal_uInt32 >( "top" );
            dumpDec< sal_uInt32 >( "right" );
            dumpDec< sal_uInt32 >( "bottom" );
        break;

        case DFF_ID_SPLITMENUCOLORS:
            dumpDffSimpleColor( "fill-color" );
            dumpDffSimpleColor( "line-color" );
            dumpDffSimpleColor( "shadow-color" );
            dumpDffSimpleColor( "3d-color" );
        break;
    }
}

void DffStreamObject::implDumpClientAnchor()
{
}

void DffStreamObject::constructDffObj()
{
    mnInstVer = 0;
    mnRealSize = 0;
    if( SequenceRecordObjectBase::implIsValid() )
    {
        maSimpleProps.insertFormats( cfg().getNameList( "DFFOPT-SIMPLE-PROPERTIES" ) );
        maComplexProps.insertFormats( cfg().getNameList( "DFFOPT-COMPLEX-PROPERTIES" ) );
    }
}

sal_uInt32 DffStreamObject::dumpDffSimpleColor( const String& rName )
{
    return dumpHex< sal_uInt32 >( rName, "DFF-SIMPLE-COLOR" );
}

sal_uInt32 DffStreamObject::dumpDffColor( const String& rName )
{
    return dumpHex< sal_uInt32 >( rName, "DFF-COLOR" );
}

namespace {

enum PropType { PROPTYPE_BINARY, PROPTYPE_STRING, PROPTYPE_BLIP, PROPTYPE_COLORARRAY };

struct PropInfo
{
    OUString            maName;
    PropType            meType;
    sal_uInt16          mnId;
    sal_uInt32          mnSize;
    inline explicit     PropInfo( const OUString& rName, PropType eType, sal_uInt16 nId, sal_uInt32 nSize ) :
                            maName( rName ), meType( eType ), mnId( nId ), mnSize( nSize ) {}
};

typedef ::std::vector< PropInfo > PropInfoVector;

} // namespace

void DffStreamObject::dumpDffOpt()
{
    sal_uInt16 nPropCount = getInst();
    PropInfoVector aPropInfos;
    mxOut->resetItemIndex();
    for( sal_uInt16 nPropIdx = 0; !mxStrm->isEof() && (nPropIdx < nPropCount); ++nPropIdx )
    {
        sal_uInt16 nPropId = dumpDffOptPropHeader();
        sal_uInt16 nBaseId = nPropId & DFF_OPT_IDMASK;
        sal_uInt32 nValue = mxStrm->readuInt32();

        IndentGuard aIndent( mxOut );
        if( getFlag( nPropId, DFF_OPT_COMPLEX ) )
        {
            writeHexItem( "complex-size", nValue, "CONV-DEC" );
            String aName;
            PropType eType = PROPTYPE_BINARY;
            ItemFormatMap::const_iterator aIt = maComplexProps.find( nBaseId );
            if( aIt != maComplexProps.end() )
            {
                const ItemFormat& rItemFmt = aIt->second;
                aName = rItemFmt.maItemName;
                if( rItemFmt.maListName.equalsAscii( "binary" ) )
                    eType = PROPTYPE_BINARY;
                else if( rItemFmt.maListName.equalsAscii( "string" ) )
                    eType = PROPTYPE_STRING;
                else if( rItemFmt.maListName.equalsAscii( "blip" ) )
                    eType = PROPTYPE_BLIP;
                else if( rItemFmt.maListName.equalsAscii( "colorarray" ) )
                    eType = PROPTYPE_COLORARRAY;
            }
            aPropInfos.push_back( PropInfo( aName( "property-data" ), eType, nBaseId, nValue ) );
        }
        else
        {
            ItemFormatMap::const_iterator aIt = maSimpleProps.find( nBaseId );
            if( aIt != maSimpleProps.end() )
            {
                const ItemFormat& rItemFmt = aIt->second;
                // flags always at end of block of 64 properties
                if( (nBaseId & DFF_OPT_FLAGSMASK) == DFF_OPT_FLAGSMASK )
                {
                    FlagsList* pFlagsList = dynamic_cast< FlagsList* >( cfg().getNameList( rItemFmt.maListName ).get() );
                    sal_Int64 nOldIgnoreFlags = 0;
                    if( pFlagsList )
                    {
                        nOldIgnoreFlags = pFlagsList->getIgnoreFlags();
                        pFlagsList->setIgnoreFlags( nOldIgnoreFlags | 0xFFFF0000 | ~(nValue >> 16) );
                    }
                    writeValueItem( rItemFmt, nValue );
                    if( pFlagsList )
                        pFlagsList->setIgnoreFlags( nOldIgnoreFlags );
                }
                else
                    writeValueItem( rItemFmt, nValue );
            }
            else
                writeHexItem( "value", nValue );
        }
    }

    mxOut->resetItemIndex();
    for( PropInfoVector::iterator aIt = aPropInfos.begin(), aEnd = aPropInfos.end(); !mxStrm->isEof() && (aIt != aEnd); ++aIt )
    {
        mxOut->startMultiItems();
        writeEmptyItem( "#complex-data" );
        writeHexItem( "id", aIt->mnId, "DFFOPT-PROPERTY-NAMES" );
        mxOut->endMultiItems();
        IndentGuard aIndent( mxOut );
        switch( aIt->meType )
        {
            case PROPTYPE_BINARY:
                dumpBinary( aIt->maName, aIt->mnSize );
            break;
            case PROPTYPE_STRING:
                dumpUnicodeArray( aIt->maName, aIt->mnSize / 2, true );
            break;
            case PROPTYPE_BLIP:
                dumpBinary( aIt->maName, aIt->mnSize );
            break;
            case PROPTYPE_COLORARRAY:
                dumpBinary( aIt->maName, aIt->mnSize );
            break;
        }
    }
}

sal_uInt16 DffStreamObject::dumpDffOptPropHeader()
{
    MultiItemsGuard aMultiGuard( mxOut );
    TableGuard aTabGuard( mxOut, 11 );
    writeEmptyItem( "#prop" );
    return dumpHex< sal_uInt16 >( "id", "DFFOPT-PROPERTY-ID" );
}

// ============================================================================

} // namespace dump
} // namespace oox

#endif

