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
        return;
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
        return;
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


