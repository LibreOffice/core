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



package ifc.sdbcx;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.sdbcx.XTablesSupplier</code>
* interface methods :
* <ul>
*  <li><code> getTables()</code></li>
* </ul> <p>
* @see com.sun.star.sdbcx.XTablesSupplier
*/
public class _XTablesSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XTablesSupplier oObj = null ;

    /**
    * Has OK status if method returns not null
    * <code>XNameAccess</code> object, FAILED otherwise.
    */
    public void _getTables() {
        boolean result = true ;

        XNameAccess tabs = oObj.getTables() ;


        String[] tabNames = tabs.getElementNames() ;

        log.println("Table names:") ;
        for (int i =0 ; i < tabNames.length; i++) {
            log.println("  " + tabNames[i]) ;
        }

        XServiceInfo info = null ;
        try {
            info = (XServiceInfo) UnoRuntime.queryInterface
                (XServiceInfo.class, tabs.getByName(tabNames[0])) ;
        } catch (com.sun.star.uno.Exception e) {}

        log.println("Support : " +
            info.supportsService("com.sun.star.sdbcx.Table")) ;


        tRes.tested("getTables()", tabs != null) ;
    }

}  // finish class _XTablesSupplier


