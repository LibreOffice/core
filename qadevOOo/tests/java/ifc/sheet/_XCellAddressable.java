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



package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XCellAddressable;
import com.sun.star.table.CellAddress;

/**
* Testing <code>com.sun.star.sheet.XCellAddressable</code>
* interface methods :
* <ul>
*  <li><code> getCellAddress()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XCellAddressable
*/
public class _XCellAddressable extends MultiMethodTest {

    public XCellAddressable oObj = null;
               CellAddress sAddr = null;

    /**
     * Test calls the method and checks returned value.
     * Has <b> OK </b> status if returned value isn't null. <p>
     */
    public void _getCellAddress(){
        sAddr = oObj.getCellAddress() ;
        tRes.tested("getCellAddress()", sAddr != null) ;
    }


} //EOC _XCellAddressable

