/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FieldDescription.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-04 08:03:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
