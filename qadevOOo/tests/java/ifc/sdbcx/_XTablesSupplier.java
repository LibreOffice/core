/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        XNameAccess tabs = oObj.getTables() ;


        String[] tabNames = tabs.getElementNames() ;

        log.println("Table names:") ;
        for (int i =0 ; i < tabNames.length; i++) {
            log.println("  " + tabNames[i]) ;
        }

        XServiceInfo info = null ;
        try {
            info = UnoRuntime.queryInterface
                (XServiceInfo.class, tabs.getByName(tabNames[0])) ;
        } catch (com.sun.star.uno.Exception e) {}

        if (info != null) {
            log.println("Support : " +
                info.supportsService("com.sun.star.sdbcx.Table")) ;
        }

        tRes.tested("getTables()", info != null) ;
    }

}  // finish class _XTablesSupplier


