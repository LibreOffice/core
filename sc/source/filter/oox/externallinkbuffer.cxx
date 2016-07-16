/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "externallinkbuffer.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/DDELinkInfo.hpp>
#include <com/sun/star/sheet/ExternalLinkType.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/XDDELinks.hpp>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/sheet/XDDELinkResults.hpp>
#include <com/sun/star/sheet/XExternalDocLink.hpp>
#include <com/sun/star/sheet/XExternalDocLinks.hpp>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "excelhandlers.hxx"
#include "formulaparser.hxx"
#include "worksheetbuffer.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::oox::core::Relation;
using ::oox::core::Relations;

namespace {

const sal_uInt16 BIFF12_EXTERNALBOOK_BOOK   = 0;
const sal_uInt16 BIFF12_EXTERNALBOOK_DDE    = 1;
const sal_uInt16 BIFF12_EXTERNALBOOK_OLE    = 2;

const sal_uInt16 BIFF12_EXTNAME_AUTOMATIC   = 0x0002;
const sal_uInt16 BIFF12_EXTNAME_PREFERPIC   = 0x0004;
const sal_uInt16 BIFF12_EXTNAME_STDDOCNAME  = 0x0008;
const sal_uInt16 BIFF12_EXTNAME_OLEOBJECT   = 0x0010;
const sal_uInt16 BIFF12_EXTNAME_ICONIFIED   = 0x0020;

} // namespace

ExternalNameModel::ExternalNameModel() :
    mbNotify( false ),
    mbPreferPic( false ),
    mbStdDocName( false ),
    mbOleObj( false ),
    mbIconified( false )
{
}

ExternalName::ExternalName( const ExternalLink& rParentLink ) :
    DefinedNameBase( rParentLink ),
    mrParentLink( rParentLink ),
    mbDdeLinkCreated( false )
{
}

void ExternalName::importDefinedName( const AttributeList& rAttribs )
{
    maModel.maName = rAttribs.getXString( XML_name, OUString() );
    OSL_ENSURE( !maModel.maName.isEmpty(), "ExternalName::importDefinedName - empty name" );
    // zero-based index into sheet list of externalBook
    maModel.mnSheet = rAttribs.getInteger( XML_sheetId, -1 );
}

void ExternalName::importDdeItem( const AttributeList& rAttribs )
{
    maModel.maName = rAttribs.getXString( XML_name, OUString() );
    OSL_ENSURE( !maModel.maName.isEmpty(), "ExternalName::importDdeItem - empty name" );
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
    OSL_ENSURE( !maModel.maName.isEmpty(), "ExternalName::importOleItem - empty name" );
    maExtNameModel.mbOleObj    = true;
    maExtNameModel.mbNotify    = rAttribs.getBool( XML_advise, false );
    maExtNameModel.mbPreferPic = rAttribs.getBool( XML_preferPic, false );
    maExtNameModel.mbIconified = rAttribs.getBool( XML_icon, false );
}

void ExternalName::importExternalName( SequenceInputStream& rStrm )
{
    rStrm >> maModel.maName;
    OSL_ENSURE( !maModel.maName.isEmpty(), "ExternalName::importExternalName - empty name" );
}

void ExternalName::importExternalNameFlags( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    sal_Int32 nSheetId;
    nFlags = rStrm.readuInt16();
    nSheetId = rStrm.readInt32();
    // index into sheet list of EXTSHEETNAMES (one-based in BIFF12)
    maModel.mnSheet = nSheetId - 1;
    // no flag for built-in names, as in OOXML...
    maExtNameModel.mbNotify     = getFlag( nFlags, BIFF12_EXTNAME_AUTOMATIC );
    maExtNameModel.mbPreferPic  = getFlag( nFlags, BIFF12_EXTNAME_PREFERPIC );
    maExtNameModel.mbStdDocName = getFlag( nFlags, BIFF12_EXTNAME_STDDOCNAME );
    maExtNameModel.mbOleObj     = getFlag( nFlags, BIFF12_EXTNAME_OLEOBJECT );
    maExtNameModel.mbIconified  = getFlag( nFlags, BIFF12_EXTNAME_ICONIFIED );
    OSL_ENSURE( (mrParentLink.getLinkType() == LINKTYPE_OLE) == maExtNameModel.mbOleObj,
        "ExternalName::importExternalNameFlags - wrong OLE flag in external name" );
}

void ExternalName::importDdeItemValues( SequenceInputStream& rStrm )
{
    sal_Int32 nRows, nCols;
    nRows = rStrm.readInt32();
    nCols = rStrm.readInt32();
    setResultSize( nCols, nRows );
}

void ExternalName::importDdeItemBool( SequenceInputStream& rStrm )
{
    appendResultValue< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
}

void ExternalName::importDdeItemDouble( SequenceInputStream& rStrm )
{
    appendResultValue( rStrm.readDouble() );
}

void ExternalName::importDdeItemError( SequenceInputStream& rStrm )
{
    appendResultValue( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) );
}

void ExternalName::importDdeItemString( SequenceInputStream& rStrm )
{
    appendResultValue( BiffHelper::readString( rStrm ) );
}

bool ExternalName::getDdeItemInfo( DDEItemInfo& orItemInfo ) const
{
    if( (mrParentLink.getLinkType() == LINKTYPE_DDE) && !maModel.maName.isEmpty() )
    {
        orItemInfo.Item = maModel.maName;
        orItemInfo.Results = ContainerHelper::matrixToSequenceSequence( maResults );
        return true;
    }
    return false;
}

bool ExternalName::getDdeLinkData( OUString& orDdeServer, OUString& orDdeTopic, OUString& orDdeItem )
{
    if( (mrParentLink.getLinkType() == LINKTYPE_DDE) && !maModel.maName.isEmpty() )
    {
        // try to create a DDE link and to set the imported link results
        if( !mbDdeLinkCreated ) try
        {
            PropertySet aDocProps( getDocument() );
            Reference< XDDELinks > xDdeLinks( aDocProps.getAnyProperty( PROP_DDELinks ), UNO_QUERY_THROW );
            mxDdeLink = xDdeLinks->addDDELink( mrParentLink.getClassName(), mrParentLink.getTargetUrl(), maModel.maName, css::sheet::DDELinkMode_DEFAULT );
            mbDdeLinkCreated = true;    // ignore if setting results fails
            if( !maResults.empty() )
            {
                Reference< XDDELinkResults > xResults( mxDdeLink, UNO_QUERY_THROW );
                xResults->setResults( ContainerHelper::matrixToSequenceSequence( maResults ) );
            }
        }
        catch( Exception& )
        {
            OSL_FAIL( "ExternalName::getDdeLinkData - cannot create DDE link" );
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

// private --------------------------------------------------------------------

void ExternalName::setResultSize( sal_Int32 nColumns, sal_Int32 nRows )
{
    OSL_ENSURE( (mrParentLink.getLinkType() == LINKTYPE_DDE) || (mrParentLink.getLinkType() == LINKTYPE_OLE) ||
        (mrParentLink.getLinkType() == LINKTYPE_MAYBE_DDE_OLE), "ExternalName::setResultSize - wrong link type" );
    OSL_ENSURE( (nRows > 0) && (nColumns > 0), "ExternalName::setResultSize - invalid matrix size" );
    const ScAddress& rMaxPos = getAddressConverter().getMaxApiAddress();
    if( (0 < nRows) && (nRows <= rMaxPos.Row() + 1) && (0 < nColumns) && (nColumns <= rMaxPos.Col() + 1) )
        maResults.resize( static_cast< size_t >( nColumns ), static_cast< size_t >( nRows ), Any( BiffHelper::calcDoubleFromError( BIFF_ERR_NA ) ) );
    else
        maResults.clear();
    maCurrIt = maResults.begin();
}

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

void ExternalLink::importExternalRef( SequenceInputStream& rStrm )
{
    rStrm >> maRelId;
}

void ExternalLink::importExternalSelf( SequenceInputStream& )
{
    meLinkType = LINKTYPE_SELF;
}

void ExternalLink::importExternalSame( SequenceInputStream& )
{
    meLinkType = LINKTYPE_SAME;
}

void ExternalLink::importExternalAddin( SequenceInputStream& )
{
    meLinkType = LINKTYPE_UNKNOWN;
}

void ExternalLink::importExternalBook( const Relations& rRelations, SequenceInputStream& rStrm )
{
    switch( rStrm.readuInt16() )
    {
        case BIFF12_EXTERNALBOOK_BOOK:
            parseExternalReference( rRelations, BiffHelper::readString( rStrm ) );
        break;
        case BIFF12_EXTERNALBOOK_DDE:
        {
            OUString aDdeService, aDdeTopic;
            rStrm >> aDdeService >> aDdeTopic;
            setDdeOleTargetUrl( aDdeService, aDdeTopic, LINKTYPE_DDE );
        }
        break;
        case BIFF12_EXTERNALBOOK_OLE:
        {
            OUString aTargetUrl = rRelations.getExternalTargetFromRelId( BiffHelper::readString( rStrm ) );
            OUString aProgId = BiffHelper::readString( rStrm );
            setDdeOleTargetUrl( aProgId, aTargetUrl, LINKTYPE_OLE );
        }
        break;
        default:
            OSL_FAIL( "ExternalLink::importExternalBook - unknown link type" );
    }
}

void ExternalLink::importExtSheetNames( SequenceInputStream& rStrm )
{
    // load external sheet names and create the sheet caches in the Calc document
    OSL_ENSURE( (meLinkType == LINKTYPE_EXTERNAL) || (meLinkType == LINKTYPE_LIBRARY),
        "ExternalLink::importExtSheetNames - invalid link type" );
    if( meLinkType == LINKTYPE_EXTERNAL )   // ignore sheets of external libraries
        for( sal_Int32 nSheet = 0, nCount = rStrm.readInt32(); !rStrm.isEof() && (nSheet < nCount); ++nSheet )
            insertExternalSheet( BiffHelper::readString( rStrm ) );
}

ExternalNameRef ExternalLink::importExternalName( SequenceInputStream& rStrm )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importExternalName( rStrm );
    return xExtName;
}

ExternalLinkInfo ExternalLink::getLinkInfo() const
{
    ExternalLinkInfo aLinkInfo;
    switch( meLinkType )
    {
        case LINKTYPE_SELF:
        case LINKTYPE_SAME:
        case LINKTYPE_INTERNAL:
            aLinkInfo.Type = css::sheet::ExternalLinkType::SELF;
        break;
        case LINKTYPE_EXTERNAL:
            aLinkInfo.Type = css::sheet::ExternalLinkType::DOCUMENT;
            aLinkInfo.Data <<= maTargetUrl;
        break;
        case LINKTYPE_LIBRARY:
            // parser will return library function names in OPCODE_BAD string tokens
            aLinkInfo.Type = css::sheet::ExternalLinkType::SPECIAL;
        break;
        case LINKTYPE_DDE:
        {
            aLinkInfo.Type = css::sheet::ExternalLinkType::DDE;
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
            aLinkInfo.Type = css::sheet::ExternalLinkType::UNKNOWN;
    }
    return aLinkInfo;
}

FunctionLibraryType ExternalLink::getFuncLibraryType() const
{
    return (meLinkType == LINKTYPE_LIBRARY) ? meFuncLibType : FUNCLIB_UNKNOWN;
}

sal_Int32 ExternalLink::getDocumentLinkIndex() const
{
    OSL_ENSURE( meLinkType == LINKTYPE_EXTERNAL, "ExternalLink::getDocumentLinkIndex - invalid link type" );
    return mxDocLink.is() ? mxDocLink->getTokenIndex() : -1;
}

sal_Int32 ExternalLink::getSheetCacheIndex( sal_Int32 nTabId ) const
{
    OSL_ENSURE( meLinkType == LINKTYPE_EXTERNAL, "ExternalLink::getSheetCacheIndex - invalid link type" );
    OSL_ENSURE( nTabId == 0,
        "ExternalLink::getSheetCacheIndex - invalid sheet index" );
    return ContainerHelper::getVectorElement( maSheetCaches, nTabId, -1 );
}

Reference< XExternalSheetCache > ExternalLink::getSheetCache( sal_Int32 nTabId ) const
{
    sal_Int32 nCacheIdx = getSheetCacheIndex( nTabId );
    if( mxDocLink.is() && (nCacheIdx >= 0) ) try
    {
        // existing mxDocLink implies that this is an external link
        Reference< XExternalSheetCache > xSheetCache( mxDocLink->getByIndex( nCacheIdx ), UNO_QUERY_THROW );
        return xSheetCache;
    }
    catch( Exception& )
    {
    }
    return nullptr;
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
            sal_Int32 nDocLinkIdx = getDocumentLinkIndex();
            // BIFF12: passed indexes point into sheet list of EXTSHEETLIST
            orSheetRange.setExternalRange( nDocLinkIdx, getSheetCacheIndex( nTabId1 ), getSheetCacheIndex( nTabId2 ) );
        }
        break;

        default:
            // unsupported/unexpected link type: #REF! error
            orSheetRange.setDeleted();
    }
}

ExternalNameRef ExternalLink::getNameByIndex( sal_Int32 nIndex ) const
{
    return maExtNames.get( nIndex );
}

// private --------------------------------------------------------------------

void ExternalLink::setExternalTargetUrl( const OUString& rTargetUrl, const OUString& rTargetType )
{
    meLinkType = LINKTYPE_UNKNOWN;
    if( rTargetType == CREATE_OFFICEDOC_RELATION_TYPE( "externalLinkPath" ) ||
            rTargetType == CREATE_OFFICEDOC_RELATION_TYPE_STRICT( "externalLinkPath" ) )
    {
        maTargetUrl = getBaseFilter().getAbsoluteUrl( rTargetUrl );
        if( !maTargetUrl.isEmpty() )
            meLinkType = LINKTYPE_EXTERNAL;
    }
    else if( rTargetType == CREATE_MSOFFICE_RELATION_TYPE( "xlExternalLinkPath/xlLibrary" ) )
    {
        meLinkType = LINKTYPE_LIBRARY;
        meFuncLibType = FunctionProvider::getFuncLibTypeFromLibraryName( rTargetUrl );
    }
    OSL_ENSURE( meLinkType != LINKTYPE_UNKNOWN, "ExternalLink::setExternalTargetUrl - empty target URL or unknown target type" );

    // create the external document link API object that will contain the sheet caches
    if( meLinkType == LINKTYPE_EXTERNAL ) try
    {
        PropertySet aDocProps( getDocument() );
        Reference< XExternalDocLinks > xDocLinks( aDocProps.getAnyProperty( PROP_ExternalDocLinks ), UNO_QUERY_THROW );
        mxDocLink = xDocLinks->addDocLink( maTargetUrl );
    }
    catch( Exception& )
    {
    }
}

void ExternalLink::setDdeOleTargetUrl( const OUString& rClassName, const OUString& rTargetUrl, ExternalLinkType eLinkType )
{
    maClassName = rClassName;
    maTargetUrl = rTargetUrl;
    meLinkType = (maClassName.isEmpty() || maTargetUrl.isEmpty()) ?  LINKTYPE_UNKNOWN : eLinkType;
    OSL_ENSURE( meLinkType == eLinkType, "ExternalLink::setDdeOleTargetUrl - missing classname or target" );
}

void ExternalLink::parseExternalReference( const Relations& rRelations, const OUString& rRelId )
{
    if( const Relation* pRelation = rRelations.getRelationFromRelId( rRelId ) )
        setExternalTargetUrl( pRelation->maTarget, pRelation->maType );
}

void ExternalLink::insertExternalSheet( const OUString& rSheetName )
{
    OSL_ENSURE( !rSheetName.isEmpty(), "ExternalLink::insertExternalSheet - empty sheet name" );
    if( mxDocLink.is() )
    {
        Reference< XExternalSheetCache > xSheetCache = mxDocLink->addSheetCache( rSheetName, false );
        sal_Int32 nCacheIdx = xSheetCache.is() ? xSheetCache->getTokenIndex() : -1;
        maSheetCaches.push_back( nCacheIdx );
    }
}

ExternalNameRef ExternalLink::createExternalName()
{
    ExternalNameRef xExtName( new ExternalName( *this ) );
    maExtNames.push_back( xExtName );
    return xExtName;
}

RefSheetsModel::RefSheetsModel() :
    mnExtRefId( -1 ),
    mnTabId1( -1 ),
    mnTabId2( -1 )
{
}

void RefSheetsModel::readBiff12Data( SequenceInputStream& rStrm )
{
    mnExtRefId = rStrm.readInt32();
    mnTabId1 = rStrm.readInt32();
    mnTabId2 = rStrm.readInt32();
}

ExternalLinkBuffer::ExternalLinkBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mxSelfRef( new ExternalLink( rHelper ) ),
    mbUseRefSheets( false )
{
    mxSelfRef->setSelfLinkType();
}

ExternalLinkRef ExternalLinkBuffer::importExternalReference( const AttributeList& rAttribs )
{
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalReference( rAttribs );
    maExtLinks.push_back( xExtLink );
    return xExtLink;
}

ExternalLinkRef ExternalLinkBuffer::importExternalRef( SequenceInputStream& rStrm )
{
    mbUseRefSheets = true;
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalRef( rStrm );
    maExtLinks.push_back( xExtLink );
    return xExtLink;
}

void ExternalLinkBuffer::importExternalSelf( SequenceInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalSelf( rStrm );
}

void ExternalLinkBuffer::importExternalSame( SequenceInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalSame( rStrm );
}

void ExternalLinkBuffer::importExternalAddin( SequenceInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalAddin( rStrm );
}

void ExternalLinkBuffer::importExternalSheets( SequenceInputStream& rStrm )
{
    OSL_ENSURE( mbUseRefSheets, "ExternalLinkBuffer::importExternalSheets - missing EXTERNALREFS records" );
    mbUseRefSheets = true;
    OSL_ENSURE( maRefSheets.empty(), "ExternalLinkBuffer::importExternalSheets - multiple EXTERNALSHEETS records" );
    maRefSheets.clear();
    sal_Int32 nRefCount;
    nRefCount = rStrm.readInt32();
    size_t nMaxCount = getLimitedValue< size_t, sal_Int64 >( nRefCount, 0, rStrm.getRemaining() / 12 );
    maRefSheets.reserve( nMaxCount );
    for( size_t nRefId = 0; !rStrm.isEof() && (nRefId < nMaxCount); ++nRefId )
    {
        RefSheetsModel aRefSheets;
        aRefSheets.readBiff12Data( rStrm );
        maRefSheets.push_back( aRefSheets );
    }
}

Sequence< ExternalLinkInfo > ExternalLinkBuffer::getLinkInfos() const
{
    ::std::vector< ExternalLinkInfo > aLinkInfos;
    // add entry for implicit index 0 (self reference to this document)
    aLinkInfos.push_back( mxSelfRef->getLinkInfo() );
    for( ExternalLinkVec::const_iterator aIt = maExtLinks.begin(), aEnd = maExtLinks.end(); aIt != aEnd; ++aIt )
        aLinkInfos.push_back( (*aIt)->getLinkInfo() );
    return ContainerHelper::vectorToSequence( aLinkInfos );
}

ExternalLinkRef ExternalLinkBuffer::getExternalLink( sal_Int32 nRefId, bool bUseRefSheets ) const
{
    ExternalLinkRef xExtLink;
    // OOXML: 0 = this document, otherwise one-based index into link list
    if( !bUseRefSheets || !mbUseRefSheets )
        xExtLink = (nRefId == 0) ? mxSelfRef : maLinks.get( nRefId - 1 );
    // BIFF12: zero-based index into ref-sheets list
    else if( const RefSheetsModel* pRefSheets = getRefSheets( nRefId ) )
        xExtLink = maLinks.get( pRefSheets->mnExtRefId );
    return xExtLink;
}

LinkSheetRange ExternalLinkBuffer::getSheetRange( sal_Int32 nRefId, sal_Int16 nTabId1, sal_Int16 nTabId2 ) const
{
    LinkSheetRange aSheetRange;
    if( const ExternalLink* pExtLink = getExternalLink( nRefId ).get() )
        pExtLink->getSheetRange( aSheetRange, nTabId1, nTabId2 );
    return aSheetRange;
}

LinkSheetRange ExternalLinkBuffer::getSheetRange( sal_Int32 nRefId ) const
{
    OSL_ENSURE( mbUseRefSheets, "ExternalLinkBuffer::getSheetRange - wrong BIFF version" );
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
        &maRefSheets[ static_cast< size_t >( nRefId ) ] : nullptr;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
