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

package ifc.beans;

import lib.MultiMethodTest;

import com.sun.star.beans.Property;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySetInfo;

/**
* Testing <code>com.sun.star.beans.XPropertySetInfo</code>
* interface methods :
* <ul>
*  <li><code>getProperties()</code></li>
*  <li><code>getPropertyByName()</code></li>
*  <li><code>hasPropertyByName()</code></li>
* </ul>
* @see com.sun.star.beans.XPropertySetInfo
*/
public class _XPropertySetInfo extends MultiMethodTest {

    public XPropertySetInfo oObj = null;// oObj filled by MultiMethodTest

    public Property IsThere = null;

    /**
    * Test calls the method and stores one of the properties.<p>
    * Has <b> OK </b> status if the method successfully returns
    * value that isn't null.<p>
    */
    public void _getProperties() {
        Property[] properties = oObj.getProperties();
        IsThere = properties[0];
        tRes.tested("getProperties()", ( properties != null ));
    }

    /**
    * Test calls the method with property name that certainly present
    * in the property set and again calls the method with property name
    * that certainly doesn't present in the property set.<p>
    * Has <b> OK </b> status if the method in one case successfully
    * returns value that isn't null and no exceptions were thrown and
    * in other case exception was thrown.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getProperties()</code> : to have a property that certainly
    *  present in the property set</li>
    * </ul>
    */
    public void _getPropertyByName() {
        requiredMethod("getProperties()");
        boolean result;
        try {
            Property prop  = oObj.getPropertyByName(IsThere.Name);
            result = (prop != null);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occurred while testing" +
                                " getPropertyByName with existing property");
            e.printStackTrace(log);
            result = false;
        }

        try {
            oObj.getPropertyByName("Jupp");
            log.println("No Exception thrown while testing"+
                                " getPropertyByName with non existing property");
            result = false;
        }
        catch (UnknownPropertyException e) {
            result = true;
        }
        tRes.tested("getPropertyByName()", result);
    }

    /**
    * Test calls the method with property name that certainly present
    * in the property set and again calls the method with property name
    * that certainly doesn't present in the property set.<p>
    * Has <b> OK </b> status if the method successfully returns true in
    * one case and false in other case.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getProperties()</code> : to have a property that certainly
    *  present in the property set</li>
    * </ul>
    */
    public void _hasPropertyByName() {
        requiredMethod("getProperties()");
        tRes.tested("hasPropertyByName()",
        (
            (oObj.hasPropertyByName(IsThere.Name)) &&
            (!oObj.hasPropertyByName("Jupp")) )
        );
    }

}    /// finish class XPropertySetInfo


