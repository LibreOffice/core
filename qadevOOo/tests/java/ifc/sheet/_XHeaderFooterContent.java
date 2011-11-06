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

import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.text.XText;

/**
 * Testing <code>com.sun.star.sheet.XHeaderFooterContent</code>
 * interface methods :
 * <ul>
 *  <li><code> getLeftText()</code></li>
 *  <li><code> getCenterText()</code></li>
 *  <li><code> getRightText()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.sheet.XHeaderFooterContent
 */
public class _XHeaderFooterContent extends MultiMethodTest {

    public XHeaderFooterContent oObj = null;

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"LEFT"</code>.
    * <p>
    */
    public void _getLeftText() {
        XText left = oObj.getLeftText();
        tRes.tested("getLeftText()",left.getString().equals("LEFT"));
    }

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"RIGHT"</code>.
    * <p>
    */
    public void _getRightText() {
        XText right = oObj.getRightText();
        tRes.tested("getRightText()",right.getString().equals("RIGHT"));
    }

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"CENTER"</code>.
    * <p>
    */
    public void _getCenterText() {
        XText center = oObj.getCenterText();
        tRes.tested("getCenterText()",center.getString().equals("CENTER"));
    }

}  // finish class _XHeaderFooterContent


