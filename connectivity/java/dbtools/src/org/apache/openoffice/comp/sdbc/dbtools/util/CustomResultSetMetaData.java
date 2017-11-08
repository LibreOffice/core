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
package org.apache.openoffice.comp.sdbc.dbtools.util;

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetMetaData;

public class CustomResultSetMetaData extends WeakBase implements XResultSetMetaData {
    private CustomColumn[] columns;

    public CustomResultSetMetaData(CustomColumn[] columns) {
        this.columns = columns;
    }

    private CustomColumn getColumn(int i) {
        if (i < 1 || columns.length < i) {
            return null;
        }
        return columns[i-1];
    }

    @Override
    public String getCatalogName(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getCatalogName();
        }
        return "";
    }

    @Override
    public int getColumnCount() throws SQLException {
        return columns.length;
    }

    @Override
    public int getColumnDisplaySize(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getColumnDisplaySize();
        }
        return 0;
    }

    @Override
    public String getColumnLabel(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getColumnLabel();
        }
        return getColumnName(column);
    }

    @Override
    public String getColumnName(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getColumnName();
        }
        return "";
    }

    @Override
    public String getColumnServiceName(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getColumnServiceName();
        }
        return "";
    }

    @Override
    public int getColumnType(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getColumnType();
        }
        return 1;
    }

    @Override
    public String getColumnTypeName(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getColumnTypeName();
        }
        return "";
    }

    @Override
    public int getPrecision(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getPrecision();
        }
        return 0;
    }

    @Override
    public int getScale(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getScale();
        }
        return 0;
    }

    @Override
    public String getSchemaName(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getSchemaName();
        }
        return "";
    }

    @Override
    public String getTableName(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getTableName();
        }
        return "";
    }

    @Override
    public boolean isAutoIncrement(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isAutoIncrement();
        }
        return false;
    }

    @Override
    public boolean isCaseSensitive(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isCaseSensitive();
        }
        return true;
    }

    @Override
    public boolean isCurrency(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isCurrency();
        }
        return false;
    }

    @Override
    public boolean isDefinitelyWritable(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isDefinitelyWritable();
        }
        return false;
    }

    @Override
    public int isNullable(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.getNullable();
        }
        return 0;
    }

    @Override
    public boolean isReadOnly(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isReadOnly();
        }
        return true;
    }

    @Override
    public boolean isSearchable(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isSearchable();
        }
        return true;
    }

    @Override
    public boolean isSigned(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isSigned();
        }
        return false;
    }

    @Override
    public boolean isWritable(int column) throws SQLException {
        CustomColumn customColumn = getColumn(column);
        if (customColumn != null) {
            return customColumn.isWritable();
        }
        return false;
    }
}
