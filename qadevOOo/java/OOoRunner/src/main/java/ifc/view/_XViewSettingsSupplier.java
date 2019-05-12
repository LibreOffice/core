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



package ifc.view;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.view.XViewSettingsSupplier;


/**
 * Testing <code>com.sun.star.view.XViewSettingsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getViewSettings()</code></li>
 * </ul> <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.view.XViewSettingsSupplier
 */
public class _XViewSettingsSupplier extends MultiMethodTest {

    public XViewSettingsSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getViewSettings(){

        XPropertySet oTVS = oObj.getViewSettings();
        tRes.tested("getViewSettings()", oTVS != null);
    } // finish _getViewSettings

}  // finish class _XViewSettingsSupplier


