/*************************************************************************
 *
 *  $RCSfile: _XFastPropertySet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:14:43 $
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

package ifc.beans;

import java.util.Random;
import java.util.Set;
import java.util.Vector;

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
    private Vector handles = new Vector();
    private int handle = -1;
    private Set exclude = null ;

    /**
     * Retrieves relation.
     */
     protected void before() {
        exclude = (Set) tEnv.getObjRelation("XFastPropertySet.ExcludeProps") ;
        if (exclude == null) {
            exclude = new java.util.HashSet() ;
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
        XPropertySet PS = (XPropertySet)UnoRuntime.queryInterface
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
                log.println("Exception occured while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change property with handle = " + handle);
                e.printStackTrace(log);
            }  catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change property with handle = " + handle);
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
        XPropertySet PS = (XPropertySet)UnoRuntime.queryInterface
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
            log.println("Exception occured while trying to get property '"
                + handle +"'");
            e.printStackTrace(log);
            tRes.tested("getFastPropertyValue()",false);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while trying to get property '"
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
        handle = ((Integer)handles.elementAt(nr)).intValue();
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


