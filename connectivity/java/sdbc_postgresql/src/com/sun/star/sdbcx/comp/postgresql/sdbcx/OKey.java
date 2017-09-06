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

import java.util.List;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.DisposedException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxKeyDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class OKey extends ODescriptor
        implements XDataDescriptorFactory, XColumnsSupplier {

    protected OTable table;
    protected String referencedTable;
    protected int type;
    protected int updateRule;
    protected int deleteRule;

    private OContainer columns;

    protected OKey(Object lock, boolean isCaseSensitive) {
        super(lock, "", isCaseSensitive);
        registerProperties();
    }

    protected OKey(Object lock, String name, boolean isCaseSensitive, String referencedTable, int type,
            int updateRule, int deleteRule, List<String> columnNames, OTable table) throws ElementExistException {
        super(lock, name, isCaseSensitive);
        this.referencedTable = referencedTable;
        this.type = type;
        this.updateRule = updateRule;
        this.deleteRule = deleteRule;
        this.table = table;
        registerProperties();
        columns = new OKeyColumnContainer(lock, this, columnNames);
    }

    public static OKey create(String name, boolean isCaseSensitive, String referencedTable, int type,
            int updateRule, int deleteRule, List<String> columnNames, OTable table) throws ElementExistException {
        final Object lock = new Object();
        return new OKey(lock, name, isCaseSensitive, referencedTable, type, updateRule, deleteRule, columnNames, table);
    }

    private void registerProperties() {
        registerProperty(PropertyIds.REFERENCEDTABLE.name, PropertyIds.REFERENCEDTABLE.id, Type.STRING, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return referencedTable;

                    }
                }, null);
        registerProperty(PropertyIds.TYPE.name, PropertyIds.TYPE.id, Type.LONG, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return type;

                    }
                }, null);
        registerProperty(PropertyIds.UPDATERULE.name, PropertyIds.UPDATERULE.id, Type.LONG, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return updateRule;

                    }
                }, null);
        registerProperty(PropertyIds.DELETERULE.name, PropertyIds.DELETERULE.id, Type.LONG, (short)PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return deleteRule;

                    }
                }, null);
    }

    // XComponent

    private void checkDisposed() {
        if (bDisposed) {
            throw new DisposedException();
        }
    }

    // XColumnsSupplier

    @Override
    public XNameAccess getColumns() {
        synchronized (lock) {
            checkDisposed();
            return columns;
        }
    }

    // XDataDescriptionFactory

    public XPropertySet createDataDescriptor() {
        SdbcxKeyDescriptor descriptor = SdbcxKeyDescriptor.create(isCaseSensitive());
        CompHelper.copyProperties(this, descriptor);
        try {
            DbTools.cloneDescriptorColumns(this, descriptor);
        } catch (SQLException sqlException) {
        }
        return descriptor;
    }

    public OTable getTable() {
        return table;
    }

    @Override
    public String toString() {
        return "OKey [referencedTable=" + referencedTable + ", type=" + type + ", updateRule=" + updateRule + ", deleteRule=" + deleteRule
                + ", name=" + getName() + "]";
    }


}
