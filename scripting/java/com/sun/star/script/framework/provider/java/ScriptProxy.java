/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptProxy.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.script.framework.provider.java;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

/**
 * A ScriptProxy object acts as a proxy for a Java <code>Method</code>
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 * @see        java.lang.reflect.Method
 */
public class ScriptProxy
{
    private Object m_targetObject;
    private Method m_method;


    /**
     * Constructs a <code>ScriptProxy</code> object for the given
     * <code>Method</code>
     *
     * @param  method  Description of the Parameter
     */
    public ScriptProxy( Method method )
    {
        this.m_method = method;
    }


    /**
     * Sets the <code>Object</code> on which the ScriptProxy should invoke
     * the method
     *
     * @param  obj  The new targetObject value
     */
    public void setTargetObject( Object obj )
    {
        m_targetObject = obj;
    }


    /**
     * Invokes the method contained in this <code>ScriptProxy</code>,
     * any exceptions resulting from the invocation will be thrown
     *
     * @param  args                           the arguments to be passed when invoking
     *                                          the method
     * @return                                the Object returned from the method
     *                                          invocation, may be null
     * @exception  IllegalAccessException     Description of the Exception
     * @exception  InvocationTargetException  Description of the Exception
     * @exception  IllegalArgumentException   Description of the Exception
     * @exception  Exception                  Description of the Exception
     * @see                                   java.lang.reflect.Method for the exceptions
     *                                          that may be thrown
     */
    public  Object invoke( Object[] args )
    throws IllegalAccessException, InvocationTargetException,
                IllegalArgumentException
    {
        return m_method.invoke( m_targetObject, args );
    }
}

