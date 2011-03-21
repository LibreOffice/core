/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package com.sun.star.wizards.ui.event;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Encapsulate a Method invocation.
 * In the constructor one defines a method, a target object and an optional
 * Parameter.
 * Then one calls "invoke", with or without a parameter. <br/>
 * Limitations: I do not check anything myself. If the param is not ok, from the
 * wrong type, or the mothod doesnot exist on the given object.
 * You can trick this class howmuch you want: it will all throw exceptions
 * on the java level. i throw no error warnings or my own excceptions...
 * @author  rpiterman
 */
public class MethodInvocation
{

    static final Class[] EMPTY_ARRAY =
    {
    };
    //the method to invoke.
    Method mMethod;
    //the object to invoke the method on.
    Object mObject;
    //with one Parameter / without
    boolean mWithParam;

    /** Creates a new instance of MethodInvokation */
    public MethodInvocation(String methodName, Object obj) throws NoSuchMethodException
    {
        this(methodName, obj, null);
    }

    public MethodInvocation(Method method, Object obj)
    {
        this(method, obj, null);
    }

    public MethodInvocation(String methodName, Object obj, Class paramClass) throws NoSuchMethodException
    {
        this(paramClass == null ? obj.getClass().getMethod(methodName, null) : obj.getClass().getMethod(methodName, new Class[]
                {
                    paramClass
                }), obj, paramClass);
    }

    public MethodInvocation(Method method, Object obj, Class paramClass)
    {
        mMethod = method;
        mObject = obj;
        mWithParam = !(paramClass == null);
    }

    /**
     * Returns the result of calling the method on the object, or null, if no result.
     */
    public Object invoke(Object param) throws IllegalAccessException, InvocationTargetException
    {
        if (mWithParam)
        {
            return mMethod.invoke(mObject, (Object) param
                    );
        }
        else
        {
            return mMethod.invoke(mObject, EMPTY_ARRAY);
        }
    }

    /**
     * This method is a convenience method.
     * It is the same as calling invoke(null);
     */
    public Object invoke() throws IllegalAccessException, InvocationTargetException
    {
        return invoke(null);
    }
}
