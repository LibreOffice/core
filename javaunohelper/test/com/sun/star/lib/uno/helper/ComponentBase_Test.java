/*************************************************************************
 *
 *  $RCSfile: ComponentBase_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-02 09:44:09 $
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

