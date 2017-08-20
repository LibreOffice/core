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

import java.util.ArrayList;

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbcx.comp.postgresql.util.DatabaseMetaDataResultSet;
import com.sun.star.sdbcx.comp.postgresql.util.ORowSetValue;
import com.sun.star.uno.UnoRuntime;

public class PostgresqlDatabaseMetadata extends WeakBase implements XDatabaseMetaData {
    private XDatabaseMetaData impl;
    private XConnection connection;
    private String url;

    public PostgresqlDatabaseMetadata(XDatabaseMetaData impl, XConnection connection, String url) {
        this.impl = impl;
        this.connection = connection;
        this.url = url;
    }

    public boolean allProceduresAreCallable() throws SQLException {
        return impl.allProceduresAreCallable();
    }

    public boolean allTablesAreSelectable() throws SQLException {
        return impl.allTablesAreSelectable();
    }

    public boolean dataDefinitionCausesTransactionCommit() throws SQLException {
        return impl.dataDefinitionCausesTransactionCommit();
    }

    public boolean dataDefinitionIgnoredInTransactions() throws SQLException {
        return impl.dataDefinitionIgnoredInTransactions();
    }

    public boolean deletesAreDetected(int arg0) throws SQLException {
        return impl.deletesAreDetected(arg0);
    }

    public boolean doesMaxRowSizeIncludeBlobs() throws SQLException {
        return impl.doesMaxRowSizeIncludeBlobs();
    }

    public XResultSet getBestRowIdentifier(Object arg0, String arg1, String arg2, int arg3, boolean arg4) throws SQLException {
        return new PostgresqlResultSet(impl.getBestRowIdentifier(arg0, arg1, arg2, arg3, arg4), null);
    }

    public String getCatalogSeparator() throws SQLException {
        return impl.getCatalogSeparator();
    }

    public String getCatalogTerm() throws SQLException {
        return impl.getCatalogTerm();
    }

    public XResultSet getCatalogs() throws SQLException {
        return new PostgresqlResultSet(impl.getCatalogs(), null);
    }

    public XResultSet getColumnPrivileges(Object arg0, String arg1, String arg2, String arg3) throws SQLException {
        return new PostgresqlResultSet(impl.getColumnPrivileges(arg0, arg1, arg2, arg3), null);
    }

    public XResultSet getColumns(Object arg0, String arg1, String arg2, String arg3) throws SQLException {
        XResultSet results = impl.getColumns(arg0, arg1, arg2, arg3);
        XRow row = UnoRuntime.queryInterface(XRow.class, results);
        ArrayList<ORowSetValue[]> table = new ArrayList<>();
        while (results.next()) {
            String tableCat = row.getString(1);
            String tableSchem = row.getString(2);
            String tableName = row.getString(3);
            String columnName = row.getString(4);
            short dataType = row.getShort(5);
            String typeName = row.getString(6);
            int columnSize = row.getInt(7);
            int bufferLength = row.getInt(8); // FIXME: is it int?
            int decimalDigits = row.getInt(9);
            int numPrecRadix = row.getInt(10);
            int nullable = row.getInt(11);
            String remarks = row.getString(12);
            String columnDef = row.getString(13);
            int sqlDataType = row.getInt(14);
            int sqlDateTimeSub = row.getInt(15);
            int charOctetLength = row.getInt(16);
            int ordinalPosition = row.getInt(17);
            String isNullable = row.getString(18);

            if (dataType == DataType.BIT) {
                if (typeName.equals("bool")) {
                    dataType = DataType.BOOLEAN;
                }
            }

            ORowSetValue[] rowOut = new ORowSetValue[18];
            rowOut[0] = new ORowSetValue(tableCat);
            rowOut[1] = new ORowSetValue(tableSchem);
            rowOut[2] = new ORowSetValue(tableName);
            rowOut[3] = new ORowSetValue(columnName);
            rowOut[4] = new ORowSetValue(dataType);
            rowOut[5] = new ORowSetValue(typeName);
            rowOut[6] = new ORowSetValue(columnSize);
            rowOut[7] = new ORowSetValue(bufferLength);
            rowOut[8] = new ORowSetValue(decimalDigits);
            rowOut[9] = new ORowSetValue(numPrecRadix);
            rowOut[10] = new ORowSetValue(nullable);
            rowOut[11] = new ORowSetValue(remarks);
            rowOut[12] = new ORowSetValue(columnDef);
            rowOut[13] = new ORowSetValue(sqlDataType);
            rowOut[14] = new ORowSetValue(sqlDateTimeSub);
            rowOut[15] = new ORowSetValue(charOctetLength);
            rowOut[16] = new ORowSetValue(ordinalPosition);
            rowOut[17] = new ORowSetValue(isNullable);
            table.add(rowOut);
        }
        return new DatabaseMetaDataResultSet(results, table);
    }

    public XConnection getConnection() throws SQLException {
        return connection;
    }

    public XResultSet getCrossReference(Object arg0, String arg1, String arg2, Object arg3, String arg4, String arg5) throws SQLException {
        return new PostgresqlResultSet(impl.getCrossReference(arg0, arg1, arg2, arg3, arg4, arg5), null);
    }

    public String getDatabaseProductName() throws SQLException {
        return impl.getDatabaseProductName();
    }

    public String getDatabaseProductVersion() throws SQLException {
        return impl.getDatabaseProductVersion();
    }

    public int getDefaultTransactionIsolation() throws SQLException {
        return impl.getDefaultTransactionIsolation();
    }

    public int getDriverMajorVersion() {
        return impl.getDriverMajorVersion();
    }

    public int getDriverMinorVersion() {
        return impl.getDriverMinorVersion();
    }

    public String getDriverName() throws SQLException {
        return impl.getDriverName();
    }

    public String getDriverVersion() throws SQLException {
        return impl.getDriverVersion();
    }

    public XResultSet getExportedKeys(Object arg0, String arg1, String arg2) throws SQLException {
        return new PostgresqlResultSet(impl.getExportedKeys(arg0, arg1, arg2), null);
    }

    public String getExtraNameCharacters() throws SQLException {
        return impl.getExtraNameCharacters();
    }

    public String getIdentifierQuoteString() throws SQLException {
        return impl.getIdentifierQuoteString();
    }

    public XResultSet getImportedKeys(Object arg0, String arg1, String arg2) throws SQLException {
        return new PostgresqlResultSet(impl.getImportedKeys(arg0, arg1, arg2), null);
    }

    public XResultSet getIndexInfo(Object arg0, String arg1, String arg2, boolean arg3, boolean arg4) throws SQLException {
        return new PostgresqlResultSet(impl.getIndexInfo(arg0, arg1, arg2, arg3, arg4), null);
    }

    public int getMaxBinaryLiteralLength() throws SQLException {
        return impl.getMaxBinaryLiteralLength();
    }

    public int getMaxCatalogNameLength() throws SQLException {
        return impl.getMaxCatalogNameLength();
    }

    public int getMaxCharLiteralLength() throws SQLException {
        return impl.getMaxCharLiteralLength();
    }

    public int getMaxColumnNameLength() throws SQLException {
        return impl.getMaxColumnNameLength();
    }

    public int getMaxColumnsInGroupBy() throws SQLException {
        return impl.getMaxColumnsInGroupBy();
    }

    public int getMaxColumnsInIndex() throws SQLException {
        return impl.getMaxColumnsInIndex();
    }

    public int getMaxColumnsInOrderBy() throws SQLException {
        return impl.getMaxColumnsInOrderBy();
    }

    public int getMaxColumnsInSelect() throws SQLException {
        return impl.getMaxColumnsInSelect();
    }

    public int getMaxColumnsInTable() throws SQLException {
        return impl.getMaxColumnsInTable();
    }

    public int getMaxConnections() throws SQLException {
        return impl.getMaxConnections();
    }

    public int getMaxCursorNameLength() throws SQLException {
        return impl.getMaxCursorNameLength();
    }

    public int getMaxIndexLength() throws SQLException {
        return impl.getMaxIndexLength();
    }

    public int getMaxProcedureNameLength() throws SQLException {
        return impl.getMaxProcedureNameLength();
    }

    public int getMaxRowSize() throws SQLException {
        return impl.getMaxRowSize();
    }

    public int getMaxSchemaNameLength() throws SQLException {
        return impl.getMaxSchemaNameLength();
    }

    public int getMaxStatementLength() throws SQLException {
        return impl.getMaxStatementLength();
    }

    public int getMaxStatements() throws SQLException {
        return impl.getMaxStatements();
    }

    public int getMaxTableNameLength() throws SQLException {
        return impl.getMaxTableNameLength();
    }

    public int getMaxTablesInSelect() throws SQLException {
        return impl.getMaxTablesInSelect();
    }

    public int getMaxUserNameLength() throws SQLException {
        return impl.getMaxUserNameLength();
    }

    public String getNumericFunctions() throws SQLException {
        return impl.getNumericFunctions();
    }

    public XResultSet getPrimaryKeys(Object arg0, String arg1, String arg2) throws SQLException {
        return new PostgresqlResultSet(impl.getPrimaryKeys(arg0, arg1, arg2), null);
    }

    public XResultSet getProcedureColumns(Object arg0, String arg1, String arg2, String arg3) throws SQLException {
        return new PostgresqlResultSet(impl.getProcedureColumns(arg0, arg1, arg2, arg3), null);
    }

    public String getProcedureTerm() throws SQLException {
        return impl.getProcedureTerm();
    }

    public XResultSet getProcedures(Object arg0, String arg1, String arg2) throws SQLException {
        return new PostgresqlResultSet(impl.getProcedures(arg0, arg1, arg2), null);
    }

    public String getSQLKeywords() throws SQLException {
        return impl.getSQLKeywords();
    }

    public String getSchemaTerm() throws SQLException {
        return impl.getSchemaTerm();
    }

    public XResultSet getSchemas() throws SQLException {
        return new PostgresqlResultSet(impl.getSchemas(), null);
    }

    public String getSearchStringEscape() throws SQLException {
        return impl.getSearchStringEscape();
    }

    public String getStringFunctions() throws SQLException {
        return impl.getStringFunctions();
    }

    public String getSystemFunctions() throws SQLException {
        return impl.getSystemFunctions();
    }

    public XResultSet getTablePrivileges(Object arg0, String arg1, String arg2) throws SQLException {
        return new PostgresqlResultSet(impl.getTablePrivileges(arg0, arg1, arg2), null);
    }

    public XResultSet getTableTypes() throws SQLException {
        return new PostgresqlResultSet(impl.getTableTypes(), null);
    }

    public XResultSet getTables(Object arg0, String arg1, String arg2, String[] arg3) throws SQLException {
        return new PostgresqlResultSet(impl.getTables(arg0, arg1, arg2, arg3), null);
    }

    public String getTimeDateFunctions() throws SQLException {
        return impl.getTimeDateFunctions();
    }

    public XResultSet getTypeInfo() throws SQLException {
        XResultSet results = impl.getTypeInfo();
        XRow row = UnoRuntime.queryInterface(XRow.class, results);
        ArrayList<ORowSetValue[]> table = new ArrayList<>();
        while (results.next()) {
            String typeName = row.getString(1);
            short dataType = row.getShort(2);
            int precision = row.getInt(3);
            String literalPrefix = row.getString(4);
            String literalSuffix = row.getString(5);
            String createParams = row.getString(6);
            short nullable = row.getShort(7);
            boolean caseSensitive = row.getBoolean(8);
            short searchable = row.getShort(9);
            boolean unsignedAttribute = row.getBoolean(10);
            boolean fixedPrecScale = row.getBoolean(11);
            boolean autoIncrement = row.getBoolean(12);
            String localTypeName = row.getString(13);
            short minimumScale = row.getShort(14);
            short maximumScale = row.getShort(15);
            int sqlDataType = row.getInt(16);
            int sqlDateTimeSub = row.getInt(17);
            int numPrecRadix = row.getInt(18);

            if (dataType == DataType.BIT) {
                if (typeName.equals("bit")) {
                    // but the editor sees multi-bit columns as single bit
                    // and single bit can't be edited either: syntax error
                    createParams = "length";
                } else if (typeName.equals("bool")) {
                    dataType = DataType.BOOLEAN;
                }
            }
            if ((dataType == DataType.CHAR || dataType == DataType.VARCHAR)) {
                precision = 10485760;
                createParams = "length";
            }

            ORowSetValue[] rowOut = new ORowSetValue[18];
            rowOut[0] = new ORowSetValue(typeName);
            rowOut[1] = new ORowSetValue(dataType);
            rowOut[2] = new ORowSetValue(precision);
            rowOut[3] = new ORowSetValue(literalPrefix);
            rowOut[4] = new ORowSetValue(literalSuffix);
            rowOut[5] = new ORowSetValue(createParams);
            rowOut[6] = new ORowSetValue(nullable);
            rowOut[7] = new ORowSetValue(caseSensitive);
            rowOut[8] = new ORowSetValue(searchable);
            rowOut[9] = new ORowSetValue(unsignedAttribute);
            rowOut[10] = new ORowSetValue(fixedPrecScale);
            rowOut[11] = new ORowSetValue(autoIncrement);
            rowOut[12] = new ORowSetValue(localTypeName);
            rowOut[13] = new ORowSetValue(minimumScale);
            rowOut[14] = new ORowSetValue(maximumScale);
            rowOut[15] = new ORowSetValue(sqlDataType);
            rowOut[16] = new ORowSetValue(sqlDateTimeSub);
            rowOut[17] = new ORowSetValue(numPrecRadix);
            table.add(rowOut);
            //System.out.println(String.format("type %s, data type %d, SQL type %d, precision %d, createParams %s", typeName, dataType, sqlDataType, precision, createParams));
        }
        return new DatabaseMetaDataResultSet(results, table);
    }

    public XResultSet getUDTs(Object arg0, String arg1, String arg2, int[] arg3) throws SQLException {
        return new PostgresqlResultSet(impl.getUDTs(arg0, arg1, arg2, arg3), null);
    }

    public String getURL() throws SQLException {
        return url;
    }

    public String getUserName() throws SQLException {
        return impl.getUserName();
    }

    public XResultSet getVersionColumns(Object arg0, String arg1, String arg2) throws SQLException {
        return new PostgresqlResultSet(impl.getVersionColumns(arg0, arg1, arg2), null);
    }

    public boolean insertsAreDetected(int arg0) throws SQLException {
        return impl.insertsAreDetected(arg0);
    }

    public boolean isCatalogAtStart() throws SQLException {
        return impl.isCatalogAtStart();
    }

    public boolean isReadOnly() throws SQLException {
        return impl.isReadOnly();
    }

    public boolean nullPlusNonNullIsNull() throws SQLException {
        return impl.nullPlusNonNullIsNull();
    }

    public boolean nullsAreSortedAtEnd() throws SQLException {
        return impl.nullsAreSortedAtEnd();
    }

    public boolean nullsAreSortedAtStart() throws SQLException {
        return impl.nullsAreSortedAtStart();
    }

    public boolean nullsAreSortedHigh() throws SQLException {
        return impl.nullsAreSortedHigh();
    }

    public boolean nullsAreSortedLow() throws SQLException {
        return impl.nullsAreSortedLow();
    }

    public boolean othersDeletesAreVisible(int arg0) throws SQLException {
        return impl.othersDeletesAreVisible(arg0);
    }

    public boolean othersInsertsAreVisible(int arg0) throws SQLException {
        return impl.othersInsertsAreVisible(arg0);
    }

    public boolean othersUpdatesAreVisible(int arg0) throws SQLException {
        return impl.othersUpdatesAreVisible(arg0);
    }

    public boolean ownDeletesAreVisible(int arg0) throws SQLException {
        return impl.ownDeletesAreVisible(arg0);
    }

    public boolean ownInsertsAreVisible(int arg0) throws SQLException {
        return impl.ownInsertsAreVisible(arg0);
    }

    public boolean ownUpdatesAreVisible(int arg0) throws SQLException {
        return impl.ownUpdatesAreVisible(arg0);
    }

    public boolean storesLowerCaseIdentifiers() throws SQLException {
        return impl.storesLowerCaseIdentifiers();
    }

    public boolean storesLowerCaseQuotedIdentifiers() throws SQLException {
        return impl.storesLowerCaseQuotedIdentifiers();
    }

    public boolean storesMixedCaseIdentifiers() throws SQLException {
        return impl.storesMixedCaseIdentifiers();
    }

    public boolean storesMixedCaseQuotedIdentifiers() throws SQLException {
        return impl.storesMixedCaseQuotedIdentifiers();
    }

    public boolean storesUpperCaseIdentifiers() throws SQLException {
        return impl.storesUpperCaseIdentifiers();
    }

    public boolean storesUpperCaseQuotedIdentifiers() throws SQLException {
        return impl.storesUpperCaseQuotedIdentifiers();
    }

    public boolean supportsANSI92EntryLevelSQL() throws SQLException {
        return impl.supportsANSI92EntryLevelSQL();
    }

    public boolean supportsANSI92FullSQL() throws SQLException {
        return impl.supportsANSI92FullSQL();
    }

    public boolean supportsANSI92IntermediateSQL() throws SQLException {
        return impl.supportsANSI92IntermediateSQL();
    }

    public boolean supportsAlterTableWithAddColumn() throws SQLException {
        return impl.supportsAlterTableWithAddColumn();
    }

    public boolean supportsAlterTableWithDropColumn() throws SQLException {
        return impl.supportsAlterTableWithDropColumn();
    }

    public boolean supportsBatchUpdates() throws SQLException {
        return impl.supportsBatchUpdates();
    }

    public boolean supportsCatalogsInDataManipulation() throws SQLException {
        return impl.supportsCatalogsInDataManipulation();
    }

    public boolean supportsCatalogsInIndexDefinitions() throws SQLException {
        return impl.supportsCatalogsInIndexDefinitions();
    }

    public boolean supportsCatalogsInPrivilegeDefinitions() throws SQLException {
        return impl.supportsCatalogsInPrivilegeDefinitions();
    }

    public boolean supportsCatalogsInProcedureCalls() throws SQLException {
        return impl.supportsCatalogsInProcedureCalls();
    }

    public boolean supportsCatalogsInTableDefinitions() throws SQLException {
        return impl.supportsCatalogsInTableDefinitions();
    }

    public boolean supportsColumnAliasing() throws SQLException {
        return impl.supportsColumnAliasing();
    }

    public boolean supportsConvert(int arg0, int arg1) throws SQLException {
        return impl.supportsConvert(arg0, arg1);
    }

    public boolean supportsCoreSQLGrammar() throws SQLException {
        return impl.supportsCoreSQLGrammar();
    }

    public boolean supportsCorrelatedSubqueries() throws SQLException {
        return impl.supportsCorrelatedSubqueries();
    }

    public boolean supportsDataDefinitionAndDataManipulationTransactions() throws SQLException {
        return impl.supportsDataDefinitionAndDataManipulationTransactions();
    }

    public boolean supportsDataManipulationTransactionsOnly() throws SQLException {
        return impl.supportsDataManipulationTransactionsOnly();
    }

    public boolean supportsDifferentTableCorrelationNames() throws SQLException {
        return impl.supportsDifferentTableCorrelationNames();
    }

    public boolean supportsExpressionsInOrderBy() throws SQLException {
        return impl.supportsExpressionsInOrderBy();
    }

    public boolean supportsExtendedSQLGrammar() throws SQLException {
        return impl.supportsExtendedSQLGrammar();
    }

    public boolean supportsFullOuterJoins() throws SQLException {
        return impl.supportsFullOuterJoins();
    }

    public boolean supportsGroupBy() throws SQLException {
        return impl.supportsGroupBy();
    }

    public boolean supportsGroupByBeyondSelect() throws SQLException {
        return impl.supportsGroupByBeyondSelect();
    }

    public boolean supportsGroupByUnrelated() throws SQLException {
        return impl.supportsGroupByUnrelated();
    }

    public boolean supportsIntegrityEnhancementFacility() throws SQLException {
        return impl.supportsIntegrityEnhancementFacility();
    }

    public boolean supportsLikeEscapeClause() throws SQLException {
        return impl.supportsLikeEscapeClause();
    }

    public boolean supportsLimitedOuterJoins() throws SQLException {
        return impl.supportsLimitedOuterJoins();
    }

    public boolean supportsMinimumSQLGrammar() throws SQLException {
        return impl.supportsMinimumSQLGrammar();
    }

    public boolean supportsMixedCaseIdentifiers() throws SQLException {
        return impl.supportsMixedCaseIdentifiers();
    }

    public boolean supportsMixedCaseQuotedIdentifiers() throws SQLException {
        return impl.supportsMixedCaseQuotedIdentifiers();
    }

    public boolean supportsMultipleResultSets() throws SQLException {
        return impl.supportsMultipleResultSets();
    }

    public boolean supportsMultipleTransactions() throws SQLException {
        return impl.supportsMultipleTransactions();
    }

    public boolean supportsNonNullableColumns() throws SQLException {
        return impl.supportsNonNullableColumns();
    }

    public boolean supportsOpenCursorsAcrossCommit() throws SQLException {
        return impl.supportsOpenCursorsAcrossCommit();
    }

    public boolean supportsOpenCursorsAcrossRollback() throws SQLException {
        return impl.supportsOpenCursorsAcrossRollback();
    }

    public boolean supportsOpenStatementsAcrossCommit() throws SQLException {
        return impl.supportsOpenStatementsAcrossCommit();
    }

    public boolean supportsOpenStatementsAcrossRollback() throws SQLException {
        return impl.supportsOpenStatementsAcrossRollback();
    }

    public boolean supportsOrderByUnrelated() throws SQLException {
        return impl.supportsOrderByUnrelated();
    }

    public boolean supportsOuterJoins() throws SQLException {
        return impl.supportsOuterJoins();
    }

    public boolean supportsPositionedDelete() throws SQLException {
        return impl.supportsPositionedDelete();
    }

    public boolean supportsPositionedUpdate() throws SQLException {
        return impl.supportsPositionedUpdate();
    }

    public boolean supportsResultSetConcurrency(int arg0, int arg1) throws SQLException {
        return impl.supportsResultSetConcurrency(arg0, arg1);
    }

    public boolean supportsResultSetType(int arg0) throws SQLException {
        return impl.supportsResultSetType(arg0);
    }

    public boolean supportsSchemasInDataManipulation() throws SQLException {
        return impl.supportsSchemasInDataManipulation();
    }

    public boolean supportsSchemasInIndexDefinitions() throws SQLException {
        return impl.supportsSchemasInIndexDefinitions();
    }

    public boolean supportsSchemasInPrivilegeDefinitions() throws SQLException {
        return impl.supportsSchemasInPrivilegeDefinitions();
    }

    public boolean supportsSchemasInProcedureCalls() throws SQLException {
        return impl.supportsSchemasInProcedureCalls();
    }

    public boolean supportsSchemasInTableDefinitions() throws SQLException {
        return impl.supportsSchemasInTableDefinitions();
    }

    public boolean supportsSelectForUpdate() throws SQLException {
        return impl.supportsSelectForUpdate();
    }

    public boolean supportsStoredProcedures() throws SQLException {
        return impl.supportsStoredProcedures();
    }

    public boolean supportsSubqueriesInComparisons() throws SQLException {
        return impl.supportsSubqueriesInComparisons();
    }

    public boolean supportsSubqueriesInExists() throws SQLException {
        return impl.supportsSubqueriesInExists();
    }

    public boolean supportsSubqueriesInIns() throws SQLException {
        return impl.supportsSubqueriesInIns();
    }

    public boolean supportsSubqueriesInQuantifieds() throws SQLException {
        return impl.supportsSubqueriesInQuantifieds();
    }

    public boolean supportsTableCorrelationNames() throws SQLException {
        return impl.supportsTableCorrelationNames();
    }

    public boolean supportsTransactionIsolationLevel(int arg0) throws SQLException {
        return impl.supportsTransactionIsolationLevel(arg0);
    }

    public boolean supportsTransactions() throws SQLException {
        return impl.supportsTransactions();
    }

    public boolean supportsTypeConversion() throws SQLException {
        return impl.supportsTypeConversion();
    }

    public boolean supportsUnion() throws SQLException {
        return impl.supportsUnion();
    }

    public boolean supportsUnionAll() throws SQLException {
        return impl.supportsUnionAll();
    }

    public boolean updatesAreDetected(int arg0) throws SQLException {
        return impl.updatesAreDetected(arg0);
    }

    public boolean usesLocalFilePerTable() throws SQLException {
        return impl.usesLocalFilePerTable();
    }

    public boolean usesLocalFiles() throws SQLException {
        return impl.usesLocalFiles();
    }
}
