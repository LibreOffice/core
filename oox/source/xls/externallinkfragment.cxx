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

#include "oox/xls/externallinkfragment.hxx"

#include <com/sun/star/sheet/XExternalSheetCache.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/defnamesbuffer.hxx"
#include "oox/xls/sheetdatacontext.hxx"
#include "oox/xls/unitconverter.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================
// ============================================================================

ExternalSheetDataContext::ExternalSheetDataContext(
        WorkbookFragmentBase& rFragment, const Reference< XExternalSheetCache >& rxSheetCache ) :
    WorkbookContextBase( rFragment ),
    mxSheetCache( rxSheetCache )
{
    OSL_ENSURE( mxSheetCache.is(), "ExternalSheetDataContext::ExternalSheetDataContext - missing sheet cache" );
}

ContextHandlerRef ExternalSheetDataContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetData ):
            if( nElement == XLS_TOKEN( row ) ) return this;
        break;
        case XLS_TOKEN( row ):
            if( nElement == XLS_TOKEN( cell ) ) { importCell( rAttribs ); return this; }
        break;
        case XLS_TOKEN( cell ):
            if( nElement == XLS_TOKEN( v ) ) return this;   // collect characters in onCharacters()
        break;
    }
    return 0;
}

void ExternalSheetDataContext::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( v ) ) )
    {
        switch( mnCurrType )
        {
            case XML_b:
            case XML_n:
                setCellValue( Any( rChars.toDouble() ) );
            break;
            case XML_e:
                setCellValue( Any( BiffHelper::calcDoubleFromError( getUnitConverter().calcBiffErrorCode( rChars ) ) ) );
            break;
            case XML_str:
                setCellValue( Any( rChars ) );
            break;
        }
        mnCurrType = XML_TOKEN_INVALID;
    }
}

ContextHandlerRef ExternalSheetDataContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_EXTSHEETDATA:
            if( nRecId == BIFF12_ID_EXTROW ) { maCurrPos.Row = rStrm.readInt32(); return this; }
        break;
        case BIFF12_ID_EXTROW:
            switch( nRecId )
            {
                case BIFF12_ID_EXTCELL_BLANK:   importExtCellBlank( rStrm );    break;
                case BIFF12_ID_EXTCELL_BOOL:    importExtCellBool( rStrm );     break;
                case BIFF12_ID_EXTCELL_DOUBLE:  importExtCellDouble( rStrm );   break;
                case BIFF12_ID_EXTCELL_ERROR:   importExtCellError( rStrm );    break;
                case BIFF12_ID_EXTCELL_STRING:  importExtCellString( rStrm );   break;
            }
        break;
    }
    return 0;
}

// private --------------------------------------------------------------------

void ExternalSheetDataContext::importCell( const AttributeList& rAttribs )
{
    if( getAddressConverter().convertToCellAddress( maCurrPos, rAttribs.getString( XML_r, OUString() ), 0, false ) )
        mnCurrType = rAttribs.getToken( XML_t, XML_n );
    else
        mnCurrType = XML_TOKEN_INVALID;
}

void ExternalSheetDataContext::importExtCellBlank( SequenceInputStream& rStrm )
{
    maCurrPos.Column = rStrm.readInt32();
    setCellValue( Any( OUString() ) );
}

void ExternalSheetDataContext::importExtCellBool( SequenceInputStream& rStrm )
{
    maCurrPos.Column = rStrm.readInt32();
    double fValue = (rStrm.readuInt8() == 0) ? 0.0 : 1.0;
    setCellValue( Any( fValue ) );
}

void ExternalSheetDataContext::importExtCellDouble( SequenceInputStream& rStrm )
{
    maCurrPos.Column = rStrm.readInt32();
    setCellValue( Any( rStrm.readDouble() ) );
}

void ExternalSheetDataContext::importExtCellError( SequenceInputStream& rStrm )
{
    maCurrPos.Column = rStrm.readInt32();
    setCellValue( Any( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) ) );
}

void ExternalSheetDataContext::importExtCellString( SequenceInputStream& rStrm )
{
    maCurrPos.Column = rStrm.readInt32();
    setCellValue( Any( BiffHelper::readString( rStrm ) ) );
}

void ExternalSheetDataContext::setCellValue( const Any& rValue )
{
    if( mxSheetCache.is() && getAddressConverter().checkCellAddress( maCurrPos, false ) ) try
    {
        mxSheetCache->setCellValue( maCurrPos.Column, maCurrPos.Row, rValue );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

ExternalLinkFragment::ExternalLinkFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ExternalLink& rExtLink ) :
    WorkbookFragmentBase( rHelper, rFragmentPath ),
    mrExtLink( rExtLink ),
    mnResultType( XML_TOKEN_INVALID )
{
}

ContextHandlerRef ExternalLinkFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( externalLink ) ) return this;
        break;

        case XLS_TOKEN( externalLink ):
            switch( nElement )
            {
                case XLS_TOKEN( externalBook ): mrExtLink.importExternalBook( getRelations(), rAttribs );   return this;
                case XLS_TOKEN( ddeLink ):      mrExtLink.importDdeLink( rAttribs );                        return this;
                case XLS_TOKEN( oleLink ):      mrExtLink.importOleLink( getRelations(), rAttribs );        return this;
            }
        break;

        case XLS_TOKEN( externalBook ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetNames ):
                case XLS_TOKEN( definedNames ):
                case XLS_TOKEN( sheetDataSet ): return this;
            }
        break;

        case XLS_TOKEN( sheetNames ):
            if( nElement == XLS_TOKEN( sheetName ) ) mrExtLink.importSheetName( rAttribs );
        break;
        case XLS_TOKEN( definedNames ):
            if( nElement == XLS_TOKEN( definedName ) ) mrExtLink.importDefinedName( rAttribs );
        break;
        case XLS_TOKEN( sheetDataSet ):
            if( (nElement == XLS_TOKEN( sheetData )) && (mrExtLink.getLinkType() == LINKTYPE_EXTERNAL) )
                return createSheetDataContext( rAttribs.getInteger( XML_sheetId, -1 ) );
        break;

        case XLS_TOKEN( ddeLink ):
            if( nElement == XLS_TOKEN( ddeItems ) ) return this;
        break;
        case XLS_TOKEN( ddeItems ):
            if( nElement == XLS_TOKEN( ddeItem ) )
            {
                mxExtName = mrExtLink.importDdeItem( rAttribs );
                return this;
            }
        break;
        case XLS_TOKEN( ddeItem ):
            if( nElement == XLS_TOKEN( values ) )
            {
                if( mxExtName.get() ) mxExtName->importValues( rAttribs );
                return this;
            }
        break;
        case XLS_TOKEN( values ):
            if( nElement == XLS_TOKEN( value ) )
            {
                mnResultType = rAttribs.getToken( XML_t, XML_n );
                return this;
            }
        break;
        case XLS_TOKEN( value ):
            if( nElement == XLS_TOKEN( val ) ) return this; // collect value in onCharacters()
        break;

        case XLS_TOKEN( oleLink ):
            if( nElement == XLS_TOKEN( oleItems ) ) return this;
        break;
        case XLS_TOKEN( oleItems ):
            if( nElement == XLS_TOKEN( oleItem ) ) mxExtName = mrExtLink.importOleItem( rAttribs );
        break;
    }
    return 0;
}

void ExternalLinkFragment::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( val ) ) )
        maResultValue = rChars;
}

void ExternalLinkFragment::onEndElement()
{
    if( isCurrentElement( XLS_TOKEN( value ) ) && mxExtName.get() ) switch( mnResultType )
    {
        case XML_b:
            mxExtName->appendResultValue( maResultValue.toDouble() );
        break;
        case XML_e:
            mxExtName->appendResultValue( BiffHelper::calcDoubleFromError( getUnitConverter().calcBiffErrorCode( maResultValue ) ) );
        break;
        case XML_n:
            mxExtName->appendResultValue( maResultValue.toDouble() );
        break;
        case XML_str:
            mxExtName->appendResultValue( maResultValue );
        break;
        default:
            mxExtName->appendResultValue( BiffHelper::calcDoubleFromError( BIFF_ERR_NA ) );
    }
}

ContextHandlerRef ExternalLinkFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_EXTERNALBOOK )
            {
                mrExtLink.importExternalBook( getRelations(), rStrm );
                return this;
            }
        break;

        case BIFF12_ID_EXTERNALBOOK:
            switch( nRecId )
            {
                case BIFF12_ID_EXTSHEETDATA:
                    if( mrExtLink.getLinkType() == LINKTYPE_EXTERNAL )
                        return createSheetDataContext( rStrm.readInt32() );
                break;

                case BIFF12_ID_EXTSHEETNAMES:       mrExtLink.importExtSheetNames( rStrm );                             break;
                case BIFF12_ID_EXTERNALNAME:        mxExtName = mrExtLink.importExternalName( rStrm );                  return this;
            }
        break;

        case BIFF12_ID_EXTERNALNAME:
            switch( nRecId )
            {
                case BIFF12_ID_EXTERNALNAMEFLAGS:   if( mxExtName.get() ) mxExtName->importExternalNameFlags( rStrm );  break;
                case BIFF12_ID_DDEITEMVALUES:       if( mxExtName.get() ) mxExtName->importDdeItemValues( rStrm );      return this;
            }
        break;

        case BIFF12_ID_DDEITEMVALUES:
            switch( nRecId )
            {
                case BIFF12_ID_DDEITEM_BOOL:        if( mxExtName.get() ) mxExtName->importDdeItemBool( rStrm );        break;
                case BIFF12_ID_DDEITEM_DOUBLE:      if( mxExtName.get() ) mxExtName->importDdeItemDouble( rStrm );      break;
                case BIFF12_ID_DDEITEM_ERROR:       if( mxExtName.get() ) mxExtName->importDdeItemError( rStrm );       break;
                case BIFF12_ID_DDEITEM_STRING:      if( mxExtName.get() ) mxExtName->importDdeItemString( rStrm );      break;
            }
        break;
    }
    return 0;
}

ContextHandlerRef ExternalLinkFragment::createSheetDataContext( sal_Int32 nSheetId )
{
    return new ExternalSheetDataContext( *this, mrExtLink.getSheetCache( nSheetId ) );
}

const RecordInfo* ExternalLinkFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_DDEITEMVALUES,  BIFF12_ID_DDEITEMVALUES + 1     },
        { BIFF12_ID_EXTERNALBOOK,   BIFF12_ID_EXTERNALBOOK + 228    },
        { BIFF12_ID_EXTERNALNAME,   BIFF12_ID_EXTERNALNAME + 10     },
        { BIFF12_ID_EXTROW,         -1                              },
        { BIFF12_ID_EXTSHEETDATA,   BIFF12_ID_EXTSHEETDATA + 1      },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

// ============================================================================
// ============================================================================

BiffExternalSheetDataContext::BiffExternalSheetDataContext( const WorkbookHelper& rHelper, bool bImportDefNames ) :
    BiffWorkbookContextBase( rHelper ),
    mbImportDefNames( bImportDefNames )
{
}

BiffExternalSheetDataContext::~BiffExternalSheetDataContext()
{
}

void BiffExternalSheetDataContext::importRecord( BiffInputStream& rStrm )
{
    sal_uInt16 nRecId = rStrm.getRecId();
    switch( getBiff() )
    {
        case BIFF2: switch( nRecId )
        {
            case BIFF2_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF2_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
        }
        break;
        case BIFF3: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF3_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF3_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF4: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF3_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF3_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF5: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF5_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF5_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF8: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF_ID_EXTERNALBOOK:  importExternalBook( rStrm );    break;
            case BIFF5_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF5_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF_UNKNOWN: break;
    }
}

// private --------------------------------------------------------------------

void BiffExternalSheetDataContext::importExternSheet( BiffInputStream& rStrm )
{
    mxSheetCache.clear();
    if( getBiff() == BIFF8 )
        getExternalLinks().importExternSheet8( rStrm );
    else
        mxExtLink = getExternalLinks().importExternSheet( rStrm );
}

void BiffExternalSheetDataContext::importExternalBook( BiffInputStream& rStrm )
{
    mxSheetCache.clear();
    mxExtLink = getExternalLinks().importExternalBook( rStrm );
}

void BiffExternalSheetDataContext::importExternalName( BiffInputStream& rStrm )
{
    if( mxExtLink.get() )
        mxExtLink->importExternalName( rStrm );
}

void BiffExternalSheetDataContext::importXct( BiffInputStream& rStrm )
{
    mxSheetCache.clear();
    if( mxExtLink.get() && (mxExtLink->getLinkType() == LINKTYPE_EXTERNAL) )
    {
        switch( getBiff() )
        {
            case BIFF2:
            break;
            case BIFF3:
            case BIFF4:
            case BIFF5:
                mxSheetCache = mxExtLink->getSheetCache( 0 );
            break;
            case BIFF8:
                rStrm.skip( 2 );
                mxSheetCache = mxExtLink->getSheetCache( rStrm.readInt16() );
            break;
            case BIFF_UNKNOWN:
            break;
        }
    }
}

void BiffExternalSheetDataContext::importCrn( BiffInputStream& rStrm )
{
    if( !mxSheetCache.is() ) return;

    sal_uInt8 nCol2, nCol1;
    sal_uInt16 nRow;
    rStrm >> nCol2 >> nCol1 >> nRow;
    bool bLoop = true;
    for( BinAddress aBinAddr( nCol1, nRow ); bLoop && !rStrm.isEof() && (aBinAddr.mnCol <= nCol2); ++aBinAddr.mnCol )
    {
        switch( rStrm.readuInt8() )
        {
            case BIFF_DATATYPE_EMPTY:
                rStrm.skip( 8 );
                setCellValue( aBinAddr, Any( OUString() ) );
            break;
            case BIFF_DATATYPE_DOUBLE:
                setCellValue( aBinAddr, Any( rStrm.readDouble() ) );
            break;
            case BIFF_DATATYPE_STRING:
            {
                OUString aText = (getBiff() == BIFF8) ? rStrm.readUniString() : rStrm.readByteStringUC( false, getTextEncoding() );
                setCellValue( aBinAddr, Any( aText ) );
            }
            break;
            case BIFF_DATATYPE_BOOL:
            {
                double fValue = (rStrm.readuInt8() == 0) ? 0.0 : 1.0;
                setCellValue( aBinAddr, Any( fValue ) );
                rStrm.skip( 7 );
            }
            break;
            case BIFF_DATATYPE_ERROR:
                setCellValue( aBinAddr, Any( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) ) );
                rStrm.skip( 7 );
            break;
            default:
                OSL_ENSURE( false, "BiffExternalSheetDataContext::importCrn - unknown data type" );
                bLoop = false;
        }
    }
}

void BiffExternalSheetDataContext::importDefinedName( BiffInputStream& rStrm )
{
    if( mbImportDefNames )
        getDefinedNames().importDefinedName( rStrm );
}

void BiffExternalSheetDataContext::setCellValue( const BinAddress& rBinAddr, const Any& rValue )
{
    CellAddress aCellPos;
    if( mxSheetCache.is() && getAddressConverter().convertToCellAddress( aCellPos, rBinAddr, 0, false ) ) try
    {
        mxSheetCache->setCellValue( aCellPos.Column, aCellPos.Row, rValue );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

BiffExternalLinkFragment::BiffExternalLinkFragment( const BiffWorkbookFragmentBase& rParent ) :
    BiffWorkbookFragmentBase( rParent )
{
}

bool BiffExternalLinkFragment::importFragment()
{
    // process all record in this sheet fragment
    BiffExternalSheetDataContext aSheetContext( *this, false );
    BiffInputStream& rStrm = getInputStream();
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        if( BiffHelper::isBofRecord( rStrm ) )
            skipFragment();  // skip unknown embedded fragments
        else
            aSheetContext.importRecord( rStrm );
    }
    return !rStrm.isEof() && (rStrm.getRecId() == BIFF_ID_EOF);
}

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox
