/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.xmerge.util;

import java.util.ArrayList;
import java.lang.Integer;

/**
 *  This is a convenience class used to create an
 *  <code>ArrayList</code> of integers.
 */
public class IntArrayList {

    /**  The list to hold our integers. */
    private ArrayList list;

    /**
     *  Constructor.
     *  Creates the list with 0 length.
     */
    public IntArrayList() {
        list = new ArrayList();
    }


    /**
     *  Constructor.
     *
     *  @param  initialCapacity  Initial capacity of the list.
     */
    public IntArrayList(int initialCapacity) {
        list = new ArrayList(initialCapacity);
    }


    /**
     *  This method ensures that the list is large enough for
     *  <code>minCapacity</code> elements.
     *
     *  @param  minCapacity  The minimum capacity of the list.
     */
    public void  ensureCapacity(int minCapacity) {

        list.ensureCapacity(minCapacity);
    }


    /**
     *  This method ensures that the list is large enough for
     *  <code>minCapacity</code> elements.  It also fills in the
     *  new slots in the list with the integer value input as
     *  <code>fillValue</code>.
     *
     *  @param  minCapacity  The minimum capacity of the list.
     *  @param  fillValue    This method adds in a integer for each
     *                       slot that was added to ensure that the
     *                       list meets the minimum capacity.
     *                       <code>fillValue</code> is the value
     *                       used as the initial value of these
     *                       added elements.
     */
    public void  ensureCapacityAndFill(int minCapacity, int fillValue) {

        list.ensureCapacity(minCapacity);

        int needToAdd = minCapacity - list.size();
        if (needToAdd > 0) {
            for (int i = 0; i < needToAdd; i++) {
                list.add(new Integer(fillValue));
            }
        }
    }


    /**
     *  This method sets an element of the list to the input
     *  integer value.
     *
     *  @param  index  The index in the list of the element
     *                 we wish to set.
     *  @param  value  The integer value that we assign to the
     *                 selected element of the list.
     */
    public void set(int index, int value) {
        list.set(index, new Integer(value));
    }


    /**
     *  This method appends an element to the list.
     *
     *  @param  value  The integer value that we assign to the
     *                 element that we are appending to the list.
     */
    public void add(int value) {
        list.add(new Integer(value));
    }


    /**
     *  This method gets the integer value stored in element index.
     *
     *  @param  index  The index in the list of the element
     *                 we wish to get the value from.
     *
     *  @return  The value of the data stored in element index.
     */
    public int get(int index) {
        return ((Integer)list.get(index)).intValue();
    }


    /**
     *  This method gets the size of the list.
     *
     *  @return  The size of the list.
     */
    public int size() {
        return list.size();
    }
}

