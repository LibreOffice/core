/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IMethodDescription.java,v $
 * $Revision: 1.6 $
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
     * Gives any array of <code>ITypeDescription> of
     * the [in] parameters.
     * <p>
     * @return  the in parameters
     */
    ITypeDescription[] getInSignature();

    /**
     * Gives any array of <code>ITypeDescription> of
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
