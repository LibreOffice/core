/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dffdumper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:58 $
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

#include "oox/dump/dffdumper.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {

// ============================================================================

DffRecordHeaderObject::DffRecordHeaderObject( const InputObjectBase& rParent )
{
    static const RecordHeaderConfigInfo saHeaderCfgInfo =
    {
        "DFFREC",
        "DFF-RECORD-NAMES",
        "show-dff-record-pos",
        "show-dff-record-size",
        "show-dff-record-id",
        "show-dff-record-name",
        "show-dff-record-body",
    };
    RecordHeaderBase< sal_uInt16, sal_uInt32 >::construct( rParent, saHeaderCfgInfo );
    if( RecordHeaderBase< sal_uInt16, sal_uInt32 >::implIsValid() )
    {
        mxRecInst = cfg().getNameList( "DFF-RECORD-INST" );
        mnBodyStart = mnBodyEnd = 0;
        mnInstVer = 0;
    }
}

bool DffRecordHeaderObject::implIsValid() const
{
    return isValid( mxRecInst ) && RecordHeaderBase< sal_uInt16, sal_uInt32 >::implIsValid();
}

bool DffRecordHeaderObject::implReadHeader( sal_Int64& ornRecPos, sal_uInt16& ornRecId, sal_uInt32& ornRecSize )
{
    ornRecPos = in().tell();
    if( ornRecPos >= in().getSize() ) return false;
    in() >> mnInstVer >> ornRecId >> ornRecSize;
    mnBodyStart = in().tell();
    mnBodyEnd = ::std::min< sal_Int64 >( mnBodyStart + ornRecSize, in().getSize() );
    return in().isValidPos();
}

void DffRecordHeaderObject::implWriteExtHeader()
{
    writeHexItem( "instance", mnInstVer, mxRecInst );
}

// ============================================================================

DffDumpObject::DffDumpObject( const InputObjectBase& rParent )
{
    InputObjectBase::construct( rParent );
    if( InputObjectBase::implIsValid() )
        mxHdrObj.reset( new DffRecordHeaderObject( *this ) );
}

DffDumpObject::~DffDumpObject()
{
}

void DffDumpObject::dumpDffClientPos( const sal_Char* pcName, sal_Int32 nSubScale )
{
    MultiItemsGuard aMultiGuard( out() );
    TableGuard aTabGuard( out(), 17 );
    dumpDec< sal_uInt16 >( pcName );
    ItemGuard aItem( out(), "sub-units" );
    sal_uInt16 nSubUnits;
    in() >> nSubUnits;
    out().writeDec( nSubUnits );
    out().writeChar( '/' );
    out().writeDec( nSubScale );
}

void DffDumpObject::dumpDffClientRect()
{
    dumpDffClientPos( "start-col", 1024 );
    dumpDffClientPos( "start-row", 256 );
    dumpDffClientPos( "end-col", 1024 );
    dumpDffClientPos( "end-row", 256 );
}

bool DffDumpObject::implIsValid() const
{
    return isValid( mxHdrObj ) && InputObjectBase::implIsValid();
}

void DffDumpObject::implDump()
{
    while( mxHdrObj->startNextRecord() )
    {
        if( mxHdrObj->getVer() != 0x0F )
        {
            if( mxHdrObj->isShowRecBody() )
                dumpRecordBody();
            in().seek( mxHdrObj->getBodyEnd() );
        }
        out().emptyLine();
    }
}

void DffDumpObject::dumpRecordBody()
{
    IndentGuard aIndGuard( out() );

    // record contents
    if( mxHdrObj->hasRecName() ) switch( mxHdrObj->getRecId() )
    {
        case 0xF00B:
            dumpDffOptRec();
        break;
        case 0xF010:
            dumpHex< sal_uInt16 >( "flags", "DFFCLIENTANCHOR-FLAGS" );
            dumpDffClientRect();
        break;
    }

    // remaining undumped data
    sal_Int64 nPos = in().tell();
    if( nPos == mxHdrObj->getBodyStart() )
        dumpRawBinary( mxHdrObj->getRecSize(), false );
    else if( nPos < mxHdrObj->getBodyEnd() )
        dumpRemaining( static_cast< sal_Int32 >( mxHdrObj->getBodyEnd() - nPos ) );
}

void DffDumpObject::dumpDffOptRec()
{
    sal_uInt16 nInst = mxHdrObj->getInst();
    sal_Int64 nBodyEnd = mxHdrObj->getBodyEnd();
    out().resetItemIndex();
    for( sal_uInt16 nIdx = 0; (nIdx < nInst) && (in().tell() < nBodyEnd); ++nIdx )
    {
        sal_uInt16 nPropId = dumpDffOptPropHeader();
        IndentGuard aIndent( out() );
        dumpDffOptPropValue( nPropId, in().readValue< sal_uInt32 >() );
    }
}

sal_uInt16 DffDumpObject::dumpDffOptPropHeader()
{
    MultiItemsGuard aMultiGuard( out() );
    TableGuard aTabGuard( out(), 11 );
    writeEmptyItem( "#prop" );
    return dumpHex< sal_uInt16 >( "id", "DFFOPT-PROPERTY-ID" );
}

void DffDumpObject::dumpDffOptPropValue( sal_uInt16 nPropId, sal_uInt32 nValue )
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

