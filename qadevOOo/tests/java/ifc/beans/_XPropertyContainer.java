/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XPropertyContainer.java,v $
 * $Revision: 1.3 $
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
            log.println("remove not removeable property '" + propertyNotRemovable + "'") ;
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
