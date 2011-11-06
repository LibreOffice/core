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



package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XMasterPagesSupplier;

/**
* Testing <code>com.sun.star.drawing.XMasterPagesSupplier</code>
* interface methods :
* <ul>
*  <li><code> getMasterPages()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XMasterPagesSupplier
*/
public class _XMasterPagesSupplier extends MultiMethodTest{
    public XMasterPagesSupplier oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the value returned is not null.
    */
    public void _getMasterPages(){
        boolean result = false;
        XDrawPages DrawPages = null;

        log.println("get the MasterPages");

        DrawPages = oObj.getMasterPages();
        result = DrawPages != null;

        tRes.tested("getMasterPages()",result);
    }
}

