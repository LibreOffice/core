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
        XControl xControl = UnoRuntime.queryInterface(XControl.class, eventObject.Source);
        return (String) Helper.getUnoPropertyValue(xControl.getModel(), PropertyNames.PROPERTY_NAME, String.class);
    }
}
