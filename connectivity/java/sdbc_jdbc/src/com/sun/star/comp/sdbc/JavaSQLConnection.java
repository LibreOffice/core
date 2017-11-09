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

import java.util.Iterator;
import java.util.Properties;
import java.util.Set;

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.CompHelper;
import org.apache.openoffice.comp.sdbc.dbtools.util.AutoRetrievingBase;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.comp.sdbc.ClassMap.ClassLoaderAndClass;
import com.sun.star.comp.sdbc.ConnectionLog.ObjectType;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.lib.util.WeakMap;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.SQLWarning;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XStringSubstitution;

public class JavaSQLConnection extends ComponentBase
        implements XConnection, XWarningsSupplier, XServiceInfo {

    private static final String[] services = {
            "com.sun.star.sdbc.Connection"
    };
    private static final ClassMap classMap = new ClassMap();

    private AutoRetrievingBase autoRetrievingBase = new AutoRetrievingBase();
    private String url;
    private JDBCDriver driver;
    private ConnectionLog logger;
    private boolean useParameterSubstitution;
    private boolean ignoreDriverPrivileges;
    private boolean ignoreCurrency;
    private Object catalogRestriction;
    private Object schemaRestriction;
    private ClassLoader driverClassLoader;
    private java.sql.Driver driverObject;
    private java.sql.Connection connection;
    private PropertyValue[] connectionInfo;
    private WeakMap statements = new WeakMap();

    public JavaSQLConnection(JDBCDriver driver) {
        this.driver = driver;
        this.logger = new ConnectionLog(driver.getLogger(), ObjectType.CONNECTION);
    }

    // XComponent

    @Override
    protected synchronized void postDisposing() {
        logger.log(LogLevel.INFO, Resources.STR_LOG_SHUTDOWN_CONNECTION);
        try {
            for (Iterator<?> it = statements.keySet().iterator(); it.hasNext();) {
                JavaSQLStatementBase statement = (JavaSQLStatementBase) it.next();
                it.remove();
                CompHelper.disposeComponent(statement);
            }
            if (connection != null) {
                connection.close();
            }
        } catch (java.sql.SQLException sqlException) {
            logger.log(LogLevel.WARNING, sqlException);
        }
    }

    // XCloseable

    public void close() throws SQLException {
        dispose();
    }

    // XServiceInfo

    @Override
    public String getImplementationName() {
        return "com.sun.star.sdbcx.JConnection";
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

    // XWarningsSupplier

    @Override
    public synchronized void clearWarnings() throws SQLException {
        checkDisposed();
        try {
            connection.clearWarnings();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized Object getWarnings() throws SQLException {
        checkDisposed();
        try {
            java.sql.SQLWarning javaWarning = connection.getWarnings();
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

    // XConnection

    @Override
    public void commit() throws SQLException {
        try {
            connection.commit();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized XStatement createStatement() throws SQLException {
        checkDisposed();
        logger.log(LogLevel.FINE, Resources.STR_LOG_CREATE_STATEMENT);
        JavaSQLStatement statement = new JavaSQLStatement(this);
        statements.put(statement, statement);
        logger.log(LogLevel.FINE, Resources.STR_LOG_CREATED_STATEMENT_ID, statement.getStatementObjectId());
        return statement;
    }

    @Override
    public boolean getAutoCommit() throws SQLException {
        try {
            return connection.getAutoCommit();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized String getCatalog() throws SQLException {
        checkDisposed();
        try {
            String catalog = connection.getCatalog();
            if (catalog != null) {
                return catalog;
            } else {
                return "";
            }
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized XDatabaseMetaData getMetaData() throws SQLException {
        checkDisposed();
        try {
            return new JavaSQLDatabaseMetaData(connection.getMetaData(), this);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized int getTransactionIsolation() throws SQLException {
        checkDisposed();
        try {
            return connection.getTransactionIsolation();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized XNameAccess getTypeMap() throws SQLException {
        checkDisposed();
        return null;
    }

    @Override
    public synchronized boolean isClosed() throws SQLException {
        try {
            return connection.isClosed() && bDisposed;
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized boolean isReadOnly() throws SQLException {
        checkDisposed();
        try {
            return connection.isReadOnly();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized String nativeSQL(String sql) throws SQLException {
        checkDisposed();
        try {
            String ret = connection.nativeSQL(sql);
            if (ret == null) {
                // UNO hates null strings
                ret = "";
            }
            logger.log(LogLevel.FINER, Resources.STR_LOG_NATIVE_SQL, sql, ret);
            return ret;
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    @Override
    public synchronized XPreparedStatement prepareCall(String sql) throws SQLException {
        checkDisposed();
        logger.log(LogLevel.FINE, Resources.STR_LOG_PREPARE_CALL, sql);
        String sqlStatement = transformPreparedStatement(sql);
        JavaSQLCallableStatement statement = new JavaSQLCallableStatement(this, sqlStatement);
        statements.put(statement, statement);
        logger.log(LogLevel.FINE, Resources.STR_LOG_PREPARED_CALL_ID, statement.getStatementObjectId());
        return statement;
    }

    @Override
    public synchronized XPreparedStatement prepareStatement(String sql) throws SQLException {
        checkDisposed();
        logger.log(LogLevel.FINE, Resources.STR_LOG_PREPARE_STATEMENT, sql);
        String sqlStatement = transformPreparedStatement(sql);
        JavaSQLPreparedStatement statement = new JavaSQLPreparedStatement(this, sqlStatement);
        statements.put(statement, statement);
        logger.log(LogLevel.FINE, Resources.STR_LOG_PREPARED_STATEMENT_ID, statement.getStatementObjectId());
        return statement;
    }

    @Override
    public void rollback() throws SQLException {
        try {
            connection.rollback();
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public void setAutoCommit(boolean autoCommit) throws SQLException {
        try {
            connection.setAutoCommit(autoCommit);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public void setCatalog(String catalog) throws SQLException {
        try {
            connection.setCatalog(catalog);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public void setReadOnly(boolean readOnly) throws SQLException {
        try {
            connection.setReadOnly(readOnly);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized void setTransactionIsolation(int level) throws SQLException {
        checkDisposed();
        try {
            connection.setTransactionIsolation(level);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoException(this, sqlException);
        }
    }

    @Override
    public synchronized void setTypeMap(XNameAccess arg0) throws SQLException {
        checkDisposed();
        String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "XConnection::setTypeMap");
        throw new SQLException(error, this,
            StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.text(),
            0, Any.VOID);
    }

    // others:

    private static String getOrDefault(PropertyValue[] properties, String name, String defaultValue) throws IllegalArgumentException {
        String ret = defaultValue;
        for (PropertyValue property : properties) {
            if (property.Name.equals(name)) {
                ret = AnyConverter.toString(property.Value);
                break;
            }
        }
        return ret;
    }

    private static boolean getOrDefault(PropertyValue[] properties, String name, boolean defaultValue) throws IllegalArgumentException {
        boolean ret = defaultValue;
        for (PropertyValue property : properties) {
            if (property.Name.equals(name)) {
                ret = AnyConverter.toBoolean(property.Value);
                break;
            }
        }
        return ret;
    }

    private static Object getOrDefault(PropertyValue[] properties, String name, Object defaultValue) throws IllegalArgumentException {
        Object ret = defaultValue;
        for (PropertyValue property : properties) {
            if (property.Name.equals(name)) {
                ret = property.Value;
                break;
            }
        }
        return ret;
    }

    private static NamedValue[] getOrDefault(PropertyValue[] properties, String name, NamedValue[] defaultValue) throws IllegalArgumentException {
        NamedValue[] ret = defaultValue;
        for (PropertyValue property : properties) {
            if (property.Name.equals(name)) {
                Object[] array = (Object[]) AnyConverter.toArray(property.Value);
                ret = new NamedValue[array.length];
                for (int i = 0; i < array.length; i++) {
                    ret[i] = (NamedValue) array[i];
                }
                break;
            }
        }
        return ret;
    }

    public boolean construct(String url, PropertyValue[] info) throws SQLException {
        this.url = url;
        String generatedValueStatement = ""; // contains the statement which should be used when query for automatically generated values
        boolean autoRetrievingEnabled = false; // set to <TRUE/> when we should allow to query for generated values
        String driverClassPath = "";
        String driverClass = "";
        NamedValue[] systemProperties = new NamedValue[0];

        try {
            driverClass = getOrDefault(info, "JavaDriverClass", driverClass);
            driverClassPath = getOrDefault(info, "JavaDriverClassPath", driverClassPath);
            if (driverClassPath.isEmpty()) {
                driverClassPath = getJavaDriverClassPath(driverClass);
            }
            autoRetrievingEnabled = getOrDefault(info, "IsAutoRetrievingEnabled", autoRetrievingEnabled);
            generatedValueStatement = getOrDefault(info, "AutoRetrievingStatement", generatedValueStatement);
            useParameterSubstitution = getOrDefault(info, "ParameterNameSubstitution", useParameterSubstitution);
            ignoreDriverPrivileges = getOrDefault(info, "IgnoreDriverPrivileges", ignoreDriverPrivileges);
            ignoreCurrency = getOrDefault(info, "IgnoreCurrency", ignoreCurrency);
            systemProperties = getOrDefault(info, "SystemProperties", systemProperties);
            catalogRestriction = getOrDefault(info, "ImplicitCatalogRestriction", Any.VOID);
            schemaRestriction = getOrDefault(info, "ImplicitSchemaRestriction", Any.VOID);

            loadDriverFromProperties(driverClass, driverClassPath, systemProperties);

            autoRetrievingBase.setAutoRetrievingEnabled(autoRetrievingEnabled);
            autoRetrievingBase.setAutoRetrievingStatement(generatedValueStatement);

            Properties properties = createStringPropertyArray(info);
            try (ContextClassLoaderScope ccl = new ContextClassLoaderScope(driverClassLoader)) {
                connection = driverObject.connect(url, properties);
            }
            logger.log(LogLevel.INFO, Resources.STR_LOG_GOT_JDBC_CONNECTION, url);
            connectionInfo = info;
            return true;
        } catch (IllegalArgumentException illegalArgumentException) {
            logger.log(LogLevel.SEVERE, illegalArgumentException);
            throw new SQLException("Driver property error", this,
                    StandardSQLState.SQL_GENERAL_ERROR.text(), 0, illegalArgumentException);
        } catch (java.sql.SQLException sqlException) {
            throw Tools.toUnoExceptionLogged(this, logger, sqlException);
        }
    }

    private String getJavaDriverClassPath(String driverClass) {
        String url = "";
        try {
            XMultiServiceFactory configurationProvider = UnoRuntime.queryInterface(XMultiServiceFactory.class,
                    driver.getContext().getServiceManager().createInstanceWithContext(
                            "com.sun.star.configuration.ConfigurationProvider", driver.getContext()));

            PropertyValue[] arguments = new PropertyValue[1];
            arguments[0] = new PropertyValue();
            arguments[0].Name = "nodepath";
            arguments[0].Value = "/org.openoffice.Office.DataAccess/JDBC/DriverClassPaths";

            Object configurationAccess = configurationProvider.createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess", arguments);
            XNameAccess myNode = UnoRuntime.queryInterface(XNameAccess.class, configurationAccess);
            if (myNode.hasByName(driverClass)) {
                XNameAccess driverNode = UnoRuntime.queryInterface(XNameAccess.class, myNode.getByName(driverClass));
                if (driverNode.hasByName("Path")) {
                    url = AnyConverter.toString(driverNode.getByName("Path"));
                }
            }
        } catch (com.sun.star.uno.Exception exception) {
            logger.log(LogLevel.WARNING, exception);
        }
        return url;
    }

    private void loadDriverFromProperties(String driverClassName, String driverClassPath, NamedValue[] properties) throws SQLException {
        if (connection != null) {
            return;
        }
        try {
            setSystemProperties(properties);
            driverClassLoader = null;

            if (driverClassName.isEmpty()) {
                logger.log(LogLevel.SEVERE, Resources.STR_LOG_NO_DRIVER_CLASS);
                throw new SQLException(getDriverLoadErrorMessage(SharedResources.getInstance(), driverClassName, driverClassPath),
                        this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, Any.VOID);
            }
            logger.log(LogLevel.INFO, Resources.STR_LOG_LOADING_DRIVER, driverClassName);
            // the driver manager holds the class of the driver for later use
            Class<?> driverClass;
            if (driverClassPath.isEmpty()) {
                driverClass = Class.forName(driverClassName);
            } else {
                ClassLoaderAndClass classLoaderAndClass = classMap.loadClass(driver.getContext(), driverClassPath, driverClassName);
                driverClassLoader = classLoaderAndClass.getClassLoader();
                driverClass = classLoaderAndClass.getClassObject();
            }
            driverObject = (java.sql.Driver) driverClass.newInstance();
            driverClass = driverObject.getClass();
            logger.log(LogLevel.INFO, Resources.STR_LOG_CONN_SUCCESS);
        } catch (SQLException sqlException) {
            throw new SQLException(
                    getDriverLoadErrorMessage(SharedResources.getInstance(), driverClassName, driverClassPath),
                    this, "", 1000, sqlException);
        } catch (Exception exception) {
            throw new SQLException(
                    getDriverLoadErrorMessage(SharedResources.getInstance(), driverClassName, driverClassPath),
                    this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0,
                    Tools.toUnoExceptionLogged(this, logger, exception));
        }
    }

    private void setSystemProperties(NamedValue[] properties) {
        for (NamedValue namedValue : properties) {
            String value = "";
            try {
                value = AnyConverter.toString(namedValue.Value);
            } catch (IllegalArgumentException illegalArgumentException) {
                logger.log(LogLevel.WARNING, illegalArgumentException);
            }
            logger.log(LogLevel.FINER, Resources.STR_LOG_SETTING_SYSTEM_PROPERTY, namedValue.Name, value);
            System.setProperty(namedValue.Name, value);
        }
    }

    private static String getDriverLoadErrorMessage(SharedResources sharedResouces, String driverClass, String driverClassPath) {
        String error1 = sharedResouces.getResourceStringWithSubstitution(
                Resources.STR_NO_CLASSNAME, "$classname$", driverClass);
        if (!driverClassPath.isEmpty()) {
            String error2 = sharedResouces.getResourceStringWithSubstitution(
                    Resources.STR_NO_CLASSNAME_PATH, "$classpath$", driverClassPath);
            error1 += error2;
        }
        return error1;
    }

    private Properties createStringPropertyArray(PropertyValue[] info) throws IllegalArgumentException {
        Properties properties = new Properties();
        for (PropertyValue propertyValue : info) {
            if (!propertyValue.Name.equals("JavaDriverClass") &&
                    !propertyValue.Name.equals("JavaDriverClassPath") &&
                    !propertyValue.Name.equals("SystemProperties") &&
                    !propertyValue.Name.equals("CharSet") &&
                    !propertyValue.Name.equals("AppendTableAliasName") &&
                    !propertyValue.Name.equals("AddIndexAppendix") &&
                    !propertyValue.Name.equals("FormsCheckRequiredFields") &&
                    !propertyValue.Name.equals("GenerateASBeforeCorrelationName") &&
                    !propertyValue.Name.equals("EscapeDateTime") &&
                    !propertyValue.Name.equals("ParameterNameSubstitution") &&
                    !propertyValue.Name.equals("IsPasswordRequired") &&
                    !propertyValue.Name.equals("IsAutoRetrievingEnabled") &&
                    !propertyValue.Name.equals("AutoRetrievingStatement") &&
                    !propertyValue.Name.equals("UseCatalogInSelect") &&
                    !propertyValue.Name.equals("UseSchemaInSelect") &&
                    !propertyValue.Name.equals("AutoIncrementCreation") &&
                    !propertyValue.Name.equals("Extension") &&
                    !propertyValue.Name.equals("NoNameLengthLimit") &&
                    !propertyValue.Name.equals("EnableSQL92Check") &&
                    !propertyValue.Name.equals("EnableOuterJoinEscape") &&
                    !propertyValue.Name.equals("BooleanComparisonMode") &&
                    !propertyValue.Name.equals("IgnoreCurrency") &&
                    !propertyValue.Name.equals("TypeInfoSettings") &&
                    !propertyValue.Name.equals("IgnoreDriverPrivileges") &&
                    !propertyValue.Name.equals("ImplicitCatalogRestriction") &&
                    !propertyValue.Name.equals("ImplicitSchemaRestriction") &&
                    !propertyValue.Name.equals("SupportsTableCreation") &&
                    !propertyValue.Name.equals("UseJava") &&
                    !propertyValue.Name.equals("Authentication") &&
                    !propertyValue.Name.equals("PreferDosLikeLineEnds") &&
                    !propertyValue.Name.equals("PrimaryKeySupport") &&
                    !propertyValue.Name.equals("RespectDriverResultSetType")) {
                properties.setProperty(propertyValue.Name, AnyConverter.toString(propertyValue.Value));
            }
        }
        return properties;
    }

    private String transformPreparedStatement(String sql) throws SQLException {
        PropertyValue[] properties = new PropertyValue[1];
        properties[0] = new PropertyValue();
        properties[0].Name = "ActiveConnection";
        properties[0].Value = this;
        XComponentContext context = driver.getContext();
        try {
            Object parameterSubstitution = context.getServiceManager().createInstanceWithArgumentsAndContext(
                    "com.sun.star.sdb.ParameterSubstitution", properties, context);
            XStringSubstitution stringSubstitution = UnoRuntime.queryInterface(XStringSubstitution.class, parameterSubstitution);
            return stringSubstitution.substituteVariables(sql, true);
        } catch (com.sun.star.uno.Exception exception) {
            throw Tools.toUnoExceptionLogged(this, logger, exception);
        }
    }

    /** returns the instance used for logging events related to this connection
    */
    public ConnectionLog getLogger() {
        return logger;
    }

    public java.sql.Connection getJDBCConnection() {
        return connection;
    }

    public boolean isAutoRetrievingEnabled() {
        return autoRetrievingBase.isAutoRetrievingEnabled();
    }

    public boolean isIgnoreCurrencyEnabled() {
        return ignoreCurrency;
    }

    public String getTransformedGeneratedStatement(String sql) {
        return autoRetrievingBase.getTransformedGeneratedStatement(sql);
    }

    public ClassLoader getDriverClassLoader() {
        return driverClassLoader;
    }

    public Object getCatalogRestriction() {
        return catalogRestriction;
    }

    public Object getSchemaRestriction() {
        return schemaRestriction;
    }

    public boolean isIgnoreDriverPrivilegesEnabled() {
        return ignoreDriverPrivileges;
    }

    public String getURL() {
        return url;
    }

    public PropertyValue[] getConnectionInfo() {
        return connectionInfo;
    }
}
