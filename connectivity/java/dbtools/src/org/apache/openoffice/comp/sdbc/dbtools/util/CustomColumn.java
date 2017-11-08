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

public class CustomColumn {
    private String catalogName = "";
    private String schemaName = "";
    private String tableName = "";
    private String columnName = "";
    private String columnLabel = "";
    private String columnServiceName = "";

    private int columnType = 0;
    private String columnTypeName = "";

    private int nullable = 0;
    private int columnDisplaySize = 0;
    private int precision = 0;
    private int scale = 0;

    private boolean isAutoIncrement = false;
    private boolean isCaseSensitive = false;
    private boolean isSearchable = true;
    private boolean isCurrency = false;
    private boolean isSigned = false;
    private boolean isReadOnly = true;
    private boolean isWritable = false;
    private boolean isDefinitelyWritable = false;

    public String getCatalogName() {
        return catalogName;
    }
    public void setCatalogName(String catalogName) {
        this.catalogName = catalogName;
    }
    public String getSchemaName() {
        return schemaName;
    }
    public void setSchemaName(String schemaName) {
        this.schemaName = schemaName;
    }
    public String getTableName() {
        return tableName;
    }
    public void setTableName(String tableName) {
        this.tableName = tableName;
    }
    public String getColumnName() {
        return columnName;
    }
    public void setColumnName(String columnName) {
        this.columnName = columnName;
    }
    public String getColumnLabel() {
        return columnLabel;
    }
    public void setColumnLabel(String columnLabel) {
        this.columnLabel = columnLabel;
    }
    public String getColumnServiceName() {
        return columnServiceName;
    }
    public void setColumnServiceName(String columnServiceName) {
        this.columnServiceName = columnServiceName;
    }
    public int getColumnType() {
        return columnType;
    }
    public void setColumnType(int columnType) {
        this.columnType = columnType;
    }
    public String getColumnTypeName() {
        return columnTypeName;
    }
    public void setColumnTypeName(String columnTypeName) {
        this.columnTypeName = columnTypeName;
    }
    public int getNullable() {
        return nullable;
    }
    public void setNullable(int nullable) {
        this.nullable = nullable;
    }
    public int getColumnDisplaySize() {
        return columnDisplaySize;
    }
    public void setColumnDisplaySize(int columnDisplaySize) {
        this.columnDisplaySize = columnDisplaySize;
    }
    public int getPrecision() {
        return precision;
    }
    public void setPrecision(int precision) {
        this.precision = precision;
    }
    public int getScale() {
        return scale;
    }
    public void setScale(int scale) {
        this.scale = scale;
    }
    public boolean isAutoIncrement() {
        return isAutoIncrement;
    }
    public void setAutoIncrement(boolean isAutoIncrement) {
        this.isAutoIncrement = isAutoIncrement;
    }
    public boolean isCaseSensitive() {
        return isCaseSensitive;
    }
    public void setCaseSensitive(boolean isCaseSensitive) {
        this.isCaseSensitive = isCaseSensitive;
    }
    public boolean isSearchable() {
        return isSearchable;
    }
    public void setSearchable(boolean isSearchable) {
        this.isSearchable = isSearchable;
    }
    public boolean isCurrency() {
        return isCurrency;
    }
    public void setCurrency(boolean isCurrency) {
        this.isCurrency = isCurrency;
    }
    public boolean isSigned() {
        return isSigned;
    }
    public void setSigned(boolean isSigned) {
        this.isSigned = isSigned;
    }
    public boolean isReadOnly() {
        return isReadOnly;
    }
    public void setReadOnly(boolean isReadOnly) {
        this.isReadOnly = isReadOnly;
    }
    public boolean isWritable() {
        return isWritable;
    }
    public void setWritable(boolean isWritable) {
        this.isWritable = isWritable;
    }
    public boolean isDefinitelyWritable() {
        return isDefinitelyWritable;
    }
    public void setDefinitelyWritable(boolean isDefinitelyWritable) {
        this.isDefinitelyWritable = isDefinitelyWritable;
    }
}
