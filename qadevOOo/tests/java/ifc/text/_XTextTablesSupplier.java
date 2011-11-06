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

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextTablesSupplier;


/**
 * Testing <code>com.sun.star.text.XTextTablesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getTextTables()</code></li>
 * </ul> <p>
 * The table collection tested <b>must</b> have a table
 * with name 'SwXTextDocument'. <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.text.XTextTablesSupplier
 */
public class _XTextTablesSupplier extends MultiMethodTest {

    public static XTextTablesSupplier oObj = null;

    /**
     * Get the collection of tables. Then check it. <p>
     * Has <b>OK</b> status if returned collection contains
     * element with name 'SwXTextDocument'.
     */
    public void _getTextTables() {
        boolean res = false;

        XNameAccess the_tables = oObj.getTextTables();
        res = the_tables.hasByName("SwXTextDocument");

        tRes.tested("getTextTables()",res);
    }

}  // finish class _XTextTablesSupplier

