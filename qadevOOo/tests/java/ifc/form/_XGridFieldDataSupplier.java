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

import com.sun.star.form.XGridFieldDataSupplier;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.form.XGridFieldDataSupplier</code>
* interface methods :
* <ul>
*  <li><code>queryFieldDataType()</code></li>
*  <li><code>queryFieldData()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XGridFieldDataSupplier
*/
public class _XGridFieldDataSupplier extends MultiMethodTest {
     public XGridFieldDataSupplier oObj = null;

    /**
    * Calls the method and checks returned value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
     public void _queryFieldDataType() {
        boolean[] dataType = oObj.queryFieldDataType(new Type(String.class));
        tRes.tested("queryFieldDataType()", dataType != null);
     }

    /**
    * Calls the method and checks returned value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
     public void _queryFieldData() {
        Object[] data = oObj.queryFieldData(0, new Type(String.class));
        tRes.tested("queryFieldData()", data != null);
     }
}// finish class _XGridFieldDataSupplier

