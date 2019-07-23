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
* Test is multithread compliant. <p>
* @see com.sun.star.sdbc.XResultSetMetaDataSupplier
*/
public class _XResultSetMetaDataSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSetMetaDataSupplier oObj = null ;

    /**
    * Tries to get the ResultSetMetaData of the Object</br>
    * an XResultSetMetaData is returned.</br>
    * The test is OK if a not null ResultSetMetaData is returned
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


