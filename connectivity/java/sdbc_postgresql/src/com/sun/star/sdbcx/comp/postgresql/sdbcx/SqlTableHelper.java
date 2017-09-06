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

package com.sun.star.sdbcx.comp.postgresql.sdbcx;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import com.sun.star.container.ElementExistException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbcx.KeyType;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.util.ComposeRule;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools;
import com.sun.star.sdbcx.comp.postgresql.util.Osl;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;

public class SqlTableHelper {
    public static class ColumnDescription {
        public String columnName;
        public int type;
        public String typeName;
        public int columnSize;
        public int decimalDigits;
        public int nullable;
        public String remarks;
        public String defaultValue;
        public int ordinalPosition;
    }

    private static class KeyProperties {
        ArrayList<String> columnNames = new ArrayList<>();
        String referencedTable;
        int type;
        int updateRule;
        int deleteRule;

        KeyProperties(String referencedTable, int type, int updateRule, int deleteRule) {
            this.referencedTable = referencedTable;
            this.type = type;
            this.updateRule = updateRule;
            this.deleteRule = deleteRule;
        }
    }

    public List<ColumnDescription> readColumns(XDatabaseMetaData metadata, String catalog, String schema, String table) throws SQLException {
        String catalogName = catalog;
        if (catalog.isEmpty()) {
            catalogName = null;
        }

        XResultSet results = metadata.getColumns(catalogName, schema, table, "%");
        List<ColumnDescription> columnDescriptions = collectColumnDescriptions(results);
        sanitizeColumnDescriptions(columnDescriptions);

        List<ColumnDescription> columnsByOrdinal = new ArrayList<>(columnDescriptions);
        for (ColumnDescription columnDescription : columnDescriptions) {
            columnsByOrdinal.set(columnDescription.ordinalPosition - 1, columnDescription);
        }
        return columnsByOrdinal;
    }

    private List<ColumnDescription> collectColumnDescriptions(XResultSet results) throws SQLException {
        List<ColumnDescription> columns = new ArrayList<>();
        XRow row = UnoRuntime.queryInterface(XRow.class, results);
        while (results.next()) {
            ColumnDescription columnDescription = new ColumnDescription();
            columnDescription.columnName = row.getString(4);
            columnDescription.type = row.getInt(5);
            columnDescription.typeName = row.getString(6);
            columnDescription.columnSize = row.getInt(7);
            columnDescription.decimalDigits = row.getInt(9);
            columnDescription.nullable = row.getInt(11);
            columnDescription.remarks = row.getString(12);
            columnDescription.defaultValue = row.getString(13);
            columnDescription.ordinalPosition = row.getInt(17);
            columns.add(columnDescription);
        }
        return columns;
    }

    private void sanitizeColumnDescriptions(List<ColumnDescription> columnDescriptions) {
        if (columnDescriptions.isEmpty()) {
            return;
        }
        Set<Integer> usedOrdinals = new TreeSet<>();
        int maxOrdinal = Integer.MIN_VALUE;
        for (ColumnDescription columnDescription : columnDescriptions) {
            usedOrdinals.add(columnDescription.ordinalPosition);
            if (maxOrdinal < columnDescription.ordinalPosition) {
                maxOrdinal = columnDescription.ordinalPosition;
            }
        }
        // we need to have as many different ordinals as we have different columns
        boolean hasDuplicates = usedOrdinals.size() != columnDescriptions.size();
        // and it needs to be a continuous range
        boolean hasGaps = (maxOrdinal - usedOrdinals.iterator().next() + 1) != columnDescriptions.size();
        // if that's not the case, normalize it
        Osl.ensure(!hasDuplicates && !hasGaps, "database provided invalid ORDINAL_POSITION values!");
        // what's left is that the range might not be from 1 to <column count>, but for instance
        // 0 to <column count>-1.
        int offset = usedOrdinals.iterator().next() - 1;
        for (ColumnDescription columnDescription : columnDescriptions) {
            columnDescription.ordinalPosition -= offset;
        }
    }

    public Map<String, OKey> readKeys(XDatabaseMetaData metadata, String catalogName, String schemaName, String tableName,
            boolean isCaseSensitive, OTable table) throws SQLException {
        Map<String, OKey> keys = new TreeMap<>();
        OKey primaryKey = readPrimaryKey(metadata, catalogName, schemaName, tableName, isCaseSensitive, table);
        keys.put(primaryKey.getName(), primaryKey);
        readForeignKeys(metadata, catalogName, schemaName, tableName, isCaseSensitive, keys, table);
        return keys;
    }

    private OKey readPrimaryKey(XDatabaseMetaData metadata,
            String catalogName, String schemaName, String tableName, boolean isCaseSensitive, OTable table) throws SQLException {
        Object catalog = Any.VOID;
        if (!catalogName.isEmpty()) {
            catalog = catalogName;
        }
        XResultSet results = null;
        try {
            ArrayList<String> columns = new ArrayList<>();
            boolean alreadyFetched = false;
            String pkName = "";
            OKey key = null;
            results = metadata.getPrimaryKeys(catalog, schemaName, tableName);
            if (results != null) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                while (results.next()) {
                    String columnName = row.getString(4);
                    columns.add(columnName);
                    if (!alreadyFetched) {
                        alreadyFetched = true;
                        pkName = row.getString(6);
                    }
                }
                key = OKey.create(pkName, isCaseSensitive, "", KeyType.PRIMARY, 0, 0, columns, table);
            }
            return key;
        } catch (ElementExistException elementExistException) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, elementExistException);
        } finally {
            CompHelper.disposeComponent(results);
        }
    }

    private void readForeignKeys(XDatabaseMetaData metadata,
            String catalogName, String schemaName, String tableName, boolean isCaseSensitive, Map<String, OKey> keys, OTable table) throws SQLException {
        Object catalog = Any.VOID;
        if (!catalogName.isEmpty()) {
            catalog = catalogName;
        }
        XResultSet results = null;
        try {
            results = metadata.getImportedKeys(catalog, schemaName, tableName);
            XRow row = UnoRuntime.queryInterface(XRow.class, results);
            if (row != null) {
                String oldFkName = "";
                KeyProperties keyProperties = null;
                while (results.next()) {
                    String catalogReturned = row.getString(1);
                    if (row.wasNull()) {
                        catalogReturned = "";
                    }
                    String schemaReturned = row.getString(2);
                    String nameReturned = row.getString(3);

                    String foreignKeyColumn = row.getString(8);
                    int updateRule = row.getInt(10);
                    int deleteRule = row.getInt(11);
                    String fkName = row.getString(12);

                    if (!row.wasNull() && !fkName.isEmpty()) {
                        if (!oldFkName.equals(fkName)) {
                            if (keyProperties != null) {
                                OKey key = OKey.create(oldFkName, isCaseSensitive, keyProperties.referencedTable, keyProperties.type,
                                        keyProperties.updateRule, keyProperties.deleteRule, keyProperties.columnNames, table);
                                keys.put(oldFkName, key);
                            }
                            String referencedName = DbTools.composeTableName(metadata, catalogReturned, schemaReturned, nameReturned,
                                    false, ComposeRule.InDataManipulation);
                            keyProperties = new KeyProperties(referencedName, KeyType.FOREIGN, updateRule, deleteRule);
                            keyProperties.columnNames.add(foreignKeyColumn);
                            oldFkName = fkName;
                        } else {
                            if (keyProperties != null) {
                                keyProperties.columnNames.add(foreignKeyColumn);
                            }
                        }
                    }
                }
                if (keyProperties != null) {
                    OKey key = OKey.create(oldFkName, isCaseSensitive, keyProperties.referencedTable, keyProperties.type,
                            keyProperties.updateRule, keyProperties.deleteRule, keyProperties.columnNames, table);
                    keys.put(oldFkName, key);
                }
            }
        } catch (ElementExistException elementExistException) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, elementExistException);
        } finally {
            CompHelper.disposeComponent(results);
        }
    }

    public ArrayList<String> readIndexes(XDatabaseMetaData metadata, String catalogName, String schemaName, String tableName, OTable table) throws SQLException {
        Object catalog = Any.VOID;
        if (!catalogName.isEmpty()) {
            catalog = catalogName;
        }
        ArrayList<String> names = new ArrayList<>();
        XResultSet results = null;
        try {
            results = metadata.getIndexInfo(catalog, schemaName, tableName, false, false);
            if (results != null) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                String catalogSep = metadata.getCatalogSeparator();
                String previousRoundName = "";
                while (results.next()) {
                    String name = row.getString(5);
                    if (!name.isEmpty()) {
                        name += catalogSep;
                    }
                    name += row.getString(6);
                    if (!name.isEmpty()) {
                        // don't insert the name if the last one we inserted was the same
                        if (!previousRoundName.equals(name)) {
                            names.add(name);
                        }
                    }
                }
            }
        } finally {
            CompHelper.disposeComponent(results);
        }
        return names;
    }
}
