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

#ifndef OOX_DUMP_XLSBDUMPER_HXX
#define OOX_DUMP_XLSBDUMPER_HXX

#include "oox/dump/dumperbase.hxx"
#include "oox/helper/recordinputstream.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox { namespace xls {
    class FontPortionModelList;
    class PhoneticPortionModelList;
    struct FunctionInfo;
    class FunctionProvider;
} }

namespace oox {
namespace dump {
namespace xlsb {

// ============================================================================

class RecordObjectBase : public SequenceRecordObjectBase
{
protected:
    explicit            RecordObjectBase();
    virtual             ~RecordObjectBase();

    using               SequenceRecordObjectBase::construct;
    void                construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const ::rtl::OUString& rSysFileName );
    void                construct( const RecordObjectBase& rParent );

    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize );

    ::rtl::OUString     getErrorName( sal_uInt8 nErrCode ) const;

    // ------------------------------------------------------------------------

    void                readAddress( Address& orAddress );
    void                readRange( Range& orRange );
    void                readRangeList( RangeList& orRanges );

    // ------------------------------------------------------------------------

    void                writeBooleanItem( const String& rName, sal_uInt8 nBool );
    void                writeErrorCodeItem( const String& rName, sal_uInt8 nErrCode );

    void                writeFontPortions( const ::oox::xls::FontPortionModelList& rPortions );
    void                writePhoneticPortions( const ::oox::xls::PhoneticPortionModelList& rPhonetics );

    // ------------------------------------------------------------------------

    sal_uInt8           dumpBoolean( const String& rName = EMPTY_STRING );
    sal_uInt8           dumpErrorCode( const String& rName = EMPTY_STRING );
    ::rtl::OUString     dumpString( const String& rName = EMPTY_STRING, bool bRich = false, bool b32BitLen = true );
    void                dumpColor( const String& rName = EMPTY_STRING );
    ::com::sun::star::util::DateTime dumpPivotDateTime( const String& rName = EMPTY_STRING );

    sal_Int32           dumpColIndex( const String& rName = EMPTY_STRING );
    sal_Int32           dumpRowIndex( const String& rName = EMPTY_STRING );
    sal_Int32           dumpColRange( const String& rName = EMPTY_STRING );
    sal_Int32           dumpRowRange( const String& rName = EMPTY_STRING );

    Address             dumpAddress( const String& rName = EMPTY_STRING );
    Range               dumpRange( const String& rName = EMPTY_STRING );
    void                dumpRangeList( const String& rName = EMPTY_STRING );

    // ------------------------------------------------------------------------
private:
    bool                readCompressedInt( BinaryInputStream& rStrm, sal_Int32& ornValue );

private:
    typedef ::boost::shared_ptr< RecordInputStream > RecordInputStreamRef;

    RecordInputStreamRef mxStrm;
    NameListRef         mxErrCodes;
};

// ============================================================================

class FormulaObject : public RecordObjectBase
{
public:
    explicit            FormulaObject( const RecordObjectBase& rParent );
    virtual             ~FormulaObject();

    void                dumpCellFormula( const String& rName = EMPTY_STRING );
    void                dumpNameFormula( const String& rName = EMPTY_STRING );

protected:
    virtual void        implDump();

private:
    void                constructFmlaObj();

    void                dumpFormula( const String& rName, bool bNameMode );

    TokenAddress        createTokenAddress( sal_Int32 nCol, sal_Int32 nRow, bool bRelC, bool bRelR, bool bNameMode ) const;
    ::rtl::OUString     createRef( const ::rtl::OUString& rData ) const;
    ::rtl::OUString     createName( sal_Int32 nNameId ) const;
    ::rtl::OUString     createPlaceHolder( size_t nIdx ) const;
    ::rtl::OUString     createPlaceHolder() const;

    ::rtl::OUString     writeFuncIdItem( sal_uInt16 nFuncId, const ::oox::xls::FunctionInfo** oppFuncInfo = 0 );

    sal_Int32           dumpTokenCol( const String& rName, bool& rbRelC, bool& rbRelR );
    sal_Int32           dumpTokenRow( const String& rName );
    TokenAddress        dumpTokenAddress( bool bNameMode );
    TokenRange          dumpTokenRange( bool bNameMode );

    sal_Int16           readTokenRefId();
    ::rtl::OUString     dumpTokenRefId();

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
    bool                dumpTableToken();
    bool                dumpAttrToken();

    void                dumpAddTokenData();
    void                dumpAddDataExp( size_t nIdx );
    void                dumpAddDataArray( size_t nIdx );
    void                dumpAddDataMemArea( size_t nIdx );

    void                dumpaddDataArrayHeader( sal_Int32& rnCols, sal_Int32& rnRows );
    ::rtl::OUString     dumpaddDataArrayValue();

private:
    enum AddDataType { ADDDATA_EXP, ADDDATA_ARRAY, ADDDATA_MEMAREA };

    typedef ::boost::shared_ptr< FormulaStack >                 FormulaStackRef;
    typedef ::boost::shared_ptr< ::oox::xls::FunctionProvider > FuncProvRef;
    typedef ::std::vector< AddDataType >                        AddDataTypeVec;

    NameListRef         mxTokens;
    NameListRef         mxClasses;
    NameListRef         mxRelFlags;
    NameListRef         mxAttrTypes;
    NameListRef         mxSpTypes;
    sal_Int32           mnColCount;
    sal_Int32           mnRowCount;

    FormulaStackRef     mxStack;
    FuncProvRef         mxFuncProv;
    AddDataTypeVec      maAddData;
    ::rtl::OUString     maRefPrefix;
    ::rtl::OUString     maName;
    sal_Int32           mnSize;
    bool                mbNameMode;
};

// ============================================================================

class RecordStreamObject : public RecordObjectBase
{
public:
    explicit            RecordStreamObject( ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const ::rtl::OUString& rSysFileName );

protected:
    virtual bool        implIsValid() const;
    virtual void        implDumpRecordBody();

private:
    void                dumpGradientHead();
    void                dumpCellHeader( bool bWithColumn );

private:
    typedef ::boost::shared_ptr< FormulaObject > FormulaObjectRef;

    FormulaObjectRef    mxFmlaObj;
};

// ============================================================================

class RootStorageObject : public StorageObjectBase
{
public:
    explicit            RootStorageObject( const DumperBase& rParent );

protected:
    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );
};

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

} // namespace xlsb
} // namespace dump
} // namespace oox

#endif
#endif

