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

import java.util.List;

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxTableDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.ComposeRule;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools.NameComponents;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class PostgresqlTables extends OContainer {
    private XDatabaseMetaData metadata;
    private PostgresqlCatalog catalog;

    public PostgresqlTables(Object lock, XDatabaseMetaData metadata, PostgresqlCatalog catalog, List<String> names) throws ElementExistException {
        super(lock, true, names);
        this.metadata = metadata;
        this.catalog = catalog;
    }

    @Override
    public XPropertySet createObject(String name) throws SQLException {
        NameComponents nameComponents = DbTools.qualifiedNameComponents(metadata, name, ComposeRule.InDataManipulation);
        Object queryCatalog = nameComponents.getCatalog().isEmpty() ? Any.VOID : nameComponents.getCatalog();
        XPropertySet ret = null;
        XResultSet results = null;
        try {
            results = metadata.getTables(
                    queryCatalog, nameComponents.getSchema(), nameComponents.getTable(), new String[] { "VIEW", "TABLE", "%" });
            if (results != null) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                if (results.next()) {
                    String type = row.getString(4);
                    String remarks = row.getString(5);
                    ret = PostgresqlTable.create(metadata.getConnection(), this, nameComponents.getTable(),
                            nameComponents.getCatalog(), nameComponents.getSchema(), remarks, type);
                }
            }
        } finally {
            CompHelper.disposeComponent(results);
        }
        return ret;

    }

    @Override
    public void dropObject(int index, String name) throws SQLException {
        try {
            Object object = getObject(index);

            NameComponents nameComponents = DbTools.qualifiedNameComponents(metadata, name, ComposeRule.InDataManipulation);

            boolean isView = false;
            XPropertySet propertySet = UnoRuntime.queryInterface(XPropertySet.class, object);
            if (propertySet != null) {
                isView = AnyConverter.toString(propertySet.getPropertyValue(PropertyIds.TYPE.name)).equals("VIEW");
            }

            String composedName = DbTools.composeTableName(metadata, nameComponents.getCatalog(), nameComponents.getSchema(), nameComponents.getTable(),
                    true, ComposeRule.InDataManipulation);
            String sql = String.format("DROP %s %s", isView ? "VIEW" : "TABLE", composedName);

            XStatement statement = null;
            try {
                statement = metadata.getConnection().createStatement();
                statement.execute(sql);
            } finally {
                CompHelper.disposeComponent(statement);
            }
            // FIXME: delete it from our views
        } catch (IllegalArgumentException | UnknownPropertyException | WrappedTargetException wrappedTargetException) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, wrappedTargetException);
        }
    }

    @Override
    public void impl_refresh() {
        catalog.refreshTables();
    }

    @Override
    public XPropertySet createDescriptor() {
        return SdbcxTableDescriptor.create(true);
    }

    @Override
    public XPropertySet appendObject(String name, XPropertySet descriptor) throws SQLException {
        createTable(descriptor);
        return createObject(name);
    }

    void createTable(XPropertySet descriptor) throws SQLException {
        XStatement statement = null;
        try {
            String sql = DbTools.createSqlCreateTableStatement(descriptor, metadata.getConnection(), null, "(M,D)");
            statement = metadata.getConnection().createStatement();
            statement.execute(sql);
        } finally {
            CompHelper.disposeComponent(statement);
        }
    }
}
