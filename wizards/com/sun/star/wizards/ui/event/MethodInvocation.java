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
 * <p>In the constructor one defines a method, a target object and an optional
 * Parameter.</p>
 * <p>Then one calls "invoke", with or without a parameter.</p>
 * <p>Limitations: I do not check anything myself. If the param is not ok, from the
 * wrong type, or the method does not exist on the given object.
 * You can trick this class how much you want: it will all throw exceptions
 * on the java level. i throw no error warnings or my own exceptions...</p>
 */
public class MethodInvocation
{
    //the method to invoke.
    private Method mMethod;
    //the object to invoke the method on.
    private Object mObject;
    //with one Parameter / without
    private boolean mWithParam;

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

    private MethodInvocation(Method method, Object obj, Class<?> paramClass)
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


}
