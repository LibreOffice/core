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

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetMetaData;

public class PostgresqlResultSetMetaData extends WeakBase implements XResultSetMetaData {

    private XResultSetMetaData impl;

    public PostgresqlResultSetMetaData(XResultSetMetaData impl) {
        this.impl = impl;
    }

    public String getCatalogName(int arg0) throws SQLException {
        return impl.getCatalogName(arg0);
    }

    public int getColumnCount() throws SQLException {
        return impl.getColumnCount();
    }

    public int getColumnDisplaySize(int arg0) throws SQLException {
        return impl.getColumnDisplaySize(arg0);
    }

    public String getColumnLabel(int arg0) throws SQLException {
        return impl.getColumnLabel(arg0);
    }

    public String getColumnName(int arg0) throws SQLException {
        return impl.getColumnName(arg0);
    }

    public String getColumnServiceName(int arg0) throws SQLException {
        return impl.getColumnServiceName(arg0);
    }

    public int getColumnType(int column) throws SQLException {
        int columnType = impl.getColumnType(column);
        if (columnType == DataType.BIT) {
            String columnName = getColumnTypeName(column);
            if (columnName.equals("bool")) {
                columnType = DataType.BOOLEAN;
            }
        }
        return columnType;
    }

    public String getColumnTypeName(int column) throws SQLException {
        return impl.getColumnTypeName(column);
    }

    public int getPrecision(int arg0) throws SQLException {
        return impl.getPrecision(arg0);
    }

    public int getScale(int arg0) throws SQLException {
        return impl.getScale(arg0);
    }

    public String getSchemaName(int arg0) throws SQLException {
        return impl.getSchemaName(arg0);
    }

    public String getTableName(int arg0) throws SQLException {
        return impl.getTableName(arg0);
    }

    public boolean isAutoIncrement(int arg0) throws SQLException {
        return impl.isAutoIncrement(arg0);
    }

    public boolean isCaseSensitive(int arg0) throws SQLException {
        return impl.isCaseSensitive(arg0);
    }

    public boolean isCurrency(int arg0) throws SQLException {
        return impl.isCurrency(arg0);
    }

    public boolean isDefinitelyWritable(int arg0) throws SQLException {
        return impl.isDefinitelyWritable(arg0);
    }

    public int isNullable(int arg0) throws SQLException {
        return impl.isNullable(arg0);
    }

    public boolean isReadOnly(int arg0) throws SQLException {
        return impl.isReadOnly(arg0);
    }

    public boolean isSearchable(int arg0) throws SQLException {
        return impl.isSearchable(arg0);
    }

    public boolean isSigned(int arg0) throws SQLException {
        return impl.isSigned(arg0);
    }

    public boolean isWritable(int arg0) throws SQLException {
        return impl.isWritable(arg0);
    }
}
