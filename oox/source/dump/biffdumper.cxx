/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: biffdumper.cxx,v $
 * $Revision: 1.5.20.26 $
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

#include "oox/dump/biffdumper.hxx"

#include <osl/thread.h>
#include <rtl/tencinfo.h>
#include "oox/dump/oledumper.hxx"
#include "oox/helper/olestorage.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffdetector.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/formulabase.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::io::XInputStream;
using ::oox::core::FilterBase;

using namespace ::oox::xls;

namespace oox {
namespace dump {
namespace biff {

namespace  {

// constants ------------------------------------------------------------------

const sal_uInt16 BIFF_FONTFLAG_BOLD         = 0x0001;
const sal_uInt16 BIFF_FONTFLAG_ITALIC       = 0x0002;

const sal_uInt16 BIFF_OBJTYPE_GROUP         = 0;
const sal_uInt16 BIFF_OBJTYPE_LINE          = 1;
const sal_uInt16 BIFF_OBJTYPE_RECTANGLE     = 2;
const sal_uInt16 BIFF_OBJTYPE_OVAL          = 3;
const sal_uInt16 BIFF_OBJTYPE_ARC           = 4;
const sal_uInt16 BIFF_OBJTYPE_CHART         = 5;
const sal_uInt16 BIFF_OBJTYPE_TEXT          = 6;
const sal_uInt16 BIFF_OBJTYPE_BUTTON        = 7;
const sal_uInt16 BIFF_OBJTYPE_PICTURE       = 8;
const sal_uInt16 BIFF_OBJTYPE_POLYGON       = 9;
const sal_uInt16 BIFF_OBJTYPE_CHECKBOX      = 11;
const sal_uInt16 BIFF_OBJTYPE_OPTIONBUTTON  = 12;
const sal_uInt16 BIFF_OBJTYPE_EDIT          = 13;
const sal_uInt16 BIFF_OBJTYPE_LABEL         = 14;
const sal_uInt16 BIFF_OBJTYPE_DIALOG        = 15;
const sal_uInt16 BIFF_OBJTYPE_SPIN          = 16;
const sal_uInt16 BIFF_OBJTYPE_SCROLLBAR     = 17;
const sal_uInt16 BIFF_OBJTYPE_LISTBOX       = 18;
const sal_uInt16 BIFF_OBJTYPE_GROUPBOX      = 19;
const sal_uInt16 BIFF_OBJTYPE_DROPDOWN      = 20;
const sal_uInt16 BIFF_OBJTYPE_NOTE          = 25;
const sal_uInt16 BIFF_OBJTYPE_DRAWING       = 30;

const sal_uInt16 BIFF_OBJFLAGS_CONTROL      = 0x0010;   /// Form control.
const sal_uInt16 BIFF_OBJFLAGS_CTLSSTREAM   = 0x0020;   /// Data in Ctls stream.

const sal_uInt16 BIFF_STYLE_BUILTIN         = 0x8000;

const sal_uInt16 BIFF_PT_NOSTRING           = 0xFFFF;

} // namespace

// ============================================================================
// ============================================================================

namespace {

void lclDumpDffClientPos( Output& rOut, BinaryInputStream& rStrm, const String& rName, sal_uInt16 nSubScale )
{
    MultiItemsGuard aMultiGuard( rOut );
    TableGuard aTabGuard( rOut, 17 );
    {
        sal_uInt16 nPos = rStrm.readuInt16();
        ItemGuard aItem( rOut, rName );
        rOut.writeDec( nPos );
    }
    {
        sal_uInt16 nSubUnits = rStrm.readuInt16();
        ItemGuard aItem( rOut, "sub-units" );
        rOut.writeDec( nSubUnits );
        rOut.writeChar( '/' );
        rOut.writeDec( nSubScale );
    }
}

void lclDumpDffClientRect( Output& rOut, BinaryInputStream& rStrm )
{
    lclDumpDffClientPos( rOut, rStrm, "start-col", 1024 );
    lclDumpDffClientPos( rOut, rStrm, "start-row", 256 );
    lclDumpDffClientPos( rOut, rStrm, "end-col", 1024 );
    lclDumpDffClientPos( rOut, rStrm, "end-row", 256 );
}

} // namespace

// ----------------------------------------------------------------------------

BiffDffStreamObject::BiffDffStreamObject( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm )
{
    DffStreamObject::construct( rParent, rxStrm );
}

void BiffDffStreamObject::implDumpClientAnchor()
{
    dumpHex< sal_uInt16 >( "flags", "DFF-CLIENTANCHOR-FLAGS" );
    lclDumpDffClientRect( out(), in() );
}

// ============================================================================

BiffCtlsStreamObject::BiffCtlsStreamObject( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm )
{
    InputObjectBase::construct( rParent, rxStrm );
    mnStartPos = mnLength = 0;
}

void BiffCtlsStreamObject::dumpControl( sal_uInt32 nStartPos, sal_uInt32 nLength )
{
    mnStartPos = nStartPos;
    mnLength = nLength;
    dump();
    mnStartPos = mnLength = 0;
}

void BiffCtlsStreamObject::implDump()
{
    if( mnLength > 0 )
    {
        out().emptyLine();
        writeEmptyItem( "CTLS-START" );
        {
            IndentGuard aIndGuard( out() );
            in().seek( mnStartPos );
            OcxGuidControlObject( *this, mnLength ).dump();
        }
        writeEmptyItem( "CTLS-END" );
        out().emptyLine();
    }
}

// ============================================================================
// ============================================================================

BiffConfig::BiffConfig( const Config& rParent, BiffType eBiff ) :
    meBiff( eBiff )
{
    Config::construct( rParent );
}

bool BiffConfig::implIsValid() const
{
    return (meBiff != BIFF_UNKNOWN) && Config::implIsValid();
}

NameListRef BiffConfig::implGetNameList( const OUString& rKey ) const
{
    NameListRef xList = Config::implGetNameList( rKey );
    if( !xList )
    {
        OUString aBaseKey = rKey + CREATE_OUSTRING( "-BIFF" );
        switch( meBiff )
        {
            // fall-through intended!
            case BIFF8: if( !xList ) xList = Config::implGetNameList( aBaseKey + OUString( sal_Unicode( '8' ) ) );
            case BIFF5: if( !xList ) xList = Config::implGetNameList( aBaseKey + OUString( sal_Unicode( '5' ) ) );
            case BIFF4: if( !xList ) xList = Config::implGetNameList( aBaseKey + OUString( sal_Unicode( '4' ) ) );
            case BIFF3: if( !xList ) xList = Config::implGetNameList( aBaseKey + OUString( sal_Unicode( '3' ) ) );
            case BIFF2: if( !xList ) xList = Config::implGetNameList( aBaseKey + OUString( sal_Unicode( '2' ) ) );
            case BIFF_UNKNOWN: break;
        }
    }
    return xList;
}

// ============================================================================

BiffSharedData::BiffSharedData( BiffType eBiff ) :
    meBiff( eBiff ),
    meTextEnc( osl_getThreadTextEncoding() )
{
}

void BiffSharedData::initializePerSheet()
{
    maFontEncs.clear();
    maXfFontIds.clear();
    meTextEnc = osl_getThreadTextEncoding();
}

void BiffSharedData::setTextEncoding( rtl_TextEncoding eTextEnc )
{
    if( eTextEnc != RTL_TEXTENCODING_DONTKNOW )
        meTextEnc = eTextEnc;
}

sal_uInt16 BiffSharedData::getFontCount() const
{
    return static_cast< sal_uInt16 >( maFontEncs.size() );
}

rtl_TextEncoding BiffSharedData::getFontEncoding( sal_uInt16 nFontId ) const
{
    return (nFontId < getFontCount()) ? maFontEncs[ nFontId ] : meTextEnc;
}

void BiffSharedData::appendFontEncoding( rtl_TextEncoding eFontEnc )
{
    maFontEncs.push_back( (eFontEnc == RTL_TEXTENCODING_DONTKNOW) ? meTextEnc : eFontEnc );
    if( maFontEncs.size() == 4 )
        maFontEncs.push_back( meTextEnc );
}

sal_uInt16 BiffSharedData::getXfCount() const
{
    return static_cast< sal_uInt16 >( maXfFontIds.size() );
}

rtl_TextEncoding BiffSharedData::getXfEncoding( sal_uInt16 nXfId ) const
{
    sal_uInt16 nFontId = (nXfId < getXfCount()) ? maXfFontIds[ nXfId ] : 0;
    return getFontEncoding( nFontId );
}

void BiffSharedData::appendXfFontId( sal_uInt16 nFontId )
{
    maXfFontIds.push_back( nFontId );
}

bool BiffSharedData::implIsValid() const
{
    return meBiff != BIFF_UNKNOWN;
}

// ============================================================================

BiffObjectBase::~BiffObjectBase()
{
}

void BiffObjectBase::construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, BiffType eBiff, const OUString& rSysFileName )
{
    if( rParent.isValid() && rxStrm.get() && (eBiff != BIFF_UNKNOWN) )
    {
        mxBiffData.reset( new BiffSharedData( eBiff ) );
        mxBiffStrm.reset( new BiffInputStream( *rxStrm ) );
        RecordObjectBase::construct( rParent, rxStrm, rSysFileName, mxBiffStrm, "RECORD-NAMES", "SIMPLE-RECORDS" );
        if( RecordObjectBase::implIsValid() )
        {
            reconstructConfig( ConfigRef( new BiffConfig( cfg(), eBiff ) ) );
            mxDffObj.reset( new BiffDffStreamObject( *this, mxBiffStrm ) );
            if( StorageBase* pRootStrg = cfg().getRootStorage().get() )
            {
                BinaryInputStreamRef xCtlsStrm( new BinaryXInputStream( pRootStrg->openInputStream( CREATE_OUSTRING( "Ctls" ) ), true ) );
                mxCtlsObj.reset( new BiffCtlsStreamObject( *this, xCtlsStrm ) );
            }
            const Config& rCfg = cfg();
            mxErrCodes = rCfg.getNameList( "ERRORCODES" );
            mxConstType = rCfg.getNameList( "CONSTVALUE-TYPE" );
            mxResultType = rCfg.getNameList( "FORMULA-RESULTTYPE" );
            mnLastRecId = BIFF_ID_UNKNOWN;
            mbMergeContRec = rCfg.getBoolOption( "merge-continue-record", true );
        }
    }
}

void BiffObjectBase::construct( const BiffObjectBase& rParent )
{
    *this = rParent;
}

bool BiffObjectBase::implIsValid() const
{
    return isValid( mxBiffData ) && mxBiffStrm.get() && isValid( mxDffObj ) && InputObjectBase::implIsValid();
}

bool BiffObjectBase::implStartRecord( BinaryInputStream&, sal_Int64& ornRecPos, sal_Int64& ornRecId, sal_Int64& ornRecSize )
{
    // previous record
    mnLastRecId = mxBiffStrm->getRecId();
    switch( mnLastRecId )
    {
        case BIFF_ID_CHBEGIN:
            out().incIndent();
        break;
    }

    // start next record
    bool bValid = mxBiffStrm->startNextRecord();
    ornRecPos = mxBiffStrm->tellBase() - 4;
    ornRecId = mxBiffStrm->getRecId();

    // record specific settings
    switch( mxBiffStrm->getRecId() )
    {
        case BIFF_ID_CHEND:
            out().decIndent();
        break;
    }

    // special CONTINUE handling
    mxBiffStrm->resetRecord( mbMergeContRec );
    if( mbMergeContRec ) switch( mxBiffStrm->getRecId() )
    {
        case BIFF_ID_OBJ:
        case BIFF_ID_TXO:
        case BIFF_ID_EOF:
        case BIFF_ID_CONT:
            mxBiffStrm->resetRecord( false );
        break;
        case BIFF_ID_MSODRAWINGGROUP:
        case BIFF_ID_CHESCHERFORMAT:
            mxBiffStrm->resetRecord( true, mxBiffStrm->getRecId() );
        break;
    }

    ornRecSize = mxBiffStrm->getLength();
    mxBiffStrm->enableNulChars( true );
    return bValid;
}

OUString BiffObjectBase::getErrorName( sal_uInt8 nErrCode ) const
{
    return cfg().getName( mxErrCodes, nErrCode );
}

// ----------------------------------------------------------------------------

sal_Int32 BiffObjectBase::readCol( bool bCol16Bit )
{
    return bCol16Bit ? mxBiffStrm->readuInt16() : mxBiffStrm->readuInt8();
}

sal_Int32 BiffObjectBase::readRow( bool bRow32Bit )
{
    return bRow32Bit ? mxBiffStrm->readInt32() : mxBiffStrm->readuInt16();
}

void BiffObjectBase::readAddress( Address& orAddress, bool bCol16Bit, bool bRow32Bit )
{
    orAddress.mnRow = readRow( bRow32Bit );
    orAddress.mnCol = readCol( bCol16Bit );
}

void BiffObjectBase::readRange( Range& orRange, bool bCol16Bit, bool bRow32Bit )
{
    orRange.maFirst.mnRow = readRow( bRow32Bit );
    orRange.maLast.mnRow = readRow( bRow32Bit );
    orRange.maFirst.mnCol = readCol( bCol16Bit );
    orRange.maLast.mnCol = readCol( bCol16Bit );
}

void BiffObjectBase::readRangeList( RangeList& orRanges, bool bCol16Bit, bool bRow32Bit )
{
    sal_uInt16 nCount;
    *mxBiffStrm >> nCount;
    orRanges.resize( nCount );
    for( RangeList::iterator aIt = orRanges.begin(), aEnd = orRanges.end(); !mxBiffStrm->isEof() && (aIt != aEnd); ++aIt )
        readRange( *aIt, bCol16Bit, bRow32Bit );
}

// ----------------------------------------------------------------------------

void BiffObjectBase::writeBooleanItem( const String& rName, sal_uInt8 nBool )
{
    writeDecItem( rName, nBool, "BOOLEAN" );
}

void BiffObjectBase::writeErrorCodeItem( const String& rName, sal_uInt8 nErrCode )
{
    writeHexItem( rName, nErrCode, mxErrCodes );
}

void BiffObjectBase::writeFontPortions( const FontPortionModelList& rPortions )
{
    if( !rPortions.empty() )
    {
        writeDecItem( "font-count", static_cast< sal_uInt32 >( rPortions.size() ) );
        TableGuard aTabGuard( out(), 14 );
        for( FontPortionModelList::const_iterator aIt = rPortions.begin(), aEnd = rPortions.end(); aIt != aEnd; ++aIt )
        {
            MultiItemsGuard aMultiGuard( out() );
            writeDecItem( "char-pos", aIt->mnPos );
            writeDecItem( "font-idx", aIt->mnFontId, "FONTNAMES" );
        }
    }
}

// ----------------------------------------------------------------------------

OUString BiffObjectBase::dumpByteString( const String& rName, BiffStringFlags nFlags, rtl_TextEncoding eDefaultTextEnc )
{
    OSL_ENSURE( !getFlag( nFlags, static_cast< BiffStringFlags >( ~(BIFF_STR_8BITLENGTH | BIFF_STR_EXTRAFONTS) ) ), "BiffObjectBase::dumpByteString - unknown flag" );
    bool b8BitLength = getFlag( nFlags, BIFF_STR_8BITLENGTH );

    OString aString = mxBiffStrm->readByteString( !b8BitLength );
    FontPortionModelList aPortions;
    if( getFlag( nFlags, BIFF_STR_EXTRAFONTS ) )
        aPortions.importPortions( *mxBiffStrm, false );

    // create string portions
    OUStringBuffer aBuffer;
    sal_Int32 nStrLen = aString.getLength();
    if( nStrLen > 0 )
    {
        // add leading and trailing string position to ease the following loop
        if( aPortions.empty() || (aPortions.front().mnPos > 0) )
            aPortions.insert( aPortions.begin(), FontPortionModel( 0, -1 ) );
        if( aPortions.back().mnPos < nStrLen )
            aPortions.push_back( FontPortionModel( nStrLen, -1 ) );

        // use global text encoding, if nothing special is specified
        if( eDefaultTextEnc == RTL_TEXTENCODING_DONTKNOW )
            eDefaultTextEnc = getBiffData().getTextEncoding();

        // create all string portions according to the font id vector
        for( FontPortionModelList::const_iterator aIt = aPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
        {
            sal_Int32 nPortionLen = (aIt + 1)->mnPos - aIt->mnPos;
            if( nPortionLen > 0 )
            {
                // convert byte string to unicode string, using current font encoding
                rtl_TextEncoding eTextEnc = mxBiffData->getFontEncoding( static_cast< sal_uInt16 >( aIt->mnFontId ) );
                if( eTextEnc == RTL_TEXTENCODING_DONTKNOW )
                    eTextEnc = eDefaultTextEnc;
                aBuffer.append( OStringToOUString( aString.copy( aIt->mnPos, nPortionLen ), eTextEnc ) );
            }
        }
    }

    OUString aUniStr = aBuffer.makeStringAndClear();
    writeStringItem( rName( "text" ), aUniStr );
    return aUniStr;
}

OUString BiffObjectBase::dumpUniString( const String& rName, BiffStringFlags nFlags )
{
    OSL_ENSURE( !getFlag( nFlags, static_cast< BiffStringFlags >( ~(BIFF_STR_8BITLENGTH | BIFF_STR_SMARTFLAGS) ) ), "BiffObjectBase::dumpUniString - unknown flag" );
    bool b8BitLength = getFlag( nFlags, BIFF_STR_8BITLENGTH );

    // --- string header ---
    sal_uInt16 nChars = b8BitLength ? mxBiffStrm->readuInt8() : mxBiffStrm->readuInt16();
    sal_uInt8 nFlagField = 0;
    if( (nChars > 0) || !getFlag( nFlags, BIFF_STR_SMARTFLAGS ) )
        *mxBiffStrm >> nFlagField;
    bool b16Bit    = getFlag( nFlagField, BIFF_STRF_16BIT );
    bool bFonts    = getFlag( nFlagField, BIFF_STRF_RICH );
    bool bPhonetic = getFlag( nFlagField, BIFF_STRF_PHONETIC );
    sal_uInt16 nFontCount = bFonts ? mxBiffStrm->readuInt16() : 0;
    sal_uInt32 nPhoneticSize = bPhonetic ? mxBiffStrm->readuInt32() : 0;

    // --- character array ---
    OUString aString = mxBiffStrm->readUniStringChars( nChars, b16Bit );
    writeStringItem( rName( "text" ), aString );

    // --- formatting ---
    // #122185# bRich flag may be set, but format runs may be missing
    if( nFontCount > 0 )
    {
        IndentGuard aIndGuard( out() );
        FontPortionModelList aPortions;
        aPortions.importPortions( *mxBiffStrm, nFontCount, BIFF_FONTPORTION_16BIT );
        writeFontPortions( aPortions );
    }

    // --- phonetic information ---
    // #122185# bPhonetic flag may be set, but phonetic data may be missing
    if( nPhoneticSize > 0 )
    {
        IndentGuard aIndGuard( out() );
        dumpBinary( "phonetic-data", nPhoneticSize, false );
    }

    return aString;
}

OUString BiffObjectBase::dumpString( const String& rName, BiffStringFlags nByteFlags, BiffStringFlags nUniFlags, rtl_TextEncoding eDefaultTextEnc )
{
    return (getBiff() == BIFF8) ? dumpUniString( rName, nUniFlags ) : dumpByteString( rName, nByteFlags, eDefaultTextEnc );
}

sal_uInt8 BiffObjectBase::dumpBoolean( const String& rName )
{
    sal_uInt8 nBool;
    *mxBiffStrm >> nBool;
    writeBooleanItem( rName( "boolean" ), nBool );
    return nBool;
}

sal_uInt8 BiffObjectBase::dumpErrorCode( const String& rName )
{
    sal_uInt8 nErrCode;
    *mxBiffStrm >> nErrCode;
    writeErrorCodeItem( rName( "error-code" ), nErrCode );
    return nErrCode;
}

rtl_TextEncoding BiffObjectBase::dumpCodePage( const String& rName )
{
    sal_uInt16 nCodePage = dumpDec< sal_uInt16 >( rName( "codepage" ), "CODEPAGES" );
    return BiffHelper::calcTextEncodingFromCodePage( nCodePage );
}

void BiffObjectBase::dumpFormulaResult( const String& rName )
{
    MultiItemsGuard aMultiGuard( out() );
    sal_uInt8 pnResult[ 8 ];
    mxBiffStrm->readMemory( pnResult, 8 );
    writeArrayItem( rName( "result" ), pnResult, 8 );
    if( (pnResult[ 6 ] == 0xFF) && (pnResult[ 7 ] == 0xFF) )
    {
        sal_uInt8 nType = pnResult[ 0 ];
        sal_uInt8 nData = pnResult[ 2 ];
        writeHexItem( "type", nType, mxResultType );
        switch( nType )
        {
            case 1: writeBooleanItem( "value", nData );     break;
            case 2: writeErrorCodeItem( "value", nData );   break;
        }
    }
    else
    {
        double* pfValue = reinterpret_cast< double* >( pnResult );
        ByteOrderConverter::convertLittleEndian( *pfValue );
        writeDecItem( "value", *pfValue );
    }
}

sal_Int32 BiffObjectBase::dumpColIndex( const String& rName, bool bCol16Bit )
{
    sal_Int32 nCol = readCol( bCol16Bit );
    writeColIndexItem( rName( "col-idx" ), nCol );
    return nCol;
}

sal_Int32 BiffObjectBase::dumpRowIndex( const String& rName, bool bRow32Bit )
{
    sal_Int32 nRow = readRow( bRow32Bit );
    writeRowIndexItem( rName( "row-idx" ), nRow );
    return nRow;
}

sal_Int32 BiffObjectBase::dumpColRange( const String& rName, bool bCol16Bit )
{
    sal_Int32 nCol1 = readCol( bCol16Bit );
    sal_Int32 nCol2 = readCol( bCol16Bit );
    writeColRangeItem( rName( "col-range" ), nCol1, nCol2 );
    return nCol2 - nCol1 + 1;
}

sal_Int32 BiffObjectBase::dumpRowRange( const String& rName, bool bRow32Bit )
{
    sal_Int32 nRow1 = readRow( bRow32Bit );
    sal_Int32 nRow2 = readRow( bRow32Bit );
    writeRowRangeItem( rName( "row-range" ), nRow1, nRow2 );
    return nRow2 - nRow1 + 1;
}

Address BiffObjectBase::dumpAddress( const String& rName, bool bCol16Bit, bool bRow32Bit )
{
    Address aPos;
    readAddress( aPos, bCol16Bit, bRow32Bit );
    writeAddressItem( rName( "addr" ), aPos );
    return aPos;
}

Range BiffObjectBase::dumpRange( const String& rName, bool bCol16Bit, bool bRow32Bit )
{
    Range aRange;
    readRange( aRange, bCol16Bit, bRow32Bit );
    writeRangeItem( rName( "range" ), aRange );
    return aRange;
}

void BiffObjectBase::dumpRangeList( const String& rName, bool bCol16Bit, bool bRow32Bit )
{
    RangeList aRanges;
    readRangeList( aRanges, bCol16Bit, bRow32Bit );
    writeRangeListItem( rName( "range-list" ), aRanges );
}

void BiffObjectBase::dumpConstArrayHeader( sal_uInt32& rnCols, sal_uInt32& rnRows )
{
    Output& rOut = out();
    MultiItemsGuard aMultiGuard( rOut );
    rnCols = dumpDec< sal_uInt8 >( "width" );
    rnRows = dumpDec< sal_uInt16 >( "height" );
    switch( getBiff() )
    {
        case BIFF2:
        case BIFF3:
        case BIFF4:
        case BIFF5: if( rnCols == 0 ) rnCols = 256; break;
        case BIFF8: ++rnCols; ++rnRows;             break;
        case BIFF_UNKNOWN:                          break;
    }
    ItemGuard aItem( rOut, "size" );
    rOut.writeDec( rnCols );
    rOut.writeChar( 'x' );
    rOut.writeDec( rnRows );
    aItem.cont();
    rOut.writeDec( rnCols * rnRows );
}

OUString BiffObjectBase::dumpConstValue( sal_Unicode cStrQuote )
{
    Output& rOut = out();
    MultiItemsGuard aMultiGuard( rOut );
    OUStringBuffer aValue;
    switch( dumpDec< sal_uInt8 >( "type", mxConstType ) )
    {
        case BIFF_DATATYPE_EMPTY:
            dumpUnused( 8 );
            aValue.append( OOX_DUMP_EMPTYVALUE );
        break;
        case BIFF_DATATYPE_DOUBLE:
            dumpDec< double >( "value" );
            aValue.append( rOut.getLastItemValue() );
        break;
        case BIFF_DATATYPE_STRING:
            aValue.append( dumpString( "value", BIFF_STR_8BITLENGTH ) );
            StringHelper::enclose( aValue, cStrQuote );
        break;
        case BIFF_DATATYPE_BOOL:
            dumpBoolean( "value" );
            aValue.append( rOut.getLastItemValue() );
            dumpUnused( 7 );
        break;
        case BIFF_DATATYPE_ERROR:
            dumpErrorCode( "value" );
            aValue.append( rOut.getLastItemValue() );
            dumpUnused( 7 );
        break;
    }
    return aValue.makeStringAndClear();
}

sal_uInt16 BiffObjectBase::dumpRepeatedRecId()
{
    return dumpHex< sal_uInt16 >( "repeated-rec-id", getRecNames() );
}

void BiffObjectBase::dumpFrHeader( bool bWithFlags, bool bWithRange )
{
    dumpHex< sal_uInt16 >( "rec-id", getRecNames() );
    sal_Int16 nFlags = bWithFlags ? dumpHex< sal_uInt16 >( "flags", "FR-FLAGS" ) : 0x0001;
    if( bWithRange )
    {
        if( getFlag< sal_uInt16 >( nFlags, 0x0001 ) )
            dumpRange( "range" );
        else
            dumpUnused( 8 );
    }
}

void BiffObjectBase::dumpDffClientRect()
{
    lclDumpDffClientRect( out(), in() );
}

void BiffObjectBase::dumpEmbeddedDff()
{
    out().decIndent();
    writeEmptyItem( "EMBEDDED-DFF-START" );
    out().incIndent();
    mxDffObj->dump();
    out().emptyLine();
    out().decIndent();
    writeEmptyItem( "EMBEDDED-DFF-END" );
    out().incIndent();
}

void BiffObjectBase::dumpOcxControl()
{
    sal_uInt32 nStartPos = dumpHex< sal_uInt32 >( "ctls-stream-pos", "CONV-DEC" );
    sal_uInt32 nLength = dumpHex< sal_uInt32 >( "ctls-stream-length", "CONV-DEC" );
    if( isValid( mxCtlsObj ) )
        mxCtlsObj->dumpControl( nStartPos, nLength );
}

// ============================================================================
// ============================================================================

FormulaObject::FormulaObject( const BiffObjectBase& rParent ) :
    mnSize( 0 )
{
    BiffObjectBase::construct( rParent );
    constructFmlaObj();
}

FormulaObject::~FormulaObject()
{
}

sal_uInt16 FormulaObject::readFormulaSize()
{
    return (getBiff() == BIFF2) ? getBiffStream().readuInt8() : getBiffStream().readuInt16();
}

sal_uInt16 FormulaObject::dumpFormulaSize( const String& rName )
{
    sal_uInt16 nSize = readFormulaSize();
    writeDecItem( rName( "formula-size" ), nSize );
    return nSize;
}

void FormulaObject::dumpCellFormula( const String& rName, sal_uInt16 nSize )
{
    dumpFormula( rName, nSize, false );
}

void FormulaObject::dumpCellFormula( const String& rName )
{
    dumpFormula( rName, false );
}

void FormulaObject::dumpNameFormula( const String& rName, sal_uInt16 nSize )
{
    dumpFormula( rName, nSize, true );
}

void FormulaObject::dumpNameFormula( const String& rName )
{
    dumpFormula( rName, true );
}

void FormulaObject::implDump()
{
    {
        MultiItemsGuard aMultiGuard( out() );
        writeEmptyItem( maName );
        writeDecItem( "formula-size", mnSize );
    }
    if( mnSize == 0 ) return;

    BinaryInputStream& rStrm = in();
    sal_Int64 nStartPos = rStrm.tell();
    sal_Int64 nEndPos = ::std::min< sal_Int64 >( nStartPos + mnSize, rStrm.getLength() );

    bool bValid = mxTokens.get();
    mxStack.reset( new FormulaStack );
    maAddData.clear();
    IndentGuard aIndGuard( out() );
    {
        TableGuard aTabGuard( out(), 8, 18 );
        while( bValid && !rStrm.isEof() && (rStrm.tell() < nEndPos) )
        {
            MultiItemsGuard aMultiGuard( out() );
            writeHexItem( EMPTY_STRING, static_cast< sal_uInt16 >( rStrm.tell() - nStartPos ) );
            sal_uInt8 nTokenId = dumpHex< sal_uInt8 >( EMPTY_STRING, mxTokens );
            bValid = mxTokens->hasName( nTokenId );
            if( bValid )
            {
                sal_uInt8 nTokClass = nTokenId & BIFF_TOKCLASS_MASK;
                sal_uInt8 nBaseId = nTokenId & BIFF_TOKID_MASK;
                if( nTokClass == BIFF_TOKCLASS_NONE )
                {
                    switch( nBaseId )
                    {
                        case BIFF_TOKID_EXP:        dumpExpToken( "EXP" );          break;
                        case BIFF_TOKID_TBL:        dumpExpToken( "TBL" );          break;
                        case BIFF_TOKID_ADD:        dumpBinaryOpToken( "+" );       break;
                        case BIFF_TOKID_SUB:        dumpBinaryOpToken( "-" );       break;
                        case BIFF_TOKID_MUL:        dumpBinaryOpToken( "*" );       break;
                        case BIFF_TOKID_DIV:        dumpBinaryOpToken( "/" );       break;
                        case BIFF_TOKID_POWER:      dumpBinaryOpToken( "^" );       break;
                        case BIFF_TOKID_CONCAT:     dumpBinaryOpToken( "&" );       break;
                        case BIFF_TOKID_LT:         dumpBinaryOpToken( "<" );       break;
                        case BIFF_TOKID_LE:         dumpBinaryOpToken( "<=" );      break;
                        case BIFF_TOKID_EQ:         dumpBinaryOpToken( "=" );       break;
                        case BIFF_TOKID_GE:         dumpBinaryOpToken( ">=" );      break;
                        case BIFF_TOKID_GT:         dumpBinaryOpToken( "<" );       break;
                        case BIFF_TOKID_NE:         dumpBinaryOpToken( "<>" );      break;
                        case BIFF_TOKID_ISECT:      dumpBinaryOpToken( " " );       break;
                        case BIFF_TOKID_LIST:       dumpBinaryOpToken( "," );       break;
                        case BIFF_TOKID_RANGE:      dumpBinaryOpToken( ":" );       break;
                        case BIFF_TOKID_UPLUS:      dumpUnaryOpToken( "+", "" );    break;
                        case BIFF_TOKID_UMINUS:     dumpUnaryOpToken( "-", "" );    break;
                        case BIFF_TOKID_PERCENT:    dumpUnaryOpToken( "", "%" );    break;
                        case BIFF_TOKID_PAREN:      dumpUnaryOpToken( "(", ")" );   break;
                        case BIFF_TOKID_MISSARG:    dumpMissArgToken();             break;
                        case BIFF_TOKID_STR:        dumpStringToken();              break;
                        case BIFF_TOKID_NLR:        bValid = dumpNlrToken();        break;
                        case BIFF_TOKID_ATTR:       bValid = dumpAttrToken();       break;
                        case BIFF_TOKID_SHEET:      dumpSheetToken();               break;
                        case BIFF_TOKID_ENDSHEET:   dumpEndSheetToken();            break;
                        case BIFF_TOKID_ERR:        dumpErrorToken();               break;
                        case BIFF_TOKID_BOOL:       dumpBoolToken();                break;
                        case BIFF_TOKID_INT:        dumpIntToken();                 break;
                        case BIFF_TOKID_NUM:        dumpDoubleToken();              break;
                        default:                    bValid = false;
                    }
                }
                else
                {
                    OUString aTokClass = cfg().getName( mxClasses, nTokClass );
                    switch( nBaseId )
                    {
                        case BIFF_TOKID_ARRAY:      dumpArrayToken( aTokClass );                break;
                        case BIFF_TOKID_FUNC:       dumpFuncToken( aTokClass );                 break;
                        case BIFF_TOKID_FUNCVAR:    dumpFuncVarToken( aTokClass );              break;
                        case BIFF_TOKID_NAME:       dumpNameToken( aTokClass );                 break;
                        case BIFF_TOKID_REF:        dumpRefToken( aTokClass, false );           break;
                        case BIFF_TOKID_AREA:       dumpAreaToken( aTokClass, false );          break;
                        case BIFF_TOKID_MEMAREA:    dumpMemAreaToken( aTokClass, true );        break;
                        case BIFF_TOKID_MEMERR:     dumpMemAreaToken( aTokClass, false );       break;
                        case BIFF_TOKID_MEMNOMEM:   dumpMemAreaToken( aTokClass, false );       break;
                        case BIFF_TOKID_MEMFUNC:    dumpMemFuncToken( aTokClass );              break;
                        case BIFF_TOKID_REFERR:     dumpRefErrToken( aTokClass, false );        break;
                        case BIFF_TOKID_AREAERR:    dumpRefErrToken( aTokClass, true );         break;
                        case BIFF_TOKID_REFN:       dumpRefToken( aTokClass, true );            break;
                        case BIFF_TOKID_AREAN:      dumpAreaToken( aTokClass, true );           break;
                        case BIFF_TOKID_MEMAREAN:   dumpMemFuncToken( aTokClass );              break;
                        case BIFF_TOKID_MEMNOMEMN:  dumpMemFuncToken( aTokClass );              break;
                        case BIFF_TOKID_FUNCCE:     dumpCmdToken( aTokClass );                  break;
                        case BIFF_TOKID_NAMEX:      dumpNameXToken( aTokClass );                break;
                        case BIFF_TOKID_REF3D:      dumpRef3dToken( aTokClass, mbNameMode );    break;
                        case BIFF_TOKID_AREA3D:     dumpArea3dToken( aTokClass, mbNameMode );   break;
                        case BIFF_TOKID_REFERR3D:   dumpRefErr3dToken( aTokClass, false );      break;
                        case BIFF_TOKID_AREAERR3D:  dumpRefErr3dToken( aTokClass, true );       break;
                        default:                    bValid = false;
                    }
                }
            }
        }
    }
    bValid = nEndPos == rStrm.tell();
    if( bValid )
    {
        dumpAddTokenData();
        writeInfoItem( "formula", mxStack->getFormulaString() );
        writeInfoItem( "classes", mxStack->getClassesString() );
    }
    else
        dumpBinary( OOX_DUMP_ERRASCII( "formula-error" ), nEndPos - rStrm.tell(), false );

    mnSize = 0;
}

void FormulaObject::dumpFormula( const String& rName, sal_uInt16 nSize, bool bNameMode )
{
    maName = rName( "formula" );
    mnSize = nSize;
    mbNameMode = bNameMode;
    dump();
    mnSize = 0;
}

void FormulaObject::dumpFormula( const String& rName, bool bNameMode )
{
    dumpFormula( rName, readFormulaSize(), bNameMode );
}

// private --------------------------------------------------------------------

void FormulaObject::constructFmlaObj()
{
    if( BiffObjectBase::implIsValid() )
    {
        mxFuncProv.reset( new FunctionProvider( FILTER_BIFF, getBiff(), true ) );

        Config& rCfg = cfg();
        mxClasses   = rCfg.getNameList( "TOKENCLASSES" );
        mxRelFlags  = rCfg.getNameList( "REFRELFLAGS" );
        mxNlrTypes  = rCfg.getNameList( "NLRTYPES" );
        mxAttrTypes = rCfg.getNameList( "ATTRTYPES" );
        mxSpTypes   = rCfg.getNameList( "ATTRSPACETYPES" );

        // create classified token names
        mxTokens = rCfg.createNameList< ConstList >( "TOKENS" );
        mxTokens->includeList( rCfg.getNameList( "BASETOKENS" ) );

        NameListRef xClassTokens = rCfg.getNameList( "CLASSTOKENS" );
        if( mxClasses.get() && xClassTokens.get() )
            for( NameListBase::const_iterator aCIt = mxClasses->begin(), aCEnd = mxClasses->end(); aCIt != aCEnd; ++aCIt )
                for( NameListBase::const_iterator aTIt = xClassTokens->begin(), aTEnd = xClassTokens->end(); aTIt != aTEnd; ++aTIt )
                    mxTokens->setName( aCIt->first | aTIt->first, aTIt->second + aCIt->second );

        mnColCount = 256;
        mnRowCount = (getBiff() == BIFF8) ? 65536 : 16384;
    }
}

// ----------------------------------------------------------------------------

namespace {

OUString lclCreateName( const OUString& rRef, sal_uInt16 nNameIdx )
{
    OUStringBuffer aName( rRef );
    StringHelper::appendIndexedText( aName, CREATE_OUSTRING( "NAME" ), nNameIdx );
    return aName.makeStringAndClear();
}

OUString lclCreateNlr( const OUString& rData, bool bRel = true )
{
    OUStringBuffer aNlr;
    if( !bRel ) aNlr.append( OOX_DUMP_ADDRABS );
    StringHelper::appendIndexedText( aNlr, CREATE_OUSTRING( "NLR" ), rData );
    return aNlr.makeStringAndClear();
}

OUString lclCreateNlr( const TokenAddress& rPos )
{
    OUStringBuffer aAddr;
    StringHelper::appendAddrCol( aAddr, rPos.mnCol, true );
    StringHelper::appendAddrRow( aAddr, rPos.mnRow, true );
    return lclCreateNlr( aAddr.makeStringAndClear(), rPos.mbRelRow );
}

} // namespace

// ----------------------------------------------------------------------------

TokenAddress FormulaObject::createTokenAddress( sal_uInt16 nCol, sal_uInt16 nRow, bool bRelC, bool bRelR, bool bNameMode ) const
{
    TokenAddress aPos;
    aPos.mnCol = nCol;
    if( bRelC && bNameMode && (nCol >= mnColCount / 2) ) aPos.mnCol -= mnColCount;
    aPos.mbRelCol = bRelC;
    aPos.mnRow = nRow;
    if( bRelR && bNameMode && (nRow >= mnRowCount / 2) ) aPos.mnRow -= mnRowCount;
    aPos.mbRelRow = bRelR;
    return aPos;
}

OUString FormulaObject::createRef( const OUString& rData ) const
{
    return maRefPrefix + rData;
}

OUString FormulaObject::createName( sal_uInt16 nNameIdx ) const
{
    return lclCreateName( maRefPrefix, nNameIdx );
}

OUString FormulaObject::createPlaceHolder( size_t nIdx ) const
{
    OUStringBuffer aStr;
    StringHelper::appendDec( aStr, static_cast< sal_uInt32 >( nIdx ) );
    StringHelper::enclose( aStr, OOX_DUMP_PLACEHOLDER );
    return aStr.makeStringAndClear();
}

OUString FormulaObject::createPlaceHolder() const
{
    return createPlaceHolder( maAddData.size() );
}

sal_uInt16 FormulaObject::readFuncId()
{
    return (getBiff() >= BIFF4) ? in().readuInt16() : in().readuInt8();
}

OUString FormulaObject::writeFuncIdItem( sal_uInt16 nFuncId, const FunctionInfo** oppFuncInfo )
{
    ItemGuard aItemGuard( out(), "func-id" );
    writeHexItem( EMPTY_STRING, nFuncId, "FUNCID" );
    OUStringBuffer aBuffer;
    const FunctionInfo* pFuncInfo = mxFuncProv->getFuncInfoFromBiffFuncId( nFuncId );
    if( pFuncInfo )
        aBuffer.append( pFuncInfo->maOoxFuncName );
    else
    {
        bool bCmd = getFlag( nFuncId, BIFF_TOK_FUNCVAR_CMD );
        aBuffer.appendAscii( bCmd ? "CMD" : "FUNC" );
        StringHelper::appendIndex( aBuffer, nFuncId & BIFF_TOK_FUNCVAR_FUNCIDMASK );
    }
    OUString aFuncName = aBuffer.makeStringAndClear();
    aItemGuard.cont();
    out().writeChar( OOX_DUMP_STRQUOTE );
    out().writeString( aFuncName );
    out().writeChar( OOX_DUMP_STRQUOTE );
    if( oppFuncInfo ) *oppFuncInfo = pFuncInfo;
    return aFuncName;
}

sal_uInt16 FormulaObject::dumpTokenCol( const String& rName, bool& rbRelC, bool& rbRelR )
{
    sal_uInt16 nCol = 0;
    if( getBiff() == BIFF8 )
    {
        nCol = dumpHex< sal_uInt16 >( rName, mxRelFlags );
        rbRelC = getFlag( nCol, BIFF_TOK_REF_COLREL );
        rbRelR = getFlag( nCol, BIFF_TOK_REF_ROWREL );
        nCol &= BIFF_TOK_REF_COLMASK;
    }
    else
        nCol = dumpDec< sal_uInt8 >( rName );
    return nCol;
}

sal_uInt16 FormulaObject::dumpTokenRow( const String& rName, bool& rbRelC, bool& rbRelR )
{
    sal_uInt16 nRow = 0;
    if( getBiff() == BIFF8 )
        nRow = dumpDec< sal_uInt16 >( rName );
    else
    {
        nRow = dumpHex< sal_uInt16 >( rName, mxRelFlags );
        rbRelC = getFlag( nRow, BIFF_TOK_REF_COLREL );
        rbRelR = getFlag( nRow, BIFF_TOK_REF_ROWREL );
        nRow &= BIFF_TOK_REF_ROWMASK;
    }
    return nRow;
}

TokenAddress FormulaObject::dumpTokenAddress( bool bNameMode )
{
    bool bRelC = false;
    bool bRelR = false;
    sal_uInt16 nRow = dumpTokenRow( "row", bRelC, bRelR );
    sal_uInt16 nCol = dumpTokenCol( "col", bRelC, bRelR );
    return createTokenAddress( nCol, nRow, bRelC, bRelR, bNameMode );
}

TokenRange FormulaObject::dumpTokenRange( bool bNameMode )
{
    bool bRelC1 = false;
    bool bRelR1 = false;
    bool bRelC2 = false;
    bool bRelR2 = false;
    sal_uInt16 nRow1 = dumpTokenRow( "row1", bRelC1, bRelR1 );
    sal_uInt16 nRow2 = dumpTokenRow( "row2", bRelC2, bRelR2 );
    sal_uInt16 nCol1 = dumpTokenCol( "col1", bRelC1, bRelR1 );
    sal_uInt16 nCol2 = dumpTokenCol( "col2", bRelC2, bRelR2 );
    TokenRange aRange;
    aRange.maFirst = createTokenAddress( nCol1, nRow1, bRelC1, bRelR1, bNameMode );
    aRange.maLast  = createTokenAddress( nCol2, nRow2, bRelC2, bRelR2, bNameMode );
    return aRange;
}

sal_Int16 FormulaObject::readTokenRefIdx()
{
    sal_Int16 nRefIdx = dumpDec< sal_Int16 >( "ref-idx" );
    switch( getBiff() )
    {
        case BIFF2: dumpUnused( 1 );    break;
        case BIFF3: dumpUnused( 2 );    break;
        case BIFF4: dumpUnused( 2 );    break;
        case BIFF5: dumpUnused( 8 );    break;
        case BIFF8:                     break;
        case BIFF_UNKNOWN:              break;
    }
    return nRefIdx;
}

OUString FormulaObject::dumpTokenRefIdx()
{
    OUStringBuffer aRef( CREATE_OUSTRING( "REF" ) );
    StringHelper::appendIndex( aRef, readTokenRefIdx() );
    aRef.append( OOX_DUMP_TABSEP );
    return aRef.makeStringAndClear();
}

OUString FormulaObject::dumpTokenRefTabIdxs()
{
    sal_Int16 nRefIdx = readTokenRefIdx();
    OUStringBuffer aRef( CREATE_OUSTRING( "REF" ) );
    StringHelper::appendIndex( aRef, nRefIdx );
    if( getBiff() == BIFF5 )
    {
        dumpDec< sal_Int16 >( "tab1" );
        sal_Int16 nTab2 = dumpDec< sal_Int16 >( "tab2" );
        if( (nRefIdx > 0) && (nTab2 > 0) && (nRefIdx != nTab2) )
        {
            aRef.append( OOX_DUMP_RANGESEP );
            aRef.appendAscii( "REF" );
            StringHelper::appendIndex( aRef, nTab2 );
        }
    }
    aRef.append( OOX_DUMP_TABSEP );
    return aRef.makeStringAndClear();
}

void FormulaObject::dumpIntToken()
{
    dumpDec< sal_uInt16 >( "value" );
    mxStack->pushOperand( out().getLastItemValue() );
}

void FormulaObject::dumpDoubleToken()
{
    dumpDec< double >( "value" );
    mxStack->pushOperand( out().getLastItemValue() );
}

void FormulaObject::dumpStringToken()
{
    OUStringBuffer aValue;
    aValue.append( dumpString( "value", BIFF_STR_8BITLENGTH, BIFF_STR_8BITLENGTH ) );
    StringHelper::enclose( aValue, OOX_DUMP_FMLASTRQUOTE );
    mxStack->pushOperand( aValue.makeStringAndClear() );
}

void FormulaObject::dumpBoolToken()
{
    dumpBoolean( "value" );
    mxStack->pushOperand( out().getLastItemValue() );
}

void FormulaObject::dumpErrorToken()
{
    dumpErrorCode( "value" );
    mxStack->pushOperand( out().getLastItemValue() );
}

void FormulaObject::dumpMissArgToken()
{
    mxStack->pushOperand( OUString( OOX_DUMP_EMPTYVALUE ) );
}

void FormulaObject::dumpArrayToken( const OUString& rTokClass )
{
    dumpUnused( (getBiff() == BIFF2) ? 6 : 7 );
    mxStack->pushOperand( createPlaceHolder(), rTokClass );
    maAddData.push_back( ADDDATA_ARRAY );
}

void FormulaObject::dumpNameToken( const OUString& rTokClass )
{
    sal_uInt16 nNameIdx = dumpDec< sal_uInt16 >( "name-idx" );
    switch( getBiff() )
    {
        case BIFF2: dumpUnused( 5 );    break;
        case BIFF3:
        case BIFF4: dumpUnused( 8 );    break;
        case BIFF5: dumpUnused( 12 );   break;
        case BIFF8: dumpUnused( 2 );    break;
        case BIFF_UNKNOWN:              break;
    }
    mxStack->pushOperand( createName( nNameIdx ), rTokClass );
}

void FormulaObject::dumpNameXToken( const OUString& rTokClass )
{
    OUString aRef = dumpTokenRefIdx();
    sal_uInt16 nNameIdx = dumpDec< sal_uInt16 >( "name-idx" );
    dumpUnused( (getBiff() == BIFF8) ? 2 : 12 );
    mxStack->pushOperand( lclCreateName( aRef, nNameIdx ), rTokClass );
}

void FormulaObject::dumpRefToken( const OUString& rTokClass, bool bNameMode )
{
    TokenAddress aPos = dumpTokenAddress( bNameMode );
    writeTokenAddressItem( "addr", aPos, bNameMode );
    mxStack->pushOperand( createRef( out().getLastItemValue() ), rTokClass );
}

void FormulaObject::dumpAreaToken( const OUString& rTokClass, bool bNameMode )
{
    TokenRange aRange = dumpTokenRange( bNameMode );
    writeTokenRangeItem( "range", aRange, bNameMode );
    mxStack->pushOperand( createRef( out().getLastItemValue() ), rTokClass );
}

void FormulaObject::dumpRefErrToken( const OUString& rTokClass, bool bArea )
{
    dumpUnused( ((getBiff() == BIFF8) ? 4 : 3) * (bArea ? 2 : 1) );
    mxStack->pushOperand( createRef( getErrorName( BIFF_ERR_REF ) ), rTokClass );
}

void FormulaObject::dumpRef3dToken( const OUString& rTokClass, bool bNameMode )
{
    OUString aRef = dumpTokenRefTabIdxs();
    TokenAddress aPos = dumpTokenAddress( bNameMode );
    writeTokenAddress3dItem( "addr", aRef, aPos, bNameMode );
    mxStack->pushOperand( out().getLastItemValue(), rTokClass );
}

void FormulaObject::dumpArea3dToken( const OUString& rTokClass, bool bNameMode )
{
    OUString aRef = dumpTokenRefTabIdxs();
    TokenRange aRange = dumpTokenRange( bNameMode );
    writeTokenRange3dItem( "range", aRef, aRange, bNameMode );
    mxStack->pushOperand( out().getLastItemValue(), rTokClass );
}

void FormulaObject::dumpRefErr3dToken( const OUString& rTokClass, bool bArea )
{
    OUString aRef = dumpTokenRefTabIdxs();
    dumpUnused( ((getBiff() == BIFF8) ? 4 : 3) * (bArea ? 2 : 1) );
    mxStack->pushOperand( aRef + getErrorName( BIFF_ERR_REF ), rTokClass );
}

void FormulaObject::dumpMemFuncToken( const OUString& /*rTokClass*/ )
{
    dumpDec< sal_uInt16, sal_uInt8 >( getBiff() != BIFF2, "size" );
}

void FormulaObject::dumpMemAreaToken( const OUString& rTokClass, bool bAddData )
{
    dumpUnused( (getBiff() == BIFF2) ? 3 : 4 );
    dumpMemFuncToken( rTokClass );
    if( bAddData )
        maAddData.push_back( ADDDATA_MEMAREA );
}

void FormulaObject::dumpExpToken( const String& rName )
{
    Address aPos;
    aPos.mnRow = dumpDec< sal_uInt16 >( "row" );
    aPos.mnCol = dumpDec< sal_uInt16, sal_uInt8 >( getBiff() != BIFF2, "col" );
    writeAddressItem( "base-addr", aPos );
    OUStringBuffer aOp( rName );
    StringHelper::appendIndex( aOp, out().getLastItemValue() );
    mxStack->pushOperand( aOp.makeStringAndClear() );
}

void FormulaObject::dumpUnaryOpToken( const String& rLOp, const String& rROp )
{
    mxStack->pushUnaryOp( rLOp, rROp );
}

void FormulaObject::dumpBinaryOpToken( const String& rOp )
{
    mxStack->pushBinaryOp( rOp );
}

void FormulaObject::dumpFuncToken( const OUString& rTokClass )
{
    sal_uInt16 nFuncId = readFuncId();
    const FunctionInfo* pFuncInfo = 0;
    OUString aFuncName = writeFuncIdItem( nFuncId, &pFuncInfo );
    if( pFuncInfo && (pFuncInfo->mnMinParamCount == pFuncInfo->mnMaxParamCount) )
        mxStack->pushFuncOp( aFuncName, rTokClass, pFuncInfo->mnMinParamCount );
    else
        mxStack->setError();
}

void FormulaObject::dumpFuncVarToken( const OUString& rTokClass )
{
    sal_uInt8 nParamCount;
    in() >> nParamCount;
    sal_uInt16 nFuncId = readFuncId();
    bool bCmd = getFlag( nFuncId, BIFF_TOK_FUNCVAR_CMD );
    if( bCmd )
        writeHexItem( "param-count", nParamCount, "PARAMCOUNT-CMD" );
    else
        writeDecItem( "param-count", nParamCount );
    OUString aFuncName = writeFuncIdItem( nFuncId );
    if( bCmd && getFlag( nParamCount, BIFF_TOK_FUNCVAR_CMDPROMPT ) )
    {
        aFuncName += OUString( OOX_DUMP_CMDPROMPT );
        nParamCount &= BIFF_TOK_FUNCVAR_COUNTMASK;
    }
    mxStack->pushFuncOp( aFuncName, rTokClass, nParamCount );
}

void FormulaObject::dumpCmdToken( const OUString& rTokClass )
{
    sal_uInt8 nParamCount = dumpDec< sal_uInt8 >( "param-count", "PARAMCOUNT-CMD" );
    sal_uInt16 nCmdId = readFuncId() | BIFF_TOK_FUNCVAR_CMD;
    OUString aFuncName = writeFuncIdItem( nCmdId );
    if( getFlag( nParamCount, BIFF_TOK_FUNCVAR_CMDPROMPT ) )
    {
        aFuncName += OUString( OOX_DUMP_CMDPROMPT );
        nParamCount &= BIFF_TOK_FUNCVAR_COUNTMASK;
    }
    mxStack->pushFuncOp( aFuncName, rTokClass, nParamCount );
}

void FormulaObject::dumpSheetToken()
{
    dumpUnused( (getBiff() == BIFF2) ? 4 : 6 );
    maRefPrefix = dumpTokenRefIdx();
}

void FormulaObject::dumpEndSheetToken()
{
    dumpUnused( (getBiff() == BIFF2) ? 3 : 4 );
    maRefPrefix = OUString();
}

bool FormulaObject::dumpAttrToken()
{
    bool bValid = true;
    bool bBiff2 = getBiff() == BIFF2;
    sal_uInt8 nType = dumpHex< sal_uInt8 >( "type", mxAttrTypes );
    switch( nType )
    {
        case BIFF_TOK_ATTR_VOLATILE:
            dumpUnused( bBiff2 ? 1 : 2 );
        break;
        case BIFF_TOK_ATTR_IF:
            dumpDec< sal_uInt16, sal_uInt8 >( !bBiff2, "skip" );
        break;
        case BIFF_TOK_ATTR_CHOOSE:
        {
            sal_uInt16 nCount = dumpDec< sal_uInt16, sal_uInt8 >( !bBiff2, "choices" );
            out().resetItemIndex();
            for( sal_uInt16 nIdx = 0; nIdx < nCount; ++nIdx )
                dumpDec< sal_uInt16, sal_uInt8 >( !bBiff2, "#skip" );
            dumpDec< sal_uInt16, sal_uInt8 >( !bBiff2, "skip-err" );
        }
        break;
        case BIFF_TOK_ATTR_SKIP:
            dumpDec< sal_uInt16, sal_uInt8 >( !bBiff2, "skip" );
        break;
        case BIFF_TOK_ATTR_SUM:
            dumpUnused( bBiff2 ? 1 : 2 );
            mxStack->pushFuncOp( CREATE_OUSTRING( "SUM" ), OUString( OOX_DUMP_BASECLASS ), 1 );
        break;
        case BIFF_TOK_ATTR_ASSIGN:
            dumpUnused( bBiff2 ? 1 : 2 );
        break;
        case BIFF_TOK_ATTR_SPACE:
        case BIFF_TOK_ATTR_SPACE | BIFF_TOK_ATTR_VOLATILE:
            switch( getBiff() )
            {
                case BIFF2:
                    bValid = false;
                break;
                case BIFF3:
                    dumpDec< sal_uInt16 >( "leading-spaces" );
                break;
                case BIFF4:
                case BIFF5:
                case BIFF8:
                    dumpDec< sal_uInt8 >( "char-type", mxSpTypes );
                    dumpDec< sal_uInt8 >( "char-count" );
                break;
                case BIFF_UNKNOWN: break;
            }
        break;
        default:
            bValid = false;
    }
    return bValid;
}

bool FormulaObject::dumpNlrToken()
{
    const OUString aRefClass = cfg().getName( mxClasses, BIFF_TOKCLASS_REF );
    const OUString aValClass = cfg().getName( mxClasses, BIFF_TOKCLASS_VAL );

    bool bValid = true;
    sal_uInt8 nType = dumpHex< sal_uInt8 >( "type", mxNlrTypes );
    switch( nType )
    {
        case BIFF_TOK_NLR_ERR:      dumpNlrErrToken();                      break;
        case BIFF_TOK_NLR_ROWR:     dumpNlrColRowToken( aRefClass, false ); break;
        case BIFF_TOK_NLR_COLR:     dumpNlrColRowToken( aRefClass, false ); break;
        case BIFF_TOK_NLR_ROWV:     dumpNlrColRowToken( aValClass, false ); break;
        case BIFF_TOK_NLR_COLV:     dumpNlrColRowToken( aValClass, false ); break;
        case BIFF_TOK_NLR_RANGE:    dumpNlrRangeToken( aRefClass, false );  break;
        case BIFF_TOK_NLR_SRANGE:   dumpNlrRangeToken( aRefClass, true );   break;
        case BIFF_TOK_NLR_SROWR:    dumpNlrColRowToken( aRefClass, true );  break;
        case BIFF_TOK_NLR_SCOLR:    dumpNlrColRowToken( aRefClass, true );  break;
        case BIFF_TOK_NLR_SROWV:    dumpNlrColRowToken( aValClass, true );  break;
        case BIFF_TOK_NLR_SCOLV:    dumpNlrColRowToken( aValClass, true );  break;
        case BIFF_TOK_NLR_RANGEERR: dumpNlrRangeErrToken();                 break;
        default:                    bValid = false;
    }
    return bValid;
}

void FormulaObject::dumpNlrErrToken()
{
    dumpDec< sal_uInt32 >( "delname-idx" );
    mxStack->pushOperand( lclCreateNlr( getErrorName( BIFF_ERR_NAME ) ) );
}

void FormulaObject::dumpNlrColRowToken( const OUString& rTokClass, bool bAddData )
{
    if( bAddData )
    {
        dumpUnused( 4 );
        mxStack->pushOperand( createPlaceHolder(), rTokClass );
        maAddData.push_back( ADDDATA_NLR );
    }
    else
    {
        TokenAddress aPos = dumpTokenAddress( false );
        writeInfoItem( "addr", lclCreateNlr( aPos ) );
        mxStack->pushOperand( out().getLastItemValue(), rTokClass );
    }
}

void FormulaObject::dumpNlrRangeToken( const OUString& rTokClass, bool bAddData )
{
    if( bAddData )
    {
        dumpUnused( 4 );
        mxStack->pushOperand( createPlaceHolder(), rTokClass );
        maAddData.push_back( ADDDATA_NLR );
    }
    else
    {
        TokenAddress aPos = dumpTokenAddress( false );
        writeInfoItem( "addr", lclCreateNlr( aPos ) );
        mxStack->pushOperand( out().getLastItemValue(), rTokClass );
    }
    dumpUnknown( 1 );
    dumpRange( "target-range" );
}

void FormulaObject::dumpNlrRangeErrToken()
{
    dumpDec< sal_uInt32 >( "delname-idx" );
    dumpUnused( 9 );
    mxStack->pushOperand( lclCreateNlr( getErrorName( BIFF_ERR_NAME ) ) );
}

void FormulaObject::dumpAddTokenData()
{
    Output& rOut = out();
    rOut.resetItemIndex();
    for( AddDataTypeVec::const_iterator aIt = maAddData.begin(), aEnd = maAddData.end(); aIt != aEnd; ++aIt )
    {
        AddDataType eType = *aIt;

        {
            ItemGuard aItem( rOut, "#add-data" );
            switch( eType )
            {
                case ADDDATA_NLR:       rOut.writeAscii( "tNlr" );      break;
                case ADDDATA_ARRAY:     rOut.writeAscii( "tArray" );    break;
                case ADDDATA_MEMAREA:   rOut.writeAscii( "tMemArea" );  break;
            }
        }

        size_t nIdx = aIt - maAddData.begin();
        IndentGuard aIndGuard( rOut );
        switch( eType )
        {
            case ADDDATA_NLR:       dumpAddDataNlr( nIdx );     break;
            case ADDDATA_ARRAY:     dumpAddDataArray( nIdx );   break;
            case ADDDATA_MEMAREA:   dumpAddDataMemArea( nIdx ); break;
        }
    }
}

void FormulaObject::dumpAddDataNlr( size_t nIdx )
{
    sal_uInt32 nFlags = dumpHex< sal_uInt32 >( "flags", "NLRADDFLAGS" );
    sal_uInt32 nCount = nFlags & BIFF_TOK_NLR_ADDMASK;
    OUStringBuffer aBuffer;
    for( sal_uInt32 nPos = 0; nPos < nCount; ++nPos )
    {
        Address aPos;
        readAddress( aPos );
        OUStringBuffer aAddr;
        StringHelper::appendAddress( aAddr, aPos );
        StringHelper::appendToken( aBuffer, aAddr.makeStringAndClear(), OOX_DUMP_LISTSEP );
    }
    OUString aAddrList = aBuffer.makeStringAndClear();
    writeInfoItem( "stacked-positions", aAddrList );
    mxStack->replaceOnTop( createPlaceHolder( nIdx ), lclCreateNlr( aAddrList ) );
}

void FormulaObject::dumpAddDataArray( size_t nIdx )
{
    sal_uInt32 nCols, nRows;
    dumpConstArrayHeader( nCols, nRows );

    OUStringBuffer aOp;
    TableGuard aTabGuard( out(), 17 );
    for( sal_uInt32 nRow = 0; nRow < nRows; ++nRow )
    {
        OUStringBuffer aArrayLine;
        for( sal_uInt32 nCol = 0; nCol < nCols; ++nCol )
            StringHelper::appendToken( aArrayLine, dumpConstValue( OOX_DUMP_FMLASTRQUOTE ), OOX_DUMP_LISTSEP );
        StringHelper::appendToken( aOp, aArrayLine.makeStringAndClear(), OOX_DUMP_ARRAYSEP );
    }
    StringHelper::enclose( aOp, '{', '}' );
    mxStack->replaceOnTop( createPlaceHolder( nIdx ), aOp.makeStringAndClear() );
}

void FormulaObject::dumpAddDataMemArea( size_t /*nIdx*/ )
{
    dumpRangeList( EMPTY_STRING, getBiff() == BIFF8 );
}

// ============================================================================
// ============================================================================

RecordStreamObject::~RecordStreamObject()
{
}

void RecordStreamObject::construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, BiffType eBiff, const OUString& rSysFileName )
{
    BiffObjectBase::construct( rParent, rxStrm, eBiff, rSysFileName );
    if( BiffObjectBase::implIsValid() )
        mxFmlaObj.reset( new FormulaObject( *this ) );
}

bool RecordStreamObject::implIsValid() const
{
    return isValid( mxFmlaObj ) && BiffObjectBase::implIsValid();
}

// ============================================================================

WorkbookStreamObject::WorkbookStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName )
{
    if( rxStrm.get() )
    {
        BiffType eBiff = BiffDetector::detectStreamBiffVersion( *rxStrm );
        RecordStreamObject::construct( rParent, rxStrm, eBiff, rSysFileName );
        if( RecordStreamObject::implIsValid() )
        {
            Config& rCfg = cfg();
            mxColors = rCfg.getNameList( "COLORS" );
            mxBorderStyles = rCfg.getNameList( "BORDERSTYLES" );
            mxFillPatterns = rCfg.getNameList( "FILLPATTERNS" );
            mnPTRowFields = 0;
            mnPTColFields = 0;
            mnPTRowColItemsIdx = 0;
            mbHasDff = false;
            initializePerSheet();
        }
    }
}

WorkbookStreamObject::~WorkbookStreamObject()
{
    if( WorkbookStreamObject::implIsValid() )
    {
        Config& rCfg = cfg();
        rCfg.eraseNameList( "FONTNAMES" );
        rCfg.eraseNameList( "FORMATS" );
    }
}

void WorkbookStreamObject::implDumpRecordBody()
{
    BiffInputStream& rStrm = getBiffStream();
    sal_uInt16 nRecId = rStrm.getRecId();
    sal_Int64 nRecSize = rStrm.getLength();
    BiffType eBiff = getBiff();

    switch( nRecId )
    {
        case BIFF2_ID_ARRAY:
        case BIFF3_ID_ARRAY:
            dumpRange( "array-range", false );
            dumpHex< sal_uInt16, sal_uInt8 >( eBiff != BIFF2, "flags", "ARRAY-FLAGS" );
            if( eBiff >= BIFF5 ) dumpUnused( 4 );
            getFormulaDumper().dumpCellFormula();
        break;

        case BIFF2_ID_BLANK:
        case BIFF3_ID_BLANK:
            dumpCellHeader( nRecId == BIFF2_ID_BLANK );
        break;

        case BIFF2_ID_BOF:
        case BIFF3_ID_BOF:
        case BIFF4_ID_BOF:
        case BIFF5_ID_BOF:
            dumpHex< sal_uInt16 >( "bof-type", "BOF-BIFFTYPE" );
            dumpHex< sal_uInt16 >( "sheet-type", "BOF-SHEETTYPE" );
            if( nRecSize >= 6 )  dumpDec< sal_uInt16 >( "build-id" );
            if( nRecSize >= 8 )  dumpDec< sal_uInt16 >( "build-year" );
            if( nRecSize >= 12 ) dumpHex< sal_uInt32 >( "history-flags", "BOF-HISTORY-FLAGS" );
            if( nRecSize >= 16 ) dumpHex< sal_uInt32 >( "lowest-version", "BOF-LOWESTVERSION-FLAGS" );
            if( (eBiff == BIFF4) && (getLastRecId() != BIFF_ID_OBJ) )
                initializePerSheet();
        break;

        case BIFF2_ID_BOOLERR:
        case BIFF3_ID_BOOLERR:
            dumpCellHeader( nRecId == BIFF2_ID_BOOLERR );
            dumpBoolErr();
        break;

        case BIFF_ID_CFHEADER:
            dumpDec< sal_uInt16 >( "rule-count" );
            dumpBool< sal_uInt16 >( "need-update" );
            dumpRange( "bounding-range" );
            dumpRangeList();
        break;

        case BIFF_ID_CFRULE:
        {
            dumpDec< sal_uInt8 >( "type", "CFRULE-TYPE" );
            dumpDec< sal_uInt8 >( "operator", "CFRULE-OPERATOR" );
            sal_uInt16 nFmla1Size = dumpDec< sal_uInt16 >( "formula1-size" );
            sal_uInt16 nFmla2Size = dumpDec< sal_uInt16 >( "formula2-size" );
            sal_uInt32 nFlags = dumpHex< sal_uInt32 >( "flags", "CFRULE-FLAGS" );
            dumpUnused( 2 );
            if( getFlag< sal_uInt32 >( nFlags, 0x04000000 ) )
            {
                writeEmptyItem( "font-block" );
                IndentGuard aIndGuard( out() );
                sal_Int64 nRecPos = rStrm.tell();
                dumpUniString( "name", BIFF_STR_8BITLENGTH );
                dumpUnused( static_cast< sal_Int32 >( nRecPos + 64 - rStrm.tell() ) );
                dumpDec< sal_Int32 >( "height", "CONV-TWIP-TO-PT" );
                dumpHex< sal_uInt32 >( "flags", "CFRULE-FONTFLAGS" );
                dumpDec< sal_uInt16 >( "weight", "FONT-WEIGHT" );
                dumpDec< sal_uInt16 >( "escapement", "FONT-ESCAPEMENT" );
                dumpDec< sal_uInt8 >( "underline", "FONT-UNDERLINE" );
                dumpUnused( 3 );    // family/charset?
                dumpDec< sal_Int32 >( "color", mxColors );
                dumpUnused( 4 );
                dumpHex< sal_uInt32 >( "used-flags", "CFRULE-FONTUSEDFLAGS" );
                dumpDec< sal_uInt32 >( "escapement-used", "CFRULE-FONTUSED" );
                dumpDec< sal_uInt32 >( "underline-used", "CFRULE-FONTUSED" );
                dumpUnused( 18 );
            }
            if( getFlag< sal_uInt32 >( nFlags, 0x08000000 ) )
            {
                writeEmptyItem( "alignment-block" );
                IndentGuard aIndGuard( out() );
                dumpHex< sal_uInt8 >( "alignent", "CFRULE-ALIGNMENT" );
                dumpHex< sal_uInt8 >( "rotation", "TEXTROTATION" );
                dumpHex< sal_uInt16 >( "indent", "CFRULE-INDENT" );
                dumpDec< sal_uInt16 >( "relative-indent" );
                dumpUnknown( 2 );
            }
            if( getFlag< sal_uInt32 >( nFlags, 0x10000000 ) )
            {
                writeEmptyItem( "border-block" );
                IndentGuard aIndGuard( out() );
                dumpHex< sal_uInt16 >( "border-style", "XF-BORDERSTYLE" );
                dumpHex< sal_uInt16 >( "border-color1", "XF-BORDERCOLOR1" );
                dumpHex< sal_uInt32 >( "border-color2", "CFRULE-BORDERCOLOR2" );
            }
            if( getFlag< sal_uInt32 >( nFlags, 0x20000000 ) )
            {
                writeEmptyItem( "pattern-block" );
                IndentGuard aIndGuard( out() );
                dumpHex< sal_uInt32 >( "pattern", "CFRULE-FILLBLOCK" );
            }
            if( getFlag< sal_uInt32 >( nFlags, 0x40000000 ) )
            {
                writeEmptyItem( "protection-block" );
                IndentGuard aIndGuard( out() );
                dumpHex< sal_uInt16 >( "flags", "CFRULE-PROTECTION-FLAGS" );
            }
            if( nFmla1Size > 0 )
                getFormulaDumper().dumpNameFormula( "formula1", nFmla1Size );
            if( nFmla2Size > 0 )
                getFormulaDumper().dumpNameFormula( "formula2", nFmla2Size );
        }
        break;

        case BIFF_ID_CH3DDATAFORMAT:
            dumpDec< sal_uInt8 >( "base", "CH3DDATAFORMAT-BASE" );
            dumpDec< sal_uInt8 >( "top", "CH3DDATAFORMAT-TOP" );
        break;

        case BIFF_ID_CHAREAFORMAT:
            dumpColorABGR( "fg-color" );
            dumpColorABGR( "bg-color" );
            dumpPatternIdx();
            dumpHex< sal_uInt16 >( "flags", "CHAREAFORMAT-FLAGS" );
            if( eBiff == BIFF8 ) dumpColorIdx( "fg-color-idx" );
            if( eBiff == BIFF8 ) dumpColorIdx( "bg-color-idx" );
        break;

        case BIFF_ID_CHAXESSET:
            dumpDec< sal_uInt16 >( "axesset-id", "CHAXESSET-ID" );
            dumpRect< sal_Int32 >( "position", (eBiff <= BIFF4) ? "CONV-TWIP-TO-CM" : "" );
        break;

        case BIFF_ID_CHAXIS:
            dumpDec< sal_uInt16 >( "axis-type", "CHAXIS-TYPE" );
            if( eBiff <= BIFF4 )
                dumpRect< sal_Int32 >( "position", "CONV-TWIP-TO-CM" );
            else
                dumpUnused( 16 );
        break;

        case BIFF_ID_CHBAR:
            dumpDec< sal_Int16 >( "overlap", "CONV-PERCENT-NEG" );
            dumpDec< sal_Int16 >( "gap", "CONV-PERCENT" );
            dumpHex< sal_uInt16 >( "flags", "CHBAR-FLAGS" );
        break;

        case BIFF_ID_CHCHART:
            dumpRect< sal_Int32 >( "chart-frame", "CONV-PT1616-TO-CM", FORMATTYPE_FIX );
        break;

        case BIFF_ID_CHCHART3D:
            dumpDec< sal_uInt16 >( "rotation-angle", "CONV-DEG" );
            dumpDec< sal_Int16 >( "elevation-angle", "CONV-DEG" );
            dumpDec< sal_uInt16 >( "eye-distance" );
            dumpDec< sal_uInt16 >( "relative-height", "CONV-PERCENT" );
            dumpDec< sal_uInt16 >( "relative-depth", "CONV-PERCENT" );
            dumpDec< sal_uInt16 >( "depth-gap", "CONV-PERCENT" );
            dumpHex< sal_uInt16 >( "flags", "CHCHART3D-FLAGS" );
        break;

        case BIFF_ID_CHTYPEGROUP:
            dumpUnused( 16 );
            dumpHex< sal_uInt16 >( "flags", "CHTYPEGROUP-FLAGS" );
            if( eBiff >= BIFF5 ) dumpDec< sal_uInt16 >( "group-idx" );
        break;

        case BIFF_ID_CHDATAFORMAT:
            dumpDec< sal_Int16 >( "point-idx", "CHDATAFORMAT-POINTIDX" );
            dumpDec< sal_Int16 >( "series-idx" );
            if( eBiff >= BIFF5 ) dumpDec< sal_Int16 >( "format-idx", "CHDATAFORMAT-FORMATIDX" );
            if( eBiff >= BIFF5 ) dumpHex< sal_uInt16 >( "flags", "CHDATAFORMAT-FLAGS" );
        break;

        case BIFF_ID_CHESCHERFORMAT:
            dumpEmbeddedDff();
        break;

        case BIFF_ID_CHFRAME:
            dumpDec< sal_uInt16 >( "format", "CHFRAME-FORMAT" );
            dumpHex< sal_uInt16 >( "flags", "CHFRAME-FLAGS" );
        break;

        case BIFF_ID_CHFRAMEPOS:
            dumpDec< sal_uInt16 >( "object-type", "CHFRAMEPOS-OBJTYPE" );
            dumpDec< sal_uInt16 >( "size-mode", "CHFRAMEPOS-SIZEMODE" );
            dumpRect< sal_Int32 >( "position", (eBiff <= BIFF4) ? "CONV-TWIP-TO-CM" : "" );
        break;

        case BIFF_ID_CHFRBLOCKBEGIN:
            dumpFrHeader( true, false );
            dumpDec< sal_uInt16 >( "type", "CHFRBLOCK-TYPE" );
            dumpDec< sal_uInt16 >( "context" );
            dumpDec< sal_uInt16 >( "value-1" );
            dumpDec< sal_uInt16 >( "value-2" );
        break;

        case BIFF_ID_CHFRBLOCKEND:
            dumpFrHeader( true, false );
            dumpDec< sal_uInt16 >( "type", "CHFRBLOCK-TYPE" );
            if( rStrm.getRemaining() >= 6 )
                dumpUnused( 6 );
        break;

        case BIFF_ID_CHFRINFO:
        {
            dumpFrHeader( true, false );
            dumpDec< sal_uInt8 >( "creator", "CHFRINFO-APPVERSION" );
            dumpDec< sal_uInt8 >( "writer", "CHFRINFO-APPVERSION" );
            sal_uInt16 nCount = dumpDec< sal_uInt16 >( "rec-range-count" );
            out().resetItemIndex();
            for( sal_uInt16 nIndex = 0; !rStrm.isEof() && (nIndex < nCount); ++nIndex )
                dumpHexPair< sal_uInt16 >( "#rec-range", '-' );
        }
        break;

        case BIFF_ID_CHFRLABELPROPS:
            dumpFrHeader( true, true );
            dumpHex< sal_uInt16 >( "flags", "CHFRLABELPROPS-FLAGS" );
            dumpUniString( "separator", BIFF_STR_SMARTFLAGS );
        break;

        case BIFF_ID_CHFRUNITPROPS:
            dumpFrHeader( true, false );
            dumpDec< sal_Int16 >( "preset", "CHFRUNITPROPS-PRESET" );
            dumpDec< double >( "unit" );
            dumpHex< sal_uInt16 >( "flags", "CHFRUNITPROPS-FLAGS" );
        break;

        case BIFF_ID_CHFRWRAPPER:
            dumpFrHeader( true, false );
        break;

        case BIFF_ID_CHLABELRANGE:
            dumpDec< sal_uInt16 >( "axis-crossing" );
            dumpDec< sal_uInt16 >( "label-frequency" );
            dumpDec< sal_uInt16 >( "tick-frequency" );
            dumpHex< sal_uInt16 >( "flags", "CHLABELRANGE-FLAGS" );
        break;

        case BIFF_ID_CHLABELRANGE2:
            dumpDec< sal_uInt16 >( "minimum-categ" );
            dumpDec< sal_uInt16 >( "maximum-categ" );
            dumpDec< sal_uInt16 >( "major-unit-value" );
            dumpDec< sal_uInt16 >( "major-unit" );
            dumpDec< sal_uInt16 >( "minor-unit-value" );
            dumpDec< sal_uInt16 >( "minor-unit" );
            dumpDec< sal_uInt16 >( "base-unit" );
            dumpDec< sal_uInt16 >( "axis-crossing-date" );
            dumpHex< sal_uInt16 >( "flags", "CHLABELRANGE2-FLAGS" );
        break;

        case BIFF_ID_CHLEGEND:
            dumpRect< sal_Int32 >( "position", (eBiff <= BIFF4) ? "CONV-TWIP-TO-CM" : "" );
            dumpDec< sal_uInt8 >( "docked-pos", "CHLEGEND-DOCKPOS" );
            dumpDec< sal_uInt8 >( "spacing", "CHLEGEND-SPACING" );
            dumpHex< sal_uInt16 >( "flags", "CHLEGEND-FLAGS" );
        break;

        case BIFF_ID_CHLINEFORMAT:
            dumpColorABGR();
            dumpDec< sal_uInt16 >( "line-type", "CHLINEFORMAT-LINETYPE" );
            dumpDec< sal_Int16 >( "line-weight", "CHLINEFORMAT-LINEWEIGHT" );
            dumpHex< sal_uInt16 >( "flags", "CHLINEFORMAT-FLAGS" );
            if( eBiff == BIFF8 ) dumpColorIdx();
        break;

        case BIFF_ID_CHMARKERFORMAT:
            dumpColorABGR( "border-color" );
            dumpColorABGR( "fill-color" );
            dumpDec< sal_uInt16 >( "marker-type", "CHMARKERFORMAT-TYPE" );
            dumpHex< sal_uInt16 >( "flags", "CHMARKERFORMAT-FLAGS" );
            if( eBiff == BIFF8 ) dumpColorIdx( "border-color-idx" );
            if( eBiff == BIFF8 ) dumpColorIdx( "fill-color-idx" );
            if( eBiff == BIFF8 ) dumpDec< sal_Int32 >( "marker-size", "CONV-TWIP-TO-PT" );
        break;

        case BIFF_ID_CHOBJECTLINK:
            dumpDec< sal_uInt16 >( "link-target", "CHOBJECTLINK-TARGET" );
            dumpDec< sal_Int16 >( "series-idx" );
            dumpDec< sal_Int16 >( "point-idx", "CHOBJECTLINK-POINT" );
        break;

        case BIFF_ID_CHPICFORMAT:
            dumpDec< sal_uInt16 >( "bitmap-mode", "CHPICFORMAT-BITMAP-MODE" );
            dumpDec< sal_uInt16 >( "image-format", "CHPICFORMAT-IMAGE-FORMAT" );
            dumpHex< sal_uInt16 >( "flags", "CHPICFORMAT-FLAGS" );
            dumpDec< double >( "scaling-factor" );
        break;

        case BIFF_ID_CHPIE:
            dumpDec< sal_uInt16 >( "angle", "CONV-DEG" );
            if( eBiff >= BIFF5 ) dumpDec< sal_uInt16 >( "hole-size" );
            if( eBiff >= BIFF8 ) dumpHex< sal_uInt16 >( "flags", "CHPIE-FLAGS" );
        break;

        case BIFF_ID_CHPLOTGROWTH:
            dumpFix< sal_Int32 >( "horizontal-growth" );
            dumpFix< sal_Int32 >( "vertical-growth" );
        break;

        case BIFF_ID_CHPROPERTIES:
            dumpHex< sal_uInt16 >( "flags", "CHPROPERTIES-FLAGS" );
            dumpDec< sal_uInt8 >( "empty-cells", "CHPROPERTIES-EMPTYCELLS" );
        break;

        case BIFF_ID_CHSCATTER:
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "bubble-size", "CONV-PERCENT" );
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "size-type", "CHSCATTER-SIZETYPE" );
            if( eBiff == BIFF8 ) dumpHex< sal_uInt16 >( "flags", "CHSCATTER-FLAGS" );
        break;

        case BIFF_ID_CHSERERRORBAR:
            dumpDec< sal_uInt8 >( "type", "CHSERERRORBAR-TYPE" );
            dumpDec< sal_uInt8 >( "source", "CHSERERRORBAR-SOURCE" );
            dumpBool< sal_uInt8 >( "draw-t-shape" );
            dumpBool< sal_uInt8 >( "draw-line" );
            dumpDec< double >( "value" );
            dumpDec< sal_uInt16 >( "custom-count" );
        break;

        case BIFF_ID_CHSERIES:
            dumpDec< sal_uInt16 >( "categories-type", "CHSERIES-TYPE" );
            dumpDec< sal_uInt16 >( "values-type", "CHSERIES-TYPE" );
            dumpDec< sal_uInt16 >( "categories-count" );
            dumpDec< sal_uInt16 >( "values-count" );
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "bubbles-type", "CHSERIES-TYPE" );
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "bubbles-count" );
        break;

        case BIFF_ID_CHSERTRENDLINE:
            switch( dumpDec< sal_uInt8 >( "type", "CHSERTRENDLINE-TYPE" ) )
            {
                case 0:     dumpDec< sal_uInt8 >( "order" );            break;
                case 4:     dumpDec< sal_uInt8 >( "average-period" );   break;
                default:    dumpUnused( 1 );
            }
            dumpDec< double >( "intercept" );
            dumpBool< sal_uInt8 >( "show-equation" );
            dumpBool< sal_uInt8 >( "show-r-sqrare" );
            dumpDec< double >( "forecast-forward" );
            dumpDec< double >( "forecast-backward" );
        break;

        case BIFF_ID_CHSOURCELINK:
            dumpDec< sal_uInt8 >( "link-target", "CHSOURCELINK-TARGET" );
            dumpDec< sal_uInt8 >( "link-type", "CHSOURCELINK-TYPE" );
            dumpHex< sal_uInt16 >( "flags", "CHSOURCELINK-FLAGS" );
            dumpFormatIdx();
            getFormulaDumper().dumpNameFormula();
        break;

        case BIFF_ID_CHSTRING:
            dumpDec< sal_uInt16 >( "text-type", "CHSTRING-TYPE" );
            dumpString( "text", BIFF_STR_8BITLENGTH, BIFF_STR_8BITLENGTH );
        break;

        case BIFF_ID_CHTEXT:
            dumpDec< sal_uInt8 >( "horizontal-align", "CHTEXT-HORALIGN" );
            dumpDec< sal_uInt8 >( "vertical-align", "CHTEXT-VERALIGN" );
            dumpDec< sal_uInt16 >( "fill-mode", "CHTEXT-FILLMODE" );
            dumpColorABGR();
            dumpRect< sal_Int32 >( "position", (eBiff <= BIFF4) ? "CONV-TWIP-TO-CM" : "" );
            dumpHex< sal_uInt16 >( "flags", "CHTEXT-FLAGS" );
            if( eBiff == BIFF8 ) dumpColorIdx();
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "placement", "CHTEXT-PLACEMENT" );
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "rotation", "TEXTROTATION" );
        break;

        case BIFF_ID_CHTICK:
            dumpDec< sal_uInt8 >( "major-ticks", "CHTICK-TYPE" );
            dumpDec< sal_uInt8 >( "minor-ticks", "CHTICK-TYPE" );
            dumpDec< sal_uInt8 >( "label-position", "CHTICK-LABELPOS" );
            dumpDec< sal_uInt8 >( "fill-mode", "CHTEXT-FILLMODE" );
            dumpColorABGR( "label-color" );
            dumpUnused( 16 );
            dumpHex< sal_uInt16 >( "flags", "CHTICK-FLAGS" );
            if( eBiff == BIFF8 ) dumpColorIdx( "label-color-idx" );
            if( eBiff == BIFF8 ) dumpDec< sal_uInt16 >( "label-rotation", "TEXTROTATION" );
        break;

        case BIFF_ID_CHVALUERANGE:
            dumpDec< double >( "minimum" );
            dumpDec< double >( "maximum" );
            dumpDec< double >( "major-inc" );
            dumpDec< double >( "minor-inc" );
            dumpDec< double >( "axis-crossing" );
            dumpHex< sal_uInt16 >( "flags", "CHVALUERANGE-FLAGS" );
        break;

        case BIFF_ID_CODENAME:
            dumpUniString( "codename" );
        break;

        case BIFF_ID_CODEPAGE:
            getBiffData().setTextEncoding( dumpCodePage() );
            mbHasCodePage = true;
        break;

        case BIFF_ID_COLINFO:
            dumpColRange();
            dumpDec< sal_uInt16 >( "col-width", "CONV-COLWIDTH" );
            dumpXfIdx( "xf-idx" );
            dumpHex< sal_uInt16 >( "flags", "COLINFO-FLAGS" );
            dumpUnused( 2 );
        break;

        case BIFF_ID_COLUMNDEFAULT:
            out().resetItemIndex();
            for( sal_Int32 nCol = 0, nCount = dumpColRange(); nCol < nCount; ++nCol )
                dumpXfIdx( "#xf-idx", true );
            dumpUnused( 2 );
        break;

        case BIFF_ID_COLWIDTH:
            dumpColRange( EMPTY_STRING, false );
            dumpDec< sal_uInt16 >( "col-width", "CONV-COLWIDTH" );
        break;

        case BIFF_ID_CONT:
            if( (eBiff == BIFF8) && (getLastRecId() == BIFF_ID_OBJ) )
                dumpEmbeddedDff();
        break;

        case BIFF_ID_COORDLIST:
        {
            out().resetItemIndex();
            TableGuard aTabGuard( out(), 12, 10 );
            while( rStrm.getRemaining() >= 4 )
            {
                MultiItemsGuard aMultiGuard( out() );
                writeEmptyItem( "#point" );
                dumpDec< sal_uInt16 >( "x" );
                dumpDec< sal_uInt16 >( "y" );
            }
        }
        break;

        case BIFF_ID_CRN:
        {
            sal_Int32 nCol2 = dumpColIndex( "last-col-idx", false );
            sal_Int32 nCol1 = dumpColIndex( "first-col-idx", false );
            sal_Int32 nRow = dumpRowIndex( "row-idx" );
            TableGuard aTabGuard( out(), 14, 17 );
            for( Address aPos( nCol1, nRow ); !rStrm.isEof() && (aPos.mnCol <= nCol2); ++aPos.mnCol )
            {
                MultiItemsGuard aMultiGuard( out() );
                writeAddressItem( "pos", aPos );
                dumpConstValue();
            }
        }
        break;

        case BIFF_ID_DCONBINAME:
            dumpDec< sal_uInt8 >( "builtin-id", "DEFINEDNAME-BUILTINID" );
            dumpUnused( 3 );
            dumpString( "source-link", BIFF_STR_8BITLENGTH, BIFF_STR_SMARTFLAGS );
        break;

        case BIFF_ID_DCONNAME:
            dumpString( "source-name", BIFF_STR_8BITLENGTH );
            dumpString( "source-link", BIFF_STR_8BITLENGTH, BIFF_STR_SMARTFLAGS );
        break;

        case BIFF_ID_DCONREF:
            dumpRange( "source-range", false );
            dumpString( "source-link", BIFF_STR_8BITLENGTH, BIFF_STR_SMARTFLAGS );
        break;

        case BIFF2_ID_DATATABLE:
            dumpRange( "table-range", false );
            dumpBoolean( "recalc-always" );
            dumpBoolean( "row-table" );
            dumpAddress( "ref1" );
        break;

        case BIFF3_ID_DATATABLE:
            dumpRange( "table-range", false );
            dumpHex< sal_uInt16 >( "flags", "DATATABLE-FLAGS" );
            dumpAddress( "ref1" );
            dumpAddress( "ref2" );
        break;

        case BIFF2_ID_DATATABLE2:
            dumpRange( "table-range", false );
            dumpBoolean( "recalc-always" );
            dumpUnused( 1 );
            dumpAddress( "ref1" );
            dumpAddress( "ref2" );
        break;

        case BIFF_ID_DATAVALIDATION:
        {
            dumpHex< sal_uInt32 >( "flags", "DATAVALIDATION-FLAGS" );
            dumpUniString( "input-title" );
            dumpUniString( "error-title" );
            dumpUniString( "input-message" );
            dumpUniString( "error-message" );
            sal_uInt16 nFmla1Size = getFormulaDumper().dumpFormulaSize( "formula1-size" );
            dumpUnused( 2 );
            if( nFmla1Size > 0 )
                getFormulaDumper().dumpNameFormula( "formula1", nFmla1Size );
            sal_uInt16 nFmla2Size = getFormulaDumper().dumpFormulaSize( "formula2-size" );
            dumpUnused( 2 );
            if( nFmla2Size > 0 )
                getFormulaDumper().dumpNameFormula( "formula2", nFmla2Size );
            dumpRangeList();
        }
        break;

        case BIFF_ID_DATAVALIDATIONS:
            dumpHex< sal_uInt16 >( "flags", "DATAVALIDATIONS-FLAGS" );
            dumpDec< sal_Int32 >( "input-box-pos-x" );
            dumpDec< sal_Int32 >( "input-box-pos-y" );
            dumpDec< sal_Int32 >( "dropdown-object-id" );
            dumpDec< sal_Int32 >( "dval-entry-count" );
        break;

        case BIFF2_ID_DEFINEDNAME:
        case BIFF3_ID_DEFINEDNAME:
        {
            rtl_TextEncoding eTextEnc = getBiffData().getTextEncoding();
            dumpHex< sal_uInt16, sal_uInt8 >( eBiff != BIFF2, "flags", "DEFINEDNAME-FLAGS" );
            if( eBiff == BIFF2 ) dumpDec< sal_uInt8 >( "macro-type", "DEFINEDNAME-MACROTYPE-BIFF2" );
            dumpChar( "accelerator", eTextEnc );
            sal_uInt8 nNameLen = dumpDec< sal_uInt8 >( "name-len" );
            sal_uInt16 nFmlaSize = getFormulaDumper().dumpFormulaSize();
            if( eBiff >= BIFF5 )
            {
                bool bBiff8 = eBiff == BIFF8;
                if( bBiff8 ) dumpUnused( 2 ); else dumpDec< sal_uInt16 >( "externsheet-idx", "DEFINEDNAME-SHEETIDX" );
                dumpDec< sal_uInt16 >( "sheet-idx", "DEFINEDNAME-SHEETIDX" );
                sal_uInt8 nMenuLen = dumpDec< sal_uInt8 >( "menu-text-len" );
                sal_uInt8 nDescrLen = dumpDec< sal_uInt8 >( "description-text-len" );
                sal_uInt8 nHelpLen = dumpDec< sal_uInt8 >( "help-text-len" );
                sal_uInt8 nStatusLen = dumpDec< sal_uInt8 >( "statusbar-text-len" );
                writeStringItem( "name", bBiff8 ? rStrm.readUniString( nNameLen ) : rStrm.readCharArray( nNameLen, eTextEnc ) );
                getFormulaDumper().dumpNameFormula( EMPTY_STRING, nFmlaSize );
                if( nMenuLen > 0 ) writeStringItem( "menu-text", bBiff8 ? rStrm.readUniString( nMenuLen ) : rStrm.readCharArray( nMenuLen, eTextEnc ) );
                if( nDescrLen > 0 ) writeStringItem( "description-text", bBiff8 ? rStrm.readUniString( nDescrLen ) : rStrm.readCharArray( nDescrLen, eTextEnc ) );
                if( nHelpLen > 0 ) writeStringItem( "help-text", bBiff8 ? rStrm.readUniString( nHelpLen ) : rStrm.readCharArray( nHelpLen, eTextEnc ) );
                if( nStatusLen > 0 ) writeStringItem( "statusbar-text", bBiff8 ? rStrm.readUniString( nStatusLen ) : rStrm.readCharArray( nStatusLen, eTextEnc ) );
            }
            else
            {
                writeStringItem( "name", rStrm.readCharArray( nNameLen, eTextEnc ) );
                getFormulaDumper().dumpNameFormula( EMPTY_STRING, nFmlaSize );
                if( eBiff == BIFF2 ) getFormulaDumper().dumpFormulaSize();
            }
        }
        break;

        case BIFF3_ID_DEFROWHEIGHT:
            dumpHex< sal_uInt16 >( "flags", "DEFROWHEIGHT-FLAGS" );
            dumpDec< sal_uInt16 >( "row-height", "CONV-TWIP-TO-PT" );
        break;

        case BIFF2_ID_DIMENSION:
        case BIFF3_ID_DIMENSION:
            dumpRange( "used-area", true, (nRecId == BIFF3_ID_DIMENSION) && (eBiff == BIFF8) );
            if( nRecId == BIFF3_ID_DIMENSION ) dumpUnused( 2 );
        break;

        case BIFF_ID_EXTERNALBOOK:
        {
            sal_uInt16 nCount = dumpDec< sal_uInt16 >( "sheet-count" );
            if( rStrm.getRemaining() == 2 )
                dumpHex< sal_uInt16 >( "special-key", "EXTERNALBOOK-KEY" );
            else
            {
                dumpString( "workbook-url" );
                out().resetItemIndex();
                for( sal_uInt16 nSheet = 0; !rStrm.isEof() && (nSheet < nCount); ++nSheet )
                    dumpString( "#sheet-name" );
            }
        }
        break;

        case BIFF2_ID_EXTERNALNAME:
        case BIFF3_ID_EXTERNALNAME:
        {
            sal_uInt16 nFlags = (eBiff >= BIFF3) ? dumpHex< sal_uInt16 >( "flags", "EXTERNALNAME-FLAGS" ) : 0;
            if( eBiff >= BIFF5 )
            {
                if( getFlag< sal_uInt16 >( nFlags, 0x0010 ) )
                {
                    dumpHex< sal_uInt32 >( "storage-id" );
                }
                else
                {
                    dumpDec< sal_uInt16 >( "externsheet-idx" );
                    dumpUnused( 2 );
                }
            }
            OUString aName = dumpString( "name", BIFF_STR_8BITLENGTH, BIFF_STR_8BITLENGTH );
            if( (aName.getLength() > 0) && (aName[ 0 ] == 1) && (rStrm.getRemaining() >= 2) )
                getFormulaDumper().dumpNameFormula();
        }
        break;

        case BIFF_ID_EXTERNSHEET:
            if( eBiff == BIFF8 )
            {
                sal_uInt16 nCount = dumpDec< sal_uInt16 >( "ref-count" );
                TableGuard aTabGuard( out(), 10, 17, 24 );
                out().resetItemIndex();
                for( sal_uInt16 nRefId = 0; !rStrm.isEof() && (nRefId < nCount); ++nRefId )
                {
                    MultiItemsGuard aMultiGuard( out() );
                    writeEmptyItem( "#ref" );
                    dumpDec< sal_uInt16 >( "extbook-idx" );
                    dumpDec< sal_Int16 >( "first-sheet", "EXTERNSHEET-IDX" );
                    dumpDec< sal_Int16 >( "last-sheet", "EXTERNSHEET-IDX" );
                }
            }
            else
            {
                OStringBuffer aUrl( rStrm.readByteString( false ) );
                if( (aUrl.getLength() > 0) && (aUrl[ 0 ] == '\x03') )
                    aUrl.append( static_cast< sal_Char >( rStrm.readuInt8() ) );
                writeStringItem( "encoded-url", OStringToOUString( aUrl.makeStringAndClear(), getBiffData().getTextEncoding() ) );
            }
        break;

        case BIFF2_ID_FONT:
        case BIFF3_ID_FONT:
            dumpFontRec();
        break;

        case BIFF2_ID_FORMAT:
        case BIFF4_ID_FORMAT:
            dumpFormatRec();
        break;

        case BIFF2_ID_FORMULA:
        case BIFF3_ID_FORMULA:
        case BIFF4_ID_FORMULA:
            dumpCellHeader( eBiff == BIFF2 );
            dumpFormulaResult();
            dumpHex< sal_uInt16, sal_uInt8 >( eBiff != BIFF2, "flags", "FORMULA-FLAGS" );
            if( eBiff >= BIFF5 ) dumpUnused( 4 );
            getFormulaDumper().dumpCellFormula();
        break;

        case BIFF_ID_FOOTER:
            if( rStrm.getRemaining() > 0 )
                dumpString( "footer", BIFF_STR_8BITLENGTH );
        break;

        case BIFF_ID_HEADER:
            if( rStrm.getRemaining() > 0 )
                dumpString( "header", BIFF_STR_8BITLENGTH );
        break;

        case BIFF_ID_HYPERLINK:
            dumpRange();
            if( cfg().getStringOption( dumpGuid( "guid" ), OUString() ).equalsAscii( "StdHlink" ) )
                StdHlinkObject( *this ).dump();
        break;

        case BIFF3_ID_IMGDATA:
        case BIFF8_ID_IMGDATA:
        {
            sal_uInt16 nFormat = dumpDec< sal_uInt16 >( "image-format", "IMGDATA-FORMAT" );
            dumpDec< sal_uInt16 >( "environment", "IMGDATA-ENV" );
            dumpDec< sal_uInt32 >( "data-size" );
            if( nFormat == 9 )
            {
                writeEmptyItem( "bitmap-header" );
                IndentGuard aIndGuard( out() );
                if( dumpDec< sal_uInt32 >( "header-size" ) == 12 )
                {
                    dumpDec< sal_Int16 >( "width" );
                    dumpDec< sal_Int16 >( "height" );
                    dumpDec< sal_Int16 >( "planes" );
                    dumpDec< sal_Int16 >( "bit-count" );
                }
            }
        }
        break;

        case BIFF2_ID_INTEGER:
            dumpCellHeader( true );
            dumpDec< sal_uInt16 >( "value" );
        break;

        case BIFF2_ID_LABEL:
        case BIFF3_ID_LABEL:
        {
            bool bBiff2 = nRecId == BIFF2_ID_LABEL;
            sal_uInt16 nXfIdx = dumpCellHeader( bBiff2 );
            rtl_TextEncoding eOldTextEnc = getBiffData().getTextEncoding();
            getBiffData().setTextEncoding( getBiffData().getXfEncoding( nXfIdx ) );
            dumpString( "value", bBiff2 ? BIFF_STR_8BITLENGTH : BIFF_STR_DEFAULT );
            getBiffData().setTextEncoding( eOldTextEnc );
        }
        break;

        case BIFF_ID_LABELRANGES:
            dumpRangeList( "row-ranges" );
            dumpRangeList( "col-ranges" );
        break;

        case BIFF_ID_LABELSST:
            dumpCellHeader();
            dumpDec< sal_Int32 >( "sst-idx" );
        break;

        case BIFF_ID_MSODRAWING:
        case BIFF_ID_MSODRAWINGGROUP:
        case BIFF_ID_MSODRAWINGSEL:
            dumpEmbeddedDff();
            mbHasDff = true;
        break;

        case BIFF_ID_MULTBLANK:
        {
            Address aPos = dumpAddress();
            {
                TableGuard aTabGuard( out(), 12 );
                for( ; rStrm.getRemaining() >= 4; ++aPos.mnCol )
                {
                    MultiItemsGuard aMultiGuard( out() );
                    writeAddressItem( "pos", aPos );
                    dumpXfIdx();
                }
            }
            dumpColIndex( "last-col-idx" );
        }
        break;

        case BIFF_ID_MULTRK:
        {
            Address aPos = dumpAddress();
            {
                TableGuard aTabGuard( out(), 12, 12 );
                for( ; rStrm.getRemaining() >= 8; ++aPos.mnCol )
                {
                    MultiItemsGuard aMultiGuard( out() );
                    writeAddressItem( "pos", aPos );
                    dumpXfIdx();
                    dumpRk( "value" );
                }
            }
            dumpColIndex( "last-col-idx" );
        }
        break;

        case BIFF_ID_NOTE:
            dumpAddress( "anchor-cell" );
            if( eBiff == BIFF8 )
            {
                dumpHex< sal_uInt16 >( "flags", "NOTE-FLAGS" );
                dumpDec< sal_uInt16 >( "obj-id" );
            }
            else
            {
                sal_uInt16 nTextLen = ::std::min( dumpDec< sal_uInt16 >( "text-len" ), static_cast< sal_uInt16 >( rStrm.getRemaining() ) );
                writeStringItem( "note-text", rStrm.readCharArray( nTextLen, getBiffData().getTextEncoding() ) );
            }
        break;

        case BIFF2_ID_NUMBER:
        case BIFF3_ID_NUMBER:
            dumpCellHeader( nRecId == BIFF2_ID_NUMBER );
            dumpDec< double >( "value" );
        break;

        case BIFF_ID_OBJ:
            dumpObjRec();
        break;

        case BIFF_ID_PAGESETUP:
            dumpDec< sal_uInt16 >( "paper-size", "PAGESETUP-PAPERSIZE" );
            dumpDec< sal_uInt16 >( "scaling", "CONV-PERCENT" );
            dumpDec< sal_uInt16 >( "first-page" );
            dumpDec< sal_uInt16 >( "scale-to-width", "PAGESETUP-SCALETOPAGES" );
            dumpDec< sal_uInt16 >( "scale-to-height", "PAGESETUP-SCALETOPAGES" );
            dumpHex< sal_uInt16 >( "flags", "PAGESETUP-FLAGS" );
            if( eBiff >= BIFF5 )
            {
                dumpDec< sal_uInt16 >( "horizontal-res", "PAGESETUP-DPI" );
                dumpDec< sal_uInt16 >( "vertical-res", "PAGESETUP-DPI" );
                dumpDec< double >( "header-margin", "CONV-INCH-TO-CM" );
                dumpDec< double >( "footer-margin", "CONV-INCH-TO-CM" );
                dumpDec< sal_uInt16 >( "copies" );
            }
        break;

        case BIFF_ID_PANE:
            dumpDec< sal_uInt16 >( "x-pos", "CONV-TWIP-TO-CM" );
            dumpDec< sal_uInt16 >( "y-pos", "CONV-TWIP-TO-CM" );
            dumpAddress( "first-visible-cell" );
            dumpDec< sal_uInt8 >( "active-pane", "PANE-ID" );
        break;

        case BIFF_ID_PCITEM_STRING:
            dumpString( "value" );
        break;

        case BIFF_ID_PHONETICPR:
            dumpDec< sal_uInt16 >( "font-id", "FONTNAMES" );
            dumpHex< sal_uInt16 >( "flags", "PHONETICPR-FLAGS" );
            dumpRangeList( "show-phonetic" );
        break;

        case BIFF_ID_PROJEXTSHEET:
            dumpDec< sal_uInt8 >( "sheet-type", "PROJEXTSHEET-TYPE" );
            dumpUnused( 1 );
            dumpByteString( "sheet-link", BIFF_STR_8BITLENGTH );
        break;

        case BIFF_ID_PTDATAFIELD:
            dumpDec< sal_Int16 >( "field" );
            dumpDec< sal_uInt16 >( "subtotal", "PTDATAFIELD-SUBTOTAL" );
            dumpDec< sal_uInt16 >( "show-data-as", "PTDATAFIELD-SHOWDATAAS" );
            dumpDec< sal_Int16 >( "base-field" );
            dumpDec< sal_Int16 >( "base-item", "PTDATAFIELD-BASEITEM" );
            dumpFormatIdx();
            dumpPivotString( "name" );
        break;

        case BIFF_ID_PTDEFINITION:
        {
            dumpRange( "output-range" );
            dumpRowIndex( "first-header-row-idx" );
            dumpAddress( "first-data-pos" );
            dumpDec< sal_uInt16 >( "cache-idx" );
            dumpUnused( 2 );
            dumpDec< sal_uInt16 >( "default-data-axis", "PTFIELD-AXISTYPE" );
            dumpDec< sal_Int16 >( "default-data-pos", "PTDEFINITION-DATAFIELD-POS" );
            dumpDec< sal_uInt16 >( "field-count" );
            mnPTRowFields = dumpDec< sal_uInt16 >( "row-field-count" );
            mnPTColFields = dumpDec< sal_uInt16 >( "column-field-count" );
            dumpDec< sal_uInt16 >( "page-field-count" );
            dumpDec< sal_uInt16 >( "data-field-count" );
            dumpDec< sal_uInt16 >( "data-row-count" );
            dumpDec< sal_uInt16 >( "data-column-count" );
            dumpHex< sal_uInt16 >( "flags", "PTDEFINITION-FLAGS" );
            dumpDec< sal_uInt16 >( "auto-format-idx" );
            sal_uInt16 nTabNameLen = dumpDec< sal_uInt16 >( "table-name-len" );
            sal_uInt16 nDataNameLen = dumpDec< sal_uInt16 >( "data-name-len" );
            dumpPivotString( "table-name", nTabNameLen );
            dumpPivotString( "data-name", nDataNameLen );
            mnPTRowColItemsIdx = 0;
        }
        break;

        case BIFF_ID_PTDEFINITION2:
        {
            dumpDec< sal_uInt16 >( "format-rec-count" );
            sal_uInt16 nErrCaptLen = dumpDec< sal_uInt16 >( "error-caption-len" );
            sal_uInt16 nMissCaptLen = dumpDec< sal_uInt16 >( "missing-caption-len" );
            sal_uInt16 nTagLen = dumpDec< sal_uInt16 >( "tag-len" );
            dumpDec< sal_uInt16 >( "select-rec-count" );
            dumpDec< sal_uInt16 >( "page-rows" );
            dumpDec< sal_uInt16 >( "page-cols" );
            dumpHex< sal_uInt32 >( "flags", "PTDEFINITION2-FLAGS" );
            sal_uInt16 nPageStyleLen = dumpDec< sal_uInt16 >( "page-field-style-len" );
            sal_uInt16 nTabStyleLen = dumpDec< sal_uInt16 >( "pivot-table-style-len" );
            sal_uInt16 nVacStyleLen = dumpDec< sal_uInt16 >( "vacated-style-len" );
            dumpPivotString( "error-caption", nErrCaptLen );
            dumpPivotString( "missing-caption", nMissCaptLen );
            dumpPivotString( "tag", nTagLen );
            dumpPivotString( "page-field-style", nPageStyleLen );
            dumpPivotString( "pivot-table-style", nTabStyleLen );
            dumpPivotString( "vacated-style", nVacStyleLen );
        }
        break;

        case BIFF_ID_PTFIELD:
            dumpDec< sal_uInt16 >( "axis-type", "PTFIELD-AXISTYPE" );
            dumpDec< sal_uInt16 >( "subtotal-count" );
            dumpHex< sal_uInt16 >( "subtotals", "PTFIELD-SUBTOTALS" );
            dumpDec< sal_uInt16 >( "item-count" );
            dumpPivotString( "field-name" );
        break;

        case BIFF_ID_PTFIELD2:
            dumpHex< sal_uInt32 >( "flags", "PTFIELD2-FLAGS" );
            dumpDec< sal_Int16 >( "autosort-basefield-idx" );
            dumpDec< sal_Int16 >( "autoshow-basefield-idx" );
            dumpFormatIdx();
            if( rStrm.getRemaining() >= 2 )
            {
                sal_uInt16 nFuncNameLen = dumpDec< sal_uInt16 >( "subtotal-func-name-len" );
                dumpUnused( 8 );
                dumpPivotString( "subtotal-func-name", nFuncNameLen );
            }
        break;

        case BIFF_ID_PTFITEM:
            dumpDec< sal_uInt16 >( "item-type", "PTFITEM-ITEMTYPE" );
            dumpHex< sal_uInt16 >( "flags", "PTFITEM-FLAGS" );
            dumpDec< sal_Int16 >( "cache-idx", "PTFITEM-CACHEIDX" );
            dumpPivotString( "item-name" );
        break;

        case BIFF_ID_PTPAGEFIELDS:
        {
            out().resetItemIndex();
            TableGuard aTabGuard( out(), 17, 17, 17 );
            while( rStrm.getRemaining() >= 6 )
            {
                writeEmptyItem( "#page-field" );
                MultiItemsGuard aMultiGuard( out() );
                IndentGuard aIndGuard( out() );
                dumpDec< sal_Int16 >( "base-field" );
                dumpDec< sal_Int16 >( "item", "PTPAGEFIELDS-ITEM" );
                dumpDec< sal_uInt16 >( "dropdown-obj-id" );
            }
        }
        break;

        case BIFF_ID_PTROWCOLFIELDS:
            out().resetItemIndex();
            for( sal_Int64 nIdx = 0, nCount = rStrm.getRemaining() / 2; nIdx < nCount; ++nIdx )
                dumpDec< sal_Int16 >( "#field-idx" );
        break;

        case BIFF_ID_PTROWCOLITEMS:
            if( mnPTRowColItemsIdx < 2 )
            {
                sal_uInt16 nCount = (mnPTRowColItemsIdx == 0) ? mnPTRowFields : mnPTColFields;
                sal_Int64 nLineSize = 8 + 2 * nCount;
                out().resetItemIndex();
                while( rStrm.getRemaining() >= nLineSize )
                {
                    writeEmptyItem( "#line-data" );
                    IndentGuard aIndGuard( out() );
                    MultiItemsGuard aMultiGuard( out() );
                    dumpDec< sal_uInt16 >( "ident-count" );
                    dumpDec< sal_uInt16 >( "item-type", "PTROWCOLITEMS-ITEMTYPE" );
                    dumpDec< sal_uInt16 >( "used-count" );
                    dumpHex< sal_uInt16 >( "flags", "PTROWCOLITEMS-FLAGS" );
                    OUStringBuffer aItemList;
                    for( sal_uInt16 nIdx = 0; nIdx < nCount; ++nIdx )
                        StringHelper::appendToken( aItemList, in().readInt16() );
                    writeInfoItem( "item-idxs", aItemList.makeStringAndClear() );
                }
                ++mnPTRowColItemsIdx;
            }
        break;

        case BIFF_ID_RK:
            dumpCellHeader();
            dumpRk( "value" );
        break;

        case BIFF2_ID_ROW:
        {
            dumpRowIndex();
            dumpColIndex( "first-used-col-idx" );
            dumpColIndex( "first-free-col-idx" );
            dumpHex< sal_uInt16 >( "height", "ROW-HEIGHT" );
            dumpUnused( 2 );
            bool bHasDefXf = dumpBool< sal_uInt8 >( "custom-format" );
            dumpDec< sal_uInt16 >( "cell-offset" );
            if( bHasDefXf ) dumpXfIdx( "custom-format", true );
            if( bHasDefXf ) dumpXfIdx( "custom-xf-idx", false );
        }
        break;

        case BIFF3_ID_ROW:
            dumpRowIndex();
            dumpColIndex( "first-used-col-idx" );
            dumpColIndex( "first-free-col-idx" );
            dumpHex< sal_uInt16 >( "height", "ROW-HEIGHT" );
            dumpUnused( (eBiff <= BIFF4) ? 2 : 4 );
            if( eBiff <= BIFF4 ) dumpDec< sal_uInt16 >( "cell-offset" );
            dumpHex< sal_uInt32 >( "flags", "ROW-FLAGS" );
        break;

        case BIFF_ID_RSTRING:
        {
            sal_uInt16 nXfIdx = dumpCellHeader();
            rtl_TextEncoding eOldTextEnc = getBiffData().getTextEncoding();
            getBiffData().setTextEncoding( getBiffData().getXfEncoding( nXfIdx ) );
            dumpString( "value" );
            getBiffData().setTextEncoding( eOldTextEnc );
            FontPortionModelList aPortions;
            aPortions.importPortions( rStrm, eBiff == BIFF8 );
            writeFontPortions( aPortions );
        }
        break;

        case BIFF_ID_SCL:
        {
            sal_uInt16 nNum = dumpDec< sal_uInt16 >( "numerator" );
            sal_uInt16 nDen = dumpDec< sal_uInt16 >( "denominator" );
            if( nDen > 0 ) writeDecItem( "current-zoom", static_cast< sal_uInt16 >( nNum * 100 / nDen ), "CONV-PERCENT" );
        }
        break;

        case BIFF_ID_SCREENTIP:
            dumpFrHeader( false, true );
            dumpNullUnicodeArray( "tooltip" );
        break;

        case BIFF_ID_SELECTION:
            dumpDec< sal_uInt8 >( "pane", "PANE-ID" );
            dumpAddress( "active-cell" );
            dumpDec< sal_uInt16 >( "list-idx" );
            dumpRangeList( "selection", false );
        break;

        case BIFF_ID_SHAREDFMLA:
            dumpRange( "formula-range", false );
            dumpUnused( 1 );
            dumpDec< sal_uInt8 >( "cell-count" );
            getFormulaDumper().dumpCellFormula();
        break;

        case BIFF_ID_SHEET:
            if( eBiff >= BIFF5 ) dumpHex< sal_uInt32 >( "sheet-stream-pos", "CONV-DEC" );
            if( eBiff >= BIFF5 ) dumpDec< sal_uInt8 >( "sheet-state", "SHEET-STATE" );
            if( eBiff >= BIFF5 ) dumpDec< sal_uInt8 >( "sheet-type", "SHEET-TYPE" );
            dumpString( "sheet-name", BIFF_STR_8BITLENGTH, BIFF_STR_8BITLENGTH );
        break;

        case BIFF_ID_SHEETHEADER:
            dumpHex< sal_uInt32 >( "substream-size", "CONV-DEC" );
            dumpByteString( "sheet-name", BIFF_STR_8BITLENGTH );
        break;

        case BIFF_ID_SHEETPROTECTION:
            dumpFrHeader( true, true );
            dumpUnused( 7 );
            dumpHex< sal_uInt16 >( "allowed-flags", "SHEETPROTECTION-FLAGS" );
            dumpUnused( 2 );
        break;

        case BIFF_ID_SST:
            dumpDec< sal_uInt32 >( "string-cell-count" );
            dumpDec< sal_uInt32 >( "sst-size" );
            out().resetItemIndex();
            while( !rStrm.isEof() && (rStrm.getRemaining() >= 3) )
                dumpUniString( "#entry" );
        break;

        case BIFF2_ID_STRING:
        case BIFF3_ID_STRING:
            dumpString( "result", ((nRecId == BIFF2_ID_STRING) && (eBiff <= BIFF4)) ? BIFF_STR_8BITLENGTH : BIFF_STR_DEFAULT );
        break;

        case BIFF_ID_STYLE:
        {
            sal_uInt16 nFlags = dumpHex< sal_uInt16 >( "flags", "STYLE-FLAGS" );
            if( getFlag( nFlags, BIFF_STYLE_BUILTIN ) )
            {
                dumpDec< sal_uInt8 >( "builtin-idx", "STYLE-BUILTIN" );
                dumpDec< sal_uInt8 >( "outline-level" );
            }
            else
                dumpString( "style-name", BIFF_STR_8BITLENGTH );
        }
        break;

        case BIFF_ID_SXEXT:
            if( eBiff == BIFF8 )
            {
                dumpHex< sal_uInt16 >( "flags", "SXEXT-FLAGS" );
                dumpDec< sal_uInt16 >( "param-string-count" );
                dumpDec< sal_uInt16 >( "sql-statement-string-count" );
                dumpDec< sal_uInt16 >( "webquery-postmethod-string-count" );
                dumpDec< sal_uInt16 >( "server-pagefields-string-count" );
                dumpDec< sal_uInt16 >( "odbc-connection-string-count" );
            }
        break;

        case BIFF_ID_TXO:
            dumpHex< sal_uInt16 >( "flags", "TXO-FLAGS" );
            dumpDec< sal_uInt16 >( "orientation", "TEXTORIENTATION" );
            dumpHex< sal_uInt16 >( "button-flags", "OBJ-BUTTON-FLAGS" );
            dumpUnicode( "accelerator" );
            dumpUnicode( "fareast-accelerator" );
            dumpDec< sal_uInt16 >( "text-len" );
            dumpDec< sal_uInt16 >( "format-run-size" );
            dumpUnused( 4 );
        break;

        case BIFF_ID_WINDOW1:
            dumpDec< sal_uInt16 >( "window-x", "CONV-TWIP-TO-CM" );
            dumpDec< sal_uInt16 >( "window-y", "CONV-TWIP-TO-CM" );
            dumpDec< sal_uInt16 >( "window-width", "CONV-TWIP-TO-CM" );
            dumpDec< sal_uInt16 >( "window-height", "CONV-TWIP-TO-CM" );
            if( eBiff <= BIFF4 )
            {
                dumpBool< sal_uInt8 >( "hidden" );
            }
            else
            {
                dumpHex< sal_uInt16 >( "flags", "WINDOW1-FLAGS" );
                dumpDec< sal_uInt16 >( "active-tab" );
                dumpDec< sal_uInt16 >( "first-visible-tab" );
                dumpDec< sal_uInt16 >( "selected-tabs" );
                dumpDec< sal_uInt16 >( "tabbar-ratio", "WINDOW1-TABBARRATIO" );
            }
        break;

        case BIFF2_ID_WINDOW2:
            dumpBool< sal_uInt8 >( "show-formulas" );
            dumpBool< sal_uInt8 >( "show-gridlines" );
            dumpBool< sal_uInt8 >( "show-headings" );
            dumpBool< sal_uInt8 >( "frozen-panes" );
            dumpBool< sal_uInt8 >( "show-zeros" );
            dumpAddress( "first-visible-cell" );
            dumpBool< sal_uInt8 >( "auto-grid-color" );
            dumpColorABGR( "grid-color" );
        break;

        case BIFF3_ID_WINDOW2:
            dumpHex< sal_uInt16 >( "flags", "WINDOW2-FLAGS" );
            dumpAddress( "first-visible-cell" );
            if( eBiff == BIFF8 )
            {
                dumpColorIdx( "grid-color-idx" );
                dumpUnused( 2 );
                if( rStrm.getRemaining() >= 8 )
                {
                    dumpDec< sal_uInt16 >( "pagebreak-zoom", "CONV-PERCENT" );
                    dumpDec< sal_uInt16 >( "normal-zoom", "CONV-PERCENT" );
                    dumpUnused( 4 );
                }
            }
            else
                dumpColorABGR( "grid-color" );
        break;

        case BIFF_ID_XCT:
            dumpDec< sal_uInt16 >( "crn-count" );
            if( eBiff == BIFF8 ) dumpDec< sal_Int16 >( "sheet-idx" );
        break;

        case BIFF2_ID_XF:
        case BIFF3_ID_XF:
        case BIFF4_ID_XF:
        case BIFF5_ID_XF:
            dumpXfRec();
        break;
    }
}

void WorkbookStreamObject::initializePerSheet()
{
    getBiffData().initializePerSheet();
    mxFontNames = cfg().createNameList< ConstList >( "FONTNAMES" );
    mxFontNames->setName( 0, createFontName( CREATE_OUSTRING( "Arial" ), 200, false, false ) );
    mxFormats = cfg().createNameList< ConstList >( "FORMATS" );
    mxFormats->includeList( cfg().getNameList( "BUILTIN-FORMATS" ) );
    mnFormatIdx = 0;
    mbHasCodePage = false;
}

OUString WorkbookStreamObject::createFontName( const OUString& rName, sal_uInt16 nHeight, bool bBold, bool bItalic ) const
{
    OUStringBuffer aName( rName );
    StringHelper::enclose( aName, OOX_DUMP_STRQUOTE );
    StringHelper::appendToken( aName, cfg().getName( "CONV-TWIP-TO-PT", nHeight ), ',' );
    if( bBold )
        StringHelper::appendToken( aName, CREATE_OUSTRING( "bold" ), ',' );
    if( bItalic )
        StringHelper::appendToken( aName, CREATE_OUSTRING( "italic" ), ',' );
    return aName.makeStringAndClear();
}

sal_uInt16 WorkbookStreamObject::dumpPatternIdx( const String& rName, bool b16Bit )
{
    return dumpDec< sal_uInt16, sal_uInt8 >( b16Bit, rName( "fill-pattern" ), mxFillPatterns );
}

sal_uInt16 WorkbookStreamObject::dumpColorIdx( const String& rName, bool b16Bit )
{
    return dumpDec< sal_uInt16, sal_uInt8 >( b16Bit, rName( "color-idx" ), mxColors );
}

sal_uInt16 WorkbookStreamObject::dumpFontIdx( const String& rName, bool b16Bit )
{
    return dumpDec< sal_uInt16, sal_uInt8 >( b16Bit, rName( "font-idx" ), "FONTNAMES" );
}

sal_uInt16 WorkbookStreamObject::dumpFormatIdx( const String& rName )
{
    return dumpDec< sal_uInt16, sal_uInt8 >( getBiff() >= BIFF5, rName( "fmt-idx" ), "FORMATS" );
}

sal_uInt16 WorkbookStreamObject::dumpXfIdx( const String& rName, bool bBiff2Style )
{
    String aName = rName( "xf-idx" );
    sal_uInt16 nXfIdx = 0;
    if( bBiff2Style )
    {
        dumpHex< sal_uInt8 >( aName, "CELL-XFINDEX" );
        dumpHex< sal_uInt8 >( "fmt-font-idx", "CELL-XFFORMAT" );
        dumpHex< sal_uInt8 >( "style", "CELL-XFSTYLE" );
    }
    else
        nXfIdx = dumpDec< sal_uInt16 >( aName );
    return nXfIdx;
}

OUString WorkbookStreamObject::dumpPivotString( const String& rName, sal_uInt16 nStrLen )
{
    OUString aString;
    if( nStrLen != BIFF_PT_NOSTRING )
    {
        aString = (getBiff() == BIFF8) ?
            getBiffStream().readUniString( nStrLen ) :
            getBiffStream().readCharArray( nStrLen, getBiffData().getTextEncoding() );
        writeStringItem( rName, aString );
    }
    return aString;
}

OUString WorkbookStreamObject::dumpPivotString( const String& rName )
{
    sal_uInt16 nStrLen = dumpDec< sal_uInt16 >( "string-len", "PIVOT-NAMELEN" );
    return dumpPivotString( rName, nStrLen );
}

sal_uInt16 WorkbookStreamObject::dumpCellHeader( bool bBiff2Style )
{
    dumpAddress();
    return dumpXfIdx( EMPTY_STRING, bBiff2Style );
}

void WorkbookStreamObject::dumpBoolErr()
{
    MultiItemsGuard aMultiGuard( out() );
    sal_uInt8 nValue = dumpHex< sal_uInt8 >( "value" );
    bool bErrCode = dumpBool< sal_uInt8 >( "is-errorcode" );
    if( bErrCode )
        writeErrorCodeItem( "errorcode", nValue );
    else
        writeBooleanItem( "boolean", nValue );
}

void WorkbookStreamObject::dumpFontRec()
{
    sal_uInt16 nFontId = getBiffData().getFontCount();
    out().resetItemIndex( nFontId );
    writeEmptyItem( "#font" );
    sal_uInt16 nHeight = dumpDec< sal_uInt16 >( "height", "CONV-TWIP-TO-PT" );
    sal_uInt16 nFlags = dumpHex< sal_uInt16 >( "flags", "FONT-FLAGS" );
    bool bBold = getFlag( nFlags, BIFF_FONTFLAG_BOLD );
    bool bItalic = getFlag( nFlags, BIFF_FONTFLAG_ITALIC );
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( getBiff() >= BIFF3 )
        dumpColorIdx();
    if( getBiff() >= BIFF5 )
    {
        bBold = dumpDec< sal_uInt16 >( "weight", "FONT-WEIGHT" ) > 450;
        dumpDec< sal_uInt16 >( "escapement", "FONT-ESCAPEMENT" );
        dumpDec< sal_uInt8 >( "underline", "FONT-UNDERLINE" );
        dumpDec< sal_uInt8 >( "family", "FONT-FAMILY" );
        sal_uInt8 nCharSet = dumpDec< sal_uInt8 >( "charset", "CHARSET" );
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( nCharSet );
        dumpUnused( 1 );
    }
    OUString aName = dumpString( "name", BIFF_STR_8BITLENGTH, BIFF_STR_8BITLENGTH );

    // append font data to vector
    mxFontNames->setName( nFontId, createFontName( aName, nHeight, bBold, bItalic ) );

    // store font encoding
    getBiffData().appendFontEncoding( eFontEnc );

    // set font encoding as default text encoding in case of missing CODEPAGE record
    if( !mbHasCodePage && (nFontId == 0) )
        getBiffData().setTextEncoding( eFontEnc );
}

void WorkbookStreamObject::dumpFormatRec()
{
    sal_uInt16 nFormatIdx = 0;
    switch( getBiff() )
    {
        case BIFF2:
        case BIFF3:
            nFormatIdx = mnFormatIdx++;
            out().resetItemIndex( nFormatIdx );
            writeEmptyItem( "#fmt" );
        break;
        case BIFF4:
            nFormatIdx = mnFormatIdx++;
            out().resetItemIndex( nFormatIdx );
            writeEmptyItem( "#fmt" );
            dumpUnused( 2 );
        break;
        case BIFF5:
        case BIFF8:
            getBiffStream() >> nFormatIdx;
            out().resetItemIndex( nFormatIdx );
            writeEmptyItem( "#fmt" );
            writeDecItem( "fmt-idx", nFormatIdx );
        break;
        case BIFF_UNKNOWN: break;
    }
    OUString aFormat = dumpString( "format", BIFF_STR_8BITLENGTH );
    mxFormats->setName( nFormatIdx, aFormat );
}

void WorkbookStreamObject::dumpXfRec()
{
    sal_uInt16 nXfId = getBiffData().getXfCount();
    out().resetItemIndex( nXfId );
    writeEmptyItem( "#xf" );
    sal_uInt16 nFontId = dumpFontIdx( EMPTY_STRING, getBiff() >= BIFF5 );
    switch( getBiff() )
    {
        case BIFF2:
            dumpUnused( 1 );
            dumpHex< sal_uInt8 >( "type-flags", "XF-TYPEFLAGS" );
            dumpHex< sal_uInt8 >( "style-flags", "XF-STYLEFLAGS" );
        break;
        case BIFF3:
            dumpFormatIdx();
            dumpHex< sal_uInt8 >( "type-flags", "XF-TYPEFLAGS" );
            dumpHex< sal_uInt8 >( "used-attributes", "XF-USEDATTRIBS-FLAGS" );
            dumpHex< sal_uInt16 >( "alignment", "XF-ALIGNMENT" );
            dumpHex< sal_uInt16 >( "fill-style", "XF-FILL" );
            dumpHex< sal_uInt32 >( "border-style", "XF-BORDER" );
        break;
        case BIFF4:
            dumpFormatIdx();
            dumpHex< sal_uInt16 >( "type-flags", "XF-TYPEFLAGS" );
            dumpHex< sal_uInt8 >( "alignment", "XF-ALIGNMENT" );
            dumpHex< sal_uInt8 >( "used-attributes", "XF-USEDATTRIBS-FLAGS" );
            dumpHex< sal_uInt16 >( "fill-style", "XF-FILL" );
            dumpHex< sal_uInt32 >( "border-style", "XF-BORDER" );
        break;
        case BIFF5:
            dumpFormatIdx();
            dumpHex< sal_uInt16 >( "type-flags", "XF-TYPEFLAGS" );
            dumpHex< sal_uInt8 >( "alignment", "XF-ALIGNMENT" );
            dumpHex< sal_uInt8 >( "orientation", "XF-ORIENTATTRIBS" );
            dumpHex< sal_uInt32 >( "fill-style", "XF-FILL" );
            dumpHex< sal_uInt32 >( "border-style", "XF-BORDER" );
        break;
        case BIFF8:
            dumpFormatIdx();
            dumpHex< sal_uInt16 >( "type-flags", "XF-TYPEFLAGS" );
            dumpHex< sal_uInt8 >( "alignment", "XF-ALIGNMENT" );
            dumpDec< sal_uInt8 >( "rotation", "TEXTROTATION" );
            dumpHex< sal_uInt8 >( "text-flags", "XF-TEXTFLAGS" );
            dumpHex< sal_uInt8 >( "used-attributes", "XF-USEDATTRIBS-FLAGS" );
            dumpHex< sal_uInt16 >( "border-style", "XF-BORDERSTYLE" );
            dumpHex< sal_uInt16 >( "border-color1", "XF-BORDERCOLOR1" );
            dumpHex< sal_uInt32 >( "border-color2", "XF-BORDERCOLOR2" );
            dumpHex< sal_uInt16 >( "fill-color", "XF-FILLCOLOR" );
        break;
        case BIFF_UNKNOWN: break;
    }
    getBiffData().appendXfFontId( nFontId );
}

void WorkbookStreamObject::dumpObjRec()
{
    switch( getBiff() )
    {
        case BIFF3:
            dumpObjRecBiff3();
        break;
        case BIFF4:
            dumpObjRecBiff4();
        break;
        case BIFF5:
            dumpObjRecBiff5();
        break;
        case BIFF8:
            // #i61786# OBJ records without DFF stream are in BIFF5 format
            if( mbHasDff ) dumpObjRecBiff8(); else dumpObjRecBiff5();
        break;
        default:;
    }
}

void WorkbookStreamObject::dumpObjRecBiff3()
{
    dumpDec< sal_uInt32 >( "obj-count" );
    sal_uInt16 nObjType = dumpDec< sal_uInt16 >( "obj-type", "OBJ-TYPE" );
    dumpDec< sal_uInt16 >( "obj-id" );
    dumpHex< sal_uInt16 >( "flags", "OBJ-FLAGS" );
    dumpDffClientRect();
    sal_uInt16 nMacroSize = dumpDec< sal_uInt16 >( "macro-size" );
    dumpUnused( 2 );
    sal_uInt16 nTextLen = 0, nFormatSize = 0, nLinkSize = 0;
    switch( nObjType )
    {
        case BIFF_OBJTYPE_GROUP:
            dumpUnused( 4 );
            dumpDec< sal_uInt16 >( "next-ungrouped-id" );
            dumpUnused( 16 );
            dumpObjRecString( "macro", nMacroSize, true );
        break;
        case BIFF_OBJTYPE_LINE:
            dumpObjRecLineData();
            dumpHex< sal_uInt16 >( "line-end", "OBJ-LINEENDS" );
            dumpDec< sal_uInt8 >( "line-direction", "OBJ-LINEDIR" );
            dumpUnused( 1 );
            dumpObjRecString( "macro", nMacroSize, true );
        break;
        case BIFF_OBJTYPE_RECTANGLE:
        case BIFF_OBJTYPE_OVAL:
            dumpObjRecRectData();
            dumpObjRecString( "macro", nMacroSize, true );
        break;
        case BIFF_OBJTYPE_ARC:
            dumpObjRecFillData();
            dumpObjRecLineData();
            dumpDec< sal_uInt8 >( "quadrant", "OBJ-ARC-QUADRANT" );
            dumpUnused( 1 );
            dumpObjRecString( "macro", nMacroSize, true );
        break;
        case BIFF_OBJTYPE_CHART:
            dumpObjRecRectData();
            dumpUnused( 18 );
            dumpObjRecString( "macro", nMacroSize, true );
        break;
        case BIFF_OBJTYPE_TEXT:
        case BIFF_OBJTYPE_BUTTON:
            dumpObjRecRectData();
            dumpObjRecTextDataBiff3( nTextLen, nFormatSize );
            dumpObjRecString( "macro", nMacroSize, true );
            dumpObjRecString( "text", nTextLen, false );
            dumpObjRecTextFmt( nFormatSize );
        break;
        case BIFF_OBJTYPE_PICTURE:
            dumpObjRecRectData();
            dumpDec< sal_Int16 >( "image-format", "IMGDATA-FORMAT" );
            dumpUnused( 4 );
            nLinkSize = dumpDec< sal_uInt16 >( "pic-link-size" );
            dumpUnused( 2 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-PICTURE-FLAGS" );
            dumpObjRecString( "macro", nMacroSize, true );
            dumpObjRecPictFmla( nLinkSize );
        break;
    }
}

void WorkbookStreamObject::dumpObjRecBiff4()
{
    dumpDec< sal_uInt32 >( "obj-count" );
    sal_uInt16 nObjType = dumpDec< sal_uInt16 >( "obj-type", "OBJ-TYPE" );
    dumpDec< sal_uInt16 >( "obj-id" );
    dumpHex< sal_uInt16 >( "flags", "OBJ-FLAGS" );
    dumpDffClientRect();
    sal_uInt16 nMacroSize = dumpDec< sal_uInt16 >( "macro-size" );
    dumpUnused( 2 );
    sal_uInt16 nTextLen = 0, nFormatSize = 0, nLinkSize = 0;
    switch( nObjType )
    {
        case BIFF_OBJTYPE_GROUP:
            dumpUnused( 4 );
            dumpDec< sal_uInt16 >( "next-ungrouped-id" );
            dumpUnused( 16 );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_LINE:
            dumpObjRecLineData();
            dumpHex< sal_uInt16 >( "line-end", "OBJ-LINEENDS" );
            dumpDec< sal_uInt8 >( "line-direction", "OBJ-LINEDIR" );
            dumpUnused( 1 );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_RECTANGLE:
        case BIFF_OBJTYPE_OVAL:
            dumpObjRecRectData();
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_ARC:
            dumpObjRecFillData();
            dumpObjRecLineData();
            dumpDec< sal_uInt8 >( "quadrant", "OBJ-ARC-QUADRANT" );
            dumpUnused( 1 );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_CHART:
            dumpObjRecRectData();
            dumpUnused( 18 );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_TEXT:
        case BIFF_OBJTYPE_BUTTON:
            dumpObjRecRectData();
            dumpObjRecTextDataBiff3( nTextLen, nFormatSize );
            dumpObjRecFmla( "macro", nMacroSize );
            dumpObjRecString( "text", nTextLen, false );
            dumpObjRecTextFmt( nFormatSize );
        break;
        case BIFF_OBJTYPE_PICTURE:
            dumpObjRecRectData();
            dumpDec< sal_Int16 >( "image-format", "IMGDATA-FORMAT" );
            dumpUnused( 4 );
            nLinkSize = dumpDec< sal_uInt16 >( "pic-link-size" );
            dumpUnused( 2 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-PICTURE-FLAGS" );
            dumpObjRecFmla( "macro", nMacroSize );
            dumpObjRecPictFmla( nLinkSize );
        break;
        case BIFF_OBJTYPE_POLYGON:
            dumpObjRecRectData();
            dumpHex< sal_uInt16 >( "flags", "OBJ-POLYGON-FLAGS" );
            dumpUnused( 10 );
            dumpDec< sal_uInt16 >( "point-count" );
            dumpUnused( 8 );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
    }
}

void WorkbookStreamObject::dumpObjRecBiff5()
{
    BiffInputStream& rStrm = getBiffStream();
    dumpDec< sal_uInt32 >( "obj-count" );
    sal_uInt16 nObjType = dumpDec< sal_uInt16 >( "obj-type", "OBJ-TYPE" );
    dumpDec< sal_uInt16 >( "obj-id" );
    dumpHex< sal_uInt16 >( "flags", "OBJ-FLAGS" );
    dumpDffClientRect();
    sal_uInt16 nMacroSize = dumpDec< sal_uInt16 >( "macro-size" );
    dumpUnused( 2 );
    sal_uInt16 nNameLen = dumpDec< sal_uInt16 >( "name-len" );
    dumpUnused( 2 );
    sal_uInt16 nTextLen = 0, nFormatSize = 0, nLinkSize = 0;
    switch( nObjType )
    {
        case BIFF_OBJTYPE_GROUP:
            dumpUnused( 4 );
            dumpDec< sal_uInt16 >( "next-ungrouped-id" );
            dumpUnused( 16 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_LINE:
            dumpObjRecLineData();
            dumpHex< sal_uInt16 >( "line-end", "OBJ-LINEENDS" );
            dumpDec< sal_uInt8 >( "line-direction", "OBJ-LINEDIR" );
            dumpUnused( 1 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_RECTANGLE:
        case BIFF_OBJTYPE_OVAL:
            dumpObjRecRectData();
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_ARC:
            dumpObjRecFillData();
            dumpObjRecLineData();
            dumpDec< sal_uInt8 >( "quadrant", "OBJ-ARC-QUADRANT" );
            dumpUnused( 1 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_CHART:
            dumpObjRecRectData();
            dumpHex< sal_uInt16 >( "chart-flags", "OBJ-CHART-FLAGS" );
            dumpUnused( 16 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_TEXT:
        case BIFF_OBJTYPE_BUTTON:
        case BIFF_OBJTYPE_LABEL:
        case BIFF_OBJTYPE_DIALOG:
            dumpObjRecRectData();
            dumpObjRecTextDataBiff5( nTextLen, nFormatSize, nLinkSize );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
            dumpObjRecString( "text", nTextLen, false );
            dumpObjRecFmla( "text-link", nLinkSize );
            dumpObjRecTextFmt( nFormatSize );
        break;
        case BIFF_OBJTYPE_PICTURE:
            dumpObjRecRectData();
            dumpDec< sal_Int16 >( "image-format", "IMGDATA-FORMAT" );
            dumpUnused( 4 );
            nLinkSize = dumpDec< sal_uInt16 >( "pic-link-size" );
            dumpUnused( 2 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-PICTURE-FLAGS" );
            dumpUnused( 4 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
            dumpObjRecPictFmla( nLinkSize );
            if( rStrm.getRemaining() >= 4 )
                dumpHex< sal_uInt32 >( "ole-storage-id" );
        break;
        case BIFF_OBJTYPE_POLYGON:
            dumpObjRecRectData();
            dumpHex< sal_uInt16 >( "flags", "OBJ-POLYGON-FLAGS" );
            dumpUnused( 10 );
            dumpDec< sal_uInt16 >( "point-count" );
            dumpUnused( 8 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", nMacroSize );
        break;
        case BIFF_OBJTYPE_CHECKBOX:
            dumpObjRecRectData();
            dumpUnused( 10 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-TEXT-FLAGS" );
            dumpUnused( 20 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecFmla( "cell-link", dumpDec< sal_uInt16 >( "cell-link-size" ) );
            dumpObjRecString( "text", dumpDec< sal_uInt16 >( "text-len" ), false );
            dumpObjRecCblsData();
        break;
        case BIFF_OBJTYPE_OPTIONBUTTON:
            dumpObjRecRectData();
            dumpUnused( 10 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-TEXT-FLAGS" );
            dumpUnused( 32 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecFmla( "cell-link", dumpDec< sal_uInt16 >( "cell-link-size" ) );
            dumpObjRecString( "text", dumpDec< sal_uInt16 >( "text-len" ), false );
            dumpObjRecCblsData();
            dumpObjRecRboData();
        break;
        case BIFF_OBJTYPE_EDIT:
            dumpObjRecRectData();
            dumpUnused( 10 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-TEXT-FLAGS" );
            dumpUnused( 14 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecString( "text", dumpDec< sal_uInt16 >( "text-len" ), false );
            dumpObjRecEdoData();
        break;
        case BIFF_OBJTYPE_SPIN:
        case BIFF_OBJTYPE_SCROLLBAR:
            dumpObjRecRectData();
            dumpObjRecSbsData();
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecFmla( "cell-link", dumpDec< sal_uInt16 >( "cell-link-size" ) );
        break;
        case BIFF_OBJTYPE_LISTBOX:
            dumpObjRecRectData();
            dumpObjRecSbsData();
            dumpUnused( 18 );
            dumpFontIdx( "font-idx" );
            dumpUnused( 4 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecFmla( "cell-link", dumpDec< sal_uInt16 >( "cell-link-size" ) );
            dumpObjRecLbsData();
        break;
        case BIFF_OBJTYPE_GROUPBOX:
            dumpObjRecRectData();
            dumpUnused( 10 );
            dumpHex< sal_uInt16 >( "flags", "OBJ-TEXT-FLAGS" );
            dumpUnused( 26 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecString( "text", dumpDec< sal_uInt16 >( "text-len" ), false );
            dumpObjRecGboData();
        break;
        case BIFF_OBJTYPE_DROPDOWN:
            dumpObjRecRectData();
            dumpObjRecSbsData();
            dumpUnused( 18 );
            dumpFontIdx( "font-idx" );
            dumpUnused( 14 );
            dumpDec< sal_uInt16 >( "bounding-left" );
            dumpDec< sal_uInt16 >( "bounding-top" );
            dumpDec< sal_uInt16 >( "bounding-right" );
            dumpDec< sal_uInt16 >( "bounding-bottom" );
            dumpUnused( 4 );
            dumpObjRecString( "obj-name", nNameLen, true );
            dumpObjRecFmla( "macro", dumpDec< sal_uInt16 >( "macro-size" ) );
            dumpObjRecFmla( "cell-link", dumpDec< sal_uInt16 >( "cell-link-size" ) );
            dumpObjRecLbsData();
            dumpDec< sal_uInt16 >( "type", "OBJ-DROPDOWN-TYPE" );
            dumpDec< sal_uInt16 >( "line-count" );
            dumpDec< sal_uInt16 >( "min-list-width" );
            dumpObjRecString( "text", dumpDec< sal_uInt16 >( "text-len" ), false );
        break;
    }
}

void WorkbookStreamObject::dumpObjRecBiff8()
{
    Output& rOut = out();
    BiffInputStream& rStrm = getBiffStream();
    NameListRef xRecNames = cfg().getNameList( "OBJ-RECNAMES" );
    sal_uInt16 nObjType = 0xFFFF;
    bool bControl = false;
    bool bCtlsStrm = false;
    bool bLoop = true;
    while( bLoop && (rStrm.getRemaining() >= 4) )
    {
        rOut.emptyLine();
        sal_uInt16 nSubRecId, nSubRecSize;
        {
            MultiItemsGuard aMultiGuard( rOut );
            writeEmptyItem( "OBJREC" );
            writeHexItem( "pos", static_cast< sal_uInt32 >( rStrm.tell() ) );
            rStrm >> nSubRecId >> nSubRecSize;
            writeHexItem( "size", nSubRecSize );
            writeHexItem( "id", nSubRecId, xRecNames );
        }

        sal_Int64 nSubRecStart = rStrm.tell();
        // sometimes the last subrecord has an invalid length
        sal_Int64 nRealRecSize = ::std::min< sal_Int64 >( nSubRecSize, rStrm.getRemaining() );
        sal_Int64 nSubRecEnd = nSubRecStart + nRealRecSize;

        IndentGuard aIndGuard( rOut );
        switch( nSubRecId )
        {
            case BIFF_ID_OBJMACRO:
                dumpObjRecFmlaRaw();
            break;
            case BIFF_ID_OBJCF:
                dumpDec< sal_Int16 >( "clipboard-format", "IMGDATA-FORMAT" );
            break;
            case BIFF_ID_OBJFLAGS:
            {
                sal_uInt16 nFlags = dumpHex< sal_uInt16 >( "flags", "OBJFLAGS-FLAGS" );
                bControl = getFlag( nFlags, BIFF_OBJFLAGS_CONTROL );
                bCtlsStrm = getFlag( nFlags, BIFF_OBJFLAGS_CTLSSTREAM );
            }
            break;
            case BIFF_ID_OBJPICTFMLA:
            {
                dumpObjRecPictFmla( dumpDec< sal_uInt16 >( "pic-link-size" ) );
                if( rStrm.tell() + 4 <= nSubRecEnd )
                {
                    if( bControl && bCtlsStrm )
                        dumpOcxControl();
                    else
                        dumpHex< sal_uInt32 >( "ole-storage-id" );
                }
                if( bControl && (rStrm.tell() + 8 <= nSubRecEnd) )
                {
                    sal_uInt32 nKeySize = dumpDec< sal_uInt32 >( "licence-key-size" );
                    if( nKeySize > 0 )
                    {
                        IndentGuard aIndGuard2( rOut );
                        sal_Int64 nKeyEnd = rStrm.tell() + nKeySize;
                        dumpArray( "licence-key", static_cast< sal_Int32 >( nKeySize ) );
                        rStrm.seek( nKeyEnd );
                    }
                    dumpObjRecFmla( "cell-link", dumpDec< sal_uInt16 >( "cell-link-size" ) );
                    dumpObjRecFmla( "source-range", dumpDec< sal_uInt16 >( "source-range-size" ) );
                }
            }
            break;
            case BIFF_ID_OBJCBLS:
                dumpDec< sal_uInt16 >( "state", "OBJ-CHECKBOX-STATE" );
                dumpUnused( 4 );
                dumpUnicode( "accelerator" );
                dumpUnicode( "fareast-accelerator" );
                dumpHex< sal_uInt16 >( "checkbox-flags", "OBJ-CHECKBOX-FLAGS" );
            break;
            case BIFF_ID_OBJRBO:
                dumpUnused( 4 );
                dumpBool< sal_uInt16 >( "first-in-group" );
            break;
            case BIFF_ID_OBJSBS:
                dumpObjRecSbsData();
            break;
            case BIFF_ID_OBJGBODATA:
                dumpObjRecGboData();
            break;
            case BIFF_ID_OBJEDODATA:
                dumpObjRecEdoData();
            break;
            case BIFF_ID_OBJRBODATA:
                dumpObjRecRboData();
            break;
            case BIFF_ID_OBJCBLSDATA:
                dumpObjRecCblsData();
            break;
            case BIFF_ID_OBJLBSDATA:
                dumpObjRecLbsData();
                if( nObjType == BIFF_OBJTYPE_DROPDOWN )
                {
                    dumpHex< sal_uInt16 >( "dropdown-flags", "OBJ-DROPDOWN-FLAGS" );
                    dumpDec< sal_uInt16 >( "line-count" );
                    dumpDec< sal_uInt16 >( "min-list-width" );
                    dumpObjRecString( "text", dumpDec< sal_uInt16 >( "text-len" ), false );
                }
            break;
            case BIFF_ID_OBJCBLSFMLA:
            case BIFF_ID_OBJSBSFMLA:
                dumpObjRecFmlaRaw();
            break;
            case BIFF_ID_OBJCMO:
                nObjType = dumpDec< sal_uInt16 >( "type", "OBJ-TYPE" );
                dumpDec< sal_uInt16 >( "id" );
                dumpHex< sal_uInt16 >( "flags", "OBJCMO-FLAGS" );
                dumpUnused( 12 );
            break;
        }
        // remaining undumped data
        if( !rStrm.isEof() && (rStrm.tell() == nSubRecStart) )
            dumpRawBinary( nRealRecSize, false );
        else
            dumpRemainingTo( nSubRecEnd );
    }
}

void WorkbookStreamObject::dumpObjRecLineData()
{
    dumpColorIdx( "line-color-idx", false );
    dumpDec< sal_uInt8 >( "line-type", "OBJ-LINETYPE" );
    dumpDec< sal_uInt8 >( "line-weight", "OBJ-LINEWEIGHT" );
    dumpHex< sal_uInt8 >( "line-flags", "OBJ-AUTO-FLAGS" );
}

void WorkbookStreamObject::dumpObjRecFillData()
{
    dumpColorIdx( "back-color-idx", false );
    dumpColorIdx( "patt-color-idx", false );
    dumpPatternIdx( EMPTY_STRING, false );
    dumpHex< sal_uInt8 >( "area-flags", "OBJ-AUTO-FLAGS" );
}

void WorkbookStreamObject::dumpObjRecRectData()
{
    dumpObjRecFillData();
    dumpObjRecLineData();
    dumpHex< sal_uInt16 >( "frame-style", "OBJ-FRAMESTYLE-FLAGS" );
}

void WorkbookStreamObject::dumpObjRecTextDataBiff3( sal_uInt16& ornTextLen, sal_uInt16& ornFormatSize )
{
    ornTextLen = dumpDec< sal_uInt16 >( "text-len" );
    dumpUnused( 2 );
    ornFormatSize = dumpDec< sal_uInt16 >( "format-run-size" );
    dumpFontIdx( "default-font-idx" );
    dumpUnused( 2 );
    dumpHex< sal_uInt16 >( "flags", "OBJ-TEXT-FLAGS" );
    dumpDec< sal_uInt16 >( "orientation", "TEXTORIENTATION" );
    dumpUnused( 8 );
}

void WorkbookStreamObject::dumpObjRecTextDataBiff5( sal_uInt16& ornTextLen, sal_uInt16& ornFormatSize, sal_uInt16& ornLinkSize )
{
    ornTextLen = dumpDec< sal_uInt16 >( "text-len" );
    dumpUnused( 2 );
    ornFormatSize = dumpDec< sal_uInt16 >( "format-run-size" );
    dumpFontIdx( "default-font-idx" );
    dumpUnused( 2 );
    dumpHex< sal_uInt16 >( "flags", "OBJ-TEXT-FLAGS" );
    dumpDec< sal_uInt16 >( "orientation", "TEXTORIENTATION" );
    dumpUnused( 2 );
    ornLinkSize = dumpDec< sal_uInt16 >( "link-size" );
    dumpUnused( 2 );
    dumpHex< sal_uInt16 >( "button-flags", "OBJ-BUTTON-FLAGS" );
    dumpUnicode( "accelerator" );
    dumpUnicode( "fareast-accelerator" );
}

void WorkbookStreamObject::dumpObjRecSbsData()
{
    dumpUnused( 4 );
    dumpDec< sal_uInt16 >( "value" );
    dumpDec< sal_uInt16 >( "min" );
    dumpDec< sal_uInt16 >( "max" );
    dumpDec< sal_uInt16 >( "step" );
    dumpDec< sal_uInt16 >( "page-step" );
    dumpBool< sal_uInt16 >( "horizontal" );
    dumpDec< sal_uInt16 >( "thumb-width" );
    dumpHex< sal_uInt16 >( "scrollbar-flags", "OBJ-SCROLLBAR-FLAGS" );
}

void WorkbookStreamObject::dumpObjRecGboData()
{
    dumpUnicode( "accelerator" );
    dumpUnicode( "fareast-accelerator" );
    dumpHex< sal_uInt16 >( "groupbox-flags", "OBJ-GROUPBOX-FLAGS" );
}

void WorkbookStreamObject::dumpObjRecEdoData()
{
    dumpDec< sal_uInt16 >( "type", "OBJ-EDIT-TYPE" );
    dumpBool< sal_uInt16 >( "multiline" );
    dumpBool< sal_uInt16 >( "scrollbar" );
    dumpDec< sal_uInt16 >( "listbox-obj-id" );
}

void WorkbookStreamObject::dumpObjRecRboData()
{
    dumpDec< sal_uInt16 >( "next-in-group" );
    dumpBool< sal_uInt16 >( "first-in-group" );
}

void WorkbookStreamObject::dumpObjRecCblsData()
{
    dumpDec< sal_uInt16 >( "state", "OBJ-CHECKBOX-STATE" );
    dumpUnicode( "accelerator" );
    dumpUnicode( "fareast-accelerator" );
    dumpHex< sal_uInt16 >( "checkbox-flags", "OBJ-CHECKBOX-FLAGS" );
}

void WorkbookStreamObject::dumpObjRecLbsData()
{
    dumpObjRecFmla( "source-range", dumpDec< sal_uInt16 >( "source-range-size" ) );
    dumpDec< sal_uInt16 >( "entry-count" );
    dumpDec< sal_uInt16 >( "selected-entry" );
    dumpHex< sal_uInt16 >( "listbox-flags", "OBJ-LISTBOX-FLAGS" );
    dumpDec< sal_uInt16 >( "edit-obj-id" );
}

void WorkbookStreamObject::dumpObjRecPadding()
{
    if( getBiffStream().tell() & 1 )
    {
        IndentGuard aIndGuard( out() );
        dumpHex< sal_uInt8 >( "padding" );
    }
}

void WorkbookStreamObject::dumpObjRecString( const String& rName, sal_uInt16 nTextLen, bool bRepeatLen )
{
    if( nTextLen > 0 )
    {
        if( bRepeatLen )
            dumpByteString( rName, BIFF_STR_8BITLENGTH );
        else
            writeStringItem( rName, getBiffStream().readCharArray( nTextLen, getBiffData().getTextEncoding() ) );
        dumpObjRecPadding();
    }
}

void WorkbookStreamObject::dumpObjRecTextFmt( sal_uInt16 nFormatSize )
{
    FontPortionModelList aPortions;
    aPortions.importPortions( getBiffStream(), nFormatSize / 8, BIFF_FONTPORTION_OBJ );
    writeFontPortions( aPortions );
}

void WorkbookStreamObject::dumpObjRecFmlaRaw()
{
    sal_uInt16 nFmlaSize = dumpDec< sal_uInt16 >( "fmla-size" );
    dumpUnused( 4 );
    getFormulaDumper().dumpNameFormula( "fmla", nFmlaSize );
    dumpObjRecPadding();
}

void WorkbookStreamObject::dumpObjRecFmla( const String& rName, sal_uInt16 nFmlaSize )
{
    BiffInputStream& rStrm = getBiffStream();
    if( nFmlaSize > 0 )
    {
        writeEmptyItem( rName );
        IndentGuard aIndGuard( out() );
        sal_Int64 nStrmEnd = rStrm.tell() + nFmlaSize;
        dumpObjRecFmlaRaw();
        if( rStrm.isEof() || (rStrm.tell() != nStrmEnd) )
            writeEmptyItem( OOX_DUMP_ERRASCII( "fmla-size" ) );
        dumpRemainingTo( nStrmEnd );
    }
}

void WorkbookStreamObject::dumpObjRecPictFmla( sal_uInt16 nFmlaSize )
{
    BiffInputStream& rStrm = getBiffStream();
    if( nFmlaSize > 0 )
    {
        writeEmptyItem( "pic-link" );
        IndentGuard aIndGuard( out() );
        sal_Int64 nStrmEnd = rStrm.tell() + nFmlaSize;
        if( (getBiff() == BIFF3) && (nStrmEnd & 1) ) ++nStrmEnd; // BIFF3 size without padding
        dumpObjRecFmlaRaw();
        if( rStrm.tell() + 2 <= nStrmEnd )
        {
            dumpString( "class-name", BIFF_STR_DEFAULT, BIFF_STR_SMARTFLAGS );
            dumpObjRecPadding();
        }
        if( rStrm.isEof() || (rStrm.tell() != nStrmEnd) )
            writeEmptyItem( OOX_DUMP_ERRASCII( "pic-link-size" ) );
        dumpRemainingTo( nStrmEnd );
    }
}

// ============================================================================

PivotCacheStreamObject::PivotCacheStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, BiffType eBiff, const ::rtl::OUString& rSysFileName )
{
    RecordStreamObject::construct( rParent, rxStrm, eBiff, rSysFileName );
}

void PivotCacheStreamObject::implDumpRecordBody()
{
    BiffInputStream& rStrm = getBiffStream();
    sal_uInt16 nRecId = rStrm.getRecId();

    switch( nRecId )
    {
        case BIFF_ID_PCDEFINITION:
            dumpDec< sal_Int32 >( "source-records" );
            dumpHex< sal_uInt16 >( "cache-id" );
            dumpHex< sal_uInt16 >( "flags", "PCDEFINITION-FLAGS" );
            dumpUnused( 2 );
            dumpDec< sal_uInt16 >( "sourcedata-field-count" );
            dumpDec< sal_uInt16 >( "cache-field-count" );
            dumpDec< sal_uInt16 >( "report-record-count" );
            dumpDec< sal_uInt16 >( "database-type", "PCDSOURCE-TYPE" );
            dumpString( "user-name" );
        break;

        case BIFF_ID_PCDEFINITION2:
            dumpDec< double >( "refreshed-date" );
            dumpDec< sal_Int32 >( "formula-count" );
        break;

        case BIFF_ID_PCDFDISCRETEPR:
            out().resetItemIndex();
            while( !rStrm.isEof() && (rStrm.getRemaining() >= 2) )
                dumpDec< sal_uInt16 >( "#item-index" );
        break;

        case BIFF_ID_PCDFIELD:
            dumpHex< sal_uInt16 >( "flags", "PCDFIELD-FLAGS" );
            dumpDec< sal_uInt16 >( "group-parent-field" );
            dumpDec< sal_uInt16 >( "group-base-field" );
            dumpDec< sal_uInt16 >( "unique-items" );
            dumpDec< sal_uInt16 >( "group-items" );
            dumpDec< sal_uInt16 >( "base-items" );
            dumpDec< sal_uInt16 >( "shared-items" );
            if( rStrm.getRemaining() >= 3 )
                dumpString( "item-name" );
        break;

        case BIFF_ID_PCITEM_DATE:
        {
            DateTime aDateTime;
            aDateTime.Year = in().readuInt16();
            aDateTime.Month = in().readuInt16();
            aDateTime.Day = in().readuInt8();
            aDateTime.Hours = in().readuInt8();
            aDateTime.Minutes = in().readuInt8();
            aDateTime.Seconds = in().readuInt8();
            writeDateTimeItem( "value", aDateTime );
        }
        break;

        case BIFF_ID_PCITEM_STRING:
            dumpString( "value" );
        break;
    }
}

// ============================================================================
// ============================================================================

RootStorageObject::RootStorageObject( const DumperBase& rParent )
{
    OleStorageObject::construct( rParent );
    addPreferredStream( "Book" );
    addPreferredStream( "Workbook" );
}

void RootStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    if( (rStrgPath.getLength() == 0) && (rStrmName.equalsAscii( "Book" ) || rStrmName.equalsAscii( "Workbook" )) )
        WorkbookStreamObject( *this, rxStrm, rSysFileName ).dump();
    else if( rStrgPath.equalsAscii( "_SX_DB" ) )
        PivotCacheStreamObject( *this, rxStrm, BIFF5, rSysFileName ).dump();
    else if( rStrgPath.equalsAscii( "_SX_DB_CUR" ) )
        PivotCacheStreamObject( *this, rxStrm, BIFF8, rSysFileName ).dump();
    else
        OleStorageObject::implDumpStream( rxStrm, rStrgPath, rStrmName, rSysFileName );
}

void RootStorageObject::implDumpStorage( const StorageRef& rxStrg, const OUString& rStrgPath, const OUString& rSysPath )
{
    if( rStrgPath.equalsAscii( "_VBA_PROJECT_CUR" ) )
        VbaProjectStorageObject( *this, rxStrg, rSysPath ).dump();
    else
        OleStorageObject::implDumpStorage( rxStrg, rStrgPath, rSysPath );
}

// ============================================================================
// ============================================================================

#define DUMP_BIFF_CONFIG_ENVVAR "OOO_BIFFDUMPER"

Dumper::Dumper( const FilterBase& rFilter )
{
    ConfigRef xCfg( new Config( DUMP_BIFF_CONFIG_ENVVAR, rFilter ) );
    DumperBase::construct( xCfg );
}

Dumper::Dumper( const Reference< XMultiServiceFactory >& rxFactory, const Reference< XInputStream >& rxInStrm, const OUString& rSysFileName )
{
    if( rxFactory.is() && rxInStrm.is() )
    {
        StorageRef xStrg( new OleStorage( rxFactory, rxInStrm, true ) );
        ConfigRef xCfg( new Config( DUMP_BIFF_CONFIG_ENVVAR, rxFactory, xStrg, rSysFileName ) );
        DumperBase::construct( xCfg );
    }
}

void Dumper::implDump()
{
    RootStorageObject aRootStrg( *this );
    if( aRootStrg.isValid() )
    {
        aRootStrg.dump();
    }
    else if( StorageBase* pRootStrg = cfg().getRootStorage().get() )
    {
        // try to dump plain BIFF stream
        BinaryInputStreamRef xStrm( new BinaryXInputStream( pRootStrg->openInputStream( OUString() ), false ) );
        WorkbookStreamObject( *this, xStrm, cfg().getSysFileName() ).dump();
    }
}

// ============================================================================

} // namespace biff
} // namespace dump
} // namespace oox

#endif

