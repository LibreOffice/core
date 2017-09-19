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

package com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertySetter;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.ODescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class SdbcxIndexDescriptor extends ODescriptor implements XColumnsSupplier, XServiceInfo {
    private static final String[] services = {
            "com.sun.star.sdbcx.IndexDescriptor"
    };

    protected String catalog = "";
    protected boolean isUnique;
    protected boolean isClustered;

    private SdbcxIndexColumnDescriptorContainer columns;

    public SdbcxIndexDescriptor(boolean isCaseSensitive) {
        super("", isCaseSensitive, false);
        columns = new SdbcxIndexColumnDescriptorContainer(this, isCaseSensitive());
        registerProperties();
    }

    private void registerProperties() {
        registerProperty(PropertyIds.CATALOG.name, PropertyIds.CATALOG.id, Type.STRING, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return catalog;
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        catalog = (String) value;
                    }
                });
        registerProperty(PropertyIds.ISUNIQUE.name, PropertyIds.ISUNIQUE.id, Type.BOOLEAN, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return isUnique;
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        isUnique = (boolean) value;
                    }
                });
        registerProperty(PropertyIds.ISCLUSTERED.name, PropertyIds.ISCLUSTERED.id, Type.BOOLEAN, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return isClustered;
                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        isClustered = (boolean) value;
                    }
                });
    }

    public SdbcxIndexColumnDescriptorContainer getColumns() {
        return columns;
    }

    // XServiceInfo

    @Override
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
