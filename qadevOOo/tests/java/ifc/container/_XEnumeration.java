/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.WrappedTargetException;

/**
* Testing <code>com.sun.star.container.XEnumeration</code>
* interface methods :
* <ul>
*  <li><code> hasMoreElements()</code></li>
*  <li><code> nextElement()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ENUM'</code> (of type <code>XEnumerationAccess</code>):
*   This test creates its own oObj because the method nextElement()
*   will be modified this Object directly so other threads may be faild.
*  </li>
* <ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.container.XEnumeration
*/
public class _XEnumeration extends MultiMethodTest {

    public XEnumeration oObj = null;
    public XEnumerationAccess Enum = null;

    /**
    * Retrieves relation and sets oObj to a separate enumeration
    * created. Retrieves all elements from enumeration.<p>
    * Has <b> OK </b> status if all elements successfully retrieved
    * and exceptions occurred.
    */
    public void _hasMoreElements() {
        boolean result = true;

        log.println("get all elements");
        int counter = 0;
        int tmpCounter = 0;
        while ( oObj.hasMoreElements() ) {
            try {
                Object oAny = oObj.nextElement();
                counter ++;
                if (counter - tmpCounter > 10000) {
                    log.println(counter+ " Elements");
                    tmpCounter = counter;
                }
            } catch (WrappedTargetException e) {
                log.println("hasMoreElements() : " + e);
                result = false;
                break;
            } catch (NoSuchElementException e) {
                log.println("hasMoreElements() : " + e);
                result = false;
                break;
            }
        }
        Object expCount = tEnv.getObjRelation("ExpectedCount");
        if (expCount != null) {
            int ec = ((Integer) expCount).intValue();
            boolean locResult = counter == ec;
            if (!locResult) {
                log.println("Not all Elements are returned: ");
                log.println("\tExpected: "+ ec);
                log.println("\tFound: "+counter);
            }
            result &= locResult;
        }
        tRes.tested("hasMoreElements()", result);
        return;
    } // end hasMoreElements

    /**
    * Calls the method (on starting this method there is no more elements
    * in the enumeration. <p>
    * Has <b> OK </b> status if only <code>NoSuchElementException</code>
    * exception rises. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> hasMoreElements() </code> : it retrieves all elements </li>
    * </ul>
    */
    public void _nextElement(){
        requiredMethod("hasMoreElements()");
        boolean result = true;
        log.println("additional call must throw NoSuchElementException");

        try {
            Object oAny = oObj.nextElement();
            log.println("nextElement: no exception!");
            result = false;
        } catch (WrappedTargetException e) {
            log.println("nextElement: wrong exception!");
            result = false;
        } catch (NoSuchElementException e) {
            log.println("nextElement: correct exception");
        }

        tRes.tested("nextElement()", result);

        return;

    } // end NextElement

} //end XEnumeration

