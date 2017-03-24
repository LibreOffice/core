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
import com.sun.star.lib.uno.environments.java.java_environment;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.NoSuchElementException;

import java.util.logging.Level;
import java.util.logging.Logger;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import org.junit.Before;
import org.junit.Test;

public class InterfaceContainer_Test
{
    private static final Logger logger = Logger.getLogger(InterfaceContainer_Test.class.getName());
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

    /** Tests add(object), size(), clear();
     */
    @Test public void add() throws Exception
    {
        logger.log(Level.INFO, "Testing List.add(Object), List.size(), List.clear(), List.isEmpty()");
        InterfaceContainer cont= new InterfaceContainer();

        assertEquals(cont.size(), 0);

        assertTrue(cont.add(obj1));
        assertEquals(cont.size(), 1);

        assertTrue(cont.add(obj1)); // add the same object again
        assertEquals(cont.size(), 2);

        assertTrue(cont.add(proxyObj3TypeProv));
        assertEquals(cont.size(), 3);

        assertFalse(cont.isEmpty());
        cont.clear();
        assertEquals(cont.size(), 0);
        assertTrue(cont.isEmpty());
    }

    /**Tests list.addAll(Collection c), list.addAll(int index, Collection c)
     */
    @Test public void listConstructors() throws Exception
    {
        logger.log(Level.INFO, "Testing Constructors of InterfaceContainer");
        InterfaceContainer cont= new InterfaceContainer(100);

        assertEquals(cont.elementData.length, 100);
    }

    @Test public void trimToSize() throws Exception
    {
        logger.log(Level.INFO, "Testing InterfaceContainer.trimToSize");
        InterfaceContainer cont= new InterfaceContainer(100);

        cont.trimToSize();
        assertTrue(cont.isEmpty());
        cont= new InterfaceContainer(10);
        cont.addAll(list1);
        cont.trimToSize();
        assertEquals(cont.elementData.length, list1.size());
    }

    @Test public void ensureCapacity() throws Exception
    {
        logger.log(Level.INFO, "Testing InterfaceContainer.ensureCapacity");
        InterfaceContainer cont= new InterfaceContainer(10);

        cont.ensureCapacity(9);
        assertTrue(cont.elementData.length >= 9);

        cont.ensureCapacity(11);
        assertTrue(cont.elementData.length >= 11);
    }

    @Test public void addAll() throws Exception
    {
        logger.log(Level.INFO, "Testing List.addAll(Collection c), List.addAll(int index, Collection c)");
        InterfaceContainer cont= new InterfaceContainer();

        assertTrue(cont.addAll(list1));
        assertEquals(cont.size(), list1.size());
        for (int c= 0; c < cont.size(); c++)
        {
            assertSame(list1.get(c), cont.get(c));
        }
        assertTrue(cont.add(obj1));
        assertTrue(cont.addAll(1, list2));
        assertSame(cont.get(0), list1.get(0));
        assertSame(cont.get(1), list2.get(0));
        assertSame(cont.get(2), list2.get(1));
        assertSame(cont.get(3), list2.get(2));
        assertSame(cont.get(4), list1.get(1));
        assertSame(cont.get(5), list1.get(2));
        assertSame(cont.get(6), obj1);

        cont.clear();
        cont.addAll(list3);
        // the null element in list3 at index 1 is not added to cont
        assertSame(cont.get(0), list3.get(0));
        assertSame(cont.get(1), list3.get(2));
        assertSame(cont.get(2), list3.get(3));
    }

    /** Tests List.add(int index, Object element), List.get(int index)
     */
    @Test public void get() throws Exception
    {
        logger.log(Level.INFO, "Testing List.add(int index, Object element), List.get(int index)");
        InterfaceContainer cont= new InterfaceContainer();

        cont.add(0, obj1);
        cont.add(1, obj2);
        cont.add(1, proxyObj3Weak1);
        cont.add(3, obj3);

        assertSame(cont.get(0), obj1);
        assertSame(cont.get(1), proxyObj3Weak1);
        assertSame(cont.get(2), obj2);
        assertSame(cont.get(3), obj3);

        try {
            cont.add(5, obj1);
            fail("IndexOutOfBoundsException expected");
        } catch (IndexOutOfBoundsException indexOutOfBoundsException) {
            logger.log(Level.FINE, "IndexOutOfBoundsException caught");
        }
    }

    /** Tests List.contains
     */
    @Test public void contains() throws Exception
    {
        logger.log(Level.INFO, "Testing List.contains()");
        InterfaceContainer cont= new InterfaceContainer();

        assertFalse(cont.contains(obj1)); // nothing in the list

        cont.add(obj1);
        cont.add(proxyObj2TypeProv);
        cont.add(proxyObj3TypeProv);

        assertTrue(cont.contains(obj1));
        assertTrue(cont.contains(obj2));
        assertTrue(cont.contains(proxyObj3Weak1));
        assertTrue(cont.contains(proxyObj3Weak2));
        assertTrue(cont.contains(proxyObj1Weak1));
        assertTrue(cont.contains(obj3));
        assertFalse(cont.contains(null));
    }

    /** Tests List.containsAll
     */
    @Test public void containsAll() throws Exception
    {
        logger.log(Level.INFO, "Testing List.containsAll");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1);
        assertTrue(cont.containsAll(list1));

        cont.clear();
        cont.addAll(list2);
        assertTrue(cont.containsAll(list2));

        cont.clear();
        cont.addAll(list3); // the null element in list3 is not added to cont
        assertFalse(cont.containsAll(list3));

        cont.clear();
        for( int x= 0; x < 6; x++)
            cont.add(obj4);
        assertFalse(cont.contains(list1));

        cont.add(1, list1.get(0));
        cont.add(3, list1.get(1));
        cont.add(5, list1.get(2));
        assertFalse(cont.contains(list1));
    }
    /** Tests List.indexOf, List.lastIndexOf
     */
    @Test public void indexOf() throws Exception
    {
        logger.log(Level.INFO, "Testing List.indexOf(Object element), List.lastIndexOf(Object element)");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1);
        cont.addAll(list1);
        assertEquals(cont.indexOf(obj3), 2);
        assertEquals(cont.lastIndexOf(obj3), 5);

        cont.clear();
        cont.addAll(list2);
        cont.addAll(list2);
        assertEquals(cont.indexOf(proxyObj3Weak1), 2);
        assertEquals(cont.lastIndexOf(proxyObj3Weak2), 5);
    }

    /** Tests List.remove(int index), List.remove(Object element), List.removeAll(Collection c)
     */
    @Test public void remove() throws Exception
    {
        logger.log(Level.INFO, "Testing List.remove(int index), List.remove(Object element), List.removeAll(Collection c)");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list2);
        assertTrue(proxyObj2TypeProv.equals(cont.remove(1)));
        assertEquals(cont.size(), 2);

        cont.clear();
        cont.addAll(list2);
        assertTrue(cont.remove(obj1));
        assertTrue(cont.remove(proxyObj2TypeProv));
        assertTrue(cont.remove(proxyObj3Weak2));
        assertTrue(cont.isEmpty());

        cont.clear();
        cont.addAll(list3);
        assertTrue(cont.removeAll(list3));
        assertTrue(cont.isEmpty());

        cont.addAll(list2);
        List<Object> list= new ArrayList<Object>();
        list.add(list2.get(0));
        list.add(list2.get(1));
        list.add(proxyObj3Weak2);
        assertTrue(cont.removeAll(list));
        assertTrue(cont.isEmpty());
    }

    /** Tests List.retainAll
     */
    @Test public void retainAll() throws Exception
    {
        logger.log(Level.INFO, "Testing List.retainAll(Collection c)");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1); //obj1, obj2, obj3
        cont.addAll(list2); //obj1, proxyObj2TypeProv, proxyObj3TypeProv
        List<Object> list = new ArrayList<Object>();
        list.add(obj1);
        list.add(proxyObj3Weak1);

        assertTrue(cont.retainAll(list));
        assertSame(cont.get(0), obj1);
        assertSame(cont.get(1), obj3);
        assertSame(cont.get(2), obj1);
        assertSame(cont.get(3), proxyObj3TypeProv);
        assertEquals(cont.size(), 4);
    }

    /** Tests List.set(int index, Object element)
     **/
    @Test public void set() throws Exception
    {
        logger.log(Level.INFO, "Testing List.set(int index, Object element)");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list2);
        Object o1= cont.set(0, obj3);
        Object o2= cont.set(2, proxyObj3Weak1);

        assertSame(list2.get(0), o1);
        assertSame(list2.get(2), o2);
        assertSame(cont.get(0), obj3);
        assertSame(cont.get(2), proxyObj3Weak1);
    }

    /** Tests List.toArray(), List.toArray(Object[] a)
     */
    @Test public void toArray() throws Exception
    {
        logger.log(Level.INFO, "Testing List.toArray(), List.toArray(Object[] a)");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1);
        Object[] ar= cont.toArray();
        Object[] arOrig= list1.toArray();
        assertEquals(ar.length, arOrig.length);
        assertArrayEquals(ar, arOrig);

        XWeak[] arWeak= new XWeak[3];
        XWeak[] arWeak2= (XWeak[])cont.toArray(arWeak);
        assertEquals(ar.length, arWeak2.length);
        assertArrayEquals(ar, arWeak2);
    }

    @Test public void Iterator_next() throws Exception
    {
        logger.log(Level.INFO, "Testing InterfaceContainer.iterator, Iterator.next()");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1);
        Iterator it= cont.iterator();
        assertSame(it.next(), list1.get(0));
        assertSame(it.next(), list1.get(1));
        assertSame(it.next(), list1.get(2));

        try {
            it.next();
            fail("NoSuchElementException expected");
        } catch (NoSuchElementException noSuchElementException) {
            logger.log(Level.FINE, "NoSuchElementException caught");
        }
    }

    @Test public void Iterator_hasNext() throws Exception
    {
        logger.log(Level.INFO, "Testing, Iterator.next()");
        InterfaceContainer cont= new InterfaceContainer();

        Iterator it= cont.iterator();
        assertFalse(it.hasNext());

        cont.addAll(list1);
        it= cont.iterator();
        assertTrue(it.hasNext());

        it.next();
        assertTrue(it.hasNext());

        it.next();
        assertTrue(it.hasNext());

        it.next();
        assertFalse(it.hasNext());
    }

    @Test public void Iterator_remove() throws Exception
    {
        logger.log(Level.INFO, "Testing, Iterator.remove()");
        InterfaceContainer cont= new InterfaceContainer();

        Iterator it= cont.iterator();
        try {
            it.remove();
            fail("IllegalStateException expected");
        } catch (IllegalStateException illegalStateException) {
            logger.log(Level.FINE, "IllegalStateException caught");
        }

        cont.add(obj1);
        it= cont.iterator();
        it.next();
        it.remove();
        assertTrue(cont.isEmpty());
        try {
            it.remove();
            fail("IllegalStateException expected");
        } catch (IllegalStateException illegalStateException) {
            logger.log(Level.FINE, "IllegalStateException caught");
        }

        cont.clear();
        cont.addAll(list1);
        it= cont.iterator();
        while (it.hasNext())
        {
            it.next();
            it.remove();
        }
        assertTrue(cont.isEmpty());

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
        for( int c= 0; c < size; c++)
            it2.next();
        assertEquals(cont.size(), 0);
    }

    @Test public void ListIterator_next() throws Exception
    {
        logger.log(Level.INFO, "Testing InterfaceContainer.listIterator, ListIterator.next()");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1);
        Iterator it= cont.listIterator();
        assertSame(it.next(), list1.get(0));
        assertSame(it.next(), list1.get(1));
        assertSame(it.next(), list1.get(2));
        try {
            it.next();
            fail("NoSuchElementException expected");
        } catch (NoSuchElementException noSuchElementException) {
            logger.log(Level.FINE, "NoSuchElementException caught");
        }
    }

    @Test public void ListIterator_hasNext() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.hasNext()");
        InterfaceContainer cont= new InterfaceContainer();

        Iterator it= cont.listIterator();
        assertFalse(it.hasNext());

        cont.addAll(list1);
        it= cont.listIterator();
        assertTrue(it.hasNext());
        it.next();
        assertTrue(it.hasNext());
        it.next();
        assertTrue(it.hasNext());
        it.next();
        assertFalse(it.hasNext());
    }

    @Test public void ListIterator_remove() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.remove()");
        InterfaceContainer cont= new InterfaceContainer();

        ListIterator it= cont.listIterator();
        try {
            it.remove();
            fail("IllegalStateException expected");
        } catch (IllegalStateException illegalStateException) {
            logger.log(Level.FINE, "IllegalStateException caught");
        }

        cont.add(obj1);
        it= cont.listIterator();
        it.next();
        it.remove();
        assertTrue(cont.isEmpty());
        try {
            it.remove();
            fail("IllegalStateException expected");
        } catch (IllegalStateException illegalStateException) {
            logger.log(Level.FINE, "IllegalStateException caught");
        }

        cont.clear();
        cont.addAll(list1);
        it= cont.listIterator();
        while (it.hasNext())
        {
            it.next();
            it.remove();
        }
        assertTrue(cont.isEmpty());

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
        for( int c= 0; c < size; c++)
            it2.next();
        assertEquals(cont.size(), 0);
    }

    @Test public void ListIterator_hasPrevious() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.hasPrevious()");
        InterfaceContainer cont= new InterfaceContainer();

        ListIterator it= cont.listIterator();
        assertFalse(it.hasPrevious());
        cont.addAll(list1);
        it= cont.listIterator();
        while (it.hasNext())
        {
            it.next();
            assertTrue(it.hasPrevious());
        }
    }

    @Test public void ListIterator_previous() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.previous()");
        InterfaceContainer cont= new InterfaceContainer();

        cont.addAll(list1);
        // go to the end of our list and list1
        ListIterator it= cont.listIterator();
        while (it.hasNext()) {
            it.next();
        }
        ListIterator it_list1= list1.listIterator();
        while (it_list1.hasNext()) {
            it_list1.next();
        }

        while (it.hasPrevious())
        {
            assertSame(it.previous(), it_list1.previous());
        }
        try {
            it.previous();
            fail("NoSuchElementException expected");
        } catch (NoSuchElementException noSuchElementException) {
            logger.log(Level.FINE, "NoSuchElementException caught");
        }
    }

    @Test public void ListIterator_nextIndex() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.nextIndex()");
        InterfaceContainer cont= new InterfaceContainer();

        ListIterator it;
        cont.addAll(list1);
        it= cont.listIterator();
        assertEquals(it.nextIndex(), 0);
        it.next();
        assertEquals(it.nextIndex(), 1);
        it.next();
        assertEquals(it.nextIndex(), 2);
    }

    @Test public void ListIterator_previousIndex() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.previousIndex()");
        InterfaceContainer cont= new InterfaceContainer();

        ListIterator it;
        cont.addAll(list1);
        it= cont.listIterator();
        while (it.hasNext()) {
            it.next();
        }

        assertEquals(it.previousIndex(), 2);
        it.previous();
        assertEquals(it.previousIndex(), 1);
        it.previous();
        assertEquals(it.previousIndex(), 0);
        it.previous();
    }

    @Test public void ListIterator_add() throws Exception
    {
        logger.log(Level.INFO, "Testing ListIterator.add()");
        InterfaceContainer cont= new InterfaceContainer();

        ListIterator it= cont.listIterator();
        it.add(obj1);
        assertEquals(cont.size(), 1);
        it.add(obj2);
        assertEquals(cont.size(), 2);
        it.add(obj3);
        assertSame(it.previous(), obj3);
        assertSame(it.previous(), obj2);
        assertSame(it.previous(), obj1);
    }

    @Test public void disposeAndClear() throws Exception
    {
        logger.log(Level.INFO, "Testing InterfaceContainer.disposeAndClear");
        InterfaceContainer cont= new InterfaceContainer(10);

        cont.add(obj1);
        cont.add(obj2);
        cont.add(obj3);
        cont.add(proxyObj1Weak1);
        cont.add(proxyObj3TypeProv);
        logger.log(Level.FINE, "Two proxies are called. Check the output:");
        cont.disposeAndClear(new com.sun.star.lang.EventObject());
        assertEquals(obj1.nDisposingCalled, 1);
        assertEquals(obj2.nDisposingCalled, 1);
        assertEquals(obj3.nDisposingCalled, 1);
    }
}
