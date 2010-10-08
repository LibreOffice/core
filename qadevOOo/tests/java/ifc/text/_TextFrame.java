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
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(new Short(com.sun.star.text.WritingMode2.LR_TB)))
                return new Short(com.sun.star.text.WritingMode2.TB_LR); else
                return new Short(com.sun.star.text.WritingMode2.LR_TB);
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

