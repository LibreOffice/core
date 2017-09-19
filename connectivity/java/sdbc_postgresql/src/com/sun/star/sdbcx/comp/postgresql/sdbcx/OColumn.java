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
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertySetter;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxColumnDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class OColumn extends ODescriptor implements XNamed, XDataDescriptorFactory, XServiceInfo {

    private static final String[] services = {
            "com.sun.star.sdbcx.Column"
    };

    private String typeName = "";
    private String description = "";
    private String defaultValue = "";
    private int isNullable;
    private int precision;
    private int scale;
    private int type;
    private boolean isAutoIncrement;
    private boolean isRowVersion;
    private boolean isCurrency;

    public OColumn(final boolean isCaseSensitive) {
        super("", isCaseSensitive);
        this.isNullable = ColumnValue.NULLABLE;
        this.precision = 0;
        this.scale = 0;
        this.type = 0;
        this.isAutoIncrement = false;
        this.isRowVersion = false;
        this.isCurrency = false;
        registerProperties();
    }

    public OColumn(
            final String name,
            final String typeName,
            final String defaultValue,
            final String description,
            final int isNullable,
            final int precision,
            final int scale,
            final int type,
            final boolean isAutoIncrement,
            final boolean isRowVersion,
            final boolean isCurrency,
            final boolean isCaseSensitive) {
        super(name, isCaseSensitive);
        this.typeName = typeName;
        this.description = description;
        this.defaultValue = defaultValue;
        this.isNullable = isNullable;
        this.precision = precision;
        this.scale = scale;
        this.type = type;
        this.isAutoIncrement = isAutoIncrement;
        this.isRowVersion = isRowVersion;
        this.isCurrency = isCurrency;
        registerProperties();
    }

    private void registerProperties() {
        registerProperty(PropertyIds.TYPENAME.name, PropertyIds.TYPENAME.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return typeName;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        typeName = (String) value;
                    }
                });
        registerProperty(PropertyIds.DESCRIPTION.name, PropertyIds.DESCRIPTION.id, Type.STRING, PropertyAttribute.READONLY,
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
        registerProperty(PropertyIds.DEFAULTVALUE.name, PropertyIds.DEFAULTVALUE.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return defaultValue;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        defaultValue = (String) value;
                    }
                });
        registerProperty(PropertyIds.PRECISION.name, PropertyIds.PRECISION.id, Type.LONG, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return precision;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        precision = (Integer) value;
                    }
                });
        registerProperty(PropertyIds.TYPE.name, PropertyIds.TYPE.id, Type.LONG, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return type;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        type = (Integer) value;
                    }
                });
        registerProperty(PropertyIds.SCALE.name, PropertyIds.SCALE.id, Type.LONG, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return scale;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        scale = (Integer) value;
                    }
                });
        registerProperty(PropertyIds.ISNULLABLE.name, PropertyIds.ISNULLABLE.id, Type.LONG, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return isNullable;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        isNullable = (Integer) value;
                    }
                });
        registerProperty(PropertyIds.ISAUTOINCREMENT.name, PropertyIds.ISAUTOINCREMENT.id, Type.BOOLEAN, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return isAutoIncrement;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        isAutoIncrement = (Boolean) value;
                    }
                });
        registerProperty(PropertyIds.ISROWVERSION.name, PropertyIds.ISROWVERSION.id, Type.BOOLEAN, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return isRowVersion;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        isRowVersion = (Boolean) value;
                    }
                });
        registerProperty(PropertyIds.ISCURRENCY.name, PropertyIds.ISCURRENCY.id, Type.BOOLEAN, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return isCurrency;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        isCurrency = (Boolean) value;
                    }
                });
    }

    // XComponent

    @Override
    protected void postDisposing() {
        super.postDisposing();
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

    // XDataDescriptorFactory

    @Override
    public XPropertySet createDataDescriptor() {
        SdbcxColumnDescriptor descriptor = new SdbcxColumnDescriptor(isCaseSensitive());
        synchronized (this) {
            CompHelper.copyProperties(this, descriptor);
        }
        return descriptor;
    }
}
