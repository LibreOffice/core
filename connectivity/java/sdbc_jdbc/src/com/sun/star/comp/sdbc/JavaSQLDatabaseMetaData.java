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

import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;

import org.apache.openoffice.comp.sdbc.dbtools.util.CustomColumn;
import org.apache.openoffice.comp.sdbc.dbtools.util.CustomResultSet;
import org.apache.openoffice.comp.sdbc.dbtools.util.CustomResultSetMetaData;
import org.apache.openoffice.comp.sdbc.dbtools.util.ORowSetValue;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDatabaseMetaData2;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XRow;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class JavaSQLDatabaseMetaData extends WeakBase implements XDatabaseMetaData2 {
    private java.sql.DatabaseMetaData jdbcDatabaseMetaData;
    private JavaSQLConnection connection;
    private ConnectionLog logger;

    public JavaSQLDatabaseMetaData(java.sql.DatabaseMetaData jdbcDatabaseMetaData, JavaSQLConnection connection) {
        this.jdbcDatabaseMetaData = jdbcDatabaseMetaData;
        this.connection = connection;
        this.logger = connection.getLogger();
    }

    @Override
    public XConnection getConnection() throws SQLException {
        return connection;
    }

    @Override
    public PropertyValue[] getConnectionInfo() {
        return connection.getConnectionInfo();
    }

    private XResultSet wrapResultSet(java.sql.ResultSet resultSet) {
        if (resultSet == null) {
            return null;
        }
        return new JavaSQLResultSet(resultSet, connection);
    }

    @Override
    public XResultSet getTypeInfo() throws SQLException {
        try {
            return wrapResultSet(jdbcDatabaseMetaData.getTypeInfo());
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getCatalogs() throws SQLException {
        try {
            return wrapResultSet(jdbcDatabaseMetaData.getCatalogs());
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getCatalogSeparator() throws SQLException {
        try {
            String catalogSeparator = jdbcDatabaseMetaData.getCatalogSeparator();
            if (catalogSeparator == null) {
                catalogSeparator = "";
            }
            return catalogSeparator;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getSchemas() throws SQLException {
        try {
            return wrapResultSet(jdbcDatabaseMetaData.getCatalogs());
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getColumnPrivileges(Object catalog, String schema, String table,
            String columnNamePattern) throws SQLException {

        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getColumnPrivileges(
                    jdbcCatalog, schema, table, columnNamePattern));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getColumns(Object catalog, String schemaPattern, String tableNamePattern, String columnNamePattern) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schemaPattern.equals("%")) {
                schemaPattern = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getColumns(
                    jdbcCatalog, schemaPattern, tableNamePattern, columnNamePattern));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getTables(Object catalog, String schemaPattern, String tableNamePattern, String[] types) throws SQLException {
        try {
            // the SDBC API allows to pass "%" as table type filter, but in JDBC, "all table types"
            // is represented by the table type being null
            String[] jdbcTypes = types;
            for (String type : types) {
                if (type.equals("%")) {
                    jdbcTypes = null;
                    break;
                }
            }
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            // if we are to display "all catalogs", then respect CatalogRestriction
            if (jdbcCatalog == null) {
                Object catalogRestriction = connection.getCatalogRestriction();
                if (AnyConverter.isString(catalogRestriction)) {
                    jdbcCatalog = AnyConverter.toString(catalogRestriction);
                }
            }
            if (schemaPattern.equals("%")) {
                Object schemaRestriction = connection.getSchemaRestriction();
                if (AnyConverter.isString(schemaRestriction)) {
                    schemaPattern = AnyConverter.toString(schemaRestriction);
                }
            }
            return wrapResultSet(jdbcDatabaseMetaData.getTables(
                    jdbcCatalog, schemaPattern, tableNamePattern, jdbcTypes));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getProcedureColumns(Object catalog, String schemaNamePattern, String procedureNamePattern,
            String columnNamePattern) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schemaNamePattern.equals("%")) {
                schemaNamePattern = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getProcedureColumns(
                    jdbcCatalog, schemaNamePattern, procedureNamePattern, columnNamePattern));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getProcedures(Object catalog, String schemaNamePattern, String procedureNamePattern) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schemaNamePattern.equals("%")) {
                schemaNamePattern = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getProcedures(
                    jdbcCatalog, schemaNamePattern, procedureNamePattern));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getVersionColumns(Object catalog, String schema, String table) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getVersionColumns(jdbcCatalog, schema, table));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public int getMaxBinaryLiteralLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxBinaryLiteralLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxRowSize() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxRowSize();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxCatalogNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxCatalogNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxCharLiteralLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxCharLiteralLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxColumnNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxColumnNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxColumnsInIndex() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxColumnsInIndex();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxCursorNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxCursorNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxConnections() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxConnections();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxColumnsInTable() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxColumnsInTable();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxStatementLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxStatementLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxTableNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxTableNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxTablesInSelect() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxTablesInSelect();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getExportedKeys(Object catalog, String schema, String table) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getExportedKeys(jdbcCatalog, schema, table));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getImportedKeys(Object catalog, String schema, String table) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getImportedKeys(jdbcCatalog, schema, table));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getPrimaryKeys(Object catalog, String schema, String table) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getPrimaryKeys(jdbcCatalog, schema, table));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getIndexInfo(Object catalog, String schema, String table,
            boolean unique, boolean approximate) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getIndexInfo(
                    jdbcCatalog, schema, table, unique, approximate));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getBestRowIdentifier(Object catalog, String schema, String table,
            int scope, boolean nullable) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schema.equals("%")) {
                schema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getBestRowIdentifier(
                    jdbcCatalog, schema, table, scope, nullable));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    private XResultSetMetaData getTablesPrivilegesMetadata() {
        CustomColumn[] columns = new CustomColumn[7];
        columns[0] = new CustomColumn();
        columns[0].setColumnName("TABLE_CAT");
        columns[0].setNullable(ColumnValue.NULLABLE);
        columns[0].setColumnDisplaySize(3);
        columns[0].setPrecision(0);
        columns[0].setScale(0);
        columns[0].setColumnType(DataType.VARCHAR);
        columns[1] = new CustomColumn();
        columns[1].setColumnName("TABLE_SCHEM");
        columns[1].setNullable(ColumnValue.NULLABLE);
        columns[1].setColumnDisplaySize(3);
        columns[1].setPrecision(0);
        columns[1].setScale(0);
        columns[1].setColumnType(DataType.VARCHAR);
        columns[2] = new CustomColumn();
        columns[2].setColumnName("TABLE_NAME");
        columns[2].setNullable(ColumnValue.NO_NULLS);
        columns[2].setColumnDisplaySize(3);
        columns[2].setPrecision(0);
        columns[2].setScale(0);
        columns[2].setColumnType(DataType.VARCHAR);
        columns[3] = new CustomColumn();
        columns[3].setColumnName("GRANTOR");
        columns[3].setNullable(ColumnValue.NULLABLE);
        columns[3].setColumnDisplaySize(0);
        columns[3].setPrecision(0);
        columns[3].setScale(0);
        columns[3].setColumnType(DataType.VARCHAR);
        columns[4] = new CustomColumn();
        columns[4].setColumnName("GRANTEE");
        columns[4].setNullable(ColumnValue.NO_NULLS);
        columns[4].setColumnDisplaySize(0);
        columns[4].setPrecision(0);
        columns[4].setScale(0);
        columns[4].setColumnType(DataType.VARCHAR);
        columns[5] = new CustomColumn();
        columns[5].setColumnName("PRIVILEGE");
        columns[5].setNullable(ColumnValue.NULLABLE);
        columns[5].setColumnDisplaySize(0);
        columns[5].setPrecision(0);
        columns[5].setScale(0);
        columns[5].setColumnType(DataType.VARCHAR);
        columns[6] = new CustomColumn();
        columns[6].setColumnName("IS_GRANTABLE");
        columns[6].setNullable(ColumnValue.NULLABLE);
        columns[6].setColumnDisplaySize(0);
        columns[6].setPrecision(0);
        columns[6].setScale(0);
        columns[6].setColumnType(DataType.VARCHAR);
        return new CustomResultSetMetaData(columns);
    }

    private XResultSet generateOwnTablePrivileges(Object catalog, String schemaPattern,
            String tableNamePattern) throws SQLException {

        XResultSetMetaData resultSetMetaData = getTablesPrivilegesMetadata();

        ArrayList<ORowSetValue[]> privileges = new ArrayList<>();
        String[] privilegeTypes = {
                "SELECT", "INSERT", "DELETE", "UPDATE", "CREATE",
                "READ", "ALTER", "DROP", "REFERENCE"
        };
        XResultSet tables = getTables(catalog, schemaPattern, tableNamePattern,
                new String[] {"VIEW", "TABLE", "%"});
        String username = getUserName();
        XRow row = UnoRuntime.queryInterface(XRow.class, tables);
        while (tables.next()) {
            for (String privilegeType : privilegeTypes) {
                ORowSetValue[] privilege = new ORowSetValue[7];
                privilege[0] = new ORowSetValue(row.getString(1));
                privilege[1] = new ORowSetValue(row.getString(2));
                privilege[2] = new ORowSetValue(row.getString(3));
                privilege[3] = new ORowSetValue("");
                privilege[3].setNull();
                privilege[4] = new ORowSetValue(username);
                privilege[5] = new ORowSetValue(privilegeType);
                privilege[6] = new ORowSetValue("YES");
                privileges.add(privilege);
            }
        }
        return new CustomResultSet(resultSetMetaData, privileges);
    }

    @Override
    public XResultSet getTablePrivileges(Object catalog, String schemaPattern, String tableNamePattern) throws SQLException {
        try {
            if (connection.isIgnoreDriverPrivilegesEnabled()) {
                return generateOwnTablePrivileges(catalog, schemaPattern, tableNamePattern);
            }
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schemaPattern.equals("%")) {
                schemaPattern = null;
            }
            java.sql.ResultSet javaResults = jdbcDatabaseMetaData.getTablePrivileges(jdbcCatalog, schemaPattern, tableNamePattern);
            if (javaResults != null) {
                XResultSet results = new JavaSQLResultSet(javaResults, connection);
                // we have to check the result columns for the tables privileges
                // #106324#
                XResultSetMetaDataSupplier metaDataSupplier = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, results);
                XResultSetMetaData metaData = null;
                if (metaDataSupplier != null) {
                    metaData = metaDataSupplier.getMetaData();
                }
                if (metaData != null && metaData.getColumnCount() != 7) {
                    // here we know that the count of column doesn't match
                    Map<Integer,Integer> columnMatching = new TreeMap<>();
                    String[] privileges = new String[] {
                            "TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME", "GRANTOR", "GRANTEE", "PRIVILEGE", "IS_GRANTABLE"
                    };
                    int count = metaData.getColumnCount();
                    for (int i = 1; i <= count; i++) {
                        String columnName = metaData.getColumnName(i);
                        for (int j = 0; j < privileges.length; j++) {
                            if (columnName.equals(privileges[j])) {
                                columnMatching.put(i, j);
                                break;
                            }
                        }
                    }
                    // fill our own resultset
                    ArrayList<ORowSetValue[]> rowsOut = new ArrayList<>();
                    XRow row = UnoRuntime.queryInterface(XRow.class, results);
                    while (results.next()) {
                        ORowSetValue[] rowOut = new ORowSetValue[7];
                        for (int i = 0; i < rowOut.length; i++) {
                            rowOut[i] = new ORowSetValue("");
                            rowOut[i].setNull();
                        }
                        for (Map.Entry<Integer,Integer> entry : columnMatching.entrySet()) {
                            String value = row.getString(entry.getKey());
                            if (row.wasNull()) {
                                rowOut[entry.getValue()].setNull();
                            } else {
                                rowOut[entry.getValue()].setString(value);
                            }
                        }
                        rowsOut.add(rowOut);
                    }
                    results = new CustomResultSet(getTablesPrivilegesMetadata(), rowsOut);
                }
                return results;
            } else {
                return null;
            }
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public XResultSet getCrossReference(Object primaryCatalog, String primarySchema, String primaryTable,
            Object foreignCatalog, String foreignSchema, String foreignTable) throws SQLException {
        try {
            String jdbcPrimaryCatalog = null;
            if (AnyConverter.isString(primaryCatalog)) {
                jdbcPrimaryCatalog = AnyConverter.toString(primaryCatalog);
            }
            String jdbcForeignCatalog = null;
            if (AnyConverter.isString(foreignCatalog)) {
                jdbcForeignCatalog = AnyConverter.toString(foreignCatalog);
            }
            if (primarySchema.equals("%")) {
                primarySchema = null;
            }
            if (foreignSchema.equals("%")) {
                foreignSchema = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getCrossReference(
                    jdbcPrimaryCatalog, primarySchema, primaryTable,
                    jdbcForeignCatalog, foreignSchema, foreignTable));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }

    @Override
    public boolean doesMaxRowSizeIncludeBlobs() throws SQLException {
        try {
            return jdbcDatabaseMetaData.doesMaxRowSizeIncludeBlobs();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean storesLowerCaseQuotedIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.storesLowerCaseQuotedIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean storesLowerCaseIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.storesLowerCaseIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean storesMixedCaseIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.storesMixedCaseIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean storesMixedCaseQuotedIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.storesMixedCaseQuotedIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean storesUpperCaseQuotedIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.storesUpperCaseQuotedIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean storesUpperCaseIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.storesUpperCaseIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsAlterTableWithAddColumn() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsAlterTableWithAddColumn();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsAlterTableWithDropColumn() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsAlterTableWithDropColumn();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxIndexLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxIndexLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsNonNullableColumns() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsNonNullableColumns();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getCatalogTerm() throws SQLException {
        try {
            String catalogTerm = jdbcDatabaseMetaData.getCatalogTerm();
            if (catalogTerm == null) {
                catalogTerm = "";
            }
            return catalogTerm;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getIdentifierQuoteString() throws SQLException {
        try {
            String identifierQuoteString = jdbcDatabaseMetaData.getIdentifierQuoteString();
            if (identifierQuoteString == null) {
                identifierQuoteString = "";
            }
            return identifierQuoteString;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getExtraNameCharacters() throws SQLException {
        try {
            String extraNameCharacters = jdbcDatabaseMetaData.getExtraNameCharacters();
            if (extraNameCharacters == null) {
                extraNameCharacters = "";
            }
            return extraNameCharacters;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsDifferentTableCorrelationNames() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsDifferentTableCorrelationNames();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isCatalogAtStart() throws SQLException {
        try {
            return jdbcDatabaseMetaData.isCatalogAtStart();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean dataDefinitionIgnoredInTransactions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.dataDefinitionIgnoredInTransactions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean dataDefinitionCausesTransactionCommit() throws SQLException {
        try {
            return jdbcDatabaseMetaData.dataDefinitionCausesTransactionCommit();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsDataManipulationTransactionsOnly() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsDataDefinitionAndDataManipulationTransactions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsDataDefinitionAndDataManipulationTransactions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsDataDefinitionAndDataManipulationTransactions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsPositionedDelete() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsPositionedDelete();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsPositionedUpdate() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsPositionedUpdate();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsOpenStatementsAcrossRollback() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsOpenStatementsAcrossRollback();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsOpenStatementsAcrossCommit() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsOpenStatementsAcrossCommit();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsOpenCursorsAcrossCommit() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsOpenCursorsAcrossCommit();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsOpenCursorsAcrossRollback() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsOpenCursorsAcrossRollback();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsTransactionIsolationLevel(int level) throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsTransactionIsolationLevel(level);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSchemasInDataManipulation() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSchemasInDataManipulation();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsANSI92FullSQL() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsANSI92FullSQL();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsANSI92EntryLevelSQL() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsANSI92EntryLevelSQL();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsIntegrityEnhancementFacility() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsIntegrityEnhancementFacility();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSchemasInIndexDefinitions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSchemasInIndexDefinitions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCatalogsInIndexDefinitions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCatalogsInIndexDefinitions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCatalogsInDataManipulation() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCatalogsInDataManipulation();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCatalogsInTableDefinitions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCatalogsInTableDefinitions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSchemasInTableDefinitions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSchemasInTableDefinitions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsOuterJoins() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsOuterJoins();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getTableTypes() throws SQLException {
        try {
            return new JavaSQLResultSet(jdbcDatabaseMetaData.getTableTypes(), connection);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxStatements() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxStatements();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxProcedureNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxProcedureNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxSchemaNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxSchemaNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsTransactions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsTransactions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean allProceduresAreCallable() throws SQLException {
        try {
            return jdbcDatabaseMetaData.allProceduresAreCallable();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsStoredProcedures() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsStoredProcedures();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSelectForUpdate() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSelectForUpdate();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean allTablesAreSelectable() throws SQLException {
        try {
            return jdbcDatabaseMetaData.allTablesAreSelectable();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean isReadOnly() throws SQLException {
        try {
            return jdbcDatabaseMetaData.isReadOnly();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean usesLocalFiles() throws SQLException {
        try {
            return jdbcDatabaseMetaData.usesLocalFiles();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean usesLocalFilePerTable() throws SQLException {
        try {
            return jdbcDatabaseMetaData.usesLocalFilePerTable();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsTypeConversion() throws SQLException {
        // This was calling JDBC's supportsTypeConversion(), which doesn't exist
        try {
            return jdbcDatabaseMetaData.supportsConvert();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean nullPlusNonNullIsNull() throws SQLException {
        try {
            return jdbcDatabaseMetaData.nullPlusNonNullIsNull();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsColumnAliasing() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsColumnAliasing();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsTableCorrelationNames() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsTableCorrelationNames();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsConvert(int fromType, int toType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsConvert(fromType, toType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsExpressionsInOrderBy() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsExpressionsInOrderBy();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsGroupBy() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsGroupBy();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsGroupByBeyondSelect() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsGroupByBeyondSelect();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsGroupByUnrelated() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsGroupByUnrelated();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsMultipleTransactions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsMultipleTransactions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsMultipleResultSets() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsMultipleResultSets();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsLikeEscapeClause() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsLikeEscapeClause();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsOrderByUnrelated() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsOrderByUnrelated();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsUnion() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsUnion();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsUnionAll() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsUnionAll();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsMixedCaseIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsMixedCaseIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsMixedCaseQuotedIdentifiers() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsMixedCaseQuotedIdentifiers();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean nullsAreSortedAtEnd() throws SQLException {
        try {
            return jdbcDatabaseMetaData.nullsAreSortedAtEnd();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean nullsAreSortedAtStart() throws SQLException {
        try {
            return jdbcDatabaseMetaData.nullsAreSortedAtStart();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean nullsAreSortedHigh() throws SQLException {
        try {
            return jdbcDatabaseMetaData.nullsAreSortedHigh();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean nullsAreSortedLow() throws SQLException {
        try {
            return jdbcDatabaseMetaData.nullsAreSortedLow();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCatalogsInProcedureCalls() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCatalogsInProcedureCalls();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSchemasInProcedureCalls() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSchemasInProcedureCalls();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCatalogsInPrivilegeDefinitions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCatalogsInPrivilegeDefinitions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSchemasInPrivilegeDefinitions() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSchemasInPrivilegeDefinitions();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCorrelatedSubqueries() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCorrelatedSubqueries();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSubqueriesInComparisons() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSubqueriesInComparisons();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSubqueriesInExists() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSubqueriesInExists();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSubqueriesInIns() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSubqueriesInIns();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsSubqueriesInQuantifieds() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsSubqueriesInQuantifieds();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsANSI92IntermediateSQL() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsANSI92IntermediateSQL();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getURL() throws SQLException {
        String url = connection.getURL();
        if (url == null) {
            try {
                url = jdbcDatabaseMetaData.getURL();
            } catch (java.sql.SQLException jdbcSQLException) {
                throw Tools.toUnoException(this, jdbcSQLException);
            }
        }
        return url;
    }

    @Override
    public String getUserName() throws SQLException {
        try {
            String username = jdbcDatabaseMetaData.getUserName();
            if (username == null) {
                username = "";
            }
            return username;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getDriverName() throws SQLException {
        try {
            String driverName = jdbcDatabaseMetaData.getDriverName();
            if (driverName == null) {
                driverName = "";
            }
            return driverName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getDriverVersion() throws SQLException {
        try {
            String driverVersion = jdbcDatabaseMetaData.getDriverVersion();
            if (driverVersion == null) {
                driverVersion = "";
            }
            return driverVersion;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getDatabaseProductVersion() throws SQLException {
        try {
            String databaseProductVersion = jdbcDatabaseMetaData.getDatabaseProductVersion();
            if (databaseProductVersion == null) {
                databaseProductVersion = "";
            }
            return databaseProductVersion;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getDatabaseProductName() throws SQLException {
        try {
            String databaseProductName = jdbcDatabaseMetaData.getDatabaseProductName();
            if (databaseProductName == null) {
                databaseProductName = "";
            }
            return databaseProductName;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getProcedureTerm() throws SQLException {
        try {
            String procedureTerm = jdbcDatabaseMetaData.getProcedureTerm();
            if (procedureTerm == null) {
                procedureTerm = "";
            }
            return procedureTerm;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getSchemaTerm() throws SQLException {
        try {
            String schemaTerm = jdbcDatabaseMetaData.getSchemaTerm();
            if (schemaTerm == null) {
                schemaTerm = "";
            }
            return schemaTerm;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getDriverMajorVersion() {
        return jdbcDatabaseMetaData.getDriverMajorVersion();
    }

    @Override
    public int getDriverMinorVersion() {
        return jdbcDatabaseMetaData.getDriverMinorVersion();
    }

    @Override
    public int getDefaultTransactionIsolation() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getDefaultTransactionIsolation();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getSQLKeywords() throws SQLException {
        try {
            String sqlKeywords = jdbcDatabaseMetaData.getSQLKeywords();
            if (sqlKeywords == null) {
                sqlKeywords = "";
            }
            return sqlKeywords;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getSearchStringEscape() throws SQLException {
        try {
            String searchStringEscape = jdbcDatabaseMetaData.getSearchStringEscape();
            if (searchStringEscape == null) {
                searchStringEscape = "";
            }
            return searchStringEscape;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getStringFunctions() throws SQLException {
        try {
            String stringFunctions = jdbcDatabaseMetaData.getStringFunctions();
            if (stringFunctions == null) {
                stringFunctions = "";
            }
            return stringFunctions;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getTimeDateFunctions() throws SQLException {
        try {
            String timeDateFunctions = jdbcDatabaseMetaData.getTimeDateFunctions();
            if (timeDateFunctions == null) {
                timeDateFunctions = "";
            }
            return timeDateFunctions;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getSystemFunctions() throws SQLException {
        try {
            String systemFunctions = jdbcDatabaseMetaData.getSystemFunctions();
            if (systemFunctions == null) {
                systemFunctions = "";
            }
            return systemFunctions;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getNumericFunctions() throws SQLException {
        try {
            String numericFunctions = jdbcDatabaseMetaData.getNumericFunctions();
            if (numericFunctions == null) {
                numericFunctions = "";
            }
            return numericFunctions;
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsExtendedSQLGrammar() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsExtendedSQLGrammar();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsCoreSQLGrammar() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsCoreSQLGrammar();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    public boolean supportsMinimumSQLGrammar() {
        try {
            return jdbcDatabaseMetaData.supportsMinimumSQLGrammar();
        } catch (java.sql.SQLException jdbcSQLException) {
            // FIXME: SDBC bug. Must be able to throw exception.
            //throw Tools.toUnoException(this, jdbcSQLException);
            return false;
        }
    }

    @Override
    public boolean supportsFullOuterJoins() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsFullOuterJoins();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsLimitedOuterJoins() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsLimitedOuterJoins();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxColumnsInGroupBy() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxColumnsInGroupBy();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxColumnsInOrderBy() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxColumnsInOrderBy();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxColumnsInSelect() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxColumnsInSelect();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public int getMaxUserNameLength() throws SQLException {
        try {
            return jdbcDatabaseMetaData.getMaxUserNameLength();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsResultSetType(int type) throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsResultSetType(type);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsResultSetConcurrency(int setType, int concurrency) throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsResultSetConcurrency(setType, concurrency);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean ownUpdatesAreVisible(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.ownUpdatesAreVisible(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean ownDeletesAreVisible(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.ownDeletesAreVisible(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean ownInsertsAreVisible(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.ownInsertsAreVisible(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean othersUpdatesAreVisible(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.othersUpdatesAreVisible(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean othersDeletesAreVisible(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.othersDeletesAreVisible(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean othersInsertsAreVisible(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.othersInsertsAreVisible(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean updatesAreDetected(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.updatesAreDetected(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean deletesAreDetected(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.deletesAreDetected(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean insertsAreDetected(int setType) throws SQLException {
        try {
            return jdbcDatabaseMetaData.insertsAreDetected(setType);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public boolean supportsBatchUpdates() throws SQLException {
        try {
            return jdbcDatabaseMetaData.supportsBatchUpdates();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getUDTs(Object catalog, String schemaPattern,
            String typeNamePattern, int[] types) throws SQLException {
        try {
            String jdbcCatalog = null;
            if (AnyConverter.isString(catalog)) {
                jdbcCatalog = AnyConverter.toString(catalog);
            }
            if (schemaPattern.equals("%")) {
                schemaPattern = null;
            }
            return wrapResultSet(jdbcDatabaseMetaData.getUDTs(jdbcCatalog, schemaPattern, typeNamePattern, types));
        } catch (java.sql.SQLException | IllegalArgumentException exception) {
            throw Tools.toUnoException(this, exception);
        }
    }
}
