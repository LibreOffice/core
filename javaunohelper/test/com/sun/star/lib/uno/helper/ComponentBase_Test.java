/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ComponentBase_Test.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.lib.uno.helper;
import com.sun.star.uno.XWeak;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XEventListener;

public class ComponentBase_Test
{
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
        System.out.println("Testing ComponentBase");
        ComponentBase comp= new ComponentBase();
        boolean r[]= new boolean[50];
        int i= 0;
        // addEventListener

        comp.addEventListener(obj1);
        comp.addEventListener(obj2);
        comp.addEventListener(obj3);
        comp.addEventListener((XEventListener) UnoRuntime.queryInterface(XEventListener.class, proxyObj1Weak1));
        comp.addEventListener((XEventListener) UnoRuntime.queryInterface(XEventListener.class, proxyObj3Weak2));
        comp.addEventListener((XEventListener) UnoRuntime.queryInterface(XEventListener.class, proxyObj3TypeProv));
        obj1.nDisposingCalled = 0;
        obj2.nDisposingCalled = 0;
        obj3.nDisposingCalled = 0;

        comp.dispose();
        r[i++]= obj1.nDisposingCalled == 1;
        r[i++]= obj2.nDisposingCalled == 1;
        r[i++]= obj3.nDisposingCalled == 1;
        // adding a listener after dispose, causes a immediate call to the listerner
        obj1.nDisposingCalled= 0;
        comp.addEventListener(obj1);
        r[i++]= obj1.nDisposingCalled == 1;
        //calling dispose again must not notify the listeners again
        obj1.nDisposingCalled= 0;
        obj2.nDisposingCalled= 0;
        obj3.nDisposingCalled= 0;
        comp.dispose(); // allready disposed;
        r[i++]= obj1.nDisposingCalled == 0;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean test_finalize()
    {
        System.out.println("Testing ComponentBase");
        ComponentBase comp= new ComponentBase();
        boolean r[]= new boolean[50];
        int i= 0;
        obj1.nDisposingCalled = 0;
        comp.addEventListener(obj1);

        comp= null;
        System.out.println("Waiting 10s");
        for(int c= 0; c < 100; c++)
        {
            try
            {
                Thread.currentThread().sleep(100);
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
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
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
        if (bOk == false)
            System.out.println("Errors occured!");
        else
            System.out.println("No errors.");

    }

}

