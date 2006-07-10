/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xldumper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:03:43 $
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
#define SC_XLDUMPER_HXX

#ifndef SC_FDUMPER_HXX
#include "fdumper.hxx"
#endif

#if SCF_INCL_DUMPER

#include <stack>

#ifndef SC_FDUMPEROLE_HXX
#include "fdumperole.hxx"
#endif
#ifndef SC_XLCONST_HXX
#include "xlconst.hxx"
#endif
#ifndef SC_XLSTRING_HXX
#include "xlstring.hxx"
#endif
#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif

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
    void                Construct( const ObjectBase& rParent, SvStream& rStrm );
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName );
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
private:
    void                ConstructOwn();

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

    inline bool         Check( bool bCond ) { return !(mbError |= !bCond); }

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
    void                DumpFormula( const sal_Char* pcName, sal_uInt16 nSize, bool bNameMode );
    void                DumpFormula( const sal_Char* pcName, bool bNameMode );

protected:
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpBody();

private:
    void                ConstructOwn();

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
    explicit            RecordStreamObject( const ObjectBase& rParent, SvStream& rStrm );
    explicit            RecordStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );
    virtual             ~RecordStreamObject();

protected:
    inline explicit     RecordStreamObject() {}
    void                Construct( const ObjectBase& rParent, SvStream& rStrm );
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpBody();

    virtual void        ImplPreProcessRecord();
    virtual void        ImplDumpRecord();
    virtual void        ImplPostProcessRecord();

    inline bool         IsMergeContRec() const { return mbMergeContRec; }

    void                DumpRepeatedRecordId();

    sal_uInt16          DumpFormulaSize( const sal_Char* pcName = 0 );
    void                DumpCellFormula( const sal_Char* pcName, sal_uInt16 nSize );
    void                DumpCellFormula( const sal_Char* pcName = 0 );
    void                DumpNameFormula( const sal_Char* pcName, sal_uInt16 nSize );
    void                DumpNameFormula( const sal_Char* pcName = 0 );

private:
    void                ConstructOwn();

    void                DumpRecordHeader();
    void                DumpRecord();
    void                DumpSimpleRecord( const String& rRecData );

private:
    String              maProgressName;
    FormulaObjectRef    mxFmlaObj;
    NameListRef         mxRecNames;
    NameListRef         mxSimpleRecs;
    bool                mbShowRecPos;
    bool                mbShowRecSize;
    bool                mbShowRecId;
    bool                mbShowRecName;
    bool                mbShowRecBody;
    bool                mbMergeContRec;
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
    void                ConstructOwn();

    const XclFontData*  GetFontData( sal_uInt16 nFontIdx ) const;
    sal_uInt16          GetXfData( sal_uInt16 nXfIdx ) const;
    rtl_TextEncoding    GetFontEncoding( sal_uInt16 nXfIdx ) const;
    String              CreateFontName( const XclFontData& rFontData ) const;

    sal_uInt16          DumpPatternIdx( const sal_Char* pcName = 0 );
    sal_uInt16          DumpColorIdx( const sal_Char* pcName = 0 );
    sal_uInt16          DumpFontIdx( const sal_Char* pcName = 0 );
    sal_uInt16          DumpFormatIdx( const sal_Char* pcName = 0 );
    sal_uInt16          DumpXfIdx( const sal_Char* pcName = 0, bool bBiff2Style = false );

    sal_uInt16          DumpCellHeader( bool bBiff2Style = false );
    void                DumpBoolErr();

    void                DumpCodePageRec();
    void                DumpFontRec();
    void                DumpFormatRec();
    void                DumpXfRec();

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

class VbaProjectStorageObject : public OleStorageObject
{
public:
    explicit            VbaProjectStorageObject( const OleStorageObject& rParentStrg );

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

