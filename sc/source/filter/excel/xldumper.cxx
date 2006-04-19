/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xldumper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-04-19 14:04:21 $
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

#ifndef SC_XLDUMPER_HXX
#include "xldumper.hxx"
#endif

#if SCF_INCL_DUMPER

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif
#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif
#ifndef SC_XLCHART_HXX
#include "xlchart.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

namespace scf {
namespace xls {
namespace dump {

using ::scf::dump::NameListRef;
using ::scf::dump::ConfigRef;
using ::scf::dump::Input;
using ::scf::dump::InputRef;
using ::scf::dump::Output;
using ::scf::dump::OutputRef;
using ::scf::dump::IndentGuard;
using ::scf::dump::TableGuard;
using ::scf::dump::MultiItemsGuard;
using ::scf::dump::BaseObject;
using ::scf::dump::InputObject;
using ::scf::dump::OleStorageObject;
using ::scf::dump::OleStreamObject;

// ============================================================================
// ============================================================================

namespace {

Output& operator<<( Output& rOut, const XclGuid& rGuid )
{
    rOut.WriteHex( SVBT32ToUInt32( rGuid.mpnData ), false );
    rOut.WriteChar( '-' );
    rOut.WriteHex( SVBT16ToShort( rGuid.mpnData + 4 ), false );
    rOut.WriteChar( '-' );
    rOut.WriteHex( SVBT16ToShort( rGuid.mpnData + 6 ), false );
    rOut.WriteChar( '-' );
    rOut.WriteHex( rGuid.mpnData[ 8 ], false );
    rOut.WriteHex( rGuid.mpnData[ 9 ], false );
    rOut.WriteChar( '-' );
    rOut.WriteHex( rGuid.mpnData[ 10 ], false );
    rOut.WriteHex( rGuid.mpnData[ 11 ], false );
    rOut.WriteHex( rGuid.mpnData[ 12 ], false );
    rOut.WriteHex( rGuid.mpnData[ 13 ], false );
    rOut.WriteHex( rGuid.mpnData[ 14 ], false );
    rOut.WriteHex( rGuid.mpnData[ 15 ], false );
    return rOut;
}

} // namespace

// ============================================================================
// ============================================================================

Config::Config() :
    ::scf::dump::Config( "XLSDUMPER", CREATE_STRING( "xldumper.dat" ) )
{
}

// ============================================================================
// ============================================================================

class CoreRootData
{
public:
    explicit            CoreRootData( SfxMedium& rMedium, SfxObjectShell* pDocShell );

    inline SfxMedium&   GetMedium() { return mrMedium; }
    inline SvStream*    GetInStream() { return mrMedium.GetInStream(); }
    inline SfxObjectShell* GetDocShell() { return mpDocShell; }

private:
    SfxMedium&          mrMedium;
    SfxObjectShell*     mpDocShell;
};

// ----------------------------------------------------------------------------

CoreRootData::CoreRootData( SfxMedium& rMedium, SfxObjectShell* pDocShell ) :
    mrMedium( rMedium ),
    mpDocShell( pDocShell )
{
}

// ============================================================================

class FilterRootData
{
public:
    explicit            FilterRootData( CoreRootData& rCoreRoot, XclBiff eBiff );

    inline const XclImpRoot& GetRoot() const { return maRoot; }

private:
    ScDocument          maDoc;
    XclImpRootData      maRootData;
    XclImpRoot          maRoot;
};

// ----------------------------------------------------------------------------

FilterRootData::FilterRootData( CoreRootData& rCoreRoot, XclBiff eBiff ) :
    maRootData( eBiff, rCoreRoot.GetMedium(), SotStorageRef(), maDoc, RTL_TEXTENCODING_MS_1252 ),
    maRoot( maRootData )
{
}

// ============================================================================
// ============================================================================

class StreamInput : public Input
{
public:
    inline explicit     StreamInput( XclImpStream& rStrm ) : mrStrm( rStrm ) {}
    virtual             ~StreamInput();

    virtual sal_Size    Size() const;
    virtual sal_Size    Tell() const;
    virtual void        Seek( sal_Size nPos );
    virtual void        SeekRel( sal_sSize nRelPos );
    virtual sal_Size    Read( void* pBuffer, sal_Size nSize );
    virtual void        ReadLine( String& rLine, rtl_TextEncoding eEnc );

    virtual StreamInput& operator>>( sal_Int8& rnData );
    virtual StreamInput& operator>>( sal_uInt8& rnData );
    virtual StreamInput& operator>>( sal_Int16& rnData );
    virtual StreamInput& operator>>( sal_uInt16& rnData );
    virtual StreamInput& operator>>( sal_Int32& rnData );
    virtual StreamInput& operator>>( sal_uInt32& rnData );
    virtual StreamInput& operator>>( float& rfData );
    virtual StreamInput& operator>>( double& rfData );

private:
    XclImpStream&       mrStrm;
};

// ----------------------------------------------------------------------------

StreamInput::~StreamInput()
{
}

sal_Size StreamInput::Size() const
{
    return static_cast< sal_Size >( mrStrm.GetRecSize() );
}

sal_Size StreamInput::Tell() const
{
    return static_cast< sal_Size >( mrStrm.GetRecPos() );
}

void StreamInput::Seek( sal_Size nPos )
{
    mrStrm.Seek( static_cast< sal_uInt32 >( nPos ) );
}

void StreamInput::SeekRel( sal_sSize nRelPos )
{
    mrStrm.Seek( static_cast< sal_uInt32 >( mrStrm.GetRecPos() + nRelPos ) );
}

sal_Size StreamInput::Read( void* pBuffer, sal_Size nSize )
{
    return static_cast< sal_Size >( mrStrm.Read( pBuffer, static_cast< sal_uInt32 >( nSize ) ) );
}

void StreamInput::ReadLine( String& rLine, rtl_TextEncoding eEnc )
{
    DBG_ERRORFILE( "scf::xls::dump::StreamInput::ReadLine - not implemented" );
}

StreamInput& StreamInput::operator>>( sal_Int8& rnData )   { mrStrm >> rnData; return *this; }
StreamInput& StreamInput::operator>>( sal_uInt8& rnData )  { mrStrm >> rnData; return *this; }
StreamInput& StreamInput::operator>>( sal_Int16& rnData )  { mrStrm >> rnData; return *this; }
StreamInput& StreamInput::operator>>( sal_uInt16& rnData ) { mrStrm >> rnData; return *this; }
StreamInput& StreamInput::operator>>( sal_Int32& rnData )  { mrStrm >> rnData; return *this; }
StreamInput& StreamInput::operator>>( sal_uInt32& rnData ) { mrStrm >> rnData; return *this; }
StreamInput& StreamInput::operator>>( float& rfData )      { mrStrm >> rfData; return *this; }
StreamInput& StreamInput::operator>>( double& rfData )     { mrStrm >> rfData; return *this; }

// ============================================================================
// ============================================================================

RecordStreamObject::RecordStreamObject( const BaseObject& rParent,
        CoreRootDataRef xCoreRoot, SvStream& rStrm )
{
    Construct( rParent, xCoreRoot, rStrm, CREATE_STRING( "Dumper" ) );
}

RecordStreamObject::RecordStreamObject( const OleStorageObject& rParentStrg,
        CoreRootDataRef xCoreRoot, const String& rStrmName )
{
    Construct( rParentStrg, xCoreRoot, rStrmName );
}

RecordStreamObject::RecordStreamObject()
{
}

RecordStreamObject::~RecordStreamObject()
{
}

void RecordStreamObject::Construct( const ::scf::dump::BaseObject& rParent,
        CoreRootDataRef xCoreRoot, SvStream& rStrm, const String& rProgressName )
{
    mxCoreRoot = xCoreRoot;
    maProgressName = rProgressName;
    XclBiff eBiff = XclImpStream::DetectBiffVersion( rStrm );
    if( eBiff != EXC_BIFF_UNKNOWN )
    {
        mxFilterRoot.reset( new FilterRootData( *mxCoreRoot, eBiff ) );
        mxStrm.reset( new XclImpStream( rStrm, mxFilterRoot->GetRoot() ) );
        InputRef xIn( new StreamInput( *mxStrm ) );
        InputObject::Construct( rParent, xIn );
    }
}

void RecordStreamObject::Construct( const OleStorageObject& rParentStrg,
        CoreRootDataRef xCoreRoot, const String& rStrmName )
{
    mxOleStrm.reset( new OleStreamObject( rParentStrg, rStrmName ) );
    if( mxOleStrm->IsValid() )
    {
        String aProgressName = CREATE_STRING( "Dumping stream '" );
        aProgressName.Append( mxOleStrm->GetFullPath() ).Append( '\'' );
        Construct( rParentStrg, xCoreRoot, mxOleStrm->GetStream(), aProgressName );
    }
}

bool RecordStreamObject::ImplIsValid() const
{
    return mxFilterRoot.is() && mxStrm.is() && InputObject::ImplIsValid();
}

void RecordStreamObject::ImplDumpHeader()
{
    if( mxOleStrm.is() )
        mxOleStrm->DumpHeader();
}

void RecordStreamObject::ImplDumpBody()
{
    bool bShowRecPos   = Cfg().GetBoolOption( "show-record-pos", true );
    bool bShowRecSize  = Cfg().GetBoolOption( "show-record-size", true );
    bool bShowRecId    = Cfg().GetBoolOption( "show-record-id", true );
    bool bShowRecName  = Cfg().GetBoolOption( "show-record-name", true );
    bool bShowRecBody  = Cfg().GetBoolOption( "show-record-body", true );
    bool bMergeContRec = Cfg().GetBoolOption( "merge-continue-record", true );

    NameListRef xRecNames = Cfg().GetNameList( "RECORDS" );

    ScfProgressBar aProgress( mxCoreRoot->GetDocShell(), maProgressName );
    sal_Int32 nStreamSeg = aProgress.AddSegment( mxStrm->GetSvStreamSize() );
    aProgress.ActivateSegment( nStreamSeg );

    while( mxStrm->StartNextRecord() )
    {
        // record header
        PrepareRecord( *mxStrm, bMergeContRec );
        {
            MultiItemsGuard aMultiGuard( *this );
            Out().WriteAscii( "REC" );
            if( bShowRecPos )
                WriteHexItem( "pos", static_cast< sal_uInt32 >( mxStrm->GetSvStreamPos() - 4 ) );
            if( bShowRecSize )
                WriteHexItem( "size", mxStrm->GetRecSize() );
            if( bShowRecId )
                WriteHexItem( "id", mxStrm->GetRecId() );
            if( bShowRecName )
                WriteNameItem( "name", xRecNames, mxStrm->GetRecId(), true );
        }

        // record contents
        if( bShowRecBody )
        {
            IndentGuard aIndGuard( Out() );
            DumpRecord( *mxStrm );
            // remaining undumped data
            DumpBinary( mxStrm->GetRecSize() - mxStrm->GetRecPos(), false );
        }
        if( !mxStrm->IsValid() )
            WriteInfoItem( "stream-state", SCF_DUMP_ERR_STREAM );

        Out().EmptyLine();

        aProgress.ProgressAbs( mxStrm->GetSvStreamPos() );
    }
}

void RecordStreamObject::ImplDumpFooter()
{
    if( mxOleStrm.is() )
        mxOleStrm->DumpFooter();
}

void RecordStreamObject::PrepareRecord( XclImpStream& rStrm, bool bMergeContRec )
{
}

void RecordStreamObject::DumpRecord( XclImpStream& rStrm )
{
}

// ============================================================================

WorkbookStreamObject::WorkbookStreamObject( const BaseObject& rParent,
        CoreRootDataRef xCoreRoot, SvStream& rStrm ) :
    RecordStreamObject( rParent, xCoreRoot, rStrm )
{
    InitMembers();
}

WorkbookStreamObject::WorkbookStreamObject( const OleStorageObject& rParentStrg,
        CoreRootDataRef xCoreRoot, const String& rStrmName ) :
    RecordStreamObject( rParentStrg, xCoreRoot, rStrmName )
{
    InitMembers();
}

void WorkbookStreamObject::PrepareRecord( XclImpStream& rStrm, bool bMergeContRec )
{
    if( bMergeContRec )
    {
        sal_uInt16 nRecId = rStrm.GetRecId();
        switch( nRecId )
        {
            case EXC_ID_OBJ:
            case EXC_ID_TXO:
            case EXC_ID_EOF:
            case EXC_ID_CONT:
                rStrm.ResetRecord( false );
            break;
            case EXC_ID_MSODRAWINGGROUP:
            case EXC_ID_CHESCHERFORMAT:
                rStrm.ResetRecord( true, nRecId );
            break;
            default:
                rStrm.ResetRecord( true );
        }
    }
    else
        rStrm.ResetRecord( false );
}

void WorkbookStreamObject::DumpRecord( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();

    XclBiff eBiff = rRoot.GetBiff();
    sal_uInt16 nRecId = rStrm.GetRecId();
    sal_uInt32 nRecSize = rStrm.GetRecSize();

    switch( nRecId )
    {
        case EXC_ID2_BOF:
        case EXC_ID3_BOF:
        case EXC_ID4_BOF:
        case EXC_ID5_BOF:
            DumpHex< sal_uInt16 >( "bof-type", mxBofBiffTypes );
            DumpHex< sal_uInt16 >( "sheet-type", mxBofSheetTypes );
            if( nRecSize >= 6 )  DumpDec< sal_uInt16 >( "build-id" );
            if( nRecSize >= 8 )  DumpDec< sal_uInt16 >( "build-year" );
            if( nRecSize >= 12 ) DumpHex< sal_uInt32 >( "history-flags", mxBofHistFlags );
            if( nRecSize >= 16 ) DumpDec< sal_uInt32 >( "lowest-ver" );
        break;
    }
}

void WorkbookStreamObject::InitMembers()
{
    if( IsValid() )
    {
        mxBofBiffTypes = Cfg().GetNameList( "BOF-BIFFTYPE" );
        mxBofSheetTypes = Cfg().GetNameList( "BOF-SHEETTYPE" );
        mxBofHistFlags = Cfg().GetNameList( "BOF-HISTORY" );
    }
}

// ============================================================================
// ============================================================================

RootStorageObject::RootStorageObject( const BaseObject& rParent, CoreRootDataRef xCoreRoot )
{
    Construct( rParent, xCoreRoot );
}

RootStorageObject::RootStorageObject()
{
}

RootStorageObject::~RootStorageObject()
{
}

void RootStorageObject::Construct( const ::scf::dump::BaseObject& rParent, CoreRootDataRef xCoreRoot )
{
    mxCoreRoot = xCoreRoot;
    if( SvStream* pRootStrm = mxCoreRoot->GetInStream() )
        OleStorageObject::Construct( rParent, *pRootStrm );
}

void RootStorageObject::ImplDumpBody()
{
    WorkbookStreamObject aWorkbookStrm( *this, mxCoreRoot, EXC_STREAM_WORKBOOK );
    aWorkbookStrm.Dump();
    WorkbookStreamObject aBookStrm( *this, mxCoreRoot, EXC_STREAM_BOOK );
    aBookStrm.Dump();
}

// ============================================================================

Dumper::Dumper( SfxMedium& rMedium, SfxObjectShell* pDocShell ) :
    mxCoreRoot( new CoreRootData( rMedium, pDocShell ) )
{
    String aOutName = mxCoreRoot->GetMedium().GetPhysicalName();
    aOutName.AppendAscii( ".txt" );
    BaseObject::Construct( ConfigRef( new Config ), aOutName );
}

Dumper::~Dumper()
{
}

void Dumper::ImplDumpBody()
{
    RootStorageObject aRootStrg( *this, mxCoreRoot );
    if( aRootStrg.IsValid() )
    {
        aRootStrg.Dump();
    }
    else if( SvStream* pStrm = mxCoreRoot->GetInStream() )
    {
        WorkbookStreamObject aRecStrm( *this, mxCoreRoot, *pStrm );
        if( aRecStrm.IsValid() )
            aRecStrm.Dump();
    }
}

// ============================================================================

} // namespace dump
} // namespace xls
} // namespace scf

#endif

