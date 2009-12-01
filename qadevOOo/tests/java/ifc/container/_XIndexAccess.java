/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XIndexAccess.java,v $
 * $Revision: 1.5 $
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
    * the middle element and the last element. Finaly it test
    * Exceptions which throws by a not available index. <p>
    * Has <b> OK </b> status if first, middle and last elements
    * successfully returned and has non null value; and if on
    * invalid index parameter <code>IndexOutOfBoundException</code>
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

        try {
            Thread.sleep(200);
        }
        catch(java.lang.InterruptedException e) {}

        boolean result = true;
        boolean loc_result = true;
        Object o = null;
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
                loc_result = oObj.getByIndex(count) == null;
                log.println("no exception thrown - FAILED");
                result = false;
            } catch (IndexOutOfBoundsException e) {
                    log.println("Expected exception cought! " + e + " OK");
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



