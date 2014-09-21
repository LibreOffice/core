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

import com.sun.star.uno.XWeak;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XEventListener;
import java.util.logging.Level;

import java.util.logging.Logger;

public class ComponentBase_Test
{
    private static final Logger logger = Logger.getLogger(ComponentBase_Test.class.getName());
    AWeakBase obj1, obj2, obj3;
    Object proxyObj1Weak1;
    Object proxyObj3Weak1;
    Object proxyObj3Weak2;
    Object proxyObj3TypeProv;
    Object proxyObj2TypeProv;

    /** Creates a new instance of ComponentBase_Test */
    public ComponentBase_Test()
    {
        obj1= new AWeakBase();
        obj2= new AWeakBase();
        obj3= new AWeakBase();
        proxyObj1Weak1= ProxyProvider.createProxy(obj1, XWeak.class);
        proxyObj3Weak1= ProxyProvider.createProxy(obj3, XWeak.class);
        proxyObj3Weak2= ProxyProvider.createProxy(obj3, XWeak.class);
        proxyObj2TypeProv= ProxyProvider.createProxy(obj2, XTypeProvider.class);
        proxyObj3TypeProv= ProxyProvider.createProxy(obj3, XTypeProvider.class);
    }

    public boolean dispose()
    {
        logger.log(Level.INFO, "Testing ComponentBase");
        ComponentBase comp= new ComponentBase();
        boolean r[]= new boolean[50];
        int i= 0;

        logger.log(Level.FINE, "addEventListener");
        comp.addEventListener(obj1);
        comp.addEventListener(obj2);
        comp.addEventListener(obj3);
        comp.addEventListener(UnoRuntime.queryInterface(XEventListener.class, proxyObj1Weak1));
        comp.addEventListener(UnoRuntime.queryInterface(XEventListener.class, proxyObj3Weak2));
        comp.addEventListener(UnoRuntime.queryInterface(XEventListener.class, proxyObj3TypeProv));
        obj1.nDisposingCalled = 0;
        obj2.nDisposingCalled = 0;
        obj3.nDisposingCalled = 0;

        comp.dispose();
        r[i++]= obj1.nDisposingCalled == 1;
        r[i++]= obj2.nDisposingCalled == 1;
        r[i++]= obj3.nDisposingCalled == 1;

        logger.log(Level.FINE, "Adding a listener after dispose, causes a immediate call to the listerner.");
        obj1.nDisposingCalled= 0;
        comp.addEventListener(obj1);
        r[i++]= obj1.nDisposingCalled == 1;

        logger.log(Level.FINE, "Calling dispose again must not notify the listeners again.");
        obj1.nDisposingCalled= 0;
        obj2.nDisposingCalled= 0;
        obj3.nDisposingCalled= 0;
        comp.dispose(); // already disposed;
        r[i++]= obj1.nDisposingCalled == 0;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean test_finalize()
    {
        logger.log(Level.INFO, "Testing ComponentBase");
        ComponentBase comp= new ComponentBase();
        boolean r[]= new boolean[50];
        int i= 0;
        obj1.nDisposingCalled = 0;
        comp.addEventListener(obj1);

        comp= null;
        logger.log(Level.FINE, "Waiting 10s");
        for(int c= 0; c < 100; c++)
        {
            try
            {
                Thread.sleep(100);
                System.gc();
                System.runFinalization();
            }catch (InterruptedException ie)
            {
            }
        }
        r[i++]= obj1.nDisposingCalled == 1;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public static void main(String[] args)
    {
        ComponentBase_Test test= new ComponentBase_Test();

        boolean r[]= new boolean[50];
        int i= 0;
        r[i++]= test.dispose();
        r[i++]= test.test_finalize();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "No errors." : "Errors occurred!");
        System.exit( bOk ? 0: -1 );
    }
}