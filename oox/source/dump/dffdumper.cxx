/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dffdumper.cxx,v $
 * $Revision: 1.3.22.10 $
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
const sal_uInt16 DFF_ID_SP                  = 0xF00A;   /// Shape.
const sal_uInt16 DFF_ID_SPGR                = 0xF009;   /// Shape group.
const sal_uInt16 DFF_ID_SPLITMENUCOLORS     = 0xF11E;   /// Current toolbar colors.

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
    MultiItemsGuard aMultiGuard( out() );
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
            out().resetItemIndex( 1 );
            TableGuard aTabGuard( out(), 15, 16 );
            for( sal_uInt32 nCluster = 1; !in().isEof() && (nCluster < nClusters); ++nCluster )
            {
                MultiItemsGuard aMultiGuard( out() );
                writeEmptyItem( "#cluster" );
                dumpDec< sal_uInt32 >( "drawing-id" );
                dumpHex< sal_uInt32 >( "next-free-id", "CONV-DEC" );
            }
        }
        break;

        case DFF_ID_OPT:
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
            dumpDffColor( "fill-color" );
            dumpDffColor( "line-color" );
            dumpDffColor( "shadow-color" );
            dumpDffColor( "3d-color" );
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

sal_uInt32 DffStreamObject::dumpDffColor( const String& rName )
{
    return dumpHex< sal_uInt32 >( rName, "DFF-COLOR" );
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

