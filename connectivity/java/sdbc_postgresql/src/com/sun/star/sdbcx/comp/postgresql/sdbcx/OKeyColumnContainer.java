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

import java.util.List;

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxKeyColumnDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class OKeyColumnContainer extends OContainer {
    private OKey key;

    public OKeyColumnContainer(Object lock, OKey key, List<String> columnNames) throws ElementExistException {
        super(lock, true, columnNames);
        this.key = key;
    }

    @Override
    protected XPropertySet createDescriptor() {
        return SdbcxKeyColumnDescriptor.create(isCaseSensitive());
    }

    @Override
    protected XPropertySet createObject(String name) throws SQLException {
        try {
            XPropertySet tableProperties = UnoRuntime.queryInterface(XPropertySet.class, key.getTable());
            Object catalog = tableProperties.getPropertyValue(PropertyIds.CATALOGNAME.name);
            String schema = AnyConverter.toString(tableProperties.getPropertyValue(PropertyIds.SCHEMANAME.name));
            String table = AnyConverter.toString(tableProperties.getPropertyValue(PropertyIds.NAME.name));

            XResultSet results = key.getTable().getConnection().getMetaData().getImportedKeys(catalog, schema, table);
            String refColumnName = "";
            if (results != null) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                while (results.next()) {
                    if (row.getString(8).equals(name) && key.getName().equals(row.getString(12))) {
                        refColumnName = row.getString(4);
                        break;
                    }
                }
            }

            XPropertySet ret = null;
            // now describe the column name and set its related column
            results = key.getTable().getConnection().getMetaData().getColumns(catalog, schema, table, name);
            if (results != null) {
                XRow row = UnoRuntime.queryInterface(XRow.class, results);
                if (results.next()) {
                    if (row.getString(4).equals(name)) {
                        int dataType = row.getInt(5);
                        String typeName = row.getString(6);
                        int size = row.getInt(7);
                        int dec = row.getInt(9);
                        int nul = row.getInt(11);
                        String columnDef = "";
                        try {
                            columnDef = row.getString(13);
                        } catch (SQLException sqlException) {
                            // sometimes we get an error when asking for this param
                        }
                        ret = OKeyColumn.create(refColumnName, name, typeName,
                                columnDef, nul, size, dec, dataType, false, false, false, isCaseSensitive());

                    }
                }
            }
            return ret;
        } catch (WrappedTargetException | UnknownPropertyException | IllegalArgumentException exception) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    @Override
    protected void impl_refresh() {
    }

    @Override
    protected XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException {
        throw new SQLException("Cannot change a key's columns, please delete and re-create the key instead");
    }

    @Override
    protected void dropObject(int index, String name) throws SQLException {
        throw new SQLException("Cannot change a key's columns, please delete and re-create the key instead");
    }
}
