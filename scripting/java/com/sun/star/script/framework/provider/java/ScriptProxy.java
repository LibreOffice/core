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

