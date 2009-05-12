/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MethodDescription.java,v $
 * $Revision: 1.8 $
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

import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import java.lang.reflect.Method;

public final class MethodDescription implements IMethodDescription {
    MethodDescription(
        String name, int index, boolean oneway, ITypeDescription[] inSignature,
        ITypeDescription[] outSignature, ITypeDescription returnSignature,
        Method method)
    {
        this.name = name;
        this.index = index;
        this.oneway = oneway;
        this.inSignature = inSignature;
        this.outSignature = outSignature;
        this.returnSignature = returnSignature;
        this.method = method;
    }

    MethodDescription(IMethodDescription other, int index) {
        this(
            other.getName(), index, other.isOneway(), other.getInSignature(),
            other.getOutSignature(), other.getReturnSignature(),
            other.getMethod());
    }

    public String getName() {
        return name;
    }

    public boolean isUnsigned() {
        return MemberDescriptionHelper.isUnsigned(returnSignature);
    }

    public boolean isAny() {
        return MemberDescriptionHelper.isAny(returnSignature);
    }

    public boolean isInterface() {
        return MemberDescriptionHelper.isInterface(returnSignature);
    }

    public int getIndex() {
        return index;
    }

    public boolean isOneway() {
        return oneway;
    }

    public boolean isConst() {
        return false;
    }

    public ITypeDescription[] getInSignature() {
        return inSignature;
    }

    public ITypeDescription[] getOutSignature() {
        return outSignature;
    }

    public ITypeDescription getReturnSignature() {
        return returnSignature;
    }

    public Method getMethod() {
        return method;
    }

    public static final int ID_QUERY_INTERFACE = 0;
    public static final int ID_ACQUIRE = 1;
    public static final int ID_RELEASE = 2;

    private final String name;
    private final int index;
    private final boolean oneway;
    private final ITypeDescription[] inSignature;
    private final ITypeDescription[] outSignature;
    private final ITypeDescription returnSignature;
    private final Method method;
}
