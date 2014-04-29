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



package ifc.style;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.style.XStyle;

/**
* Testing <code>com.sun.star.style.XStyle</code>
* interface methods :
* <ul>
*  <li><code> isUserDefined()</code></li>
*  <li><code> isInUse()</code></li>
*  <li><code> getParentStyle()</code></li>
*  <li><code> setParentStyle()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'PoolStyle'</code> (of type <code>XStyle</code>):
*   some style from the SOffice collection (not user defined) </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.style.XStyle
*/
public class _XStyle extends MultiMethodTest {

    public XStyle oObj = null;
    XStyle oMyStyle = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        oMyStyle = (XStyle) tEnv.getObjRelation("PoolStyle");
        if (oMyStyle == null) throw new StatusException
            (Status.failed("Relation not found")) ;
    }

    /**
    * Gets the parent style . <p>
    * Has <b> OK </b> status if the name of style returned is
    * equal to the name of style which was set before. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setParentStyle() </code> : to set the parent style </li>
    * </ul>
    */
    public void _getParentStyle() {
        requiredMethod("setParentStyle()");
        tRes.tested("getParentStyle()",
            oObj.getParentStyle().equals(oMyStyle.getName()));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method sreturns <code>true</code>. <p>
    */
    public void _isInUse() {
        tRes.tested("isInUse()",oObj.isInUse());
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns <code>true</code>. <p>
    */
    public void _isUserDefined() {
        tRes.tested("isUserDefined()",
            oObj.isUserDefined() && !oMyStyle.isUserDefined() );
    }

    /**
    * Sets the style name which was passed as relation. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setParentStyle() {
        boolean result = true ;
        try {
            oObj.setParentStyle(oMyStyle.getName());
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Exception occurred while method call: " + e);
            result = false ;
        }

        tRes.tested("setParentStyle()",result);
    }
} //finish class _XStyle

