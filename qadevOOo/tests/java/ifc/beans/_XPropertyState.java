/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                    "occurred while getting Property default");
                result=false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception " + e +
                    "occurred while getting Property default");
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
                    "occurred while getting Property state");
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
                    "occurred while getting Property state");
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
                try {
                    oObj.setPropertyToDefault(pName);
                }
                catch(RuntimeException e) {
                    System.out.println("Ignoring RuntimeException: " + e.getMessage());
                }
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
                    "occurred while setting Property to default");
                result=false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception " + e +
                    "occurred while testing property value");
                result=false;
            }

            tRes.tested("setPropertyToDefault()", result);
        }

        public void after() {
            disposeEnvironment() ;
        }

 }// EOF _XPropertyState

