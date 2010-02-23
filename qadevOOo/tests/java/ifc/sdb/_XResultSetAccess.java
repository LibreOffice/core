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


