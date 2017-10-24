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
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.CheckOption;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OView;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxViewDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.ComposeRule;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools.NameComponents;
import com.sun.star.sdbcx.comp.postgresql.util.Osl;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.UnoRuntime;

public class PostgresqlViews extends OContainer {
    private PostgresqlCatalog catalog;
    private XDatabaseMetaData metadata;

    public PostgresqlViews(Object lock, XDatabaseMetaData metadata, PostgresqlCatalog catalog, List<String> names) throws ElementExistException {
        super(lock, true, names);
        this.metadata = metadata;
        this.catalog = catalog;
    }

    @Override
    protected XPropertySet createObject(String name) throws SQLException {
        NameComponents nameComponents = DbTools.qualifiedNameComponents(metadata, name, ComposeRule.InDataManipulation);

        String sql = "SELECT view_definition,check_option FROM INFORMATION_SCHEMA.views WHERE ";
        if (!nameComponents.getCatalog().isEmpty()) {
            sql += "table_catalog=? AND ";
        }
        if (!nameComponents.getSchema().isEmpty()) {
            sql += "table_schema=? AND ";
        }
        sql += "table_name=?";

        final String command;
        final String checkOption;
        XPreparedStatement statement = null;
        XResultSet results = null;
        try {
            statement = metadata.getConnection().prepareStatement(sql);
            XParameters parameters = UnoRuntime.queryInterface(XParameters.class, statement);
            int next = 1;
            if (!nameComponents.getCatalog().isEmpty()) {
                parameters.setString(next++, nameComponents.getCatalog());
            }
            if (!nameComponents.getSchema().isEmpty()) {
                parameters.setString(next++, nameComponents.getSchema());
            }
            parameters.setString(next, nameComponents.getTable());
            results = statement.executeQuery();
            if (results.next()) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                command = row.getString(1);
                checkOption = row.getString(2);
            } else {
                throw new SQLException("View not found", this, StandardSQLState.SQL_TABLE_OR_VIEW_NOT_FOUND.text(), 0, null);
            }
        } finally {
            CompHelper.disposeComponent(results);
            CompHelper.disposeComponent(statement);
        }

        final int checkOptionInt;
        if (checkOption.equals("NONE")) {
            checkOptionInt = CheckOption.NONE;
        } else if (checkOption.equals("LOCAL")) {
            checkOptionInt = CheckOption.LOCAL;
        } else if (checkOption.equals("CASCADED")) {
            checkOptionInt = CheckOption.CASCADE;
        } else {
            throw new SQLException("Unsupported check option '" + checkOption + "'", this,
                    StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.text(), 0, null);
        }

        return new OView(nameComponents.getCatalog(), nameComponents.getSchema(), nameComponents.getTable(), isCaseSensitive(),
                command, checkOptionInt);
    }

    @Override
    protected void dropObject(int index, String name) throws SQLException {
        XStatement statement = null;
        try {
            Object object = getObject(index);
            XPropertySet propertySet = UnoRuntime.queryInterface(XPropertySet.class, object);
            Osl.ensure(propertySet != null, "Object returned from view collection isn't an XPropertySet");
            String sql = String.format("DROP VIEW %s", DbTools.composeTableName(metadata, propertySet, ComposeRule.InTableDefinitions,
                    false, false, true));

            statement = metadata.getConnection().createStatement();
            statement.execute(sql);
        } catch (WrappedTargetException exception) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        } finally {
            CompHelper.disposeComponent(statement);
        }
    }

    @Override
    protected XPropertySet createDescriptor() {
        return new SdbcxViewDescriptor(isCaseSensitive());
    }

    @Override
    protected XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException {
        XStatement statement = null;
        try {
            String sql = String.format("CREATE VIEW %s AS %s",
                    DbTools.composeTableName(metadata, descriptor, ComposeRule.InTableDefinitions, false, false, true),
                    descriptor.getPropertyValue(PropertyIds.COMMAND.name));
            statement = metadata.getConnection().createStatement();
            statement.execute(sql);
        } catch (WrappedTargetException | UnknownPropertyException exception) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        } finally {
            CompHelper.disposeComponent(statement);
        }
        // Append it to the tables container too:
        catalog.getTablesInternal().insertElement(_rForName, null);
        return createObject(_rForName);
    }

    @Override
    protected void impl_refresh() {
        catalog.refreshObjects();
    }
}
