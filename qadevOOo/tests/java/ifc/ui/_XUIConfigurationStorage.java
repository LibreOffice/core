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



package ifc.ui;

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationStorage;
import lib.MultiMethodTest;
/**
* Testing <code>com.sun.star.ui.XUIConfigurationStorage</code>
* interface methods :
* <ul>
*  <li><code> setStorage()</code></li>
*  <li><code> hasStorage()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XUIConfigurationStorage
*/

public class _XUIConfigurationStorage extends MultiMethodTest {

    public XUIConfigurationStorage oObj;
    private XStorage xStore = null;

    public void before() {
        xStore = (XStorage)tEnv.getObjRelation("XUIConfigurationStorage.Storage");
    }

    /**
    * Set the object relation <code>XUIConfigurationStorage.Storage</code>
    * as storage.
    * Has <b>OK</b> status if no exception is thrown.
    */
    public void _setStorage() {
        if (!oObj.hasStorage())
            oObj.setStorage(xStore);
        else
            log.println("For initializing this object, the setStorage() method was already called at the object.");
        tRes.tested("setStorage()", true);
    }

    /**
     * Queries object for a storage.
     * Has <b>OK</b> status if <code>hasStorage</code> returns <code>OK</code>
     */
    public void _hasStorage() {
        requiredMethod("setStorage()");
        boolean has = oObj.hasStorage();
        tRes.tested("hasStorage()", has);
    }

}