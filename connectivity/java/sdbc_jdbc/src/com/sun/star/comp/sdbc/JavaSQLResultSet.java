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
package com.sun.star.comp.sdbc;

import java.io.InputStreamReader;
import java.math.BigDecimal;
import java.nio.charset.Charset;

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySet;
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySetAdapter.PropertyGetter;
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySetAdapter.PropertySetter;
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.ResourceBasedEventLogger;
import org.apache.openoffice.comp.sdbc.dbtools.util.DBTypeConversion;
import org.apache.openoffice.comp.sdbc.dbtools.util.DbTools;
import org.apache.openoffice.comp.sdbc.dbtools.util.PropertyIds;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.comp.sdbc.ConnectionLog.ObjectType;
import com.sun.star.container.XNameAccess;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.adapter.InputStreamToXInputStreamAdapter;
import com.sun.star.lib.uno.adapter.XInputStreamToInputStreamAdapter;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.SQLWarning;
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
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class JavaSQLResultSet extends PropertySet
        implements XResultSet, XRow, XResultSetMetaDataSupplier,
            XWarningsSupplier, XResultSetUpdate, XRowUpdate, XCloseable,
            XColumnLocate, XServiceInfo {

    private static final String[] services = {
            "com.sun.star.sdbc.ResultSet"
    };

    private java.sql.ResultSet jdbcResultSet;
    private JavaSQLConnection connection;
    private Object statement;
    private ResourceBasedEventLogger logger;

    public JavaSQLResultSet(java.sql.ResultSet jdbcResultSet, JavaSQLConnection connection) {
        this (jdbcResultSet, connection, null);
    }

    public JavaSQLResultSet(java.sql.ResultSet jdbcResultSet, JavaSQLConnection connection, Object statement) {
        this.jdbcResultSet = jdbcResultSet;
        this.connection = connection;
        this.statement = statement;
        logger = new ConnectionLog(connection.getLogger(), ObjectType.RESULT);
        registerProperties();
    }

    // XComponent

    @Override
    protected synchronized void postDisposing() {
        super.postDisposing();
        if (jdbcResultSet != null) {
            try {
                jdbcResultSet.close();
            } catch (java.sql.SQLException jdbcSqlException) {
                logger.log(LogLevel.WARNING, jdbcSqlException);
            }
            jdbcResultSet = null;
        }
    }

    // XCloseable

    @Override
    public void close() throws SQLException {
        dispose();
    }

    // XServiceInfo

    @Override
    public String getImplementationName() {
        return "com.sun.star.sdbcx.JResultSet";
    }

    @Override
    public String[] getSupportedServiceNames() {
        return services.clone();
    }

    @Override
    public boolean supportsService(String serviceName) {
        for (String service : services) {
            if (service.equals(serviceName)) {
                return true;
            }
        }
        return false;
    }

    // XPropertySet

    private void registerProperties() {
        registerProperty(PropertyIds.CURSORNAME.name, PropertyIds.CURSORNAME.id, Type.STRING, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getCursorName();
                    }
                }, null);
        registerProperty(PropertyIds.RESULTSETCONCURRENCY.name, PropertyIds.RESULTSETCONCURRENCY.id,
                Type.LONG, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getResultSetConcurrency();
                    }
                }, null);
        registerProperty(PropertyIds.RESULTSETTYPE.name, PropertyIds.RESULTSETTYPE.id,
                Type.LONG, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getResultSetType();
                    }
                }, null);
        registerProperty(PropertyIds.FETCHDIRECTION.name, PropertyIds.FETCHDIRECTION.id,
                Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getFetchDirection();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setFetchDirection((int)value);
                    }
                });
        registerProperty(PropertyIds.FETCHSIZE.name, PropertyIds.FETCHSIZE.id,
                Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getFetchSize();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setFetchSize((int)value);
                    }
                });
    }

    private String getCursorName() throws WrappedTargetException {
        try {
            String cursorName = jdbcResultSet.getCursorName();
            if (cursorName == null) {
                cursorName = "";
            }
            return cursorName;
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getFetchDirection() throws WrappedTargetException {
        try {
            return jdbcResultSet.getFetchDirection();
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private void setFetchDirection(int value) throws WrappedTargetException {
        try {
            jdbcResultSet.setFetchDirection(value);
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getFetchSize() throws WrappedTargetException {
        try {
            return jdbcResultSet.getFetchSize();
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private void setFetchSize(int value) throws WrappedTargetException {
        try {
            jdbcResultSet.setFetchSize(value);
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getResultSetConcurrency() throws WrappedTargetException {
        try {
            return jdbcResultSet.getConcurrency();
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getResultSetType() throws WrappedTargetException {
        try {
            return jdbcResultSet.getType();
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    // everything else

    @Override
    public int findColumn(String columnName) throws SQLException {
        try {
            return jdbcResultSet.findColumn(columnName);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XInputStream getBinaryStream(int columnIndex) throws SQLException {
        try {
            return new InputStreamToXInputStreamAdapter(jdbcResultSet.getBinaryStream(columnIndex));
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XInputStream getCharacterStream(int columnIndex) throws SQLException {
        try {
            return new InputStreamToXInputStreamAdapter(new ReaderInputStream(jdbcResultSet.getCharacterStream(columnIndex)));
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean getBoolean(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getBoolean(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public byte getByte(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getByte(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public byte[] getBytes(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getBytes(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public Date getDate(int columnIndex) throws SQLException {
        try {
            java.sql.Date jdbcDate = jdbcResultSet.getDate(columnIndex);
            if (jdbcDate != null) {
                return DBTypeConversion.toDate(jdbcDate.toString());
            } else {
                return null;
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public double getDouble(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getDouble(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public float getFloat(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getFloat(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public int getInt(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getInt(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public int getRow() throws SQLException {
        try {
            return jdbcResultSet.getRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public long getLong(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getLong(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSetMetaData getMetaData() throws SQLException {
        try {
            java.sql.ResultSetMetaData jdbcMetaData = jdbcResultSet.getMetaData();
            if (jdbcMetaData != null) {
                return new JavaSQLResultSetMetaData(connection, jdbcMetaData);
            } else {
                return null;
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XArray getArray(int columnIndex) throws SQLException {
        try {
            java.sql.Array array = jdbcResultSet.getArray(columnIndex);
            if (array != null) {
                return new JavaSQLArray(logger, array);
            } else {
                return null;
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XClob getClob(int columnIndex) throws SQLException {
        try {
            java.sql.Clob clob = jdbcResultSet.getClob(columnIndex);
            if (clob != null) {
                return new JavaSQLClob(logger, clob);
            } else {
                return null;
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XBlob getBlob(int columnIndex) throws SQLException {
        try {
            java.sql.Blob blob = jdbcResultSet.getBlob(columnIndex);
            if (blob != null) {
                return new JavaSQLBlob(logger, blob);
            } else {
                return null;
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XRef getRef(int columnIndex) throws SQLException {
        try {
            java.sql.Ref ref = jdbcResultSet.getRef(columnIndex);
            if (ref != null) {
                return new JavaSQLRef(ref);
            } else {
                return null;
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public Object getObject(int columnIndex, XNameAccess typeMap) throws SQLException {
        if (typeMap.hasElements()) {
            throw new SQLException(
                    SharedResources.getInstance().getResourceStringWithSubstitution(
                            Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "Type maps"),
                    this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
        }
        try {
            Object ret = Any.VOID;
            Object object = jdbcResultSet.getObject(columnIndex);
            if (object instanceof String) {
                ret = (String) object;
            } else if (object instanceof Boolean) {
                ret = (Boolean) object;
            } else if (object instanceof java.sql.Date) {
                ret = DBTypeConversion.toDate(((java.sql.Date)object).toString());
            } else if (object instanceof java.sql.Time) {
                ret = DBTypeConversion.toTime(((java.sql.Time)object).toString());
            } else if (object instanceof java.sql.Timestamp) {
                ret = DBTypeConversion.toDateTime(((java.sql.Timestamp)object).toString());
            }
            return ret;
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoExceptionLogged(this, logger, exception);
        }
    }

    @Override
    public short getShort(int columnIndex) throws SQLException {
        try {
            return jdbcResultSet.getShort(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public String getString(int columnIndex) throws SQLException {
        try {
            String string = jdbcResultSet.getString(columnIndex);
            if (string != null) {
                return string;
            } else {
                return "";
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public Time getTime(int columnIndex) throws SQLException {
        try {
            java.sql.Time time = jdbcResultSet.getTime(columnIndex);
            if (time != null) {
                return DBTypeConversion.toTime(time.toString());
            } else {
                return new Time();
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public DateTime getTimestamp(int columnIndex) throws SQLException {
        try {
            java.sql.Timestamp timestamp = jdbcResultSet.getTimestamp(columnIndex);
            if (timestamp != null) {
                return DBTypeConversion.toDateTime(timestamp.toString());
            } else {
                return new DateTime();
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean isAfterLast() throws SQLException {
        try {
            return jdbcResultSet.isAfterLast();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean isFirst() throws SQLException {
        try {
            return jdbcResultSet.isFirst();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean isLast() throws SQLException {
        try {
            return jdbcResultSet.isLast();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void beforeFirst() throws SQLException {
        try {
            jdbcResultSet.beforeFirst();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void afterLast() throws SQLException {
        try {
            jdbcResultSet.afterLast();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean first() throws SQLException {
        try {
            return jdbcResultSet.first();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean last() throws SQLException {
        try {
            return jdbcResultSet.last();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean absolute(int row) throws SQLException {
        try {
            return jdbcResultSet.absolute(row);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean relative(int row) throws SQLException {
        try {
            return jdbcResultSet.relative(row);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean previous() throws SQLException {
        try {
            return jdbcResultSet.previous();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public Object getStatement() throws SQLException {
        return statement;
    }

    @Override
    public boolean rowDeleted() throws SQLException {
        try {
            return jdbcResultSet.rowDeleted();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean rowInserted() throws SQLException {
        try {
            return jdbcResultSet.rowInserted();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean rowUpdated() throws SQLException {
        try {
            return jdbcResultSet.rowUpdated();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean isBeforeFirst() throws SQLException {
        try {
            return jdbcResultSet.isBeforeFirst();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean next() throws SQLException {
        try {
            return jdbcResultSet.next();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean wasNull() throws SQLException {
        try {
            return jdbcResultSet.wasNull();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void clearWarnings() throws SQLException {
        try {
            jdbcResultSet.clearWarnings();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public Object getWarnings() throws SQLException {
        try {
            java.sql.SQLWarning javaWarning = jdbcResultSet.getWarnings();
            if (javaWarning != null) {
                java.lang.Throwable nextException = javaWarning.getCause();
                SQLWarning warning = new SQLWarning(javaWarning.getMessage());
                warning.Context = this;
                warning.SQLState = javaWarning.getSQLState();
                warning.ErrorCode = javaWarning.getErrorCode();
                warning.NextException = nextException != null ? Tools.toUnoException(this, nextException) : Any.VOID;
                return warning;
            }
            return Any.VOID;
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public void insertRow() throws SQLException {
        try {
            jdbcResultSet.insertRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateRow() throws SQLException {
        try {
            jdbcResultSet.updateRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void deleteRow() throws SQLException {
        try {
            jdbcResultSet.deleteRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void cancelRowUpdates() throws SQLException {
        try {
            jdbcResultSet.cancelRowUpdates();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void moveToInsertRow() throws SQLException {
        try {
            jdbcResultSet.moveToInsertRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void moveToCurrentRow() throws SQLException {
        try {
            jdbcResultSet.moveToCurrentRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateNull(int columnIndex) throws SQLException {
        try {
            jdbcResultSet.updateNull(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateBoolean(int columnIndex, boolean x) throws SQLException {
        try {
            jdbcResultSet.updateBoolean(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateByte(int columnIndex, byte x) throws SQLException {
        try {
            jdbcResultSet.updateByte(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateShort(int columnIndex, short x) throws SQLException {
        try {
            jdbcResultSet.updateShort(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateInt(int columnIndex, int x) throws SQLException {
        try {
            jdbcResultSet.updateInt(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateLong(int columnIndex, long x) throws SQLException {
        try {
            jdbcResultSet.updateLong(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateFloat(int columnIndex, float x) throws SQLException {
        try {
            jdbcResultSet.updateFloat(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateDouble(int columnIndex, double x) throws SQLException {
        try {
            jdbcResultSet.updateDouble(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateString(int columnIndex, String x) throws SQLException {
        try {
            jdbcResultSet.updateString(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateBytes(int columnIndex, byte[] x) throws SQLException {
        try {
            jdbcResultSet.updateBytes(columnIndex, x);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateDate(int columnIndex, Date x) throws SQLException {
        try {
            jdbcResultSet.updateDate(columnIndex, java.sql.Date.valueOf(DBTypeConversion.toDateString(x)));
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateTime(int columnIndex, Time x) throws SQLException {
        try {
            jdbcResultSet.updateTime(columnIndex, java.sql.Time.valueOf(DBTypeConversion.toTimeString(x)));
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateTimestamp(int columnIndex, DateTime x) throws SQLException {
        try {
            jdbcResultSet.updateTimestamp(columnIndex,
                    java.sql.Timestamp.valueOf(DBTypeConversion.toDateTimeString(x)));
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateBinaryStream(int columnIndex, XInputStream x, int length) throws SQLException {
        try {
            jdbcResultSet.updateBinaryStream(columnIndex,
                    new BoundedInputStream(new XInputStreamToInputStreamAdapter(x), length & 0xffff_ffffL), length);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateCharacterStream(int columnIndex, XInputStream x, int length) throws SQLException {
        try {
            // FIXME: charset?
            jdbcResultSet.updateCharacterStream(columnIndex, new InputStreamReader(
                    new BoundedInputStream(new XInputStreamToInputStreamAdapter(x), length & 0xffff_ffffL),
                    Charset.forName("UTF16-LE")), length);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void updateObject(int columnIndex, Object x) throws SQLException {
        if (!DbTools.updateObject(this, columnIndex, x)) {
            String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                    Resources.STR_UNKNOWN_COLUMN_TYPE, "$position$", Integer.toString(columnIndex));
            throw new SQLException(error, this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, Any.VOID);
        }
    }

    @Override
    public void updateNumericObject(int columnIndex, Object x, int scale) throws SQLException {
        try {
            BigDecimal bigDecimal;
            if (AnyConverter.isDouble(x)) {
                bigDecimal = BigDecimal.valueOf(AnyConverter.toDouble(x));
            } else {
                bigDecimal = new BigDecimal(AnyConverter.toString(x));
            }
            jdbcResultSet.updateObject(columnIndex, bigDecimal, scale);
        } catch (IllegalArgumentException | java.sql.SQLException exception) {
            updateObject(columnIndex, x);
        }
    }

    @Override
    public void refreshRow() throws SQLException {
        try {
            jdbcResultSet.refreshRow();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }


}
