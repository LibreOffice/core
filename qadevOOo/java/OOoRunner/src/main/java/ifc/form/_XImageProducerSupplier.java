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

import com.sun.star.awt.XImageProducer;
import com.sun.star.form.XImageProducerSupplier;

/**
* Testing <code>com.sun.star.form.XImageProducerSupplier</code>
* interface methods :
* <ul>
*  <li><code> getImageProducer()</code></li>
* </ul>
* @see com.sun.star.form.XImageProducerSupplier
*/
public class _XImageProducerSupplier extends MultiMethodTest {

    public XImageProducerSupplier oObj = null;

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns non null value
    * and no exceptions were thrown. <p>
    */
    public void _getImageProducer() {
        boolean bResult = false;
        XImageProducer ip = oObj.getImageProducer();
        bResult = ip!=null;
        tRes.tested("getImageProducer()", bResult);
    }
}


