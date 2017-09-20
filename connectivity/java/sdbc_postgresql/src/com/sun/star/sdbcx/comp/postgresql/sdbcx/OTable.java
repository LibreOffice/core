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
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbcx.XAlterTable;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.XIndexesSupplier;
import com.sun.star.sdbcx.XKeysSupplier;
import com.sun.star.sdbcx.XRename;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public abstract class OTable extends ODescriptor
        implements XColumnsSupplier, XKeysSupplier, XNamed, XServiceInfo, XDataDescriptorFactory,
            XIndexesSupplier, XRename, XAlterTable {

    private static String[] services = {
            "com.sun.star.sdbcx.Table"
    };

    private XConnection connection;
    protected String catalogName;
    protected String schemaName;
    protected String description = "";
    protected String type = "";

    protected OContainer keys;
    protected OContainer columns;
    protected OContainer indexes;
    protected OContainer tables;

    protected OTable(String name, boolean isCaseSensitive, XConnection connection, OContainer tables) {
        super(name, isCaseSensitive);
        this.tables = tables;
        this.connection = connection;
        registerProperties();
    }

    private void registerProperties() {
        registerProperty(PropertyIds.CATALOGNAME.name, PropertyIds.CATALOGNAME.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return catalogName;

                    }
                }, null);
        registerProperty(PropertyIds.SCHEMANAME.name, PropertyIds.SCHEMANAME.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return schemaName;

                    }
                }, null);
        registerProperty(PropertyIds.DESCRIPTION.name, PropertyIds.DESCRIPTION.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return description;

                    }
                }, null);
        registerProperty(PropertyIds.TYPE.name, PropertyIds.TYPE.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return type;

                    }
                }, null);
    }

    @Override
    protected void postDisposing() {
        super.postDisposing();
        if (keys != null) {
            keys.dispose();
        }
        if (columns != null) {
            columns.dispose();
        }
        if (indexes != null) {
            indexes.dispose();
        }
        tables = null;
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
            if (serviceName.equals(service)) {
                return true;
            }
        }
        return false;
    }

    // XColumnsSupplier

    @Override
    public synchronized XNameAccess getColumns() {
        if (columns == null) {
            columns = refreshColumns();
        }
        return columns;
    }

    @Override
    public synchronized XNameAccess getIndexes() {
        checkDisposed();
        if (indexes == null) {
            indexes = refreshIndexes();
        }
        return indexes;
    }

    @Override
    public synchronized XIndexAccess getKeys() {
        checkDisposed();
        if (keys == null) {
            keys = refreshKeys();
        }
        return keys;
    }

    public XConnection getConnection() {
        return connection;
    }

    public String getTypeCreatePattern() {
        return "";
    }

    protected abstract OContainer refreshColumns();
    protected abstract OContainer refreshIndexes();
    protected abstract OContainer refreshKeys();
}
