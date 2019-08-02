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

package org.openoffice.xmerge.util;

import java.util.ArrayList;

/**
 * This is a convenience class used to create an {@code ArrayList} of integers.
 */
public class IntArrayList {

    /** The list to hold our integers. */
    private final ArrayList<Integer> list;

    /**
     * Constructor.
     *
     * <p>Creates the list with 0 length.</p>
     */
    public IntArrayList() {
        list = new ArrayList<Integer>();
    }

    /**
     * Constructor.
     *
     * @param  initialCapacity  Initial capacity of the list.
     */
    public IntArrayList(int initialCapacity) {
        list = new ArrayList<Integer>(initialCapacity);
    }

    /**
     * This method ensures that the list is large enough for {@code minCapacity}
     * elements.
     *
     * @param  minCapacity  The minimum capacity of the list.
     */
    public void  ensureCapacity(int minCapacity) {
        list.ensureCapacity(minCapacity);
    }

    /**
     * This method ensures that the list is large enough for {@code minCapacity}
     * elements.
     *
     * <p>It also fills in the new slots in the list with the integer value
     * input as {@code fillValue}.</p>
     *
     * @param  minCapacity  The minimum capacity of the list.
     * @param  fillValue    This method adds in an integer for each slot that was
     *                      added to ensure that the list meets the minimum
     *                      capacity. {@code fillValue} is the value used as the
     *                      initial value of these added elements.
     */
    public void  ensureCapacityAndFill(int minCapacity, int fillValue) {

        list.ensureCapacity(minCapacity);

        int needToAdd = minCapacity - list.size();
        if (needToAdd > 0) {
            for (int i = 0; i < needToAdd; i++) {
                list.add(Integer.valueOf(fillValue));
            }
        }
    }

    /**
     * This method sets an element of the list to the input integer value.
     *
     * @param  index  The index in the list of the element we wish to set.
     * @param  value  The integer value that we assign to the selected element
     *                of the list.
     */
    public void set(int index, int value) {
        list.set(index, Integer.valueOf(value));
    }

    /**
     * This method appends an element to the list.
     *
     * @param  value  The integer value that we assign to the element that we
     *                are appending to the list.
     */
    public void add(int value) {
        list.add(Integer.valueOf(value));
    }

    /**
     * This method gets the integer value stored in element index.
     *
     * @param   index  The index in the list of the element we wish to get the
     *                 value from.
     *
     * @return  The value of the data stored in element index.
     */
    public int get(int index) {
        return list.get(index).intValue();
    }

    /**
     * This method gets the size of the list.
     *
     * @return  The size of the list.
     */
    public int size() {
        return list.size();
    }
}