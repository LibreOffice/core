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

package ifc.sdb;

import lib.MultiPropertyTest;

import com.sun.star.beans.PropertyValue;

/**
 * Testing <code>com.sun.star.sdb.DataSource</code>
 * service properties :
 * <ul>
 *  <li><code> Name</code></li>
 *  <li><code> URL</code></li>
 *  <li><code> Info</code></li>
 *  <li><code> User</code></li>
 *  <li><code> Password</code></li>
 *  <li><code> IsPasswordRequired</code></li>
 *  <li><code> SuppressVersionColumns</code></li>
 *  <li><code> IsReadOnly</code></li>
 *  <li><code> NumberFormatsSupplier</code></li>
 *  <li><code> TableFilter</code></li>
 *  <li><code> TableTypeFilter</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code> <p>.
* After this interface test <b>it's better to recreate</b> object tested.
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.beans.XPropertySetInfo
* @see com.sun.star.beans.Property
* @see com.sun.star.lang.XServiceInfo
*/
public class _DataSource extends MultiPropertyTest {

    /**
    * This property is an array of additional parameters for database
    * connecting. Parameter is <code>PropertyValue</code> structure.
    * The test just changes existing array onto array with a single
    * element <code>("user", "API_QA_Tester")</code> <p>
    *
    * After testing old value is set for this property. <p>
    *
    * Result is OK: if property successfully changed with no excepions.
    * @see com.sun.star.beans.PropertyValue
    */
    public void _Info() {
        try {
            Object oldInfo = oObj.getPropertyValue("Info") ;

            testProperty("Info", new PropertyTester() {
                protected Object getNewValue(String propName, Object oldValue) {

                    PropertyValue propUsr = new PropertyValue(),
                                  propPass = new PropertyValue() ;

                    propUsr.Name = "user" ;
                    propUsr.Value = "API_QA_Tester" ;
                    propPass.Name = "password" ;
                    propPass.Value = "guest" ;

                    return new PropertyValue[] { propUsr, propPass } ;
                }
            }) ;

            oObj.setPropertyValue("Info", oldInfo) ;
        } catch(com.sun.star.beans.UnknownPropertyException e) {}
        catch(com.sun.star.beans.PropertyVetoException e) {}
        catch(com.sun.star.lang.IllegalArgumentException e) {}
        catch(com.sun.star.lang.WrappedTargetException e) {}
    }

    /**
    * Property is tested by the common method, but after testing
    * old value is set for this property.
    */
    public void _URL() {
        try {
            Object oldURL = oObj.getPropertyValue("URL") ;

            testProperty("URL") ;

            oObj.setPropertyValue("URL", oldURL) ;
        } catch(com.sun.star.beans.UnknownPropertyException e) {}
        catch(com.sun.star.beans.PropertyVetoException e) {}
        catch(com.sun.star.lang.IllegalArgumentException e) {}
        catch(com.sun.star.lang.WrappedTargetException e) {}
    }

    /**
    * Property is tested by the common method, but after testing
    * old value is set for this property.
    */
    public void _User() {
        try {
            Object oldUser = oObj.getPropertyValue("User") ;

            testProperty("User") ;

            oObj.setPropertyValue("User", oldUser) ;
        } catch(com.sun.star.beans.UnknownPropertyException e) {}
        catch(com.sun.star.beans.PropertyVetoException e) {}
        catch(com.sun.star.lang.IllegalArgumentException e) {}
        catch(com.sun.star.lang.WrappedTargetException e) {}
    }

    /**
    * Property is tested by the common method, but after testing
    * old value is set for this property.
    */
    public void _Password() {
        try {
            Object oldPass = oObj.getPropertyValue("Password") ;

            testProperty("Password") ;

            oObj.setPropertyValue("Password", oldPass) ;
        } catch(com.sun.star.beans.UnknownPropertyException e) {}
        catch(com.sun.star.beans.PropertyVetoException e) {}
        catch(com.sun.star.lang.IllegalArgumentException e) {}
        catch(com.sun.star.lang.WrappedTargetException e) {}
    }

    /**
    * New value for the test is always <code>null</code>.
    */
    public void _NumberFormatsSupplier() {
        testProperty("NumberFormatsSupplier", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                return null ;
            }
        }) ;
    }

    /**
    * If object test allows to recreate environment it is better to do it.
    */
    public void after() {
        try {
            oObj.setPropertyValue("IsPasswordRequired",new Boolean(false));
        } catch (Exception e) {
            log.println("Couldn't set 'IsPasswordRequired' to false");
        }
    }

}  // finish class _DataSource


