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

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetMetaData;

public class JavaSQLResultSetMetaData extends WeakBase implements XResultSetMetaData {
    private JavaSQLConnection connection;
    private java.sql.ResultSetMetaData jdbcResultSetMetaData;
    private int columnCount;

    public JavaSQLResultSetMetaData(JavaSQLConnection connection, java.sql.ResultSetMetaData jdbcResultSetMetaData) {
        this.connection = connection;
        this.jdbcResultSetMetaData = jdbcResultSetMetaData;
        columnCount = -1;
    }

    @Override
    public int getColumnDisplaySize(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.getColumnDisplaySize(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getColumnType(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.getColumnType(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getColumnCount() throws SQLException {
        try {
            if (columnCount == -1) {
                columnCount = jdbcResultSetMetaData.getColumnCount();
            }
            return columnCount;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isCaseSensitive(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isCaseSensitive(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getSchemaName(int column) throws SQLException {
        try {
            String schemaName = jdbcResultSetMetaData.getSchemaName(column);
            if (schemaName == null) {
                schemaName = "";
            }
            return schemaName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getColumnName(int column) throws SQLException {
        try {
            String columnName = jdbcResultSetMetaData.getColumnName(column);
            if (columnName == null) {
                columnName = "";
            }
            return columnName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getTableName(int column) throws SQLException {
        try {
            String tableName = jdbcResultSetMetaData.getTableName(column);
            if (tableName == null) {
                tableName = "";
            }
            return tableName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getCatalogName(int column) throws SQLException {
        try {
            String catalogName = jdbcResultSetMetaData.getCatalogName(column);
            if (catalogName == null) {
                catalogName = "";
            }
            return catalogName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getColumnTypeName(int column) throws SQLException {
        try {
            String columnTypeName = jdbcResultSetMetaData.getColumnTypeName(column);
            if (columnTypeName == null) {
                columnTypeName = "";
            }
            return columnTypeName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getColumnLabel(int column) throws SQLException {
        try {
            String columnLabel = jdbcResultSetMetaData.getColumnLabel(column);
            if (columnLabel == null) {
                columnLabel = "";
            }
            return columnLabel;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getColumnServiceName(int column) throws SQLException {
        try {
            String columnServiceName = jdbcResultSetMetaData.getColumnClassName(column);
            if (columnServiceName == null) {
                columnServiceName = "";
            }
            return columnServiceName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isCurrency(int column) throws SQLException {
        try {
            if (connection.isIgnoreCurrencyEnabled()) {
                return false;
            }
            return jdbcResultSetMetaData.isCurrency(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isAutoIncrement(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isAutoIncrement(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isSigned(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isSigned(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getPrecision(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.getPrecision(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getScale(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.getScale(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int isNullable(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isNullable(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isSearchable(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isSearchable(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isReadOnly(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isReadOnly(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isDefinitelyWritable(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isDefinitelyWritable(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isWritable(int column) throws SQLException {
        try {
            return jdbcResultSetMetaData.isWritable(column);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }
}
