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

package ifc.drawing;

import com.sun.star.beans.PropertyValue;
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

