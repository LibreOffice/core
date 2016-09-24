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

