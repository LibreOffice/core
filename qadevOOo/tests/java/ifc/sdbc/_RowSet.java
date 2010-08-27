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

import lib.MultiPropertyTest;

import com.sun.star.uno.Any;


/**
* Testing <code>com.sun.star.sdbc.RowSet</code>
* service properties :
* <ul>
*  <li><code> DataSourceName</code></li>
*  <li><code> URL</code></li>
*  <li><code> Command</code></li>
*  <li><code> TransactionIsolation</code></li>
*  <li><code> TypeMap</code></li>
*  <li><code> EscapeProcessing</code></li>
*  <li><code> QueryTimeOut</code></li>
*  <li><code> MaxFieldSize</code></li>
*  <li><code> MaxRows</code></li>
*  <li><code> User</code></li>
*  <li><code> Password</code></li>
*  <li><code> ResultSetType</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sdbc.RowSet
*/
public class _RowSet extends MultiPropertyTest {

    /**
    * Redefined method returns object, that contains changed property value.
    */
    public void _TypeMap() {
        boolean result = false;
        try {
            Any TypeMap = (Any) oObj.getPropertyValue("TypeMap");
            String TypeName = TypeMap.getType().getTypeName();
            String expected = "com.sun.star.container.XNameAccess";
            result = TypeName.equals(expected);
            if (! result ) {
                log.println("Expected Type is "+expected);
                log.println("but the returned is "+TypeName);
            }
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception " + e.getMessage());
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception " + e.getMessage());
        }

        tRes.tested("TypeMap",result);
    }

}  // finish class _RowSet

