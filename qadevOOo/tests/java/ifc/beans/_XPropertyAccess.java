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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.lang.WrappedTargetException;
import lib.MultiMethodTest;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyAccess;
import lib.Status;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.beans.XPropertyAccess</code>
 * interface methods :
 * <ul>
 *  <li><code>getPropertyValues()</code></li>
 *  <li><code>setPropertyValues()</code></li>
 * </ul>
 * @see com.sun.star.beans.XPropertyAccess
 */
public class _XPropertyAccess extends MultiMethodTest {

    /**
     * oObj filled by MultiMethodTest
     */
    public XPropertyAccess oObj = null;// oObj filled by MultiMethodTest

    /**
     * object relation X<CODE>PropertyAccess.propertyToChange</CODE><br>
     * This relation must be filled from the module. It contains a property which must
     * be kind of String property, available at <CODE>getPropertyValues()</CODE> and changeable by
     * <CODE>setPropertyValues()</CODE>
     */
    public PropertyValue propertyToChange = null;

    /**
     * checks if the object relation <CODE>XPropertyAccess.propertyToChange</CODE>
     * is available
     */
    public void before() {
        propertyToChange = (PropertyValue) tEnv.getObjRelation("XPropertyAccess.propertyToChange");
        if (propertyToChange == null) {
            throw new StatusException(Status.failed("Object raltion 'XPropertyAccess.propertyToChange' is null"));
        }
    }

    /**
     * Test calls the method and checks if the returned sequenze contanis a propterty which is named
     * in the object relation <code>XPropertyAccess.propertyToChange</code>.
     */
    public void _getPropertyValues() {
        PropertyValue[] properties = oObj.getPropertyValues();

        boolean ok = true;

        if (properties != null){

            boolean found = false;
            for (int i=0; i < properties.length; i++){
                if (properties[i].Name.equals(propertyToChange.Name)) found = true;
            }
            if (! found){
                log.println("ERROR: could not find desired property '"+ propertyToChange.Name+"'");
                ok=false;
            }

        } else {
            log.println("ERROR: the method returned NULL");
            ok =false;
        }

        tRes.tested("getPropertyValues()", ok );
        return;
    }

    /**
     * Test calls the method and checks if:
     * <ul>
     *    <li>the property given by the object relation
     *    <CODE>XPropertyAccess.propertyToChange</CODE> has changed</LI>
     *    <li><CODE>com.sun.star.lang.IllegalArgumentException</CODE> was thrown if a <CODE>Integer</CODE>
     *    value was set to a <CODE>String</CODE> property</LI>
     *    <li><CODE>com.sun.star.beans.UnknownPropertyException</CODE> was throen if an invalid property
     *    was set</LI>
     * </ul>
     */
    public void _setPropertyValues(){

        boolean ok = true;
        boolean test = true;
        boolean exp = false;

        try {
            PropertyValue[] newProps = new PropertyValue[1];
            newProps[0] = propertyToChange;

            log.println("try to set property vlaues given by object relation 'XPropertyAccess.propertyToChange'...");
            oObj.setPropertyValues(newProps);

        } catch (UnknownPropertyException ex) {
            log.println("ERROR: Exception was thrown while trying to set property value: " +
                ex.toString());
            test = false;
        } catch (PropertyVetoException ex) {
            log.println("ERROR: Exception was thrown while trying to set property value: " +
                ex.toString());
            test = false;
        } catch (WrappedTargetException ex) {
            log.println("ERROR: Exception was thrown while trying to set property value: " +
                ex.toString());
            test = false;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: Exception was thrown while trying to set property value: " +
                ex.toString());
            test = false;
        }

        if ( test){
            log.println("... OK");
        }

        ok &= test;
        test = false;
        exp = false;
        try {
            log.println("try to set integer value to string property, " +
                "expect 'com.sun.star.lang.IllegalArgumentException'...");
            PropertyValue[] newProps = new PropertyValue[1];
            PropertyValue failedProp = new PropertyValue();
            failedProp.Name = propertyToChange.Name;
            failedProp.Value = new Integer(10);
            newProps[0] = failedProp;
            oObj.setPropertyValues(newProps);
        } catch (PropertyVetoException ex) {
            log.println("ERROR: unexptected exception was thrown while trying to set null value: " +
                ex.toString());
            exp = true;
        } catch (WrappedTargetException ex) {
            log.println("ERROR: unexptected exception was thrown while trying to set null value: " +
                ex.toString());
            exp = true;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("OK: exptected exception was thrown while trying to set null value: " +
                ex.toString());
            test = true;
            exp = true;
        } catch (UnknownPropertyException ex) {
            log.println("ERROR: unexptected exception was thrown while trying to set null value: " +
                ex.toString());
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

            log.println("try to set values with invalid property name. " +
                "Expect 'com.sun.star.beans.UnknownPropertyException'...");

            PropertyValue[] newProps = new PropertyValue[1];
            PropertyValue newProp = new PropertyValue();
            newProp.Name = "XPropertyAccess.InvalidPropertyName";
            newProp.Value = "invalid property";
            newProps[0] = newProp;

            oObj.setPropertyValues(newProps);

        } catch (WrappedTargetException ex) {
            log.println("ERROR: unexptected exception was thrown while trying to set invalid value: " +
                ex.toString());
            exp = true;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: unexptected exception was thrown while trying to set invalid value: " +
                ex.toString());
            exp = true;
        } catch (PropertyVetoException ex) {
            log.println("ERROR: unexptected exception was thrown while trying to set invalid value: " +
                ex.toString());
            exp = true;
        } catch (UnknownPropertyException ex) {
            log.println("OK: Exptected exception was thrown while trying to set invalid value: " +
                ex.toString());
            exp = true;
            test = true;
        }

        ok &= test;

        if (! exp){
            log.println("FAILED: expected exception 'UnknownPropertyException' was not thrown");
        } else {
            if (test) log.println("... OK");
        }

        tRes.tested("setPropertyValues()", ok);
        return;

    }

}    /// finish class XPropertyAccess


