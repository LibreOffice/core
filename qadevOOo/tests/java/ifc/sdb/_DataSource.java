/*************************************************************************
 *
 *  $RCSfile: _DataSource.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:50:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


