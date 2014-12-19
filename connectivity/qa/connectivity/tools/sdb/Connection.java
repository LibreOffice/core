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

package connectivity.tools.sdb;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;

/**
 * is a convenience wrapper around a SDB-level connection object
 */
public class Connection
{
    private final   XConnection m_connection;

    public Connection( final XConnection _connection )
    {
        m_connection = _connection;
    }

    public XConnection  getXConnection()
    {
        return m_connection;
    }

    public void refreshTables()
    {
        final XTablesSupplier suppTables = UnoRuntime.queryInterface(XTablesSupplier.class, m_connection);
        final XRefreshable refresh = UnoRuntime.queryInterface( XRefreshable.class, suppTables.getTables() );
        refresh.refresh();
    }

    public XSingleSelectQueryComposer createSingleSelectQueryComposer() throws Exception
    {
        final XMultiServiceFactory connectionFactory = UnoRuntime.queryInterface( XMultiServiceFactory.class, m_connection );
        return UnoRuntime.queryInterface(
            XSingleSelectQueryComposer.class, connectionFactory.createInstance( "com.sun.star.sdb.SingleSelectQueryComposer" ) );
    }

    public
    XStatement createStatement() throws SQLException
    {
        return m_connection.createStatement();
    }

    public
    XPreparedStatement prepareStatement( String _sql ) throws SQLException
    {
        return m_connection.prepareStatement( _sql );
    }

    public
    void close() throws SQLException
    {
        m_connection.close();
    }
}
