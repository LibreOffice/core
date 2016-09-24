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
import com.sun.star.container.XNameAccess;

/**
* Testing <code>com.sun.star.container.XNameAccess</code> interface methods. <p>
* Test is <b> NOT </b> multithread compliant. <p>
*/
public class _XNameAccess extends MultiMethodTest {
    public XNameAccess oObj = null;
    public String[] Names = null;

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method successfully returns
    * not null value and no exceptions were thrown. <p>
    */
    public void _getElementNames() {
        boolean result = true;
        log.println("getting elements names");
        Names = oObj.getElementNames();

        result = (Names != null);
        tRes.tested("getElementNames()", result);
    } // end getElementNames()

    /**
    * First test calls the method with existing element name,
    * then with non existing. <p>
    * Has <b> OK </b> status if in the first case the method returns
    * true and in the second - false. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getElementNames </code> : to retrieve at least one
    *    element name. </li>
    * </ul>
    */
    public void _hasByName() {
        requiredMethod("getElementNames()");
        log.println("testing hasByName() ...");

        boolean result = true;
        boolean loc_result = true;

        String name = null;

    if (Names.length != 0) {
            name = Names[0];
            log.println("testing hasByName() with valid name '" + name + "'");
            loc_result = oObj.hasByName(name);
            log.println("hasByName with valid names: " + loc_result);
            result &= loc_result;
    }

    name = "non_existent_name__1234";
    log.println("testing hasByName() with invalid name");
        try {
            loc_result = !oObj.hasByName(name);
        } catch ( Exception nsee) {
            log.println("Expected exception was thrown");
        }
        log.println("hasByName with invalid names: " + loc_result);
    result &= loc_result;

    tRes.tested("hasByName()", result);
    } // end hasByName()


    /**
    * First test calls the method with existing element name,
    * then with non existing. <p>
    * Has <b> OK </b> status if in the first case the method returns
    * not null value and no exceptions were thrown,
    * and in the second case <code>NoSuchElementException</code> was
    * thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getElementNames </code> : to retrieve at least one
    *    element name. </li>
    * </ul>
    */
    public void _getByName() {
        log.println("reqiure getElementNames() ...");
        requiredMethod("getElementNames()");
        log.println("require getElementNames() ...OK");
        log.println("testing getByName() ...");

        boolean result = true;
        boolean loc_result = true;

        String name = null;

        if (Names.length != 0) {
            name = Names[0];
            log.println("testing with valid name '" + name + "'");
            try {
                loc_result = (null != oObj.getByName(name));
            } catch (Exception e) {
                log.println("Exception! - FAILED");
                log.println(e.toString());
                loc_result = false;
            }
            log.println("getByName with valid name: " + loc_result);
            result &= loc_result;
        }

        log.println("testing with non-existent name");
        name = "non_existent_name__1234";
        try {
            loc_result = (null != oObj.getByName(name));
            loc_result = false;
            log.println("getByName: Exception expected - FAILED");
        } catch (NoSuchElementException e) {
            log.println("getByName: Expected exception - OK");
            loc_result = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("getByName: Wrong exception - " + e + " - FAILED");
            loc_result = false;
        }

        result &= loc_result;
        tRes.tested("getByName()", result);
    } // end getByName()
} /// finished class _XNameAccess



