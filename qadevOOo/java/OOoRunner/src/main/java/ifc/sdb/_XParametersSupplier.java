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

import com.sun.star.container.XIndexAccess;
import com.sun.star.sdb.XParametersSupplier;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.sdb.XParametersSupplier</code>
* interface methods :
* <ul>
*  <li><code> getParameters()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.sdb.XParametersSupplier
*/
public class _XParametersSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XParametersSupplier oObj = null ;

    /**
    * checks of the return of <code>getParameters()</code>
    * is not null
    */
    public void _getParameters() {

        XIndexAccess the_Set = oObj.getParameters();
        if (the_Set == null) log.println("'getParameters()' returns NULL");
        tRes.tested("getParameters()",the_Set != null);

    }
}  // finish class _XParametersSupplier


