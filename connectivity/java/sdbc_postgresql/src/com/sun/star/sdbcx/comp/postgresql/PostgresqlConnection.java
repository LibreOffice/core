/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package com.sun.star.sdbcx.comp.postgresql;

import java.util.HashSet;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.UnoRuntime;

public class PostgresqlConnection extends ComponentBase implements XConnection, XWarningsSupplier, XEventListener {
    private XConnection impl;
    private XComponent implComponent;
    private XWarningsSupplier implWarningsSupplier;
    private String url;
    private HashSet<XComponent> statements = new HashSet<>();

    public PostgresqlConnection(XConnection impl, String url) {
        this.impl = impl;
        implComponent = UnoRuntime.queryInterface(XComponent.class, impl);
        implWarningsSupplier = UnoRuntime.queryInterface(XWarningsSupplier.class, impl);
        this.url = url;
    }

    // XComponent:

    @Override
    protected synchronized void postDisposing() {
        implComponent.dispose();
        for (XComponent pgStatement : statements) {
            try {
                pgStatement.dispose();
            } catch (DisposedException disposedException) {
            }
        }
    };

    // XEventListener:

    public synchronized void disposing(EventObject source) {
        statements.remove(source.Source);
    }

    // XWarningsSupplier:

    public synchronized void clearWarnings() throws SQLException {
        checkDisposed();
        implWarningsSupplier.clearWarnings();
    }

    public synchronized Object getWarnings() throws SQLException {
        checkDisposed();
        return implWarningsSupplier.getWarnings();
    }

    // XConnection:

    public void close() throws SQLException {
        dispose();
    }

    public synchronized void commit() throws SQLException {
        checkDisposed();
        impl.commit();
    }

    public synchronized XStatement createStatement() throws SQLException {
        checkDisposed();
        PostgresqlStatement pgStatement = new PostgresqlStatement(impl.createStatement(), this);
        statements.add(pgStatement);
        pgStatement.addEventListener(this);
        return pgStatement;
    }

    public synchronized boolean getAutoCommit() throws SQLException {
        checkDisposed();
        return impl.getAutoCommit();
    }

    public synchronized String getCatalog() throws SQLException {
        checkDisposed();
        return impl.getCatalog();
    }

    public synchronized XDatabaseMetaData getMetaData() throws SQLException {
        checkDisposed();
        return new PostgresqlDatabaseMetadata(impl.getMetaData(), this, url);
    }

    public synchronized int getTransactionIsolation() throws SQLException {
        checkDisposed();
        return impl.getTransactionIsolation();
    }

    public synchronized XNameAccess getTypeMap() throws SQLException {
        checkDisposed();
        return impl.getTypeMap();
    }

    public synchronized boolean isClosed() throws SQLException {
        checkDisposed();
        return impl.isClosed();
    }

    public synchronized boolean isReadOnly() throws SQLException {
        checkDisposed();
        return impl.isReadOnly();
    }

    public synchronized String nativeSQL(String arg0) throws SQLException {
        checkDisposed();
        return impl.nativeSQL(arg0);
    }

    public synchronized XPreparedStatement prepareCall(String arg0) throws SQLException {
        checkDisposed();
        PostgresqlPreparedStatement pgStatement = new PostgresqlPreparedStatement(impl.prepareCall(arg0), this);
        statements.add(pgStatement);
        pgStatement.addEventListener(this);
        return pgStatement;
    }

    public synchronized XPreparedStatement prepareStatement(String arg0) throws SQLException {
        checkDisposed();
        PostgresqlPreparedStatement pgStatement = new PostgresqlPreparedStatement(impl.prepareStatement(arg0), this);
        statements.add(pgStatement);
        pgStatement.addEventListener(this);
        return pgStatement;
    }

    public synchronized void rollback() throws SQLException {
        checkDisposed();
        impl.rollback();
    }

    public synchronized void setAutoCommit(boolean arg0) throws SQLException {
        checkDisposed();
        impl.setAutoCommit(arg0);
    }

    public synchronized void setCatalog(String arg0) throws SQLException {
        checkDisposed();
        impl.setCatalog(arg0);
    }

    public synchronized void setReadOnly(boolean arg0) throws SQLException {
        checkDisposed();
        impl.setReadOnly(arg0);
    }

    public synchronized void setTransactionIsolation(int arg0) throws SQLException {
        checkDisposed();
        impl.setTransactionIsolation(arg0);
    }

    public synchronized void setTypeMap(XNameAccess arg0) throws SQLException {
        checkDisposed();
        impl.setTypeMap(arg0);
    }
}
