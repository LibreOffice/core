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
 */
public class MethodInvocation
{
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

    public MethodInvocation(String methodName, Object obj, Class<?> paramClass) throws NoSuchMethodException
    {
        this(paramClass == null ? obj.getClass().getMethod(methodName) : obj.getClass().getMethod(methodName, paramClass), obj, paramClass);
    }

    public MethodInvocation(Method method, Object obj, Class<?> paramClass)
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
            return mMethod.invoke(mObject, param);
        }
        else
        {
            return mMethod.invoke(mObject);
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
