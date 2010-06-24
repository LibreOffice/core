/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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


