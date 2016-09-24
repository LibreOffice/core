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

import java.lang.reflect.Method;

/**
 * Allows to examine a method in detail. It gives a view to java methods from a UNO point.
 */
public final class MethodDescription {
    MethodDescription(
        String name, int index, boolean oneway, TypeDescription[] inSignature,
        TypeDescription[] outSignature, TypeDescription returnSignature,
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

    MethodDescription(MethodDescription other, int index) {
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

    /**
     * Indicates if this method is <code>oneWay</code>,
     * respectively if this method may become executed asynchronously.
     * @return  true means may execute asynchronously .
     */
    public boolean isOneway() {
        return oneway;
    }

    /**
     * Indicates if this method is const.
     * @return true means it is const.
     */
    public boolean isConst() {
        return false;
    }

    /**
     * Gives any array of <code>TypeDescription</code> of
     * the [in] parameters.
     * @return the in parameters
     */
    public TypeDescription[] getInSignature() {
        return inSignature;
    }

    /**
     * Gives any array of <code>TypeDescription</code> of
     * the [out] parameters.
     * @return the out parameters
     */
    public TypeDescription[] getOutSignature() {
        return outSignature;
    }

    /**
     * Gives the <code>TypeDescription</code> of
     * the return type.
     * @return the return type <code>TypeDescription</code>
     */
    public TypeDescription getReturnSignature() {
        return returnSignature;
    }

    /**
     * Gives native java method of this method.
     * @return the java method
     */
    public Method getMethod() {
        return method;
    }

    public static final int ID_QUERY_INTERFACE = 0;
    public static final int ID_ACQUIRE = 1;
    public static final int ID_RELEASE = 2;

    private final String name;
    private final int index;
    private final boolean oneway;
    private final TypeDescription[] inSignature;
    private final TypeDescription[] outSignature;
    private final TypeDescription returnSignature;
    private final Method method;
}
