/*************************************************************************
*
 *  $RCSfile: AbstractListener.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $  $Date: 2004-11-27 09:07:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.ui.event;

import com.sun.star.awt.XControl;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

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
public class AbstractListener {
    private Hashtable mHashtable = new Hashtable();

    /** Creates a new instance of AbstractListener */
    public AbstractListener() {
    }

    public void add(String componentName, String eventName, String methodName, Object target) {
        try {
            add(componentName, eventName, new MethodInvocation(methodName, target));
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public void add(String componentName, String eventName, MethodInvocation mi) {
        mHashtable.put(componentName + eventName, mi);
    }

    public MethodInvocation get(String componentName, String eventName) {
        return (MethodInvocation) mHashtable.get(componentName + eventName);
    }

    public Object invoke(String componentName, String eventName, Object param) {
        try {
            MethodInvocation mi = get(componentName, eventName);
            if (mi != null)
                return mi.invoke(param);
            else
                return null;
        } catch (InvocationTargetException ite) {

            System.out.println("=======================================================");
            System.out.println("=== Note: An Exception was thrown which should have ===");
            System.out.println("=== caused a crash. I caught it. Please report this ===");
            System.out.println("=== to  openoffice.org                              ===");
            System.out.println("=======================================================");

            ite.printStackTrace();
            ite.getCause().printStackTrace();


        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
        }
        catch (Exception ex) {
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
    public static String getEventSourceName(EventObject eventObject) {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, eventObject.Source);
        return (String) Helper.getUnoPropertyValue(xControl.getModel(), "Name", String.class);
    }
}
