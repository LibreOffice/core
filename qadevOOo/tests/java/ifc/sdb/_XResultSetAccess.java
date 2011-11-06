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



package ifc.sdb;

import lib.MultiMethodTest;

import com.sun.star.sdb.XResultSetAccess;
import com.sun.star.sdbc.XResultSet;

/**
* Testing <code>com.sun.star.sdb.XResultSetAccess</code>
* interface methods :
* <ul>
*  <li><code> createResultSet()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.sdb.XResultSetAccess
*/
public class _XResultSetAccess extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSetAccess oObj = null ;

    /**
    * tries to create a ResultSet for the Object</br>
    * an XResultSet is returned.</br>
    * The test is OK if an not null ResultSet is returned
    */
    public void _createResultSet() {

        try {
            XResultSet the_Set = oObj.createResultSet();
            if (the_Set == null) log.println("'createResulSet()' returns NULL");
            tRes.tested("createResultSet()",the_Set != null);
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Exception while checking 'createResultSet()'");
            e.printStackTrace(log);
            tRes.tested("createResultSet()",false);
        }

    }
}  // finish class _XResultSetAccess


