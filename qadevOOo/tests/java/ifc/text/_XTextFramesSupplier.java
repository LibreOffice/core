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

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextFramesSupplier;
import lib.MultiMethodTest;


/**
 * Testing <code>com.sun.star.text.XTextFramesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getTextFrames()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XTextFramesSupplier
 */
public class _XTextFramesSupplier extends MultiMethodTest {

    public static XTextFramesSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the collection returned has at least
     * one element.
     */
    public void _getTextFrames() {
        boolean res = false;

        XNameAccess the_frames = oObj.getTextFrames();
        log.println("Found: ");
        util.dbg.printArray(the_frames.getElementNames());
        res = the_frames.getElementNames().length>0;

        tRes.tested("getTextFrames()",res);
    }

}  // finish class _XTextFramesSupplier
