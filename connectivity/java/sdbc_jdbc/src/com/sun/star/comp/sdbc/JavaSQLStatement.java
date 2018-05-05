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

import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XBatchExecution;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XStatement;

public class JavaSQLStatement extends JavaSQLStatementBase
        implements XStatement, XServiceInfo, XBatchExecution {

    private static final String[] services = {
            "com.sun.star.sdbc.Statement"
    };

    public JavaSQLStatement(JavaSQLConnection connection) {
        super(connection);
    }

    // XServiceInfo

    @Override
    public String getImplementationName() {
        return "com.sun.star.sdbcx.JStatement";
    }

    @Override
    public String[] getSupportedServiceNames() {
        return services.clone();
    }

    @Override
    public boolean supportsService(String serviceName) {
        for (String service : services) {
            if (service.equals(serviceName)) {
                return true;
            }
        }
        return false;
    }

    // XBatchExecution

    @Override
    public synchronized void addBatch(String sql) throws SQLException {
        createStatement();
        try {
            jdbcStatement.addBatch(sql);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public void clearBatch() throws SQLException {
        createStatement();
        try {
            jdbcStatement.clearBatch();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized int[] executeBatch() throws SQLException {
        createStatement();
        try {
            return jdbcStatement.executeBatch();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    // XStatement

    @Override
    public synchronized boolean execute(String sql) throws SQLException {
        createStatement();
        sqlStatement = sql;
        try (ContextClassLoaderScope ccl = new ContextClassLoaderScope(connection.getDriverClassLoader())) {
            return jdbcStatement.execute(sql);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized XResultSet executeQuery(String sql) throws SQLException {
        logger.log(LogLevel.FINE, Resources.STR_LOG_EXECUTE_QUERY, sql);
        createStatement();
        sqlStatement = sql;
        try (ContextClassLoaderScope ccl = new ContextClassLoaderScope(connection.getDriverClassLoader())) {
            java.sql.ResultSet jdbcResultSet = jdbcStatement.executeQuery(sql);
            if (jdbcResultSet != null) {
                return new JavaSQLResultSet(jdbcResultSet, connection, this);
            } else {
                return null;
            }
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized int executeUpdate(String sql) throws SQLException {
        createStatement();
        logger.log(LogLevel.FINE, Resources.STR_LOG_EXECUTE_UPDATE, sql);
        sqlStatement = sql;
        // FIXME: why didn't the C++ implementation do this in a ContextClassLoaderScope?
        try {
            return jdbcStatement.executeUpdate(sql);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized XConnection getConnection() throws SQLException {
        checkDisposed();
        return connection;
    }

    // others

    @Override
    protected synchronized void createStatement() throws SQLException {
        checkDisposed();
        if (jdbcStatement == null) {
            try {
                try {
                    jdbcStatement = connection.getJDBCConnection().createStatement(resultSetType, resultSetConcurrency);
                } catch (NoSuchMethodError noSuchMethodError) {
                    jdbcStatement = connection.getJDBCConnection().createStatement();
                }
            } catch (java.sql.SQLException sqlException) {
                throw Tools.toUnoExceptionLogged(this, logger, sqlException);
            }
        }
    }
}
