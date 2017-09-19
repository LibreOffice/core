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
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertySetter;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.ODescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class SdbcxColumnDescriptor extends ODescriptor implements XServiceInfo {
    private static final String[] services = {
            "com.sun.star.sdbcx.ColumnDescriptor"
    };

    protected int type;
    protected String typeName = "";
    protected int precision;
    protected int scale;
    protected int isNullable;
    protected boolean isAutoIncrement;
    protected boolean isRowVersion;
    protected String description = "";
    protected String defaultValue = "";
    protected boolean isCurrency;

    public SdbcxColumnDescriptor(boolean isCaseSensitive) {
        super("", isCaseSensitive, false);
        registerProperties();
    }

    private void registerProperties() {
        registerProperty(PropertyIds.TYPE.name, PropertyIds.TYPE.id, Type.LONG, (short)0,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return type;

                    }
                },
                new PropertySetter() {
                    @Override
                    public void setValue(Object value) {
                        type = (int) value;
                    }
                });
        registerProperty(PropertyIds.TYPENAME.name, PropertyIds.TYPENAME.id, Type.STRING, (short)0,
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
        registerProperty(PropertyIds.PRECISION.name, PropertyIds.PRECISION.id, Type.LONG, (short)0,
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
        registerProperty(PropertyIds.SCALE.name, PropertyIds.SCALE.id, Type.LONG, (short)0,
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
        registerProperty(PropertyIds.ISNULLABLE.name, PropertyIds.ISNULLABLE.id, Type.LONG, (short)0,
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
        registerProperty(PropertyIds.ISAUTOINCREMENT.name, PropertyIds.ISAUTOINCREMENT.id, Type.BOOLEAN, (short)0,
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
        registerProperty(PropertyIds.ISROWVERSION.name, PropertyIds.ISROWVERSION.id, Type.BOOLEAN, (short)0,
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
        registerProperty(PropertyIds.DEFAULTVALUE.name, PropertyIds.DEFAULTVALUE.id, Type.STRING, (short)0,
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
        registerProperty(PropertyIds.ISCURRENCY.name, PropertyIds.ISCURRENCY.id, Type.BOOLEAN, (short)0,
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
