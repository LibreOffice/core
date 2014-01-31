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

package com.sun.star.uno;


import java.lang.reflect.Method;


/**
 * The <code>IMethodDescription</code> allows to examine a method
 * in detail. It gives a view to java methods from a UNO point.
 *
 * @deprecated This interface does not cover all the features supported by the
 * corresponding (unpublished) implementation.  But no client code should need
 * to access this functionality, anyway.
 */
public interface IMethodDescription extends IMemberDescription {
    /**
     * Indicates if this method is <code>oneWay</code>,
     * respectivly if this method may become executed asynchronously.
     * <p>
     * @return  true means may execute asynchronously .
     */
    boolean isOneway();

    /**
     * Indicates if this method is const.
     * <p>
     * @return true means it is const.
     */
    boolean isConst();

    /**
     * Gives any array of <code>ITypeDescription</code> of
     * the [in] parameters.
     * <p>
     * @return  the in parameters
     */
    ITypeDescription[] getInSignature();

    /**
     * Gives any array of <code>ITypeDescription</code> of
     * the [out] parameters.
     * <p>
     * @return  the out parameters
     */
    ITypeDescription[] getOutSignature();

    /**
     * Gives the <code>ITypeDescription</code> of
     * the return type.
     * <p>
     * @return  the return type <code>ITypeDescription</code>
     */
    ITypeDescription getReturnSignature();

    /**
     * Gives native java method of this method.
     * <p>
     * @return  the java methodd
     */
    Method getMethod();
}
