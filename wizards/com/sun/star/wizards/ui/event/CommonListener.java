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
import java.util.HashMap;

import com.sun.star.awt.*;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;

/**
 * <p>It uses a hashtable to map between a ComponentName, EventName and a MethodInvokation Object.
 * To use this class do the following:</p>
 * <ul>
 * <li>Write a subclass which implements the needed Listener(s).</li>
 * <li>In the even methods, use invoke(...).</li>
 * <li>When instantiating the component, register the subclass as the event listener.</li>
 * <li>Write the methods which should be performed when the event occurs.</li>
 * <li>call the "add" method, to define a component-event-action mapping.</li>
 * </ul>
 */
public class CommonListener implements XActionListener, XItemListener, XTextListener, XWindowListener, XMouseListener, XFocusListener, XKeyListener
{

    private final HashMap<String,MethodInvocation> mHashtable = new HashMap<String,MethodInvocation>();

    public void add(String componentName, EventNames eventName, String methodName, Object target)
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

    public void add(String componentName, EventNames eventName, MethodInvocation mi)
    {
        mHashtable.put(componentName + eventName, mi);
    }

    private MethodInvocation get(String componentName, EventNames eventName)
    {
        return mHashtable.get(componentName + eventName);
    }

    private Object invoke(String componentName, EventNames eventName, Object param)
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
     * Returns the property "name" of the Object which is the source of the event.
     */
    private static String getEventSourceName(EventObject eventObject)
    {
        XControl xControl = UnoRuntime.queryInterface(XControl.class, eventObject.Source);
        return (String) Helper.getUnoPropertyValue(xControl.getModel(), PropertyNames.PROPERTY_NAME, String.class);
    }

    /**
     * Implementation of com.sun.star.awt.XActionListener
     */
    public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
    {
        invoke(getEventSourceName(actionEvent), EventNames.ACTION_PERFORMED, actionEvent);
    }

    public void disposing(com.sun.star.lang.EventObject eventObject)
    {
    }

    /**
     * Implementation of com.sun.star.awt.XItemListener
     */
    public void itemStateChanged(ItemEvent itemEvent)
    {
        invoke(getEventSourceName(itemEvent), EventNames.ITEM_CHANGED, itemEvent);
    }

    /**
     * Implementation of com.sun.star.awt.XTextListener
     */
    public void textChanged(TextEvent textEvent)
    {
        invoke(getEventSourceName(textEvent), EventNames.TEXT_CHANGED, textEvent);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowResized(com.sun.star.awt.WindowEvent)
     */
    public void windowResized(WindowEvent event)
    {
        invoke(getEventSourceName(event), EventNames.WINDOW_RESIZED, event);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowMoved(com.sun.star.awt.WindowEvent)
     */
    public void windowMoved(WindowEvent event)
    {
        invoke(getEventSourceName(event), EventNames.WINDOW_MOVED, event);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowShown(com.sun.star.lang.EventObject)
     */
    public void windowShown(EventObject event)
    {
        invoke(getEventSourceName(event), EventNames.WINDOW_SHOWN, event);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowHidden(com.sun.star.lang.EventObject)
     */
    public void windowHidden(EventObject event)
    {
        invoke(getEventSourceName(event), EventNames.WINDOW_HIDDEN, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mousePressed(com.sun.star.awt.MouseEvent)
     */
    public void mousePressed(MouseEvent event)
    {
        invoke(getEventSourceName(event), EventNames.MOUSE_PRESSED, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mouseReleased(com.sun.star.awt.MouseEvent)
     */
    public void mouseReleased(MouseEvent event)
    {
        invoke(getEventSourceName(event), EventNames.KEY_RELEASED, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mouseEntered(com.sun.star.awt.MouseEvent)
     */
    public void mouseEntered(MouseEvent event)
    {
        invoke(getEventSourceName(event), EventNames.MOUSE_ENTERED, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mouseExited(com.sun.star.awt.MouseEvent)
     */
    public void mouseExited(MouseEvent event)
    {
        invoke(getEventSourceName(event), EventNames.MOUSE_EXITED, event);
    }

    /**
     * @see com.sun.star.awt.XFocusListener#focusGained(com.sun.star.awt.FocusEvent)
     */
    public void focusGained(FocusEvent event)
    {
        invoke(getEventSourceName(event), EventNames.FOCUS_GAINED, event);
    }

    /**
     * @see com.sun.star.awt.XFocusListener#focusLost(com.sun.star.awt.FocusEvent)
     */
    public void focusLost(FocusEvent event)
    {
        invoke(getEventSourceName(event), EventNames.FOCUS_LOST, event);
    }

    /**
     * @see com.sun.star.awt.XKeyListener#keyPressed(com.sun.star.awt.KeyEvent)
     */
    public void keyPressed(KeyEvent event)
    {
        invoke(getEventSourceName(event), EventNames.KEY_PRESSED, event);
    }

    /**
     * @see com.sun.star.awt.XKeyListener#keyReleased(com.sun.star.awt.KeyEvent)
     */
    public void keyReleased(KeyEvent event)
    {
        invoke(getEventSourceName(event), EventNames.KEY_RELEASED, event);
    }
}
