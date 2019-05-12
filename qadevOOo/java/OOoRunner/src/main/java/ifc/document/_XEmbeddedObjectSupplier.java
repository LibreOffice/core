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

import com.sun.star.document.XEmbeddedObjectSupplier;

/**
* Testing <code>com.sun.star.document.XEmbeddedObjectSupplier</code>
* interface methods :
* <ul>
*  <li><code> getEmbeddedObject()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.document.XEmbeddedObjectSupplier
*/
public class _XEmbeddedObjectSupplier extends MultiMethodTest {

    public XEmbeddedObjectSupplier oObj = null;

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned.
    */
    public void _getEmbeddedObject() {
        tRes.tested("getEmbeddedObject()",
                oObj.getEmbeddedObject() != null);
    }
}  // finish class _XEmbeddedObjectSupplier

