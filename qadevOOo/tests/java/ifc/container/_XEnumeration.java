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

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.lang.WrappedTargetException;

/**
* Testing <code>com.sun.star.container.XEnumeration</code>
* interface methods :
* <ul>
*  <li><code> hasMoreElements()</code></li>
*  <li><code> nextElement()</code></li>
* </ul>
* Test is multithread compliant. <p>
* @see com.sun.star.container.XEnumeration
*/
public class _XEnumeration extends MultiMethodTest {

    public XEnumeration oObj = null;

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
                oObj.nextElement();
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
            oObj.nextElement();
            log.println("nextElement: no exception!");
            result = false;
        } catch (WrappedTargetException e) {
            log.println("nextElement: wrong exception!");
            result = false;
        } catch (NoSuchElementException e) {
            log.println("nextElement: correct exception");
        }

        tRes.tested("nextElement()", result);
    } // end NextElement

} //end XEnumeration

