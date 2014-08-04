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
import com.sun.star.text.XTextDocument;

/**
 * Testing <code>com.sun.star.text.XTextDocument</code>
 * interface methods :
 * <ul>
 *  <li><code> getText()</code></li>
 *  <li><code> reformat()</code></li>
 * </ul> <p>
 * Test is multithread compliant. <p>
 * @see com.sun.star.text.XTextDocument
 */
public class _XTextDocument extends MultiMethodTest {

    public XTextDocument oObj = null;
    public XText myText = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getText(){
        myText = oObj.getText();
        tRes.tested("getText()", myText != null );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method successfully returns
     * and no exceptions were thrown. <p>
     */
     public void _reformat(){
        oObj.reformat();
        tRes.tested("reformat()", true );
    }

}  // finish class _XTextDocument

