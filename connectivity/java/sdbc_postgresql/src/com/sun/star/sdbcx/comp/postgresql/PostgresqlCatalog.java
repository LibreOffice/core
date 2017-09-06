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
import java.util.List;

import com.sun.star.container.ElementExistException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OCatalog;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OContainer;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;

public class PostgresqlCatalog extends OCatalog {
    public PostgresqlCatalog(PostgresqlConnection connection) throws SQLException {
        super(connection.getMetaData());
    }

    @Override
    public OContainer refreshTables() {
        XResultSet results = null;
        try {
            // Using { "VIEW", "TABLE", "%" } shows INFORMATION_SCHEMA and others, but it also shows indexes :-(
            results = metadata.getTables(Any.VOID, "%", "%", new String[] { "VIEW", "TABLE" });
            XRow row = UnoRuntime.queryInterface(XRow.class, results);
            List<String> names = new ArrayList<>();
            while (results.next()) {
                String name = buildName(row);
                System.out.println("Table " + name);
                names.add(name);
            }
            return new PostgresqlTables(lock, metadata, this, names);
        } catch (ElementExistException elementExistException) {
        } catch (SQLException sqlException) {
        } finally {
            CompHelper.disposeComponent(results);
        }
        return null;
    }

    @Override
    public OContainer refreshViews() {
        XResultSet results = null;
        try {
            results = metadata.getTables(Any.VOID, "%", "%", new String[] { "VIEW" });
            XRow row = UnoRuntime.queryInterface(XRow.class, results);
            List<String> names = new ArrayList<>();
            while (results.next()) {
                String name = buildName(row);
                names.add(name);
            }
            return new PostgresqlTables(lock, metadata, this, names);
        } catch (ElementExistException elementExistException) {
        } catch (SQLException sqlException) {
        } finally {
            CompHelper.disposeComponent(results);
        }
        return null;
    }

    @Override
    public OContainer refreshGroups() {
        return null;
    }

    @Override
    public OContainer refreshUsers() {
        return null;
    }
}
