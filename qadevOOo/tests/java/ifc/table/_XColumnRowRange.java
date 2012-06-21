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

package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.table.XColumnRowRange;

/**
* Testing <code>com.sun.star.table.XColumnRowRange</code>
* interface methods :
* <ul>
*  <li><code> getColumns()</code></li>
*  <li><code> getRows()</code></li>
* </ul> <p>
* @see com.sun.star.table.XColumnRowRange
*/
public class _XColumnRowRange extends MultiMethodTest {

    public XColumnRowRange oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not null value
     * and no exceptions were thrown. <p>
     */
    public void _getColumns() {
        tRes.tested( "getColumns()", oObj.getColumns()!=null );
    } // getColumns()

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not null value
     * and no exceptions were thrown. <p>
     */
    public void _getRows() {
        tRes.tested( "getRows()", oObj.getRows()!=null );
    } // getRows()

} // finish class _XColumnRowRange

