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

import com.sun.star.beans.IllegalTypeException;
import com.sun.star.beans.NotRemoveableException;
import com.sun.star.beans.PropertyExistException;
import com.sun.star.uno.Any;
import lib.MultiMethodTest;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyContainer;
import lib.Status;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.beans.XPropertyAccess</code>
 * interface methods :
 * <ul>
 *  <li><code>addProperty()</code></li>
 *  <li><code>removeProperty()</code></li>
 * </ul>
 * @see com.sun.star.beans.XPropertyAccess
 */
public class _XPropertyContainer extends MultiMethodTest {

    /**
     * oObj filled by MultiMethodTest
     */
    public XPropertyContainer oObj = null;// oObj filled by MultiMethodTest

    /**
     * object relation X<CODE>PropertyAccess.propertyNotRemovable</CODE><br>
     * This relation must be filled from the module. It contains a property which must
     * be a property of the implementaion object.
     */
    private String propertyNotRemovable = null;

    /**
     * checks if the object relation <CODE>XPropertyAccess.propertyNotRemovable</CODE>
     * is available
     */
    public void before() {
        propertyNotRemovable = (String) tEnv.getObjRelation("XPropertyContainer.propertyNotRemovable");
        if (propertyNotRemovable == null) {
            throw new StatusException(Status.failed("Object raltion 'XPropertyAccess.propertyNotRemovable' is null"));
        }
    }



    /**
     * Test calls the method and checks if the returned sequence contanis a propterty which is named
     * in the object relation <code>XPropertyAccess.propertyNotRemovable</code>.
     */
    public void _addProperty() {

        boolean ok = true;
        boolean test = true;
        boolean exp = false;

        try {
            log.println("try to add following property:\n" +
                    "\t('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) ...");
            oObj.addProperty("myXPropertContainerProperty" , com.sun.star.beans.PropertyAttribute.MAYBEVOID, null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: Exception was thrown while adding following property: " +
                    "('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) : " +
                    ex.toString());
            log.println("... FAILED");
            test = false;
        } catch (PropertyExistException ex) {
            log.println("ERROR: Exception was thrown while adding following property: " +
                    "('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) : " +
                    ex.toString());
            log.println("... FAILED");
            test = false;
        } catch (IllegalTypeException ex) {
            log.println("ERROR: Exception was thrown while adding following property: " +
                    "('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) : " +
                    ex.toString());
           log.println("... FAILED");
           test = false;
        }

        if ( test){
            log.println("... OK");
        }

        ok &= test;
        test = false;
        exp = false;
        try {
            log.println("add following property second time:\n" +
                    "\t('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) ...");
            oObj.addProperty("myXPropertContainerProperty" , com.sun.star.beans.PropertyAttribute.MAYBEVOID, null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: wrong Exception was thrown while adding following property: " +
                    "('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) : " +
                    "Expected: 'PropertyExistException'\n" +
                    "Got:      'IllegalArgumentException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        } catch (PropertyExistException ex) {
            log.println("OK: expected Exception was thrown." + ex.toString());
            test = true;
            exp = true;
        } catch (IllegalTypeException ex) {
            log.println("ERROR: wrong Exception was thrown while adding following property: " +
                    "('myXPropertContainerProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null) : " +
                    "Expected: 'PropertyExistException'\n" +
                    "Got:      'IllegalTypeException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        }
        if (! exp){
            log.println("FAILED: expected exception 'UnknownPropertyException' was not thrown");
        } else {
            if (test) log.println("... OK");
        }

        ok &= test;
        test = false;
        exp = false;
        try {
            log.println("try to add following property:\n" +
                    "\t('IllegalTypeProperty', com.sun.star.beans.PropertyAttribute.MAYBEVOID, Any.VOID) ...");
            oObj.addProperty("IllegalTypeProperty", com.sun.star.beans.PropertyAttribute.MAYBEVOID, Any.VOID);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: wrong Exception was thrown while adding following property: " +
                    "'', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null\n" +
                    "Expected: 'IllegalTypeException'\n" +
                    "Got:      'IllegalArgumentException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        } catch (PropertyExistException ex) {
            log.println("ERROR: wrong Exception was thrown while adding following property: " +
                    "'', com.sun.star.beans.PropertyAttribute.MAYBEVOID, null\n" +
                    "Expected: 'IllegalArgumentException'\n" +
                    "Got:      'PropertyExistException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        } catch (IllegalTypeException ex) {
            log.println("OK: expected Exception was thrown." + ex.toString());
            test = true;
            exp = true;
        }

        if (! exp){
            log.println("FAILED: expected exception 'IllegalTypeException' was not thrown");
        } else {
            if (test) log.println("... OK");
        }

        ok &= test;
        test = false;
        exp = false;
        try {
            short attrib = -1;
            log.println("add following property:\n" +
                    "'', -1, new Object");
            oObj.addProperty("", attrib, null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("OK: expected Exception was thrown." + ex.toString());
            test = true;
            exp = true;
        } catch (PropertyExistException ex) {
            log.println("ERROR: wrong Exception was thrown while adding following property: " +
                    "'invalidPropertyAttribute', -1, null\n" +
                    "Expected: 'IllegalTypeException'\n" +
                    "Got:      'PropertyExistException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        } catch (IllegalTypeException ex) {
            log.println("ERROR: unexpected Exception was thrown while adding following property: " +
                    "'', -1, null\n" +
                    "Expected: 'IllegalArgumentException'\n" +
                    "Got:      'IllegalTypeException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        }

        if (! exp){
            log.println("FAILED: expected exception 'IllegalArgumentException' was not thrown");
        } else {
            if (test) log.println("... OK");
        }

        ok &= test;

        tRes.tested("addProperty()", ok );
        return;
    }

    /**
     * Test calls the method and checks if the returned sequence contanis a propterty which is named
     * in the object relation <code>XPropertyAccess.propertyNotRemovable</code>.
     */
    public void _removeProperty() {

        requiredMethod("addProperty()");

        boolean ok = true;
        boolean test = true;
        boolean exp = false;

        try {
            log.println("remove 'myXPropertContainerProperty'");
            oObj.removeProperty("myXPropertContainerProperty");
        } catch (NotRemoveableException ex) {
            log.println("ERROR: could not remote property 'myXPropertContainerProperty'" + ex.toString());
            log.println("... FAILED");
            test = false;
        } catch (UnknownPropertyException ex) {
            log.println("ERROR: could not remote property 'myXPropertContainerProperty'" + ex.toString());
            log.println("... FAILED");
            test = false;
        }
        if ( test){
            log.println("... OK");
        }

        ok &= test;
        test = false;
        exp=false;

        try {
            log.println("remove not removable property '" + propertyNotRemovable + "'") ;
            oObj.removeProperty(propertyNotRemovable);
        } catch (NotRemoveableException ex) {
            log.println("OK: expected Exception was thrown: " + ex.toString());
            test = true;
            exp = true;
        } catch (UnknownPropertyException ex) {
            log.println("ERROR: wrong exception was thrown.\n" +
                    "Expected: 'NotRemoveableException'\n" +
                    "Got:      'UnknownPropertyException'\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        }

        if (! exp){
            log.println("FAILED: expected exception 'NotRemoveableException' was not thrown");
        } else {
            if (test) log.println("... OK");
        }

        ok &= test;
        test = false;
        exp = false;

        try {
            log.println("remove property 'thisPropertyDoesNotExist'");
            oObj.removeProperty("thisPropertyDoesNotExist");
        } catch (UnknownPropertyException ex) {
            log.println("OK: expected Exception was thrown: " + ex.toString());
            test = true;
            exp = true;
        } catch (NotRemoveableException ex) {
            log.println("ERROR: wrong exception was thrown.\n" +
                    "Expected: 'UnknownPropertyException'\n" +
                    "Got:      'NotRemoveableException\n" +
                    ex.toString());
            log.println("... FAILED");
            exp = true;
        }
        if (! exp){
            log.println("FAILED: expected exception 'UnknownPropertyException' was not thrown");
        } else {
            if (test) log.println("... OK");
        }

        tRes.tested("removeProperty()", ok );
        return;
    }


}    /// finish class XPropertyContainer
