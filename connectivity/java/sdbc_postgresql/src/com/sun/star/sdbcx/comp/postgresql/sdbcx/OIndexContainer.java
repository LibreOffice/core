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

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.IndexType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxIndexDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.ComposeRule;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class OIndexContainer extends OContainer {
    protected OTable table;

    public OIndexContainer(Object lock, List<String> names, boolean isCaseSensitive, OTable table) throws ElementExistException {
        super(lock, isCaseSensitive, names);
        this.table = table;
    }

    @Override
    protected XPropertySet createDescriptor() {
        return SdbcxIndexDescriptor.create(isCaseSensitive());
    }

    @Override
    protected XPropertySet createObject(String name) throws SQLException {
        try {
            XConnection connection = table.getConnection();
            if (connection == null) {
                return null;
            }

            XPropertySet ret = null;
            String qualifier = "";
            String subname;
            int len = name.indexOf('.');
            if (len >= 0) {
                qualifier = name.substring(0, len);
                subname = name.substring(len + 1);
            } else {
                subname = name;
            }

            Object catalog = table.getPropertyValue(PropertyIds.CATALOGNAME.name);
            String schemaName = AnyConverter.toString(table.getPropertyValue(PropertyIds.SCHEMANAME.name));
            String tableName = AnyConverter.toString(table.getPropertyValue(PropertyIds.NAME.name));
            XResultSet results = null;
            try {
                results = connection.getMetaData().getIndexInfo(catalog, schemaName, tableName, false, false);
                if (results != null) {
                    XRow row = UnoRuntime.queryInterface(XRow.class, results);
                    boolean found = false;
                    boolean isUnique = false;
                    int clustered = -1;
                    boolean isPrimaryKeyIndex = false;
                    ArrayList<String> columnNames = new ArrayList<>();
                    while (results.next()) {
                        isUnique  = !row.getBoolean(4);
                        if ((qualifier.isEmpty() || row.getString(5).equals(qualifier)) && row.getString(6).equals(subname)) {
                            found = true;
                            clustered = row.getShort(7);
                            isPrimaryKeyIndex = isPrimaryKeyIndex(connection.getMetaData(), catalog, schemaName, tableName, subname);
                            String columnName = row.getString(9);
                            if (!row.wasNull()) {
                                columnNames.add(columnName);
                            }
                        }
                    }
                    if (found) {
                        ret = OIndex.create(subname, isCaseSensitive(), qualifier, isUnique, isPrimaryKeyIndex, clustered == IndexType.CLUSTERED,
                                columnNames, table);
                    }
                }
            } finally {
                CompHelper.disposeComponent(results);
            }
            return ret;
        } catch (WrappedTargetException | UnknownPropertyException | IllegalArgumentException | ElementExistException exception) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    private static boolean isPrimaryKeyIndex(XDatabaseMetaData metadata, Object catalog, String schema, String table, String name) throws SQLException {
        XResultSet results = null;
        try {
            results = metadata.getPrimaryKeys(catalog, schema, table);
            if (results != null) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                if (results.next()) { // there can be only one primary key
                    return row.getString(6).equals(name);
                }
            }
            return false;
        } finally {
            CompHelper.disposeComponent(results);
        }
    }

    @Override
    protected XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException {
        try {
            XConnection connection = table.getConnection();
            if (connection == null) {
                return null;
            }
            String quote = connection.getMetaData().getIdentifierQuoteString();
            boolean isUnique = AnyConverter.toBoolean(descriptor.getPropertyValue(PropertyIds.ISUNIQUE.name));
            String composedName = DbTools.composeTableName(connection.getMetaData(), table, ComposeRule.InIndexDefinitions, false, false, true);
            StringBuilder columnsText = new StringBuilder();
            String separator = "";
            XColumnsSupplier columnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, descriptor);
            XIndexAccess columns = UnoRuntime.queryInterface(XIndexAccess.class, columnsSupplier.getColumns());
            for (int i = 0; i < columns.getCount(); i++) {
                columnsText.append(separator);
                separator = ", ";
                XPropertySet column = AnyConverter.toObject(XPropertySet.class, columns.getByIndex(i));
                columnsText.append(DbTools.quoteName(quote, AnyConverter.toString(column.getPropertyValue(PropertyIds.NAME.name))));
                // FIXME: ::dbtools::getBooleanDataSourceSetting( m_pTable->getConnection(), "AddIndexAppendix" );
                boolean isAscending = AnyConverter.toBoolean(column.getPropertyValue(PropertyIds.ISASCENDING.name));
                columnsText.append(isAscending ? " ASC" : " DESC");
            }
            String sql = String.format("CREATE %s INDEX %s ON %s (%s)",
                    isUnique ? "UNIQUE" : "",
                    _rForName.isEmpty() ? "" : DbTools.quoteName(quote, _rForName),
                    composedName,
                    columnsText.toString());
            XStatement statement = null;
            try {
                statement = connection.createStatement();
                statement.execute(sql);
            } finally {
                CompHelper.disposeComponent(statement);
            }
            return createObject(_rForName);
        } catch (WrappedTargetException | UnknownPropertyException | IllegalArgumentException | IndexOutOfBoundsException exception) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    @Override
    protected void dropObject(int index, String elementName) throws SQLException {
        XConnection connection = table.getConnection();
        if (connection == null) {
            return;
        }
        String name;
        String schema = "";
        int len = elementName.indexOf('.');
        if (len >= 0) {
            schema = elementName.substring(0, len);
        }
        name = elementName.substring(len + 1);

        String composedName = DbTools.composeTableName(connection.getMetaData(), table, ComposeRule.InTableDefinitions, false, false, true);
        String indexName = DbTools.composeTableName(connection.getMetaData(), "", schema, name, true, ComposeRule.InIndexDefinitions);
        String sql = String.format("DROP INDEX %s ON %s", indexName, composedName);
        XStatement statement = null;
        try {
            statement = connection.createStatement();
            statement.execute(sql);
        } finally {
            CompHelper.disposeComponent(statement);
        }
    }

    @Override
    protected void impl_refresh() {
        // FIXME
    }
}
