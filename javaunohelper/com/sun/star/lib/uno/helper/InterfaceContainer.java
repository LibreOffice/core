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
import java.util.Iterator;
import java.util.ListIterator;
import java.util.NoSuchElementException;
import java.util.Collection;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XEventListener;
import com.sun.star.uno.UnoRuntime;

/**
 * This class is a container for interfaces.
 *
 * It is intended to be used as storage for UNO interface of a specific type.
 * The client has to ensure that the container contains only elements of the same
 * type. If one needs to store different types, then one uses OMultiTypeInterfaceContainer.
 * When the client calls disposeAndClear, the contained objects are queried for
 * com.sun.star.lang.XEventListener and disposing is called. Afterwards
 * the list cannot be used anymore.
 *
 * This list does not allow null values.
 * All methods are thread-safe. The same holds true for
 * iterators, issued by this class. Several iterators can exist at the same time and can also
 * be modified (java.util.ListIterator.add, java.util.ListIterator.remove etc.). To make this work,
 * the InterfaceContainer provides the iterators with copys of the list's data.
 * The add and remove calls on the iterator modify the data in the iterator's list as well as
 * in InterfaceContainer. Modification on InterfaceContainer, however, are not
 * synchronized with existing iterators. For example
 * <pre>
 * InterfaceContainer cont= new InterfaceContainer();
 * ListIterator it= cont.listIterator();
 *
 * cont.add( someInterface);
 * // one cannot obtain someInterface through iterator it,
 * // instead get a new iterator
 * it= cont.listIterator();
 * // it now keeps a fresh copy of cont and hence contains someInterface
 *
 * // Adding an interface on the iterator will cause the interface also to be added
 * // to InterfaceContainer
 * it.add( someOtherInterface);
 * // someOtherInterface is now in it and cont
 * ListIterator it2= cont.listIterator();
 * //someOtherInterface can also be obtained by all newly created iterators, e.g. it2.
 * </pre>
 *
 *  The add and remove methods of an iterator work on a particular location within a list,
 *  dependent on what the value of the iterator's cursor is. After the call the value at the
 *  appropriate position has been modified. Since the iterator received a copy of InterfaceContainer's
 *  data, InterfaceContainer may have been modified (by List methods or through other iterators).
 *  Therefore both data sets may not contain the same elements anymore. Consequently, a List method
 *  that modifies data, does not modify InterfaceContainer's data at a certain index
 *  (according to the iterators cursor). Instead, new elements are added at the end of list. When
 *  Iterator.remove is called, then the first occurrence of that element in InterfaceContainer
 *  is removed.
 *  ListIterator.set is not supported.
 *
 * A lot of  methods resemble those of the  to java.util.List interface, allthough
 * this class does not implement it. However, the list iterators returned, for example by
 * the listIterator method implement the java.util.ListIterator interface.
 * Implementing the List interface would mean to support all none - optional methods as
 * prescribed by the interface declaration. Among those is the subList method which returns
 * a range of values of the list's data wrapped in a List implementation. Changes to the sub
 * list have to cause changes in the main list. This is a problem, since this class is to be
 * used in a multi-threaded environment. The sub list could work on a copy as the iterators
 * do, but all the functions which work on an given index could not be properly supported.
 * Unfortunatly, the List interface documentation states that all optional methods implemented
 * by the list have to be implemented in the sub list. That would mean to do without all those
 * critical methods, allthough they might work well in the "main list" (as opposed to sub list).
 */
public class InterfaceContainer implements Cloneable
{
    final boolean DEBUG= false;
    /**
     * The array buffer into which the elements of the ArrayList are stored.
     * The capacity of the ArrayList is the length of this array buffer.
     */
    Object elementData[];

    /**
     * The size of the ArrayList (the number of elements it contains).
     */
    private int size;


    //private ArrayList data= new ArrayList();
    /** Creates a new instance of InterfaceContainer */
    public InterfaceContainer()
    {
        this(10);
    }
    /**
     * Constructs an empty list with the specified initial capacity.
     *
     * @param   initialCapacity   the initial capacity of the list.
     * @exception IllegalArgumentException if the specified initial capacity
     *            is negative
     */
    public InterfaceContainer(int initialCapacity)
    {
        if (initialCapacity < 0)
            throw new java.lang.IllegalArgumentException("Illegal Capacity: "+
            initialCapacity);
        this.elementData = new Object[initialCapacity];
    }

    /**
     * Trims the capacity of this <tt>ArrayList</tt> instance to be the
     * list's current size.  An application can use this operation to minimize
     * the storage of an <tt>ArrayList</tt> instance.
     */
    synchronized public void trimToSize()
    {
        int oldCapacity = elementData.length;
        if (size < oldCapacity)
        {
            Object oldData[] = elementData;
            elementData = new Object[size];
            System.arraycopy(oldData, 0, elementData, 0, size);
        }
    }

    /**
     * Increases the capacity of this <tt>ArrayList</tt> instance, if
     * necessary, to ensure  that it can hold at least the number of elements
     * specified by the minimum capacity argument.
     *
     * @param   minCapacity   the desired minimum capacity.
     */
    synchronized public void ensureCapacity(int minCapacity)
    {
        int oldCapacity = elementData.length;
        if (minCapacity > oldCapacity)
        {
            Object oldData[] = elementData;
            int newCapacity = (oldCapacity * 3)/2 + 1;
            if (newCapacity < minCapacity)
                newCapacity = minCapacity;
            elementData = new Object[newCapacity];
            System.arraycopy(oldData, 0, elementData, 0, size);
        }
    }

    /**
     * Appends the specified element to the end of this list.
     *
     * @param o element to be appended to this list.
     * @return <tt>true</tt> (as per the general contract of Collection.add).
     */
    synchronized public boolean add(Object o)
    {
        boolean ret= false;
        if (elementData != null && o != null)
        {
            ensureCapacity(size + 1);  // Increments modCount!!
            elementData[size++] = o;
            ret= true;
        }
        return ret;
    }

    /**
     * Inserts the specified element at the specified position in this
     * list. Shifts the element currently at that position (if any) and
     * any subsequent elements to the right (adds one to their indices).
     *
     * @param index index at which the specified element is to be inserted.
     * @param element element to be inserted.
     * @throws    IndexOutOfBoundsException if index is out of range
     *        <tt>(index &lt; 0 || index &gt; size())</tt>.
     */
    synchronized public void add(int index, Object element)
    {
        if (elementData != null && element != null)
        {
            if (index > size || index < 0)
                throw new IndexOutOfBoundsException(
                "Index: "+index+", Size: "+size);

            ensureCapacity(size+1);
            System.arraycopy(elementData, index, elementData, index + 1,
            size - index);
            elementData[index] = element;
            size++;
        }
    }


    /**
     * Appends all of the elements in the specified Collection to the end of
     * this list, in the order that they are returned by the
     * specified Collection's Iterator.  The behavior of this operation is
     * undefined if the specified Collection is modified while the operation
     * is in progress.  (This implies that the behavior of this call is
     * undefined if the specified Collection is this list, and this
     * list is nonempty.)
     *
     * @param c the elements to be inserted into this list.
     * @throws    IndexOutOfBoundsException if index out of range <tt>(index
     *        &lt; 0 || index &gt; size())</tt>.
     */
    synchronized public boolean addAll(Collection c)
    {
        int numNew = c.size();
        ensureCapacity(size + numNew);

        Iterator e = c.iterator();
        for (int i=0; i<numNew; i++)
        {
            Object o= e.next();
            if (o != null)
                elementData[size++] = o;
        }
        return numNew != 0;
    }
    /**
     * Inserts all of the elements in the specified Collection into this
     * list, starting at the specified position.  Shifts the element
     * currently at that position (if any) and any subsequent elements to
     * the right (increases their indices).  The new elements will appear
     * in the list in the order that they are returned by the
     * specified Collection's iterator.
     *
     * @param index index at which to insert first element
     *          from the specified collection.
     * @param c elements to be inserted into this list.
     * @throws    IndexOutOfBoundsException if index out of range <tt>(index
     *        &lt; 0 || index &gt; size())</tt>.
     */
    synchronized public boolean addAll(int index, Collection c)
    {
        boolean ret= false;
        if (elementData != null)
        {
            if (index > size || index < 0)
                throw new IndexOutOfBoundsException(
                "Index: "+index+", Size: "+size);
            // only add  the non-null elements
            int sizeCol= c.size();
            Object[] arColl= new Object[sizeCol];
            Iterator icol= c.iterator();
            int curIndex= 0;
            for (int i=0; i < sizeCol; i++)
            {
                Object o= icol.next();
                if (o != null)
                    arColl[curIndex++]= o;
            }
            int numNew = curIndex;
            ensureCapacity(size + numNew);  // Increments modCount!!

            int numMoved = size - index;
            if (numMoved > 0)
                System.arraycopy(elementData, index, elementData, index + numNew,
                numMoved);

            for (int i=0; i<numNew; i++)
            {
                elementData[index++]= arColl[i];
            }
            size += numNew;
            ret= numNew != 0;
        }
        return ret;
    }

    /**
     * Removes all of the elements from this list.  The list will
     * be empty after this call returns.
     */
    synchronized public void clear()
    {
        if (elementData != null)
        {
            // Let gc do its work
            for (int i = 0; i < size; i++)
                elementData[i] = null;

            size = 0;
        }
    }
    /**
     * Returns <tt>true</tt> if this list contains the specified element.
     *
     * @param elem element whose presence in this List is to be tested.
     */
    synchronized public boolean contains(Object elem)
    {
        return indexOf(elem) >= 0;
    }

    synchronized public boolean containsAll(Collection collection)
    {
        boolean retVal= true;
        if (elementData != null && collection != null)
        {
            Iterator it= collection.iterator();
            while (it.hasNext())
            {
                Object obj= it.next();
                if (false == contains(obj))
                {
                    retVal= false;
                    break;
                }
            }
        }
        return retVal;
    }
    /**
     * Returns the element at the specified position in this list.
     *
     * @param  index index of element to return.
     * @return the element at the specified position in this list.
     * @throws    IndexOutOfBoundsException if index is out of range <tt>(index
     *        &lt; 0 || index &gt;= size())</tt>.
     */
    synchronized public Object get(int index)
    {
        if (elementData != null)
        {
            RangeCheck(index);
            return elementData[index];
        }
        return null;
    }

    /**
     * Searches for the first occurrence of the given argument, testing
     * for equality using the <tt>equals</tt> method.
     *
     * @param   elem   an object.
     * @return  the index of the first occurrence of the argument in this
     *          list; returns <tt>-1</tt> if the object is not found.
     * @see     Object#equals(Object)
     */
    synchronized public int indexOf(Object elem)
    {
        int index= -1;
        if (elementData != null && elem != null)
        {
            for (int i = 0; i < size; i++)
            {
                if (elem == elementData[i])
                {
                    index= i;
                    break;
                }
            }

            if (index == -1)
            {
                for (int i = 0; i < size; i++)
                {
                    if (UnoRuntime.areSame(elem, elementData[i]))
                    {
                        index= i;
                        break;
                    }
                }
            }
        }
        return index;
    }
    /**
     * Tests if this list has no elements.
     *
     * @return  <tt>true</tt> if this list has no elements;
     *          <tt>false</tt> otherwise.
     */
    synchronized public boolean isEmpty()
    {
        return size == 0;
    }

    synchronized public Iterator iterator()
    {
        if (elementData != null)
        {
            InterfaceContainer aCopy= (InterfaceContainer) clone();
            return new Itr(aCopy);
        }
        return null;
    }
    /**
     * Returns the index of the last occurrence of the specified object in
     * this list.
     *
     * @param   elem   the desired element.
     * @return  the index of the last occurrence of the specified object in
     *          this list; returns -1 if the object is not found.
     */
    synchronized public int lastIndexOf(Object elem)
    {
        int index= -1;
        if (elementData != null && elem != null)
        {
            for (int i = size-1; i >= 0; i--)
            {
                if (elem == elementData[i])
                {
                    index= i;
                    break;
                }
            }
            if (index == -1)
            {
                for (int i = size-1; i >= 0; i--)
                {
                    if (UnoRuntime.areSame(elem, elementData[i]))
                    {
                        index= i;
                        break;
                    }
                }
            }
        }
        return index;
    }

    /**
     * Returns a shallow copy of this <tt>ArrayList</tt> instance. The contained
     * references are copied but the objects not.
     *
     * @return  a clone of this <tt>List</tt> instance.
     */
    synchronized public Object clone()
    {
        Object ret= null;
        if (elementData != null)
        {
            InterfaceContainer cont= new InterfaceContainer();
            cont.elementData = new Object[size];
            cont.size= size;
            System.arraycopy(elementData, 0, cont.elementData, 0, size);
            ret= cont;
        }
        return ret;
    }
    synchronized public ListIterator listIterator()
    {
        return listIterator(0);
    }

    /** The iterator keeps a copy of the list. Changes to InterfaceContainer do not
     *  affect the data of the iterator. Conversly, changes to the iterator are effect
     *  InterfaceContainer.
     */
    synchronized public ListIterator listIterator(int index)
    {
        if (elementData != null)
        {
            InterfaceContainer aCopy= (InterfaceContainer) clone();
            return new LstItr(aCopy, index);
        }
        return null;
    }
    /**
     * Removes the element at the specified position in this list.
     * Shifts any subsequent elements to the left (subtracts one from their
     * indices).
     *
     * @param index the index of the element to removed.
     * @return the element that was removed from the list.
     * @throws    IndexOutOfBoundsException if index out of range <tt>(index
     *        &lt; 0 || index &gt;= size())</tt>.
     */
    synchronized public Object remove(int index)
    {
        Object ret= null;
        if (elementData != null)
        {
            RangeCheck(index);
            ret= elementData[index];

            int numMoved = size - index - 1;
            if (numMoved > 0)
                System.arraycopy(elementData, index+1, elementData, index,
                numMoved);
            elementData[--size] = null; // Let gc do its work
        }
        return ret;
    }


    /** Parameter obj may  */
    synchronized public boolean remove(Object obj)
    {
        boolean ret= false;
        if (elementData != null && obj != null)
        {
            int index= indexOf(obj);
            if (index != -1)
            {
                ret= true;
                remove(index);
            }
        }
        return ret;
    }

    synchronized public boolean removeAll(Collection collection)
    {
        boolean retVal= false;
        if (elementData != null && collection != null)
        {
            Iterator it= collection.iterator();
            while (it.hasNext())
            {
                Object obj= it.next();
                boolean bMod= remove( obj);
                if (bMod)
                    retVal= true;
            }
        }
        return retVal;
    }

    synchronized public boolean retainAll(Collection collection)
    {
        boolean retVal= false;
        if (elementData != null && collection != null)
        {
            // iterate over data
            Object[] arRetained= new Object[size];
            int indexRetained= 0;
            for(int i= 0; i < size; i++)
            {
                Object curElem= elementData[i];
                // try to find the element in collection
                Iterator itColl= collection.iterator();
                boolean bExists= false;
                while (itColl.hasNext())
                {
                    if (curElem == itColl.next())
                    {
                        // current element is in collection
                        bExists= true;
                        break;
                    }
                }
                if (bExists == false)
                {
                    itColl= collection.iterator();
                    while (itColl.hasNext())
                    {
                        Object o= itColl.next();
                        if (o != null)
                        {
                            if (UnoRuntime.areSame(o, curElem))
                            {
                                bExists= true;
                                break;
                            }
                        }
                    }
                }
                if (bExists == true)
                    arRetained[indexRetained++]= curElem;
            }
            retVal= size != indexRetained;
            if (indexRetained > 0)
            {
                elementData= arRetained;
                size= indexRetained;
            }
        }
        return retVal;
    }


    /** Not supported.
     * @param index index of element to replace.
     * @param element element to be stored at the specified position.
     * @return the element previously at the specified position.
     * @throws    IndexOutOfBoundsException if index out of range
     *        <tt>(index &lt; 0 || index &gt;= size())</tt>.
     */
    synchronized public Object set(int index, Object element)
    {
          Object ret= null;
          if (elementData != null && element != null)
          {
              RangeCheck(index);
              ret = elementData[index];
              elementData[index] = element;
          }
          return ret;
    }

    /**
     * Returns the number of elements in this list.
     *
     * @return  the number of elements in this list.
     */
    synchronized public int size()
    {
        if (elementData != null)
            return size;
        return 0;
    }


    /**
     * Returns an array containing all of the elements in this list
     * in the correct order.
     *
     * @return an array containing all of the elements in this list
     *         in the correct order.
     */
    synchronized public Object[] toArray()
    {
        if (elementData != null)
        {
            Object[] result = new Object[size];
            System.arraycopy(elementData, 0, result, 0, size);
            return result;
        }
        return null;
    }

    /**
     * Returns an array containing all of the elements in this list in the
     * correct order.  The runtime type of the returned array is that of the
     * specified array.  If the list fits in the specified array, it is
     * returned therein.  Otherwise, a new array is allocated with the runtime
     * type of the specified array and the size of this list.<p>
     *
     * If the list fits in the specified array with room to spare (i.e., the
     * array has more elements than the list), the element in the array
     * immediately following the end of the collection is set to
     * <tt>null</tt>.  This is useful in determining the length of the list
     * <i>only</i> if the caller knows that the list does not contain any
     * <tt>null</tt> elements.
     *
     * @param a the array into which the elements of the list are to
     *      be stored, if it is big enough; otherwise, a new array of the
     *      same runtime type is allocated for this purpose.
     * @return an array containing the elements of the list.
     * @throws ArrayStoreException if the runtime type of a is not a supertype
     *         of the runtime type of every element in this list.
     */
    synchronized public Object[] toArray(Object a[])
    {
        if (a.length < size)
            a = (Object[])java.lang.reflect.Array.newInstance(
            a.getClass().getComponentType(), size);
        if (elementData != null)
            System.arraycopy(elementData, 0, a, 0, size);

        if (a.length > size)
            a[size] = null;

        return a;
    }

    /**
     * Check if the given index is in range.  If not, throw an appropriate
     * runtime exception.
     */
    private void RangeCheck(int index)
    {
        if (index >= size || index < 0)
            throw new IndexOutOfBoundsException(
            "Index: "+index+", Size: "+size);
    }

    public void disposeAndClear(EventObject evt)
    {
        Iterator aIt;
        synchronized (this)
        {
            aIt= iterator();
            // Container freigeben, falls im disposing neue Eintraege kommen
            // set the member to null, the iterator delete the values
            clear();
            elementData= null;
            size= 0;
        }
        if (aIt != null)
        {
            while( aIt.hasNext() )
            {
                try
                {
                    Object o= aIt.next();
                    XEventListener evtListener= UnoRuntime.queryInterface(
                    XEventListener.class, o);
                    if( evtListener != null )
                        evtListener.disposing( evt );
                }
                catch ( RuntimeException e)
                {
                    // be robust, if e.g. a remote bridge has disposed already.
                    // there is no way, to delegate the error to the caller :o(.
                }
            }
        }
    }


    private class Itr implements Iterator
    {
        InterfaceContainer dataIt;
        /**
         * Index of element to be returned by subsequent call to next.
         */
        int cursor= 0;
        /**
         * Index of element returned by most recent call to next or
         * previous.  Reset to -1 if this element is deleted by a call
         * to remove.
         */
        int lastRet = -1;

        /** The object that has been returned by most recent call to next
         *  or previous. Reset to null if this element is deleted by a call
         *  to remove.
         */
        Object lastRetObj= null;

        Itr(InterfaceContainer _data)
        {
            dataIt= _data;
        }

        synchronized public boolean hasNext()
        {
            return cursor !=dataIt.size();
        }

        public synchronized Object next()
        {
            try
            {
                Object next = dataIt.get(cursor);
                lastRet = cursor++;
                lastRetObj= next;
                return next;
            }
            catch(java.lang.IndexOutOfBoundsException e)
            {
                throw new java.util.NoSuchElementException();
            }
        }

        /** Removes the interface from the list, that has been last returned by a
         *  call to next(). This is done according to the specification of the interface
         *  method. The element is also removed from InterfaceContainer but independent
         *  of the location. If the element is multiple times in InterfaceContainer then
         *  it is up to the java.util.ArrayList implementation what element is removed.
         */
        public synchronized void remove()
        {
            if (lastRet == -1)
                throw new IllegalStateException();
            // Remove the entry from InterfaceContainer.
            InterfaceContainer.this.remove(lastRetObj);
            dataIt.remove(lastRet);

            if (lastRet < cursor)
                cursor--;
            lastRet = -1;
            lastRetObj= null;
        }
    }

    private class LstItr extends Itr implements ListIterator
    {

        LstItr(InterfaceContainer _data, int _index)
        {
            super(_data);
            cursor= _index;
        }

        /** Inserts an element to the iterators list according to the specification
         *  of this interface method. The element is also added to InterfaceContainer
         *  but its location within the list cannot be guaranteed.
         */
        public synchronized void add(Object o)
        {
            InterfaceContainer.this.add(o);
            dataIt.add(cursor++, o);
            lastRet = -1;
            lastRetObj= null;
        }

        synchronized public boolean hasPrevious()
        {
            return cursor != 0;
        }

        synchronized public int nextIndex()
        {
            return cursor;
        }

        public synchronized Object previous()
        {
            try
            {
                Object previous = dataIt.get(--cursor);
                lastRet = cursor;
                lastRetObj= previous;
                return previous;
            } catch(IndexOutOfBoundsException e)
            {
                throw new NoSuchElementException();
            }
        }

        synchronized public int previousIndex()
        {
            return cursor-1;
        }

        /** This is not possible since several iterators can modify InterfaceContainer
         */
        public synchronized void set(Object o)
        {
            throw new UnsupportedOperationException();
        }


    } // class LstItr
}

