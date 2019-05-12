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



package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XTextField;

/**
 * Testing <code>com.sun.star.text.XTextField</code>
 * interface methods :
 * <ul>
 *  <li><code> getPresentation()</code></li>
 * </ul> <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.text.XTextField
 */
public class _XTextField extends MultiMethodTest{
    public XTextField oObj = null;

    /**
     * Calls the method with <code>true</code> and <code>false</code>
     * parameter. <p>
     * Has <b>OK</b> status if in both cases not <code>null</code>
     * value returned.
     */
    public void _getPresentation() {

        boolean result = true;

        // begin test here
        log.println("getting presentetion with bShowCommand flag...");
        result &= oObj.getPresentation(true) != null;
        log.println("getting presentetion without bShowCommand flag...");
        result &= oObj.getPresentation(false) != null;

        tRes.tested( "getPresentation()", result );

     } // end getPresentation()
}

