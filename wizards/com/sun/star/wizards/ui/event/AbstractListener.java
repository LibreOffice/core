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

import com.sun.star.awt.XControl;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;

/**
 * This class is a base class for listener classes.
 * It uses a hashtable to map between a ComponentName, EventName and a MethodInvokation Object.
 * To use this class do the following:<br/>
 * <list>
 * <li>Write a subclass which implements the needed Listener(s).</li>
 * in the even methods, use invoke(...).
 * <li>When instanciating the component, register the subclass as the event listener.</li>
 * <li>Write the methods which should be performed when the event occures.</li>
 * <li>call the "add" method, to define a component-event-action mapping.</li>
 * </list>
 */
public class AbstractListener
{

    private HashMap<String,MethodInvocation> mHashtable = new HashMap<String,MethodInvocation>();

    /** Creates a new instance of AbstractListener */
    public AbstractListener()
    {
    }

    public void add(String componentName, String eventName, String methodName, Object target)
    {
        try
        {
            add(componentName, eventName, new MethodInvocation(methodName, target));
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public void add(String componentName, String eventName, MethodInvocation mi)
    {
        mHashtable.put(componentName + eventName, mi);
    }

    public MethodInvocation get(String componentName, String eventName)
    {
        return mHashtable.get(componentName + eventName);
    }

    public Object invoke(String componentName, String eventName, Object param)
    {
        try
        {
            MethodInvocation mi = get(componentName, eventName);
            if (mi != null)
            {
                return mi.invoke(param);
            }
            else
            {
                return null;
            }
        }
        catch (InvocationTargetException ite)
        {

            System.out.println("=======================================================");
            System.out.println("=== Note: An Exception was thrown which should have ===");
            System.out.println("=== caused a crash. I caught it. Please report this ===");
            System.out.println("=== to https://www.libreoffice.org/get-help/bug/    ===");
            System.out.println("=======================================================");

            ite.printStackTrace();

        }
        catch (IllegalAccessException iae)
        {
            iae.printStackTrace();
        }
        catch (Exception ex)
        {
            System.out.println("=======================================================");
            System.out.println("=== Note: An Exception was thrown which should have ===");
            System.out.println("=== caused a crash. I caught it. Please report this ===");
            System.out.println("=== to https://www.libreoffice.org/get-help/bug/    ===");
            System.out.println("=======================================================");
            ex.printStackTrace();
        }

        return null;
    }

    /**
     * Rerurns the property "name" of the Object which is the source of the event.
     */
    public static String getEventSourceName(EventObject eventObject)
    {
        XControl xControl = UnoRuntime.queryInterface(XControl.class, eventObject.Source);
        return (String) Helper.getUnoPropertyValue(xControl.getModel(), PropertyNames.PROPERTY_NAME, String.class);
    }
}
