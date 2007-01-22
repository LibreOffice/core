/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fdumperdff.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 13:18:44 $
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

#include "precompiled_sc.hxx"

#ifndef SC_FDUMPERDFF_HXX
#include "fdumperdff.hxx"
#endif

#if SCF_INCL_DUMPER

#ifndef _MSDFFDEF_HXX
#include <svx/msdffdef.hxx>
#endif

namespace scf {
namespace dump {

// ============================================================================
// ============================================================================

DffRecordHeaderObject::DffRecordHeaderObject( const InputObjectBase& rParent )
{
    static const RecordHeaderConfigInfo saHeaderCfgInfo =
    {
        "DFF-RECORD-NAMES",
        "show-dff-record-pos",
        "show-dff-record-size",
        "show-dff-record-id",
        "show-dff-record-name",
        "show-dff-record-body",
    };
    RecordHeaderBase::Construct( rParent, saHeaderCfgInfo );
    if( RecordHeaderBase::ImplIsValid() )
    {
        mxRecInst = Cfg().GetNameList( "DFF-RECORD-INST" );
        mnRecSize = 0;
        mnRecId = 0xFFFF;
        mnVer = 0;
        mnInst = 0;
        mnBodyStart = 0;
        mnBodyEnd = 0;
    }
}

bool DffRecordHeaderObject::ImplIsValid() const
{
    return IsValid( mxRecInst ) && RecordHeaderBase::ImplIsValid();
}

void DffRecordHeaderObject::ImplDumpBody()
{
    // read record header
    sal_uInt16 nInstVer;
    In() >> nInstVer >> mnRecId >> mnRecSize;

    mnBodyStart = In().Tell();
    mnBodyEnd = ::std::min( static_cast< sal_Size >( mnBodyStart + mnRecSize ), In().GetSize() );
    mnVer = nInstVer & 0x000F;
    mnInst = (nInstVer & 0xFFF0) >> 4;

    // dump record header
    Out().EmptyLine();
    {
        MultiItemsGuard aMultiGuard( Out() );
        WriteEmptyItem( "DFFREC" );
        if( IsShowRecPos() )  WriteHexItem( "pos", static_cast< sal_uInt32 >( mnBodyStart - DFF_COMMON_RECORD_HEADER_SIZE ) );
        if( IsShowRecSize() ) WriteHexItem( "size", mnRecSize );
        if( IsShowRecId() )   WriteHexItem( "id", mnRecId );
        if( IsShowRecName() ) WriteNameItem( "name", mnRecId, GetRecNames() );
    }
    WriteHexItem( "instance", nInstVer, mxRecInst );
}

// ============================================================================
// ============================================================================

DffDumpObject::DffDumpObject( const InputObjectBase& rParent ) :
    InputObjectBase( rParent )
{
    ConstructOwn();
}

DffDumpObject::~DffDumpObject()
{
}

void DffDumpObject::DumpDffClientPos( const sal_Char* pcName, sal_Int32 nSubScale )
{
    MultiItemsGuard aMultiGuard( Out() );
    TableGuard aTabGuard( Out(), 17 );
    DumpDec< sal_uInt16 >( pcName );
    ItemGuard aItem( Out(), "sub-units" );
    sal_uInt16 nSubUnits;
    In() >> nSubUnits;
    Out().WriteDec( nSubUnits );
    Out().WriteChar( '/' );
    Out().WriteDec( nSubScale );
}

void DffDumpObject::DumpDffClientRect()
{
    DumpDffClientPos( "start-col", 1024 );
    DumpDffClientPos( "start-row", 256 );
    DumpDffClientPos( "end-col", 1024 );
    DumpDffClientPos( "end-row", 256 );
}

bool DffDumpObject::ImplIsValid() const
{
    return IsValid( mxHdrObj ) && InputObjectBase::ImplIsValid();
}

void DffDumpObject::ImplDumpBody()
{
    while( In().IsValidPos() )
    {
        // record header
        mxHdrObj->Dump();
        // record contents
        if( mxHdrObj->GetVer() != DFF_PSFLAG_CONTAINER )
        {
            if( mxHdrObj->IsShowRecBody() )
                DumpRecordBody();
            In().Seek( mxHdrObj->GetBodyEnd() );
        }
    }
}

void DffDumpObject::ConstructOwn()
{
    if( InputObjectBase::ImplIsValid() )
        mxHdrObj.reset( new DffRecordHeaderObject( *this ) );
}

void DffDumpObject::DumpRecordBody()
{
    IndentGuard aIndGuard( Out() );

    // record contents
    if( mxHdrObj->HasRecName() ) switch( mxHdrObj->GetRecId() )
    {
        case DFF_msofbtOPT:
            DumpDffOptRec();
        break;
        case DFF_msofbtClientAnchor:
            DumpHex< sal_uInt16 >( "flags", "DFFCLIENTANCHOR-FLAGS" );
            DumpDffClientRect();
        break;
    }

    // remaining undumped data
    sal_Size nPos = In().Tell();
    if( nPos == mxHdrObj->GetBodyStart() )
        DumpRawBinary( mxHdrObj->GetRecSize(), false );
    else if( nPos < mxHdrObj->GetBodyEnd() )
        DumpRemaining( mxHdrObj->GetBodyEnd() - nPos );
}

void DffDumpObject::DumpDffOptRec()
{
    sal_uInt16 nInst = mxHdrObj->GetInst();
    sal_Size nBodyEnd = mxHdrObj->GetBodyEnd();
    Out().ResetItemIndex();
    for( sal_uInt16 nIdx = 0; (nIdx < nInst) && (In().Tell() < nBodyEnd); ++nIdx )
    {
        sal_uInt16 nPropId = DumpDffOptPropHeader();
        IndentGuard aIndent( Out() );
        DumpDffOptPropValue( nPropId, In().ReadValue< sal_uInt32 >() );
    }
}

sal_uInt16 DffDumpObject::DumpDffOptPropHeader()
{
    MultiItemsGuard aMultiGuard( Out() );
    TableGuard aTabGuard( Out(), 11 );
    WriteEmptyItem( "#prop" );
    return DumpHex< sal_uInt16 >( "id", "DFFOPT-PROPERTY-ID" );
}

void DffDumpObject::DumpDffOptPropValue( sal_uInt16 nPropId, sal_uInt32 nValue )
{
    switch( nPropId & 0x3FFF )
    {
        case DFF_Prop_LockAgainstGrouping:      WriteHexItem( "flags", nValue, "DFFOPT-LOCK-FLAGS" );       break;
        case DFF_Prop_FitTextToShape:           WriteHexItem( "flags", nValue, "DFFOPT-TEXT-FLAGS" );       break;
        case DFF_Prop_gtextFStrikethrough:      WriteHexItem( "flags", nValue, "DFFOPT-TEXTGEO-FLAGS" );    break;
        case DFF_Prop_pictureActive:            WriteHexItem( "flags", nValue, "DFFOPT-PICTURE-FLAGS" );    break;
        case DFF_Prop_fFillOK:                  WriteHexItem( "flags", nValue, "DFFOPT-GEO-FLAGS" );        break;
        case DFF_Prop_fNoFillHitTest:           WriteHexItem( "flags", nValue, "DFFOPT-FILL-FLAGS" );       break;
        case DFF_Prop_fNoLineDrawDash:          WriteHexItem( "flags", nValue, "DFFOPT-LINE-FLAGS" );       break;
        case DFF_Prop_fshadowObscured:          WriteHexItem( "flags", nValue, "DFFOPT-SHADOW-FLAGS" );     break;
        case DFF_Prop_fPerspective:             WriteHexItem( "flags", nValue, "DFFOPT-PERSP-FLAGS" );      break;
        case DFF_Prop_fc3DLightFace:            WriteHexItem( "flags", nValue, "DFFOPT-3DOBJ-FLAGS" );      break;
        case DFF_Prop_fc3DFillHarsh:            WriteHexItem( "flags", nValue, "DFFOPT-3DSTYLE-FLAGS" );    break;
        case DFF_Prop_fBackground:              WriteHexItem( "flags", nValue, "DFFOPT-SHAPE1-FLAGS" );     break;
        case DFF_Prop_fCalloutLengthSpecified:  WriteHexItem( "flags", nValue, "DFFOPT-CALLOUT-FLAGS" );    break;
        case DFF_Prop_fPrint:                   WriteHexItem( "flags", nValue, "DFFOPT-SHAPE2-FLAGS" );     break;

        default:
            WriteHexItem( "value", nValue );
    }
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif

