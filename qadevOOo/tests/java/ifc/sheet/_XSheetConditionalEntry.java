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



package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XSheetConditionalEntry;

/**
* Testing <code>com.sun.star.sheet.XSheetConditionalEntry</code>
* interface methods :
* <ul>
*  <li><code> getStyleName()</code></li>
*  <li><code> setStyleName()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetConditionalEntry
*/
public class _XSheetConditionalEntry extends MultiMethodTest {

    public XSheetConditionalEntry oObj = null;
    public String StyleName = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getStyleName() {
        StyleName = oObj.getStyleName();
        tRes.tested("getStyleName()", StyleName != null);
    }

    /**
    * Test sets new value of style name, gets current style name and compares
    * returned value with value that was stored by method <code>getStyleName()
    * </code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getStyleName() </code> : to have current style name </li>
    * </ul>
    */
    public void _setStyleName() {
        requiredMethod("getStyleName()");
        oObj.setStyleName("Heading");
        tRes.tested("setStyleName()", !StyleName.equals(oObj.getStyleName()));
    }

}  // finish class _XSheetConditionalEntry


