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

package ifc.style;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.style.ParagraphPropertiesAsian</code>
* service properties :
* <ul>
*  <li><code> ParaIsHangingPunctuation </code></li>
*  <li><code> ParaIsCharacterDistance </code></li>
*  <li><code> ParaIsForbiddenRules </code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.ParagraphProperties
*/
public class _ParagraphPropertiesAsian extends MultiPropertyTest {
        /**
     * Custom tester for properties which have <code>boolean</code> type
     * and can be void, so if they have void value, the new value must
     * be specified. Switches between true and false.
     */
    protected PropertyTester BooleanTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if ((oldValue != null) &&
                    (oldValue.equals(new Boolean((boolean) false)))) {
                return new Boolean((boolean) true);
            } else {
                return new Boolean((boolean) false);
            }
        }
    };

    /**
     * Tested with custom property tester.
     */
    public void _ParaIsHangingPunctuation() {
        log.println("Testing with custom Property tester");
        testProperty("ParaIsHangingPunctuation", BooleanTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _ParaIsCharacterDistance() {
        log.println("Testing with custom Property tester");
        testProperty("ParaIsCharacterDistance", BooleanTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _ParaIsForbiddenRules() {
        log.println("Testing with custom Property tester");
        testProperty("ParaIsForbiddenRules", BooleanTester);
    }


}  // finish class _ParagraphProperties

