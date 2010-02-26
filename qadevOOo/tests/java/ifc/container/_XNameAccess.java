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
import com.sun.star.container.XNameAccess;

/**
* Testing <code>com.sun.star.container.XNameAccess</code> interface methods. <p>
* Test is <b> NOT </b> multithread compilant. <p>
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
        return;
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

    name = "non_existant_name__1234";
    log.println("testing hasByName() with invalid name");
        try {
            loc_result = !oObj.hasByName(name);
        } catch ( Exception nsee) {
            log.println("Expected exception was thrown");
        }
        log.println("hasByName with invalid names: " + loc_result);
    result &= loc_result;

    tRes.tested("hasByName()", result);

        return;
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

        log.println("testing with non-existant name");
        name = "non_existant_name__1234";
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

        return;

    } // end getByName()
} /// finished class _XNameAccess



