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

import org.apache.openoffice.comp.sdbc.dbtools.util.DBTypeConversion;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.container.XNameAccess;
import com.sun.star.io.XInputStream;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XOutParameters;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XRow;
import com.sun.star.uno.Any;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class JavaSQLCallableStatement extends JavaSQLPreparedStatement implements XRow, XOutParameters {
    public JavaSQLCallableStatement(JavaSQLConnection connection, String sql) {
        super(connection, sql);
    }

    @Override
    protected void createStatement() throws SQLException {
        checkDisposed();
        if (jdbcStatement == null) {
            try {
                try {
                    jdbcStatement = connection.getJDBCConnection().prepareCall(
                            sqlStatement, resultSetType, resultSetConcurrency);
                } catch (NoSuchMethodError noSuchMethodError) {
                    jdbcStatement = connection.getJDBCConnection().prepareCall(sqlStatement);
                }
            } catch (java.sql.SQLException sqlException) {
                throw Tools.toUnoExceptionLogged(this, logger, sqlException);
            }
        }
    }

    // XRow

    public synchronized XInputStream getBinaryStream(int columnIndex) throws SQLException {
        createStatement();
        XBlob blob = getBlob(columnIndex);
        if (blob != null) {
            return blob.getBinaryStream();
        }
        return null;
    }

    @Override
    public XInputStream getCharacterStream(int columnIndex) throws SQLException {
        createStatement();
        XClob clob = getClob(columnIndex);
        if (clob != null) {
            return clob.getCharacterStream();
        }
        return null;
    }

    @Override
    public synchronized boolean getBoolean(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getBoolean(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized byte getByte(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getByte(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized byte[] getBytes(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getBytes(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized Date getDate(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Date jdbcDate = ((java.sql.CallableStatement)jdbcStatement).getDate(columnIndex);
            if (jdbcDate != null) {
                return DBTypeConversion.toDate(jdbcDate.toString());
            } else {
                return new Date();
            }
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized double getDouble(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getDouble(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized float getFloat(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getFloat(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized int getInt(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getInt(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized long getLong(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getLong(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized XArray getArray(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Array array = ((java.sql.CallableStatement)jdbcStatement).getArray(columnIndex);
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
    public synchronized XClob getClob(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Clob clob = ((java.sql.CallableStatement)jdbcStatement).getClob(columnIndex);
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
    public synchronized XBlob getBlob(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Blob blob = ((java.sql.CallableStatement)jdbcStatement).getBlob(columnIndex);
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
    public synchronized XRef getRef(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Ref ref = ((java.sql.CallableStatement)jdbcStatement).getRef(columnIndex);
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
    public synchronized Object getObject(int columnIndex, XNameAccess typeMap) throws SQLException {
        createStatement();
        if (typeMap.hasElements()) {
            throw new SQLException(
                    SharedResources.getInstance().getResourceStringWithSubstitution(
                            Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "Type maps"),
                    this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
        }
        try {
            Object ret = Any.VOID;
            Object object = ((java.sql.CallableStatement)jdbcStatement).getObject(columnIndex);
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
    public synchronized short getShort(int columnIndex) throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).getShort(columnIndex);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public synchronized String getString(int columnIndex) throws SQLException {
        createStatement();
        try {
            String string = ((java.sql.CallableStatement)jdbcStatement).getString(columnIndex);
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
    public synchronized Time getTime(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Time time = ((java.sql.CallableStatement)jdbcStatement).getTime(columnIndex);
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
    public synchronized DateTime getTimestamp(int columnIndex) throws SQLException {
        createStatement();
        try {
            java.sql.Timestamp timestamp = ((java.sql.CallableStatement)jdbcStatement).getTimestamp(columnIndex);
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
    public synchronized boolean wasNull() throws SQLException {
        createStatement();
        try {
            return ((java.sql.CallableStatement)jdbcStatement).wasNull();
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    // XOutParameters

    @Override
    public synchronized void registerOutParameter(int index, int sqlType, String typeName) throws SQLException {
        createStatement();
        try {
            ((java.sql.CallableStatement)jdbcStatement).registerOutParameter(index, sqlType, typeName);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public void registerNumericOutParameter(int index, int sqlType, int scale) throws SQLException {
        createStatement();
        try {
            ((java.sql.CallableStatement)jdbcStatement).registerOutParameter(index, sqlType, scale);
        } catch (java.sql.SQLException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }
}
