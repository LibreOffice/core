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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_CONNECTIONSBUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_CONNECTIONSBUFFER_HXX

#include <oox/helper/refvector.hxx>
#include "workbookhelper.hxx"

namespace oox {
namespace xls {

const sal_Int32 BIFF12_CONNECTION_UNKNOWN               = 0;
const sal_Int32 BIFF12_CONNECTION_ODBC                  = 1;
const sal_Int32 BIFF12_CONNECTION_DAO                   = 2;
const sal_Int32 BIFF12_CONNECTION_FILE                  = 3;
const sal_Int32 BIFF12_CONNECTION_HTML                  = 4;
const sal_Int32 BIFF12_CONNECTION_OLEDB                 = 5;
const sal_Int32 BIFF12_CONNECTION_TEXT                  = 6;
const sal_Int32 BIFF12_CONNECTION_ADO                   = 7;
const sal_Int32 BIFF12_CONNECTION_DSP                   = 8;

/** Special properties for data connections representing web queries. */
struct WebPrModel
{
    typedef ::std::vector< css::uno::Any > TablesVector;

    TablesVector        maTables;           /// Names or indexes of the web query tables.
    OUString     maUrl;              /// Source URL to refresh the data.
    OUString     maPostMethod;       /// POST method to query data.
    OUString     maEditPage;         /// Web page showing query data (for XML queries).
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

/** Common properties of an external data connection. */
struct ConnectionModel
{
    typedef ::std::unique_ptr< WebPrModel > WebPrModelPtr;

    WebPrModelPtr       mxWebPr;            /// Special settings for web queries.
    OUString     maName;             /// Unique name of this connection.
    OUString     maDescription;      /// User description of this connection.
    OUString     maSourceFile;       /// URL of a source data file.
    OUString     maSourceConnFile;   /// URL of a source connection file.
    OUString     maSsoId;            /// Single sign-on identifier.
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

    /** Returns the unique connection identifier. */
    inline sal_Int32    getConnectionId() const { return maModel.mnId; }
    /** Returns the source data type of the connection. */
    inline sal_Int32    getConnectionType() const { return maModel.mnType; }
    /** Returns read-only access to the connection model data. */
    const ConnectionModel& getModel() const { return maModel; }

private:
    ConnectionModel     maModel;
};

typedef std::shared_ptr< Connection > ConnectionRef;

class ConnectionsBuffer : public WorkbookHelper
{
public:
    explicit            ConnectionsBuffer( const WorkbookHelper& rHelper );

    /** Creates a new empty connection. */
    Connection&         createConnection();

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

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
