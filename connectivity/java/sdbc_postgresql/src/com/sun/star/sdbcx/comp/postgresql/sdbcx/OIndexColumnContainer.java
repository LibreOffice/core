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
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxIndexColumnDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class OIndexColumnContainer extends OContainer {
    private OIndex index;

    public OIndexColumnContainer(Object lock, OIndex index, List<String> columnNames) throws ElementExistException {
        super(lock, true, columnNames);
        this.index = index;
    }

    @Override
    protected XPropertySet createDescriptor() {
        return SdbcxIndexColumnDescriptor.create(isCaseSensitive());
    }

    @Override
    protected XPropertySet createObject(String name) throws SQLException {
        try {
            Object catalog = index.getTable().getPropertyValue(PropertyIds.CATALOGNAME.name);
            String schema = AnyConverter.toString(index.getTable().getPropertyValue(PropertyIds.SCHEMANAME.name));
            String table = AnyConverter.toString(index.getTable().getPropertyValue(PropertyIds.NAME.name));

            boolean isAscending = true;
            XResultSet results = null;
            try {
                results = index.getTable().getConnection().getMetaData().getIndexInfo(catalog, schema, table, false, false);
                if (results != null) {
                    XRow row = UnoRuntime.queryInterface(XRow.class, results);
                    while (results.next()) {
                        if (row.getString(9).equals(name)) {
                            isAscending = !row.getString(10).equals("D");
                        }
                    }
                }
            } finally {
                CompHelper.disposeComponent(results);
            }

            XPropertySet ret = null;
            results = null;
            try {
                results = index.getTable().getConnection().getMetaData().getColumns(catalog, schema, table, name);
                if (results != null) {
                    XRow row = UnoRuntime.queryInterface(XRow.class, results);
                    while (results.next()) {
                        if (row.getString(4).equals(name)) {
                            int dataType = row.getInt(5);
                            String typeName = row.getString(6);
                            int size = row.getInt(7);
                            int dec = row.getInt(9);
                            int nul = row.getInt(11);
                            String columnDef = row.getString(13);

                            ret = OIndexColumn.create(isAscending, name, typeName, columnDef,
                                    nul, size, dec, dataType, false, false, false, isCaseSensitive());
                            break;
                        }
                    }
                }
            } finally {
                CompHelper.disposeComponent(results);
            }

            return ret;
        } catch (WrappedTargetException | UnknownPropertyException | IllegalArgumentException exception) {
            throw new SQLException("Error", this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    @Override
    protected void impl_refresh() {
        // FIXME
    }

    @Override
    protected XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException {
        throw new SQLException("Unsupported");
    }

    @Override
    protected void dropObject(int index, String name) throws SQLException {
        throw new SQLException("Unsupported");
    }
}
