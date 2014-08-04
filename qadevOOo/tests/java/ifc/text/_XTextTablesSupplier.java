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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextTablesSupplier;


/**
 * Testing <code>com.sun.star.text.XTextTablesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getTextTables()</code></li>
 * </ul> <p>
 * The table collection tested <b>must</b> have a table
 * with name 'SwXTextDocument'. <p>
 * Test is multithread compliant. <p>
 * @see com.sun.star.text.XTextTablesSupplier
 */
public class _XTextTablesSupplier extends MultiMethodTest {

    public static XTextTablesSupplier oObj = null;

    /**
     * Get the collection of tables. Then check it. <p>
     * Has <b>OK</b> status if returned collection contains
     * element with name 'SwXTextDocument'.
     */
    public void _getTextTables() {
        boolean res = false;

        XNameAccess the_tables = oObj.getTextTables();
        res = the_tables.hasByName("SwXTextDocument");

        tRes.tested("getTextTables()",res);
    }

}  // finish class _XTextTablesSupplier

