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

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.CompHelper;
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySet;
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySetAdapter.PropertyGetter;
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySetAdapter.PropertySetter;
import org.apache.openoffice.comp.sdbc.dbtools.util.PropertyIds;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.comp.sdbc.ConnectionLog.ObjectType;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.ResultSetConcurrency;
import com.sun.star.sdbc.ResultSetType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.SQLWarning;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XGeneratedResultSet;
import com.sun.star.sdbc.XMultipleResults;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.util.XCancellable;

public abstract class JavaSQLStatementBase extends PropertySet
        implements XGeneratedResultSet, XMultipleResults, XCloseable, XCancellable, XWarningsSupplier {

    protected JavaSQLConnection connection;
    protected ConnectionLog logger;
    protected java.sql.Statement jdbcStatement;
    protected boolean escapeProcessing = true;
    protected int resultSetType = ResultSetType.FORWARD_ONLY;
    protected int resultSetConcurrency = ResultSetConcurrency.READ_ONLY;
    protected String sqlStatement = "";
    protected XStatement generatedStatement;

    public JavaSQLStatementBase(JavaSQLConnection connection) {
        this.connection = connection;
        this.logger = new ConnectionLog(connection.getLogger(), ObjectType.STATEMENT);
        registerProperties();
    }

    public int getStatementObjectId() {
        return logger.getObjectId();
    }

    // XComponent

    @Override
    protected synchronized void postDisposing() {
        super.postDisposing();
        logger.log(LogLevel.FINE, Resources.STR_LOG_CLOSING_STATEMENT);
        if (jdbcStatement != null) {
            try {
                jdbcStatement.close();
            } catch (java.sql.SQLException sqlException) {
                logger.log(LogLevel.WARNING, sqlException);
            }
        }
        CompHelper.disposeComponent(generatedStatement);
    }

    protected abstract void createStatement() throws SQLException;

    // XPropertySet

    private void registerProperties() {
        registerProperty(PropertyIds.CURSORNAME.name, PropertyIds.CURSORNAME.id, Type.STRING, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getCursorName();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setCursorName((String)value);
                    }
                }
        );
        registerProperty(PropertyIds.ESCAPEPROCESSING.name, PropertyIds.ESCAPEPROCESSING.id, Type.BOOLEAN, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getEscapeProcessing();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setEscapeProcessing((boolean)value);
                    }
                }
        );
        registerProperty(PropertyIds.FETCHDIRECTION.name, PropertyIds.FETCHDIRECTION.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getFetchDirection();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setFetchDirection((int)value);
                    }
                }
        );
        registerProperty(PropertyIds.FETCHSIZE.name, PropertyIds.FETCHSIZE.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getFetchSize();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setFetchSize((int)value);
                    }
                }
        );
        registerProperty(PropertyIds.MAXFIELDSIZE.name, PropertyIds.MAXFIELDSIZE.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getMaxFieldSize();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setMaxFieldSize((int)value);
                    }
                }
        );
        registerProperty(PropertyIds.MAXROWS.name, PropertyIds.MAXROWS.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getMaxRows();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setMaxRows((int)value);
                    }
                }
        );
        registerProperty(PropertyIds.QUERYTIMEOUT.name, PropertyIds.QUERYTIMEOUT.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getQueryTimeOut();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws WrappedTargetException{
                        setQueryTimeOut((int)value);
                    }
                }
        );
        registerProperty(PropertyIds.RESULTSETCONCURRENCY.name, PropertyIds.RESULTSETCONCURRENCY.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getResultSetConcurrency();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setResultSetConcurrency((int)value);
                    }
                }
        );
        registerProperty(PropertyIds.RESULTSETTYPE.name, PropertyIds.RESULTSETTYPE.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() throws WrappedTargetException {
                        return getResultSetType();
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
                        setResultSetType((int)value);
                    }
                }
        );
    }

    private String getCursorName() throws WrappedTargetException {
        try {
            createStatement();
            // FIXME: C++'s jdbcStatement.getCursorName() doesn't exist in JDBC. We always return a blank string.
            return "";
        } catch (SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setCursorName(String value) throws WrappedTargetException {
        try {
            createStatement();
            jdbcStatement.setCursorName(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private boolean getEscapeProcessing() throws WrappedTargetException {
        try {
            createStatement();
            return escapeProcessing;
        } catch (SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setEscapeProcessing(boolean value) throws WrappedTargetException {
        escapeProcessing = value;
        try {
            createStatement();
            logger.log(LogLevel.FINE, Resources.STR_LOG_SET_ESCAPE_PROCESSING, value);
            jdbcStatement.setEscapeProcessing(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getFetchDirection() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getFetchDirection();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setFetchDirection(int value) throws WrappedTargetException {
        try {
            createStatement();
            logger.log(LogLevel.FINER, Resources.STR_LOG_FETCH_DIRECTION, value);
            jdbcStatement.setFetchDirection(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getFetchSize() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getFetchSize();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setFetchSize(int value) throws WrappedTargetException {
        try {
            createStatement();
            logger.log(LogLevel.FINER, Resources.STR_LOG_FETCH_SIZE, value);
            jdbcStatement.setFetchSize(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getMaxFieldSize() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getMaxFieldSize();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setMaxFieldSize(int value) throws WrappedTargetException {
        try {
            createStatement();
            jdbcStatement.setMaxFieldSize(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getMaxRows() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getMaxRows();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setMaxRows(int value) throws WrappedTargetException {
        try {
            createStatement();
            jdbcStatement.setMaxRows(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getQueryTimeOut() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getQueryTimeout();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setQueryTimeOut(int value) throws WrappedTargetException {
        try {
            createStatement();
            jdbcStatement.setQueryTimeout(value);
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private int getResultSetConcurrency() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getResultSetConcurrency();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setResultSetConcurrency(int value) throws WrappedTargetException {
        checkDisposed();
        try {
            logger.log(LogLevel.FINE, Resources.STR_LOG_RESULT_SET_CONCURRENCY, value);
            resultSetConcurrency = value;
            jdbcStatement.close();
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        } finally {
            jdbcStatement = null;
        }
    }

    private int getResultSetType() throws WrappedTargetException {
        try {
            createStatement();
            return jdbcStatement.getResultSetType();
        } catch (java.sql.SQLException | SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        }
    }

    private synchronized void setResultSetType(int value) throws WrappedTargetException {
        checkDisposed();
        try {
            logger.log(LogLevel.FINE, Resources.STR_LOG_RESULT_SET_TYPE, value);
            resultSetType = value;
            jdbcStatement.close();
        } catch (java.sql.SQLException exception) {
            throw new WrappedTargetException("SQL error", this, Tools.toUnoException(this, exception));
        } finally {
            jdbcStatement = null;
        }
    }

    // XCancellable

    @Override
    public void cancel() {
        try {
            createStatement();
            jdbcStatement.cancel();
        } catch (SQLException sqlException) {
            logger.log(LogLevel.SEVERE, sqlException);
        } catch (java.sql.SQLException jdbcException) {
            logger.log(LogLevel.SEVERE, jdbcException);
        }
    }

    // XCloseable

    @Override
    public synchronized void close() throws SQLException {
        checkDisposed();
        dispose();
    }

    // XWarningsSupplier

    @Override
    public synchronized void clearWarnings() throws SQLException {
        try {
            createStatement();
            jdbcStatement.clearWarnings();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized Object getWarnings() throws SQLException {
        try {
            createStatement();
            java.sql.SQLWarning javaWarning = jdbcStatement.getWarnings();
            if (javaWarning != null) {
                java.lang.Throwable nextException = javaWarning.getCause();
                SQLWarning warning = new SQLWarning(javaWarning.getMessage());
                warning.Context = this;
                warning.SQLState = javaWarning.getSQLState();
                warning.ErrorCode = javaWarning.getErrorCode();
                warning.NextException = nextException != null ? Tools.toUnoException(this, nextException) : Any.VOID;
                return warning;
            }
            return Any.VOID;
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    // XGeneratedResultSet

    @Override
    public synchronized com.sun.star.sdbc.XResultSet getGeneratedValues() throws SQLException {
        logger.log(LogLevel.FINE, Resources.STR_LOG_GENERATED_VALUES);
        createStatement();
        java.sql.ResultSet jdbcResultSet = null;
        try {
            jdbcResultSet = jdbcStatement.getGeneratedKeys();
        } catch (java.sql.SQLException jdbcException) {
        }

        XResultSet resultSet = null;
        if (jdbcResultSet != null) {
            resultSet = new JavaSQLResultSet(jdbcResultSet, connection, this);
        } else {
            if (connection != null && connection.isAutoRetrievingEnabled()) {
                String statement = connection.getTransformedGeneratedStatement(sqlStatement);
                if (!statement.isEmpty()) {
                    logger.log(LogLevel.FINER, Resources.STR_LOG_GENERATED_VALUES_FALLBACK, statement);
                    CompHelper.disposeComponent(generatedStatement);
                    generatedStatement = connection.createStatement();
                    resultSet = generatedStatement.executeQuery(statement);
                }
            }
        }
        return resultSet;
    }

    // XMultipleResults

    @Override
    public boolean getMoreResults() throws SQLException {
        try {
            createStatement();
            return jdbcStatement.getMoreResults();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public XResultSet getResultSet() throws SQLException {
        try {
            createStatement();
            java.sql.ResultSet jdbcResultSet = jdbcStatement.getResultSet();
            if (jdbcResultSet != null) {
                return new JavaSQLResultSet(jdbcResultSet, connection, this);
            } else {
                return null;
            }
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public int getUpdateCount() throws SQLException {
        try {
            createStatement();
            int count = jdbcStatement.getUpdateCount();
            logger.log(LogLevel.FINER, Resources.STR_LOG_UPDATE_COUNT, count);
            return count;
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }
}
