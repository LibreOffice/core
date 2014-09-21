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

    /** Creates a new instance of MultiTypeInterfaceContainer_Test */
    public MultiTypeInterfaceContainer_Test()
    {
        obj1= new AWeakBase();
        obj2= new AWeakBase();
        obj3= new AWeakBase();
        obj4= new AWeakBase();

        proxyObj1Weak1= ProxyProvider.createProxy(obj1, XWeak.class);
        proxyObj3Weak1= ProxyProvider.createProxy(obj3, XWeak.class);
        proxyObj3Weak2= ProxyProvider.createProxy(obj3, XWeak.class);
        proxyObj2TypeProv= ProxyProvider.createProxy(obj2, XTypeProvider.class);
        proxyObj3TypeProv= ProxyProvider.createProxy(obj3, XTypeProvider.class);

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

    public boolean addInterface()
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.addInterface");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();
        boolean r[]= new boolean[200];
        int i= 0;

        int ci= 0;
        ci= cont.addInterface(new Type(XInterface.class), obj1);
        ci= cont.addInterface(new Type(XInterface.class), obj2);
        ci= cont.addInterface(new Type(XInterface.class), obj3);
        r[i++]= ci == 3;
        ci= cont.addInterface(new Type(XWeak.class), obj1);
        ci= cont.addInterface(new Type(XWeak.class), obj2);
        r[i++]= ci ==2;
        ci= cont.addInterface(null,obj1);
        r[i++]= ci == 1;
        ci= cont.addInterface(new Type(XTypeProvider.class), null);
        r[i++]= ci == 0;

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
            r[i++]= ci == 100 -c -1;

        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean getContainedTypes()
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.getContainedTypes");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XWeak.class), obj1);
        cont.addInterface(null, obj1);
        cont.addInterface(new Type(XTypeProvider.class), null);
        Object aObj= new Object();
        cont.addInterface(aObj, obj1);
        cont.addInterface(XSingleComponentFactory.class, obj1);
        Type[] types= cont.getContainedTypes();
        // 3 types and no XTypeProvider
        r[i++]= types.length == 5;
        for (int c= 0; c < types.length; c++)
        {
            if (types[c] == null)
                r[i++]= true;
            else if(types[c].equals( new Type(XTypeProvider.class)))
                r[i++]= false;
            else if(types[c].equals(new Type(XInterface.class)))
                r[i++]= true;
            else if (types[c].equals(new Type(XWeak.class)))
                r[i++]= true;
            else if (types[c].equals(new Type()))
                r[i++]= true;
            else if (types[c].equals(new Type( aObj.getClass())))
                r[i++]= true;
            else if (types[c].equals(new Type(XSingleComponentFactory.class)))
                r[i++]= true;
            else
                r[i++]= false;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean getContainer()
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.getContainedTypes");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XInterface.class), obj2);
        cont.addInterface(new Type(XInterface.class), obj3);
        cont.addInterface(new Type(XWeak.class), obj1);
        cont.addInterface(new Type(XWeak.class), obj2);
        cont.addInterface(null, obj1);
        cont.addInterface(new Type(XTypeProvider.class), null);

        InterfaceContainer icont= null;
        icont= cont.getContainer( new Type(XTypeProvider.class));
        r[i++]= icont.size() == 0;
        icont= cont.getContainer(new Type(XWeak.class));
        r[i++]= icont.size() == 2;
        icont= cont.getContainer(null);
        r[i++]= icont.size() == 1;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean removeInterface()
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.removeInterface");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        int count= 0;
        count= cont.removeInterface( new Type(XTypeProvider.class), obj1);
        r[i++]= count == 0;
        count= cont.removeInterface( new Type(XTypeProvider.class), null);
        r[i++]= count == 0;
        count= cont.removeInterface(null, obj2);
        r[i++]= count == 0;

        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(null, obj1);
        count= cont.removeInterface(null, obj2);
        // count must still be 1
        r[i++]= count == 1;
        count= cont.removeInterface(null, obj1);
        r[i++]= count == 0;
        count= cont.removeInterface(new Type(XInterface.class), obj1);
        r[i++]= count == 0;

          boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean clear()
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.clear");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.clear();
        Type[] types= cont.getContainedTypes();
        r[i++]= types.length == 0;
        cont.addInterface(new Type(XInterface.class), obj1);
        cont.addInterface(new Type(XInterface.class), obj2);
        cont.addInterface(new Type(XInterface.class), obj3);
        cont.addInterface(new Type(XWeak.class), obj1);
        cont.addInterface(new Type(XWeak.class), obj2);
        cont.addInterface(null, obj1);
        cont.addInterface(new Type(XTypeProvider.class), null);
        types= cont.getContainedTypes();
        r[i++]= types.length == 3;
        cont.clear();
        types= cont.getContainedTypes();
        r[i++]= types.length == 0;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean disposeAndClear()
    {
        logger.log(Level.INFO, "Testing MultiTypeInterfaceContainer.disposeAndClear");
        MultiTypeInterfaceContainer cont= new MultiTypeInterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
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

        r[i++]= obj1.nDisposingCalled == 3;
        r[i++]= obj2.nDisposingCalled == 2;
        r[i++]= obj3.nDisposingCalled == 1;
        Type[] types= cont.getContainedTypes();
        r[i++]= types.length == 0;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }


    public static void main(String[] args)
    {
        MultiTypeInterfaceContainer_Test test= new MultiTypeInterfaceContainer_Test();
        boolean r[]= new boolean[50];
        int i= 0;
        r[i++]= test.addInterface();
        r[i++]= test.getContainedTypes();
        r[i++]= test.getContainer();
        r[i++]= test.removeInterface();
        r[i++]= test.clear();
        r[i++]= test.disposeAndClear();
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, "Test finished.");
        logger.log(Level.INFO, bOk ? "No errors." : "Errors occurred!!!");
        System.exit( bOk ? 0: -1 );
    }
}