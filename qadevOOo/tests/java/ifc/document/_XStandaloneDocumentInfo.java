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



package ifc.document;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.io.IOException;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.document.XStandaloneDocumentInfo</code>
* interface methods. <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DOCURL'</code> (of type <code>String</code>):
*   URL of document which info is loaded.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XStandaloneDocumentInfo
*/
public class _XStandaloneDocumentInfo extends MultiMethodTest {

    public XStandaloneDocumentInfo oObj = null;
    String url = null;

    protected void before() {
        url = (String)tEnv.getObjRelation("DOCURL");
        if (url == null) {
            throw new StatusException
                (Status.failed("Relation 'DOCURL' not found"));
        }
    }

    String oldProp = null;
    String newProp = null;
    /**
    * Sets new value of the property 'Author' and calls the method. <p>
    * Has <b> OK </b> status if no exception occurred.
    */
    public void _storeIntoURL() {
        try {
            oObj.loadFromURL(url);
            XPropertySet propSet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oObj);
            oldProp = (String)propSet.getPropertyValue("Author");
            newProp = oldProp + "_";
            propSet.setPropertyValue("Author", newProp);

            oObj.storeIntoURL(url);
            tRes.tested("storeIntoURL()", true);
        } catch (IOException e) {
            log.println("Couldn't store to " + url
                    + " : " + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        }
    }

    /**
    * Calls the method and checks value of the property 'Author'. <p>
    * Has <b> OK </b> status if no exception occurred and value of the property
    * 'Author' is equal to value that was set in the method
    * <code>storeIntoURL</code>.
    */
    public void _loadFromURL() {
        requiredMethod("storeIntoURL()");
        try {
            oObj.loadFromURL(url);
            XPropertySet propSet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oObj);
            String resProp = (String)propSet.getPropertyValue("Author");
            log.println("Was: '" + oldProp + "',Set: '" + newProp +
                        "', New: " + resProp + "'");
            tRes.tested("loadFromURL()", resProp.equals(newProp) );
        } catch (IOException e) {
            log.println("Couldn't load from " + url
                    + " : " + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("loadFromURL()", false);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("loadFromURL()", false);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("loadFromURL()", false);
        }
    }

}  // finish class _XStandaloneDocumentInfo

