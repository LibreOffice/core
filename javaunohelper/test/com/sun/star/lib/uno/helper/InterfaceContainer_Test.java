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

import com.sun.star.uno.XInterface;
import java.util.Iterator;
import java.util.ListIterator;
import com.sun.star.uno.Type;
import com.sun.star.uno.XWeak;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XEventListener;
import java.util.ArrayList;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.IQueryInterface;
//import com.sun.star.lib.uno.environments.java.IRequester;
import com.sun.star.bridge.XBridge;
//import com.sun.star.lib.uno.environments.java.Proxy;
import com.sun.star.lib.uno.environments.java.java_environment;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import java.util.HashMap;
import java.util.List;
import java.util.NoSuchElementException;

public class InterfaceContainer_Test
{
    java_environment env= new java_environment(null);
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

    public InterfaceContainer_Test()
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

    /** Tests add(object), size(), clear();
     */
    public boolean add()
    {
        System.out.println("Testing List.add(Object), List.size(), List.clear(), List.isEmpty()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        r[i++]= cont.size() == 0;
        r[i++]= cont.add(obj1);
        r[i++]= cont.size() == 1;
        r[i++]= cont.add(obj1); // add the same object again
        r[i++]= cont.size() == 2;
        r[i++]= cont.add(proxyObj3TypeProv);
        r[i++]= cont.size() == 3;
        r[i++]= cont.isEmpty() ? false: true;
        cont.clear();
        r[i++]= cont.size() == 0;
        r[i++]= cont.isEmpty();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    /**Tests list.addAll(Collection c), list.addAll(int index, Collection c)
     */
    public boolean listConstructors()
    {
        System.out.println("Testing Constructors of InterfaceContainer");
        boolean r[]= new boolean[50];
        int i= 0;
        InterfaceContainer cont= new InterfaceContainer(100);

        r[i++]= cont.elementData.length == 100;
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    public boolean trimToSize()
    {
        System.out.println("Testing InterfaceContainer.trimToSize");
        InterfaceContainer cont= new InterfaceContainer(100);
        boolean r[]= new boolean[50];
        int i= 0;
        cont.trimToSize();
        r[i++]= cont.isEmpty();
        cont= new InterfaceContainer(10);
        cont.addAll(list1);
        cont.trimToSize();
        r[i++]= cont.elementData.length == list1.size();
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    public boolean ensureCapacity()
    {
        System.out.println("Testing InterfaceContainer.ensureCapacity");
        InterfaceContainer cont= new InterfaceContainer(10);
        boolean r[]= new boolean[50];
        int i= 0;

        cont.ensureCapacity(9);
        r[i++]= cont.elementData.length >= 9;
        cont.ensureCapacity(11);
        r[i++]= cont.elementData.length >= 11;
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean addAll()
    {
        System.out.println("Testing List.addAll(Collection c), List.addAll(int index, Collection c)");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        r[i++]= cont.addAll(list1);
        r[i++]= cont.size() == list1.size();
        for (int c= 0; c < cont.size(); c++)
        {
            r[i++]= list1.get(c) == cont.get(c);
        }
        cont.add(obj1);
        r[i++]= cont.addAll(1, list2);
        r[i++]= cont.get(0) == list1.get(0);
        r[i++]= cont.get(1) == list2.get(0);
        r[i++]= cont.get(2) == list2.get(1);
        r[i++]= cont.get(3) == list2.get(2);
        r[i++]= cont.get(4) == list1.get(1);
        r[i++]= cont.get(5) == list1.get(2);
        r[i++]= cont.get(6) == obj1;
        cont.clear();
        cont.addAll(list3);
        // the null element in list3 at index 1 is not added to cont
        r[i++]= cont.get(0) == list3.get(0);
        r[i++]= cont.get(1) == list3.get(2);
        r[i++]= cont.get(2) == list3.get(3);

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    /** Tests List.add(int index, Object element), List.get(int index)
     */
    public boolean get()
    {
        System.out.println("Testing List.add(int index, Object element), List.get(int index)");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.add(0, obj1);
        cont.add(1, obj2);
        cont.add(1, proxyObj3Weak1);
        cont.add(3, obj3);
        r[i++]= cont.get(0) == obj1;
        r[i++]= cont.get(1) == proxyObj3Weak1;
        r[i++]= cont.get(2) == obj2;
        r[i++]= cont.get(3) == obj3;
        try
        {
            cont.add(5, obj1);
        }catch( java.lang.Exception e)
        {
            r[i++]= true;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (bOk == false)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    /** Tests List.contains
     */
    public boolean contains()
    {
        System.out.println("Testing List.contains()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        r[i++]= cont.contains(obj1) ? false : true; // nothing in the list
        cont.add(obj1);
        cont.add(proxyObj2TypeProv);
        cont.add(proxyObj3TypeProv);
        r[i++]= cont.contains(obj1);
        r[i++]= cont.contains(obj2);
        r[i++]= cont.contains(proxyObj3Weak1);
        r[i++]= cont.contains(proxyObj3Weak2);
        r[i++]= cont.contains(proxyObj1Weak1);
        r[i++]= cont.contains(obj3);
        r[i++]= cont.contains(null) ? false : true;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    /** Tests List.containsAll
     */
    public boolean containsAll()
    {
        System.out.println("Testing List.containsAll");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
        cont.addAll(list1);
        r[i++]= cont.containsAll(list1);
        cont.clear();
        cont.addAll(list2);
        r[i++]= cont.containsAll(list2);
        cont.clear();
        cont.addAll(list3); // the null element in list3 is not added to cont
        r[i++]= cont.containsAll(list3) ? false : true;
        cont.clear();
        for( int x= 0; x < 6; x++)
            cont.add(obj4);
        r[i++]= cont.contains(list1) ? false : true;
        cont.add(1, list1.get(0));
        cont.add(3, list1.get(1));
        cont.add(5, list1.get(2));
        r[i++]= cont.contains(list1) ? false : true;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    /** Tests List.indexOf, List.lastIndexOf
     */
    public boolean indexOf()
    {
        System.out.println("Testing List.indexOf(Object element), List.lastIndexOf(Object element)");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
        cont.addAll(list1);
        cont.addAll(list1);
        r[i++]= cont.indexOf(obj3) == 2;
        r[i++]= cont.lastIndexOf(obj3) == 5;
        cont.clear();
        cont.addAll(list2);
        cont.addAll(list2);
        r[i++]= cont.indexOf(proxyObj3Weak1) == 2;
        r[i++]= cont.lastIndexOf(proxyObj3Weak2) == 5;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    /** Tests List.remove(int index), List.remove(Object element), List.removeAll(Collection c)
     */
    public boolean remove()
    {
        System.out.println("Testing List.remove(int index), List.remove(Object element), List.removeAll(Collection c)");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addAll(list2);
        r[i++]=  proxyObj2TypeProv.equals(cont.remove(1));
        r[i++]= cont.size() == 2;
        cont.clear();
        cont.addAll(list2);
        r[i++]= cont.remove(obj1);
        r[i++]= cont.remove(proxyObj2TypeProv);
        r[i++]= cont.remove(proxyObj3Weak2);
        r[i++]= cont.isEmpty();
        cont.clear();
        cont.addAll(list3);
        r[i++]= cont.removeAll(list3);
        r[i++]= cont.isEmpty();
        cont.addAll(list2);

        List<Object> list= new ArrayList<Object>();
        list.add(list2.get(0));
        list.add(list2.get(1));
        list.add(proxyObj3Weak2);
        r[i++]= cont.removeAll(list);
        r[i++]= cont.isEmpty();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    /** Tests List.retainAll
     */
    public boolean retainAll()
    {
        System.out.println("Testing List.retainAll(Collection c)");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addAll(list1); //obj1, obj2, obj3
        cont.addAll(list2); //obj1, proxyObj2TypeProv, proxyObj3TypeProv
        List<Object> list = new ArrayList<Object>();
        list.add(obj1);
        list.add(proxyObj3Weak1);

        r[i++]= cont.retainAll(list);
        r[i++]= cont.get(0) == obj1;
        r[i++]= cont.get(1) == obj3;
        r[i++]= cont.get(2) == obj1;
        r[i++]= cont.get(3) == proxyObj3TypeProv;
        r[i++]= 4 == cont.size();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    /** Tests List.set(int index, Object element)
     **/
    public boolean set()
    {
        System.out.println("Testing List.set(int index, Object element)");
        boolean r[]= new boolean[50];
        int i= 0;
        InterfaceContainer cont= new InterfaceContainer();
        cont.addAll(list2);
        Object o1= cont.set(0, obj3);
        Object o2= cont.set(2, proxyObj3Weak1);
        r[i++]= o1 == list2.get(0);
        r[i++]= o2 == list2.get(2);
        r[i++]= cont.get(0) == obj3;
        r[i++]= cont.get(2) == proxyObj3Weak1;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    /** Tests List.toArray(), List.toArray(Object[] a)
     */
    public boolean toArray()
    {
        System.out.println("Testing List.toArray(), List.toArray(Object[] a)");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addAll(list1);
        Object[] ar= cont.toArray();
        Object[] arOrig= list1.toArray();
        r[i++]= ar.length == arOrig.length;

        r[i]= true;
        for (int c= 0; c < ar.length; c++)
            r[i]= r[i] && ar[c] == arOrig[c];

        i++;
        XWeak[] arWeak= new XWeak[3];
        XWeak[] arWeak2= (XWeak[])cont.toArray(arWeak);
        r[i++]= ar.length == arWeak2.length;
        r[i]= true;
        for (int c= 0; c < ar.length; c++)
            r[i]= r[i] && ar[c] == arWeak2[c];

        i++;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean Iterator_next()
    {
        System.out.println("Testing InterfaceContainer.iterator, Iterator.next()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addAll(list1);
        Iterator it= cont.iterator();
        r[i++]= it.next() == list1.get(0);
        r[i++]= it.next() == list1.get(1);
        r[i++]= it.next() == list1.get(2);
        try
        {
            it.next();
        }catch(java.util.NoSuchElementException ne)
        {
            r[i++]= true;
        }catch(Exception e)
        {
            r[i++]= false;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean Iterator_hasNext()
    {
        System.out.println("Testing, Iterator.next()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        Iterator it= cont.iterator();
        r[i++]= ! it.hasNext();
        cont.addAll(list1);
        it= cont.iterator();
        r[i++]= it.hasNext();
        it.next();
        r[i++]= it.hasNext();
        it.next();
        r[i++]= it.hasNext();
        it.next();
        r[i++]= ! it.hasNext();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean Iterator_remove()
    {
        System.out.println("Testing, Iterator.remove()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        Iterator it= cont.iterator();
        try
        {
            it.remove();
        }
        catch( IllegalStateException ie)
        {
            r[i++]= true;
        }
        catch(java.lang.Exception e)
        {
            r[i++]= false;
        }
        cont.add(obj1);
        it= cont.iterator();
        it.next();
        it.remove();
        r[i++]= cont.isEmpty();
        try
        {
            it.remove();
        }
        catch (IllegalStateException ie)
        {
            r[i++]= true;
        }
        catch (Exception e)
        {
            r[i++]= false;
        }
        cont.clear();
        cont.addAll(list1);
        it= cont.iterator();
        while (it.hasNext())
        {
            it.next();
            it.remove();
        }
        r[i++]= cont.isEmpty();

        // 2 iterators, remove must not impair the other iterator
        cont.clear();
        cont.addAll(list1);
        int size= cont.size();
        it= cont.iterator();
        Iterator it2= cont.iterator();
        while (it.hasNext())
        {
            it.next();
            it.remove();
        }
        try
        {
            for( int c= 0; c < size; c++)
                it2.next();
            r[i++]= true;
        }
        catch(Exception e)
        {
            r[i++]= false;
        }
        r[i++]= cont.size() == 0;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    public boolean ListIterator_next()
    {
        System.out.println("Testing InterfaceContainer.listIerator, ListIterator.next()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        cont.addAll(list1);
        Iterator it= cont.listIterator();
        r[i++]= it.next() == list1.get(0);
        r[i++]= it.next() == list1.get(1);
        r[i++]= it.next() == list1.get(2);
        try
        {
            it.next();
        }catch(java.util.NoSuchElementException ne)
        {
            r[i++]= true;
        }catch(Exception e)
        {
            r[i++]= false;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean ListIterator_hasNext()
    {
        System.out.println("Testing ListIterator.hasNext()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        Iterator it= cont.listIterator();
        r[i++]= ! it.hasNext();
        cont.addAll(list1);
        it= cont.listIterator();
        r[i++]= it.hasNext();
        it.next();
        r[i++]= it.hasNext();
        it.next();
        r[i++]= it.hasNext();
        it.next();
        r[i++]= ! it.hasNext();

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean ListIterator_remove()
    {
        System.out.println("Testing ListIterator.remove()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        ListIterator it= cont.listIterator();
        try
        {
            it.remove();
        }
        catch( IllegalStateException ie)
        {
            r[i++]= true;
        }
        catch(java.lang.Exception e)
        {
            r[i++]= false;
        }
        cont.add(obj1);
        it= cont.listIterator();
        it.next();
        it.remove();
        r[i++]= cont.isEmpty();
        try
        {
            it.remove();
        }
        catch (IllegalStateException ie)
        {
            r[i++]= true;
        }
        catch (Exception e)
        {
            r[i++]= false;
        }
        cont.clear();
        cont.addAll(list1);
        it= cont.listIterator();
        while (it.hasNext())
        {
            it.next();
            it.remove();
        }
        r[i++]= cont.isEmpty();

        // 2 iterators, remove must not impair the other iterator
        cont.clear();
        cont.addAll(list1);
        int size= cont.size();
        it= cont.listIterator();
        Iterator it2= cont.listIterator();
        while (it.hasNext())
        {
            it.next();
            it.remove();
        }
        try
        {
            for( int c= 0; c < size; c++)
                it2.next();
            r[i++]= true;
        }
        catch(Exception e)
        {
            r[i++]= false;
        }
        r[i++]= cont.size() == 0;

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean ListIterator_hasPrevious()
    {
        System.out.println("Testing ListIterator.hasPrevious()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;

        ListIterator it= cont.listIterator();
        r[i++]= ! it.hasPrevious();
        cont.addAll(list1);
        it= cont.listIterator();
        while (it.hasNext())
        {
            it.next();
            r[i++]= it.hasPrevious();
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean ListIterator_previous()
    {
        System.out.println("Testing ListIterator.previous()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
        boolean bOk= true;

        cont.addAll(list1);
        // go to the end of our list and list1
        ListIterator it= cont.listIterator();
        while (it.hasNext())
            it.next();
        ListIterator it_list1= list1.listIterator();
        while (it_list1.hasNext())
            it_list1.next();

        while (it.hasPrevious())
        {
            r[i++]= it.previous() == it_list1.previous();
        }
        try
        {
            it.previous();
            r[i++]= false;
        }
        catch (java.util.NoSuchElementException e)
        {
            r[i++]=true;
        }

        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean ListIterator_nextIndex()
    {
        System.out.println("Testing ListIterator.nextIndex()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
        boolean bOk= true;

        ListIterator it= cont.listIterator();
        cont.addAll(list1);
        it= cont.listIterator();
        r[i++]= it.nextIndex() == 0;
        it.next();
        r[i++]= it.nextIndex() == 1;
        it.next();
        r[i++]= it.nextIndex() == 2;

        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    public boolean ListIterator_previousIndex()
    {
        System.out.println("Testing ListIterator.previousIndex()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
        boolean bOk= true;

        ListIterator it= cont.listIterator();
        cont.addAll(list1);
        it= cont.listIterator();
        while (it.hasNext())
            it.next();

        r[i++]= it.previousIndex() == 2;
        it.previous();
        r[i++]= it.previousIndex() == 1;
        it.previous();
        r[i++]= it.previousIndex() == 0;
        it.previous();

        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }
    public boolean ListIterator_add()
    {
        System.out.println("Testing ListIterator.add()");
        InterfaceContainer cont= new InterfaceContainer();
        boolean r[]= new boolean[50];
        int i= 0;
        boolean bOk= true;

        ListIterator it= cont.listIterator();
        it.add(obj1);
        r[i++]= cont.size() == 1;
        it.add(obj2);
        r[i++]= cont.size() == 2;
        it.add(obj3);
        r[i++]= it.previous() == obj3;
        r[i++]= it.previous() == obj2;
        r[i++]= it.previous() == obj1;

        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    public boolean disposeAndClear()
    {
        System.out.println("Testing InterfaceContainer.disposeAndClear");
        InterfaceContainer cont= new InterfaceContainer(10);
        boolean r[]= new boolean[50];
        int i= 0;

        cont.add(obj1);
        cont.add(obj2);
        cont.add(obj3);
        cont.add(proxyObj1Weak1);
        cont.add(proxyObj3TypeProv);
        System.out.println("Two proxies are called. Check the output:");
        cont.disposeAndClear(new com.sun.star.lang.EventObject());
        r[i++]= obj1.nDisposingCalled == 1;
        r[i++]= obj2.nDisposingCalled == 1;
        r[i++]= obj3.nDisposingCalled == 1;
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
        InterfaceContainer_Test test= new InterfaceContainer_Test();
        boolean r[]= new boolean[50];
        int i= 0;
        r[i++]= test.listConstructors();
        r[i++]= test.trimToSize();
        r[i++]= test.ensureCapacity();
        r[i++]= test.add();
        r[i++]= test.indexOf();
        r[i++]= test.contains();
        r[i++]= test.containsAll();
        r[i++]= test.get();
        r[i++]= test.addAll();
        r[i++]= test.remove();
        r[i++]= test.retainAll();
        r[i++]= test.set();
        r[i++]= test.toArray();
        r[i++]= test.Iterator_next();
        r[i++]= test.Iterator_hasNext();
        r[i++]= test.Iterator_remove();
        r[i++]= test.ListIterator_next();
        r[i++]= test.ListIterator_hasNext();
        r[i++]= test.ListIterator_remove();
        r[i++]= test.ListIterator_hasPrevious();
        r[i++]= test.ListIterator_previous();
        r[i++]= test.ListIterator_nextIndex();
        r[i++]= test.ListIterator_previousIndex();
        r[i++]= test.ListIterator_add();
        r[i++]= test.disposeAndClear();
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if ( ! bOk )
            System.out.println("Test finished.\nErrors occurred!!!");
        else
            System.out.println("Test finished. \nNo errors.");

    }

}
