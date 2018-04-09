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



package com.sun.star.lib.uno.typedesc;

import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.ITypeDescription;
import java.lang.reflect.Field;

final class FieldDescription implements IFieldDescription {
    public FieldDescription(
        String name, int index, ITypeDescription typeDescription, Field field)
    {
        this.name = name;
        this.index = index;
        this.typeDescription = typeDescription;
        this.field = field;
    }

    public String getName() {
        return name;
    }

    public boolean isUnsigned() {
        return MemberDescriptionHelper.isUnsigned(typeDescription);
    }

    public boolean isAny() {
        return MemberDescriptionHelper.isAny(typeDescription);
    }

    public boolean isInterface() {
        return MemberDescriptionHelper.isInterface(typeDescription);
    }

    public int getIndex() {
        return index;
    }

    public ITypeDescription getTypeDescription() {
        return typeDescription;
    }

    public Field getField() {
        return field;
    }

    private final String name;
    private final int index;
    private final ITypeDescription typeDescription;
    private final Field field;
}
