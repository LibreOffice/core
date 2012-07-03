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

package org.openoffice.xmerge.merger.diff;

import org.openoffice.xmerge.merger.Iterator;

/**
 *  <p>This is an implementation of the <code>Iterator</code> interface.
 *  It is based upon a simple <code>Object</code> array.</p>
 *
 * <p>Note: this class is not thread safe for performance reasons.</p>
 *
 *  @author smak
 */
public final class ObjectArrayIterator implements Iterator {


    /**
     *  The <code>Object</code> array.
     */
    private Object [] objArray;
    private int currentPosition;


    /**
     *  Standard constructor.
     *
     *  @param  objArray  The <code>Object</code> array.
     */
    public ObjectArrayIterator(Object [] objArray) {
        if (objArray != null) {
            this.objArray = new Object[objArray.length];
            System.arraycopy(objArray, 0, this.objArray, 0, objArray.length);
            currentPosition = 0;
        } else {
            this.objArray = new Object[0];
        }
    }


    public Object next() {
        if (currentPosition < objArray.length - 1) {
            currentPosition++;
            return currentElement();
        } else {
            return null;
        }

    }


    public Object previous() {
        if (currentPosition > 0) {
            currentPosition--;
            return currentElement();
        } else {
            return null;
        }
    }


    public Object start() {
        currentPosition = 0;
        return currentElement();
    }


    public Object end() {
        if (objArray.length > 0) {
            currentPosition = objArray.length - 1;
        }
        return currentElement();
    }


    public Object currentElement() {
        if (objArray.length > 0) {
            return objArray[currentPosition];
        } else {
            return null;
        }
    }


    /**
     *  Replace current <code>Object</code>.
     *
     *  @param  object  <code>Object</code> to replace.
     */
    public void replace(Object object) {
        objArray[currentPosition] = object;
    }


    /**
     *  Insert <code>Object</code> after current <code>Object</code>.
     *
     *  @param  object  <code>Object</code> to insert.
     */
    public void insert(Object object) {
        Object [] objArray2 = new Object[objArray.length+1];

        // copy the array content up before the currentposition
        if (currentPosition > 0) {
            System.arraycopy(objArray, 0, objArray2, 0, currentPosition);
        }

        objArray2[currentPosition] = object;

        // copy the array content up after the currentposition
        System.arraycopy(objArray, currentPosition, objArray2,
                   currentPosition + 1, objArray.length - currentPosition);

        objArray = objArray2;
        currentPosition++;
    }

    /**
     *  Append <code>Object</code> after current <code>Object</code>.
     *
     *  @param  object  <code>Object</code> to append.
     */
    public void append(Object object) {
        Object [] objArray2 = new Object[objArray.length + 1];

        int newPosition = currentPosition + 1;

        // copy the array content up to the currentposition
        System.arraycopy(objArray, 0, objArray2, 0, newPosition);

        objArray2[newPosition] = object;

        // copy the array content up after the currentposition
        if (currentPosition < objArray.length - 1) {
            System.arraycopy(objArray, newPosition, objArray2,
                   newPosition + 1, objArray.length - newPosition);
        }

        objArray = objArray2;
    }

    /**
     *  Remove current <code>Object</code>.
     */
    public void remove() {
        Object [] objArray2 = new Object[objArray.length - 1];

        // copy the array content up before the currentposition
        if (currentPosition > 0) {
            System.arraycopy(objArray, 0, objArray2, 0, currentPosition);
        }

        // copy the array content up after the currentposition
        if (currentPosition < objArray.length - 1) {
            System.arraycopy(objArray, currentPosition + 1, objArray2,
                   currentPosition, objArray.length - currentPosition - 1);
        }

        objArray = objArray2;

        if (currentPosition == objArray.length)
            currentPosition--;
    }

    public int elementCount() {
        return objArray.length;
    }

    public boolean equivalent(Object obj1, Object obj2) {
        return obj1.equals(obj2);
    }

    public void refresh() {
        // do nothing
    }
}

