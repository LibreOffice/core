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

#include <connectionsbuffer.hxx>
#include <biffhelper.hxx>

#include <osl/diagnose.h>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/helper/binaryinputstream.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::uno;

namespace {

const sal_Int32 BIFF12_RECONNECT_AS_REQUIRED            = 1;

const sal_uInt8 BIFF12_CONNECTION_SAVEPASSWORD_ON       = 1;

const sal_uInt16 BIFF12_CONNECTION_KEEPALIVE            = 0x0001;
const sal_uInt16 BIFF12_CONNECTION_NEW                  = 0x0002;
const sal_uInt16 BIFF12_CONNECTION_DELETED              = 0x0004;
const sal_uInt16 BIFF12_CONNECTION_ONLYUSECONNFILE      = 0x0008;
const sal_uInt16 BIFF12_CONNECTION_BACKGROUND           = 0x0010;
const sal_uInt16 BIFF12_CONNECTION_REFRESHONLOAD        = 0x0020;
const sal_uInt16 BIFF12_CONNECTION_SAVEDATA             = 0x0040;

const sal_uInt16 BIFF12_CONNECTION_HAS_SOURCEFILE       = 0x0001;
const sal_uInt16 BIFF12_CONNECTION_HAS_SOURCECONNFILE   = 0x0002;
const sal_uInt16 BIFF12_CONNECTION_HAS_DESCRIPTION      = 0x0004;
const sal_uInt16 BIFF12_CONNECTION_HAS_NAME             = 0x0008;
const sal_uInt16 BIFF12_CONNECTION_HAS_SSOID            = 0x0010;

const sal_uInt32 BIFF12_WEBPR_XML                       = 0x00000100;
const sal_uInt32 BIFF12_WEBPR_SOURCEDATA                = 0x00000200;
const sal_uInt32 BIFF12_WEBPR_PARSEPRE                  = 0x00000400;
const sal_uInt32 BIFF12_WEBPR_CONSECUTIVE               = 0x00000800;
const sal_uInt32 BIFF12_WEBPR_FIRSTROW                  = 0x00001000;
const sal_uInt32 BIFF12_WEBPR_XL97CREATED               = 0x00002000;
const sal_uInt32 BIFF12_WEBPR_TEXTDATES                 = 0x00004000;
const sal_uInt32 BIFF12_WEBPR_XL2000REFRESHED           = 0x00008000;
const sal_uInt32 BIFF12_WEBPR_HTMLTABLES                = 0x00010000;

const sal_uInt8 BIFF12_WEBPR_HAS_POSTMETHOD             = 0x01;
const sal_uInt8 BIFF12_WEBPR_HAS_EDITPAGE               = 0x02;
const sal_uInt8 BIFF12_WEBPR_HAS_URL                    = 0x04;

} // namespace

WebPrModel::WebPrModel() :
    mnHtmlFormat( XML_none ),
    mbXml( false ),
    mbSourceData( false ),
    mbParsePre( false ),
    mbConsecutive( false ),
    mbFirstRow( false ),
    mbXl97Created( false ),
    mbTextDates( false ),
    mbXl2000Refreshed( false ),
    mbHtmlTables( false )
{
}

ConnectionModel::ConnectionModel() :
    mnId( -1 ),
    mnType( BIFF12_CONNECTION_UNKNOWN ),
    mnReconnectMethod( BIFF12_RECONNECT_AS_REQUIRED ),
    mnCredentials( XML_integrated ),
    mnInterval( 0 ),
    mbKeepAlive( false ),
    mbNew( false ),
    mbDeleted( false ),
    mbOnlyUseConnFile( false ),
    mbBackground( false ),
    mbRefreshOnLoad( false ),
    mbSaveData( false ),
    mbSavePassword( false )
{
}

WebPrModel& ConnectionModel::createWebPr()
{
    OSL_ENSURE( !mxWebPr.get(), "ConnectionModel::createWebPr - multiple call" );
    mxWebPr.reset( new WebPrModel );
    return *mxWebPr;
}

Connection::Connection( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
    maModel.mnId = -1;
}

void Connection::importConnection( const AttributeList& rAttribs )
{
    maModel.maName            = rAttribs.getXString( XML_name, OUString() );
    maModel.maDescription     = rAttribs.getXString( XML_description, OUString() );
    maModel.maSourceFile      = rAttribs.getXString( XML_sourceFile, OUString() );
    maModel.maSourceConnFile  = rAttribs.getXString( XML_odcFile, OUString() );
    maModel.maSsoId           = rAttribs.getXString( XML_singleSignOnId, OUString() );
    maModel.mnId              = rAttribs.getInteger( XML_id, -1 );
    // type and reconnectionMethod are using the BIFF12 constants instead of XML tokens
    maModel.mnType            = rAttribs.getInteger( XML_type, BIFF12_CONNECTION_UNKNOWN );
    maModel.mnReconnectMethod = rAttribs.getInteger( XML_reconnectionMethod, BIFF12_RECONNECT_AS_REQUIRED );
    maModel.mnCredentials     = rAttribs.getToken( XML_credentials, XML_integrated );
    maModel.mnInterval        = rAttribs.getInteger( XML_interval, 0 );
    maModel.mbKeepAlive       = rAttribs.getBool( XML_keepAlive, false );
    maModel.mbNew             = rAttribs.getBool( XML_new, false );
    maModel.mbDeleted         = rAttribs.getBool( XML_deleted, false );
    maModel.mbOnlyUseConnFile = rAttribs.getBool( XML_onlyUseConnectionFile, false );
    maModel.mbBackground      = rAttribs.getBool( XML_background, false );
    maModel.mbRefreshOnLoad   = rAttribs.getBool( XML_refreshOnLoad, false );
    maModel.mbSaveData        = rAttribs.getBool( XML_saveData, false );
    maModel.mbSavePassword    = rAttribs.getBool( XML_savePassword, false );
}

void Connection::importWebPr( const AttributeList& rAttribs )
{
    WebPrModel& rWebPr = maModel.createWebPr();

    rWebPr.maUrl             = rAttribs.getXString( XML_url, OUString() );
    rWebPr.maPostMethod      = rAttribs.getXString( XML_post, OUString() );
    rWebPr.maEditPage        = rAttribs.getXString( XML_editPage, OUString() );
    rWebPr.mnHtmlFormat      = rAttribs.getToken( XML_htmlFormat, XML_none );
    rWebPr.mbXml             = rAttribs.getBool( XML_xml, false );
    rWebPr.mbSourceData      = rAttribs.getBool( XML_sourceData, false );
    rWebPr.mbParsePre        = rAttribs.getBool( XML_parsePre, false );
    rWebPr.mbConsecutive     = rAttribs.getBool( XML_consecutive, false );
    rWebPr.mbFirstRow        = rAttribs.getBool( XML_firstRow, false );
    rWebPr.mbXl97Created     = rAttribs.getBool( XML_xl97, false );
    rWebPr.mbTextDates       = rAttribs.getBool( XML_textDates, false );
    rWebPr.mbXl2000Refreshed = rAttribs.getBool( XML_xl2000, false );
    rWebPr.mbHtmlTables      = rAttribs.getBool( XML_htmlTables, false );
}

void Connection::importTables()
{
    if( maModel.mxWebPr.get() )
    {
        OSL_ENSURE( maModel.mxWebPr->maTables.empty(), "Connection::importTables - multiple calls" );
        maModel.mxWebPr->maTables.clear();
    }
}

void Connection::importTable( const AttributeList& rAttribs, sal_Int32 nElement )
{
    if( maModel.mxWebPr.get() )
    {
        Any aTableAny;
        switch( nElement )
        {
            case XLS_TOKEN( m ):                                                            break;
            case XLS_TOKEN( s ):    aTableAny <<= rAttribs.getXString( XML_v, OUString() ); break;
            case XLS_TOKEN( x ):    aTableAny <<= rAttribs.getInteger( XML_v, -1 );         break;
            default:
                OSL_ENSURE( false, "Connection::importTable - unexpected element" );
                return;
        }
        maModel.mxWebPr->maTables.push_back( aTableAny );
    }
}

void Connection::importConnection( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags, nStrFlags;
    sal_uInt8 nSavePassword, nCredentials;
    rStrm.skip( 2 );
    nSavePassword = rStrm.readuChar();
    rStrm.skip( 1 );
    maModel.mnInterval = rStrm.readuInt16();
    nFlags = rStrm.readuInt16();
    nStrFlags = rStrm.readuInt16();
    maModel.mnType = rStrm.readInt32();
    maModel.mnReconnectMethod = rStrm.readInt32();
    maModel.mnId = rStrm.readInt32();
    nCredentials = rStrm.readuChar();

    if( getFlag( nStrFlags, BIFF12_CONNECTION_HAS_SOURCEFILE ) )
        rStrm >> maModel.maSourceFile;
    if( getFlag( nStrFlags, BIFF12_CONNECTION_HAS_SOURCECONNFILE ) )
        rStrm >> maModel.maSourceConnFile;
    if( getFlag( nStrFlags, BIFF12_CONNECTION_HAS_DESCRIPTION ) )
        rStrm >> maModel.maDescription;
    if( getFlag( nStrFlags, BIFF12_CONNECTION_HAS_NAME ) )
        rStrm >> maModel.maName;
    if( getFlag( nStrFlags, BIFF12_CONNECTION_HAS_SSOID ) )
        rStrm >> maModel.maSsoId;

    static const sal_Int32 spnCredentials[] = { XML_integrated, XML_none, XML_stored, XML_prompt };
    maModel.mnCredentials = STATIC_ARRAY_SELECT( spnCredentials, nCredentials, XML_integrated );

    maModel.mbKeepAlive       = getFlag( nFlags, BIFF12_CONNECTION_KEEPALIVE );
    maModel.mbNew             = getFlag( nFlags, BIFF12_CONNECTION_NEW );
    maModel.mbDeleted         = getFlag( nFlags, BIFF12_CONNECTION_DELETED );
    maModel.mbOnlyUseConnFile = getFlag( nFlags, BIFF12_CONNECTION_ONLYUSECONNFILE );
    maModel.mbBackground      = getFlag( nFlags, BIFF12_CONNECTION_BACKGROUND );
    maModel.mbRefreshOnLoad   = getFlag( nFlags, BIFF12_CONNECTION_REFRESHONLOAD );
    maModel.mbSaveData        = getFlag( nFlags, BIFF12_CONNECTION_SAVEDATA );
    maModel.mbSavePassword    = nSavePassword == BIFF12_CONNECTION_SAVEPASSWORD_ON;
}

void Connection::importWebPr( SequenceInputStream& rStrm )
{
    WebPrModel& rWebPr = maModel.createWebPr();

    sal_uInt32 nFlags;
    sal_uInt8 nStrFlags;
    nFlags = rStrm.readuInt32();
    nStrFlags = rStrm.readuChar();

    if( getFlag( nStrFlags, BIFF12_WEBPR_HAS_URL ) )
        rStrm >> rWebPr.maUrl;
    if( getFlag( nStrFlags, BIFF12_WEBPR_HAS_POSTMETHOD ) )
        rStrm >> rWebPr.maPostMethod;
    if( getFlag( nStrFlags, BIFF12_WEBPR_HAS_EDITPAGE ) )
        rStrm >> rWebPr.maEditPage;

    static const sal_Int32 spnHmlFormats[] = { XML_none, XML_rtf, XML_all };
    rWebPr.mnHtmlFormat = STATIC_ARRAY_SELECT( spnHmlFormats, extractValue< sal_uInt8 >( nFlags, 0, 8 ), XML_none );

    rWebPr.mbXml             = getFlag( nFlags, BIFF12_WEBPR_XML );
    rWebPr.mbSourceData      = getFlag( nFlags, BIFF12_WEBPR_SOURCEDATA );
    rWebPr.mbParsePre        = getFlag( nFlags, BIFF12_WEBPR_PARSEPRE );
    rWebPr.mbConsecutive     = getFlag( nFlags, BIFF12_WEBPR_CONSECUTIVE );
    rWebPr.mbFirstRow        = getFlag( nFlags, BIFF12_WEBPR_FIRSTROW );
    rWebPr.mbXl97Created     = getFlag( nFlags, BIFF12_WEBPR_XL97CREATED );
    rWebPr.mbTextDates       = getFlag( nFlags, BIFF12_WEBPR_TEXTDATES );
    rWebPr.mbXl2000Refreshed = getFlag( nFlags, BIFF12_WEBPR_XL2000REFRESHED );
    rWebPr.mbHtmlTables      = getFlag( nFlags, BIFF12_WEBPR_HTMLTABLES );
}

void Connection::importWebPrTables( SequenceInputStream& /*rStrm*/ )
{
    if( maModel.mxWebPr.get() )
    {
        OSL_ENSURE( maModel.mxWebPr->maTables.empty(), "Connection::importWebPrTables - multiple calls" );
        maModel.mxWebPr->maTables.clear();
    }
}

void Connection::importWebPrTable( SequenceInputStream& rStrm, sal_Int32 nRecId )
{
    if( maModel.mxWebPr.get() )
    {
        Any aTableAny;
        switch( nRecId )
        {
            case BIFF12_ID_PCITEM_MISSING:                                                  break;
            case BIFF12_ID_PCITEM_STRING:   aTableAny <<= BiffHelper::readString( rStrm );  break;
            case BIFF12_ID_PCITEM_INDEX:    aTableAny <<= rStrm.readInt32();                break;
            default:
                OSL_ENSURE( false, "Connection::importWebPrTable - unexpected record" );
                return;
        }
        maModel.mxWebPr->maTables.push_back( aTableAny );
    }
}

ConnectionsBuffer::ConnectionsBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnUnusedId( 1 )
{
}

Connection& ConnectionsBuffer::createConnection()
{
    ConnectionRef xConnection( new Connection( *this ) );
    maConnections.push_back( xConnection );
    return *xConnection;
}

void ConnectionsBuffer::finalizeImport()
{
    for( const auto& rxConnection : maConnections )
        insertConnectionToMap( rxConnection );
}

ConnectionRef ConnectionsBuffer::getConnection( sal_Int32 nConnId ) const
{
    return maConnectionsById.get( nConnId );
}

void ConnectionsBuffer::insertConnectionToMap( const ConnectionRef& rxConnection )
{
    sal_Int32 nConnId = rxConnection->getConnectionId();
    if( nConnId > 0 )
    {
        OSL_ENSURE( !maConnectionsById.has( nConnId ), "ConnectionsBuffer::insertConnectionToMap - multiple connection identifier" );
        maConnectionsById[ nConnId ] = rxConnection;
        mnUnusedId = ::std::max< sal_Int32 >( mnUnusedId, nConnId + 1 );
    }
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
