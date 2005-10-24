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
     *  Private default constructor.
     */
    private ObjectArrayIterator() {
        // do not allow user new a ObjectArrayIterator without argument
    }


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

