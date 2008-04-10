/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xldumper.hxx,v $
 * $Revision: 1.11 $
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

#ifndef SC_XLDUMPER_HXX
#define SC_XLDUMPER_HXX

#include "fdumper.hxx"

#if SCF_INCL_DUMPER

#include <stack>
#include "fdumperdff.hxx"
#include "fdumperole.hxx"
#include "xlconst.hxx"
#include "xlstring.hxx"
#include "xlstyle.hxx"

class SfxMedium;
class SfxObjectShell;
class ScDocument;
struct XclImpRootData;
class XclImpRoot;
class XclImpStream;
class XclFunctionProvider;

namespace scf {
namespace dump {
namespace xls {

// ============================================================================
// ============================================================================

struct Address
{
    sal_Int32           mnCol;
    sal_Int32           mnRow;
    inline explicit     Address() : mnCol( 0 ), mnRow( 0 ) {}
    void                Read( XclImpStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
};

// ----------------------------------------------------------------------------

struct Range
{
    Address             maFirst;
    Address             maLast;
    inline explicit     Range() {}
    void                Read( XclImpStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
};

// ----------------------------------------------------------------------------

struct RangeList : public ::std::vector< Range >
{
    inline explicit     RangeList() {}
    void                Read( XclImpStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
};

// ============================================================================

struct FormulaAddress : public Address
{
    bool                mbRelCol;
    bool                mbRelRow;
    inline explicit     FormulaAddress() : mbRelCol( false ), mbRelRow( false ) {}
};

// ----------------------------------------------------------------------------

struct FormulaRange
{
    FormulaAddress      maFirst;
    FormulaAddress      maLast;
    inline explicit     FormulaRange() {}
};

// ============================================================================

class StringHelper : public ::scf::dump::StringHelper
{
public:
    static void         AppendAddrCol( String& rStr, sal_Int32 nCol, bool bRel );
    static void         AppendAddrRow( String& rStr, sal_Int32 nRow, bool bRel );
    static void         AppendAddrName( String& rStr, sal_Unicode cPrefix, sal_Int32 nColRow, bool bRel );

    static void         AppendAddress( String& rStr, const Address& rPos );
    static void         AppendRange( String& rStr, const Range& rRange );
    static void         AppendRangeList( String& rStr, const RangeList& rRanges );

    static void         AppendAddress( String& rStr, const FormulaAddress& rPos, bool bNameMode );
    static void         AppendRange( String& rStr, const FormulaRange& rRange, bool bNameMode );
};

// ============================================================================
// ============================================================================

class RecordHeaderObject : public RecordHeaderBase
{
public:
    explicit            RecordHeaderObject( const InputObjectBase& rParent );

    inline bool         IsMergeContRec() const { return mbMergeContRec; }
    inline bool         HasRecName( sal_uInt16 nRecId ) const { return GetRecNames()->HasName( nRecId ); }

    void                DumpRecordHeader( XclImpStream& rStrm );

protected:
    virtual bool        ImplIsValid() const;

private:
    bool                mbMergeContRec;
};

typedef ScfRef< RecordHeaderObject > RecHeaderObjectRef;

// ============================================================================
// ============================================================================

class BiffConfig : public Config
{
public:
    explicit            BiffConfig( const Config& rParent, XclBiff eBiff );

protected:
    virtual bool        ImplIsValid() const;
    virtual NameListRef ImplGetNameList( const String& rKey ) const;

private:
    XclBiff             meBiff;
};

typedef ScfRef< BiffConfig > BiffConfigRef;

// ============================================================================

class RootData : public Base
{
public:
    explicit            RootData( SfxMedium& rMedium, XclBiff eBiff );
    virtual             ~RootData();

    inline const XclImpRoot& GetRoot() const { return *mxRoot; }

    rtl_TextEncoding    GetTextEncoding() const;
    void                SetTextEncoding( rtl_TextEncoding eTextEnc );

protected:
    virtual bool        ImplIsValid() const;

private:
    typedef ScfRef< ScDocument >        ScDocumentRef;
    typedef ScfRef< XclImpRootData >    XclImpRootDataRef;
    typedef ScfRef< XclImpRoot >        XclImpRootRef;

    ScDocumentRef       mxDoc;
    XclImpRootDataRef   mxRootData;
    XclImpRootRef       mxRoot;
};

typedef ScfRef< RootData > RootDataRef;

// ============================================================================

class RootObjectBase : public WrappedStreamObject
{
public:
    virtual             ~RootObjectBase();

    inline RootData&    Root() const { return *mxRootData; }
    inline const XclImpRoot& GetXclRoot() const { return mxRootData->GetRoot(); }
    inline XclImpStream& GetXclStream() const { return *mxStrm; }
    inline XclBiff      GetBiff() const { return meBiff; }

protected:
    explicit            RootObjectBase();
    void                Construct( const ObjectBase& rParent, SvStream& rStrm, XclBiff eBiff );
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName, XclBiff eBiff );
    void                Construct( const RootObjectBase& rParent );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpExtendedHeader();
    virtual ConfigRef   ImplReconstructConfig();
    virtual InputRef    ImplReconstructInput();

    String              GetErrorName( sal_uInt8 nErrCode ) const;

    // ------------------------------------------------------------------------

    double              WriteRkItem( const sal_Char* pcName, sal_Int32 nRk );
    void                WriteBooleanItem( const sal_Char* pcName, sal_uInt8 nBool );
    void                WriteErrorCodeItem( const sal_Char* pcName, sal_uInt8 nErrCode );

    void                WriteColIndexItem( const sal_Char* pcName, sal_Int32 nCol );
    void                WriteColRangeItem( const sal_Char* pcName, sal_Int32 nCol1, sal_Int32 nCol2 );
    void                WriteRowIndexItem( const sal_Char* pcName, sal_Int32 nRow );
    void                WriteRowRangeItem( const sal_Char* pcName, sal_Int32 nRow1, sal_Int32 nRow2 );

    void                WriteAddressItem( const sal_Char* pcName, const Address& rPos );
    void                WriteRangeItem( const sal_Char* pcName, const Range& rRange );
    void                WriteRangeListItem( const sal_Char* pcName, const RangeList& rRanges );

    template< typename Type >
    void                WriteRectItem( const sal_Char* pcName,
                            Type nLeft, Type nTop, Type nWidth, Type nHeight,
                            const NameListWrapper& rListWrp = NO_LIST,
                            FormatType eFmtType = FORMATTYPE_DEC );

    // ------------------------------------------------------------------------

    String              DumpString( const sal_Char* pcName = 0, XclStrFlags nFlags = EXC_STR_DEFAULT );
    double              DumpRk( const sal_Char* pcName = 0 );
    sal_uInt8           DumpBoolean( const sal_Char* pcName = 0 );
    sal_uInt8           DumpErrorCode( const sal_Char* pcName = 0 );

    Color               DumpRgbColor( const sal_Char* pcName = 0 );
    rtl_TextEncoding    DumpCodePage( const sal_Char* pcName = 0 );
    void                DumpFormulaResult( const sal_Char* pcName = 0 );

    sal_Int32           DumpColIndex( const sal_Char* pcName = 0, bool bCol16Bit = true );
    sal_Int32           DumpColRange( const sal_Char* pcName = 0, bool bCol16Bit = true );
    sal_Int32           DumpRowIndex( const sal_Char* pcName = 0, bool bRow32Bit = false );
    sal_Int32           DumpRowRange( const sal_Char* pcName = 0, bool bRow32Bit = false );

    Address             DumpAddress( const sal_Char* pcName = 0, bool bCol16Bit = true, bool bRow32Bit = false );
    Range               DumpRange( const sal_Char* pcName = 0, bool bCol16Bit = true, bool bRow32Bit = false );
    void                DumpRangeList( const sal_Char* pcName = 0, bool bCol16Bit = true, bool bRow32Bit = false );

    void                DumpConstArrayHeader( sal_uInt32& rnCols, sal_uInt32& rnRows );
    String              DumpConstValue();

    template< typename Type >
    void                DumpRect( const sal_Char* pcName,
                            const NameListWrapper& rListWrp = NO_LIST,
                            FormatType eFmtType = FORMATTYPE_DEC );

    // ------------------------------------------------------------------------

    using               WrappedStreamObject::Construct;

private:
    void                ConstructRootObjBase( XclBiff eBiff );

private:
    typedef ScfRef< XclImpStream > XclImpStreamRef;

    RootDataRef         mxRootData;
    BiffConfigRef       mxBiffCfg;
    XclImpStreamRef     mxStrm;
    XclBiff             meBiff;
    NameListRef         mxBoolean;
    NameListRef         mxErrCodes;
    NameListRef         mxConstType;
    NameListRef         mxResultType;
};

// ----------------------------------------------------------------------------

template< typename Type >
void RootObjectBase::WriteRectItem( const sal_Char* pcName,
        Type nLeft, Type nTop, Type nWidth, Type nHeight,
        const NameListWrapper& rListWrp, FormatType eFmtType )
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteEmptyItem( pcName );
    WriteValueItem( "x-pos", nLeft, eFmtType, rListWrp );
    WriteValueItem( "y-pos", nTop, eFmtType, rListWrp );
    WriteValueItem( "x-size", nWidth, eFmtType, rListWrp );
    WriteValueItem( "y-size", nHeight, eFmtType, rListWrp );
}

template< typename Type >
void RootObjectBase::DumpRect( const sal_Char* pcName,
        const NameListWrapper& rListWrp, FormatType eFmtType )
{
    Type nLeft, nTop, nWidth, nHeight;
    *mxStrm >> nLeft >> nTop >> nWidth >> nHeight;
    WriteRectItem( pcName, nLeft, nTop, nWidth, nHeight, rListWrp, eFmtType );
}

// ============================================================================
// ============================================================================

class SimpleObjectBase : public RootObjectBase
{
public:
    virtual             ~SimpleObjectBase();

protected:
    inline explicit     SimpleObjectBase() {}
    void                Construct( const RootObjectBase& rParent );

    virtual void        ImplDumpHeader();
    virtual void        ImplDumpFooter();

    using               RootObjectBase::Construct;
};

// ============================================================================
// ============================================================================

/** Stack to create a human readable formula string from a UPN token array. */
class FormulaStack
{
public:
    explicit            FormulaStack();

    inline const String& GetFormulaString() const { return GetString( maFmlaStack ); }
    inline const String& GetClassesString() const { return GetString( maClassStack ); }

    void                PushOperand( const StringWrapper& rOp, const String& rTokClass );
    void                PushOperand( const StringWrapper& rOp );
    void                PushUnaryOp( const StringWrapper& rLOp, const StringWrapper& rROp );
    void                PushBinaryOp( const StringWrapper& rOp );
    void                PushFuncOp( const StringWrapper& rFunc, const String& rTokClass, sal_uInt8 nParamCount );

    inline void         SetError() { mbError = true; }
    void                ReplaceOnTop( const String& rOld, const String& rNew );

private:
    typedef ::std::stack< String > StringStack;

    inline bool         Check( bool bCond ) { return (mbError |= !bCond) == false; }

    const String&       GetString( const StringStack& rStack ) const;
    void                PushUnaryOp( StringStack& rStack, const String& rLOp, const String& rROp );
    void                PushBinaryOp( StringStack& rStack, const String& rOp );
    void                PushFuncOp( StringStack& rStack, const String& rOp, sal_uInt8 nParamCount );

private:
    StringStack         maFmlaStack;
    StringStack         maClassStack;
    bool                mbError;
};

// ============================================================================

class FormulaObject : public SimpleObjectBase
{
public:
    explicit            FormulaObject( const RootObjectBase& rParent );
    virtual             ~FormulaObject();

    sal_uInt16          ReadFormulaSize();
    sal_uInt16          DumpFormulaSize( const sal_Char* pcName = 0 );

    void                DumpCellFormula( const sal_Char* pcName, sal_uInt16 nSize );
    void                DumpCellFormula( const sal_Char* pcName = 0 );
    void                DumpNameFormula( const sal_Char* pcName, sal_uInt16 nSize );
    void                DumpNameFormula( const sal_Char* pcName = 0 );

protected:
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpBody();

private:
    void                ConstructFmlaObj();

    void                DumpFormula( const sal_Char* pcName, sal_uInt16 nSize, bool bNameMode );
    void                DumpFormula( const sal_Char* pcName, bool bNameMode );

    FormulaAddress      CreateTokenAddress( sal_uInt16 nCol, sal_uInt16 nRow, bool bRelC, bool bRelR, bool bNameMode ) const;
    String              CreateFunc( sal_uInt16 nFuncIdx ) const;
    String              CreateRef( const String& rData ) const;
    String              CreateName( sal_uInt16 nNameIdx ) const;
    String              CreatePlaceHolder( size_t nIdx ) const;
    String              CreatePlaceHolder() const;

    void                WriteTokenAddressItem( const sal_Char* pcName, const FormulaAddress& rPos, bool bNameMode );
    void                WriteTokenAddress3dItem( const sal_Char* pcName, const String& rRef, const FormulaAddress& rPos, bool bNameMode );
    void                WriteTokenRangeItem( const sal_Char* pcName, const FormulaRange& rRange, bool bNameMode );
    void                WriteTokenRange3dItem( const sal_Char* pcName, const String& rRef, const FormulaRange& rRange, bool bNameMode );

    sal_uInt16          DumpTokenFuncIdx();
    sal_uInt16          DumpTokenCol( const sal_Char* pcName, bool& rbRelC, bool& rbRelR );
    sal_uInt16          DumpTokenRow( const sal_Char* pcName, bool& rbRelC, bool& rbRelR );
    FormulaAddress      DumpTokenAddress( bool bNameMode );
    FormulaRange        DumpTokenRange( bool bNameMode );
    String              DumpTokenRefIdx();
    void                DumpTokenRefTabIdxs();

    void                DumpIntToken();
    void                DumpDoubleToken();
    void                DumpStringToken();
    void                DumpBoolToken();
    void                DumpErrorToken();
    void                DumpMissArgToken();

    void                DumpArrayToken( const String& rTokClass );
    void                DumpNameToken( const String& rTokClass );
    void                DumpNameXToken( const String& rTokClass );
    void                DumpRefToken( const String& rTokClass, bool bNameMode );
    void                DumpAreaToken( const String& rTokClass, bool bNameMode );
    void                DumpRefErrToken( const String& rTokClass, bool bArea );
    void                DumpRef3dToken( const String& rTokClass, bool bNameMode );
    void                DumpArea3dToken( const String& rTokClass, bool bNameMode );
    void                DumpRefErr3dToken( const String& rTokClass, bool bArea );
    void                DumpMemFuncToken( const String& rTokClass );
    void                DumpMemAreaToken( const String& rTokClass, bool bAddData );

    void                DumpExpToken( const StringWrapper& rName );
    void                DumpUnaryOpToken( const StringWrapper& rLOp, const StringWrapper& rROp );
    void                DumpBinaryOpToken( const StringWrapper& rOp );
    void                DumpFuncToken( const String& rTokClass );
    void                DumpFuncVarToken( const String& rTokClass );
    void                DumpCmdToken( const String& rTokClass );

    void                DumpSheetToken();
    void                DumpEndSheetToken();
    bool                DumpAttrToken();

    bool                DumpNlrToken();
    void                DumpNlrErrToken();
    void                DumpNlrColRowToken( const String& rTokClass, bool bAddData );
    void                DumpNlrRangeToken( const String& rTokClass, bool bAddData );
    void                DumpNlrRangeErrToken();

    void                DumpAddTokenData();
    void                DumpAddDataNlr( size_t nIdx );
    void                DumpAddDataArray( size_t nIdx );
    void                DumpAddDataMemArea( size_t nIdx );

private:
    enum AddDataType { ADDDATA_NLR, ADDDATA_ARRAY, ADDDATA_MEMAREA };

    typedef ScfRef< FormulaStack >          FormulaStackRef;
    typedef ScfRef< XclFunctionProvider >   XclFuncProvRef;
    typedef ::std::vector< AddDataType >    AddDataTypeVec;

    NameListRef         mxTokens;
    NameListRef         mxClasses;
    NameListRef         mxFuncNames;
    NameListRef         mxParamCnt;
    NameListRef         mxRelFlags;
    NameListRef         mxNlrTypes;
    NameListRef         mxAttrTypes;
    NameListRef         mxSpTypes;

    FormulaStackRef     mxStack;
    XclFuncProvRef      mxFuncProv;
    AddDataTypeVec      maAddData;
    String              maRefPrefix;
    const sal_Char*     mpcName;
    sal_uInt16          mnSize;
    bool                mbNameMode;
};

typedef ScfRef< FormulaObject > FormulaObjectRef;

// ============================================================================
// ============================================================================

class RecordStreamObject : public RootObjectBase
{
public:
    explicit            RecordStreamObject( const ObjectBase& rParent, SvStream& rStrm, XclBiff eBiff = EXC_BIFF_UNKNOWN );
    explicit            RecordStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName, XclBiff eBiff = EXC_BIFF_UNKNOWN );
    virtual             ~RecordStreamObject();

protected:
    inline explicit     RecordStreamObject() {}
    void                Construct( const ObjectBase& rParent, SvStream& rStrm, XclBiff eBiff );
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName, XclBiff eBiff );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpBody();

    virtual void        ImplPreProcessRecord();
    virtual void        ImplDumpRecord();
    virtual void        ImplPostProcessRecord();

    inline RecordHeaderObject& GetRecordHeader() const { return *mxHdrObj; }
    inline FormulaObject& GetFormulaDumper() const { return *mxFmlaObj; }
    inline DffDumpObject& GetDffDumper() const { return *mxDffObj; }

    void                DumpRepeatedRecordId();

    using               RootObjectBase::Construct;

private:
    void                ConstructRecStrmObj();

    void                DumpRecordBody();
    void                DumpSimpleRecord( const String& rRecData );

private:
    String              maProgressName;
    RecHeaderObjectRef  mxHdrObj;
    FormulaObjectRef    mxFmlaObj;
    DffDumpObjectRef    mxDffObj;
    NameListRef         mxSimpleRecs;
};

// ============================================================================

class WorkbookStreamObject : public RecordStreamObject
{
public:
    explicit            WorkbookStreamObject( const ObjectBase& rParent, SvStream& rStrm );
    explicit            WorkbookStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );
    virtual             ~WorkbookStreamObject();

protected:
    virtual void        ImplPreProcessRecord();
    virtual void        ImplDumpRecord();
    virtual void        ImplPostProcessRecord();

private:
    void                ConstructWbStrmObj();

    const XclFontData*  GetFontData( sal_uInt16 nFontIdx ) const;
    sal_uInt16          GetXfData( sal_uInt16 nXfIdx ) const;
    rtl_TextEncoding    GetFontEncoding( sal_uInt16 nXfIdx ) const;
    String              CreateFontName( const XclFontData& rFontData ) const;

    template< typename Type >
    Type                DumpPatternIdx( const sal_Char* pcName = 0 );
    template< typename Type >
    Type                DumpColorIdx( const sal_Char* pcName = 0 );

    sal_uInt16          DumpFontIdx( const sal_Char* pcName = 0 );
    sal_uInt16          DumpFormatIdx( const sal_Char* pcName = 0 );
    sal_uInt16          DumpXfIdx( const sal_Char* pcName = 0, bool bBiff2Style = false );

    sal_uInt16          DumpCellHeader( bool bBiff2Style = false );
    void                DumpBoolErr();

    void                DumpFontRec();
    void                DumpFormatRec();
    void                DumpXfRec();

    void                DumpObjRec();
    void                DumpObjRec5();
    void                DumpObjRec8();

private:
    typedef ::std::vector< XclFontData > XclFontDataVec;

    NameListRef         mxColors;
    NameListRef         mxBorderStyles;
    NameListRef         mxFillPatterns;
    NameListRef         mxFontNames;
    NameListRef         mxFormats;
    XclFontDataVec      maFontDatas;
    ScfUInt16Vec        maXfDatas;
    sal_uInt16          mnFormatIdx;
    sal_uInt16          mnPTRowFields;
    sal_uInt16          mnPTColFields;
    sal_uInt16          mnPTSxliIdx;
    bool                mbHasCodePage;
};

// ----------------------------------------------------------------------------

template< typename Type >
Type WorkbookStreamObject::DumpPatternIdx( const sal_Char* pcName )
{
    return DumpDec< Type >( pcName ? pcName : "fill-pattern", mxFillPatterns );
}

template< typename Type >
Type WorkbookStreamObject::DumpColorIdx( const sal_Char* pcName )
{
    return DumpDec< Type >( pcName ? pcName : "color-idx", mxColors );
}

// ============================================================================

class PivotCacheStreamObject : public RecordStreamObject
{
public:
    explicit            PivotCacheStreamObject( const ObjectBase& rParent, SvStream& rStrm );
    explicit            PivotCacheStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );
    virtual             ~PivotCacheStreamObject();

protected:
    virtual void        ImplDumpRecord();
};

// ============================================================================

class VbaProjectStreamObject : public OleStreamObject
{
public:
    explicit            VbaProjectStreamObject( const OleStorageObject& rParentStrg );

protected:
    virtual void        ImplDumpBody();
};

// ============================================================================
// ============================================================================

class PivotCacheStorageObject : public OleStorageObject
{
public:
    explicit            PivotCacheStorageObject( const OleStorageObject& rParentStrg );

protected:
    virtual void        ImplDumpBody();
};

// ============================================================================

class VbaProjectStorageObject : public OleStorageObject
{
public:
    explicit            VbaProjectStorageObject( const OleStorageObject& rParentStrg );

protected:
    virtual void        ImplDumpBody();
};

// ============================================================================

class VbaStorageObject : public OleStorageObject
{
public:
    explicit            VbaStorageObject( const OleStorageObject& rParentStrg );

protected:
    virtual void        ImplDumpBody();
};

// ============================================================================

class RootStorageObject : public OleStorageObject
{
public:
    explicit            RootStorageObject( const ObjectBase& rParent );

protected:
    virtual void        ImplDumpBody();
};

// ============================================================================
// ============================================================================

class Dumper : public DumperBase
{
public:
    explicit            Dumper( SfxMedium& rMedium, SfxObjectShell* pDocShell );

protected:
    virtual void        ImplDumpBody();
};

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace dump
} // namespace scf

#endif
#endif

