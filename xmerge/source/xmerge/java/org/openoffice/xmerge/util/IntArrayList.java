/************************************************************************
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

