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

#ifndef OOX_XLS_CONNECTIONSBUFFER_HXX
#define OOX_XLS_CONNECTIONSBUFFER_HXX

#include "oox/helper/refvector.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

const sal_Int32 BIFF12_CONNECTION_UNKNOWN               = 0;
const sal_Int32 BIFF12_CONNECTION_ODBC                  = 1;
const sal_Int32 BIFF12_CONNECTION_DAO                   = 2;
const sal_Int32 BIFF12_CONNECTION_FILE                  = 3;
const sal_Int32 BIFF12_CONNECTION_HTML                  = 4;
const sal_Int32 BIFF12_CONNECTION_OLEDB                 = 5;
const sal_Int32 BIFF12_CONNECTION_TEXT                  = 6;
const sal_Int32 BIFF12_CONNECTION_ADO                   = 7;
const sal_Int32 BIFF12_CONNECTION_DSP                   = 8;

// ============================================================================

/** Special properties for data connections representing web queries. */
struct WebPrModel
{
    typedef ::std::vector< ::com::sun::star::uno::Any > TablesVector;

    TablesVector        maTables;           /// Names or indexes of the web query tables.
    ::rtl::OUString     maUrl;              /// Source URL to refresh the data.
    ::rtl::OUString     maPostMethod;       /// POST method to query data.
    ::rtl::OUString     maEditPage;         /// Web page showing query data (for XML queries).
    sal_Int32           mnHtmlFormat;       /// Plain text, rich text, or HTML.
    bool                mbXml;              /// True = XML query, false = HTML query.
    bool                mbSourceData;       /// True = import XML source data referred by HTML table.
    bool                mbParsePre;         /// True = parse preformatted sections (<pre> tag).
    bool                mbConsecutive;      /// True = join consecutive delimiters.
    bool                mbFirstRow;         /// True = use column withs of first row for entire <pre> tag.
    bool                mbXl97Created;      /// True = web query created with Excel 97.
    bool                mbTextDates;        /// True = read date values as text, false = parse dates.
    bool                mbXl2000Refreshed;  /// True = refreshed with Excel 2000 or newer.
    bool                mbHtmlTables;       /// True = HTML tables, false = entire document.

    explicit            WebPrModel();
};

// ----------------------------------------------------------------------------

/** Common properties of an external data connection. */
struct ConnectionModel
{
    typedef ::std::auto_ptr< WebPrModel > WebPrModelPtr;

    WebPrModelPtr       mxWebPr;            /// Special settings for web queries.
    ::rtl::OUString     maName;             /// Unique name of this connection.
    ::rtl::OUString     maDescription;      /// User description of this connection.
    ::rtl::OUString     maSourceFile;       /// URL of a source data file.
    ::rtl::OUString     maSourceConnFile;   /// URL of a source connection file.
    ::rtl::OUString     maSsoId;            /// Single sign-on identifier.
    sal_Int32           mnId;               /// Unique connection identifier.
    sal_Int32           mnType;             /// Data source type.
    sal_Int32           mnReconnectMethod;  /// Reconnection method.
    sal_Int32           mnCredentials;      /// Credentials method.
    sal_Int32           mnInterval;         /// Refresh interval in minutes.
    bool                mbKeepAlive;        /// True = keep connection open after import.
    bool                mbNew;              /// True = new connection, never updated.
    bool                mbDeleted;          /// True = connection has been deleted.
    bool                mbOnlyUseConnFile;  /// True = use maSourceConnFile, ignore mnReconnectMethod.
    bool                mbBackground;       /// True = background refresh enabled.
    bool                mbRefreshOnLoad;    /// True = refresh connection on import.
    bool                mbSaveData;         /// True = save cached data with connection.
    bool                mbSavePassword;     /// True = save password in connection string.

    explicit            ConnectionModel();

    WebPrModel&         createWebPr();
};

// ----------------------------------------------------------------------------

/** An external data connection (database, web query, etc.). */
class Connection : public WorkbookHelper
{
public:
    explicit            Connection( const WorkbookHelper& rHelper, sal_Int32 nConnId = -1 );

    /** Imports connection settings from the connection element. */
    void                importConnection( const AttributeList& rAttribs );
    /** Imports web query settings from the webPr element. */
    void                importWebPr( const AttributeList& rAttribs );
    /** Imports web query table settings from the tables element. */
    void                importTables( const AttributeList& rAttribs );
    /** Imports a web query table identifier from the m, s, or x element. */
    void                importTable( const AttributeList& rAttribs, sal_Int32 nElement );

    /** Imports connection settings from the CONNECTION record. */
    void                importConnection( SequenceInputStream& rStrm );
    /** Imports web query settings from the WEBPR record. */
    void                importWebPr( SequenceInputStream& rStrm );
    /** Imports web query table settings from the WEBPRTABLES record. */
    void                importWebPrTables( SequenceInputStream& rStrm );
    /** Imports a web query table identifier from the PCITEM_MISSING, PCITEM_STRING, or PCITEM_INDEX record. */
    void                importWebPrTable( SequenceInputStream& rStrm, sal_Int32 nRecId );

    /** Imports connection settings from the DBQUERY record. */
    void                importDbQuery( BiffInputStream& rStrm );
    /** Imports connection settings from the QUERYTABLESETTINGS record. */
    void                importQueryTableSettings( BiffInputStream& rStrm );

    /** Returns the unique connection identifier. */
    inline sal_Int32    getConnectionId() const { return maModel.mnId; }
    /** Returns the source data type of the connection. */
    inline sal_Int32    getConnectionType() const { return maModel.mnType; }
    /** Returns read-only access to the connection model data. */
    const ConnectionModel& getModel() const { return maModel; }

private:
    ConnectionModel     maModel;
};

typedef ::boost::shared_ptr< Connection > ConnectionRef;

// ============================================================================

class ConnectionsBuffer : public WorkbookHelper
{
public:
    explicit            ConnectionsBuffer( const WorkbookHelper& rHelper );

    /** Creates a new empty connection. */
    Connection&         createConnection();
    /** Creates a new empty connection with a valid but unused identifier. */
    Connection&         createConnectionWithId();

    /** Maps all connections by their identifier. */
    void                finalizeImport();

    /** Returns a data connection by its unique identifier. */
    ConnectionRef       getConnection( sal_Int32 nConnId ) const;

private:
    /** Inserts the passed connection into the map according to its identifier. */
    void                insertConnectionToMap( const ConnectionRef& rxConnection );

private:
    typedef RefVector< Connection >         ConnectionVector;
    typedef RefMap< sal_Int32, Connection > ConnectionMap;

    ConnectionVector    maConnections;
    ConnectionMap       maConnectionsById;
    sal_Int32           mnUnusedId;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
