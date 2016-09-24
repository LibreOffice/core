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
 * Test is <b> NOT </b> multithread compliant. <p>
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


