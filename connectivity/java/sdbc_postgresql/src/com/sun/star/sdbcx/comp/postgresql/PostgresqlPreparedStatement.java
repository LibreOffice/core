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

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XMultipleResults;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XPreparedBatchExecution;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;
import com.sun.star.util.XCancellable;

public class PostgresqlPreparedStatement extends ComponentBase
        implements XPreparedStatement, XCloseable, XPropertySet, XCancellable, XResultSetMetaDataSupplier, XParameters, XPreparedBatchExecution,
            XWarningsSupplier, XMultipleResults {

    private XPreparedStatement impl;
    private XCloseable implCloseable;
    private XPropertySet implPropertySet;
    private XCancellable implCancellable;
    private XResultSetMetaDataSupplier implResultSetMetaDataSupplier;
    private XParameters implParameters;
    private XPreparedBatchExecution implPreparedBatchExecution;
    private XWarningsSupplier implWarningsSupplier;
    private XMultipleResults implMultipleResults;
    private XConnection connection;

    public PostgresqlPreparedStatement(XPreparedStatement impl, XConnection connection) {
        this.impl = impl;
        this.implCloseable = UnoRuntime.queryInterface(XCloseable.class, impl);
        this.implPropertySet = UnoRuntime.queryInterface(XPropertySet.class, impl);
        this.implCancellable = UnoRuntime.queryInterface(XCancellable.class, impl);
        this.implResultSetMetaDataSupplier = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, impl);
        this.implParameters = UnoRuntime.queryInterface(XParameters.class, impl);
        this.implPreparedBatchExecution = UnoRuntime.queryInterface(XPreparedBatchExecution.class, impl);
        this.implWarningsSupplier = UnoRuntime.queryInterface(XWarningsSupplier.class, impl);
        this.implMultipleResults = UnoRuntime.queryInterface(XMultipleResults.class, impl);
        this.connection = connection;
    }

    // XComponentBase:

    @Override
    protected void postDisposing() {
        try {
            implCloseable.close();
        } catch (SQLException sqlException) {
        }
    }

    // XPreparedStatement:

    public boolean execute() throws SQLException {
        return impl.execute();
    }

    public XResultSet executeQuery() throws SQLException {
        return new PostgresqlResultSet(impl.executeQuery(), this);
    }

    public int executeUpdate() throws SQLException {
        return impl.executeUpdate();
    }

    public XConnection getConnection() throws SQLException {
        return connection;
    }

    // XCloseable:

    public void close() throws SQLException {
        dispose();
    }

    // XPropertySet:

    public void addPropertyChangeListener(String arg0, XPropertyChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        implPropertySet.addPropertyChangeListener(arg0, arg1);
    }

    public void addVetoableChangeListener(String arg0, XVetoableChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        implPropertySet.addVetoableChangeListener(arg0, arg1);
    }

    public XPropertySetInfo getPropertySetInfo() {
        return implPropertySet.getPropertySetInfo();
    }

    public Object getPropertyValue(String arg0) throws UnknownPropertyException, WrappedTargetException {
        return implPropertySet.getPropertyValue(arg0);
    }

    public void removePropertyChangeListener(String arg0, XPropertyChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        implPropertySet.removePropertyChangeListener(arg0, arg1);
    }

    public void removeVetoableChangeListener(String arg0, XVetoableChangeListener arg1) throws UnknownPropertyException, WrappedTargetException {
        implPropertySet.removeVetoableChangeListener(arg0, arg1);
    }

    public void setPropertyValue(String arg0, Object arg1)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        implPropertySet.setPropertyValue(arg0, arg1);
    }

    // XCancellable:

    public void cancel() {
        implCancellable.cancel();
    }

    // XResultSetMetaDataSupplier:

    public XResultSetMetaData getMetaData() throws SQLException {
        return new PostgresqlResultSetMetaData(implResultSetMetaDataSupplier.getMetaData());
    }

    // XParameters:

    public void clearParameters() throws SQLException {
        implParameters.clearParameters();
    }

    public void setArray(int arg0, XArray arg1) throws SQLException {
        implParameters.setArray(arg0, arg1);
    }

    public void setBinaryStream(int arg0, XInputStream arg1, int arg2) throws SQLException {
        implParameters.setBinaryStream(arg0, arg1, arg2);
    }

    public void setBlob(int arg0, XBlob arg1) throws SQLException {
        implParameters.setBlob(arg0, arg1);
    }

    public void setBoolean(int arg0, boolean arg1) throws SQLException {
        implParameters.setBoolean(arg0, arg1);
    }

    public void setByte(int arg0, byte arg1) throws SQLException {
        implParameters.setByte(arg0, arg1);
    }

    public void setBytes(int arg0, byte[] arg1) throws SQLException {
        implParameters.setBytes(arg0, arg1);
    }

    public void setCharacterStream(int arg0, XInputStream arg1, int arg2) throws SQLException {
        implParameters.setCharacterStream(arg0, arg1, arg2);
    }

    public void setClob(int arg0, XClob arg1) throws SQLException {
        implParameters.setClob(arg0, arg1);
    }

    public void setDate(int arg0, Date arg1) throws SQLException {
        implParameters.setDate(arg0, arg1);
    }

    public void setDouble(int arg0, double arg1) throws SQLException {
        implParameters.setDouble(arg0, arg1);
    }

    public void setFloat(int arg0, float arg1) throws SQLException {
        implParameters.setFloat(arg0, arg1);
    }

    public void setInt(int arg0, int arg1) throws SQLException {
        implParameters.setInt(arg0, arg1);
    }

    public void setLong(int arg0, long arg1) throws SQLException {
        implParameters.setLong(arg0, arg1);
    }

    public void setNull(int arg0, int arg1) throws SQLException {
        implParameters.setNull(arg0, arg1);
    }

    public void setObject(int arg0, Object arg1) throws SQLException {
        implParameters.setObject(arg0, arg1);
    }

    public void setObjectNull(int arg0, int arg1, String arg2) throws SQLException {
        implParameters.setObjectNull(arg0, arg1, arg2);
    }

    public void setObjectWithInfo(int arg0, Object arg1, int arg2, int arg3) throws SQLException {
        implParameters.setObjectWithInfo(arg0, arg1, arg2, arg3);
    }

    public void setRef(int arg0, XRef arg1) throws SQLException {
        implParameters.setRef(arg0, arg1);
    }

    public void setShort(int arg0, short arg1) throws SQLException {
        implParameters.setShort(arg0, arg1);
    }

    public void setString(int arg0, String arg1) throws SQLException {
        implParameters.setString(arg0, arg1);
    }

    public void setTime(int arg0, Time arg1) throws SQLException {
        implParameters.setTime(arg0, arg1);
    }

    public void setTimestamp(int arg0, DateTime arg1) throws SQLException {
        implParameters.setTimestamp(arg0, arg1);
    }

    // XPreparedBatchExecution:

    public void addBatch() throws SQLException {
        implPreparedBatchExecution.addBatch();
    }

    public void clearBatch() throws SQLException {
        implPreparedBatchExecution.clearBatch();
    }

    public int[] executeBatch() throws SQLException {
        return implPreparedBatchExecution.executeBatch();
    }

    // XWarningsSupplier:

    public void clearWarnings() throws SQLException {
        implWarningsSupplier.clearWarnings();
    }

    public Object getWarnings() throws SQLException {
        return implWarningsSupplier.getWarnings();
    }

    // XMultipleResults:

    public boolean getMoreResults() throws SQLException {
        return implMultipleResults.getMoreResults();
    }

    public XResultSet getResultSet() throws SQLException {
        return new PostgresqlResultSet(implMultipleResults.getResultSet(), this);
    }

    public int getUpdateCount() throws SQLException {
        return implMultipleResults.getUpdateCount();
    }
}
