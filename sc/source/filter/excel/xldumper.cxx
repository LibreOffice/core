/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xldumper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:17:15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
#ifndef SC_XLCHART_HXX
#include "xlchart.hxx"
#endif
#ifndef SC_XLCONTENT_HXX
#include "xlcontent.hxx"
#endif
#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif
#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif
#ifndef SC_XLTABLE_HXX
#include "xltable.hxx"
#endif
#ifndef SC_XISTRING_HXX
#include "xistring.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

namespace scf {
namespace dump {
namespace xls {

const sal_Unicode SCF_DUMP_BASECLASS    = 'B';
const sal_Unicode SCF_DUMP_FUNCSEP      = ',';
const sal_Unicode SCF_DUMP_LISTSEP      = ',';
const sal_Unicode SCF_DUMP_ADDRABS      = '$';
const sal_Unicode SCF_DUMP_RANGESEP     = ':';
const sal_Unicode SCF_DUMP_TABSEP       = '!';
const sal_Unicode SCF_DUMP_ARRAYSEP     = ';';
const sal_Unicode SCF_DUMP_EMPTYVALUE   = '~';
const sal_Unicode SCF_DUMP_CMDPROMPT    = '?';
const sal_Unicode SCF_DUMP_PLACEHOLDER  = '\x01';

// ============================================================================
// ============================================================================

void Address::Read( XclImpStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    mnRow = bRow32Bit ? rStrm.ReadInt32() : rStrm.ReaduInt16();
    mnCol = bCol16Bit ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
}

// ----------------------------------------------------------------------------

void Range::Read( XclImpStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    maFirst.mnRow = bRow32Bit ? rStrm.ReadInt32() : rStrm.ReaduInt16();
    maLast.mnRow = bRow32Bit ? rStrm.ReadInt32() : rStrm.ReaduInt16();
    maFirst.mnCol = bCol16Bit ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
    maLast.mnCol = bCol16Bit ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
}

// ----------------------------------------------------------------------------

void RangeList::Read( XclImpStream& rStrm, bool bCol16Bit, bool bRow32Bit )
{
    sal_uInt16 nCount;
    rStrm >> nCount;
    resize( nCount );
    for( iterator aIt = begin(), aEnd = end(); rStrm.IsValid() && (aIt != aEnd); ++aIt )
        aIt->Read( rStrm, bCol16Bit, bRow32Bit );
}

// ============================================================================

void StringHelper::AppendAddrCol( String& rStr, sal_Int32 nCol, bool bRel )
{
    if( !bRel ) rStr.Append( SCF_DUMP_ADDRABS );
    xub_StrLen nPos = rStr.Len();
    for( sal_Int32 nTemp = nCol; nTemp >= 0; (nTemp /= 26) -= 1 )
        rStr.Insert( static_cast< sal_Unicode >( 'A' + (nTemp % 26) ), nPos );
}

void StringHelper::AppendAddrRow( String& rStr, sal_Int32 nRow, bool bRel )
{
    if( !bRel ) rStr.Append( SCF_DUMP_ADDRABS );
    AppendDec( rStr, nRow + 1 );
}

void StringHelper::AppendAddrName( String& rStr, sal_Unicode cPrefix, sal_Int32 nColRow, bool bRel )
{
    rStr.Append( cPrefix );
    bool bWriteParen = bRel && (nColRow != 0);
    if( bWriteParen ) rStr.Append( '(' );
    if( !bRel || (nColRow != 0) ) AppendDec( rStr, nColRow );
    if( bWriteParen ) rStr.Append( ')' );
}

void StringHelper::AppendAddress( String& rStr, const Address& rPos )
{
    AppendAddrCol( rStr, rPos.mnCol, true );
    AppendAddrRow( rStr, rPos.mnRow, true );
}

void StringHelper::AppendRange( String& rStr, const Range& rRange )
{
    AppendAddress( rStr, rRange.maFirst );
    rStr.Append( SCF_DUMP_RANGESEP );
    AppendAddress( rStr, rRange.maLast );
}

void StringHelper::AppendRangeList( String& rStr, const RangeList& rRanges )
{
    String aData;
    for( RangeList::const_iterator aIt = rRanges.begin(), aEnd = rRanges.end(); aIt != aEnd; ++aIt )
    {
        String aRange;
        AppendRange( aRange, *aIt );
        AppendToken( aData, aRange, SCF_DUMP_LISTSEP );
    }
    rStr.Append( aData );
}

void StringHelper::AppendAddress( String& rStr, const FormulaAddress& rPos, bool bNameMode )
{
    if( bNameMode && (rPos.mbRelCol || rPos.mbRelRow) )
    {
        AppendAddrName( rStr, 'R', rPos.mnRow, rPos.mbRelRow );
        AppendAddrName( rStr, 'C', rPos.mnCol, rPos.mbRelCol );
    }
    else
    {
        AppendAddrCol( rStr, rPos.mnCol, rPos.mbRelCol );
        AppendAddrRow( rStr, rPos.mnRow, rPos.mbRelRow );
    }
}

void StringHelper::AppendRange( String& rStr, const FormulaRange& rRange, bool bNameMode )
{
    AppendAddress( rStr, rRange.maFirst, bNameMode );
    rStr.Append( SCF_DUMP_RANGESEP );
    AppendAddress( rStr, rRange.maLast, bNameMode );
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
    return mrStrm.GetRecSize();
}

sal_Size StreamInput::Tell() const
{
    return mrStrm.GetRecPos();
}

void StreamInput::Seek( sal_Size nPos )
{
    mrStrm.Seek( nPos );
}

void StreamInput::SeekRel( sal_sSize nRelPos )
{
    mrStrm.Seek( mrStrm.GetRecPos() + nRelPos );
}

sal_Size StreamInput::Read( void* pBuffer, sal_Size nSize )
{
    return mrStrm.Read( pBuffer, nSize );
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

BiffConfig::BiffConfig( const Config& rParent, XclBiff eBiff ) :
    meBiff( eBiff )
{
    Config::Construct( rParent );
}

bool BiffConfig::ImplIsValid() const
{
    return (meBiff != EXC_BIFF_UNKNOWN) && Config::ImplIsValid();
}

NameListRef BiffConfig::ImplGetNameList( const String& rKey ) const
{
    NameListRef xList = Config::ImplGetNameList( rKey );
    if( !xList )
    {
        String aBaseKey( rKey );
        aBaseKey.AppendAscii( "-BIFF" );
        switch( meBiff )
        {
            // fall-through intended!
            case EXC_BIFF8: if( !xList ) xList = Config::ImplGetNameList( String( aBaseKey ).Append( '8' ) );
            case EXC_BIFF5: if( !xList ) xList = Config::ImplGetNameList( String( aBaseKey ).Append( '5' ) );
            case EXC_BIFF4: if( !xList ) xList = Config::ImplGetNameList( String( aBaseKey ).Append( '4' ) );
            case EXC_BIFF3: if( !xList ) xList = Config::ImplGetNameList( String( aBaseKey ).Append( '3' ) );
            case EXC_BIFF2: if( !xList ) xList = Config::ImplGetNameList( String( aBaseKey ).Append( '2' ) );
            break;
            default: DBG_ERROR_BIFF();
        }
    }
    return xList;
}

// ============================================================================

RootData::RootData( SfxMedium& rMedium, XclBiff eBiff )
{
    if( eBiff != EXC_BIFF_UNKNOWN )
    {
        mxDoc.reset( new ScDocument );
        mxRootData.reset( new XclImpRootData(
            eBiff, rMedium, SotStorageRef(), *mxDoc, ScfTools::GetSystemCharSet() ) );
        mxRoot.reset( new XclImpRoot( *mxRootData ) );
    }
}

RootData::~RootData()
{
}

rtl_TextEncoding RootData::GetTextEncoding() const
{
    return mxRoot->GetCharSet();
}

void RootData::SetTextEncoding( rtl_TextEncoding eTextEnc )
{
    mxRoot->SetCharSet( eTextEnc );
}

bool RootData::ImplIsValid() const
{
    return mxRoot.is();
}

// ============================================================================

RootObjectBase::RootObjectBase()
{
}

RootObjectBase::~RootObjectBase()
{
}

void RootObjectBase::Construct( const ObjectBase& rParent, SvStream& rStrm )
{
    StreamObjectRef xStrmObj( new SvStreamObject( rParent, rStrm ) );
    WrappedStreamObject::Construct( rParent, xStrmObj );
    ConstructOwn();
}

void RootObjectBase::Construct( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    StreamObjectRef xStrmObj( new OleStreamObject( rParentStrg, rStrmName ) );
    WrappedStreamObject::Construct( rParentStrg, xStrmObj );
    ConstructOwn();
}

void RootObjectBase::Construct( const RootObjectBase& rParent )
{
    *this = rParent;
}

bool RootObjectBase::ImplIsValid() const
{
    return IsValid( mxRootData ) && IsValid( mxBiffCfg ) && mxStrm.is() && WrappedStreamObject::ImplIsValid();
}

void RootObjectBase::ImplDumpExtendedHeader()
{
    WrappedStreamObject::ImplDumpExtendedHeader();
    WriteNameItem( "biff-version", static_cast< sal_uInt32 >( meBiff ), "BIFF" );
}

ConfigRef RootObjectBase::ImplReconstructConfig()
{
    mxBiffCfg.reset( new BiffConfig( Cfg(), meBiff ) );
    return mxBiffCfg;
}

InputRef RootObjectBase::ImplReconstructInput()
{
    InputRef xNewIn;
    if( mxStrm.is() )
        xNewIn.reset( new StreamInput( *mxStrm ) );
    return xNewIn;
}

String RootObjectBase::GetErrorName( sal_uInt8 nErrCode ) const
{
    return Cfg().GetName( mxErrCodes, nErrCode );
}

// ----------------------------------------------------------------------------

double RootObjectBase::WriteRkItem( const sal_Char* pcName, sal_Int32 nRk )
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteHexItem( pcName, static_cast< sal_uInt32 >( nRk ), "RK-FLAGS" );
    double fValue = XclTools::GetDoubleFromRK( nRk );
    WriteDecItem( "decoded", fValue );
    return fValue;
}

void RootObjectBase::WriteBooleanItem( const sal_Char* pcName, sal_uInt8 nBool )
{
    WriteDecItem( pcName, nBool, mxBoolean );
}

void RootObjectBase::WriteErrorCodeItem( const sal_Char* pcName, sal_uInt8 nErrCode )
{
    WriteHexItem( pcName, nErrCode, mxErrCodes );
}

void RootObjectBase::WriteAddressItem( const sal_Char* pcName, const Address& rPos )
{
    ItemGuard aItem( Out(), pcName );
    StringHelper::AppendAddress( Out().GetLine(), rPos );
}

void RootObjectBase::WriteRangeItem( const sal_Char* pcName, const Range& rRange )
{
    ItemGuard aItem( Out(), pcName );
    StringHelper::AppendRange( Out().GetLine(), rRange );
}

void RootObjectBase::WriteRangeListItem( const sal_Char* pcName, const RangeList& rRanges )
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteEmptyItem( pcName );
    WriteDecItem( "count", static_cast< sal_uInt16 >( rRanges.size() ) );
    ItemGuard aItem( Out(), "ranges" );
    StringHelper::AppendRangeList( Out().GetLine(), rRanges );
}

// ----------------------------------------------------------------------------

String RootObjectBase::DumpString( const sal_Char* pcName, XclStrFlags nFlags )
{
    XclImpString aString;
    aString.Read( GetXclStream(), nFlags );
    WriteStringItem( pcName ? pcName : "text", aString.GetText() );
    return aString.GetText();
}

double RootObjectBase::DumpRk( const sal_Char* pcName )
{
    sal_Int32 nRk;
    *mxStrm >> nRk;
    return WriteRkItem( pcName ? pcName : "rk-value", nRk );
}

sal_uInt8 RootObjectBase::DumpBoolean( const sal_Char* pcName )
{
    sal_uInt8 nBool;
    *mxStrm >> nBool;
    WriteBooleanItem( pcName ? pcName : "boolean", nBool );
    return nBool;
}

sal_uInt8 RootObjectBase::DumpErrorCode( const sal_Char* pcName )
{
    sal_uInt8 nErrCode;
    *mxStrm >> nErrCode;
    WriteErrorCodeItem( pcName ? pcName : "errorcode", nErrCode );
    return nErrCode;
}

Color RootObjectBase::DumpRgbColor( const sal_Char* pcName )
{
    Color aColor( mxStrm->ReaduInt32() );
    WriteColorItem( pcName ? pcName : "color-rgb", aColor );
    return aColor;
}

rtl_TextEncoding RootObjectBase::DumpCodePage( const sal_Char* pcName )
{
    sal_uInt16 nCodePage = DumpDec< sal_uInt16 >( pcName ? pcName : "codepage", "CODEPAGES" );
    return XclTools::GetTextEncoding( nCodePage );
}

void RootObjectBase::DumpFormulaResult( const sal_Char* pcName )
{
    MultiItemsGuard aMultiGuard( Out() );
    sal_uInt8 pnResult[ 8 ];
    mxStrm->Read( pnResult, 8 );
    WriteArrayItem( pcName ? pcName : "result", pnResult, 8 );
    if( (pnResult[ 6 ] == 0xFF) && (pnResult[ 7 ] == 0xFF) )
    {
        sal_uInt8 nType = pnResult[ 0 ];
        sal_uInt8 nData = pnResult[ 2 ];
        WriteHexItem( "type", nType, mxResultType );
        switch( nType )
        {
            case EXC_FORMULA_RES_BOOL:  WriteBooleanItem( "value", nData );     break;
            case EXC_FORMULA_RES_ERROR: WriteErrorCodeItem( "value", nData );   break;
        }
    }
    else
        WriteDecItem( "value", SVBT64ToDouble( pnResult ) );
}

Address RootObjectBase::DumpAddress( const sal_Char* pcName, bool bCol16Bit, bool bRow32Bit )
{
    Address aPos;
    aPos.Read( *mxStrm, bCol16Bit, bRow32Bit );
    WriteAddressItem( pcName ? pcName : "addr", aPos );
    return aPos;
}

Range RootObjectBase::DumpRange( const sal_Char* pcName, bool bCol16Bit, bool bRow32Bit )
{
    Range aRange;
    aRange.Read( *mxStrm, bCol16Bit, bRow32Bit );
    WriteRangeItem( pcName ? pcName : "range", aRange );
    return aRange;
}

void RootObjectBase::DumpRangeList( const sal_Char* pcName, bool bCol16Bit, bool bRow32Bit )
{
    RangeList aRanges;
    aRanges.Read( *mxStrm, bCol16Bit, bRow32Bit );
    WriteRangeListItem( pcName ? pcName : "range-list", aRanges );
}

void RootObjectBase::DumpConstArrayHeader( sal_uInt32& rnCols, sal_uInt32& rnRows )
{
    MultiItemsGuard aMultiGuard( Out() );
    rnCols = DumpDec< sal_uInt8 >( "width" );
    rnRows = DumpDec< sal_uInt16 >( "height" );
    switch( GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            if( rnCols == 0 )
                rnCols = 256;
        break;
        case EXC_BIFF8:
            ++rnCols;
            ++rnRows;
        break;
        default:    DBG_ERROR_BIFF();
    }
    ItemGuard aItem( Out(), "size" );
    Out().WriteDec( rnCols );
    Out().WriteChar( 'x' );
    Out().WriteDec( rnRows );
    aItem.Cont();
    Out().WriteDec( rnCols * rnRows );
}

String RootObjectBase::DumpConstValue()
{
    MultiItemsGuard aMultiGuard( Out() );
    String aValue;
    switch( DumpDec< sal_uInt8 >( "type", mxConstType ) )
    {
        case EXC_CACHEDVAL_EMPTY:
            DumpUnused( 8 );
            aValue.Assign( SCF_DUMP_EMPTYVALUE );
        break;
        case EXC_CACHEDVAL_DOUBLE:
            DumpDec< double >( "value" );
            aValue = Out().GetLastItemValue();
        break;
        case EXC_CACHEDVAL_STRING:
        {
            XclStrFlags nFlags = (GetBiff() <= EXC_BIFF5) ? EXC_STR_8BITLENGTH : EXC_STR_DEFAULT;
            aValue = DumpString( "value", nFlags );
            StringHelper::Enclose( aValue, '"' );
        }
        break;
        case EXC_CACHEDVAL_BOOL:
            DumpBoolean( "value" );
            aValue = Out().GetLastItemValue();
            DumpUnused( 7 );
        break;
        case EXC_CACHEDVAL_ERROR:
            DumpErrorCode( "value" );
            aValue = Out().GetLastItemValue();
            DumpUnused( 7 );
        break;
    }
    return aValue;
}

// ----------------------------------------------------------------------------

void RootObjectBase::ConstructOwn()
{
    if( WrappedStreamObject::ImplIsValid() )
    {
        SvStream& rStrm = GetStream();
        meBiff = XclImpStream::DetectBiffVersion( rStrm );
        mxRootData.reset( new RootData( GetCoreMedium(), meBiff ) );
        if( IsValid( mxRootData ) )
            mxStrm.reset( new XclImpStream( rStrm, mxRootData->GetRoot() ) );
        ReconstructConfig();
        ReconstructInput();
    }
    if( RootObjectBase::ImplIsValid() )
    {
        const Config& rCfg = Cfg();
        mxBoolean       = rCfg.GetNameList( "BOOLEAN" );
        mxErrCodes      = rCfg.GetNameList( "ERRORCODES" );
        mxConstType     = rCfg.GetNameList( "CONSTVALUE-TYPE" );
        mxResultType    = rCfg.GetNameList( "FORMULA-RESULTTYPE" );
    }
}

// ============================================================================

SimpleObjectBase::~SimpleObjectBase()
{
}

void SimpleObjectBase::Construct( const RootObjectBase& rParent )
{
    RootObjectBase::Construct( rParent );
}

void SimpleObjectBase::ImplDumpHeader()
{
}

void SimpleObjectBase::ImplDumpFooter()
{
}

// ============================================================================
// ============================================================================

FormulaStack::FormulaStack() :
    mbError( false )
{
}

void FormulaStack::PushOperand( const StringWrapper& rOp, const String& rTokClass )
{
    maFmlaStack.push( rOp.GetString() );
    maClassStack.push( rTokClass );
}

void FormulaStack::PushOperand( const StringWrapper& rOp )
{
    PushOperand( rOp, String( SCF_DUMP_BASECLASS ) );
}

void FormulaStack::PushUnaryOp( const StringWrapper& rLOp, const StringWrapper& rROp )
{
    PushUnaryOp( maFmlaStack, rLOp.GetString(), rROp.GetString() );
    PushUnaryOp( maClassStack, rLOp.GetString(), rROp.GetString() );
}

void FormulaStack::PushBinaryOp( const StringWrapper& rOp )
{
    PushBinaryOp( maFmlaStack, rOp.GetString() );
    PushBinaryOp( maClassStack, rOp.GetString() );
}

void FormulaStack::PushFuncOp( const StringWrapper& rFunc, const String& rTokClass, sal_uInt8 nParamCount )
{
    PushFuncOp( maFmlaStack, rFunc.GetString(), nParamCount );
    PushFuncOp( maClassStack, rTokClass, nParamCount );
}

void FormulaStack::ReplaceOnTop( const String& rOld, const String& rNew )
{
    if( !maFmlaStack.empty() )
        maFmlaStack.top().SearchAndReplace( rOld, rNew );
}

const String& FormulaStack::GetString( const StringStack& rStack ) const
{
    static const String saStackError = SCF_DUMP_ERRSTRING( "stack" );
    return (mbError || rStack.empty()) ? saStackError : rStack.top();
}

void FormulaStack::PushUnaryOp( StringStack& rStack, const String& rLOp, const String& rROp )
{
    if( Check( !rStack.empty() ) )
        rStack.top().Insert( rLOp, 0 ).Append( rROp );
}

void FormulaStack::PushBinaryOp( StringStack& rStack, const String& rOp )
{
    String aSecond;
    if( Check( !rStack.empty() ) )
    {
        aSecond = rStack.top();
        rStack.pop();
    }
    if( Check( !rStack.empty() ) )
        rStack.top().Append( rOp ).Append( aSecond );
}

void FormulaStack::PushFuncOp( StringStack& rStack, const String& rOp, sal_uInt8 nParamCount )
{
    String aFunc;
    for( sal_uInt8 nParam = 0; (nParam < nParamCount) && Check( !rStack.empty() ); ++nParam )
    {
        StringHelper::PrependToken( aFunc, rStack.top(), SCF_DUMP_FUNCSEP );
        rStack.pop();
    }
    StringHelper::Enclose( aFunc, '(', ')' );
    aFunc.Insert( rOp, 0 );
    rStack.push( aFunc );
}

// ============================================================================

FormulaObject::FormulaObject( const RootObjectBase& rParent ) :
    mpcName( 0 ),
    mnSize( 0 )
{
    RootObjectBase::Construct( rParent );
    ConstructOwn();
}

FormulaObject::~FormulaObject()
{
}

sal_uInt16 FormulaObject::ReadFormulaSize()
{
    return (GetBiff() == EXC_BIFF2) ? GetXclStream().ReaduInt8() : GetXclStream().ReaduInt16();
}

sal_uInt16 FormulaObject::DumpFormulaSize( const sal_Char* pcName )
{
    if( !pcName ) pcName = "formula-size";
    sal_uInt16 nSize = ReadFormulaSize();
    WriteDecItem( pcName, nSize );
    return nSize;
}

void FormulaObject::DumpFormula( const sal_Char* pcName, sal_uInt16 nSize, bool bNameMode )
{
    mpcName = pcName ? pcName : "formula";
    mnSize = nSize;
    mbNameMode = bNameMode;
    Dump();
}

void FormulaObject::DumpFormula( const sal_Char* pcName, bool bNameMode )
{
    DumpFormula( pcName, ReadFormulaSize(), bNameMode );
}

void FormulaObject::ImplDumpHeader()
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteEmptyItem( mpcName );
    WriteDecItem( "formula-size", mnSize );
}

void FormulaObject::ImplDumpBody()
{
    if( mnSize == 0 ) return;

    bool bValid = mxTokens.is();
    mxStack.reset( new FormulaStack );
    maAddData.clear();
    IndentGuard aIndGuard( Out() );
    TableGuard aTabGuard( Out(), 8, 18 );

    Input& rIn = In();
    sal_Size nStartPos = rIn.Tell();
    sal_Size nEndPos = ::std::min( nStartPos + mnSize, rIn.Size() );
    while( bValid && (rIn.Tell() < nEndPos) )
    {
        MultiItemsGuard aMultiGuard( Out() );
        WriteHexItem( 0, static_cast< sal_uInt16 >( rIn.Tell() - nStartPos ) );
        sal_uInt8 nTokenId = DumpHex< sal_uInt8 >( 0, mxTokens );
        bValid = mxTokens->HasName( nTokenId );
        if( bValid )
        {
            sal_uInt8 nTokClass = XclTokenArrayHelper::GetTokenClass( nTokenId );
            sal_uInt8 nBaseId = XclTokenArrayHelper::GetBaseTokenId( nTokenId );
            if( nTokClass == EXC_TOKCLASS_NONE )
            {
                switch( nBaseId )
                {
                    case EXC_TOKID_EXP:         DumpExpToken( "EXP" );          break;
                    case EXC_TOKID_TBL:         DumpExpToken( "TBL" );          break;
                    case EXC_TOKID_ADD:         DumpBinaryOpToken( "+" );       break;
                    case EXC_TOKID_SUB:         DumpBinaryOpToken( "-" );       break;
                    case EXC_TOKID_MUL:         DumpBinaryOpToken( "*" );       break;
                    case EXC_TOKID_DIV:         DumpBinaryOpToken( "/" );       break;
                    case EXC_TOKID_POWER:       DumpBinaryOpToken( "^" );       break;
                    case EXC_TOKID_CONCAT:      DumpBinaryOpToken( "&" );       break;
                    case EXC_TOKID_LT:          DumpBinaryOpToken( "<" );       break;
                    case EXC_TOKID_LE:          DumpBinaryOpToken( "<=" );      break;
                    case EXC_TOKID_EQ:          DumpBinaryOpToken( "=" );       break;
                    case EXC_TOKID_GE:          DumpBinaryOpToken( ">=" );      break;
                    case EXC_TOKID_GT:          DumpBinaryOpToken( "<" );       break;
                    case EXC_TOKID_NE:          DumpBinaryOpToken( "<>" );      break;
                    case EXC_TOKID_ISECT:       DumpBinaryOpToken( " " );       break;
                    case EXC_TOKID_LIST:        DumpBinaryOpToken( "," );       break;
                    case EXC_TOKID_RANGE:       DumpBinaryOpToken( ":" );       break;
                    case EXC_TOKID_UPLUS:       DumpUnaryOpToken( "+", "" );    break;
                    case EXC_TOKID_UMINUS:      DumpUnaryOpToken( "-", "" );    break;
                    case EXC_TOKID_PERCENT:     DumpUnaryOpToken( "", "%" );    break;
                    case EXC_TOKID_PAREN:       DumpUnaryOpToken( "(", ")" );   break;
                    case EXC_TOKID_MISSARG:     DumpMissArgToken();             break;
                    case EXC_TOKID_STR:         DumpStringToken();              break;
                    case EXC_TOKID_NLR:         bValid = DumpNlrToken();        break;
                    case EXC_TOKID_ATTR:        bValid = DumpAttrToken();       break;
                    case EXC_TOKID_SHEET:       DumpSheetToken();               break;
                    case EXC_TOKID_ENDSHEET:    DumpEndSheetToken();            break;
                    case EXC_TOKID_ERR:         DumpErrorToken();               break;
                    case EXC_TOKID_BOOL:        DumpBoolToken();                break;
                    case EXC_TOKID_INT:         DumpIntToken();                 break;
                    case EXC_TOKID_NUM:         DumpDoubleToken();              break;
                }
            }
            else
            {
                String aTokClass = Cfg().GetName( mxClasses, nTokClass );
                switch( nBaseId )
                {
                    case EXC_TOKID_ARRAY:       DumpArrayToken( aTokClass );                break;
                    case EXC_TOKID_FUNC:        DumpFuncToken( aTokClass );                 break;
                    case EXC_TOKID_FUNCVAR:     DumpFuncVarToken( aTokClass );              break;
                    case EXC_TOKID_NAME:        DumpNameToken( aTokClass );                 break;
                    case EXC_TOKID_REF:         DumpRefToken( aTokClass, false );           break;
                    case EXC_TOKID_AREA:        DumpAreaToken( aTokClass, false );          break;
                    case EXC_TOKID_MEMAREA:     DumpMemAreaToken( aTokClass, true );        break;
                    case EXC_TOKID_MEMERR:      DumpMemAreaToken( aTokClass, false );       break;
                    case EXC_TOKID_MEMNOMEM:    DumpMemAreaToken( aTokClass, false );       break;
                    case EXC_TOKID_MEMFUNC:     DumpMemFuncToken( aTokClass );              break;
                    case EXC_TOKID_REFERR:      DumpRefErrToken( aTokClass, false );        break;
                    case EXC_TOKID_AREAERR:     DumpRefErrToken( aTokClass, true );         break;
                    case EXC_TOKID_REFN:        DumpRefToken( aTokClass, true );            break;
                    case EXC_TOKID_AREAN:       DumpAreaToken( aTokClass, true );           break;
                    case EXC_TOKID_MEMAREAN:    DumpMemFuncToken( aTokClass );              break;
                    case EXC_TOKID_MEMNOMEMN:   DumpMemFuncToken( aTokClass );              break;
                    case EXC_TOKID_FUNCCE:      DumpCmdToken( aTokClass );                  break;
                    case EXC_TOKID_NAMEX:       DumpNameXToken( aTokClass );                break;
                    case EXC_TOKID_REF3D:       DumpRef3dToken( aTokClass, mbNameMode );    break;
                    case EXC_TOKID_AREA3D:      DumpArea3dToken( aTokClass, mbNameMode );   break;
                    case EXC_TOKID_REFERR3D:    DumpRefErr3dToken( aTokClass, false );      break;
                    case EXC_TOKID_AREAERR3D:   DumpRefErr3dToken( aTokClass, true );       break;
                }
            }
        }
    }
    bValid = nEndPos == rIn.Tell();
    if( bValid )
    {
        DumpAddTokenData();
        WriteInfoItem( "formula", mxStack->GetFormulaString() );
        WriteInfoItem( "classes", mxStack->GetClassesString() );
    }
    else
        DumpBinary( SCF_DUMP_ERRASCII( "formula-error" ), nEndPos - rIn.Tell(), false );

    mpcName = 0;
    mnSize = 0;
}

// private --------------------------------------------------------------------

void FormulaObject::ConstructOwn()
{
    if( RootObjectBase::ImplIsValid() )
    {
        mxFuncProv.reset( new XclFunctionProvider( GetXclRoot() ) );

        Config& rCfg = Cfg();
        mxClasses   = rCfg.GetNameList( "TOKENCLASSES" );
        mxFuncNames = rCfg.GetNameList( "FUNCTIONNAMES" );
        mxParamCnt  = rCfg.GetNameList( "PARAMCOUNT" );
        mxRelFlags  = rCfg.GetNameList( "REFRELFLAGS" );
        mxNlrTypes  = rCfg.GetNameList( "NLRTYPES" );
        mxAttrTypes = rCfg.GetNameList( "ATTRTYPES" );
        mxSpTypes   = rCfg.GetNameList( "ATTRSPACETYPES" );

        // create classified token names
        mxTokens = rCfg.CreateNameList< ConstList >( "TOKENS" );
        mxTokens->IncludeList( rCfg.GetNameList( "BASETOKENS" ) );

        NameListRef xClassTokens = rCfg.GetNameList( "CLASSTOKENS" );
        if( mxClasses.is() && xClassTokens.is() )
            for( NameListBase::const_iterator aCIt = mxClasses->begin(), aCEnd = mxClasses->end(); aCIt != aCEnd; ++aCIt )
                for( NameListBase::const_iterator aTIt = xClassTokens->begin(), aTEnd = xClassTokens->end(); aTIt != aTEnd; ++aTIt )
                    mxTokens->SetName( aCIt->first | aTIt->first, String( aTIt->second ).Append( aCIt->second ) );
    }
}

// ----------------------------------------------------------------------------

namespace {

String lclCreateName( const String& rRef, sal_uInt16 nNameIdx )
{
    String aName = rRef;
    StringHelper::AppendIndexedText( aName, CREATE_STRING( "NAME" ), nNameIdx );
    return aName;
}

String lclCreateNlr( const String& rData, bool bRel = true )
{
    String aNlr;
    if( !bRel ) aNlr.Append( SCF_DUMP_ADDRABS );
    StringHelper::AppendIndexedText( aNlr, CREATE_STRING( "NLR" ), rData );
    return aNlr;
}

String lclCreateNlr( const FormulaAddress& rPos )
{
    String aAddr;
    StringHelper::AppendAddrCol( aAddr, rPos.mnCol, true );
    StringHelper::AppendAddrRow( aAddr, rPos.mnRow, true );
    return lclCreateNlr( aAddr, rPos.mbRelRow );
}

} // namespace

// ----------------------------------------------------------------------------

FormulaAddress FormulaObject::CreateTokenAddress( sal_uInt16 nCol, sal_uInt16 nRow, bool bRelC, bool bRelR, bool bNameMode ) const
{
    FormulaAddress aPos;
    aPos.mnCol = nCol;
    if( bRelC && bNameMode ) aPos.mnCol -= 0x80;
    aPos.mbRelCol = bRelC;
    aPos.mnRow = nRow;
    if( bRelR && bNameMode ) aPos.mnRow -= ((GetBiff() <= EXC_BIFF5) ? 0x2000 : 0x8000);
    aPos.mbRelRow = bRelR;
    return aPos;
}

String FormulaObject::CreateFunc( sal_uInt16 nFuncIdx ) const
{
    String aFunc;
    if( Cfg().HasName( mxFuncNames, nFuncIdx ) )
        aFunc = Cfg().GetName( mxFuncNames, nFuncIdx );
    else
    {
        bool bCmd = ::get_flag( nFuncIdx, EXC_TOK_FUNCVAR_CMD );
        aFunc = String::CreateFromAscii( bCmd ? "CMD" : "FUNC" );
        StringHelper::AppendIndex( aFunc, nFuncIdx & EXC_TOK_FUNCVAR_INDEXMASK );
    }
    return aFunc;
}

String FormulaObject::CreateRef( const String& rData ) const
{
    return String( maRefPrefix ).Append( rData );
}

String FormulaObject::CreateName( sal_uInt16 nNameIdx ) const
{
    return lclCreateName( maRefPrefix, nNameIdx );
}

String FormulaObject::CreatePlaceHolder( size_t nIdx ) const
{
    String aStr;
    StringHelper::AppendDec( aStr, static_cast< sal_uInt32 >( nIdx ) );
    StringHelper::Enclose( aStr, SCF_DUMP_PLACEHOLDER );
    return aStr;
}

String FormulaObject::CreatePlaceHolder() const
{
    return CreatePlaceHolder( maAddData.size() );
}

void FormulaObject::WriteTokenAddressItem( const sal_Char* pcName, const FormulaAddress& rPos, bool bNameMode )
{
    ItemGuard aItem( Out(), pcName );
    StringHelper::AppendAddress( Out().GetLine(), rPos, bNameMode );
}

void FormulaObject::WriteTokenAddress3dItem( const sal_Char* pcName, const String& rRef, const FormulaAddress& rPos, bool bNameMode )
{
    ItemGuard aItem( Out(), pcName );
    Out().WriteString( rRef );
    StringHelper::AppendAddress( Out().GetLine(), rPos, bNameMode );
}

void FormulaObject::WriteTokenRangeItem( const sal_Char* pcName, const FormulaRange& rRange, bool bNameMode )
{
    ItemGuard aItem( Out(), pcName );
    StringHelper::AppendRange( Out().GetLine(), rRange, bNameMode );
}

void FormulaObject::WriteTokenRange3dItem( const sal_Char* pcName, const String& rRef, const FormulaRange& rRange, bool bNameMode )
{
    ItemGuard aItem( Out(), pcName );
    Out().WriteString( rRef );
    StringHelper::AppendRange( Out().GetLine(), rRange, bNameMode );
}

sal_uInt16 FormulaObject::DumpTokenFuncIdx()
{
    return DumpDec< sal_uInt16, sal_uInt8 >( GetBiff() >= EXC_BIFF4, "func-idx", mxFuncNames );
}

sal_uInt16 FormulaObject::DumpTokenCol( const sal_Char* pcName, bool& rbRelC, bool& rbRelR )
{
    sal_uInt16 nCol = 0;
    if( GetBiff() <= EXC_BIFF5 )
        nCol = DumpDec< sal_uInt8 >( pcName );
    else
    {
        nCol = DumpHex< sal_uInt16 >( pcName, mxRelFlags );
        rbRelC = ::get_flag( nCol, EXC_TOK_REF_COLREL );
        rbRelR = ::get_flag( nCol, EXC_TOK_REF_ROWREL );
        nCol &= 0x00FF;
    }
    return nCol;
}

sal_uInt16 FormulaObject::DumpTokenRow( const sal_Char* pcName, bool& rbRelC, bool& rbRelR )
{
    sal_uInt16 nRow = 0;
    if( GetBiff() <= EXC_BIFF5 )
    {
        nRow = DumpHex< sal_uInt16 >( pcName, mxRelFlags );
        rbRelC = ::get_flag( nRow, EXC_TOK_REF_COLREL );
        rbRelR = ::get_flag( nRow, EXC_TOK_REF_ROWREL );
        nRow &= 0x3FFF;
    }
    else
        nRow = DumpDec< sal_uInt16 >( pcName );
    return nRow;
}

FormulaAddress FormulaObject::DumpTokenAddress( bool bNameMode )
{
    bool bRelC = false;
    bool bRelR = false;
    sal_uInt16 nRow = DumpTokenRow( "row", bRelC, bRelR );
    sal_uInt16 nCol = DumpTokenCol( "col", bRelC, bRelR );
    return CreateTokenAddress( nCol, nRow, bRelC, bRelR, bNameMode );
}

FormulaRange FormulaObject::DumpTokenRange( bool bNameMode )
{
    bool bRelC1 = false;
    bool bRelR1 = false;
    bool bRelC2 = false;
    bool bRelR2 = false;
    sal_uInt16 nRow1 = DumpTokenRow( "row1", bRelC1, bRelR1 );
    sal_uInt16 nRow2 = DumpTokenRow( "row2", bRelC2, bRelR2 );
    sal_uInt16 nCol1 = DumpTokenCol( "col1", bRelC1, bRelR1 );
    sal_uInt16 nCol2 = DumpTokenCol( "col2", bRelC2, bRelR2 );
    FormulaRange aRange;
    aRange.maFirst = CreateTokenAddress( nCol1, nRow1, bRelC1, bRelR1, bNameMode );
    aRange.maLast  = CreateTokenAddress( nCol2, nRow2, bRelC2, bRelR2, bNameMode );
    return aRange;
}

String FormulaObject::DumpTokenRefIdx()
{
    sal_Int16 nRefIdx = DumpDec< sal_Int16 >( "ref-idx" );
    switch( GetBiff() )
    {
        case EXC_BIFF2: DumpUnused( 1 );    break;
        case EXC_BIFF3: DumpUnused( 2 );    break;
        case EXC_BIFF4: DumpUnused( 2 );    break;
        case EXC_BIFF5: DumpUnused( 8 );    break;
        case EXC_BIFF8:                     break;
        default:        DBG_ERROR_BIFF();
    }
    String aRef = CREATE_STRING( "REF" );
    StringHelper::AppendIndex( aRef, nRefIdx );
    aRef.Append( SCF_DUMP_TABSEP );
    return aRef;
}

void FormulaObject::DumpTokenRefTabIdxs()
{
    if( GetBiff() <= EXC_BIFF5 )
    {
        DumpDec< sal_Int16 >( "tab1" );
        DumpDec< sal_Int16 >( "tab2" );
    }
}

void FormulaObject::DumpIntToken()
{
    sal_uInt16 nValue = DumpDec< sal_uInt16 >( "value" );
    mxStack->PushOperand( Out().GetLastItemValue() );
}

void FormulaObject::DumpDoubleToken()
{
    double fValue = DumpDec< double >( "value" );
    mxStack->PushOperand( Out().GetLastItemValue() );
}

void FormulaObject::DumpStringToken()
{
    String aValue = DumpString( "value", EXC_STR_8BITLENGTH );
    StringHelper::Enclose( aValue, '"' );
    mxStack->PushOperand( aValue );
}

void FormulaObject::DumpBoolToken()
{
    DumpBoolean( "value" );
    mxStack->PushOperand( Out().GetLastItemValue() );
}

void FormulaObject::DumpErrorToken()
{
    DumpErrorCode( "value" );
    mxStack->PushOperand( Out().GetLastItemValue() );
}

void FormulaObject::DumpMissArgToken()
{
    mxStack->PushOperand( String( SCF_DUMP_EMPTYVALUE ) );
}

void FormulaObject::DumpArrayToken( const String& rTokClass )
{
    DumpUnused( (GetBiff() == EXC_BIFF2) ? 6 : 7 );
    mxStack->PushOperand( CreatePlaceHolder(), rTokClass );
    maAddData.push_back( ADDDATA_ARRAY );
}

void FormulaObject::DumpNameToken( const String& rTokClass )
{
    sal_uInt16 nNameIdx = DumpDec< sal_uInt16 >( "name-idx" );
    switch( GetBiff() )
    {
        case EXC_BIFF2: DumpUnused( 5 );    break;
        case EXC_BIFF3:
        case EXC_BIFF4: DumpUnused( 8 );    break;
        case EXC_BIFF5: DumpUnused( 12 );   break;
        case EXC_BIFF8: DumpUnused( 2 );    break;
        default:        DBG_ERROR_BIFF();
    }
    mxStack->PushOperand( CreateName( nNameIdx ), rTokClass );
}

void FormulaObject::DumpNameXToken( const String& rTokClass )
{
    String aRef = DumpTokenRefIdx();
    sal_uInt16 nNameIdx = DumpDec< sal_uInt16 >( "name-idx" );
    DumpUnused( (GetBiff() <= EXC_BIFF5) ? 12 : 2 );
    mxStack->PushOperand( lclCreateName( aRef, nNameIdx ), rTokClass );
}

void FormulaObject::DumpRefToken( const String& rTokClass, bool bNameMode )
{
    FormulaAddress aPos = DumpTokenAddress( bNameMode );
    WriteTokenAddressItem( "addr", aPos, bNameMode );
    mxStack->PushOperand( CreateRef( Out().GetLastItemValue() ), rTokClass );
}

void FormulaObject::DumpAreaToken( const String& rTokClass, bool bNameMode )
{
    FormulaRange aRange = DumpTokenRange( bNameMode );
    WriteTokenRangeItem( "range", aRange, bNameMode );
    mxStack->PushOperand( CreateRef( Out().GetLastItemValue() ), rTokClass );
}

void FormulaObject::DumpRefErrToken( const String& rTokClass, bool bArea )
{
    DumpUnused( ((GetBiff() <= EXC_BIFF5) ? 3 : 4) * (bArea ? 2 : 1) );
    mxStack->PushOperand( CreateRef( GetErrorName( EXC_ERR_REF ) ), rTokClass );
}

void FormulaObject::DumpRef3dToken( const String& rTokClass, bool bNameMode )
{
    String aRef = DumpTokenRefIdx();
    DumpTokenRefTabIdxs();
    FormulaAddress aPos = DumpTokenAddress( bNameMode );
    WriteTokenAddress3dItem( "addr", aRef, aPos, bNameMode );
    mxStack->PushOperand( Out().GetLastItemValue(), rTokClass );
}

void FormulaObject::DumpArea3dToken( const String& rTokClass, bool bNameMode )
{
    String aRef = DumpTokenRefIdx();
    DumpTokenRefTabIdxs();
    FormulaRange aRange = DumpTokenRange( bNameMode );
    WriteTokenRange3dItem( "range", aRef, aRange, bNameMode );
    mxStack->PushOperand( Out().GetLastItemValue(), rTokClass );
}

void FormulaObject::DumpRefErr3dToken( const String& rTokClass, bool bArea )
{
    String aRef = DumpTokenRefIdx();
    DumpTokenRefTabIdxs();
    DumpUnused( ((GetBiff() <= EXC_BIFF5) ? 3 : 4) * (bArea ? 2 : 1) );
    aRef.Append( GetErrorName( EXC_ERR_REF ) );
    mxStack->PushOperand( aRef, rTokClass );
}

void FormulaObject::DumpMemFuncToken( const String& rTokClass )
{
    DumpDec< sal_uInt16, sal_uInt8 >( GetBiff() >= EXC_BIFF3, "size" );
}

void FormulaObject::DumpMemAreaToken( const String& rTokClass, bool bAddData )
{
    DumpUnused( (GetBiff() == EXC_BIFF2) ? 3 : 4 );
    DumpMemFuncToken( rTokClass );
    if( bAddData )
        maAddData.push_back( ADDDATA_MEMAREA );
}

void FormulaObject::DumpExpToken( const StringWrapper& rName )
{
    Address aPos;
    aPos.mnRow = DumpDec< sal_uInt16 >( "row" );
    aPos.mnCol = DumpDec< sal_uInt16, sal_uInt8 >( GetBiff() >= EXC_BIFF3, "col" );
    WriteAddressItem( "base-addr", aPos );
    String aOp = rName.GetString();
    StringHelper::AppendIndex( aOp, Out().GetLastItemValue() );
    mxStack->PushOperand( aOp );
}

void FormulaObject::DumpUnaryOpToken( const StringWrapper& rLOp, const StringWrapper& rROp )
{
    mxStack->PushUnaryOp( rLOp, rROp );
}

void FormulaObject::DumpBinaryOpToken( const StringWrapper& rOp )
{
    mxStack->PushBinaryOp( rOp );
}

void FormulaObject::DumpFuncToken( const String& rTokClass )
{
    sal_uInt16 nFuncIdx = DumpTokenFuncIdx();
    String aFuncName = CreateFunc( nFuncIdx );
    if( const XclFunctionInfo* pFuncInfo = mxFuncProv->GetFuncInfoFromXclFunc( nFuncIdx ) )
        mxStack->PushFuncOp( aFuncName, rTokClass, pFuncInfo->mnMinParamCount );
    else
        mxStack->SetError();
}

void FormulaObject::DumpFuncVarToken( const String& rTokClass )
{
    sal_uInt8 nParamCount = DumpDec< sal_uInt8 >( "param-count", mxParamCnt );
    bool bPrompt = ::get_flag( nParamCount, EXC_TOK_FUNCVAR_PROMPT );
    nParamCount &= EXC_TOK_FUNCVAR_COUNTMASK;
    sal_uInt16 nFuncIdx = DumpTokenFuncIdx();
    String aFuncName = CreateFunc( nFuncIdx );
    if( bPrompt ) aFuncName.Append( SCF_DUMP_CMDPROMPT );
    mxStack->PushFuncOp( aFuncName, rTokClass, nParamCount );
}

void FormulaObject::DumpCmdToken( const String& rTokClass )
{
    sal_uInt8 nParamCount = DumpDec< sal_uInt8 >( "param-count" );
    sal_uInt16 nCmdIdx = DumpTokenFuncIdx() | EXC_TOK_FUNCVAR_CMD;
    mxStack->PushFuncOp( CreateFunc( nCmdIdx ), rTokClass, nParamCount );
}

void FormulaObject::DumpSheetToken()
{
    DumpUnused( (GetBiff() == EXC_BIFF2) ? 4 : 6 );
    maRefPrefix = DumpTokenRefIdx();
}

void FormulaObject::DumpEndSheetToken()
{
    DumpUnused( (GetBiff() == EXC_BIFF2) ? 3 : 4 );
    maRefPrefix.Erase();
}

bool FormulaObject::DumpAttrToken()
{
    bool bValid = true;
    bool bBiff3 = GetBiff() >= EXC_BIFF3;
    sal_uInt8 nType = DumpHex< sal_uInt8 >( "type", mxAttrTypes );
    switch( nType )
    {
        case EXC_TOK_ATTR_VOLATILE:
            DumpUnused( bBiff3 ? 2 : 1 );
        break;
        case EXC_TOK_ATTR_IF:
            DumpDec< sal_uInt16, sal_uInt8 >( bBiff3, "skip" );
        break;
        case EXC_TOK_ATTR_CHOOSE:
        {
            sal_uInt16 nCount = DumpDec< sal_uInt16, sal_uInt8 >( bBiff3, "choices" );
            Out().ResetItemIndex();
            for( sal_uInt16 nIdx = 0; nIdx < nCount; ++nIdx )
                DumpDec< sal_uInt16, sal_uInt8 >( bBiff3, "#skip" );
            DumpDec< sal_uInt16, sal_uInt8 >( bBiff3, "skip-err" );
        }
        break;
        case EXC_TOK_ATTR_GOTO:
            DumpDec< sal_uInt16, sal_uInt8 >( bBiff3, "skip" );
        break;
        case EXC_TOK_ATTR_SUM:
            DumpUnused( bBiff3 ? 2 : 1 );
            mxStack->PushFuncOp( CREATE_STRING( "SUM" ), String( SCF_DUMP_BASECLASS ), 1 );
        break;
        case EXC_TOK_ATTR_ASSIGN:
            DumpUnused( bBiff3 ? 2 : 1 );
        break;
        case EXC_TOK_ATTR_SPACE:
        case EXC_TOK_ATTR_SPACE | EXC_TOK_ATTR_VOLATILE:
            switch( GetBiff() )
            {
                case EXC_BIFF3:
                    DumpDec< sal_uInt16 >( "leading-spaces" );
                break;
                case EXC_BIFF4:
                case EXC_BIFF5:
                case EXC_BIFF8:
                    DumpDec< sal_uInt8 >( "char-type", mxSpTypes );
                    DumpDec< sal_uInt8 >( "char-count" );
                break;
                default:
                    DBG_ERROR_BIFF();
                    bValid = false;
            }
        break;
        default:
            bValid = false;
    }
    return bValid;
}

bool FormulaObject::DumpNlrToken()
{
    const String aRefClass = Cfg().GetName( mxClasses, EXC_TOKCLASS_REF );
    const String aValClass = Cfg().GetName( mxClasses, EXC_TOKCLASS_VAL );

    bool bValid = true;
    sal_uInt8 nType = DumpHex< sal_uInt8 >( "type", mxNlrTypes );
    switch( nType )
    {
        case EXC_TOK_NLR_ERR:       DumpNlrErrToken();                      break;
        case EXC_TOK_NLR_ROWR:      DumpNlrColRowToken( aRefClass, false ); break;
        case EXC_TOK_NLR_COLR:      DumpNlrColRowToken( aRefClass, false ); break;
        case EXC_TOK_NLR_ROWV:      DumpNlrColRowToken( aValClass, false ); break;
        case EXC_TOK_NLR_COLV:      DumpNlrColRowToken( aValClass, false ); break;
        case EXC_TOK_NLR_RANGE:     DumpNlrRangeToken( aRefClass, false );  break;
        case EXC_TOK_NLR_SRANGE:    DumpNlrRangeToken( aRefClass, true );   break;
        case EXC_TOK_NLR_SROWR:     DumpNlrColRowToken( aRefClass, true );  break;
        case EXC_TOK_NLR_SCOLR:     DumpNlrColRowToken( aRefClass, true );  break;
        case EXC_TOK_NLR_SROWV:     DumpNlrColRowToken( aValClass, true );  break;
        case EXC_TOK_NLR_SCOLV:     DumpNlrColRowToken( aValClass, true );  break;
        case EXC_TOK_NLR_RANGEERR:  DumpNlrRangeErrToken();                 break;
        default:                    bValid = false;
    }
    return bValid;
}

void FormulaObject::DumpNlrErrToken()
{
    DumpDec< sal_uInt32 >( "delname-idx" );
    mxStack->PushOperand( lclCreateNlr( GetErrorName( EXC_ERR_NAME ) ) );
}

void FormulaObject::DumpNlrColRowToken( const String& rTokClass, bool bAddData )
{
    if( bAddData )
    {
        DumpUnused( 4 );
        mxStack->PushOperand( CreatePlaceHolder(), rTokClass );
        maAddData.push_back( ADDDATA_NLR );
    }
    else
    {
        FormulaAddress aPos = DumpTokenAddress( false );
        WriteInfoItem( "addr", lclCreateNlr( aPos ) );
        mxStack->PushOperand( Out().GetLastItemValue(), rTokClass );
    }
}

void FormulaObject::DumpNlrRangeToken( const String& rTokClass, bool bAddData )
{
    if( bAddData )
    {
        DumpUnused( 4 );
        mxStack->PushOperand( CreatePlaceHolder(), rTokClass );
        maAddData.push_back( ADDDATA_NLR );
    }
    else
    {
        FormulaAddress aPos = DumpTokenAddress( false );
        WriteInfoItem( "addr", lclCreateNlr( aPos ) );
        mxStack->PushOperand( Out().GetLastItemValue(), rTokClass );
    }
    DumpUnknown( 1 );
    DumpRange( "target-range" );
}

void FormulaObject::DumpNlrRangeErrToken()
{
    DumpDec< sal_uInt32 >( "delname-idx" );
    DumpUnused( 9 );
    mxStack->PushOperand( lclCreateNlr( GetErrorName( EXC_ERR_NAME ) ) );
}

void FormulaObject::DumpAddTokenData()
{
    Output& rOut = Out();
    rOut.ResetItemIndex();
    for( AddDataTypeVec::const_iterator aIt = maAddData.begin(), aEnd = maAddData.end(); aIt != aEnd; ++aIt )
    {
        AddDataType eType = *aIt;

        {
            ItemGuard aItem( rOut, "#add-data" );
            switch( eType )
            {
                case ADDDATA_NLR:       rOut.WriteAscii( "tNlr" );      break;
                case ADDDATA_ARRAY:     rOut.WriteAscii( "tArray" );    break;
                case ADDDATA_MEMAREA:   rOut.WriteAscii( "tMemArea" );  break;
            }
        }

        size_t nIdx = aIt - maAddData.begin();
        IndentGuard aIndGuard( rOut );
        switch( eType )
        {
            case ADDDATA_NLR:       DumpAddDataNlr( nIdx );     break;
            case ADDDATA_ARRAY:     DumpAddDataArray( nIdx );   break;
            case ADDDATA_MEMAREA:   DumpAddDataMemArea( nIdx ); break;
        }
    }
}

void FormulaObject::DumpAddDataNlr( size_t nIdx )
{
    sal_uInt32 nFlags = DumpHex< sal_uInt32 >( "flags", "NLRADDFLAGS" );
    bool bRel = ::get_flag( nFlags, EXC_TOK_NLR_ADDREL );
    sal_uInt32 nCount = nFlags & EXC_TOK_NLR_ADDMASK;
    String aAddrList;
    for( sal_uInt32 nPos = 0; nPos < nCount; ++nPos )
    {
        Address aPos;
        aPos.Read( GetXclStream() );
        String aAddr;
        StringHelper::AppendAddress( aAddr, aPos );
        StringHelper::AppendToken( aAddrList, aAddr, SCF_DUMP_LISTSEP );
    }
    WriteInfoItem( "stacked-positions", aAddrList );
    mxStack->ReplaceOnTop( CreatePlaceHolder( nIdx ), lclCreateNlr( aAddrList ) );
}

void FormulaObject::DumpAddDataArray( size_t nIdx )
{
    sal_uInt32 nCols, nRows;
    DumpConstArrayHeader( nCols, nRows );

    String aOp;
    TableGuard aTabGuard( Out(), 17 );
    for( sal_uInt32 nRow = 0; nRow < nRows; ++nRow )
    {
        String aArrayLine;
        for( sal_uInt32 nCol = 0; nCol < nCols; ++nCol )
            StringHelper::AppendToken( aArrayLine, DumpConstValue(), SCF_DUMP_LISTSEP );
        StringHelper::AppendToken( aOp, aArrayLine, SCF_DUMP_ARRAYSEP );
    }
    StringHelper::Enclose( aOp, '{', '}' );
    mxStack->ReplaceOnTop( CreatePlaceHolder( nIdx ), aOp );
}

void FormulaObject::DumpAddDataMemArea( size_t nIdx )
{
    DumpRangeList( 0, GetBiff() == EXC_BIFF8 );
}

// ============================================================================
// ============================================================================

RecordStreamObject::RecordStreamObject( const ObjectBase& rParent, SvStream& rStrm )
{
    Construct( rParent, rStrm );
}

RecordStreamObject::RecordStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    Construct( rParentStrg, rStrmName );
}

RecordStreamObject::~RecordStreamObject()
{
}

void RecordStreamObject::Construct( const ObjectBase& rParent, SvStream& rStrm )
{
    RootObjectBase::Construct( rParent, rStrm );
    ConstructOwn();
}

void RecordStreamObject::Construct( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    RootObjectBase::Construct( rParentStrg, rStrmName );
    ConstructOwn();
}

bool RecordStreamObject::ImplIsValid() const
{
    return IsValid( mxFmlaObj ) && RootObjectBase::ImplIsValid();
}

void RecordStreamObject::ImplDumpBody()
{
    XclImpStream& rStrm = GetXclStream();

    ScfProgressBar aProgress( Core().GetDocShell(), maProgressName );
    sal_Int32 nSegId = aProgress.AddSegment( rStrm.GetSvStreamSize() );
    aProgress.ActivateSegment( nSegId );

    while( rStrm.StartNextRecord() )
    {
        aProgress.ProgressAbs( rStrm.GetSvStreamPos() );

        // record header
        rStrm.ResetRecord( mbMergeContRec );
        ImplPreProcessRecord();
        DumpRecordHeader();

        // record contents
        if( mbShowRecBody )
        {
            IndentGuard aIndGuard( Out() );
            if( Cfg().HasName( mxRecNames, rStrm.GetRecId() ) )
                DumpRecord();
            else
                DumpRawBinary( rStrm.GetRecSize(), false );
        }
        if( !rStrm.IsValid() )
            WriteInfoItem( "stream-state", SCF_DUMP_ERR_STREAM );

        // postprocessing
        ImplPostProcessRecord();
        Out().EmptyLine();
    }
}

void RecordStreamObject::ImplPreProcessRecord()
{
}

void RecordStreamObject::ImplDumpRecord()
{
}

void RecordStreamObject::ImplPostProcessRecord()
{
}

void RecordStreamObject::DumpRepeatedRecordId()
{
    DumpHex< sal_uInt16 >( "repeated-rec-id", mxRecNames );
    DumpUnused( 2 );
}

sal_uInt16 RecordStreamObject::DumpFormulaSize( const sal_Char* pcName )
{
    return mxFmlaObj->DumpFormulaSize( pcName );
}

void RecordStreamObject::DumpCellFormula( const sal_Char* pcName, sal_uInt16 nSize )
{
    mxFmlaObj->DumpFormula( pcName, nSize, false );
}

void RecordStreamObject::DumpCellFormula( const sal_Char* pcName )
{
    mxFmlaObj->DumpFormula( pcName, false );
}

void RecordStreamObject::DumpNameFormula( const sal_Char* pcName, sal_uInt16 nSize )
{
    mxFmlaObj->DumpFormula( pcName, nSize, true );
}

void RecordStreamObject::DumpNameFormula( const sal_Char* pcName )
{
    mxFmlaObj->DumpFormula( pcName, true );
}

void RecordStreamObject::ConstructOwn()
{
    if( RootObjectBase::ImplIsValid() )
        mxFmlaObj.reset( new FormulaObject( *this ) );

    if( IsValid() )
    {
        maProgressName.AssignAscii( "Dumping stream '" ).Append( GetFullName() ).AppendAscii( "'." );

        const Config& rCfg = Cfg();
        mxRecNames      = rCfg.GetNameList( "RECORD-NAMES" );
        mxSimpleRecs    = rCfg.GetNameList( "SIMPLE-RECORDS" );
        mbShowRecPos    = rCfg.GetBoolOption( "show-record-pos", true );
        mbShowRecSize   = rCfg.GetBoolOption( "show-record-size", true );
        mbShowRecId     = rCfg.GetBoolOption( "show-record-id", true );
        mbShowRecName   = rCfg.GetBoolOption( "show-record-name", true );
        mbShowRecBody   = rCfg.GetBoolOption( "show-record-body", true );
        mbMergeContRec  = rCfg.GetBoolOption( "merge-continue-record", true );
    }
}

void RecordStreamObject::DumpRecordHeader()
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteEmptyItem( "REC" );
    if( mbShowRecPos )  WriteHexItem( "pos", static_cast< sal_uInt32 >( GetStream().Tell() - 4 ) );
    XclImpStream& rStrm = GetXclStream();
    if( mbShowRecSize ) WriteHexItem( "size", static_cast< sal_uInt32 >( rStrm.GetRecSize() ) );
    if( mbShowRecId )   WriteHexItem( "id", rStrm.GetRecId() );
    if( mbShowRecName ) WriteNameItem( "name", rStrm.GetRecId(), mxRecNames );
}

void RecordStreamObject::DumpRecord()
{
    XclImpStream& rStrm = GetXclStream();
    sal_uInt16 nRecId = rStrm.GetRecId();
    if( Cfg().HasName( mxSimpleRecs, nRecId ) )
        DumpSimpleRecord( Cfg().GetName( mxSimpleRecs, nRecId ) );
    else
        ImplDumpRecord();
    // remaining undumped data
    if( rStrm.GetRecPos() == 0 )
        DumpRawBinary( rStrm.GetRecSize(), false );
    else
        DumpRemaining( rStrm.GetRecLeft() );
}

void RecordStreamObject::DumpSimpleRecord( const String& rRecData )
{
    ItemFormat aItemFmt;
    aItemFmt.Parse( rRecData );
    DumpItem( aItemFmt );
}

// ============================================================================

WorkbookStreamObject::WorkbookStreamObject( const ObjectBase& rParent, SvStream& rStrm )
{
    RecordStreamObject::Construct( rParent, rStrm );
    ConstructOwn();
}

WorkbookStreamObject::WorkbookStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    RecordStreamObject::Construct( rParentStrg, rStrmName );
    ConstructOwn();
}

WorkbookStreamObject::~WorkbookStreamObject()
{
    if( WorkbookStreamObject::ImplIsValid() )
    {
        Config& rCfg = Cfg();
        rCfg.EraseNameList( "FONTNAMES" );
        rCfg.EraseNameList( "FORMATS" );
    }
}

void WorkbookStreamObject::ImplPreProcessRecord()
{
    XclImpStream& rStrm = GetXclStream();
    sal_uInt16 nRecId = rStrm.GetRecId();

    // record specific settings
    switch( nRecId )
    {
        case EXC_ID_CHEND:
            Out().DecIndent();
        break;
    }

    // special CONTINUE handling
    if( IsMergeContRec() )
    {
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
        }
    }
}

void WorkbookStreamObject::ImplDumpRecord()
{
    XclImpStream& rStrm = GetXclStream();
    sal_uInt16 nRecId = rStrm.GetRecId();
    sal_Size nRecSize = rStrm.GetRecSize();
    XclBiff eBiff = GetBiff();

    switch( nRecId )
    {
        case EXC_ID2_BOF:
        case EXC_ID3_BOF:
        case EXC_ID4_BOF:
        case EXC_ID5_BOF:
            DumpHex< sal_uInt16 >( "bof-type", "BOF-BIFFTYPE" );
            DumpHex< sal_uInt16 >( "sheet-type", "BOF-SHEETTYPE" );
            if( nRecSize >= 6 )  DumpDec< sal_uInt16 >( "build-id" );
            if( nRecSize >= 8 )  DumpDec< sal_uInt16 >( "build-year" );
            if( nRecSize >= 12 ) DumpHex< sal_uInt32 >( "history-flags", "BOF-HISTORY-FLAGS" );
            if( nRecSize >= 16 ) DumpDec< sal_uInt32 >( "lowest-ver" );
        break;

        case EXC_ID2_BLANK:
        case EXC_ID3_BLANK:
            DumpCellHeader( nRecId == EXC_ID2_BLANK );
        break;

        case EXC_ID2_BOOLERR:
        case EXC_ID3_BOOLERR:
            DumpCellHeader( nRecId == EXC_ID2_BOOLERR );
            DumpBoolErr();
        break;

        case EXC_ID_CH3DDATAFORMAT:
            DumpDec< sal_uInt8 >( "base", "CH3DDATAFORMAT-BASE" );
            DumpDec< sal_uInt8 >( "top", "CH3DDATAFORMAT-TOP" );
        break;

        case EXC_ID_CHAREAFORMAT:
            DumpRgbColor( "fg-color-rgb" );
            DumpRgbColor( "bg-color-rgb" );
            DumpPatternIdx();
            DumpHex< sal_uInt16 >( "flags", "CHAREAFORMAT-FLAGS" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx( "fg-color-idx" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx( "bg-color-idx" );
        break;

        case EXC_ID_CHAXESSET:
            DumpDec< sal_uInt16 >( "axesset-id", "CHAXESSET-ID" );
            DumpRect< sal_Int32 >( "position", (eBiff <= EXC_BIFF4) ? "CONV-TWIP-TO-CM" : "" );
        break;

        case EXC_ID_CHAXIS:
            DumpDec< sal_uInt16 >( "axis-type", "CHAXIS-TYPE" );
            if( eBiff <= EXC_BIFF4 )
                DumpRect< sal_Int32 >( "position", "CONV-TWIP-TO-CM" );
            else
                DumpUnused( 16 );
        break;

        case EXC_ID_CHBAR:
            DumpDec< sal_Int16 >( "overlap", "CONV-PERCENT-NEG" );
            DumpDec< sal_Int16 >( "gap", "CONV-PERCENT" );
            DumpHex< sal_uInt16 >( "flags", "CHBAR-FLAGS" );
        break;

        case EXC_ID_CHCHART:
            DumpRect< sal_Int32 >( "chart-frame", "CONV-PT-TO-CM", FORMATTYPE_FIX );
        break;

        case EXC_ID_CHCHART3D:
            DumpDec< sal_uInt16 >( "rotation-angle", "CONV-DEG" );
            DumpDec< sal_Int16 >( "elevation-angle", "CONV-DEG" );
            DumpDec< sal_uInt16 >( "eye-distance" );
            DumpDec< sal_uInt16 >( "relative-height" );
            DumpDec< sal_uInt16 >( "relative-depth" );
            DumpDec< sal_Int16 >( "gap", "CONV-PERCENT" );
            DumpHex< sal_uInt16 >( "flags", "CHCHART3D-FLAGS" );
        break;

        case EXC_ID_CHCHARTGROUP:
            DumpUnused( 16 );
            DumpHex< sal_uInt16 >( "flags", "CHCHARTGROUP-FLAGS" );
            if( eBiff >= EXC_BIFF5 ) DumpDec< sal_uInt16 >( "group-idx" );
        break;

        case EXC_ID_CHDATAFORMAT:
            DumpDec< sal_Int16 >( "point-idx", "CHDATAFORMAT-POINTIDX" );
            DumpDec< sal_Int16 >( "series-idx" );
            if( eBiff >= EXC_BIFF5 ) DumpDec< sal_Int16 >( "format-idx", "CHDATAFORMAT-FORMATIDX" );
            if( eBiff >= EXC_BIFF5 ) DumpHex< sal_uInt16 >( "flags", "CHDATAFORMAT-FLAGS" );
        break;

        case EXC_ID_CHEXTRANGE:
            DumpDec< sal_uInt16 >( "minimum-categ" );
            DumpDec< sal_uInt16 >( "maximum-categ" );
            DumpDec< sal_uInt16 >( "major-unit-value" );
            DumpDec< sal_uInt16 >( "major-unit" );
            DumpDec< sal_uInt16 >( "minor-unit-value" );
            DumpDec< sal_uInt16 >( "minor-unit" );
            DumpDec< sal_uInt16 >( "base-unit" );
            DumpDec< sal_uInt16 >( "axis-crossing-date" );
            DumpHex< sal_uInt16 >( "flags", "CHEXTRANGE-FLAGS" );
        break;

        case EXC_ID_CHFRAME:
            DumpDec< sal_uInt16 >( "format", "CHFRAME-FORMAT" );
            DumpHex< sal_uInt16 >( "flags", "CHFRAME-FLAGS" );
        break;

        case EXC_ID_CHFRAMEPOS:
            DumpDec< sal_uInt16 >( "object-type", "CHFRAMEPOS-OBJTYPE" );
            DumpDec< sal_uInt16 >( "size-mode", "CHFRAMEPOS-SIZEMODE" );
            DumpRect< sal_Int32 >( "position", (eBiff <= EXC_BIFF4) ? "CONV-TWIP-TO-CM" : "" );
        break;

        case EXC_ID_CHLABELRANGE:
            DumpDec< sal_uInt16 >( "axis-crossing" );
            DumpDec< sal_uInt16 >( "label-frequency" );
            DumpDec< sal_uInt16 >( "tick-frequency" );
            DumpHex< sal_uInt16 >( "flags", "CHLABELRANGE-FLAGS" );
        break;

        case EXC_ID_CHLEGEND:
            DumpRect< sal_Int32 >( "position", (eBiff <= EXC_BIFF4) ? "CONV-TWIP-TO-CM" : "" );
            DumpDec< sal_uInt8 >( "docked-pos", "CHLEGEND-DOCKPOS" );
            DumpDec< sal_uInt8 >( "spacing", "CHLEGEND-SPACING" );
            DumpHex< sal_uInt16 >( "flags", "CHLEGEND-FLAGS" );
        break;

        case EXC_ID_CHLINEFORMAT:
            DumpRgbColor();
            DumpDec< sal_uInt16 >( "line-type", "CHLINEFORMAT-LINETYPE" );
            DumpDec< sal_Int16 >( "line-weight", "CHLINEFORMAT-LINEWEIGHT" );
            DumpHex< sal_uInt16 >( "flags", "CHLINEFORMAT-FLAGS" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx();
        break;

        case EXC_ID_CHMARKERFORMAT:
            DumpRgbColor( "border-color-rgb" );
            DumpRgbColor( "fill-color-rgb" );
            DumpDec< sal_uInt16 >( "marker-type", "CHMARKERFORMAT-TYPE" );
            DumpDec< sal_uInt16 >( "flags", "CHMARKERFORMAT-FLAGS" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx( "border-color-idx" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx( "fill-color-idx" );
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_Int32 >( "marker-size", "CONV-TWIP-TO-PT" );
        break;

        case EXC_ID_CHOBJECTLINK:
            DumpDec< sal_uInt16 >( "link-target", "CHOBJECTLINK-TARGET" );
            DumpDec< sal_Int16 >( "series-idx" );
            DumpDec< sal_Int16 >( "point-idx", "CHOBJECTLINK-POINT" );
        break;

        case EXC_ID_CHPIE:
            DumpDec< sal_uInt16 >( "angle", "CONV-DEG" );
            if( eBiff >= EXC_BIFF5 ) DumpDec< sal_uInt16 >( "hole-size" );
            if( eBiff >= EXC_BIFF8 ) DumpHex< sal_uInt16 >( "flags", "CHPIE-FLAGS" );
        break;

        case EXC_ID_CHPLOTGROWTH:
            DumpFix< sal_Int32 >( "horizontal-growth" );
            DumpFix< sal_Int32 >( "vertical-growth" );
        break;

        case EXC_ID_CHPROPERTIES:
            DumpHex< sal_uInt16 >( "flags", "CHPROPERTIES-FLAGS" );
            DumpDec< sal_uInt8 >( "empty-cells", "CHPROPERTIES-EMPTYCELLS" );
        break;

        case EXC_ID_CHSCATTER:
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "bubble-size", "CONV-PERCENT" );
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "size-type", "CHSCATTER-SIZETYPE" );
            if( eBiff == EXC_BIFF8 ) DumpHex< sal_uInt16 >( "flags", "CHSCATTER-FLAGS" );
        break;

        case EXC_ID_CHSERERRORBAR:
            DumpDec< sal_uInt8 >( "type", "CHSERERRORBAR-TYPE" );
            DumpDec< sal_uInt8 >( "source", "CHSERERRORBAR-SOURCE" );
            DumpBool< sal_uInt8 >( "draw-t-shape" );
            DumpUnused( 1 );
            DumpDec< double >( "value" );
            DumpDec< sal_uInt16 >( "custom-count" );
        break;

        case EXC_ID_CHSERIES:
            DumpDec< sal_uInt16 >( "categories-type", "CHSERIES-TYPE" );
            DumpDec< sal_uInt16 >( "values-type", "CHSERIES-TYPE" );
            DumpDec< sal_uInt16 >( "categories-count" );
            DumpDec< sal_uInt16 >( "values-count" );
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "bubbles-type", "CHSERIES-TYPE" );
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "bubbles-count" );
        break;

        case EXC_ID_CHSERTRENDLINE:
            switch( DumpDec< sal_uInt8 >( "type", "CHSERTRENDLINE-TYPE" ) )
            {
                case EXC_CHSERTREND_POLYNOMIAL: DumpDec< sal_uInt8 >( "order" );            break;
                case EXC_CHSERTREND_MOVING_AVG: DumpDec< sal_uInt8 >( "average-period" );   break;
                default:                        DumpUnused( 1 );
            }
            DumpDec< double >( "intercept" );
            DumpBool< sal_uInt8 >( "show-equation" );
            DumpBool< sal_uInt8 >( "show-r-sqrare" );
            DumpDec< double >( "forecast-forward" );
            DumpDec< double >( "forecast-backward" );
        break;

        case EXC_ID_CHSOURCELINK:
            DumpDec< sal_uInt8 >( "link-target", "CHSOURCELINK-TARGET" );
            DumpDec< sal_uInt8 >( "link-type", "CHSOURCELINK-TYPE" );
            DumpHex< sal_uInt16 >( "flags", "CHSOURCELINK-FLAGS" );
            DumpFormatIdx();
            DumpNameFormula();
        break;

        case EXC_ID_CHSTRING:
            DumpDec< sal_uInt16 >( "text-type", "CHSTRING-TYPE" );
            DumpString( "text", EXC_STR_8BITLENGTH );
        break;

        case EXC_ID_CHTEXT:
            DumpDec< sal_uInt8 >( "horizontal-align", "CHTEXT-HORALIGN" );
            DumpDec< sal_uInt8 >( "vertical-align", "CHTEXT-VERALIGN" );
            DumpDec< sal_uInt16 >( "fill-mode", "CHTEXT-FILLMODE" );
            DumpRgbColor();
            DumpRect< sal_Int32 >( "position", (eBiff <= EXC_BIFF4) ? "CONV-TWIP-TO-CM" : "" );
            DumpHex< sal_uInt16 >( "flags", "CHTEXT-FLAGS" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx();
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "placement", "CHTEXT-PLACEMENT" );
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "rotation", "TEXTROTATION" );
        break;

        case EXC_ID_CHTICK:
            DumpDec< sal_uInt8 >( "major-ticks", "CHTICK-TYPE" );
            DumpDec< sal_uInt8 >( "minor-ticks", "CHTICK-TYPE" );
            DumpDec< sal_uInt8 >( "label-position", "CHTICK-LABELPOS" );
            DumpDec< sal_uInt8 >( "fill-mode", "CHTEXT-FILLMODE" );
            DumpRgbColor( "label-color-rgb" );
            DumpUnused( 16 );
            DumpHex< sal_uInt16 >( "flags", "CHTICK-FLAGS" );
            if( eBiff == EXC_BIFF8 ) DumpColorIdx( "label-color-idx" );
            if( eBiff == EXC_BIFF8 ) DumpDec< sal_uInt16 >( "label-rotation", "TEXTROTATION" );
        break;

        case EXC_ID_CHUNITPROPERTIES:
            DumpRepeatedRecordId();
            DumpDec< sal_Int16 >( "preset", "CHUNITPROPERTIES-PRESET" );
            DumpDec< double >( "unit" );
            DumpHex< sal_uInt16 >( "flags", "CHUNITPROPERTIES-FLAGS" );
        break;

        case EXC_ID_CHVALUERANGE:
            DumpDec< double >( "minimum" );
            DumpDec< double >( "maximum" );
            DumpDec< double >( "major-inc" );
            DumpDec< double >( "minor-inc" );
            DumpDec< double >( "axis-crossing" );
            DumpHex< sal_uInt16 >( "flags", "CHVALUERANGE-FLAGS" );
        break;

        case EXC_ID_CHWRAPPEDRECORD:
            DumpRepeatedRecordId();
        break;

        case EXC_ID_CODEPAGE:
            DumpCodePageRec();
        break;

        case EXC_ID3_DEFROWHEIGHT:
            DumpHex< sal_uInt16 >( "flags", "DEFROWHEIGHT-FLAGS" );
            DumpDec< sal_uInt16 >( "row-height", "CONV-TWIP-TO-PT" );
        break;

        case EXC_ID2_DIMENSIONS:
        case EXC_ID3_DIMENSIONS:
            DumpRange( "used-area", true, (nRecId == EXC_ID3_DIMENSIONS) && (eBiff == EXC_BIFF8) );
            if( nRecId == EXC_ID3_DIMENSIONS ) DumpUnused( 2 );
        break;

        case EXC_ID2_FONT:
        case EXC_ID3_FONT:
            DumpFontRec();
        break;

        case EXC_ID2_FORMAT:
        case EXC_ID4_FORMAT:
            DumpFormatRec();
        break;

        case EXC_ID2_FORMULA:
            DumpCellHeader( eBiff == EXC_BIFF2 );
            DumpFormulaResult();
            DumpHex< sal_uInt16, sal_uInt8 >( eBiff != EXC_BIFF2, "flags", "FORMULA-FLAGS" );
            if( eBiff >= EXC_BIFF5 ) DumpUnused( 4 );
            DumpCellFormula();
        break;

        case EXC_ID3_FORMULA:
        case EXC_ID4_FORMULA:
            DumpCellHeader();
            DumpFormulaResult();
            DumpHex< sal_uInt16 >( "flags", "FORMULA-FLAGS" );
            DumpCellFormula();
        break;

        case EXC_ID2_INTEGER:
            DumpCellHeader( true );
            DumpDec< sal_uInt16 >( "value" );
        break;

        case EXC_ID2_LABEL:
        case EXC_ID3_LABEL:
        {
            sal_uInt16 nXfIdx = DumpCellHeader( nRecId == EXC_ID2_LABEL );
            sal_uInt16 nFontIdx = GetXfData( nXfIdx );
            rtl_TextEncoding eOldTextEnc = Root().GetTextEncoding();
            Root().SetTextEncoding( GetFontEncoding( nXfIdx ) );
            DumpString( "value", ((nRecId == EXC_ID2_LABEL) && (GetBiff() <= EXC_BIFF5)) ? EXC_STR_8BITLENGTH : EXC_STR_DEFAULT );
            Root().SetTextEncoding( eOldTextEnc );
        }
        break;

        case EXC_ID_LABELSST:
            DumpCellHeader();
            DumpDec< sal_uInt32 >( "sst-idx" );
        break;

        case EXC_ID2_NUMBER:
        case EXC_ID3_NUMBER:
            DumpCellHeader( nRecId == EXC_ID2_NUMBER );
            DumpDec< double >( "value" );
        break;

        case EXC_ID_RK:
            DumpCellHeader();
            DumpRk( "value" );
        break;

        case EXC_ID_SST:
            DumpDec< sal_uInt32 >( "string-cell-count" );
            DumpDec< sal_uInt32 >( "sst-size" );
            Out().ResetItemIndex();
            while( rStrm.IsValid() && (rStrm.GetRecLeft() >= 3) )
                DumpString( "#entry" );
        break;

        case EXC_ID2_STRING:
        case EXC_ID3_STRING:
            DumpString( "result", (eBiff == EXC_BIFF2) ? EXC_STR_8BITLENGTH : EXC_STR_DEFAULT );
        break;

        case EXC_ID2_XF:
        case EXC_ID3_XF:
        case EXC_ID4_XF:
        case EXC_ID5_XF:
            DumpXfRec();
        break;
    }
}

void WorkbookStreamObject::ImplPostProcessRecord()
{
    XclImpStream& rStrm = GetXclStream();
    sal_uInt16 nRecId = rStrm.GetRecId();

    // record specific settings
    switch( nRecId )
    {
        case EXC_ID_CHBEGIN:
            Out().IncIndent();
        break;
    }
}

void WorkbookStreamObject::ConstructOwn()
{
    if( IsValid() )
    {
        Config& rCfg = Cfg();
        mxColors        = rCfg.GetNameList( "COLORS" );
        mxBorderStyles  = rCfg.GetNameList( "BORDERSTYLES" );
        mxFillPatterns  = rCfg.GetNameList( "FILLPATTERNS" );
        mxFontNames     = rCfg.CreateNameList< ConstList >( "FONTNAMES" );
        mxFontNames->SetName( EXC_FONT_APP, "'Arial'/10pt" );
        mxFormats       = rCfg.CreateNameList< ConstList >( "FORMATS" );
        mxFormats->IncludeList( rCfg.GetNameList( "BUILTIN-FORMATS" ) );
        mnFormatIdx = 0;
    }
}

const XclFontData* WorkbookStreamObject::GetFontData( sal_uInt16 nFontIdx ) const
{
    return (nFontIdx < maFontDatas.size()) ? &maFontDatas[ nFontIdx ] : 0;
}

sal_uInt16 WorkbookStreamObject::GetXfData( sal_uInt16 nXfIdx ) const
{
    return (nXfIdx < maXfDatas.size()) ? maXfDatas[ nXfIdx ] : EXC_FONT_NOTFOUND;
}

rtl_TextEncoding WorkbookStreamObject::GetFontEncoding( sal_uInt16 nXfIdx ) const
{
    const XclFontData* pFontData = GetFontData( GetXfData( nXfIdx ) );
    rtl_TextEncoding eFontEnc = pFontData ? pFontData->GetScCharSet() : Root().GetTextEncoding();
    return (eFontEnc == RTL_TEXTENCODING_DONTKNOW) ? Root().GetTextEncoding() : eFontEnc;
}

String WorkbookStreamObject::CreateFontName( const XclFontData& rFontData ) const
{
    String aName = rFontData.maName;
    StringHelper::Enclose( aName, '\'' );
    StringHelper::AppendToken( aName, Cfg().GetName( "CONV-TWIP-TO-PT", rFontData.mnHeight ), '/' );
    if( rFontData.mnWeight > EXC_FONTWGHT_NORMAL )
        StringHelper::AppendToken( aName, CREATE_STRING( "bold" ), '/' );
    if( rFontData.mbItalic )
        StringHelper::AppendToken( aName, CREATE_STRING( "italic" ), '/' );
    return aName;
}

sal_uInt16 WorkbookStreamObject::DumpPatternIdx( const sal_Char* pcName )
{
    return DumpDec< sal_uInt16 >( pcName ? pcName : "fill-pattern", mxFillPatterns );
}

sal_uInt16 WorkbookStreamObject::DumpColorIdx( const sal_Char* pcName )
{
    return DumpDec< sal_uInt16 >( pcName ? pcName : "color-idx", mxColors );
}

sal_uInt16 WorkbookStreamObject::DumpFontIdx( const sal_Char* pcName )
{
    return DumpDec< sal_uInt16, sal_uInt8 >( GetBiff() >= EXC_BIFF5, pcName ? pcName : "font-idx", "FONTNAMES" );
}

sal_uInt16 WorkbookStreamObject::DumpFormatIdx( const sal_Char* pcName )
{
    return DumpDec< sal_uInt16, sal_uInt8 >( GetBiff() >= EXC_BIFF5, pcName ? pcName : "fmt-idx", "FORMATS" );
}

sal_uInt16 WorkbookStreamObject::DumpXfIdx( const sal_Char* pcName, bool bBiff2Style )
{
    if( !pcName ) pcName = "xf-idx";
    sal_uInt16 nXfIdx = 0;
    if( bBiff2Style )
    {
        DumpHex< sal_uInt8 >( pcName, "CELL-XFINDEX" );
        DumpHex< sal_uInt8 >( "fmt-font-idx", "CELL-XFFORMAT" );
        DumpHex< sal_uInt8 >( "style", "CELL-XFSTYLE" );
    }
    else
        nXfIdx = DumpDec< sal_uInt16 >( pcName );
    return nXfIdx;
}

sal_uInt16 WorkbookStreamObject::DumpCellHeader( bool bBiff2Style )
{
    DumpAddress();
    return DumpXfIdx( 0, bBiff2Style );
}

void WorkbookStreamObject::DumpBoolErr()
{
    MultiItemsGuard aMultiGuard( Out() );
    sal_uInt8 nValue = DumpHex< sal_uInt8 >( "value" );
    bool bErrCode = DumpBool< sal_uInt8 >( "is-errorcode" );
    if( bErrCode )
        WriteErrorCodeItem( "errorcode", nValue );
    else
        WriteBooleanItem( "boolean", nValue );
}

void WorkbookStreamObject::DumpCodePageRec()
{
    rtl_TextEncoding eTextEnc = DumpCodePage();
    if( eTextEnc != RTL_TEXTENCODING_DONTKNOW )
        Root().SetTextEncoding( eTextEnc );
}

void WorkbookStreamObject::DumpFontRec()
{
    if( maFontDatas.size() == 4 )
        maFontDatas.push_back( XclFontData() );

    Out().ResetItemIndex( static_cast< sal_Int64 >( maFontDatas.size() ) );
    WriteEmptyItem( "#font" );
    XclFontData aFontData;
    aFontData.mnHeight = DumpDec< sal_uInt16 >( "height", "CONV-TWIP-TO-PT" );
    sal_uInt16 nFlags = DumpHex< sal_uInt16 >( "flags", "FONT-FLAGS" );
    aFontData.mbItalic = ::get_flag( nFlags, EXC_FONTATTR_ITALIC );
    aFontData.mbStrikeout = ::get_flag( nFlags, EXC_FONTATTR_STRIKEOUT );
    aFontData.mbOutline = ::get_flag( nFlags, EXC_FONTATTR_OUTLINE );
    aFontData.mbShadow = ::get_flag( nFlags, EXC_FONTATTR_SHADOW );
    aFontData.mnWeight = ::get_flagvalue( nFlags, EXC_FONTATTR_BOLD, EXC_FONTWGHT_BOLD, EXC_FONTWGHT_NORMAL );
    aFontData.mnUnderline = ::get_flagvalue( nFlags, EXC_FONTATTR_UNDERLINE, EXC_FONTUNDERL_SINGLE, EXC_FONTUNDERL_NONE );
    if( GetBiff() >= EXC_BIFF3 )
        aFontData.mnColor = DumpColorIdx();
    if( GetBiff() >= EXC_BIFF5 )
    {
        aFontData.mnWeight = DumpDec< sal_uInt16 >( "weight", "FONT-WEIGHT" );
        aFontData.mnEscapem = DumpDec< sal_uInt16 >( "escapement", "FONT-ESCAPEMENT" );
        aFontData.mnUnderline = DumpDec< sal_uInt8 >( "underline", "FONT-UNDERLINE" );
        aFontData.mnFamily = DumpDec< sal_uInt8 >( "family", "FONT-FAMILY" );
        aFontData.mnCharSet = DumpDec< sal_uInt8 >( "charset", "FONT-CHARSET" );
        DumpUnused( 1 );
    }
    aFontData.maName = DumpString( "name", EXC_STR_8BITLENGTH );

    // append font data to vector
    mxFontNames->SetName( static_cast< sal_Int64 >( maFontDatas.size() ), CreateFontName( aFontData ) );
    maFontDatas.push_back( aFontData );
}

void WorkbookStreamObject::DumpFormatRec()
{
    sal_uInt16 nFormatIdx = 0;
    switch( GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
            nFormatIdx = mnFormatIdx++;
            Out().ResetItemIndex( nFormatIdx );
            WriteEmptyItem( "#fmt" );
        break;
        case EXC_BIFF4:
            nFormatIdx = mnFormatIdx++;
            Out().ResetItemIndex( nFormatIdx );
            WriteEmptyItem( "#fmt" );
            DumpUnused( 2 );
        break;
        case EXC_BIFF5:
        case EXC_BIFF8:
            GetXclStream() >> nFormatIdx;
            Out().ResetItemIndex( nFormatIdx );
            WriteEmptyItem( "#fmt" );
            WriteDecItem( "fmt-idx", nFormatIdx );
        break;
    }
    String aFormat = DumpString( "format", (GetBiff() <= EXC_BIFF5) ? EXC_STR_8BITLENGTH : EXC_STR_DEFAULT );
    mxFormats->SetName( nFormatIdx, aFormat );
}

void WorkbookStreamObject::DumpXfRec()
{
    Out().ResetItemIndex( static_cast< sal_Int64 >( maXfDatas.size() ) );
    WriteEmptyItem( "#xf" );
    sal_uInt16 nFontIdx = DumpFontIdx();
    switch( GetBiff() )
    {
        case EXC_BIFF2:
            DumpUnused( 1 );
            DumpHex< sal_uInt8 >( "type-flags", "XF-TYPEFLAGS" );
            DumpHex< sal_uInt8 >( "style-flags", "XF-STYLEFLAGS" );

        break;
        case EXC_BIFF3:
            DumpFormatIdx();
            DumpHex< sal_uInt8 >( "type-flags", "XF-TYPEFLAGS" );
            DumpHex< sal_uInt8 >( "used-attributes", "XF-USEDATTRIBS-FLAGS" );
            DumpHex< sal_uInt16 >( "alignment", "XF-ALIGNMENT" );
            DumpHex< sal_uInt16 >( "fill-style", "XF-FILL" );
            DumpHex< sal_uInt32 >( "border-style", "XF-BORDER" );
        break;
        case EXC_BIFF4:
            DumpFormatIdx();
            DumpHex< sal_uInt16 >( "type-flags", "XF-TYPEFLAGS" );
            DumpHex< sal_uInt8 >( "alignment", "XF-ALIGNMENT" );
            DumpHex< sal_uInt8 >( "used-attributes", "XF-USEDATTRIBS-FLAGS" );
            DumpHex< sal_uInt16 >( "fill-style", "XF-FILL" );
            DumpHex< sal_uInt32 >( "border-style", "XF-BORDER" );
        break;
        case EXC_BIFF5:
            DumpFormatIdx();
            DumpHex< sal_uInt16 >( "type-flags", "XF-TYPEFLAGS" );
            DumpHex< sal_uInt8 >( "alignment", "XF-ALIGNMENT" );
            DumpHex< sal_uInt8 >( "orientation", "XF-ORIENTATTRIBS" );
            DumpHex< sal_uInt32 >( "fill-style", "XF-FILL" );
            DumpHex< sal_uInt32 >( "border-style", "XF-BORDER" );
        break;
        case EXC_BIFF8:
            DumpFormatIdx();
            DumpHex< sal_uInt16 >( "type-flags", "XF-TYPEFLAGS" );
            DumpHex< sal_uInt8 >( "alignment", "XF-ALIGNMENT" );
            DumpDec< sal_uInt8 >( "rotation", "TEXTROTATION" );
            DumpHex< sal_uInt8 >( "text-flags", "XF-TEXTFLAGS" );
            DumpHex< sal_uInt8 >( "used-attributes", "XF-USEDATTRIBS-FLAGS" );
            DumpHex< sal_uInt16 >( "border-style", "XF-BORDERSTYLE" );
            DumpHex< sal_uInt16 >( "border-color1", "XF-BORDERCOLOR1" );
            DumpHex< sal_uInt32 >( "border-color2", "XF-BORDERCOLOR2" );
            DumpHex< sal_uInt16 >( "fill-color", "XF-FILLCOLOR" );
        break;
    }
    maXfDatas.push_back( nFontIdx );
}

// ============================================================================

VbaProjectStreamObject::VbaProjectStreamObject( const OleStorageObject& rParentStrg )
{
    OleStreamObject::Construct( rParentStrg, CREATE_STRING( "PROJECT" ) );
}

void VbaProjectStreamObject::ImplDumpBody()
{
    DumpTextStream( ScfTools::GetSystemCharSet() );
}

// ============================================================================
// ============================================================================

VbaProjectStorageObject::VbaProjectStorageObject( const OleStorageObject& rParentStrg )
{
    OleStorageObject::Construct( rParentStrg, EXC_STORAGE_VBA_PROJECT );
}

void VbaProjectStorageObject::ImplDumpBody()
{
    VbaProjectStreamObject( *this ).Dump();
    OleStorageObject( *this, EXC_STORAGE_VBA ).Dump();
}

// ============================================================================

RootStorageObject::RootStorageObject( const ObjectBase& rParent )
{
    OleStorageObject::Construct( rParent );
}

void RootStorageObject::ImplDumpBody()
{
    ExtractStorageToFileSystem();
    WorkbookStreamObject( *this, EXC_STREAM_WORKBOOK ).Dump();
    WorkbookStreamObject( *this, EXC_STREAM_BOOK ).Dump();
    OlePropertyStreamObject( *this, CREATE_STRING( "\005SummaryInformation" ) ).Dump();
    OlePropertyStreamObject( *this, CREATE_STRING( "\005DocumentSummaryInformation" ) ).Dump();
    OleStreamObject( *this, EXC_STREAM_CTLS ).Dump();
    VbaProjectStorageObject( *this ).Dump();
}

// ============================================================================
// ============================================================================

Dumper::Dumper( SfxMedium& rMedium, SfxObjectShell* pDocShell )
{
    ConfigRef xCfg( new Config( "XLSDUMPER" ) );
    DumperBase::Construct( xCfg, rMedium, pDocShell );
    OlePropertyStreamObject::InitializeConfig( *xCfg );
}

void Dumper::ImplDumpBody()
{
    WorkbookStreamObject( *this, GetCoreStream() ).Dump();
    RootStorageObject( *this ).Dump();
}

// ============================================================================

} // namespace xls
} // namespace dump
} // namespace scf

#endif

