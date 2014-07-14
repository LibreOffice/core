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

import com.sun.star.beans.XPropertySet;
import com.sun.star.form.XGridColumnFactory;


/**
* Testing <code>com.sun.star.form.XGridColumnFactory</code>
* interface methods :
* <ul>
*  <li><code> createColumn()</code></li>
*  <li><code> getColumnTypes()</code></li>
* </ul>
* Test is multithread compilant. <p>
* @see com.sun.star.form.XGridColumnFactory
*/
public class _XGridColumnFactory extends MultiMethodTest {

    public XGridColumnFactory oObj = null;


    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns
    * non null value.
    */
    public void _getColumnTypes() {

        String[] types = oObj.getColumnTypes();
        tRes.tested("getColumnTypes()",types!=null);
    }

    /**
    * Creates column of type 'TextField'. <p>
    * Has <b>OK</b> status if non null value returned.
    */
    public void _createColumn() {

        try {
            XPropertySet column = oObj.createColumn("TextField");
            tRes.tested("createColumn()",column!=null);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while creating Column");
            tRes.tested("createColumn()",false);
        }
    }
}  // finish class _XGridColumnFactory



