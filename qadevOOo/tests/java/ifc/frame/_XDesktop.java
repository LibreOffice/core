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

package ifc.frame;

import lib.MultiMethodTest;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.frame.XDesktop</code>
* interface methods:
* <ul>
*  <li><code> getComponents() </code></li>
*  <li><code> terminate() </code></li>
*  <li><code> addTerminateListener() </code></li>
*  <li><code> removeTerminateListener() </code></li>
*  <li><code> getCurrentComponent() </code></li>
*  <li><code> getCurrentFrame() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.frame.XDesktop
*/
public class _XDesktop extends MultiMethodTest {
    public XDesktop oObj = null; // oObj filled by MultiMethodTest

    /**
    * Test calls the method. Then elements enumeration is created and tested.<p>
    * Has <b> OK </b> status if no exceptions were thrown.
    */
    public void _getComponents() {
        XEnumerationAccess xComps = oObj.getComponents();
        XEnumeration xEnum = xComps.createEnumeration();
        boolean result = false;

        try {
            while (xEnum.hasMoreElements()) {
                try {
                    AnyConverter.toObject(
                            new Type(XInterface.class), xEnum.nextElement());
                } catch (com.sun.star.lang.IllegalArgumentException iae) {
                    log.println("Can't convert any");
                }
            }
            result = true;
        } catch (WrappedTargetException e) {
            log.println("Couldn't get a component : " + e.getMessage());
            e.printStackTrace();
        } catch (NoSuchElementException e) {
            log.println("Couldn't get a component : " + e.getMessage());
            e.printStackTrace();
        }
        tRes.tested("getComponents()", result);
    }

    /**
    * Cannot test the method because it requires
    * terminating StarOffice. Will add real test later.
    */
    public void _terminate() {
        tRes.tested("terminate()", true);
    }

    /**
    * Cannot test the method because of terminate().
    * Will add real test later.
    */
    public void _addTerminateListener() {
        tRes.tested("addTerminateListener()", true);
    }

    /**
    * Cannot test the method because of terminate().
    * Will add real test later.
    */
    public void _removeTerminateListener() {
        tRes.tested("removeTerminateListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _getCurrentComponent() {
        tRes.tested("getCurrentComponent()",
            oObj.getCurrentComponent() != null);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _getCurrentFrame() {
        tRes.tested("getCurrentFrame()", oObj.getCurrentFrame() != null);
    }

}

