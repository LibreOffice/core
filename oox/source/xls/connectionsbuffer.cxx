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

#include "oox/xls/connectionsbuffer.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_Int32 BIFF12_RECONNECT_AS_REQUIRED            = 1;
const sal_Int32 BIFF12_RECONNECT_ALWAYS                 = 2;
const sal_Int32 BIFF12_RECONNECT_NEVER                  = 3;

const sal_uInt8 BIFF12_CONNECTION_SAVEPASSWORD_ON       = 1;
const sal_uInt8 BIFF12_CONNECTION_SAVEPASSWORD_OFF      = 2;

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

const sal_uInt16 BIFF_DBQUERY_ODBC                      = 0x0008;
const sal_uInt16 BIFF_DBQUERY_SQLQUERY                  = 0x0010;
const sal_uInt16 BIFF_DBQUERY_SERVERBASEDSQL            = 0x0020;
const sal_uInt16 BIFF_DBQUERY_HTML                      = 0x0040;
const sal_uInt16 BIFF_DBQUERY_SAVEPASSWORD              = 0x0080;
const sal_uInt16 BIFF_DBQUERY_HTMLTABLES                = 0x0100;

const sal_uInt16 BIFF_QTSETTINGS_KEEPALIVE              = 0x0001;
const sal_uInt16 BIFF_QTSETTINGS_NEW                    = 0x0002;
const sal_uInt16 BIFF_QTSETTINGS_SOURCEDATA             = 0x0004;
const sal_uInt16 BIFF_QTSETTINGS_WEBBASEDPROV           = 0x0008;
const sal_uInt16 BIFF_QTSETTINGS_REINITLIST             = 0x0010;
const sal_uInt16 BIFF_QTSETTINGS_XML                    = 0x0080;

const sal_uInt16 BIFF_QTSETTINGS_PARSEPRE               = 0x0001;
const sal_uInt16 BIFF_QTSETTINGS_CONSECUTIVE            = 0x0002;
const sal_uInt16 BIFF_QTSETTINGS_FIRSTROW               = 0x0004;
const sal_uInt16 BIFF_QTSETTINGS_XL97CREATED            = 0x0008;
const sal_uInt16 BIFF_QTSETTINGS_TEXTDATES              = 0x0010;
const sal_uInt16 BIFF_QTSETTINGS_XL2000REFRESHED        = 0x0020;

const sal_uInt16 BIFF_QTSETTINGS_TEXTQUERY              = 0x0001;
const sal_uInt16 BIFF_QTSETTINGS_TABLENAMES             = 0x0002;

// ----------------------------------------------------------------------------

OUString lclReadQueryString( BiffInputStream& rStrm, sal_uInt16 nCount )
{
    bool bValidRec = true;
    OUStringBuffer aBuffer;
    for( sal_uInt16 nIndex = 0; bValidRec && (nIndex < nCount); ++nIndex )
    {
        bValidRec = (rStrm.getNextRecId() == BIFF_ID_PCITEM_STRING) && rStrm.startNextRecord();
        if( bValidRec )
            aBuffer.append( rStrm.readUniString() );
    }
    OSL_ENSURE( bValidRec, "lclReadQueryString - missing PCITEM_STRING records" );
    return aBuffer.makeStringAndClear();
}

void lclParseTables( WebPrModel::TablesVector& rTables, const OUString& rTableNames )
{
    rTables.clear();
    OUString aTableNames = rTableNames.trim();
    while( aTableNames.getLength() > 0 )
    {
        sal_Int32 nSep = -1;
        // table names are enclosed in double quotes
        if( aTableNames[ 0 ] == '"' )
        {
            // search closing quote character
            sal_Int32 nEndQuote = aTableNames.indexOf( '"', 1 );
            OSL_ENSURE( nEndQuote >= 1, "lclParseTables - invalid syntax" );
            if( nEndQuote < 0 )
                nEndQuote = aTableNames.getLength();
            else
                nSep = aTableNames.indexOf( ',', nEndQuote + 1 );
            // extract text between quote characters
            OUString aTableName = aTableNames.copy( 1, nEndQuote - 1 ).trim();
            if( aTableName.getLength() > 0 )
                rTables.push_back( Any( aTableName ) );
            else
                rTables.push_back( Any() );
        }
        else
        {
            nSep = aTableNames.indexOf( ',' );
            if( nSep < 0 )
                nSep = aTableNames.getLength();
            OUString aTableIndex = aTableNames.copy( 0, nSep ).trim();
            if( (aTableIndex.getLength() > 0) && (aTableIndex[ 0 ] >= '1') && (aTableIndex[ 0 ] <= '9') )
                rTables.push_back( Any( aTableIndex.toInt32() ) );
            else
                rTables.push_back( Any() );
        }

        // remove processed item from aTableNames
        if( (nSep < 0) || (nSep >= aTableNames.getLength()) )
            aTableNames = OUString();
        else
            aTableNames = aTableNames.copy( nSep + 1 ).trim();
    }
}

} // namespace

// ============================================================================

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

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

Connection::Connection( const WorkbookHelper& rHelper, sal_Int32 nConnId ) :
    WorkbookHelper( rHelper )
{
    maModel.mnId = nConnId;
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

void Connection::importTables( const AttributeList& /*rAttribs*/ )
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
    rStrm >> nSavePassword;
    rStrm.skip( 1 );
    maModel.mnInterval = rStrm.readuInt16();
    rStrm >> nFlags >> nStrFlags >> maModel.mnType >> maModel.mnReconnectMethod >> maModel.mnId >> nCredentials;

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
    rStrm >> nFlags >> nStrFlags;

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

void Connection::importDbQuery( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags, nSqlParamCount, nCommandCount, nPostMethodCount, nServerSqlCount, nOdbcConnCount;
    rStrm >> nFlags >> nSqlParamCount >> nCommandCount >> nPostMethodCount >> nServerSqlCount >> nOdbcConnCount;

    // same type constants in all BIFF versions
    maModel.mnType = extractValue< sal_Int32 >( nFlags, 0, 3 );
    maModel.mbSavePassword = getFlag( nFlags, BIFF_DBQUERY_SAVEPASSWORD );

    OSL_ENSURE( getFlag( nFlags, BIFF_DBQUERY_ODBC ) == (maModel.mnType == BIFF12_CONNECTION_ODBC), "Connection::importDbQuery - wrong ODBC flag" );
    OSL_ENSURE( getFlag( nFlags, BIFF_DBQUERY_SQLQUERY ) != (maModel.mnType == BIFF12_CONNECTION_HTML), "Connection::importDbQuery - wrong SQL query flag" );
    OSL_ENSURE( getFlag( nFlags, BIFF_DBQUERY_HTML ) == (maModel.mnType == BIFF12_CONNECTION_HTML), "Connection::importDbQuery - wrong HTML flag" );

    if( (maModel.mnType == BIFF12_CONNECTION_HTML) && getFlag( nFlags, BIFF_DBQUERY_HTML ) )
    {
        WebPrModel& rWebPr = maModel.createWebPr();
        rWebPr.mbHtmlTables = getFlag( nFlags, BIFF_DBQUERY_HTMLTABLES );

        // read HTML query URL and post method
        rWebPr.maUrl = lclReadQueryString( rStrm, nCommandCount );
        rWebPr.maPostMethod = lclReadQueryString( rStrm, nPostMethodCount );
    }
}

void Connection::importQueryTableSettings( BiffInputStream& rStrm )
{
    rStrm.skip( 4 );
    // source data type, again
    sal_uInt16 nType = rStrm.readuInt16();
    OSL_ENSURE( nType == maModel.mnType, "Connection::importQueryTableSettings - source data type mismatch" );
    if( nType == maModel.mnType )
    {
        sal_uInt16 nFlags1, nFlags2, nFlags3, nHtmlFormat;
        rStrm >> nFlags1 >> nFlags2 >> nFlags3;
        rStrm.skip( 10 );
        maModel.mnInterval = rStrm.readuInt16();
        rStrm >> nHtmlFormat;

        // first flags field: generic connection flags
        maModel.mbKeepAlive = getFlag( nFlags1, BIFF_QTSETTINGS_KEEPALIVE );
        maModel.mbNew       = getFlag( nFlags1, BIFF_QTSETTINGS_NEW );

        // meaning of second flags field is dependent on source data type
        if( (maModel.mnType == BIFF12_CONNECTION_HTML) && maModel.mxWebPr.get() )
        {
            WebPrModel& rWebPr = *maModel.mxWebPr;

            // HTML format is one-based in BIFF8 (but zero-based in BIFF12)
            static const sal_Int32 spnHmlFormats[] = { XML_none, XML_none, XML_rtf, XML_all };
            rWebPr.mnHtmlFormat = STATIC_ARRAY_SELECT( spnHmlFormats, nHtmlFormat, XML_none );

            rWebPr.mbXml             = getFlag( nFlags1, BIFF_QTSETTINGS_XML );
            rWebPr.mbSourceData      = getFlag( nFlags1, BIFF_QTSETTINGS_SOURCEDATA );
            rWebPr.mbParsePre        = getFlag( nFlags2, BIFF_QTSETTINGS_PARSEPRE );
            rWebPr.mbConsecutive     = getFlag( nFlags2, BIFF_QTSETTINGS_CONSECUTIVE );
            rWebPr.mbFirstRow        = getFlag( nFlags2, BIFF_QTSETTINGS_FIRSTROW );
            rWebPr.mbXl97Created     = getFlag( nFlags2, BIFF_QTSETTINGS_XL97CREATED );
            rWebPr.mbTextDates       = getFlag( nFlags2, BIFF_QTSETTINGS_TEXTDATES );
            rWebPr.mbXl2000Refreshed = getFlag( nFlags2, BIFF_QTSETTINGS_XL2000REFRESHED );

            // list of HTML table names or indexes
            if( getFlag( nFlags3, BIFF_QTSETTINGS_TABLENAMES ) )
            {
                // a QUERYTABLESTRING record containing the table names must follow
                bool bHasQTString = (rStrm.getNextRecId() == BIFF_ID_QUERYTABLESTRING) && rStrm.startNextRecord();
                OSL_ENSURE( bHasQTString, "Connection::importQueryTableSettings - missing QUERYTABLESTRING record" );
                if( bHasQTString )
                {
                    rStrm.skip( 4 );
                    lclParseTables( rWebPr.maTables, rStrm.readUniString() );
                }
            }
        }
    }
}

// ============================================================================

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

Connection& ConnectionsBuffer::createConnectionWithId()
{
    ConnectionRef xConnection( new Connection( *this, mnUnusedId ) );
    maConnections.push_back( xConnection );
    insertConnectionToMap( xConnection );
    return *xConnection;
}

void ConnectionsBuffer::finalizeImport()
{
    for( ConnectionVector::iterator aIt = maConnections.begin(), aEnd = maConnections.end(); aIt != aEnd; ++aIt )
        insertConnectionToMap( *aIt );
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

// ============================================================================

} // namespace xls
} // namespace oox
