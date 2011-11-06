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
