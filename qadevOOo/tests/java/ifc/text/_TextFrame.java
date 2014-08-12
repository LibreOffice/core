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
 * Testing <code>com.sun.star.text.TextFrame</code>
 * service properties :
 * <ul>
 *  <li><code> FrameHeightAbsolute</code></li>
 *  <li><code> FrameWidthAbsolute</code></li>
 *  <li><code> FrameWidthPercent</code></li>
 *  <li><code> FrameHeightPercent</code></li>
 *  <li><code> FrameIsAutomaticHeight</code></li>
 *  <li><code> SizeType</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextFrame
 */
public class _TextFrame extends MultiPropertyTest {

    /**
     * Property tester which switches two shorts.
     */
    protected PropertyTester WModeTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(Short.valueOf(com.sun.star.text.WritingMode2.LR_TB)))
                return Short.valueOf(com.sun.star.text.WritingMode2.TB_LR); else
                return Short.valueOf(com.sun.star.text.WritingMode2.LR_TB);
        }
    } ;

    /**
     * This property must have predefined values
     */
    public void _WritingMode() {
        log.println("Testing with custom Property tester") ;
        testProperty("WritingMode", WModeTester) ;
    }

} //finish class _TextFrame

