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

package com.sun.star.lib.uno.helper;

import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XTypeProvider;

import com.sun.star.uno.XWeak;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;

import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import org.junit.Before;
import org.junit.Test;

public class MultiTypeInterfaceContainer_Test
{
    private static final Logger logger = Logger.getLogger(MultiTypeInterfaceContainer_Test.class.getName());
    /** Creates a new instance of InterfaceContainerTest */
    AWeakBase obj1,obj2,obj3,obj4;
    Object proxyObj1Weak1;
    Object proxyObj3Weak1;
    Object proxyObj3Weak2;
    Object proxyObj3TypeProv;
    Object proxyObj2TypeProv;
    //contains original objects
    List<Object> list1;
    //contains original objects + proxies
    List<Object> list2;
    //contains original object + proxies + null value
    List<Object> list3;

    /** Class variables are initialized before each Test method */
    @Before public void setUp() throws Exception
    {
        obj1= new AWeakBase();
        obj2= new AWeakBase();
        obj3= new AWeakBase();
        obj4= new AWeakBase();

        proxyObj1Weak1= ProxyProvider.createProxy(obj1, XWeak.class);
        proxyObj3Weak1= ProxyProvider.createProxy(obj3, XWeak.class);
        proxyObj3Weak2= ProxyProvider.createProxy(obj3, XWeak.class);
        assertNotNull(proxyObj1Weak1);
        assertNotNull(proxyObj3Weak1);
        assertNotNull(proxyObj3Weak2);

        proxyObj2TypeProv= ProxyProvider.createProxy(obj2, XTypeProvider.class);
        proxyObj3TypeProv= ProxyProvider.createProxy(obj3, XTypeProvider.class);
        assertNotNull(proxyObj2TypeProv);
        assertNotNull(proxyObj3TypeProv);

        list1= new ArrayList<Object>();
        list1.add(obj1);
        list1.add(obj2);
        list1.add(obj3);

        list2= new ArrayList<Object>();
        list2.add(obj1);
        list2.add(proxyObj2TypeProv);
        list2.add(proxyObj3TypeProv);

        list3= new ArrayList<Object>();
        list3.add(obj1);
        list3.add(null);
        list3.add(proxyObj2TypeProv);
        list3.add(proxyObj3Weak1);
    }

    @Test public void addInterface() throws Exception
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.addInterface");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();

        int ci= 0;
        ci= cont.addInterface(new Type(XInterface.class), obj1);
        ci= cont.addInterface(new Type(XInterface.class), obj2);
        ci= cont.addInterface(new Type(XInterface.class), obj3);
        assertEquals(ci, 3);

        ci= cont.addInterface(new Type(XWeak.class), obj1);
        ci= cont.addInterface(new Type(XWeak.class), obj2);
        assertEquals(ci, 2);

        ci= cont.addInterface(null,obj1);
        assertEquals(ci, 1);

        ci= cont.addInterface(new Type(XTypeProvider.class), null);
        assertEquals(ci, 0);

        cont= new MultiTypeInterfaceContainer();
        AWeakBase[] arObj= new AWeakBase[100];
        for (int c= 0; c < 100; c++)
        {
            arObj[c]= new AWeakBase();
            ci= cont.addInterface(new Type(XInterface.class), arObj[c]);
        }
        Type[] arT= cont.getContainedTypes();
        for (int c=0; c < 100; c++)
        {
            ci= cont.removeInterface(new Type(XInterface.class), arObj[c]);
            assertEquals(ci, 100 -c -1);
        }
    }

    @Test public void getContainedTypes() throws Exception
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.getContainedTypes");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();

        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XWeak.class), obj1);
        cont.addInterface(null, obj1);
        cont.addInterface(new Type(XTypeProvider.class), null);
        Object aObj= new Object();
        cont.addInterface(aObj, obj1);
        cont.addInterface(XSingleComponentFactory.class, obj1);
        Type[] types= cont.getContainedTypes();
        // 3 types and no XTypeProvider
        assertEquals(types.length, 5);
        for (int c= 0; c < types.length; c++)
        {
            boolean result= false;
            if (types[c] == null)
                result= true;
            else if (types[c].equals(new Type(XTypeProvider.class)))
                result= false;
            else if (types[c].equals(new Type(XInterface.class)))
                result= true;
            else if (types[c].equals(new Type(XWeak.class)))
                result= true;
            else if (types[c].equals(new Type()))
                result= true;
            else if (types[c].equals(new Type( aObj.getClass())))
                result= true;
            else if (types[c].equals(new Type(XSingleComponentFactory.class)))
                result= true;
            assertTrue(result);
        }
    }

    @Test public void getContainer() throws Exception
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.getContainedTypes");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();

        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XInterface.class), obj2);
        cont.addInterface(new Type(XInterface.class), obj3);
        cont.addInterface(new Type(XWeak.class), obj1);
        cont.addInterface(new Type(XWeak.class), obj2);
        cont.addInterface(null, obj1);
        cont.addInterface(new Type(XTypeProvider.class), null);

        InterfaceContainer icont= null;
        icont= cont.getContainer( new Type(XTypeProvider.class));
        assertEquals(icont.size(), 0);
        icont= cont.getContainer(new Type(XWeak.class));
        assertEquals(icont.size(), 2);
        icont= cont.getContainer(null);
        assertEquals(icont.size(), 1);
    }

    @Test public void removeInterface() throws Exception
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.removeInterface");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();

        int count= 0;
        count= cont.removeInterface( new Type(XTypeProvider.class), obj1);
        assertEquals(count, 0);
        count= cont.removeInterface( new Type(XTypeProvider.class), null);
        assertEquals(count, 0);
        count= cont.removeInterface(null, obj2);
        assertEquals(count, 0);

        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(null, obj1);
        count= cont.removeInterface(null, obj2);
        // count must still be 1
        assertEquals(count, 1);
        count= cont.removeInterface(null, obj1);
        assertEquals(count, 0);
        count= cont.removeInterface(new Type(XInterface.class), obj1);
        assertEquals(count, 0);
    }

    @Test public void clear() throws Exception
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.clear");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();

        cont.clear();
        Type[] types= cont.getContainedTypes();
        assertEquals(types.length, 0);
        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XInterface.class), obj2);
        cont.addInterface(new Type(XInterface.class), obj3);
        cont.addInterface(new Type(XWeak.class), obj1);
        cont.addInterface(new Type(XWeak.class), obj2);
        cont.addInterface(null, obj1);
        cont.addInterface(new Type(XTypeProvider.class), null);
        types= cont.getContainedTypes();
        assertEquals(types.length, 3);
        cont.clear();
        types= cont.getContainedTypes();
        assertEquals(types.length, 0);
    }

    @Test public void disposeAndClear() throws Exception
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.disposeAndClear");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();

        obj1.nDisposingCalled= 0;
        obj2.nDisposingCalled= 0;
        obj3.nDisposingCalled= 0;
        cont.addInterface(new Type(XInterface.class), null);
        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XInterface.class), obj2);
        cont.addInterface(new Type(XInterface.class), obj3);
        cont.addInterface(new Type(XWeak.class),obj1);
        cont.addInterface(new Type(XWeak.class), obj2);
        cont.addInterface(new Type(XTypeProvider.class), obj1);
        cont.disposeAndClear(new com.sun.star.lang.EventObject("blabla"));

        assertEquals(obj1.nDisposingCalled, 3);
        assertEquals(obj2.nDisposingCalled, 2);
        assertEquals(obj3.nDisposingCalled, 1);
        Type[] types= cont.getContainedTypes();
        assertEquals(types.length, 0);
    }
}