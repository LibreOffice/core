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

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.sdbc.DriverPropertyInfo;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDriver;
import com.sun.star.sdbc.XDriverManager;
import com.sun.star.sdbcx.XDataDefinitionSupplier;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.sdbcx.comp.postgresql.util.Resources;
import com.sun.star.sdbcx.comp.postgresql.util.SharedResources;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class PostgresqlDriver extends ComponentBase implements XServiceInfo, XDriver, XDataDefinitionSupplier {
    private static String[] services = new String[] {
            "com.sun.star.sdbc.Driver",
            "com.sun.star.sdbcx.Driver"
    };
    private XComponentContext componentContext;

    public static XSingleComponentFactory __getComponentFactory(String implName) {
        XSingleComponentFactory xSingleComponentFactory = null;
        if (implName.equals(getImplementationNameStatic())) {
            xSingleComponentFactory = Factory.createComponentFactory(PostgresqlDriver.class,
                    getImplementationNameStatic(), services);
        }
        return xSingleComponentFactory;
    }

    public PostgresqlDriver(XComponentContext componentContext) {
        this.componentContext = componentContext;
        SharedResources.registerClient(componentContext);
    }

    private static String getImplementationNameStatic() {
        return PostgresqlDriver.class.getName();
    }

    // XComponent:

    @Override
    protected synchronized void postDisposing() {
        componentContext = null;
        SharedResources.revokeClient();
    }

    // XServiceInfo:

    @Override
    public String getImplementationName() {
        return getImplementationNameStatic();
    }

    @Override
    public String[] getSupportedServiceNames() {
        return services.clone();
    }

    @Override
    public boolean supportsService(String serviceName) {
        for (String service : getSupportedServiceNames()) {
            if (service.equals(serviceName)) {
                return true;
            }
        }
        return false;
    }

    // XDriver:

    @Override
    public boolean acceptsURL(String url) throws SQLException {
        return url.startsWith("sdbc:postgresql:jdbc:");
    }

    @Override
    public synchronized XConnection connect(String url, PropertyValue[] info) throws SQLException {
        checkDisposed();
        XConnection connection = null;
        if (acceptsURL(url)) {
            String jdbcUrl = transformUrl(url);
            System.out.println("Using SDBC URL " + url + " and JDBC URL " + jdbcUrl);

            try {
                Object driverManagerObject = componentContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.sdbc.DriverManager", componentContext);
                XDriverManager driverManager = UnoRuntime.queryInterface(XDriverManager.class, driverManagerObject);

                ArrayList<PropertyValue> properties = new ArrayList<>();
                boolean haveJavaClass = false;
                for (PropertyValue property : info) {
                    if (property.Name.equals("JavaDriverClass")) {
                        haveJavaClass = true;
                    }
                    properties.add(property);
                }
                if (!haveJavaClass) {
                    PropertyValue javaClassProperty = new PropertyValue();
                    javaClassProperty.Name = "JavaDriverClass";
                    javaClassProperty.Value = "org.postgresql.Driver";
                    properties.add(javaClassProperty);
                }
                PropertyValue[] jdbcInfo = properties.toArray(new PropertyValue[properties.size()]);

                connection = driverManager.getConnectionWithInfo(jdbcUrl, jdbcInfo);
                if (connection != null) {
                    connection = new PostgresqlConnection(connection, url);
                }
            } catch (SQLException sqlException) {
                throw sqlException;
            } catch (Exception exception) {
                throw new SQLException(exception.getMessage(), this, StandardSQLState.SQL_UNABLE_TO_CONNECT.text(), 0, exception);
            }
        }
        return connection;
    }

    @Override
    public int getMajorVersion() {
        return 1;
    }

    @Override
    public int getMinorVersion() {
        return 0;
    }

    @Override
    public DriverPropertyInfo[] getPropertyInfo(String url, PropertyValue[] info) throws SQLException {
        if (!acceptsURL(url)) {
            return new DriverPropertyInfo[0];
        }
        return new DriverPropertyInfo [] {
                new DriverPropertyInfo("JavaClassName", "The JDBC driver class name.", true,
                        "com.postgresql.Driver", new String[0]),
        };
    }

    private static String transformUrl(String url) {
        // 012345678901234567890
        // sdbc:postgresql:jdbc:
        return "jdbc:postgresql:" + url.substring(21);
    }

    // XDataDefinitionSupplier:

    public synchronized XTablesSupplier getDataDefinitionByConnection(XConnection connection) throws SQLException {
        checkDisposed();
        return new PostgresqlCatalog((PostgresqlConnection)connection);
    }

    public synchronized XTablesSupplier getDataDefinitionByURL(String url, PropertyValue[] info) throws SQLException {
        checkDisposed();
        if (!acceptsURL(url)) {
            String error = SharedResources.getInstance().getResourceString(Resources.STR_URI_SYNTAX_ERROR);
            throw new SQLException(error, this, StandardSQLState.SQL_UNABLE_TO_CONNECT.text(), 0, null);
        }
        return getDataDefinitionByConnection(connect(url, info));
    }
}
