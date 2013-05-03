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

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Set;

import lib.MultiMethodTest;
import lib.StatusException;
import util.ValueChanger;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.beans.XFastPropertySet</code>
* interface methods :
* <ul>
*  <li><code> setFastPropertyValue()</code></li>
*  <li><code> getFastPropertyValue()</code></li>
* </ul> <p>
* Required relations :
* <ul>
*  <li> <code>'XFastPropertySet.ExcludeProps'</code>
*    <b>(optional) </b> : java.util.Set.
*    Has property names which must be skipped from testing in
*    some reasons (for example property accepts restricted set
*    of values).
*  </li>
* <ul> <p>
* @see com.sun.star.beans.XFastPropertySet
*/
public class _XFastPropertySet extends MultiMethodTest {

    public XFastPropertySet oObj = null;
    private List<Integer> handles = new ArrayList<Integer>();
    private int handle = -1;
    private Set<String> exclude = null ;

    /**
     * Retrieves relation.
     */
     protected void before() {
        exclude = (Set<String>) tEnv.getObjRelation("XFastPropertySet.ExcludeProps") ;
        if (exclude == null) {
            exclude = new java.util.HashSet<String>() ;
        }
     }

    /**
    * Test selects random property which can not be VOID and
    * is writable, then change property value using <code>
    * get/set</code> methods, and checks if value properly changed.
    * Has <b> OK </b> status if value after change is not equal to value
    * before and no exceptions were thrown. <p>
    */
    public void _setFastPropertyValue() {
        XPropertySet PS = UnoRuntime.queryInterface
            (XPropertySet.class, oObj);
        XPropertySetInfo propertySetInfo = PS.getPropertySetInfo();

        if (propertySetInfo == null) {
            log.println("getPropertySetInfo() method returned null");
            tRes.tested("setFastPropertyValue()", false) ;
        }
        getPropsToTest(propertySetInfo);

        Object gValue = null;
        Object sValue = null;

        if ( handle == -1) {
            log.println("*** No changeable properties found ***");
            tRes.tested("setFastPropertyValue()", false) ;
        } else {
            try {
                gValue = oObj.getFastPropertyValue(handle);
                sValue = ValueChanger.changePValue(gValue);
                oObj.setFastPropertyValue(handle, sValue);
                sValue = oObj.getFastPropertyValue(handle);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception occurred while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occurred while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occurred while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            }  catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occurred while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            }

            tRes.tested("setFastPropertyValue()",(!gValue.equals(sValue)));
        }
    }

    /**
    * Test selects random property which can not be VOID and
    * is writable, then calls the method and  checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if exceptions were thrown. <p>
    */
    public void _getFastPropertyValue() {
        XPropertySet PS = UnoRuntime.queryInterface
            (XPropertySet.class, oObj);
        XPropertySetInfo propertySetInfo = PS.getPropertySetInfo();

        if (propertySetInfo == null) {
            log.println("getPropertySetInfo() method returned null");
            tRes.tested("getFastPropertyValue()", false) ;
        }

        getPropsToTest(propertySetInfo);

        try {
            oObj.getFastPropertyValue(handle);
            tRes.tested("getFastPropertyValue()",true);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occurred while trying to get property '"
                + handle +"'");
            e.printStackTrace(log);
            tRes.tested("getFastPropertyValue()",false);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred while trying to get property '"
                + handle +"'");
            e.printStackTrace(log);
            tRes.tested("getFastPropertyValue()",false);
        }
        return;
    }


    //Get the properties being tested
    private void getPropsToTest(XPropertySetInfo xPSI) {

        Property[] properties = xPSI.getProperties();

        for (int i = 0; i < properties.length; i++) {
            if (exclude.contains(properties[i].Name)) continue ;
            Property property = properties[i];
            String name = property.Name;
            int handle = property.Handle;
            log.println("Checking '" + name + "' with handle = " + handle);
            boolean isWritable =
                ((property.Attributes & PropertyAttribute.READONLY) == 0);
            boolean isNotNull =
                ((property.Attributes & PropertyAttribute.MAYBEVOID) == 0);
            boolean canChange = false;
            if ( isWritable && isNotNull )
                canChange = isChangeable(handle);
            if ( isWritable && isNotNull && canChange)
                handles.add(new Integer(handle));
        } // endfor

        Random rnd = new Random();
        int nr = rnd.nextInt(handles.size());
        handle = handles.get(nr).intValue();
    }

    private boolean isChangeable(int handle) {
        boolean hasChanged = false;
        try {
            Object getProp = oObj.getFastPropertyValue(handle);
            Object setValue = null;

            if (getProp != null)
                setValue = ValueChanger.changePValue(getProp);
            else
                log.println("Property with handle = " + handle
                    + " is null but 'MAYBEVOID' isn't set");
            if (setValue != null) {
                oObj.setFastPropertyValue(handle, setValue);
                hasChanged =
                    (!getProp.equals(oObj.getFastPropertyValue(handle)));
            }
            else
                log.println("Couldn't change Property with handle " + handle);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Property with handle " + handle + " throws exception");
            e.printStackTrace(log);
            throw new StatusException("Property with handle " + handle
                + " throws exception", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Property with handle " + handle + " throws exception");
            e.printStackTrace(log);
            throw new StatusException("Property with handle " + handle
                + " throws exception", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Property with handle " + handle + " throws exception");
            e.printStackTrace(log);
            throw new StatusException("Property with handle " + handle
                + " throws exception", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Property with handle " + handle + " throws exception");
            e.printStackTrace(log);
            throw new StatusException("Property with handle " + handle
                + " throws exception", e);
        }

        return hasChanged;
    }
}


