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



package ifc.sdbc;

import lib.MultiMethodTest;

import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;

/**
* Testing <code>com.sun.star.sdbc.XResultSetMetaDataSupplier</code>
* interface methods :
* <ul>
*  <li><code> getMetaData()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.sdbc.XResultSetMetaDataSupplier
*/
public class _XResultSetMetaDataSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSetMetaDataSupplier oObj = null ;

    /**
    * Tries to get the ResultSetMetaData of the Object</br>
    * an XResultSetMetaData is returned.</br>
    * The test is OK if an not null ResultSetMetaData is returned
    */
    public void _getMetaData() {

        try {
            XResultSetMetaData the_Meta = oObj.getMetaData();
            if (the_Meta == null) log.println("'getMetaData()' returns NULL");
            tRes.tested("getMetaData()",the_Meta != null);
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Exception while checking 'createResultSet()'");
            e.printStackTrace(log);
            tRes.tested("getMetaData()",false);
        }

    }
}  // finish class _XResultSetMetaDataSupplier


