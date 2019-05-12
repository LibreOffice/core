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



package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XBoundControl;

/**
* Testing <code>com.sun.star.form.XBoundControl</code>
* interface methods:
* <ul>
*  <li><code> getLock() </code></li>
*  <li><code> setLock() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XBoundControl
*/
public class _XBoundControl extends MultiMethodTest {
    public XBoundControl oObj = null;
    public boolean defaultState = false;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _getLock() {
        boolean res = false;
        defaultState = oObj.getLock();
        res = true;
        tRes.tested("getLock()",res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if method locks input (set and gotten values are
    * equal). <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLock() </code>: determines whether the input is
    *  currently locked or not </li>
    * </ul>
    */
    public void _setLock() {
        boolean res = false;
        boolean toSet = !defaultState;

        requiredMethod("getLock()");
        oObj.setLock(toSet);
        res = (oObj.getLock() != defaultState);
        oObj.setLock(defaultState);
        tRes.tested("setLock()", res);
    }

} //EOF of XBoundControl

