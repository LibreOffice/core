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
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxKeyColumnDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.Type;

public class OKeyColumn extends OColumn {
    protected String referencedColumn;

    protected OKeyColumn(boolean isCaseSensitive) {
        super(isCaseSensitive);
        registerProperties();
    }

    protected OKeyColumn(
            final String referencedColumn,
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
        super(name, typeName, defaultValue, description, isNullable,
                precision, scale, type, isAutoIncrement, isRowVersion, isCurrency, isCaseSensitive);
        this.referencedColumn = referencedColumn;
        registerProperties();
    }

    public static OKeyColumn create(
            final String referencedColumn,
            final String name,
            final String typeName,
            final String defaultValue,
            final int isNullable,
            final int precision,
            final int scale,
            final int type,
            final boolean isAutoIncrement,
            final boolean isRowVersion,
            final boolean isCurrency,
            final boolean isCaseSensitive) {
        return new OKeyColumn(referencedColumn, name, typeName,
                defaultValue, "", isNullable, precision, scale,
                type, isAutoIncrement, isRowVersion, isCurrency, isCaseSensitive);
    }

    private void registerProperties() {
        registerProperty(PropertyIds.RELATEDCOLUMN.name, PropertyIds.RELATEDCOLUMN.id, Type.STRING, PropertyAttribute.READONLY,
                new PropertyGetter() {
                    @Override
                    public Object getValue() {
                        return referencedColumn;

                    }
                }, null);
    }

    // XDataDescriptorFactory

    @Override
    public XPropertySet createDataDescriptor() {
        SdbcxKeyColumnDescriptor descriptor = SdbcxKeyColumnDescriptor.create(isCaseSensitive());
        synchronized (this) {
            CompHelper.copyProperties(this, descriptor);
        }
        return descriptor;
    }
}
