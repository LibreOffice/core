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
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyState;
import com.sun.star.beans.XMultiPropertyStates;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.beans.XMultiPropertyStates</code>
* interface methods :
* <ul>
*  <li><code> getPropertyStates()</code></li>
*  <li><code> setAllPropertiesToDefault()</code></li>
*  <li><code> getPropertyValues()</code></li>
*  <li><code> setPropertiesToDefault()</code></li>
*  <li><code> getPropertyDefaults()</code></li>
* </ul>
* @see com.sun.star.beans.XMultiPropertyStates
*/
public class _XMultiPropertyStates extends MultiMethodTest {

    public XMultiPropertyStates oObj = null;

    private PropertyState[] states = null;
    private String[] names = null;

    public void before() {
        names = (String[]) tEnv.getObjRelation("PropertyNames");
        if (names == null) {
            throw new StatusException(Status.failed("No PropertyNames given"));
        }

        log.println("Totally " + names.length + " properties encountered:");
        log.print("{");
        for (int i = 0; i < names.length; i++)
            log.print(names[i] + " ");
        log.print("}");
        log.println("");
    }


    /**
    * Test calls the method and checks return value.
    * <code>PropertyDefaults</code> are stored<p>
    * Has <b> OK </b> status if the method returns not null value
    * and no exceptions were thrown. <p>
    */
    public void _getPropertyDefaults() {
        boolean result = false;
        try {
            Object[] defaults = oObj.getPropertyDefaults(names);
            result = (defaults != null) && defaults.length == names.length;
            log.println("Number of default values: " + defaults.length);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("some properties seem to be unknown: " + e.toString());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Wrapped target Exception was thrown: " + e.toString());
        }
        tRes.tested("getPropertyDefaults()", result) ;
    }

    /**
    * Test calls the method and checks return value.
    * Has <b> OK </b> status if the method returns not null value
    * and no exceptions were thrown. <p>
    */
    public void _getPropertyStates() {
        boolean result = false;
        try {
            states = oObj.getPropertyStates(names);
            result = (states != null) && (states.length == names.length);
            log.println("Number of states: " + states.length);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("some properties seem to be unknown: " + e.toString());
        }
        tRes.tested("getPropertyStates()", result) ;
    }

    /**
    * Test calls the method and checks return value.
    * Has <b> OK </b> status if the Property
    * has default state afterwards. <p>
    */
    public void _setPropertiesToDefault() {
        requiredMethod("getPropertyStates()");
        // searching for property which currently don't have default value
        // and preferable has MAYBEDEFAULT attr
        // if no such properties are found then the first one is selected

        String ro = (String) tEnv.getObjRelation("allReadOnly");
        if (ro != null) {
            log.println(ro);
            tRes.tested("setPropertiesToDefault()",Status.skipped(true));
            return;
        }

        boolean mayBeDef = false;
        String propName = names[0];

        for(int i = 0; i < names.length; i++) {
            if (!mayBeDef && states[i] != PropertyState.DEFAULT_VALUE ) {
                propName = names[i];
                XPropertySet xPropSet = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class, oObj);
                XPropertySetInfo xPropSetInfo = xPropSet.getPropertySetInfo();
                Property prop = null;
                try {
                    prop = xPropSetInfo.getPropertyByName(names[i]);
                }
                catch(com.sun.star.beans.UnknownPropertyException e) {
                    log.println("couldn't get property info: " + e.toString());
                    throw new StatusException(Status.failed
                        ("couldn't get property info"));
                }
                if ( (prop.Attributes & PropertyAttribute.MAYBEDEFAULT) != 0){
                    log.println("Property " + names[i] +
                        " 'may be default' and doesn't have default value");
                    mayBeDef = true;
                }
            }
        }
        log.println("The property " + propName + " selected");

        boolean result = false;
        try {
            String[] the_first = new String[1];
            the_first[0] = propName;
            log.println("Setting " + propName + " to default");
            oObj.setPropertiesToDefault(the_first);
            result = (oObj.getPropertyStates(the_first)[0].equals
                (PropertyState.DEFAULT_VALUE));
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("some properties seem to be unknown: " + e.toString());
        }

        if (!result) {
            log.println("The property didn't change its state to default ...");
            if (mayBeDef) {
                log.println("   ... and it may be default - FAILED");
            } else {
                log.println("   ... but it may not be default - OK");
                result = true;
            }
        }

        tRes.tested("setPropertiesToDefault()", result) ;
    }

    /**
    * Test calls the method and checks return value.
    * Has <b> OK </b> status if the all Properties
    * have default state afterwards. <p>
    */
    public void _setAllPropertiesToDefault() {
        requiredMethod("setPropertiesToDefault()");
        boolean result = true;

       try {
            oObj.setAllPropertiesToDefault();
       } catch(RuntimeException e) {
           log.println("Ignore Runtime Exception: " + e.getMessage());
       }
        log.println("Checking that all properties are now in DEFAULT state" +
            " excepting may be those which 'cann't be default'");

        try {
            states = oObj.getPropertyStates(names);
            for (int i = 0; i < states.length; i++) {
                boolean part_result = states[i].equals
                    (PropertyState.DEFAULT_VALUE);
                if (!part_result) {
                    log.println("Property '" + names[i] +
                        "' wasn't set to default");
                    XPropertySet xPropSet = (XPropertySet)
                        UnoRuntime.queryInterface(XPropertySet.class, oObj);
                    XPropertySetInfo xPropSetInfo =
                        xPropSet.getPropertySetInfo();
                    Property prop = xPropSetInfo.getPropertyByName(names[i]);
                    if ( (prop.Attributes &
                            PropertyAttribute.MAYBEDEFAULT) != 0 ) {
                        log.println("   ... and it has MAYBEDEFAULT "+
                            "attribute - FAILED");
                    } else {
                        log.println("   ... but it has no MAYBEDEFAULT "+
                            "attribute - OK");
                        part_result = true;
                    }
                }

                result &= part_result;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("some properties seem to be unknown: " + e.toString());
            result=false;
        }

        tRes.tested("setAllPropertiesToDefault()", result) ;
    }

}

