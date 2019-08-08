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

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;

/**
* Testing <code>com.sun.star.container.XIndexAccess</code>
* interface methods :
* <ul>
*  <li><code> getCount()</code></li>
*  <li><code> getByIndex()</code></li>
* </ul> <p>
* Test seems to work properly in multithreaded environment.
* @see com.sun.star.container.XIndexAccess
*/
public class _XIndexAccess extends MultiMethodTest {

    public XIndexAccess oObj = null;

    /**
    * Number of elements in the container.
    */
    public int count = 0;

    /**
    * Get number of element in the container. <p>
    * Has <b> OK </b> status if method returns number lager than -1.
    */
    public void _getCount() {
        boolean result = true;
        log.println("getting the number of the elements");
        // hope we haven't a count lower than zerro ;-)
        count = -1;
        count = oObj.getCount();
        result = (count != -1);
        tRes.tested("getCount()", result);
    } //end getCount()

    /**
    * This method tests the IndexAccess from the first element,
    * the middle element and the last element. Finally it test
    * Exceptions which throws by a not available index. <p>
    * Has <b> OK </b> status if first, middle and last elements
    * successfully returned and has non null value; and if on
    * invalid index parameter <code>IndexOutOfBoundsException</code>
    * is thrown.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getCount() </code> : to have number of elements
    *   in container. </li>
    * </ul>
    */
    public void _getByIndex() {
        requiredMethod("getCount()");
        // get count from holder

        waitForEventIdle();

        boolean result = true;
        log.println("Testing getByIndex()");

        if (count > 0) {
            // Check the first element
            log.println("Check the first element");
            result &= checkGetByIndex(0);

            // Check the middle element
            log.println("Check the middle element");
            result &= checkGetByIndex(count /2);

            // Check the last element
            log.println("Check the last element");
            result &= checkGetByIndex(count -1);

            // Testing getByIndex with wrong params.
            log.println("Testing getByIndex with wrong params.");
            try {
                log.println("getByIndex(" + count + ")");
                oObj.getByIndex(count);
                log.println("no exception thrown - FAILED");
                result = false;
            } catch (IndexOutOfBoundsException e) {
                    log.println("Expected exception caught! " + e + " OK");
            } catch (WrappedTargetException e) {
                    log.println("Wrong exception! " + e + " FAILED");
                    result = false;
            }
        }

        tRes.tested("getByIndex()", result);

    } // end getByIndex

    private boolean checkGetByIndex(int index){
        Object o = null;
        boolean result = true;
        try {
            log.println("getByIndex(" + index + ")");
            o = oObj.getByIndex(index);

            if ( tEnv.getObjRelation("XIndexAccess.getByIndex.mustBeNull") != null){
                result = (o == null);
                if (result) log.println("OK"); else log.println("FAILED ->  not null");
            } else {
                result = (o != null);
                if (result) log.println("OK"); else log.println("FAILED -> null");
            }

        } catch (WrappedTargetException e) {
                log.println("Exception! " + e);
                result = false;
        } catch (IndexOutOfBoundsException e) {
                log.println("Exception! " + e);
                result = false;
        }

        return result;
    }

} // end XIndexAccess



