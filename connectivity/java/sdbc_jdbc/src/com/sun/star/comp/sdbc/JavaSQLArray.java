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

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.ResourceBasedEventLogger;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.container.XNameAccess;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.uno.Any;

public class JavaSQLArray extends ComponentBase implements XArray {
    private final ResourceBasedEventLogger logger;
    private final java.sql.Array jdbcArray;

    public JavaSQLArray(ResourceBasedEventLogger logger, java.sql.Array jdcbArray) {
        this.logger = logger;
        this.jdbcArray = jdcbArray;
    }

    @Override
    protected void postDisposing() {
        try {
            jdbcArray.free();
        } catch (java.sql.SQLException jdbcSQLException) {
            logger.log(LogLevel.WARNING, jdbcSQLException);
        }
    }

    @Override
    public Object[] getArray(XNameAccess typeMap) throws SQLException {
        if (typeMap.hasElements()) {
            throw new SQLException(
                    SharedResources.getInstance().getResourceStringWithSubstitution(
                            Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "Type maps"),
                    this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
        }
        return new Object[0];
    }

    @Override
    public Object[] getArrayAtIndex(int index, int count, XNameAccess typeMap) throws SQLException {
        if (typeMap.hasElements()) {
            throw new SQLException(
                    SharedResources.getInstance().getResourceStringWithSubstitution(
                            Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "Type maps"),
                    this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
        }
        return new Object[0];
    }

    @Override
    public int getBaseType() throws SQLException {
        try {
            return jdbcArray.getBaseType();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public String getBaseTypeName() throws SQLException {
        try {
            return jdbcArray.getBaseTypeName();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public XResultSet getResultSet(XNameAccess typeMap) throws SQLException {
        if (typeMap.hasElements()) {
            throw new SQLException(
                    SharedResources.getInstance().getResourceStringWithSubstitution(
                            Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "Type maps"),
                    this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
        }
        return null;
    }

    @Override
    public XResultSet getResultSetAtIndex(int index, int count, XNameAccess typeMap) throws SQLException {
        if (typeMap.hasElements()) {
            throw new SQLException(
                    SharedResources.getInstance().getResourceStringWithSubstitution(
                            Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "Type maps"),
                    this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
        }
        return null;
    }
}
