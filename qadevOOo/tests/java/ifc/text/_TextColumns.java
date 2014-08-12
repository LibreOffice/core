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

import lib.MultiPropertyTest;

import com.sun.star.text.XTextColumns;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.text.TextColumns</code>
* service properties :
* <ul>
*  <li><code> IsAutomatic</code></li>
*  <li><code> AutomaticDistance</code></li>
*  <li><code> SeparatorLineWidth</code></li>
*  <li><code> SeparatorLineColor</code></li>
*  <li><code> SeparatorLineRelativeHeight</code></li>
*  <li><code> SeparatorLineVerticalAlignment</code></li>
*  <li><code> SeparatorLineIsOn</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.TextColumns
*/
public class _TextColumns extends MultiPropertyTest {


    /**
    * Redefined method returns specific value, that differs from property
    * value. ( (oldValue + referenceValue) / 2 ).
    */
    public void _AutomaticDistance() {
        log.println("Testing with custom Property tester") ;
        testProperty("AutomaticDistance", new PropertyTester() {
            @Override
            protected Object getNewValue(String propName, Object oldValue) {
                XTextColumns xTC = UnoRuntime.queryInterface
                    (XTextColumns.class,tEnv.getTestObject());
                int ref = xTC.getReferenceValue();
                int setting = ( ( (Integer) oldValue).intValue() + ref) / 2;
                return Integer.valueOf(setting);
            }
        });
    }


} //finish class _TextColumns

