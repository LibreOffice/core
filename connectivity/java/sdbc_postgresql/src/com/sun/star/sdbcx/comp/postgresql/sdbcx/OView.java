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

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertySetter;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class OView extends ODescriptor
        implements XNamed, XServiceInfo {

    private static String[] services = {
            "com.sun.star.sdbcx.View"
    };

    protected String catalogName;
    protected String schemaName;
    protected String command;
    protected int checkOption;

    public OView(String catalog, String schema, String name, boolean isCaseSensitive, String command, int checkOption) {
        super(name, isCaseSensitive, true);
        this.catalogName = catalog;
        this.schemaName = schema;
        this.command = command;
        this.checkOption = checkOption;
        registerProperties();
    }

    private void registerProperties() {
        registerProperty(PropertyIds.CATALOGNAME.name, PropertyIds.CATALOGNAME.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return catalogName;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        catalogName = (String) value;
                    }
                });
        registerProperty(PropertyIds.SCHEMANAME.name, PropertyIds.SCHEMANAME.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return schemaName;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        schemaName = (String) value;
                    }
                });
        registerProperty(PropertyIds.COMMAND.name, PropertyIds.COMMAND.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return command;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        command = (String) value;
                    }
                });
        registerProperty(PropertyIds.CHECKOPTION.name, PropertyIds.CHECKOPTION.id, Type.LONG, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return checkOption;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        checkOption = (int) value;
                    }
                });
    }

    // XServiceInfo

    public String getImplementationName() {
        return getClass().getName();
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
}
