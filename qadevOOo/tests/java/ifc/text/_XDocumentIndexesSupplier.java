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

import com.sun.star.container.XIndexAccess;
import com.sun.star.text.XDocumentIndexesSupplier;

import lib.MultiMethodTest;


/**
 * Testing <code>com.sun.star.text.XDocumentIndexesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getDocumentIndexes()()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XDocumentIndexesSupplier
 */
public class _XDocumentIndexesSupplier extends MultiMethodTest {

    public static XDocumentIndexesSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the collection returned has at least
     * one element.
     */
    public void _getDocumentIndexes() {
        boolean res = false;

        XIndexAccess the_documentIndexes = oObj.getDocumentIndexes();
        res = the_documentIndexes.getCount()>0;

        tRes.tested("getDocumentIndexes()",res);
    }

}  // finish class _XDocumentIndexesSupplier
