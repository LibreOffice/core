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

package ifc.sheet;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.sheet.Spreadsheet</code>
* service properties :
* <ul>
*  <li><code> IsVisible</code></li>
*  <li><code> PageStyle</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.Spreadsheet
*/
public class _Spreadsheet extends MultiPropertyTest {

    /**
     *This class is destined to custom test of property <code>PageStyle</code>.
     */
    protected PropertyTester styleTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            String str = "Default";
            String str2= "Report";
            return str.equals(oldValue) ? str2 : str;
        }
    };

    /**
     * Test property <code>PageStyle</code> using custom <code>PropertyTest</code>.
     */
    public void _PageStyle() {
        testProperty("PageStyle", styleTester);
    }
} // finish class _Spreadsheet


