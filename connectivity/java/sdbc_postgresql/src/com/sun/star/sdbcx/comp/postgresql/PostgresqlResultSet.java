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
import com.sun.star.container.XNameAccess;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XColumnLocate;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;
import com.sun.star.util.XCancellable;

public class PostgresqlResultSet extends ComponentBase
        implements XResultSet, XRow, XResultSetMetaDataSupplier, XCancellable, XWarningsSupplier, XResultSetUpdate,
        XRowUpdate, XCloseable, XColumnLocate, XPropertySet {

    private XResultSet impl;
    private XRow implRow;
    private XResultSetMetaDataSupplier implResultSetMetaDataSupplier;
    private XCancellable implCancellable;
    private XWarningsSupplier implWarningsSupplier;
    private XResultSetUpdate implResultSetUpdate;
    private XRowUpdate implRowUpdate;
    private XCloseable implCloseable;
    private XColumnLocate implColumnLocate;
    private XPropertySet implPropertySet;
    private Object statement;
    private AtomicBoolean isDisposed = new AtomicBoolean(false);

    public PostgresqlResultSet(XResultSet impl, Object statement) {
        this.impl = impl;
        this.implRow = UnoRuntime.queryInterface(XRow.class, impl);
        this.implResultSetMetaDataSupplier = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, impl);
        this.implCancellable = UnoRuntime.queryInterface(XCancellable.class, impl);
        this.implWarningsSupplier = UnoRuntime.queryInterface(XWarningsSupplier.class, impl);
        this.implResultSetUpdate = UnoRuntime.queryInterface(XResultSetUpdate.class, impl);
        this.implRowUpdate = UnoRuntime.queryInterface(XRowUpdate.class, impl);
        this.implCloseable = UnoRuntime.queryInterface(XCloseable.class, impl);
        this.implColumnLocate = UnoRuntime.queryInterface(XColumnLocate.class, impl);
        this.implPropertySet = UnoRuntime.queryInterface(XPropertySet.class, impl);
        this.statement = statement;
    }

    // XComponent:

    @Override
    protected synchronized void postDisposing() {
        isDisposed.set(true);
        try {
            implCloseable.close();
        } catch (SQLException sqlException) {
        }
    };

    private void checkDisposed() throws DisposedException {
        if (isDisposed.get()) {
            throw new DisposedException();
        }
    }

    // XCloseable:

    public void close() throws SQLException {
        dispose();
    }

    // XResultSet:

    public boolean absolute(int arg0) throws SQLException {
        checkDisposed();
        return impl.absolute(arg0);
    }

    public void afterLast() throws SQLException {
        checkDisposed();
        impl.afterLast();
    }

    public void beforeFirst() throws SQLException {
        checkDisposed();
        impl.beforeFirst();
    }

    public boolean first() throws SQLException {
        checkDisposed();
        return impl.first();
    }

    public int getRow() throws SQLException {
        checkDisposed();
        return impl.getRow();
    }

    public Object getStatement() throws SQLException {
        checkDisposed();
        return statement;
    }

    public boolean isAfterLast() throws SQLException {
        checkDisposed();
        return impl.isAfterLast();
    }

    public boolean isBeforeFirst() throws SQLException {
        checkDisposed();
        return impl.isBeforeFirst();
    }

    public boolean isFirst() throws SQLException {
        checkDisposed();
        return impl.isFirst();
    }

    public boolean isLast() throws SQLException {
        checkDisposed();
        return impl.isLast();
    }

    public boolean last() throws SQLException {
        checkDisposed();
        return impl.last();
    }

    public boolean next() throws SQLException {
        checkDisposed();
        return impl.next();
    }

    public boolean previous() throws SQLException {
        checkDisposed();
        return impl.previous();
    }

    public void refreshRow() throws SQLException {
        checkDisposed();
        impl.refreshRow();
    }

    public boolean relative(int arg0) throws SQLException {
        checkDisposed();
        return impl.relative(arg0);
    }

    public boolean rowDeleted() throws SQLException {
        checkDisposed();
        return impl.rowDeleted();
    }

    public boolean rowInserted() throws SQLException {
        checkDisposed();
        return impl.rowInserted();
    }

    public boolean rowUpdated() throws SQLException {
        checkDisposed();
        return impl.rowUpdated();
    }

    // XRow:

    public XArray getArray(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getArray(arg0);
    }

    public XInputStream getBinaryStream(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getBinaryStream(arg0);
    }

    public XBlob getBlob(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getBlob(arg0);
    }

    public boolean getBoolean(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getBoolean(arg0);
    }

    public byte getByte(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getByte(arg0);
    }

    public byte[] getBytes(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getBytes(arg0);
    }

    public XInputStream getCharacterStream(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getCharacterStream(arg0);
    }

    public XClob getClob(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getClob(arg0);
    }

    public Date getDate(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getDate(arg0);
    }

    public double getDouble(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getDouble(arg0);
    }

    public float getFloat(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getFloat(arg0);
    }

    public int getInt(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getInt(arg0);
    }

    public long getLong(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getLong(arg0);
    }

    public Object getObject(int arg0, XNameAccess arg1) throws SQLException {
        checkDisposed();
        return implRow.getObject(arg0, arg1);
    }

    public XRef getRef(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getRef(arg0);
    }

    public short getShort(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getShort(arg0);
    }

    public String getString(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getString(arg0);
    }

    public Time getTime(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getTime(arg0);
    }

    public DateTime getTimestamp(int arg0) throws SQLException {
        checkDisposed();
        return implRow.getTimestamp(arg0);
    }

    public boolean wasNull() throws SQLException {
        checkDisposed();
        return implRow.wasNull();
    }

    // XResultSetMetaDataSupplier:

    public XResultSetMetaData getMetaData() throws SQLException {
        checkDisposed();
        return new PostgresqlResultSetMetaData(implResultSetMetaDataSupplier.getMetaData());
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

    // XResultSetUpdate:

    public void cancelRowUpdates() throws SQLException {
        checkDisposed();
        implResultSetUpdate.cancelRowUpdates();
    }

    public void deleteRow() throws SQLException {
        checkDisposed();
        implResultSetUpdate.deleteRow();
    }

    public void insertRow() throws SQLException {
        checkDisposed();
        implResultSetUpdate.insertRow();
    }

    public void moveToCurrentRow() throws SQLException {
        checkDisposed();
        implResultSetUpdate.moveToCurrentRow();
    }

    public void moveToInsertRow() throws SQLException {
        checkDisposed();
        implResultSetUpdate.moveToInsertRow();
    }

    public void updateRow() throws SQLException {
        checkDisposed();
        implResultSetUpdate.updateRow();
    }

    // XRowUpdate:

    public void updateBinaryStream(int arg0, XInputStream arg1, int arg2) throws SQLException {
        checkDisposed();
        implRowUpdate.updateBinaryStream(arg0, arg1, arg2);
    }

    public void updateBoolean(int arg0, boolean arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateBoolean(arg0, arg1);
    }

    public void updateByte(int arg0, byte arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateByte(arg0, arg1);
    }

    public void updateBytes(int arg0, byte[] arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateBytes(arg0, arg1);
    }

    public void updateCharacterStream(int arg0, XInputStream arg1, int arg2) throws SQLException {
        checkDisposed();
        implRowUpdate.updateCharacterStream(arg0, arg1, arg2);
    }

    public void updateDate(int arg0, Date arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateDate(arg0, arg1);
    }

    public void updateDouble(int arg0, double arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateDouble(arg0, arg1);
    }

    public void updateFloat(int arg0, float arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateFloat(arg0, arg1);
    }

    public void updateInt(int arg0, int arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateInt(arg0, arg1);
    }

    public void updateLong(int arg0, long arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateLong(arg0, arg1);
    }

    public void updateNull(int arg0) throws SQLException {
        checkDisposed();
        implRowUpdate.updateNull(arg0);
    }

    public void updateNumericObject(int arg0, Object arg1, int arg2) throws SQLException {
        checkDisposed();
        implRowUpdate.updateNumericObject(arg0, arg1, arg2);
    }

    public void updateObject(int arg0, Object arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateObject(arg0, arg1);
    }

    public void updateShort(int arg0, short arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateShort(arg0, arg1);
    }

    public void updateString(int arg0, String arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateString(arg0, arg1);
    }

    public void updateTime(int arg0, Time arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateTime(arg0, arg1);
    }

    public void updateTimestamp(int arg0, DateTime arg1) throws SQLException {
        checkDisposed();
        implRowUpdate.updateTimestamp(arg0, arg1);
    }

    // XColumnLocate:

    public int findColumn(String arg0) throws SQLException {
        checkDisposed();
        return implColumnLocate.findColumn(arg0);
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
}
