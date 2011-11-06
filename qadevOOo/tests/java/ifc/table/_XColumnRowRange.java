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



package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.table.XColumnRowRange;

/**
* Testing <code>com.sun.star.table.XColumnRowRange</code>
* interface methods :
* <ul>
*  <li><code> getColumns()</code></li>
*  <li><code> getRows()</code></li>
* </ul> <p>
* @see com.sun.star.table.XColumnRowRange
*/
public class _XColumnRowRange extends MultiMethodTest {

    public XColumnRowRange oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not null value
     * and no exceptions were thrown. <p>
     */
    public void _getColumns() {
        tRes.tested( "getColumns()", oObj.getColumns()!=null );
    } // getColumns()

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not null value
     * and no exceptions were thrown. <p>
     */
    public void _getRows() {
        tRes.tested( "getRows()", oObj.getRows()!=null );
    } // getRows()

} // finish class _XColumnRowRange

