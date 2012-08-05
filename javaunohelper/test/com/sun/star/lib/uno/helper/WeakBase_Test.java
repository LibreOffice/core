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
import com.sun.star.uno.Type;
import com.sun.star.bridge.XBridgeSupplier2;
import com.sun.star.uno.XReference;
import com.sun.star.uno.XWeak;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.XAdapter;

public class WeakBase_Test
{

    /** Creates a new instance of WeakBase_Test */
    public WeakBase_Test()
    {
    }

    public boolean getTypes()
    {
        System.out.println("Testing WeakBase.getTypes");
        boolean[] r= new boolean[50];
        int i= 0;

        SomeClass comp= new SomeClass();
        Type[] types= comp.getTypes(); //XWeak,XTypeProvider,XReference,XBridgeSupplier2
        r[i++]= types.length == 4;
        for (int c= 0; c < types.length; c++)
        {
            if (types[c].equals( new Type( XWeak.class)))
                r[i++]= true;
            else if (types[c].equals(new Type(XTypeProvider.class)))
                r[i++]= true;
            else if (types[c].equals(new Type(XReference.class)))
                r[i++]= true;
            else if (types[c].equals(new Type(XBridgeSupplier2.class)))
                r[i++]= true;
            else
                r[i++]= false;

        }

        Foo1 f1= new Foo1();
        Foo1 f2= new Foo1();
        Type[] t1= f1.getTypes();
        Type[] t2= f2.getTypes();
        r[i++]= t1.equals(t2);
        Foo2 f3= new Foo2();
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean getImplementationId()
    {
        System.out.println("Testing WeakBase.getImplementationId");
        boolean[] r= new boolean[50];
        int i= 0;

        SomeClass comp= new SomeClass();
        // byte 0 - 3 contain hashcode and the remaining bytes represent the classname
        byte [] ar= comp.getImplementationId();

        StringBuffer buff= new StringBuffer();
        for (int c= 0; c < ar.length - 4; c++){
            buff.append((char) ar[4 + c]);
//            buff.append(" ");
        }
        String retStr= buff.toString();
        r[i++]= retStr.equals("com.sun.star.lib.uno.helper.SomeClass");
//        System.out.println(buff.toString());

        Foo1 f1= new Foo1();
        Foo1 f2= new Foo1();
        r[i++]= f1.getImplementationId().equals(f2.getImplementationId());
        Foo2 f3= new Foo2();
        r[i++]= ! f1.getImplementationId().equals(f3.getImplementationId());
        Foo3 f4= new Foo3();
        r[i++]= ! f1.getImplementationId().equals(f4.getImplementationId());
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean queryAdapter()
    {
        System.out.println("Testing WeakBase.queryAdapter, XAdapter tests");
        boolean[] r= new boolean[50];
        int i= 0;

        SomeClass comp= new SomeClass();
        XAdapter adapter= comp.queryAdapter();
        MyRef aRef1= new MyRef();
        MyRef aRef2= new MyRef();
        adapter.addReference(aRef1);
        adapter.addReference(aRef2);

        r[i++]= adapter.queryAdapted() == comp;
        comp= null;
        System.out.println("Wait 5 sec");
        for(int c= 0; c < 50; c++)
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

        r[i++]= aRef1.nDisposeCalled == 1;
        r[i++]= aRef2.nDisposeCalled == 1;
        r[i++]= adapter.queryAdapted() == null;
        adapter.removeReference(aRef1); // should not do any harm
        adapter.removeReference(aRef2);

        comp= new SomeClass();
        adapter= comp.queryAdapter();
        aRef1.nDisposeCalled= 0;
        aRef2.nDisposeCalled= 0;

        adapter.addReference(aRef1);
        adapter.addReference(aRef2);

        adapter.removeReference(aRef1);
        System.out.println("Wait 5 sec");
        comp= null;
        for(int c= 0; c < 50; c++)
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
        r[i++]= aRef1.nDisposeCalled == 0;
        r[i++]= aRef2.nDisposeCalled == 1;

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
        WeakBase_Test test= new WeakBase_Test();
        boolean r[]= new boolean[50];
        int i= 0;
        r[i++]= test.getTypes();
        r[i++]= test.getImplementationId();
        r[i++]= test.queryAdapter();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Errors occurred!");
        else
            System.out.println("No errors.");

    }

}

interface Aint
{
}
class OtherClass extends WeakBase implements XBridgeSupplier2
{

    public Object createBridge(Object obj, byte[] values, short param, short param3) throws com.sun.star.lang.IllegalArgumentException
    {
        return null;
    }
}

class SomeClass extends OtherClass implements Aint,XReference
{

    public void dispose()
    {
    }

}

class MyRef implements XReference
{
    int nDisposeCalled;

    public void dispose()
    {
        nDisposeCalled++;
    }
}

class Foo1 extends WeakBase
{
}

class Foo2 extends WeakBase
{
}

class Foo3 extends Foo1
{
}
