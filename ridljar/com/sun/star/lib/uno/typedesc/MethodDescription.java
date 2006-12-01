/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MethodDescription.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:46:15 $
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
