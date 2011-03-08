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

import lib.MultiMethodTest;

import com.sun.star.text.XText;
import com.sun.star.text.XTextFrame;


/**
 * Testing <code>com.sun.star.text.XTextFrame</code>
 * interface methods :
 * <ul>
 *  <li><code> getText()</code></li>
 * </ul> <p>
 *
 * The frame <b>must contain</b> the text 'The FrameText'
 * for proper interface testing. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextFrame
 */
public class _XTextFrame extends MultiMethodTest {

    public XTextFrame oObj = null;

    /**
     * Gets the text of the frame. <p>
     *
     * Has <b>OK</b> status if the text is equal to 'The FrameText'.
     */
    public void _getText() {

        log.println("Testing getText ...");
        XText oFrameText = oObj.getText();
        String Content = oFrameText.getString();
        tRes.tested( "getText()",Content.equals("The FrameText") );
    }

}  // finish class _XTextFrame


