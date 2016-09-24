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
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            if ((oldValue != null) &&
                    (oldValue.equals(Boolean.FALSE))) {
                return Boolean.TRUE;
            } else {
                return Boolean.FALSE;
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

