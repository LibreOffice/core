/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FieldDescription.java,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
