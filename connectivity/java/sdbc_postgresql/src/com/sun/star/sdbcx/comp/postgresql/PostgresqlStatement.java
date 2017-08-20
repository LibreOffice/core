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

import java.util.concurrent.atomic.AtomicBoolean;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XMultipleResults;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCancellable;

public class PostgresqlStatement extends ComponentBase
        implements XCloseable, XPropertySet, XCancellable, XStatement, XWarningsSupplier, XMultipleResults {

    private XStatement impl;
    private XCloseable implCloseable;
    private XPropertySet implPropertySet;
    private XCancellable implCancellable;
    private XWarningsSupplier implWarningsSupplier;
    private XMultipleResults implMultipleResults;
    private XConnection connection;
    private AtomicBoolean isDisposed = new AtomicBoolean(false);

    public PostgresqlStatement(XStatement impl, XConnection connection) {
        this.impl = impl;
        this.implCloseable = UnoRuntime.queryInterface(XCloseable.class, impl);
        this.implPropertySet = UnoRuntime.queryInterface(XPropertySet.class, impl);
        this.implCancellable = UnoRuntime.queryInterface(XCancellable.class, impl);
        this.implWarningsSupplier = UnoRuntime.queryInterface(XWarningsSupplier.class, impl);
        this.implMultipleResults = UnoRuntime.queryInterface(XMultipleResults.class, impl);
        this.connection = connection;
    }

    // XComponentBase:

    @Override
    protected void postDisposing() {
        isDisposed.set(true);
        try {
            implCloseable.close();
        } catch (SQLException sqlException) {
        }
    }

    private void checkDisposed() throws DisposedException {
        if (isDisposed.get()) {
            throw new DisposedException();
        }
    }

    // XStatement:

    public boolean execute(String arg0) throws SQLException {
        checkDisposed();
        System.out.println(arg0);
        return impl.execute(arg0);
    }

    public XResultSet executeQuery(String arg0) throws SQLException {
        checkDisposed();
        XResultSet results = impl.executeQuery(arg0);
        return new PostgresqlResultSet(results, this);
    }

    public int executeUpdate(String arg0) throws SQLException {
        checkDisposed();
        return impl.executeUpdate(arg0);
    }

    public XConnection getConnection() throws SQLException {
        checkDisposed();
        return connection;
    }

    // XCloseable:

    public void close() throws SQLException {
        dispose();
    }

    // XPropertySet:

    public void addPropertyChangeListener(String arg0, XPropertyChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        checkDisposed();
        implPropertySet.addPropertyChangeListener(arg0, arg1);
    }

    public void addVetoableChangeListener(String arg0, XVetoableChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        checkDisposed();
        implPropertySet.addVetoableChangeListener(arg0, arg1);
    }

    public XPropertySetInfo getPropertySetInfo() {
        checkDisposed();
        return implPropertySet.getPropertySetInfo();
    }

    public Object getPropertyValue(String arg0) throws UnknownPropertyException, WrappedTargetException {
        checkDisposed();
        return implPropertySet.getPropertyValue(arg0);
    }

    public void removePropertyChangeListener(String arg0, XPropertyChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        checkDisposed();
        implPropertySet.removePropertyChangeListener(arg0, arg1);
    }

    public void removeVetoableChangeListener(String arg0, XVetoableChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        checkDisposed();
        implPropertySet.removeVetoableChangeListener(arg0, arg1);
    }

    public void setPropertyValue(String arg0, Object arg1)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        checkDisposed();
        implPropertySet.setPropertyValue(arg0, arg1);
    }

    // XCancellable:

    public void cancel() {
        checkDisposed();
        implCancellable.cancel();
    }

    // XWarningsSupplier:

    public void clearWarnings() throws SQLException {
        checkDisposed();
        implWarningsSupplier.clearWarnings();
    }

    public Object getWarnings() throws SQLException {
        checkDisposed();
        return implWarningsSupplier.getWarnings();
    }

    // XMultipleResults:

    public boolean getMoreResults() throws SQLException {
        checkDisposed();
        return implMultipleResults.getMoreResults();
    }

    public XResultSet getResultSet() throws SQLException {
        checkDisposed();
        return new PostgresqlResultSet(implMultipleResults.getResultSet(), this);
    }

    public int getUpdateCount() throws SQLException {
        checkDisposed();
        return implMultipleResults.getUpdateCount();
    }
}
