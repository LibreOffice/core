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
        case 0xF001:    pcListName = "DFFBSTORECONT-RECORD-INST";   break;  // DFFBSTORECONTAINER contains BLIP count
        case 0xF007:    pcListName = "DFFBSE-RECORD-INST";          break;  // DFFBSE contains BLIP type
        case 0xF00A:    pcListName = "DFFSP-RECORD-INST";           break;  // DFFSP contains shape type
        case 0xF00B:    pcListName = "DFFOPT-RECORD-INST";          break;  // DFFOPT contains property count
    }
    MultiItemsGuard aMultiGuard( out() );
    writeHexItem( "instance", mnInstVer, pcListName );
    if( isContainer() ) writeDecItem( "container-size", mnRealSize );
}

void DffStreamObject::implDumpRecordBody()
{
    switch( getRecId() )
    {
        case 0xF007:    // DFFBSE
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

        case 0xF00A:    // DFFSP
            dumpHex< sal_uInt32 >( "shape-id", "CONV-DEC" );
            dumpHex< sal_uInt32 >( "shape-flags", "DFFSP-FLAGS" );
        break;

        case 0xF00B:    // DFFOPT
        {
            sal_uInt16 nPropCount = getInst();
            out().resetItemIndex();
            for( sal_uInt16 nPropIdx = 0; !in().isEof() && (nPropIdx < nPropCount); ++nPropIdx )
            {
                sal_uInt16 nPropId = dumpDffOptPropHeader();
                IndentGuard aIndent( out() );
                dumpDffOptPropValue( nPropId, in().readuInt32() );
            }
        }
        break;

        case 0xF010:    // DFFCLIENTANCHOR
            implDumpClientAnchor();
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
}

sal_uInt16 DffStreamObject::dumpDffOptPropHeader()
{
    MultiItemsGuard aMultiGuard( out() );
    TableGuard aTabGuard( out(), 11 );
    writeEmptyItem( "#prop" );
    return dumpHex< sal_uInt16 >( "id", "DFFOPT-PROPERTY-ID" );
}

void DffStreamObject::dumpDffOptPropValue( sal_uInt16 nPropId, sal_uInt32 nValue )
{
    switch( nPropId & 0x3FFF )
    {
        case 127:   writeHexItem( "flags", nValue, "DFFOPT-LOCK-FLAGS" );       break;
        case 191:   writeHexItem( "flags", nValue, "DFFOPT-TEXT-FLAGS" );       break;
        case 255:   writeHexItem( "flags", nValue, "DFFOPT-TEXTGEO-FLAGS" );    break;
        case 319:   writeHexItem( "flags", nValue, "DFFOPT-PICTURE-FLAGS" );    break;
        case 383:   writeHexItem( "flags", nValue, "DFFOPT-GEO-FLAGS" );        break;
        case 447:   writeHexItem( "flags", nValue, "DFFOPT-FILL-FLAGS" );       break;
        case 511:   writeHexItem( "flags", nValue, "DFFOPT-LINE-FLAGS" );       break;
        case 575:   writeHexItem( "flags", nValue, "DFFOPT-SHADOW-FLAGS" );     break;
        case 639:   writeHexItem( "flags", nValue, "DFFOPT-PERSP-FLAGS" );      break;
        case 703:   writeHexItem( "flags", nValue, "DFFOPT-3DOBJ-FLAGS" );      break;
        case 767:   writeHexItem( "flags", nValue, "DFFOPT-3DSTYLE-FLAGS" );    break;
        case 831:   writeHexItem( "flags", nValue, "DFFOPT-SHAPE1-FLAGS" );     break;
        case 895:   writeHexItem( "flags", nValue, "DFFOPT-CALLOUT-FLAGS" );    break;
        case 959:   writeHexItem( "flags", nValue, "DFFOPT-SHAPE2-FLAGS" );     break;
        default:    writeHexItem( "value", nValue );
    }
}

// ============================================================================

} // namespace dump
} // namespace oox

#endif

