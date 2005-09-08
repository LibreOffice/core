/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IMethodDescription.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:23:26 $
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
