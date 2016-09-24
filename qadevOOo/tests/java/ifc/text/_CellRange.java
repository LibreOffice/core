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

/**
* Testing <code>com.sun.star.text.CellRange</code>
* service properties :
* <ul>
*  <li><code> BackColor</code></li>
*  <li><code> BackGraphicURL</code></li>
*  <li><code> BackGraphicFilter</code></li>
*  <li><code> BackGraphicLocation</code></li>
*  <li><code> BackTransparent</code></li>
*  <li><code> ChartColumnAsLabel</code></li>
*  <li><code> ChartRowAsLabel</code></li>
*  <li><code> NumberFormat</code></li>
*  <li><code> TopMargin</code></li>
*  <li><code> BottomMargin</code></li>
*
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code> crazy-blue.jpg </code> : jpeg image used to test
*  BackGraphicURL()</li>
*  <li> <code> space-metal.jpg </code> : jpeg image used to test
*  BackGraphicURL()</li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.CellProperties
*/
public class _CellRange extends MultiPropertyTest {

    /**
    * Redefined method returns value, that differs from property value.
    */
    public void _BackColor() {
        final Short val1 = Short.valueOf( (short) 4 );
        final Short val2 = Short.valueOf( (short) 6 );
        log.println("Testing with custom Property tester") ;
        testProperty("BackColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String propName, Object oldValue) {
                if ( oldValue.equals(val1) )
                    return val2;
                else
                    return val1;
            }
        });
    }

    /**
     * This property could be changed only when graphic
     * URL is set.
     */
    public void _BackGraphicFilter() {
        executeMethod("BackGraphicURL");
        testProperty("BackGraphicFilter");
    }

    /**
    * Redefined method returns value, that differs from property value.
    */
    public void _BackGraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("BackGraphicURL", new PropertyTester() {
            @Override
            protected Object getNewValue(String propName, Object oldValue) {
                if (oldValue.equals(util.utils.getFullTestURL
                        ("space-metal.jpg")))
                    return util.utils.getFullTestURL("crazy-blue.jpg");
                else
                    return util.utils.getFullTestURL("space-metal.jpg");
            }
        });
    }
}

