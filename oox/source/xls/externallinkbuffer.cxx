/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: externallinkbuffer.cxx,v $
 * $Revision: 1.5.4.1 $
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

#include "oox/xls/externallinkbuffer.hxx"
#include <rtl/strbuf.hxx>
#include <com/sun/star/sheet/DDELinkInfo.hpp>
#include <com/sun/star/sheet/ExternalLinkType.hpp>
#include <com/sun/star/sheet/XDDELinks.hpp>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/sheet/XDDELinkResults.hpp>
#include <com/sun/star/sheet/XExternalDocLink.hpp>
#include <com/sun/star/sheet/XExternalDocLinks.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/worksheetbuffer.hxx"

using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::sheet::DDEItemInfo;
using ::com::sun::star::sheet::DDELinkInfo;
using ::com::sun::star::sheet::ExternalLinkInfo;
using ::com::sun::star::sheet::XDDELinks;
using ::com::sun::star::sheet::XDDELinkResults;
using ::com::sun::star::sheet::XExternalDocLinks;
using ::com::sun::star::sheet::XExternalSheetCache;
using ::oox::core::Relation;
using ::oox::core::Relations;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt16 OOBIN_EXTERNALBOOK_BOOK    = 0;
const sal_uInt16 OOBIN_EXTERNALBOOK_DDE     = 1;
const sal_uInt16 OOBIN_EXTERNALBOOK_OLE     = 2;

const sal_uInt16 OOBIN_EXTNAME_AUTOMATIC    = 0x0002;
const sal_uInt16 OOBIN_EXTNAME_PREFERPIC    = 0x0004;
const sal_uInt16 OOBIN_EXTNAME_STDDOCNAME   = 0x0008;
const sal_uInt16 OOBIN_EXTNAME_OLEOBJECT    = 0x0010;
const sal_uInt16 OOBIN_EXTNAME_ICONIFIED    = 0x0020;

const sal_uInt16 BIFF_EXTNAME_BUILTIN       = 0x0001;
const sal_uInt16 BIFF_EXTNAME_AUTOMATIC     = 0x0002;
const sal_uInt16 BIFF_EXTNAME_PREFERPIC     = 0x0004;
const sal_uInt16 BIFF_EXTNAME_STDDOCNAME    = 0x0008;
const sal_uInt16 BIFF_EXTNAME_OLEOBJECT     = 0x0010;
const sal_uInt16 BIFF_EXTNAME_ICONIFIED     = 0x8000;

} // namespace

// ============================================================================

ExternalNameModel::ExternalNameModel() :
    mbBuiltIn( false ),
    mbNotify( false ),
    mbPreferPic( false ),
    mbStdDocName( false ),
    mbOleObj( false ),
    mbIconified( false )
{
}

// ============================================================================

ExternalName::ExternalName( const ExternalLink& rParentLink, sal_Int32 nLocalSheet ) :
    DefinedNameBase( rParentLink, nLocalSheet ),
    mrParentLink( rParentLink ),
    mnStorageId( 0 ),
    mbDdeLinkCreated( false )
{
}

void ExternalName::importDefinedName( const AttributeList& rAttribs )
{
    maModel.maName = rAttribs.getXString( XML_name, OUString() );
    OSL_ENSURE( maModel.maName.getLength() > 0, "ExternalName::importDefinedName - empty name" );
    // zero-based index into sheet list of externalBook
    maModel.mnSheet = rAttribs.getInteger( XML_sheetId, -1 );
}

void ExternalName::importDdeItem( const AttributeList& rAttribs )
{
    maModel.maName = rAttribs.getXString( XML_name, OUString() );
    OSL_ENSURE( maModel.maName.getLength() > 0, "ExternalName::importDdeItem - empty name" );
    maExtNameModel.mbOleObj     = false;
    maExtNameModel.mbStdDocName = rAttribs.getBool( XML_ole, false );
    maExtNameModel.mbNotify     = rAttribs.getBool( XML_advise, false );
    maExtNameModel.mbPreferPic  = rAttribs.getBool( XML_preferPic, false );
}

void ExternalName::importValues( const AttributeList& rAttribs )
{
    setResultSize( rAttribs.getInteger( XML_cols, 1 ), rAttribs.getInteger( XML_rows, 1 ) );
}

void ExternalName::importOleItem( const AttributeList& rAttribs )
{
    maModel.maName = rAttribs.getXString( XML_name, OUString() );
    OSL_ENSURE( maModel.maName.getLength() > 0, "ExternalName::importOleItem - empty name" );
    maExtNameModel.mbOleObj    = true;
    maExtNameModel.mbNotify    = rAttribs.getBool( XML_advise, false );
    maExtNameModel.mbPreferPic = rAttribs.getBool( XML_preferPic, false );
    maExtNameModel.mbIconified = rAttribs.getBool( XML_icon, false );
}

void ExternalName::importExternalName( RecordInputStream& rStrm )
{
    rStrm >> maModel.maName;
    OSL_ENSURE( maModel.maName.getLength() > 0, "ExternalName::importExternalName - empty name" );
}

void ExternalName::importExternalNameFlags( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags;
    sal_Int32 nSheetId;
    rStrm >> nFlags >> nSheetId;
    // index into sheet list of EXTSHEETNAMES (one-based in OOBIN)
    maModel.mnSheet = nSheetId - 1;
    // no flag for built-in names, as in OOX...
    maExtNameModel.mbNotify     = getFlag( nFlags, OOBIN_EXTNAME_AUTOMATIC );
    maExtNameModel.mbPreferPic  = getFlag( nFlags, OOBIN_EXTNAME_PREFERPIC );
    maExtNameModel.mbStdDocName = getFlag( nFlags, OOBIN_EXTNAME_STDDOCNAME );
    maExtNameModel.mbOleObj     = getFlag( nFlags, OOBIN_EXTNAME_OLEOBJECT );
    maExtNameModel.mbIconified  = getFlag( nFlags, OOBIN_EXTNAME_ICONIFIED );
    OSL_ENSURE( (mrParentLink.getLinkType() == LINKTYPE_OLE) == maExtNameModel.mbOleObj,
        "ExternalName::importExternalNameFlags - wrong OLE flag in external name" );
}

void ExternalName::importDdeItemValues( RecordInputStream& rStrm )
{
    sal_Int32 nRows, nCols;
    rStrm >> nRows >> nCols;
    setResultSize( nCols, nRows );
}

void ExternalName::importDdeItemBool( RecordInputStream& rStrm )
{
    appendResultValue< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
}

void ExternalName::importDdeItemDouble( RecordInputStream& rStrm )
{
    appendResultValue( rStrm.readDouble() );
}

void ExternalName::importDdeItemError( RecordInputStream& rStrm )
{
    appendResultValue( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) );
}

void ExternalName::importDdeItemString( RecordInputStream& rStrm )
{
    appendResultValue( rStrm.readString() );
}

void ExternalName::importExternalName( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags = 0;
    if( getBiff() >= BIFF3 )
    {
        rStrm >> nFlags;
        maExtNameModel.mbBuiltIn    = getFlag( nFlags, BIFF_EXTNAME_BUILTIN );
        maExtNameModel.mbNotify     = getFlag( nFlags, BIFF_EXTNAME_AUTOMATIC );
        maExtNameModel.mbPreferPic  = getFlag( nFlags, BIFF_EXTNAME_PREFERPIC );

        // BIFF5-BIFF8: sheet index for sheet-local names, OLE settings
        if( getBiff() >= BIFF5 )
        {
            maExtNameModel.mbStdDocName = getFlag( nFlags, BIFF_EXTNAME_STDDOCNAME );
            maExtNameModel.mbOleObj     = getFlag( nFlags, BIFF_EXTNAME_OLEOBJECT );
            maExtNameModel.mbIconified  = getFlag( nFlags, BIFF_EXTNAME_ICONIFIED );

            if( maExtNameModel.mbOleObj )
            {
                rStrm >> mnStorageId;
            }
            else
            {
                /*  Import the reference ID for names that are sheet-local in
                    the external document. This index will be resolved later to
                    the index of the external sheet cache which is able to
                    provide the name of the sheet related to this defined name.
                    - BIFF5: one-based index to EXTERNSHEET record containing
                        the document and sheet name
                    - BIFF8: one-based index into EXTERNALBOOK sheet name list
                    The value zero means this external name is a global name.
                 */
                rStrm.skip( 2 );
                maModel.mnSheet = rStrm.readuInt16();
            }
        }
    }

    maModel.maName = (getBiff() == BIFF8) ?
        rStrm.readUniStringBody( rStrm.readuInt8() ) :
        rStrm.readByteStringUC( false, getTextEncoding() );
    OSL_ENSURE( maModel.maName.getLength() > 0, "ExternalName::importExternalName - empty name" );

    switch( mrParentLink.getLinkType() )
    {
        case LINKTYPE_INTERNAL:
        case LINKTYPE_EXTERNAL:
            // cell references that are stored in hidden external names (seen in BIFF3-BIFF4)
            if( (getBiff() <= BIFF4) && (maModel.maName.getLength() > 0) && (maModel.maName[ 0 ] == '\x01') && (rStrm.getRemaining() > 2) )
            {
                TokensFormulaContext aContext( true, true );
                importBiffFormula( aContext, rStrm );
                setReference( aContext.getTokens() );
            }
        break;

        case LINKTYPE_DDE:
        case LINKTYPE_OLE:
        case LINKTYPE_MAYBE_DDE_OLE:
            // DDE/OLE link results
            if( rStrm.getRemaining() > 3 )
            {
                bool bBiff8 = getBiff() == BIFF8;
                sal_Int32 nCols = rStrm.readuInt8();
                sal_Int32 nRows = rStrm.readuInt16();
                if( bBiff8 ) { ++nCols; ++nRows; } else if( nCols == 0 ) nCols = 256;
                setResultSize( nCols, nRows );

                bool bLoop = true;
                while( bLoop && !rStrm.isEof() && (maCurrIt != maResults.end()) )
                {
                    switch( rStrm.readuInt8() )
                    {
                        case BIFF_DATATYPE_EMPTY:
                            appendResultValue( OUString() );
                            rStrm.skip( 8 );
                        break;
                        case BIFF_DATATYPE_DOUBLE:
                            appendResultValue( rStrm.readDouble() );
                        break;
                        case BIFF_DATATYPE_STRING:
                            appendResultValue( bBiff8 ? rStrm.readUniString() : rStrm.readByteStringUC( false, getTextEncoding() ) );
                        break;
                        case BIFF_DATATYPE_BOOL:
                            appendResultValue< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
                            rStrm.skip( 7 );
                        break;
                        case BIFF_DATATYPE_ERROR:
                            appendResultValue( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) );
                            rStrm.skip( 7 );
                        break;
                        default:
                            bLoop = false;
                    }
                }
                OSL_ENSURE( bLoop && !rStrm.isEof() && (maCurrIt == maResults.end()),
                    "ExternalName::importExternalName - stream error in result set" );
            }
        break;

        default:;
    }
}

sal_Int32 ExternalName::getSheetCacheIndex() const
{
    OSL_ENSURE( mrParentLink.getLinkType() == LINKTYPE_DDE, "ExternalName::getSheetCacheIndex - unexpected link type" );
    sal_Int32 nCacheIdx = -1;
    switch( getFilterType() )
    {
        case FILTER_OOX:
            // OOXML/OOBIN: zero-based index into sheet list, -1 means global name
            if( maModel.mnSheet >= 0 )
                nCacheIdx = mrParentLink.getSheetIndex( maModel.mnSheet );
        break;
        case FILTER_BIFF:
            switch( getBiff() )
            {
                case BIFF2:
                case BIFF3:
                case BIFF4:
                break;
                case BIFF5:
                    if( maModel.mnSheet > 0 )
                        if( const ExternalLink* pExtLink = getExternalLinks().getExternalLink( maModel.mnSheet ).get() )
                            if( pExtLink->getLinkType() == LINKTYPE_EXTERNAL )
                                nCacheIdx = pExtLink->getSheetIndex();
                break;
                case BIFF8:
                    if( maModel.mnSheet > 0 )
                        nCacheIdx = mrParentLink.getSheetIndex( maModel.mnSheet - 1 );
                break;
                case BIFF_UNKNOWN:
                break;
            }
        break;
        case FILTER_UNKNOWN:
        break;
    }
    return nCacheIdx;
}

bool ExternalName::getDdeItemInfo( DDEItemInfo& orItemInfo ) const
{
    if( (mrParentLink.getLinkType() == LINKTYPE_DDE) && (maModel.maName.getLength() > 0) )
    {
        orItemInfo.Item = maModel.maName;
        orItemInfo.Results = ContainerHelper::matrixToSequenceSequence( maResults );
        return true;
    }
    return false;
}

bool ExternalName::getDdeLinkData( OUString& orDdeServer, OUString& orDdeTopic, OUString& orDdeItem )
{
    if( (mrParentLink.getLinkType() == LINKTYPE_DDE) && (maModel.maName.getLength() > 0) )
    {
        // try to create a DDE link and to set the imported link results
        if( !mbDdeLinkCreated ) try
        {
            Reference< XDDELinks > xDdeLinks( getDdeLinks(), UNO_QUERY_THROW );
            mxDdeLink = xDdeLinks->addDDELink( mrParentLink.getClassName(), mrParentLink.getTargetUrl(), maModel.maName, ::com::sun::star::sheet::DDELinkMode_DEFAULT );
            if( !maResults.empty() )
            {
                Reference< XDDELinkResults > xResults( mxDdeLink, UNO_QUERY_THROW );
                xResults->setResults( ContainerHelper::matrixToSequenceSequence( maResults ) );
            }
            mbDdeLinkCreated = true;
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "ExternalName::getDdeLinkData - cannot create DDE link" );
        }
        // get link data from created DDE link
        if( mxDdeLink.is() )
        {
            orDdeServer = mxDdeLink->getApplication();
            orDdeTopic = mxDdeLink->getTopic();
            orDdeItem = mxDdeLink->getItem();
            return true;
        }
    }
    return false;
}

void ExternalName::setResultSize( sal_Int32 nColumns, sal_Int32 nRows )
{
    OSL_ENSURE( (mrParentLink.getLinkType() == LINKTYPE_DDE) || (mrParentLink.getLinkType() == LINKTYPE_OLE) ||
        (mrParentLink.getLinkType() == LINKTYPE_MAYBE_DDE_OLE), "ExternalName::setResultSize - wrong link type" );
    OSL_ENSURE( (nRows > 0) && (nColumns > 0), "ExternalName::setResultSize - invalid matrix size" );
    const CellAddress& rMaxPos = getAddressConverter().getMaxApiAddress();
    if( (0 < nRows) && (nRows <= rMaxPos.Row + 1) && (0 < nColumns) && (nColumns <= rMaxPos.Column + 1) )
        maResults.resize( static_cast< size_t >( nColumns ), static_cast< size_t >( nRows ), Any( BiffHelper::calcDoubleFromError( BIFF_ERR_NA ) ) );
    else
        maResults.clear();
    maCurrIt = maResults.begin();
}

// ============================================================================

void LinkSheetRange::setDeleted()
{
    meType = LINKSHEETRANGE_INTERNAL;
    mnDocLink = mnFirst = mnLast = -1;
}

void LinkSheetRange::setSameSheet()
{
    meType = LINKSHEETRANGE_SAMESHEET;
    mnDocLink = -1;
    mnFirst = mnLast = 0;
}

void LinkSheetRange::setRange( sal_Int32 nFirst, sal_Int32 nLast )
{
    meType = LINKSHEETRANGE_INTERNAL;
    mnDocLink = -1;
    mnFirst = ::std::min( nFirst, nLast );
    mnLast = ::std::max( nFirst, nLast );
}

void LinkSheetRange::setExternalRange( sal_Int32 nDocLink, sal_Int32 nFirst, sal_Int32 nLast )
{
    if( nDocLink < 0 )
    {
        setDeleted();
    }
    else
    {
        meType = LINKSHEETRANGE_EXTERNAL;
        mnDocLink = nDocLink;
        mnFirst = ::std::min( nFirst, nLast );
        mnLast = ::std::max( nFirst, nLast );
    }
}

// ============================================================================

ExternalLink::ExternalLink( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    meLinkType( LINKTYPE_UNKNOWN ),
    meFuncLibType( FUNCLIB_UNKNOWN )
{
}

void ExternalLink::importExternalReference( const AttributeList& rAttribs )
{
    maRelId = rAttribs.getString( R_TOKEN( id ), OUString() );
}

void ExternalLink::importExternalBook( const Relations& rRelations, const AttributeList& rAttribs )
{
    parseExternalReference( rRelations, rAttribs.getString( R_TOKEN( id ), OUString() ) );
}

void ExternalLink::importSheetName( const AttributeList& rAttribs )
{
    insertExternalSheet( rAttribs.getXString( XML_val, OUString() ) );
}

void ExternalLink::importDefinedName( const AttributeList& rAttribs )
{
    createExternalName()->importDefinedName( rAttribs );
}

void ExternalLink::importDdeLink( const AttributeList& rAttribs )
{
    OUString aDdeService = rAttribs.getXString( XML_ddeService, OUString() );
    OUString aDdeTopic = rAttribs.getXString( XML_ddeTopic, OUString() );
    setDdeOleTargetUrl( aDdeService, aDdeTopic, LINKTYPE_DDE );
}

ExternalNameRef ExternalLink::importDdeItem( const AttributeList& rAttribs )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importDdeItem( rAttribs );
    return xExtName;
}

void ExternalLink::importOleLink( const Relations& rRelations, const AttributeList& rAttribs )
{
    OUString aProgId = rAttribs.getXString( XML_progId, OUString() );
    OUString aTargetUrl = rRelations.getExternalTargetFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    setDdeOleTargetUrl( aProgId, aTargetUrl, LINKTYPE_OLE );
}

ExternalNameRef ExternalLink::importOleItem( const AttributeList& rAttribs )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importOleItem( rAttribs );
    return xExtName;
}

void ExternalLink::importExternalRef( RecordInputStream& rStrm )
{
    rStrm >> maRelId;
}

void ExternalLink::importExternalSelf( RecordInputStream& )
{
    meLinkType = LINKTYPE_SELF;
}

void ExternalLink::importExternalSame( RecordInputStream& )
{
    meLinkType = LINKTYPE_SAME;
}

void ExternalLink::importExternalAddin( RecordInputStream& )
{
    meLinkType = LINKTYPE_UNKNOWN;
}

void ExternalLink::importExternalBook( const Relations& rRelations, RecordInputStream& rStrm )
{
    switch( rStrm.readuInt16() )
    {
        case OOBIN_EXTERNALBOOK_BOOK:
            parseExternalReference( rRelations, rStrm.readString() );
        break;
        case OOBIN_EXTERNALBOOK_DDE:
        {
            OUString aDdeService, aDdeTopic;
            rStrm >> aDdeService >> aDdeTopic;
            setDdeOleTargetUrl( aDdeService, aDdeTopic, LINKTYPE_DDE );
        }
        break;
        case OOBIN_EXTERNALBOOK_OLE:
        {
            OUString aTargetUrl = rRelations.getExternalTargetFromRelId( rStrm.readString() );
            OUString aProgId = rStrm.readString();
            setDdeOleTargetUrl( aProgId, aTargetUrl, LINKTYPE_OLE );
        }
        break;
        default:
            OSL_ENSURE( false, "ExternalLink::importExternalBook - unknown link type" );
    }
}

void ExternalLink::importExtSheetNames( RecordInputStream& rStrm )
{
    // load external sheet names and create the sheet caches in the Calc document
    OSL_ENSURE( (meLinkType == LINKTYPE_EXTERNAL) || (meLinkType == LINKTYPE_LIBRARY),
        "ExternalLink::importExtSheetNames - invalid link type" );
    if( meLinkType == LINKTYPE_EXTERNAL )   // ignore sheets of external libraries
        for( sal_Int32 nSheet = 0, nCount = rStrm.readInt32(); !rStrm.isEof() && (nSheet < nCount); ++nSheet )
            insertExternalSheet( rStrm.readString() );
}

ExternalNameRef ExternalLink::importExternalName( RecordInputStream& rStrm )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importExternalName( rStrm );
    return xExtName;
}

void ExternalLink::importExternSheet( BiffInputStream& rStrm )
{
    OStringBuffer aTargetBuffer( rStrm.readByteString( false, true ) );
    // references to own sheets have wrong string length field (off by 1)
    if( (aTargetBuffer.getLength() > 0) && (aTargetBuffer[ 0 ] == 3) )
        aTargetBuffer.append( static_cast< sal_Char >( rStrm.readuInt8() ) );
    // parse the encoded URL
    OUString aBiffTarget = OStringToOUString( aTargetBuffer.makeStringAndClear(), getTextEncoding() );
    OUString aSheetName = parseBiffTargetUrl( aBiffTarget );
    switch( meLinkType )
    {
        case LINKTYPE_INTERNAL:
            maIndexes.push_back( getWorksheets().getCalcSheetIndex( aSheetName ) );
        break;
        case LINKTYPE_EXTERNAL:
            insertExternalSheet( aSheetName );
        break;
        default:;
    }
}

void ExternalLink::importExternalBook( BiffInputStream& rStrm )
{
    OUString aTarget;
    sal_uInt16 nSheetCount;
    rStrm >> nSheetCount;
    if( rStrm.getRemaining() == 2 )
    {
        if( rStrm.readuInt8() == 1 )
        {
            sal_Char cChar = static_cast< sal_Char >( rStrm.readuInt8() );
            if( cChar != 0 )
                aTarget = OStringToOUString( OString( cChar ), getTextEncoding() );
        }
    }
    else if( rStrm.getRemaining() >= 3 )
    {
        // NUL characters may occur
        aTarget = rStrm.readUniString( true );
    }

    // parse the encoded URL
    OUString aDummySheetName = parseBiffTargetUrl( aTarget );
    OSL_ENSURE( aDummySheetName.getLength() == 0, "ExternalLink::importExternalBook - sheet name in encoded URL" );
    (void)aDummySheetName;  // prevent compiler warning

    // load external sheet names and create the sheet caches in the Calc document
    if( meLinkType == LINKTYPE_EXTERNAL )
        for( sal_uInt16 nSheet = 0; !rStrm.isEof() && (nSheet < nSheetCount); ++nSheet )
            insertExternalSheet( rStrm.readUniString() );
}

void ExternalLink::importExternalName( BiffInputStream& rStrm )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importExternalName( rStrm );
    switch( meLinkType )
    {
        case LINKTYPE_DDE:
            OSL_ENSURE( !xExtName->isOleObject(), "ExternalLink::importExternalName - OLE object in DDE link" );
        break;
        case LINKTYPE_OLE:
            OSL_ENSURE( xExtName->isOleObject(), "ExternalLink::importExternalName - anything but OLE object in OLE link" );
        break;
        case LINKTYPE_MAYBE_DDE_OLE:
            meLinkType = xExtName->isOleObject() ? LINKTYPE_OLE : LINKTYPE_DDE;
        break;
        default:
            OSL_ENSURE( !xExtName->isOleObject(), "ExternalLink::importExternalName - OLE object in external name" );
    }
}

ExternalLinkInfo ExternalLink::getLinkInfo() const
{
    ExternalLinkInfo aLinkInfo;
    switch( meLinkType )
    {
        case LINKTYPE_EXTERNAL:
            aLinkInfo.Type = ::com::sun::star::sheet::ExternalLinkType::DOCUMENT;
            aLinkInfo.Data <<= maTargetUrl;
        break;
        case LINKTYPE_DDE:
        {
            aLinkInfo.Type = ::com::sun::star::sheet::ExternalLinkType::DDE;
            DDELinkInfo aDdeLinkInfo;
            aDdeLinkInfo.Service = maClassName;
            aDdeLinkInfo.Topic = maTargetUrl;
            ::std::vector< DDEItemInfo > aItemInfos;
            DDEItemInfo aItemInfo;
            for( ExternalNameVector::const_iterator aIt = maExtNames.begin(), aEnd = maExtNames.end(); aIt != aEnd; ++aIt )
                if( (*aIt)->getDdeItemInfo( aItemInfo ) )
                    aItemInfos.push_back( aItemInfo );
            aDdeLinkInfo.Items = ContainerHelper::vectorToSequence( aItemInfos );
            aLinkInfo.Data <<= aDdeLinkInfo;
        }
        break;
        default:
            aLinkInfo.Type = ::com::sun::star::sheet::ExternalLinkType::UNKNOWN;
    }
    return aLinkInfo;
}

FunctionLibraryType ExternalLink::getFuncLibraryType() const
{
    return (meLinkType == LINKTYPE_LIBRARY) ? meFuncLibType : FUNCLIB_UNKNOWN;
}

sal_Int32 ExternalLink::getSheetIndex( sal_Int32 nTabId ) const
{
    OSL_ENSURE( (nTabId == 0) || (getFilterType() == FILTER_OOX) || (getBiff() == BIFF8),
        "ExternalLink::getSheetIndex - invalid sheet index" );
    return ContainerHelper::getVectorElement< sal_Int32 >( maIndexes, nTabId, -1 );
}

void ExternalLink::getSheetRange( LinkSheetRange& orSheetRange, sal_Int32 nTabId1, sal_Int32 nTabId2 ) const
{
    switch( meLinkType )
    {
        case LINKTYPE_SAME:
            orSheetRange.setSameSheet();
        break;

        case LINKTYPE_SELF:
        case LINKTYPE_INTERNAL:
            orSheetRange.setRange( nTabId1, nTabId2 );
        break;

        case LINKTYPE_EXTERNAL:
        {
            sal_Int32 nDocLinkIndex = mxDocLink.is() ? mxDocLink->getTokenIndex() : -1;
            switch( getFilterType() )
            {
                case FILTER_OOX:
                    // OOBIN: passed indexes point into sheet list of EXTSHEETLIST
                    orSheetRange.setExternalRange( nDocLinkIndex, getSheetIndex( nTabId1 ), getSheetIndex( nTabId2 ) );
                break;
                case FILTER_BIFF:
                    switch( getBiff() )
                    {
                        case BIFF2:
                        case BIFF3:
                        case BIFF4:
                            orSheetRange.setExternalRange( nDocLinkIndex, getSheetIndex( nTabId1 ), getSheetIndex( nTabId2 ) );
                        break;
                        case BIFF5:
                            // BIFF5: first sheet from this external link, last sheet is passed in nTabId2
                            if( const ExternalLink* pExtLink2 = getExternalLinks().getExternalLink( nTabId2 ).get() )
                                if( (pExtLink2->getLinkType() == LINKTYPE_EXTERNAL) && (maTargetUrl == pExtLink2->getTargetUrl()) )
                                    orSheetRange.setExternalRange( nDocLinkIndex, getSheetIndex(), pExtLink2->getSheetIndex() );
                        break;
                        case BIFF8:
                            // BIFF8: passed indexes point into sheet list of EXTERNALBOOK
                            orSheetRange.setExternalRange( nDocLinkIndex, getSheetIndex( nTabId1 ), getSheetIndex( nTabId2 ) );
                        break;
                        case BIFF_UNKNOWN: break;
                    }
                break;
                case FILTER_UNKNOWN: break;
            }
        }
        break;

        default:
            // unsupported/unexpected link type: #REF! error
            orSheetRange.setDeleted();
    }
}

Reference< XExternalSheetCache > ExternalLink::getExternalSheetCache( sal_Int32 nTabId )
{
    const sal_Int32* pnCacheId = ContainerHelper::getVectorElement( maIndexes, nTabId );
    if( mxDocLink.is() && pnCacheId ) try
    {
        // existing mxDocLink implies that this is an external link
        Reference< XExternalSheetCache > xSheetCache( mxDocLink->getByIndex( *pnCacheId ), UNO_QUERY_THROW );
        return xSheetCache;
    }
    catch( Exception& )
    {
    }
    return 0;
}

ExternalNameRef ExternalLink::getNameByIndex( sal_Int32 nIndex ) const
{
    return maExtNames.get( nIndex );
}

// private --------------------------------------------------------------------

#define OOX_TARGETTYPE_EXTLINK      CREATE_OFFICEDOC_RELATIONSTYPE( "externalLinkPath" )
#define OOX_TARGETTYPE_LIBRARY      CREATE_MSOFFICE_RELATIONSTYPE( "xlExternalLinkPath/xlLibrary" )

void ExternalLink::setExternalTargetUrl( const OUString& rTargetUrl, const OUString& rTargetType )
{
    meLinkType = LINKTYPE_UNKNOWN;
    if( rTargetType == OOX_TARGETTYPE_EXTLINK )
    {
        maTargetUrl = getBaseFilter().getAbsoluteUrl( rTargetUrl );
        if( maTargetUrl.getLength() > 0 )
            meLinkType = LINKTYPE_EXTERNAL;
    }
    else if( rTargetType == OOX_TARGETTYPE_LIBRARY )
    {
        meLinkType = LINKTYPE_LIBRARY;
        meFuncLibType = getFormulaParser().getFuncLibTypeFromLibraryName( rTargetUrl );
    }
    OSL_ENSURE( meLinkType != LINKTYPE_UNKNOWN, "ExternalLink::setExternalTargetUrl - empty target URL or unknown target type" );

    // create the external document link API object that will contain the sheet caches
    if( meLinkType == LINKTYPE_EXTERNAL )
    {
        Reference< XExternalDocLinks > xDocLinks = getExternalDocLinks();
        if( xDocLinks.is() )
            mxDocLink = xDocLinks->addDocLink( maTargetUrl );
    }
}

void ExternalLink::setDdeOleTargetUrl( const OUString& rClassName, const OUString& rTargetUrl, ExternalLinkType eLinkType )
{
    maClassName = rClassName;
    maTargetUrl = rTargetUrl;
    meLinkType = ((maClassName.getLength() > 0) && (maTargetUrl.getLength() > 0)) ? eLinkType : LINKTYPE_UNKNOWN;
    OSL_ENSURE( meLinkType == eLinkType, "ExternalLink::setDdeOleTargetUrl - missing classname or target" );
}

void ExternalLink::parseExternalReference( const Relations& rRelations, const OUString& rRelId )
{
    if( const Relation* pRelation = rRelations.getRelationFromRelId( rRelId ) )
        setExternalTargetUrl( pRelation->maTarget, pRelation->maType );
}

OUString ExternalLink::parseBiffTargetUrl( const OUString& rBiffTargetUrl )
{
    meLinkType = LINKTYPE_UNKNOWN;

    OUString aClassName, aTargetUrl, aSheetName;
    switch( getAddressConverter().parseBiffTargetUrl( aClassName, aTargetUrl, aSheetName, rBiffTargetUrl ) )
        {
        case BIFF_TARGETTYPE_URL:
            if( aTargetUrl.getLength() == 0 )
        {
                meLinkType = (aSheetName.getLength() > 0) ? LINKTYPE_INTERNAL : LINKTYPE_SELF;
        }
        else if( (aTargetUrl.getLength() == 1) && (aTargetUrl[ 0 ] == ':') )
        {
            if( getBiff() >= BIFF4 )
                meLinkType = LINKTYPE_ANALYSIS;
        }
            else if( (aTargetUrl.getLength() > 1) || (aTargetUrl[ 0 ] != ' ') )
        {
                setExternalTargetUrl( aTargetUrl, OOX_TARGETTYPE_EXTLINK );
        }
        break;

        case BIFF_TARGETTYPE_SAMESHEET:
            OSL_ENSURE( (aTargetUrl.getLength() == 0) && (aSheetName.getLength() == 0), "ExternalLink::parseBiffTargetUrl - unexpected target or sheet name" );
            meLinkType = LINKTYPE_SAME;
        break;

        case BIFF_TARGETTYPE_LIBRARY:
            OSL_ENSURE( aSheetName.getLength() == 0, "ExternalLink::parseBiffTargetUrl - unexpected sheet name" );
            setExternalTargetUrl( aTargetUrl, OOX_TARGETTYPE_LIBRARY );
        break;

        case BIFF_TARGETTYPE_DDE_OLE:
            setDdeOleTargetUrl( aClassName, aTargetUrl, LINKTYPE_MAYBE_DDE_OLE );
        break;

        case BIFF_TARGETTYPE_UNKNOWN:
        break;
    }
    return aSheetName;
}

void ExternalLink::insertExternalSheet( const OUString& rSheetName )
{
    OSL_ENSURE( rSheetName.getLength() > 0, "ExternalLink::insertExternalSheet - empty sheet name" );
    if( mxDocLink.is() )
    {
        Reference< XExternalSheetCache > xSheetCache = mxDocLink->addSheetCache( rSheetName );
        sal_Int32 nCacheIdx = xSheetCache.is() ? xSheetCache->getTokenIndex() : -1;
        maIndexes.push_back( nCacheIdx );
    }
}

ExternalNameRef ExternalLink::createExternalName()
{
    ExternalNameRef xExtName( new ExternalName( *this, getSheetIndex() ) );
    maExtNames.push_back( xExtName );
    return xExtName;
}

// ============================================================================

RefSheetsModel::RefSheetsModel() :
    mnExtRefId( -1 ),
    mnTabId1( -1 ),
    mnTabId2( -1 )
{
}

void RefSheetsModel::readOobData( RecordInputStream& rStrm )
{
    rStrm >> mnExtRefId >> mnTabId1 >> mnTabId2;
}

void RefSheetsModel::readBiff8Data( BiffInputStream& rStrm )
{
    mnExtRefId = rStrm.readuInt16();
    mnTabId1 = rStrm.readInt16();
    mnTabId2 = rStrm.readInt16();
}

// ----------------------------------------------------------------------------

ExternalLinkBuffer::ExternalLinkBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbUseRefSheets( false )
{
}

ExternalLinkRef ExternalLinkBuffer::importExternalReference( const AttributeList& rAttribs )
{
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalReference( rAttribs );
    maExtLinks.push_back( xExtLink );
    return xExtLink;
}

ExternalLinkRef ExternalLinkBuffer::importExternalRef( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalRef( rStrm );
    maExtLinks.push_back( xExtLink );
    return xExtLink;
}

void ExternalLinkBuffer::importExternalSelf( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalSelf( rStrm );
}

void ExternalLinkBuffer::importExternalSame( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalSame( rStrm );
}

void ExternalLinkBuffer::importExternalAddin( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalAddin( rStrm );
}

void ExternalLinkBuffer::importExternalSheets( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbUseRefSheets, "ExternalLinkBuffer::importExternalSheets - missing EXTERNALREFS records" );
    mbUseRefSheets = true;
    OSL_ENSURE( maRefSheets.empty(), "ExternalLinkBuffer::importExternalSheets - multiple EXTERNALSHEETS records" );
    maRefSheets.clear();
    sal_Int32 nRefCount;
    rStrm >> nRefCount;
    size_t nMaxCount = getLimitedValue< size_t, sal_Int64 >( nRefCount, 0, rStrm.getRemaining() / 12 );
    maRefSheets.reserve( nMaxCount );
    for( size_t nRefId = 0; !rStrm.isEof() && (nRefId < nMaxCount); ++nRefId )
    {
        RefSheetsModel aRefSheets;
        aRefSheets.readOobData( rStrm );
        maRefSheets.push_back( aRefSheets );
    }
}

ExternalLinkRef ExternalLinkBuffer::importExternSheet( BiffInputStream& rStrm )
{
    OSL_ENSURE( getBiff() <= BIFF5, "ExternalLinkBuffer::importExternSheet - wrong BIFF version" );
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternSheet( rStrm );
    return xExtLink;
}

ExternalLinkRef ExternalLinkBuffer::importExternalBook( BiffInputStream& rStrm )
{
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalBook( rStrm );
    return xExtLink;
}

void ExternalLinkBuffer::importExternalName( BiffInputStream& rStrm )
{
    if( !maLinks.empty() )
        maLinks.back()->importExternalName( rStrm );
}

void ExternalLinkBuffer::importExternSheet8( BiffInputStream& rStrm )
{
    OSL_ENSURE( getBiff() == BIFF8, "ExternalLinkBuffer::importExternSheet - wrong BIFF version" );
    OSL_ENSURE( maRefSheets.empty(), "ExternalLinkBuffer::importExternSheet - multiple EXTERNSHEET records" );
    maRefSheets.clear();
    sal_uInt16 nRefCount;
    rStrm >> nRefCount;
    maRefSheets.reserve( nRefCount );
    for( sal_uInt16 nRefId = 0; !rStrm.isEof() && (nRefId < nRefCount); ++nRefId )
    {
        RefSheetsModel aRefSheets;
        aRefSheets.readBiff8Data( rStrm );
        maRefSheets.push_back( aRefSheets );
    }
}

Sequence< ExternalLinkInfo > ExternalLinkBuffer::getLinkInfos() const
{
    ::std::vector< ExternalLinkInfo > aLinkInfos;
    // dummy entry for index 0
    aLinkInfos.push_back( ExternalLinkInfo( ::com::sun::star::sheet::ExternalLinkType::UNKNOWN, Any() ) );
    for( ExternalLinkVec::const_iterator aIt = maExtLinks.begin(), aEnd = maExtLinks.end(); aIt != aEnd; ++aIt )
        aLinkInfos.push_back( (*aIt)->getLinkInfo() );
    return ContainerHelper::vectorToSequence( aLinkInfos );
}

ExternalLinkRef ExternalLinkBuffer::getExternalLink( sal_Int32 nRefId ) const
{
    ExternalLinkRef xExtLink;
    switch( getFilterType() )
    {
        case FILTER_OOX:
            // OOXML: one-based index
            if( !mbUseRefSheets )
                xExtLink = maLinks.get( nRefId - 1 );
            // OOBIN: zero-based index into ref-sheets list
            else if( const RefSheetsModel* pRefSheets = getRefSheets( nRefId ) )
                xExtLink = maLinks.get( pRefSheets->mnExtRefId );
        break;
        case FILTER_BIFF:
            switch( getBiff() )
            {
                case BIFF2:
                case BIFF3:
                case BIFF4:
                    // one-based index to EXTERNSHEET records
                    xExtLink = maLinks.get( nRefId - 1 );
                break;
                case BIFF5:
                    if( nRefId < 0 )
                    {
                        // internal links in formula tokens have negative index
                        xExtLink = maLinks.get( -nRefId - 1 );
                        if( xExtLink.get() && !xExtLink->isInternalLink() )
                            xExtLink.reset();
                    }
                    else
                    {
                        // one-based index to EXTERNSHEET records
                        xExtLink = maLinks.get( nRefId - 1 );
                    }
                break;
                case BIFF8:
                    // zero-based index into REF list in EXTERNSHEET record
                    if( const RefSheetsModel* pRefSheets = getRefSheets( nRefId ) )
                        xExtLink = maLinks.get( pRefSheets->mnExtRefId );
                break;
                case BIFF_UNKNOWN: break;
            }
        break;
        case FILTER_UNKNOWN: break;
    }
    return xExtLink;
}

LinkSheetRange ExternalLinkBuffer::getSheetRange( sal_Int32 nRefId, sal_Int16 nTabId1, sal_Int16 nTabId2 ) const
{
    OSL_ENSURE( getBiff() <= BIFF5, "ExternalLinkBuffer::getSheetRange - wrong BIFF version" );
    LinkSheetRange aSheetRange;
    if( const ExternalLink* pExtLink = getExternalLink( nRefId ).get() )
        pExtLink->getSheetRange( aSheetRange, nTabId1, nTabId2 );
    return aSheetRange;
}

LinkSheetRange ExternalLinkBuffer::getSheetRange( sal_Int32 nRefId ) const
{
    OSL_ENSURE( ((getFilterType() == FILTER_OOX) && mbUseRefSheets) || (getBiff() == BIFF8), "ExternalLinkBuffer::getSheetRange - wrong BIFF version" );
    LinkSheetRange aSheetRange;
    if( const ExternalLink* pExtLink = getExternalLink( nRefId ).get() )
        if( const RefSheetsModel* pRefSheets = getRefSheets( nRefId ) )
            pExtLink->getSheetRange( aSheetRange, pRefSheets->mnTabId1, pRefSheets->mnTabId2 );
    return aSheetRange;
}

// private --------------------------------------------------------------------

ExternalLinkRef ExternalLinkBuffer::createExternalLink()
{
    ExternalLinkRef xExtLink( new ExternalLink( *this ) );
    maLinks.push_back( xExtLink );
    return xExtLink;
}

const RefSheetsModel* ExternalLinkBuffer::getRefSheets( sal_Int32 nRefId ) const
{
    return ((0 <= nRefId) && (static_cast< size_t >( nRefId ) < maRefSheets.size())) ?
        &maRefSheets[ static_cast< size_t >( nRefId ) ] : 0;
}

// ============================================================================

} // namespace xls
} // namespace oox

