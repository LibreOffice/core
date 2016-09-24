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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XAutoTextContainer;
import com.sun.star.text.XAutoTextGroup;

/**
 * Testing <code>com.sun.star.text.XAutoTextContainer</code>
 * interface methods :
 * <ul>
 *  <li><code> insertNewByName()</code></li>
 *  <li><code> removeByName()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XAutoTextContainer
 */
public class _XAutoTextContainer  extends MultiMethodTest {
    public XAutoTextContainer oObj = null;
    // every Thread must insert it's own AutoTextContainer:
    public String Name = "";

    /**
     * First removes old element from container with the specified name
     * if it exists. Then tries to add a new group with the specified
     * name. <p>
     *
     * Has <b>OK</b> status if not <code>null</code>
     * <code>AutoTextGroup</code> instance is returned.
     */
    public void _insertNewByName() {
        System.out.println("Starting: insertNewByName");
        boolean result = true;
        Name = "XAutoTextContainerx" + Thread.currentThread().getName();
        Name = Name.replace('-','x');
        Name = Name.replace(':','x');
        Name = Name.replace('.','x');
        XAutoTextGroup oGroup = null;
        //first clear the container
        log.println("remove old elements in container");
        System.out.println("remove old elements in container");
        try {
            oObj.removeByName(Name);
            log.println("old elements removed -> OK");
            System.out.println("old elements removed -> OK");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("no old elements available -> OK");
            System.out.println("no old elements available -> OK");
        }

        // insert an element
        log.println("insertNewByName");
        try {
            System.out.println("Inserting element with name '" + Name + "'");
            log.println("Inserting element with name '" + Name + "'");
            oGroup = oObj.insertNewByName(Name);
            System.out.println("done");
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("insertNewByName(): " + e);
            result &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("insertNewByName(): " + e);
            result &= false;
        }

        result &= ( oGroup != null );
        tRes.tested("insertNewByName()", result);
    } // end insertNewByName()

    /**
     * First removes element by name which was added before,
     * then tries to remove the element with the same name again. <p>
     *
     * Has <b> OK </b> status if in the first case no exceptions
     * were thrown, and in the second case
     * <code>NoSuchElementException</code> was thrown. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertNewByName() </code> : new element inserted here.</li>
     * </ul>
     */
    public void _removeByName() {
        requiredMethod("insertNewByName()");

        boolean result = true;
        // remove the element
        log.println("removeByName()");
        try {
            log.println("Removing element with name '" + Name + "'");
            oObj.removeByName(Name);
            result &= true;
        } catch (com.sun.star.container.NoSuchElementException e) {
            result = false;
            log.println("removeByName(): " + e + " -> FAILED");
        }

        log.println("2nd removeByName()");
        try {
            oObj.removeByName(Name);
            log.println("No exceptions were thrown -> FAILED");
            result = false ;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("2nd removeByName(): -> OK");
            result &= true;
        }

        tRes.tested("removeByName()", result);

    } // end removeByName()

}    /// finish class XAutoTextContainer


