/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef OOX_DUMP_BIFFDUMPER_HXX
#define OOX_DUMP_BIFFDUMPER_HXX

#include "oox/dump/dumperbase.hxx"
#include "oox/dump/dffdumper.hxx"
#include "oox/dump/oledumper.hxx"
#include "oox/xls/richstring.hxx"
#include "oox/xls/biffinputstream.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox { namespace xls {
    class BiffInputStream;
    class FontPortionModelList;
    struct FunctionInfo;
    class FunctionProvider;
} }

namespace oox {
namespace dump {
namespace biff {

typedef ::boost::shared_ptr< ::oox::xls::BiffInputStream > BiffInputStreamRef;

// ============================================================================
// ============================================================================

class BiffDffStreamObject : public DffStreamObject
{
public:
    explicit            BiffDffStreamObject(
                            const OutputObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm );

protected:
    virtual void        implDumpClientAnchor();
};

// ============================================================================

class BiffCtlsStreamObject : public InputObjectBase
{
public:
    explicit            BiffCtlsStreamObject( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm );

    void                dumpControl( sal_uInt32 nStartPos, sal_uInt32 nLength );

protected:
    virtual void        implDump();

private:
    sal_uInt32          mnStartPos;
    sal_uInt32          mnLength;
};

// ============================================================================
// ============================================================================

class BiffConfig : public Config
{
public:
    explicit            BiffConfig( const Config& rParent, ::oox::xls::BiffType eBiff );

protected:
    virtual bool        implIsValid() const;
    virtual NameListRef implGetNameList( const ::rtl::OUString& rKey ) const;

private:
    ::oox::xls::BiffType meBiff;
};

// ============================================================================

class BiffSharedData : public Base
{
public:
    explicit            BiffSharedData( ::oox::xls::BiffType eBiff );

    void                initializePerSheet();

    inline ::oox::xls::BiffType getBiff() const { return meBiff; }

    inline rtl_TextEncoding getTextEncoding() const { return meTextEnc; }
    void                setTextEncoding( rtl_TextEncoding eTextEnc );

    sal_uInt16          getFontCount() const;
    rtl_TextEncoding    getFontEncoding( sal_uInt16 nFontId ) const;
    void                appendFontEncoding( rtl_TextEncoding eFontEnc );

    sal_uInt16          getXfCount() const;
    rtl_TextEncoding    getXfEncoding( sal_uInt16 nXfId ) const;
    void                appendXfFontId( sal_uInt16 nFontId );

protected:
    virtual bool        implIsValid() const;

private:
    typedef ::std::vector< rtl_TextEncoding >   TextEncVec;
    typedef ::std::vector< sal_uInt16 >         FontIdVec;

    TextEncVec          maFontEncs;
    FontIdVec           maXfFontIds;
    ::oox::xls::BiffType meBiff;
    rtl_TextEncoding    meTextEnc;
};

// ============================================================================

class BiffObjectBase : public RecordObjectBase
{
public:
    inline BiffSharedData& getBiffData() const { return *mxBiffData; }
    inline ::oox::xls::BiffInputStream& getBiffStream() const { return *mxBiffStrm; }
    inline ::oox::xls::BiffType getBiff() const { return mxBiffData->getBiff(); }

protected:
    inline explicit     BiffObjectBase() {}
    virtual             ~BiffObjectBase();

    using               InputObjectBase::construct;
    void                construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, ::oox::xls::BiffType eBiff, const ::rtl::OUString& rSysFileName );
    void                construct( const BiffObjectBase& rParent );

    virtual bool        implIsValid() const;
    virtual bool        implStartRecord( BinaryInputStream& rBaseStrm, sal_Int64& ornRecPos, sal_Int64& ornRecId, sal_Int64& ornRecSize );

    inline sal_uInt16   getLastRecId() const { return mnLastRecId; }
    ::rtl::OUString     getErrorName( sal_uInt8 nErrCode ) const;

    // ------------------------------------------------------------------------

    sal_Int32           readCol( bool bCol16Bit );
    sal_Int32           readRow( bool bRow32Bit );
    void                readAddress( Address& orAddress, bool bCol16Bit = true, bool bRow32Bit = false );
    void                readRange( Range& orRange, bool bCol16Bit = true, bool bRow32Bit = false );
    void                readRangeList( RangeList& orRanges, bool bCol16Bit = true, bool bRow32Bit = false );

    // ------------------------------------------------------------------------

    void                writeBooleanItem( const String& rName, sal_uInt8 nBool );
    void                writeErrorCodeItem( const String& rName, sal_uInt8 nErrCode );

    void                writeFontPortions( const ::oox::xls::FontPortionModelList& rPortions );

    template< typename Type >
    void                writeRectItem( const String& rName,
                            Type nLeft, Type nTop, Type nWidth, Type nHeight,
                            const NameListWrapper& rListWrp = NO_LIST,
                            FormatType eFmtType = FORMATTYPE_DEC );

    // ------------------------------------------------------------------------

    ::rtl::OUString     dumpByteString(
                            const String& rName,
                            ::oox::xls::BiffStringFlags nFlags = ::oox::xls::BIFF_STR_DEFAULT,
                            rtl_TextEncoding eDefaultTextEnc = RTL_TEXTENCODING_DONTKNOW );
    ::rtl::OUString     dumpUniString(
                            const String& rName,
                            ::oox::xls::BiffStringFlags nFlags = ::oox::xls::BIFF_STR_DEFAULT );
    ::rtl::OUString     dumpString(
                            const String& rName,
                            ::oox::xls::BiffStringFlags nByteFlags = ::oox::xls::BIFF_STR_DEFAULT,
                            ::oox::xls::BiffStringFlags nUniFlags = ::oox::xls::BIFF_STR_DEFAULT,
                            rtl_TextEncoding eDefaultTextEnc = RTL_TEXTENCODING_DONTKNOW );

    sal_uInt8           dumpBoolean( const String& rName = EMPTY_STRING );
    sal_uInt8           dumpErrorCode( const String& rName = EMPTY_STRING );

    rtl_TextEncoding    dumpCodePage( const String& rName = EMPTY_STRING );
    void                dumpFormulaResult( const String& rName = EMPTY_STRING );

    sal_Int32           dumpColIndex( const String& rName = EMPTY_STRING, bool bCol16Bit = true );
    sal_Int32           dumpRowIndex( const String& rName = EMPTY_STRING, bool bRow32Bit = false );
    sal_Int32           dumpColRange( const String& rName = EMPTY_STRING, bool bCol16Bit = true );
    sal_Int32           dumpRowRange( const String& rName = EMPTY_STRING, bool bRow32Bit = false );

    Address             dumpAddress( const String& rName = EMPTY_STRING, bool bCol16Bit = true, bool bRow32Bit = false );
    Range               dumpRange( const String& rName = EMPTY_STRING, bool bCol16Bit = true, bool bRow32Bit = false );
    void                dumpRangeList( const String& rName = EMPTY_STRING, bool bCol16Bit = true, bool bRow32Bit = false );

    void                dumpConstArrayHeader( sal_uInt32& rnCols, sal_uInt32& rnRows );
    ::rtl::OUString     dumpConstValue( sal_Unicode cStrQuote = OOX_DUMP_STRQUOTE );

    template< typename Type >
    void                dumpRect( const String& rName,
                            const NameListWrapper& rListWrp = NO_LIST,
                            FormatType eFmtType = FORMATTYPE_DEC );
    template< typename Type >
    void                dumpRectWithGaps( const String& rName, sal_Int32 nGap,
                            const NameListWrapper& rListWrp = NO_LIST,
                            FormatType eFmtType = FORMATTYPE_DEC );

    sal_uInt16          dumpRepeatedRecId();
    void                dumpFrHeader( bool bWithFlags, bool bWithRange );

    void                dumpDffClientRect();
    void                dumpEmbeddedDff();
    void                dumpControl();

private:
    typedef ::boost::shared_ptr< BiffSharedData >       BiffSharedDataRef;
    typedef ::boost::shared_ptr< BiffDffStreamObject >  BiffDffStreamObjRef;
    typedef ::boost::shared_ptr< BiffCtlsStreamObject > BiffCtlsStrmObjRef;

    BiffSharedDataRef   mxBiffData;
    BiffInputStreamRef  mxBiffStrm;
    BiffDffStreamObjRef mxDffObj;
    BiffCtlsStrmObjRef  mxCtlsObj;
    NameListRef         mxErrCodes;
    NameListRef         mxConstType;
    NameListRef         mxResultType;
    sal_uInt16          mnLastRecId;
    bool                mbMergeContRec;
};

// ----------------------------------------------------------------------------

template< typename Type >
void BiffObjectBase::writeRectItem( const String& rName,
        Type nLeft, Type nTop, Type nWidth, Type nHeight,
        const NameListWrapper& rListWrp, FormatType eFmtType )
{
    MultiItemsGuard aMultiGuard( mxOut );
    writeEmptyItem( rName );
    writeValueItem( "x-pos", nLeft, eFmtType, rListWrp );
    writeValueItem( "y-pos", nTop, eFmtType, rListWrp );
    writeValueItem( "x-size", nWidth, eFmtType, rListWrp );
    writeValueItem( "y-size", nHeight, eFmtType, rListWrp );
}

template< typename Type >
void BiffObjectBase::dumpRect( const String& rName,
        const NameListWrapper& rListWrp, FormatType eFmtType )
{
    Type nLeft, nTop, nWidth, nHeight;
    *mxBiffStrm >> nLeft >> nTop >> nWidth >> nHeight;
    writeRectItem( rName, nLeft, nTop, nWidth, nHeight, rListWrp, eFmtType );
}

template< typename Type >
void BiffObjectBase::dumpRectWithGaps( const String& rName, sal_Int32 nGap,
        const NameListWrapper& rListWrp, FormatType eFmtType )
{
    Type nLeft, nTop, nWidth, nHeight;
    *mxBiffStrm >> nLeft;
    mxBiffStrm->skip( nGap );
    *mxBiffStrm >> nTop;
    mxBiffStrm->skip( nGap );
    *mxBiffStrm >> nWidth;
    mxBiffStrm->skip( nGap );
    *mxBiffStrm >> nHeight;
    mxBiffStrm->skip( nGap );
    writeRectItem( rName, nLeft, nTop, nWidth, nHeight, rListWrp, eFmtType );
}

// ============================================================================
// ============================================================================

class FormulaObject : public BiffObjectBase
{
public:
    explicit            FormulaObject( const BiffObjectBase& rParent );
    virtual             ~FormulaObject();

    sal_uInt16          readFormulaSize();
    sal_uInt16          dumpFormulaSize( const String& rName = EMPTY_STRING );

    void                dumpCellFormula( const String& rName, sal_uInt16 nSize );
    void                dumpCellFormula( const String& rName = EMPTY_STRING );
    void                dumpNameFormula( const String& rName, sal_uInt16 nSize );
    void                dumpNameFormula( const String& rName = EMPTY_STRING );

protected:
    virtual void        implDump();

private:
    void                constructFmlaObj();

    void                dumpFormula( const String& rName, sal_uInt16 nSize, bool bNameMode );
    void                dumpFormula( const String& rName, bool bNameMode );

    TokenAddress        createTokenAddress( sal_uInt16 nCol, sal_uInt16 nRow, bool bRelC, bool bRelR, bool bNameMode ) const;
    ::rtl::OUString     createRef( const ::rtl::OUString& rData ) const;
    ::rtl::OUString     createName( sal_uInt16 nNameIdx ) const;
    ::rtl::OUString     createPlaceHolder( size_t nIdx ) const;
    ::rtl::OUString     createPlaceHolder() const;

    sal_uInt16          readFuncId();
    ::rtl::OUString     writeFuncIdItem( sal_uInt16 nFuncId, const ::oox::xls::FunctionInfo** oppFuncInfo = 0 );

    sal_uInt16          dumpTokenCol( const String& rName, bool& rbRelC, bool& rbRelR );
    sal_uInt16          dumpTokenRow( const String& rName, bool& rbRelC, bool& rbRelR );
    TokenAddress        dumpTokenAddress( bool bNameMode );
    TokenRange          dumpTokenRange( bool bNameMode );

    sal_Int16           readTokenRefIdx();
    ::rtl::OUString     dumpTokenRefIdx();
    ::rtl::OUString     dumpTokenRefTabIdxs();

    void                dumpIntToken();
    void                dumpDoubleToken();
    void                dumpStringToken();
    void                dumpBoolToken();
    void                dumpErrorToken();
    void                dumpMissArgToken();

    void                dumpArrayToken( const ::rtl::OUString& rTokClass );
    void                dumpNameToken( const ::rtl::OUString& rTokClass );
    void                dumpNameXToken( const ::rtl::OUString& rTokClass );
    void                dumpRefToken( const ::rtl::OUString& rTokClass, bool bNameMode );
    void                dumpAreaToken( const ::rtl::OUString& rTokClass, bool bNameMode );
    void                dumpRefErrToken( const ::rtl::OUString& rTokClass, bool bArea );
    void                dumpRef3dToken( const ::rtl::OUString& rTokClass, bool bNameMode );
    void                dumpArea3dToken( const ::rtl::OUString& rTokClass, bool bNameMode );
    void                dumpRefErr3dToken( const ::rtl::OUString& rTokClass, bool bArea );
    void                dumpMemFuncToken( const ::rtl::OUString& rTokClass );
    void                dumpMemAreaToken( const ::rtl::OUString& rTokClass, bool bAddData );

    void                dumpExpToken( const String& rName );
    void                dumpUnaryOpToken( const String& rLOp, const String& rROp );
    void                dumpBinaryOpToken( const String& rOp );
    void                dumpFuncToken( const ::rtl::OUString& rTokClass );
    void                dumpFuncVarToken( const ::rtl::OUString& rTokClass );
    void                dumpCmdToken( const ::rtl::OUString& rTokClass );

    void                dumpSheetToken();
    void                dumpEndSheetToken();
    bool                dumpAttrToken();

    bool                dumpNlrToken();
    void                dumpNlrErrToken();
    void                dumpNlrColRowToken( const ::rtl::OUString& rTokClass, bool bAddData );
    void                dumpNlrRangeToken( const ::rtl::OUString& rTokClass, bool bAddData );
    void                dumpNlrRangeErrToken();

    void                dumpAddTokenData();
    void                dumpAddDataNlr( size_t nIdx );
    void                dumpAddDataArray( size_t nIdx );
    void                dumpAddDataMemArea( size_t nIdx );

private:
    enum AddDataType { ADDDATA_NLR, ADDDATA_ARRAY, ADDDATA_MEMAREA };

    typedef ::boost::shared_ptr< FormulaStack >                 FormulaStackRef;
    typedef ::boost::shared_ptr< ::oox::xls::FunctionProvider > FuncProvRef;
    typedef ::std::vector< AddDataType >                        AddDataTypeVec;

    NameListRef         mxTokens;
    NameListRef         mxClasses;
    NameListRef         mxRelFlags;
    NameListRef         mxNlrTypes;
    NameListRef         mxAttrTypes;
    NameListRef         mxSpTypes;
    sal_Int32           mnColCount;
    sal_Int32           mnRowCount;

    FormulaStackRef     mxStack;
    FuncProvRef         mxFuncProv;
    AddDataTypeVec      maAddData;
    ::rtl::OUString     maRefPrefix;
    ::rtl::OUString     maName;
    sal_uInt16          mnSize;
    bool                mbNameMode;
};

// ============================================================================
// ============================================================================

class RecordStreamObject : public BiffObjectBase
{
protected:
    inline explicit     RecordStreamObject() {}
    virtual             ~RecordStreamObject();

    using               BiffObjectBase::construct;
    void                construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, ::oox::xls::BiffType eBiff, const ::rtl::OUString& rSysFileName );

    virtual bool        implIsValid() const;

    inline FormulaObject& getFormulaDumper() const { return *mxFmlaObj; }

private:
    typedef ::boost::shared_ptr< FormulaObject > FormulaObjectRef;
    FormulaObjectRef    mxFmlaObj;
};

// ============================================================================

class WorkbookStreamObject : public RecordStreamObject
{
public:
    explicit            WorkbookStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const ::rtl::OUString& rSysFileName );
    virtual             ~WorkbookStreamObject();

protected:
    virtual void        implDumpRecordBody();

private:
    void                initializePerSheet();

    ::rtl::OUString     createFontName( const ::rtl::OUString& rName, sal_uInt16 nHeight, bool bBold, bool bItalic ) const;

    sal_uInt16          dumpPatternIdx( const String& rName = EMPTY_STRING, bool b16Bit = true );
    sal_uInt16          dumpColorIdx( const String& rName = EMPTY_STRING, bool b16Bit = true );
    sal_uInt16          dumpFontIdx( const String& rName = EMPTY_STRING, bool b16Bit = true );
    sal_uInt16          dumpFormatIdx( const String& rName = EMPTY_STRING );
    sal_uInt16          dumpXfIdx( const String& rName = EMPTY_STRING, bool bBiff2Style = false );

    template< typename Type >
    inline Type         dumpExtColorType() { return dumpDec< Type >( "color-type", "EXTCOLOR-TYPE" ); }
    void                dumpExtColorValue( sal_uInt32 nColorType );
    void                dumpExtColor( const String& rName = EMPTY_STRING );
    void                dumpExtCfColor( const String& rName = EMPTY_STRING );
    void                dumpExtGradientHead();

    ::rtl::OUString     dumpPivotString( const String& rName, sal_uInt16 nStrLen );
    ::rtl::OUString     dumpPivotString( const String& rName );

    sal_uInt16          dumpCellHeader( bool bBiff2Style = false );
    void                dumpBoolErr();

    void                dumpCfRuleProp();
    void                dumpXfExtProp();
    void                dumpDxfProp();
    void                dumpDxf12Prop();
    void                dumpCfRule12Param( sal_uInt16 nSubType );

    void                dumpFontRec();
    void                dumpFormatRec();
    void                dumpXfRec();

    void                dumpObjRec();
    void                dumpObjRecBiff3();
    void                dumpObjRecBiff4();
    void                dumpObjRecBiff5();
    void                dumpObjRecBiff8();

    void                dumpObjRecLineData();
    void                dumpObjRecFillData();
    void                dumpObjRecRectData();
    void                dumpObjRecTextDataBiff3( sal_uInt16& ornTextLen, sal_uInt16& ornFormatSize );
    void                dumpObjRecTextDataBiff5( sal_uInt16& ornTextLen, sal_uInt16& ornFormatSize, sal_uInt16& ornLinkSize );
    void                dumpObjRecSbsData();
    void                dumpObjRecGboData();
    void                dumpObjRecEdoData();
    void                dumpObjRecRboData();
    void                dumpObjRecCblsData();
    void                dumpObjRecLbsData();

    void                dumpObjRecPadding();
    void                dumpObjRecString( const String& rName, sal_uInt16 nTextLen, bool bRepeatLen );
    void                dumpObjRecTextFmt( sal_uInt16 nFormatSize );
    void                dumpObjRecFmlaRaw();
    void                dumpObjRecFmla( const String& rName, sal_uInt16 nFmlaSize );
    void                dumpObjRecPictFmla( sal_uInt16 nFmlaSize );

    typedef ::std::pair< sal_uInt8, ::rtl::OUString > ChFrExtPropInfo;

    void                dumpChFrExtProps();
    ChFrExtPropInfo     dumpChFrExtPropHeader();

private:
    NameListRef         mxColors;
    NameListRef         mxBorderStyles;
    NameListRef         mxFillPatterns;
    NameListRef         mxFontNames;
    NameListRef         mxFormats;
    sal_uInt16          mnFormatIdx;
    sal_uInt16          mnPTRowFields;
    sal_uInt16          mnPTColFields;
    sal_uInt16          mnPTRowColItemsIdx;
    bool                mbHasCodePage;
    bool                mbHasDff;
};

// ============================================================================

class PivotCacheStreamObject : public RecordStreamObject
{
public:
    explicit            PivotCacheStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            ::oox::xls::BiffType eBiff,
                            const ::rtl::OUString& rSysFileName );

protected:
    virtual void        implDumpRecordBody();
};

// ============================================================================
// ============================================================================

class RootStorageObject : public OleStorageObject
{
public:
    explicit            RootStorageObject( const DumperBase& rParent );

protected:
    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );

    virtual void        implDumpStorage(
                            const StorageRef& rxStrg,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rSysPath );

    virtual void        implDumpBaseStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName );
};

// ============================================================================
// ============================================================================

class Dumper : public DumperBase
{
public:
    explicit            Dumper( const ::oox::core::FilterBase& rFilter );

    explicit            Dumper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            const ::rtl::OUString& rSysFileName );

protected:
    virtual void        implDump();
};

// ============================================================================
// ============================================================================

} // namespace biff
} // namespace dump
} // namespace oox

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
