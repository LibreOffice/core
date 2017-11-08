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

import org.apache.openoffice.comp.sdbc.dbtools.util.DatabaseMetaDataResultSet;
import org.apache.openoffice.comp.sdbc.dbtools.util.ORowSetValue;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.uno.UnoRuntime;

public class PostgresqlDatabaseMetaDataResultSet extends DatabaseMetaDataResultSet {
    private XResultSetMetaDataSupplier resultSetMetaDataSupplier;

    public PostgresqlDatabaseMetaDataResultSet(XResultSet resultSet, ArrayList<ORowSetValue[]> rows) {
        super(resultSet, rows);
        this.resultSetMetaDataSupplier = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, resultSet);
    }

    @Override
    public synchronized XResultSetMetaData getMetaData() throws SQLException {
        return new PostgresqlResultSetMetaData(resultSetMetaDataSupplier.getMetaData());
    }
}
