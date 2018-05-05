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

import java.io.ByteArrayInputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.math.BigDecimal;

import org.apache.openoffice.comp.sdbc.dbtools.util.DBTypeConversion;
import org.apache.openoffice.comp.sdbc.dbtools.util.DbTools;
import org.apache.openoffice.comp.sdbc.dbtools.util.ORowSetValue;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.io.IOException;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XPreparedBatchExecution;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class JavaSQLPreparedStatement extends JavaSQLStatementBase
        implements XPreparedStatement, XResultSetMetaDataSupplier, XParameters, XPreparedBatchExecution, XServiceInfo {

    private static final String[] services = {
            "com.sun.star.sdbc.PreparedStatement"
    };

    public JavaSQLPreparedStatement(JavaSQLConnection connection, String sqlStatement) {
        super(connection);
        this.sqlStatement = sqlStatement;
    }

    @Override
    protected void createStatement() throws SQLException {
        checkDisposed();
        if (jdbcStatement == null) {
            try {
                try {
                    jdbcStatement = connection.getJDBCConnection().prepareStatement(
                            sqlStatement, resultSetType, resultSetConcurrency);
                } catch (NoSuchMethodError noSuchMethodError) {
                    jdbcStatement = connection.getJDBCConnection().prepareStatement(sqlStatement);
                }
            } catch (java.sql.SQLException sqlException) {
                throw Tools.toUnoExceptionLogged(this, logger, sqlException);
            }
        }
    }

    // XServiceInfo

    @Override
    public String getImplementationName() {
        return "com.sun.star.sdbcx.JPreparedStatement";
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

    // XPreparedStatement

    @Override
    public synchronized boolean execute() throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_EXECUTING_PREPARED);
        try {
            return ((java.sql.PreparedStatement)jdbcStatement).execute();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized int executeUpdate() throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_EXECUTING_PREPARED_UPDATE);
        try {
            return ((java.sql.PreparedStatement)jdbcStatement).executeUpdate();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized XResultSet executeQuery() throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_EXECUTING_PREPARED_QUERY);
        try {
            java.sql.ResultSet jdbcResults = ((java.sql.PreparedStatement)jdbcStatement).executeQuery();
            if (jdbcResults != null) {
                return new JavaSQLResultSet(jdbcResults, connection);
            }
            return null;
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public XConnection getConnection() throws SQLException {
        return connection;
    }

    // XParameters

    @Override
    public synchronized void setArray(int index, XArray x) throws SQLException {
        String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "XParameters::setArray");
        throw new SQLException(error, this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.text(), 0, Any.VOID);
    }

    @Override
    public synchronized void setBinaryStream(int index, XInputStream x, int length) throws SQLException {
        logger.log(LogLevel.FINER, Resources.STR_LOG_BINARYSTREAM_PARAMETER, index);
        createStatement();
        try {
            // FIXME: why did the C++ implementation copy the stream here? It's a huge waste of memory.
            byte[][] bytesReference = new byte[1][0];
            int bytesRead = x.readBytes(bytesReference, length);
            ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(bytesReference[0], 0, bytesRead);
            ((java.sql.PreparedStatement)jdbcStatement).setBinaryStream(index, byteArrayInputStream, bytesRead);
        } catch (java.sql.SQLException | IOException exception) {
            throw Tools.toUnoExceptionLogged(this, logger, exception);
        }
    }

    @Override
    public synchronized void setBlob(int index, XBlob x) throws SQLException {
        String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "XParameters::setBlob");
        throw new SQLException(error, this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.text(), 0, Any.VOID);
    }

    @Override
    public synchronized void setBoolean(int index, boolean x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_BOOLEAN_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setBoolean(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setByte(int index, byte x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_BYTE_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setByte(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setBytes(int index, byte[] x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_BYTES_PARAMETER, index);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setBytes(index, x.clone());
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setCharacterStream(int index, XInputStream x, int length) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINER, Resources.STR_LOG_CHARSTREAM_PARAMETER, index);
        try {
            // FIXME: why did the C++ implementation copy the stream here? It's a huge waste of memory.
            byte[][] bytesReference = new byte[1][0];
            int bytesRead = x.readBytes(bytesReference, length);
            ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(bytesReference[0], 0, bytesRead);
            InputStreamReader inputStreamReader = new InputStreamReader(byteArrayInputStream, "UTF-16LE");
            ((java.sql.PreparedStatement)jdbcStatement).setCharacterStream(index, inputStreamReader, length);
        } catch (java.sql.SQLException | IOException | UnsupportedEncodingException exception) {
            throw Tools.toUnoExceptionLogged(this, logger, exception);
        }
    }

    @Override
    public synchronized void setClob(int index, XClob x) throws SQLException {
        String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "XParameters::setClob");
        throw new SQLException(error, this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.text(), 0, Any.VOID);
    }

    @Override
    public synchronized void setDate(int index, Date x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_DATE_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setDate(
                    index, java.sql.Date.valueOf(DBTypeConversion.toDateString(x)));
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setDouble(int index, double x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_DOUBLE_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setDouble(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setFloat(int index, float x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_FLOAT_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setFloat(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setInt(int index, int x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_INT_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setInt(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setLong(int index, long x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_LONG_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setLong(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setNull(int index, int sqlType) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_NULL_PARAMETER, index, sqlType);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setNull(index, sqlType);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public void setObject(int index, Object x) throws SQLException {
        if (!DbTools.setObject(this, index, x)) {
            String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                    Resources.STR_UNKNOWN_PARA_TYPE, "$position$", Integer.toString(index));
            throw new SQLException(error, this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, Any.VOID);
        }
    }

    @Override
    public synchronized void setObjectNull(int index, int sqlType, String typeName) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINER, Resources.STR_LOG_OBJECT_NULL_PARAMETER, index);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setObject(index, null);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setObjectWithInfo(int index, Object x, int targetSqlType, int scale) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINER, Resources.STR_LOG_OBJECT_NULL_PARAMETER, index);
        try {
            if (targetSqlType == DataType.DECIMAL || targetSqlType == DataType.NUMERIC) {
                BigDecimal bigDecimal;
                if (AnyConverter.isDouble(x)) {
                    bigDecimal = new BigDecimal(AnyConverter.toDouble(x));
                } else {
                    ORowSetValue rowSetValue = new ORowSetValue();
                    rowSetValue.fill(x);
                    String value = rowSetValue.toString();
                    if (value.isEmpty()) {
                        bigDecimal = new BigDecimal(0.0);
                    } else {
                        bigDecimal = new BigDecimal(value);
                    }
                }
                ((java.sql.PreparedStatement)jdbcStatement).setObject(index, bigDecimal, targetSqlType, scale);
            } else {
                ((java.sql.PreparedStatement)jdbcStatement).setObject(index, AnyConverter.toString(x), targetSqlType, scale);
            }
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoExceptionLogged(this, logger, exception);
        }
    }

    @Override
    public void setRef(int index, XRef x) throws SQLException {
        String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "XParameters::setRef");
        throw new SQLException(error, this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.text(), 0, Any.VOID);
    }

    @Override
    public synchronized void setShort(int index, short x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_SHORT_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setShort(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setString(int index, String x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_STRING_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setString(index, x);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setTime(int index, Time x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_TIME_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setTime(
                    index, java.sql.Time.valueOf(DBTypeConversion.toTimeString(x)));
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void setTimestamp(int index, DateTime x) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_TIMESTAMP_PARAMETER, index, x);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).setTimestamp(
                    index, java.sql.Timestamp.valueOf(DBTypeConversion.toDateTimeString(x)));
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void clearParameters() throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_CLEAR_PARAMETERS);
        try {
            ((java.sql.PreparedStatement)jdbcStatement).clearParameters();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    // XPreparedBatchExecution

    @Override
    public synchronized void clearBatch() throws SQLException {
        createStatement();
        try {
            ((java.sql.PreparedStatement)jdbcStatement).clearBatch();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized void addBatch() throws SQLException {
        createStatement();
        try {
            ((java.sql.PreparedStatement)jdbcStatement).addBatch();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized int[] executeBatch() throws SQLException {
        createStatement();
        try {
            return ((java.sql.PreparedStatement)jdbcStatement).executeBatch();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    // XResultSetMetaDataSupplier

    @Override
    public synchronized XResultSetMetaData getMetaData() throws SQLException {
        createStatement();
        try {
            java.sql.ResultSetMetaData jdbcMetadata = ((java.sql.PreparedStatement)jdbcStatement).getMetaData();
            if (jdbcMetadata != null) {
                return new JavaSQLResultSetMetaData(connection, jdbcMetadata);
            } else {
                return null;
            }
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }
}
