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
import org.apache.openoffice.comp.sdbc.dbtools.comphelper.ResourceBasedEventLogger;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.DriverPropertyInfo;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDriver;
import com.sun.star.uno.Any;
import com.sun.star.uno.XComponentContext;

public class JDBCDriver extends ComponentBase implements XServiceInfo, XDriver {
    private static String[] services = new String[] {
            "com.sun.star.sdbc.Driver"
    };
    private XComponentContext context;
    private final ResourceBasedEventLogger logger;

    public static XSingleComponentFactory __getComponentFactory(String implName) {
        XSingleComponentFactory xSingleComponentFactory = null;
        if (implName.equals(getImplementationNameStatic())) {
            xSingleComponentFactory = Factory.createComponentFactory(JDBCDriver.class,
                    getImplementationNameStatic(), services);
        }
        return xSingleComponentFactory;
    }

    public JDBCDriver(XComponentContext componentContext) {
        this.context = componentContext;
        SharedResources.registerClient(componentContext);
        logger = new ResourceBasedEventLogger(context, "sdbcl", "org.openoffice.sdbc.jdbcBridge");
    }

    public XComponentContext getContext() {
        return context;
    }

    public ResourceBasedEventLogger getLogger() {
        return logger;
    }

    private static String getImplementationNameStatic() {
        // this name is referenced in the configuration and in the jdbc.xml
        // Please take care when changing it.
        return JDBCDriver.class.getName();
    }

    // XComponent:

    @Override
    protected synchronized void postDisposing() {
        context = null;
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
        // don't ask the real driver for the url
        // I feel responsible for all jdbc url's
        return url.startsWith("jdbc:");
    }

    @Override
    public synchronized XConnection connect(String url, PropertyValue[] info) throws SQLException {
        checkDisposed();
        logger.log(LogLevel.INFO, Resources.STR_LOG_DRIVER_CONNECTING_URL, url);
        XConnection out = null;
        if (acceptsURL(url)) {
            JavaSQLConnection connection = new JavaSQLConnection(this);
            out = connection;
            if (!connection.construct(url, info)) {
                // an error occurred and the java driver didn't throw an exception
                CompHelper.disposeComponent(out);
                out = null;
            } else {
                logger.log(LogLevel.INFO, Resources.STR_LOG_DRIVER_SUCCESS);
            }
        }
        return out;
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
            String message = SharedResources.getInstance().getResourceString(Resources.STR_URI_SYNTAX_ERROR);
            throw new SQLException(message, this, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, Any.VOID);
        }
        String[] booleanValues = { "false", "true" };
        return new DriverPropertyInfo [] {
                new DriverPropertyInfo(
                        "JavaDriverClass", "The JDBC driver class name.",
                        true, "", new String[0]),
                new DriverPropertyInfo(
                        "JavaDriverClassPath", "The class path where to look for the JDBC driver.",
                        true, "", new String[0]),
                new DriverPropertyInfo(
                        "SystemProperties", "Additional properties to set at java.lang.System before loading the driver.",
                        true, "", new String[0]),
                new DriverPropertyInfo(
                        "ParameterNameSubstitution", "Change named parameters with '?'.",
                        false, "false", booleanValues),
                new DriverPropertyInfo(
                        "IgnoreDriverPrivileges", "Ignore the privileges from the database driver.",
                        false, "false", booleanValues),
                new DriverPropertyInfo(
                        "IsAutoRetrievingEnabled", "Retrieve generated values.",
                        false, "false", booleanValues),
                new DriverPropertyInfo(
                        "AutoRetrievingStatement", "Auto-increment statement.",
                        false, "", new String[0]),
                new DriverPropertyInfo(
                        "GenerateASBeforeCorrelationName", "Generate AS before table correlation names.",
                        false, "true", booleanValues),
                new DriverPropertyInfo(
                        "IgnoreCurrency", "Ignore the currency field from the ResultsetMetaData.",
                        false, "false", booleanValues),
                new DriverPropertyInfo(
                        "EscapeDateTime", "Escape date time format.",
                        false, "true", booleanValues),
                new DriverPropertyInfo(
                        "TypeInfoSettings", "Defines how the type info of the database metadata should be manipulated.",
                        false, "", new String[0]),
                new DriverPropertyInfo(
                        "ImplicitCatalogRestriction", "The catalog which should be used in getTables calls, when the caller passed NULL.",
                        false, "", new String[0]),
                new DriverPropertyInfo(
                        "ImplicitSchemaRestriction", "The schema which should be used in getTables calls, when the caller passed NULL.",
                        false, "", new String[0])
        };

    }
}