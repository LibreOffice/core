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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XKeysSupplier;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertySetter;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.ODescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class SdbcxTableDescriptor extends ODescriptor implements XColumnsSupplier, XKeysSupplier {
    protected String catalogName;
    protected String schemaName;
    protected String description;

    private OContainer columns;
    private OContainer keys;

    public SdbcxTableDescriptor(boolean isCaseSensitive) {
        super("", isCaseSensitive, false);
        columns = new SdbcxColumnDescriptorContainer(this, isCaseSensitive());
        keys = new SdbcxKeyDescriptorContainer(this, isCaseSensitive());
        registerProperties();
    }

    private void registerProperties() {
        registerProperty(PropertyIds.CATALOGNAME.name, PropertyIds.CATALOGNAME.id, Type.STRING, (short)0,
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
        registerProperty(PropertyIds.SCHEMANAME.name, PropertyIds.SCHEMANAME.id, Type.STRING, (short)0,
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
        registerProperty(PropertyIds.DESCRIPTION.name, PropertyIds.DESCRIPTION.id, Type.STRING, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return description;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        description = (String) value;
                    }
                });
    }

    @Override
    public XNameAccess getColumns() {
        return columns;
    }

    @Override
    public XIndexAccess getKeys() {
        return keys;
    }
}
