/*************************************************************************
 *
 *  $RCSfile: _XPropertyState.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:16:05 $
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
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XPropertyState;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;


/**
* Testing <code>com.sun.star.beans.XPropertyState</code>
* interface methods :
* <ul>
*  <li><code> getPropertyState()</code></li>
*  <li><code> getPropertyStates()</code></li>
*  <li><code> setPropertyToDefault()</code></li>
*  <li><code> getPropertyDefault()</code></li>
* </ul>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated. <p>
* <b>Note:</b> object tested must also implement
* <code>com.sun.star.beans.XPropertySet</code> interface.
* @see com.sun.star.beans.XPropertyState
*/
public class _XPropertyState extends MultiMethodTest {

        public XPropertyState oObj = null;

        private XPropertySet oPS = null ;
        private XPropertySetInfo propertySetInfo = null;
        private Property[] properties = null ;
        private String pName = null ;
        private Object propDef = null ;

        /**
        * Queries object for <code>XPropertySet</code> interface and
        * initializes some fields used by all methods. <p>
        *
        * Searches property which is not READONLY and MAYBEDEFAULT, if
        * such property is not found, then uses property with only
        * READONLY attribute. This property name is stored and is used
        * by all tests.
        *
        * @throws StatusException If <code>XPropertySet</code> is not
        * implemented by object.
        */
        public void before() throws StatusException {
            oPS = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, oObj );
            if (oPS == null)
                throw new StatusException
                    ("XPropertySet interface isn't implemented.",
                        new NullPointerException
                            ("XPropertySet interface isn't implemented.")) ;

            propertySetInfo = oPS.getPropertySetInfo();
            properties = propertySetInfo.getProperties();
            Property prop = null;
            for (int i=0;i<properties.length;i++) {
                try {
                    prop = propertySetInfo.getPropertyByName
                        (properties[i].Name);
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    log.println("Unknown Property "+prop.Name);
                }
                boolean readOnly = (prop.Attributes &
                    PropertyAttribute.READONLY) != 0;
                boolean maybeDefault = (prop.Attributes &
                    PropertyAttribute.MAYBEDEFAULT) != 0;
                if (!readOnly && maybeDefault) {
                    pName = properties[i].Name;
                    log.println("Property '" + pName + "' has attributes "+
                         prop.Attributes);
                    break ;
                } else
                if (!readOnly) {
                    pName = properties[i].Name;
                    log.println("Property '" + pName +
                         "' is not readonly, may be used ...");
                } else {
                    log.println("Skipping property '" + properties[i].Name +
                        "' Readonly: " + readOnly + ", MaybeDefault: " +
                             maybeDefault);
                }
            }

        }

        /**
        * Test calls the method and checks that no exceptions were thrown. <p>
        * Has <b> OK </b> status if no exceptions were thrown. <p>
        */
        public void _getPropertyDefault(){
            boolean result = true ;
            String localName = pName;
            if (localName == null) {
                localName = (propertySetInfo.getProperties()[0]).Name;
            }
            try {
                propDef = oObj.getPropertyDefault(localName);
                log.println("Default property value is : '" + propDef + "'");
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception " + e +
                    "occured while getting Property default");
                result=false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception " + e +
                    "occured while getting Property default");
                result=false;
            }
            tRes.tested("getPropertyDefault()", result);
        }

        /**
        * Test calls the method and checks return value and that
        * no exceptions were thrown. <p>
        * Has <b> OK </b> status if the method returns not null value
        * and no exceptions were thrown. <p>
        */
        public void _getPropertyState(){
            boolean result = true ;

            String localName = pName;
            if (localName == null) {
                localName = (propertySetInfo.getProperties()[0]).Name;
            }

            try {
                PropertyState ps = oObj.getPropertyState(localName);
                if (ps == null) {
                    log.println("!!! Returned value == null") ;
                    result = false ;
                }
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception " + e +
                    "occured while getting Property state");
                result = false;
            }
            tRes.tested("getPropertyState()", result);
        }

        /**
        * Test calls the method with array of one property name
        * and checks return value and that no exceptions were thrown. <p>
        * Has <b> OK </b> status if the method returns array with one
        * PropertyState and no exceptions were thrown. <p>
        */
        public void _getPropertyStates(){
            boolean result = true ;

            String localName = pName;
            if (localName == null) {
                localName = (propertySetInfo.getProperties()[0]).Name;
            }

            try {
                PropertyState[] ps = oObj.getPropertyStates
                    (new String[] {localName});
                if (ps == null) {
                    log.println("!!! Returned value == null") ;
                    result = false ;
                } else {
                    if (ps.length != 1) {
                        log.println("!!! Array lebgth returned is invalid - " +
                             ps.length) ;
                        result = false ;
                    }
                }
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception " + e +
                    "occured while getting Property state");
                result = false;
            }

            tRes.tested("getPropertyStates()", result);
        }


        /**
        * Sets the property to default, then compares the current property
        * value to value received by method <code>getPropertyDefault</code>.
        * Has <b> OK </b> status if the current proeprty value equals to
        * default property. <p>
        * The following method tests are to be completed successfully before :
        * <ul>
        *  <li> <code>getPropertyDefault</code>: we have to know what is
        * default value</li></ul>
        */
        public void _setPropertyToDefault(){
            requiredMethod("getPropertyDefault()") ;

            if (pName == null) {
                log.println("all found properties are read only");
                tRes.tested("setPropertyToDefault()",Status.skipped(true));
                return;
            }

            boolean result = true ;
            try {
                oObj.setPropertyToDefault(pName);
                if ((properties[0].Attributes &
                        PropertyAttribute.MAYBEDEFAULT) != 0) {
                    Object actualDef = propDef ;
                    if (propDef instanceof Any) {
                        actualDef = ((Any)propDef).getObject() ;
                    }
                    Object actualVal = oPS.getPropertyValue(pName) ;
                    if (actualVal instanceof Any) {
                        actualVal = ((Any)actualVal).getObject() ;
                    }
                    result = util.ValueComparer.equalValue
                        (actualDef,actualVal) ;
                    log.println("Default value = '" + actualDef +
                         "', returned value = '"
                        + actualVal + "' for property " + pName) ;
                }
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception " + e +
                    "occured while setting Property to default");
                result=false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception " + e +
                    "occured while testing property value");
                result=false;
            }

            tRes.tested("setPropertyToDefault()", result);
        }

        public void after() {
            disposeEnvironment() ;
        }

 }// EOF _XPropertyState

