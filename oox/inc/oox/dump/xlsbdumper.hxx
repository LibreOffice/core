/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlsbdumper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:53:43 $
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

#ifndef OOX_DUMP_XLSBDUMPER_HXX
#define OOX_DUMP_XLSBDUMPER_HXX

#include "oox/dump/dumperbase.hxx"
#include "oox/helper/recordinputstream.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox { namespace xls {
    class BinFontPortionList;
    class BinPhoneticPortionList;
    struct FunctionInfo;
    class FunctionProvider;
} }

namespace oox {
namespace dump {
namespace xlsb {

typedef ::boost::shared_ptr< RecordInputStream > RecordInputStreamRef;

// ============================================================================

class RecordStreamInput : public Input
{
public:
    explicit            RecordStreamInput();
    virtual             ~RecordStreamInput();

    void                createStream( const RecordDataSequence& rData );
    inline RecordInputStream& getStream() { return *mxStrm; }

    virtual sal_Int64   getSize() const;
    virtual sal_Int64   tell() const;
    virtual void        seek( sal_Int64 nPos );
    virtual void        skip( sal_Int32 nBytes );
    virtual sal_Int32   read( void* pBuffer, sal_Int32 nBytes );

    virtual RecordStreamInput& operator>>( sal_Int8& rnData );
    virtual RecordStreamInput& operator>>( sal_uInt8& rnData );
    virtual RecordStreamInput& operator>>( sal_Int16& rnData );
    virtual RecordStreamInput& operator>>( sal_uInt16& rnData );
    virtual RecordStreamInput& operator>>( sal_Int32& rnData );
    virtual RecordStreamInput& operator>>( sal_uInt32& rnData );
    virtual RecordStreamInput& operator>>( float& rfData );
    virtual RecordStreamInput& operator>>( double& rfData );

protected:
    virtual bool        implIsValid() const;

private:
    RecordInputStreamRef mxStrm;
};

typedef ::boost::shared_ptr< RecordStreamInput > RecordStreamInputRef;

// ============================================================================

class RecordObjectBase : public InputObjectBase
{
protected:
    explicit            RecordObjectBase();
    virtual             ~RecordObjectBase();

    void                construct( const OutputObjectBase& rParent );
    void                construct( const RecordObjectBase& rParent );

    virtual bool        implIsValid() const;

    void                createRecordStream( const RecordDataSequence& rData );
    inline RecordInputStream& getRecordStream() const { return mxStrmIn->getStream(); }

    ::rtl::OUString     getErrorName( sal_uInt8 nErrCode ) const;

    // ------------------------------------------------------------------------

    void                readAddress( Address& orAddress );
    void                readRange( Range& orRange );
    void                readRangeList( RangeList& orRanges );

    // ------------------------------------------------------------------------

    void                writeBooleanItem( const sal_Char* pcName, sal_uInt8 nBool );
    void                writeErrorCodeItem( const sal_Char* pcName, sal_uInt8 nErrCode );

    void                writeFontPortions( const ::oox::xls::BinFontPortionList& rPortions );
    void                writePhoneticPortions( const ::oox::xls::BinPhoneticPortionList& rPhonetics );

    // ------------------------------------------------------------------------

    sal_uInt8           dumpBoolean( const sal_Char* pcName = 0 );
    sal_uInt8           dumpErrorCode( const sal_Char* pcName = 0 );
    ::rtl::OUString     dumpString( const sal_Char* pcName = 0, bool bRich = false, bool b32BitLen = true );
    void                dumpColor( const sal_Char* pcName = 0 );

    sal_Int32           dumpColIndex( const sal_Char* pcName = 0 );
    sal_Int32           dumpRowIndex( const sal_Char* pcName = 0 );
    sal_Int32           dumpColRange( const sal_Char* pcName = 0 );
    sal_Int32           dumpRowRange( const sal_Char* pcName = 0 );

    Address             dumpAddress( const sal_Char* pcName = 0 );
    Range               dumpRange( const sal_Char* pcName = 0 );
    void                dumpRangeList( const sal_Char* pcName = 0 );

    // ------------------------------------------------------------------------

    using               InputObjectBase::construct;

private:
    void                constructRecObjBase();

private:
    RecordStreamInputRef mxStrmIn;
    NameListRef         mxErrCodes;
};

// ============================================================================

class FormulaObject : public RecordObjectBase
{
public:
    explicit            FormulaObject( const RecordObjectBase& rParent );
    virtual             ~FormulaObject();

    void                dumpCellFormula( const sal_Char* pcName = 0 );
    void                dumpNameFormula( const sal_Char* pcName = 0 );

protected:
    virtual void        implDump();

private:
    void                constructFmlaObj();

    void                dumpFormula( const sal_Char* pcName, bool bNameMode );

    TokenAddress        createTokenAddress( sal_Int32 nCol, sal_Int32 nRow, bool bRelC, bool bRelR, bool bNameMode ) const;
    ::rtl::OUString     createRef( const ::rtl::OUString& rData ) const;
    ::rtl::OUString     createName( sal_Int32 nNameId ) const;
    ::rtl::OUString     createPlaceHolder( size_t nIdx ) const;
    ::rtl::OUString     createPlaceHolder() const;

    ::rtl::OUString     writeFuncIdItem( sal_uInt16 nFuncId, const ::oox::xls::FunctionInfo** oppFuncInfo = 0 );

    sal_Int32           dumpTokenCol( const sal_Char* pcName, bool& rbRelC, bool& rbRelR );
    sal_Int32           dumpTokenRow( const sal_Char* pcName );
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

    void                dumpExpToken( const StringWrapper& rName );
    void                dumpUnaryOpToken( const StringWrapper& rLOp, const StringWrapper& rROp );
    void                dumpBinaryOpToken( const StringWrapper& rOp );
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
    const sal_Char*     mpcName;
    sal_Int32           mnSize;
    bool                mbNameMode;
};

typedef ::boost::shared_ptr< FormulaObject > FormulaObjectRef;

// ============================================================================

class RecordObject : public RecordObjectBase
{
public:
    explicit            RecordObject( OutputObjectBase& rParent );

    void                dumpRecord( const RecordDataSequence& rData, sal_Int32 nRecId );


protected:
    virtual bool        implIsValid() const;
    virtual void        implDump();

private:
    void                dumpCellHeader( bool bWithColumn );
    void                dumpSimpleRecord( const ::rtl::OUString& rRecData );
    void                dumpRecordBody();

private:
    FormulaObjectRef    mxFmlaObj;
    NameListRef         mxSimpleRecs;
    sal_Int32           mnRecId;
};

typedef ::boost::shared_ptr< RecordObject > RecordObjectRef;

// ============================================================================

class RecordHeaderObject : public RecordHeaderBase< sal_Int32, sal_Int32 >
{
public:
    explicit            RecordHeaderObject( const InputObjectBase& rParent );
    virtual             ~RecordHeaderObject();

    inline const RecordDataSequence& getRecordData() const { return maData; }

protected:
    virtual bool        implReadHeader( sal_Int64& ornRecPos, sal_Int32& ornRecId, sal_Int32& ornRecSize );

private:
    bool                readByte( sal_Int64& ornRecPos, sal_uInt8& ornByte );
    bool                readCompressedInt( sal_Int64& ornRecPos, sal_Int32& ornValue );

private:
    RecordDataSequence  maData;
};

typedef ::boost::shared_ptr< RecordHeaderObject > RecordHeaderObjectRef;

// ============================================================================

class RecordStreamObject : public InputStreamObject
{
public:
    explicit            RecordStreamObject( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, BinaryInputStreamRef xStrm );

protected:
    virtual bool        implIsValid() const;
    virtual void        implDump();

private:
    RecordHeaderObjectRef mxHdrObj;
    RecordObjectRef     mxRecObj;
};

// ============================================================================

class RootStorageObject : public RootStorageObjectBase
{
public:
    explicit            RootStorageObject( const DumperBase& rParent );

protected:
    virtual void        implDumpStream(
                            BinaryInputStreamRef xStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSystemFileName );
};

// ============================================================================

class Dumper : public DumperBase
{
public:
    explicit            Dumper( const ::oox::core::FilterBase& rFilter );

protected:
    virtual void        implDump();
};

// ============================================================================

} // namespace xlsb
} // namespace dump
} // namespace oox

#endif
#endif

