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

package ifc.drawing;

import com.sun.star.drawing.LineDash;
import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.drawing.LineProperties</code>
* service properties :
* <ul>
*  <li><code> LineStyle</code></li>
*  <li><code> LineDash</code></li>
*  <li><code> LineColor</code></li>
*  <li><code> LineTransparence</code></li>
*  <li><code> LineWidth</code></li>
*  <li><code> LineJoint</code></li>
*  <li><code> LineStartName</code></li>
*  <li><code> LineStart</code></li>
*  <li><code> LineEnd</code></li>
*  <li><code> LineStartCenter</code></li>
*  <li><code> LineStartWidth</code></li>
*  <li><code> LineEndCenter</code></li>
*  <li><code> LineEndWidth</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.LineProperties
*/
public class _LineProperties extends MultiPropertyTest {

    /**
     * Tester used for property LineStartName which can have
     * only predefined String values.
     */
    protected PropertyTester LineTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals("Arrow"))
                return "Square"; else
                return "Arrow";
        }
    }  ;

    /**
     * The property switched between 'Square' and 'Arrow' values.
     */
    public void _LineStartName() {
        log.println("Testing with custom Property tester");
        testProperty("LineStartName", LineTester) ;
    }

    public void _LineDash() {
        LineDash aLineDash = new LineDash();
        LineDash aLineDash2 = new LineDash();
        aLineDash.DashLen = 5;
        aLineDash2.DashLen = 1;
        testProperty("LineDash",aLineDash,aLineDash2);
    }
}

