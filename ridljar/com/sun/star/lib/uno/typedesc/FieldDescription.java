/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.lib.uno.typedesc;

import java.lang.reflect.Field;

/**
 * Describes non method members.
 */
public final class FieldDescription {
    public FieldDescription(
        String name, int index, TypeDescription typeDescription, Field field)
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

    /**
     * Gives the name of this member.
     * <p>
     * @return  the name
     */
    public TypeDescription getTypeDescription() {
        return typeDescription;
    }

    /**
     * Gives native java field of this member.
     * <p>
     * @return  the java field
     */
    public Field getField() {
        return field;
    }

    private final String name;
    private final int index;
    private final TypeDescription typeDescription;
    private final Field field;
}
