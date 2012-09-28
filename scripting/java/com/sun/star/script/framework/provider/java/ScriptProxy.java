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

package com.sun.star.script.framework.provider.java;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

/**
 * A ScriptProxy object acts as a proxy for a Java <code>Method</code>
 *
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

