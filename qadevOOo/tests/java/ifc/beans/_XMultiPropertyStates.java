/*************************************************************************
 *
 *  $RCSfile: _XMultiPropertyStates.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:15:28 $
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

    Object[] defaults = null;
    PropertyState[] states = null;
    String[] names = null;

    public void before() {
        names = (String[]) tEnv.getObjRelation("PropertyNames");
        if (names == null) {
            throw new StatusException(Status.failed("No PropertyNames given"));
        }

        log.println("Totally " + names.length + " properties encountered.");
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
            defaults = oObj.getPropertyDefaults(names);
            result = (defaults != null) && defaults.length == names.length;
            log.println("Number of default values: " + defaults.length);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("some properties seem to be unknown");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Wrapped target Exception was thrown");
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
            log.println("some properties seem to be unknown");
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
                } catch(com.sun.star.beans.UnknownPropertyException e) {
                    log.println("couldn't get property info");
                    throw new StatusException(Status.failed
                        ("couldn't get property info"));
                }
                if ( (prop.Attributes & PropertyAttribute.MAYBEDEFAULT) == 1 ){
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
            log.println("some properties seem to be unknown");
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

        oObj.setAllPropertiesToDefault();
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
                            PropertyAttribute.MAYBEDEFAULT) == 1 ) {
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
            log.println("some properties seem to be unknown");
            result=false;
        }

        tRes.tested("setAllPropertiesToDefault()", result) ;
    }

}

