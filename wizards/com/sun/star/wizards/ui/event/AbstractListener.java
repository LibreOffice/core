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

import com.sun.star.awt.XControl;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;

import java.lang.reflect.InvocationTargetException;
import java.util.Hashtable;

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
 * @author  rpiterman
 */
public class AbstractListener
{

    private Hashtable mHashtable = new Hashtable();

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
        return (MethodInvocation) mHashtable.get(componentName + eventName);
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
            System.out.println("=== to  openoffice.org                              ===");
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
            System.out.println("=== caused a crash. I Catched it. Please report this ==");
            System.out.println("=== to  openoffice.org                               ==");
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
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, eventObject.Source);
        return (String) Helper.getUnoPropertyValue(xControl.getModel(), PropertyNames.PROPERTY_NAME, String.class);
    }
}
