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

package ifc.sdbcx;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XColumnsSupplier;

/**
* Testing <code>com.sun.star.sdbcx.XColumnsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getColumns()</code></li>
* </ul> <p>
* @see com.sun.star.sdbcx.XColumnsSupplier
*/
public class _XColumnsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XColumnsSupplier oObj = null ;

    /**
    * Has OK status if method returns not null
    * <code>XNameAccess</code> object, FAILED otherwise.
    */
    public void _getColumns() {

        XNameAccess cols = oObj.getColumns() ;

        /*
        String[] colNames = cols.getElementNames() ;

        log.println("Column names:") ;
        for (int i =0 ; i < colNames.length; i++) {
            log.println("  " + colNames[i]) ;
        }

        XServiceInfo info = null ;
        try {
            info = (XServiceInfo) UnoRuntime.queryInterface
                (XServiceInfo.class, cols.getByName("colNames[0]")) ;
        } catch (com.sun.star.uno.Exception e) {}

        log.println("Support : " +
            info.supportsService("com.sun.star.sdbcx.Column")) ;
        */

        tRes.tested("getColumns()", cols != null) ;
    }

}  // finish class _XColumnsSupplier


